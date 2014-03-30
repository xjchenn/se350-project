/**
 * @brief timer.c - Timer example code. Tiemr IRQ is invoked every 1ms
 * @author T. Reidemeister
 * @author Y. Huang
 * @author NXP Semiconductors
 * @date 2012/02/12
 */

#include <LPC17xx.h>
#include "uart_def.h"
#include "timer.h"
#include "k_message.h"
#include "printf.h"
#include "k_process.h"

volatile uint32_t g_timer_count = 0; // increment every 1 ms
volatile uint32_t test_timer_count = 0; // incremenet every 10 microseconds

linkedlist_t timeout_queue; // queue of messages
int32_t switch_flag = 0;

/**
 * @brief: initialize timer. Only timer 0 is supported
 */
uint32_t timer_init(uint8_t n_timer) {
    LPC_TIM_TypeDef* pTimer;
    LPC_TIM_TypeDef* testTimer;
    if (n_timer == 0) {
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
        //LPC_SC->PCONP |= BIT(1);

        -----------------------------------------------------
        Step2: Select the clock source,
               default PCLK=CCLK/4 , where CCLK = 100MHZ.
               See tables 40 & 42 on pg56-57 in LPC17xx_UM.
        -----------------------------------------------------
        Check the PLL0 configuration to see how XTAL=12.0MHZ
        gets to CCLK=100MHZ in system_LPC17xx.c file.
        PCLK = CCLK/4, default setting in system_LPC17xx.c.
        Enclose the code for your reference
        //LPC_SC->PCLKSEL0 &= ~(BIT(3)|BIT(2));

        -----------------------------------------------------
        Step 3: Pin Ctrl Block configuration.
                Optional, not used in this example
                See Table 82 on pg110 in LPC17xx_UM
        -----------------------------------------------------
        */
        pTimer = (LPC_TIM_TypeDef*) LPC_TIM0;

        /* Step 4.1: Prescale Register PR setting
           CCLK = 100 MHZ, PCLK = CCLK/4 = 25 MHZ
           2*(12499 + 1)*(1/25) * 10^(-6) s = 10^(-3) s = 1 ms
           TC (Timer Counter) toggles b/w 0 and 1 every 12500 PCLKs
           see MR setting below
        */
        pTimer->PR = 12499;

        /*
        -----------------------------------------------------
        Step 4: Interrupts configuration
        -----------------------------------------------------
        */

        /* Step 4.2: MR setting, see section 21.6.7 on pg496 of LPC17xx_UM. */

        pTimer->MR0 = 1;

        /* Step 4.3: MCR setting, see table 429 on pg496 of LPC17xx_UM.
           Interrupt on MR0: when MR0 mathches the value in the TC,
                             generate an interrupt.
           Reset on MR0: Reset TC if MR0 mathches it.
        */

        pTimer->MCR = BIT(0) | BIT(1);

        g_timer_count = 0;

        /* Step 4.4: CSMSIS enable timer0 IRQ */
        NVIC_EnableIRQ(TIMER0_IRQn);
        
        /* Step 4.5: Enable the TCR. See table 427 on pg494 of LPC17xx_UM. */
        pTimer->TCR = 1;

        // Initialize the linkedlist
        linkedlist_init(&timeout_queue);

    } else if (n_timer == 1) { /* Initialize timer 1 */
        
        testTimer = (LPC_TIM_TypeDef*) LPC_TIM1;
        /*
        PR Value Reference:

                PR of 12499 = 1   millisecond
                PR of 1249  = 100 microseconds
                PR of 124   = 10  microseconds
                PR of 11.5  = 1   microsecond

        */
        testTimer->PR = 11.5;
        testTimer->MR0 = 1;

        testTimer->MCR = BIT(0) | BIT(1);

        test_timer_count = 0;

        NVIC_EnableIRQ(TIMER1_IRQn);

        testTimer->TCR = 1;
    } else {
        return 1;
    }
    return 0;
}

/**
 * @brief: use CMSIS ISR for TIMER0 IRQ Handler
 * NOTE: This example shows how to save/restore all registers rather than just
 *       those backed up by the exception stack frame. We add extra
 *       push and pop instructions in the assembly routine.
 *       The actual c_TIMER0_IRQHandler does the rest of irq handling
 */
__asm void TIMER0_IRQHandler(void) {
    CPSID i                         ;// disable interrupts
    PRESERVE8
    IMPORT c_TIMER0_IRQHandler
    IMPORT k_release_processor
    PUSH {r4 - r11, lr}
    BL c_TIMER0_IRQHandler
    LDR R4, = __cpp(&switch_flag)
    LDR R4, [R4]
    MOV R5, #0
    CMP R4, R5
    CPSIE i                         ;// enable interrupts
    BEQ RESTORE_0
    BL k_release_processor

RESTORE_0
    POP {r4 - r11, pc}
}

__asm void TIMER1_IRQHandler(void) {
    CPSID i                         ;// disable interrupts
    PRESERVE8
    IMPORT c_TIMER1_IRQHandler
    IMPORT k_release_processor
    PUSH {r4 - r11, lr}
    BL c_TIMER1_IRQHandler
    LDR R4, = __cpp(&switch_flag)
    LDR R4, [R4]
    MOV R5, #0
    CMP R4, R5
    CPSIE i                         ;// enable interrupts
    BEQ RESTORE_1
    BL k_release_processor

RESTORE_1
    POP {r4 - r11, pc}
}

void c_TIMER0_IRQHandler(void) {
    /* ack inttrupt, see section  21.6.1 on pg 493 of LPC17XX_UM */
    LPC_TIM0->IR = BIT(0);
    g_timer_count++;
    timer_i_process();
}

void c_TIMER1_IRQHandler(void) {
    LPC_TIM1->IR = BIT(0);
    test_timer_count++;
}

void timer_i_process(void) {
    node_t* previous_pcb_node = current_pcb_node;
    node_t* queue_iter;
    message_t* current_message;
    switch_flag = 0;

    queue_iter = timeout_queue.first;
    while (queue_iter != NULL && ((message_t*)queue_iter)->expiry <= g_timer_count) {
        current_message = (message_t*)linkedlist_pop_front(&timeout_queue);

        current_pcb_node = pcb_nodes[current_message->sender_pid];
        k_send_message_i(current_message->receiver_pid, USER_MSG_ADDR(current_message));
        current_pcb_node = previous_pcb_node;
        
        if (pcbs[current_message->receiver_pid]->priority <= ((pcb_t*)previous_pcb_node->value)->priority) {
            switch_flag = 1;
        }

        if (pcbs[current_message->receiver_pid]->priority <= ((pcb_t*)previous_pcb_node->value)->priority) {
            switch_flag = 1;
        }

        queue_iter = timeout_queue.first;
    }
}
