/* menu.c
   This file written 2019-12-01 by E All

   For copyright and licensing, see file LICENSE
*/

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "marslander.h"

#define DEFAULT_PLATS 5

void display_menu()
{
    display_text("SW1 for 1P", 0, 0, set_pixels);
    display_text("SW2 for 2P", 0, 1, set_pixels);
    display_text("SW3 for HiScore", 0, 2, set_pixels);
    display_text("SW4 for help", 0, 3, set_pixels);
}

void display_help()
{
    display_text("R ship: BTN1-2", 0, 0, set_pixels);
    display_text("L ship: BTN3-4", 0, 1, set_pixels);
    display_text("R button thrusts", 0, 2, set_pixels);
    display_text("left and L right", 0, 3, set_pixels);
}

void count_down()
{
    display_text("Starting in 3...", 0, 3, set_pixels);
    display_update();
    sleep(ONE_SEC);
    display_text("Starting in 3...", 0, 3, clear_pixels);
    display_text("Starting in 2...", 0, 3, set_pixels);
    display_update();
    sleep(ONE_SEC);
    display_text("Starting in 2...", 0, 3, clear_pixels);
    display_text("Starting in 1...", 0, 3, set_pixels);
    display_update();
    sleep(ONE_SEC);
    display_text("Starting in 1...", 0, 3, clear_pixels);
    display_update();
}

void menu_mode()
{
    clear_leds();
    clear_screen();
    display_update();
    display_menu();
    while (game_state == 0) {
        switch (get_sw()) {
        case 1:
            clear_screen();
            count_down();
            ground_init(DEFAULT_PLATS);
            display_ground();
            game_state = 1;
            set_leds();
            break;
        case 2:
            clear_screen();
            count_down();
            ground_init(DEFAULT_PLATS);
            display_ground();
            game_state = 2;
            set_leds();
            break;
        case 4:
            clear_screen();
            display_list();
            display_update();
            break;
        case 8:
            clear_screen();
            display_help();
            display_update();
            break;
        default:
            clear_screen();
            display_menu();
            display_update();
            break;
        }
    }
}
