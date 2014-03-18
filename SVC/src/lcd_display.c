#include "lcd_display.h"
#include "GLCD.h"
#include "utils.h"

#define SMALL_FONT  0
#define BIG_FONT    1

void display_logo(void) {
    uint32_t line;
	
    GLCD_Clear(White); // clear display
	
    GLCD_SetBackColor(Blue);
    GLCD_SetTextColor(White);
    GLCD_DisplayString(0, 0, BIG_FONT, "    OS WOW     ");
    GLCD_SetBackColor(White);
    GLCD_SetTextColor(Black);

    line = 3;

    // www.degraeve.com/img2txt.php
    GLCD_DisplayString(line++, 0, SMALL_FONT, "...i..,.iiii.ii..:::::::::::::::::::::::");
    GLCD_DisplayString(line++, 0, SMALL_FONT, "..i.....i.i,.ij..:::::::::::::::.,::::::");
    GLCD_DisplayString(line++, 0, SMALL_FONT, "......i.,,,:,ijj.:::::::::::::,ijf,:::::");
    GLCD_DisplayString(line++, 0, SMALL_FONT, "..ii....,,::.ijjj:::::::::::::itjGL:::::");
    GLCD_DisplayString(line++, 0, SMALL_FONT, "ii..,,,,:::,.itfjj.::::::::::ijjjfL:::::");
    GLCD_DisplayString(line++, 0, SMALL_FONT, ",,:::,,,::,.ifiiiiittiitt.,,tjffffft::::");
    GLCD_DisplayString(line++, 0, SMALL_FONT, ":::,,,,,,..ii..iittiiiiiitjjjGjfGLff::::");
    GLCD_DisplayString(line++, 0, SMALL_FONT, ",,,..ittjti...iiittttiiiittLLGjLEEfL::::");
    GLCD_DisplayString(line++, 0, SMALL_FONT, "itjjjjjt......itjttitttttttiGLjfEDff::::");
    GLCD_DisplayString(line++, 0, SMALL_FONT, "ffjjjji......iijjttttttttttiiLfLKDjj::::");
    GLCD_DisplayString(line++, 0, SMALL_FONT, "fjjjt.,,,..,,itjttittiittttiiifKKLtf::::");
    GLCD_DisplayString(line++, 0, SMALL_FONT, "jjjti,.,.....ijttiittiiitttiiiifLffG::::");
    GLCD_DisplayString(line++, 0, SMALL_FONT, "jjtt.,,,i.i..tjj....ttiiiiiitiiifjjt.:::");
    GLCD_DisplayString(line++, 0, SMALL_FONT, "tti.,::,fWG.ttji....ittttti.iiiiijfLi:::");
    GLCD_DisplayString(line++, 0, SMALL_FONT, "...,,..,KWKi..ii.itjitjjttiiiiiiiifLt.::");
    GLCD_DisplayString(line++, 0, SMALL_FONT, ",,,,..:iEEi....ttGij#jtttiiiiiitiitGf.::");
    GLCD_DisplayString(line++, 0, SMALL_FONT, ",,,::.,it.i....jGKfWWKjtiiiiiiiiiitffi::");
    GLCD_DisplayString(line++, 0, SMALL_FONT, ",,,.::,.,...,..jjEGKD..........iiiitji::");
    GLCD_DisplayString(line++, 0, SMALL_FONT, ".,,:.:::..i...iiiiii.i.,,,........ittt::");
    GLCD_DisplayString(line++, 0, SMALL_FONT, "..:::,jLLGti,,..iiii...,,.ii......iitt::");
    GLCD_DisplayString(line++, 0, SMALL_FONT, "..:::G#WWKEt,,.ii.,.....,,,,.,.....iii,:");
    GLCD_DisplayString(line++, 0, SMALL_FONT, "i.:,:jWK##Wf,......,.,,..,....,,....iii:");
    GLCD_DisplayString(line++, 0, SMALL_FONT, "..:,,fKWWWDt.tiii.,.,,.,,....,..,..iiii:");
    GLCD_DisplayString(line++, 0, SMALL_FONT, "i.:,.fDWKDft...i.....,,...........iitii,");
    GLCD_DisplayString(line++, 0, SMALL_FONT, "i,:,.fDEEDLft.tii.i..i..i......iiiitjti.");
    GLCD_DisplayString(line++, 0, SMALL_FONT, "i.:,.iE##DLtititfiiiiiiiii...iiii.itjii.");
    GLCD_DisplayString(line++, 0, SMALL_FONT, "i,:,,iDKEKWGttfDDtiiii.ii...iii..ittjtti");
    GLCD_DisplayString(line++, 0, SMALL_FONT, "::,:,.iDEDEEKGGfj.it.i.....i.....ittttti");
    GLCD_DisplayString(line++, 0, SMALL_FONT, "::::,..jjfjjtfttiiiii...i.i..,...ittjtti");
    GLCD_DisplayString(line++, 0, SMALL_FONT, "::,,,...iiiiiiiiiiiiiii..........ttjtttt");
    GLCD_DisplayString(line++, 0, SMALL_FONT, ":::,,.....iiiiittiti...ii.i.iiiiittjjjji");
    GLCD_DisplayString(line++, 0, SMALL_FONT, ":::,,.......iiiiiti...iiii...iitijjtjfjt");
    GLCD_DisplayString(line++, 0, SMALL_FONT, ":::.....i.iiiiiiti.i....i...iititjtjjjtt");
    GLCD_DisplayString(line++, 0, SMALL_FONT, "::,.....iiiiiiiiiiiii..,..iiiiittjjfjttt");
    GLCD_DisplayString(line++, 0, SMALL_FONT, ",,,..i...iiiiiitiiiii...i.iiiiittjffjtti");
    GLCD_DisplayString(line++, 0, SMALL_FONT, ",,,....i..iiiiitiitiiiiiitiiiiiittjjiii.");
    GLCD_DisplayString(line++, 0, SMALL_FONT, ",,,,...iiiiiiiiitttiititiiiiiiittjjtii..");

    GLCD_SetTextColor(Navy);
    GLCD_DisplayString(2, 2, BIG_FONT, "wow");

    GLCD_SetTextColor(DarkGreen);
    GLCD_DisplayString(8, 24, BIG_FONT, "such c");

    GLCD_SetTextColor(Red);
    GLCD_DisplayString(11, 2, BIG_FONT, "much faults");
}

void init_logo_on_lcd(void) {
    GLCD_Init();
    display_logo();
}
