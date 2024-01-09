/*
  input.c
  This file written 2019-09-24 by D Ljunggren

  For copyright and licensing, see file LICENSE
*/

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "marslander.h"

void input_init() {
    TRISDSET = 0xfe0; // SW1-4 in 0xf00
    TRISFSET = 2; // BTN1
}

/* Return SW1-4 (bit 8-11 in PORTD) at LSB */
int get_sw()
{
    return ((PORTD>>8) & 0xf);
}

/* Return status of BTN1-4 at LSB */
int get_btns()
{
    uint8_t btn2_4 = PORTD & (7<<5);
    uint8_t btn1 = (PORTF >> 1) & 1;
    return (btn2_4 >> 4) | btn1;
}
