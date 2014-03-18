/**
 * @brief: uart_irq.c
 * @author: NXP Semiconductors
 * @author: Y. Huang
 * @date: 2014/02/28
 */

#include <LPC17xx.h>
#include "uart.h"
#include "uart_polling.h"
#include "k_process.h"
#include "k_message.h"
#include "message.h"
#include "k_memory.h"
#include "utils.h"
#include "string.h"
#include "k_proc.h"
#include "printf.h"


uint32_t buffer_size = USER_DATA_BLOCK_SIZE - 4;
uint8_t g_buffer[USER_DATA_BLOCK_SIZE - 4];
uint8_t* gp_buffer = "\0";
uint32_t buffer_index = 0;
uint8_t g_send_char = 0;
uint8_t g_char_in;
uint8_t g_char_out;
msg_buf_t* message = NULL;

extern uint32_t g_switch_flag;

extern uint32_t k_release_processor(void);

void irq_i_process(void);

/**
 * @brief: initialize the n_uart
 * NOTES: It only supports UART0. It can be easily extended to support UART1 IRQ.
 * The step number in the comments matches the item number in Section 14.1 on pg 298
 * of LPC17xx_UM
 */
int uart_irq_init(int n_uart) {

    LPC_UART_TypeDef* pUart;

    if ( n_uart == 0 ) {
        /*
        Steps 1 & 2: system control configuration.
        Under CMSIS, system_LPC17xx.c does these two steps

        -----------------------------------------------------
        Step 1: Power control configuration.
                See table 46 pg63 in LPC17xx_UM
        -----------------------------------------------------
        Enable UART0 power, this is the default setting
        done in system_LPC17xx.c under CMSIS.
        Enclose the code for your refrence
        //LPC_SC->PCONP |= BIT(3);

        -----------------------------------------------------
        Step2: Select the clock source.
               Default PCLK=CCLK/4 , where CCLK = 100MHZ.
               See tables 40 & 42 on pg56-57 in LPC17xx_UM.
        -----------------------------------------------------
        Check the PLL0 configuration to see how XTAL=12.0MHZ
        gets to CCLK=100MHZin system_LPC17xx.c file.
        PCLK = CCLK/4, default setting after reset.
        Enclose the code for your reference
        //LPC_SC->PCLKSEL0 &= ~(BIT(7)|BIT(6));

        -----------------------------------------------------
        Step 5: Pin Ctrl Block configuration for TXD and RXD
                See Table 79 on pg108 in LPC17xx_UM.
        -----------------------------------------------------
        Note this is done before Steps3-4 for coding purpose.
        */

        /* Pin P0.2 used as TXD0 (Com0) */
        LPC_PINCON->PINSEL0 |= (1 << 4);

        /* Pin P0.3 used as RXD0 (Com0) */
        LPC_PINCON->PINSEL0 |= (1 << 6);

        pUart = (LPC_UART_TypeDef*) LPC_UART0;

    } else if ( n_uart == 1) {

        /* see Table 79 on pg108 in LPC17xx_UM */
        /* Pin P2.0 used as TXD1 (Com1) */
        LPC_PINCON->PINSEL4 |= (2 << 0);

        /* Pin P2.1 used as RXD1 (Com1) */
        LPC_PINCON->PINSEL4 |= (2 << 2);

        pUart = (LPC_UART_TypeDef*) LPC_UART1;

    } else {
        return 1; /* not supported yet */
    }

    /*
    -----------------------------------------------------
    Step 3: Transmission Configuration.
            See section 14.4.12.1 pg313-315 in LPC17xx_UM
            for baud rate calculation.
    -----------------------------------------------------
        */

    /* Step 3a: DLAB=1, 8N1 */
    pUart->LCR = UART_8N1; /* see uart.h file */

    /* Step 3b: 115200 baud rate @ 25.0 MHZ PCLK */
    pUart->DLM = 0; /* see table 274, pg302 in LPC17xx_UM */
    pUart->DLL = 9; /* see table 273, pg302 in LPC17xx_UM */

    /* FR = 1.507 ~ 1/2, DivAddVal = 1, MulVal = 2
       FR = 1.507 = 25MHZ/(16*9*115200)
       see table 285 on pg312 in LPC_17xxUM
    */
    pUart->FDR = 0x21;



    /*
    -----------------------------------------------------
    Step 4: FIFO setup.
           see table 278 on pg305 in LPC17xx_UM
    -----------------------------------------------------
        enable Rx and Tx FIFOs, clear Rx and Tx FIFOs
    Trigger level 0 (1 char per interrupt)
    */

    pUart->FCR = 0x07;

    /* Step 5 was done between step 2 and step 4 a few lines above */

    /*
    -----------------------------------------------------
    Step 6 Interrupt setting and enabling
    -----------------------------------------------------
    */
    /* Step 6a:
       Enable interrupt bit(s) wihtin the specific peripheral register.
           Interrupt Sources Setting: RBR, THRE or RX Line Stats
       See Table 50 on pg73 in LPC17xx_UM for all possible UART0 interrupt sources
       See Table 275 on pg 302 in LPC17xx_UM for IER setting
    */
    /* disable the Divisior Latch Access Bit DLAB=0 */
    pUart->LCR &= ~(BIT(7));

    //pUart->IER = IER_RBR | IER_THRE | IER_RLS;
    pUart->IER = IER_RBR | IER_RLS;

    /* Step 6b: enable the UART interrupt from the system level */

    if ( n_uart == 0 ) {
        NVIC_EnableIRQ(UART0_IRQn); /* CMSIS function */
    } else if ( n_uart == 1 ) {
        NVIC_EnableIRQ(UART1_IRQn); /* CMSIS function */
    } else {
        return 1; /* not supported yet */
    }

    pUart->THR = '\0';
    return 0;
}

