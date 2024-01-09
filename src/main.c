/* init.c

   This file written 2015 by Axel Isaksson,
   modified 2015, 2017 by F Lundevall
   Latest update 2019-10-13 by E All

   For copyright and licensing, see file LICENSE
*/

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "marslander.h"


void main_init(void)
{
    /*
      This will set the peripheral bus clock to the same frequency
      as the sysclock. That means 80 MHz, when the microcontroller
      is running at 80 MHz. Changed 2017, as recommended by Axel.
    */
    SYSKEY = 0xAA996655;  /* Unlock OSCCON, step 1 */
    SYSKEY = 0x556699AA;  /* Unlock OSCCON, step 2 */
    while(OSCCON & (1 << 21)); /* Wait until PBDIV ready */
    OSCCONCLR = 0x180000; /* clear PBDIV bit <0,1> */
    while(OSCCON & (1 << 21));  /* Wait until PBDIV ready */
    SYSKEY = 0x0;  /* Lock OSCCON */
    
    /* Set up output pins */
    AD1PCFG = 0xFFFF;
    ODCE = 0x0;
    TRISECLR = 0xFF;
    PORTE = 0x0;
    
    /* Output pins for display signals */
    PORTF = 0xFFFF;
    PORTG = (1 << 9);
    ODCF = 0x0;
    ODCG = 0x0;
    TRISFCLR = 0x70;
    TRISGCLR = 0x200;
    
    /* Set up SPI as master */
    SPI2CON = 0;
    SPI2BRG = 4;
    /* SPI2STAT bit SPIROV = 0; */
    SPI2STATCLR = 0x40;
    /* SPI2CON bit CKP = 1; */
    SPI2CONSET = 0x40;
    /* SPI2CON bit MSTEN = 1; */
    SPI2CONSET = 0x20;
    /* SPI2CON bit ON = 1; */
    SPI2CONSET = 0x8000;

    game_state = 0;
}

void timer_init()
{
    TMR2 = 0; // Reset the timer
    T2CON = (7<<4); // Prescaling 1:256
    PR2 = ((80000000 / 256) / 100); // Period to 1/100s
    IECSET(0) = (1<<8); // Set bits for
    IPCSET(2) = (0x1f); // timer interruption and priority (highest)
    T2CONSET = (1<<15); // Init the timer
}

void enable_interrupts() {
    asm volatile("ei");
}

void howdy()
{
    display_text("Good luck!", text_center(10), 0, set_pixels);
    display_text("You're gonna", text_center(11), 1, set_pixels);
    display_text("need it!", text_center(8), 2, set_pixels);
    display_update();
    sleep(ONE_SEC<<2);
}

int main(void)
{
    main_init();
    display_init();
    input_init();
    led_init();
    timer_init();
    init_hi_scores();
    howdy();
    menu_mode();
    enable_interrupts();
    return 0;
}
