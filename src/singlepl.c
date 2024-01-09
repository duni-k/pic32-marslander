/* singlepl.c

   This file written 2019-12-01 by Edvard All

   For copyright and licensing, see file LICENSE
*/

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include "marslander.h"

#define INIT_POS -SHIP_SIZE*SCALE
#define FUEL_MAX 99

uint16_t platform_c = 5;
float fuel_step = 0.15;

struct player p = {
    .button_set = 2,
    .x_pos = INIT_POS,
    .y_pos = INIT_POS,
    .x_speed = SCALE,
    .y_speed = 0,
    .sprite = SHIP_SPRITE,
    .fuel = FUEL_MAX,
    .score = 0
};

void show_hud()
{
    /* fuel */
    display_text("F", 105, 0, set_pixels);
    display_text(itoaconv(p.fuel), 113, 0, set_pixels);
    update_fuel_leds(p.fuel, FUEL_MAX);
    /* score */
    display_text("S", 105, 1, set_pixels);
    display_text(itoaconv(p.score), 113, 1, set_pixels);
}

void clear_hud()
{
    display_text(itoaconv(p.fuel), 113, 0, clear_pixels);
    display_text("F", 105, 0, clear_pixels);
    display_text("S", 105, 1, clear_pixels);
    display_text(itoaconv(p.score), 113, 1, clear_pixels);
}

static void reset_game()
{
    p.score = 0;
    reset_player(&p, FUEL_MAX, INIT_POS, INIT_POS, 1);
    game_state = 0;
    fuel_step = 0.15;
    set_leds();
}

void continue_game()
{
    display_text("Stage cleared.", text_center(14), 0, set_pixels);
    display_update();
    sleep(ONE_SEC);

    reset_player(&p, FUEL_MAX, INIT_POS, INIT_POS, 1);
    platform_c += 5;
    fuel_step += 0.02;
    ++p.score;

    clear_screen();
    ground_init(platform_c);
    display_ground();
    set_leds();
}

static void post_game()
{
    uint8_t index;
    if ((p.score > 0) && (index = highscore(p.score)) >= 0) {
        display_text("Highscore!", text_center(10), 0, set_pixels);
        display_update();
        sleep(ONE_SEC<<1);

        char name[4] = "a__";
        prompt_name(name);
        insert_item(name, p.score, index);

        clear_screen();
        display_list();
    } else {
        display_text("Ouchy!", text_center(6), 0, set_pixels);
        display_update();
        sleep(ONE_SEC);

        clear_screen();
        display_text("Game over!", text_center(10), 0, set_pixels);
        display_text("Better luck", text_center(11), 1, set_pixels);
        display_text("next time.", text_center(10), 2, set_pixels);
        sprite_update(text_center(1), 24, happy_smiley, 8, set_pixels);
    }

    display_update();
    sleep(ONE_SEC<<1);
    reset_game();
}

void sp_mode()
{
    if (move(&p))
        use_fuel(&(p.fuel), fuel_step);

    if (ms++ >= 10) {
        ms = 0;

        check_quit_pause(clear_hud, reset_game);

        update_player_sprite(p, clear_pixels);
        update_player(&p);
        update_player_sprite(p, set_pixels);

        show_hud();
        display_update();
        clear_hud();

        if (game_over(p))
            if (landed(p)) {
                continue_game();
            } else
                post_game();
    }
}