/**
 * @brief: use CMSIS ISR for UART0 IRQ Handler
 * NOTE: This example shows how to save/restore all registers rather than just
 *       those backed up by the exception stack frame. We add extra
 *       push and pop instructions in the assembly routine.
 *       The actual c_UART0_IRQHandler does the rest of irq handling
 */
__asm void UART0_IRQHandler(void) {
    CPSID i                         ;// disable interrupts
    PRESERVE8
    IMPORT c_UART0_IRQHandler
    IMPORT k_release_processor
    PUSH {r4 - r11, lr}
    BL c_UART0_IRQHandler
    LDR R4, = __cpp(&g_switch_flag) ;// check switch_flag set by handler
    LDR R4, [R4]
    MOV R5, #0
    CMP R4, R5
    CPSIE i                         ;// enable interrupts
    BEQ  RESTORE                    ;// if g_switch_flag == 0, then restore the process that was interrupted
    BL k_release_processor          ;// otherwise (i.e g_switch_flag == 1, then switch to the other process)

RESTORE
    POP {r4 - r11, pc}
}

void c_UART0_IRQHandler(void) {
    irq_i_process();
}

void reset_g_buffer() {
    uint32_t i;

    buffer_index = 0;

    for (i = 0; i < buffer_size; i++) {
        g_buffer[i] = '\0';
    }
}

