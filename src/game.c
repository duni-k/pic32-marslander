/* game.c

   This file written 2019-11-13 by Edvard All
   Latest update by D Ljunggren 2019-12-09

   For copyright and licensing, see file LICENSE
*/

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include <stdlib.h>
#include "marslander.h"

// Returns 1 if we landed, 0 if we crashed
uint8_t landed(struct player p)
{
    if ((p.x_speed <= CRASH_TRESHOLD) && (p.x_speed >= -CRASH_TRESHOLD)
        && (p.y_speed <= CRASH_TRESHOLD)) {
        int16_t x_actual;
        int i;
        for (i = 1; i < SHIP_SIZE; ++i)
            x_actual = (p.x_pos / SCALE) + GROUND_OFFSET;
            if (ground_arr[x_actual] != ground_arr[x_actual + i])
                return 0;
        return 1;
    }
    return 0;
}

uint8_t game_over(struct player p)
{
    int16_t x_actual = p.x_pos / SCALE + GROUND_OFFSET;
    int16_t y_actual = p.y_pos / SCALE + SHIP_SIZE;
    if (((y_actual) >= ground_arr[x_actual])
        || (y_actual >= ground_arr[x_actual + SHIP_SIZE]))
        return 1;
    return 0;
}

uint8_t check_quit_pause(void (*clear_hud)(), void (*reset_game)())
{
    uint8_t sw = get_sw();
    if (sw == 3) {
        clear_hud();
        display_text("Paused.", text_center(6), 0, set_pixels);
        display_update();
        while ((sw = get_sw()) == 3)
            ;
        display_text("Paused.", text_center(6), 0, clear_pixels);
    }
    if (sw == 10) {
        game_state = 0;
        reset_game();
        clear_screen();
    }
}

/*
  We prandomise the first byte of ground and then all succeeding ground
  is based on the previous one with a prandom offset between 1 and -1
*/
void ground_init(uint8_t platform_c)
{
    int8_t noise;
    int8_t dir = 0;
    int8_t height = 18;

    srand(TMR2);

    /* Init the first ground-byte */
    uint8_t y = (32 - (rand() % height));
    ground_arr[0] = y;

    int x, i;
    for (x = 1; x < (GROUND_LEN - 1); ++x) {
        srand(TMR2);
        noise =  rand() % 2; //

        /* We don't want the ground to go too far up or down so we use 'dir' to
           keep track of where we're going in order to create a (more or less)
           sin-like wave */
        if (y >= 31) {
            dir = 1;
        } else if (y < height)
            dir = -1;
        switch (dir) {
        case 1:
            y -= noise;
            break;
        default:
            y += noise;
        }

        // Make manual platforms
        if ((TMR2 % platform_c) == 0) {
            for (i = 0; i < PLATFORM_SIZE; ++i)
                ground_arr[x + i] = y;
            x += PLATFORM_SIZE - 1;
            continue;
        }
        ground_arr[x] = y;
    }
}

/* Interrupt Service Routine is run on timer interrupt
   (100 times every second) */
void game_isr(void)
{
    IFSCLR(0) = (1<<8);
    switch (game_state) {
    case 0:
        menu_mode();
        break;
    case 1:
        sp_mode();
        break;
    case 2:
        mp_mode();
        break;
    }
}