void irq_i_process(void) {
    LPC_UART_TypeDef* pUart = (LPC_UART_TypeDef*)LPC_UART0;
    uint8_t IIR_IntId; // Interrupt ID from IIR
    msg_buf_t* read_msg;
    node_t* previous_pcb_node = current_pcb_node;

    g_switch_flag = 0;

    // Reading IIR automatically acknowledges the interrupt
    IIR_IntId = (pUart->IIR) >> 1 ; // skip pending bit in IIR

    if (IIR_IntId & IIR_RDA) {
        /**************************************************************************
        * Interrupt came from Receive Buffer Register (i.e. a keyboard input)
        **************************************************************************/

        g_char_in = pUart->RBR; // read input and clear the interrupt
        g_send_char = 1;

        if ((buffer_index < buffer_size - 3) && (g_char_in != '\r')) {
            if (g_char_in != 8 && g_char_in != 127) {
                g_buffer[buffer_index++] = g_char_in;
            } else {
                // if it's a backspace or delete, make index go back by 1
                if (buffer_index > 0) {
                    buffer_index--;
                }
            }

            pUart->THR = g_char_in;
        } else {
            g_buffer[buffer_index++] = '\r';
            pUart->THR = '\r';
            g_buffer[buffer_index++] = '\n';
            pUart->THR = '\n';
            g_buffer[buffer_index++] = '\0';
            pUart->THR = '\0';

            // prepare message to kcd to decode
            read_msg = (msg_buf_t*)k_request_memory_block_i();
            if (read_msg == NULL) {
                return;
            }
            read_msg->msg_type = DEFAULT;
            strncpy(read_msg->msg_data, (char*)g_buffer, buffer_index);

            // send message to kcd
            current_pcb_node = pcb_nodes[PID_UART_IPROC];
            k_send_message_i(PID_KCD, read_msg);
            current_pcb_node = previous_pcb_node;

            reset_g_buffer();
            g_switch_flag = 1;
        }

#ifdef _DEBUG_HOTKEYS
        if (g_char_in == KEY_READY_QUEUE ||
            g_char_in == KEY_BLOCKED_MEM_QUEUE ||
            g_char_in == KEY_BLOCKED_MSG_QUEUE ||
            g_char_in == KEY_MSG_LOG_BUFFER) {

            PRINT_HEADER;
            println("CURRENT PROCESS:");
            println("PID:%d Priority:%d SP:%x", ((pcb_t*)current_pcb_node->value)->pid, ((pcb_t*)current_pcb_node->value)->priority, ((pcb_t*)current_pcb_node->value)->stack_ptr);
            PRINT_NEWLINE;

            switch (g_char_in) {
                case KEY_READY_QUEUE:
                    println("READY QUEUE:");
                    k_print_queues(ready_pqs);
                    break;

                case KEY_BLOCKED_MEM_QUEUE:
                    println("MEM BLOCKED QUEUE:");
                    k_print_queues(mem_blocked_pqs);
                    break;

                case KEY_BLOCKED_MSG_QUEUE:
                    println("MESSAGE BLOCKED QUEUE:");
                    k_print_queues(msg_blocked_pqs);
                    break;

                case KEY_MSG_LOG_BUFFER:
                    // TODO fix later...
                    //k_print_msg_logs(); 
                    break;

                default:
                    break;
            }

            PRINT_HEADER;
        }
#endif

    } else if (IIR_IntId & IIR_THRE) {
        /**************************************************************************
        * Interrupt came from empty Transmit Holding Register
        * (i.e. we want to print something)
        **************************************************************************/

        if (*gp_buffer != '\0' ) {
PRINT:
            g_char_out = *gp_buffer;
            pUart->THR = g_char_out;
            gp_buffer++;
        } else {
            k_release_memory_block_i(message);

            current_pcb_node = pcb_nodes[PID_UART_IPROC];
            message = k_receive_message_i(NULL);
            current_pcb_node = previous_pcb_node;

            if (message != NULL) {
                // got a message so we set the buffer to it
                // after this interrupt finishes, another THRE interrupt will fire
                gp_buffer = (uint8_t*)message->msg_data;
                goto PRINT; // need to make sure something is in our THR otherwise it'll enter this else block again immediately
            } else {
                // no more messages so we finished printing
                pUart->IER ^= IER_THRE; // toggle the IER_THRE bit
                pUart->THR = '\0';
                g_send_char = k_should_preempt_current_process();
            }
        }
    } else {
        DEBUG_PRINT("irq_i_process cannot handle the interrupt");
        return;
    }
}

/**
 * a THRE interrupt will fire immediately since the Transmission Holding Register is empty
 * this will allow the irq_i_process() to read in its messages and print it to the uart
 */
void read_interrupt() {
    LPC_UART_TypeDef* pUart = (LPC_UART_TypeDef*)LPC_UART0;
    pUart->IER |= IER_THRE; //
}
