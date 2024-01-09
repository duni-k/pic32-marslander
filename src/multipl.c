/* multipl.c

   This file written 2019-12-01 by E All
   Last edited by D Ljunggren 2019-12-09

   For copyright and licensing, see file LICENSE
*/

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <math.h>
#include "marslander.h"

#define P1_XPOS -SHIP_SIZE*SCALE
#define P1_YPOS P1_XPOS
#define P2_XPOS (DISPLAY_COLS - 1)*SCALE
#define P2_YPOS P1_YPOS
#define FUEL_POS text_center(3)
#define PLATFORM_C 10
#define FIRST_TO 3
#define FUEL_STEP 0.05
#define FUEL_MAX 99

struct player p1 = {
    .button_set = 2,
    .x_pos = P1_XPOS,
    .y_pos = P1_YPOS,
    .x_speed = SCALE,
    .y_speed = 0,
    .sprite = SHIP_SPRITE,
    .fuel = FUEL_MAX,
    .score = 0
};

struct player p2 = {
    .button_set = 0,
    .x_pos = P2_XPOS,
    .y_pos = P2_YPOS,
    .x_speed = -SCALE,
    .y_speed = 0,
    .sprite = SHIP_SPRITE,
    .fuel = FUEL_MAX,
    .score = 0
};

/*
  Collision landing is 1 if p1 collision landed with p2 and 2 if the other way around.

  Returns 1 if player 1 won the series, 2 if player 2 won the series, 0 otherwise.
*/
uint8_t check_winner()
{
    uint8_t p1_land = landed(p1);
    uint8_t p2_land = landed(p2);
    if (p1_land && (!p2_land || (p1.fuel > p2.fuel))) {
        ++p1.score;
        if (p1.score >= FIRST_TO)
            return 1;
        display_text("P1 scores!", text_center(8), 0, set_pixels);
    } else if (p2_land && (p1.fuel != p2.fuel)) {
        ++p2.score;
        if (p2.score >= FIRST_TO)
            return 2;
        display_text("P2 scores!", text_center(8), 0, set_pixels);
    } else
        display_text("Draw!", text_center(5), 0, set_pixels);
    display_update();
    sleep(ONE_SEC);
    return 0;
}

/*
  Return 0 if no one landed, 1 if p1 landed, 2 if p2 landed and 3 if both landed
  By landed we here mean both crashed and landings.
*/
static uint8_t state()
{
    int i;
    if (game_over(p2)) {
        if (game_over(p1))
            return 3;
        return 2;
    }
    if (game_over(p1)) {
        return 1;
    }
    return 0;
}

static void post_game(uint8_t winner)
{
    display_text("Game over.", 30, 0, set_pixels);
    display_update();
    sleep(ONE_SEC);
    clear_screen();

    switch (winner) {
    case 1:
        display_text("P1", text_center(2), 0, set_pixels);
        break;
    case 2:
        display_text("P2", text_center(2), 0, set_pixels);
        break;
    }
    display_text(" won the series!", text_center(16), 1, set_pixels);

    display_text(itoaconv(p1.score), text_center(7), 2, set_pixels);
    display_text("-", text_center(1), 2, set_pixels);
    display_text(itoaconv(p2.score), text_center(7) + 6*8, 2, set_pixels);

    display_update();
    sleep(ONE_SEC<<2);

    game_state = 0;
    p1.score = 0;
    p2.score = 0;
}

static void reset_game()
{
    clear_screen();
    set_leds();
    ground_init(PLATFORM_C);
    display_ground();
    reset_player(&p1, FUEL_MAX, P1_XPOS, P1_YPOS, 1);
    reset_player(&p2, FUEL_MAX, P2_XPOS, P2_YPOS, -1);
}

/* Player1 fuel in text, Player2 fuel in LEDs*/
static void show_hud()
{
    display_text(itoaconv(p1.score), 0, 1, set_pixels);
    display_text(itoaconv(p2.score), (DISPLAY_COLS - 8), 1, set_pixels);
    display_text("F", FUEL_POS, 0, set_pixels);
    display_text(itoaconv(p1.fuel), FUEL_POS+8, 0, set_pixels);
    update_fuel_leds(p2.fuel, FUEL_MAX);
}

static void clear_hud()
{
    display_text(itoaconv(p1.score), 0, 0, clear_pixels);
    display_text(itoaconv(p2.score), (DISPLAY_COLS - 8), 0, clear_pixels);
    display_text("F", FUEL_POS, 0, clear_pixels);
    display_text(itoaconv(p1.fuel), FUEL_POS+8, 0, clear_pixels);
}

uint8_t has_collision()
{
    int32_t p1_xnext = (p1.x_pos + p1.x_speed) / SCALE;
    int32_t p2_xnext = (p2.x_pos + p2.x_speed) / SCALE;
    int32_t p1_ynext = (p1.y_pos + p1.y_speed) / SCALE;
    int32_t p2_ynext = (p2.y_pos + p2.y_speed) / SCALE;

    return ((p1_xnext <= (p2_xnext + SHIP_SIZE))
            && ((p1_xnext + SHIP_SIZE) >= p2_xnext)
            && (p1_ynext <= (p2_ynext + SHIP_SIZE))
            && ((p1_ynext + SHIP_SIZE) >= p2_ynext));
}

/*
  We check whether or not the ships are going to collide in the next frame,
  and if they will we adjust their positions to not 'get stuck inside eachother'
  and update speeds depending on who's the fastest moving object.

  However, if one of the ships have landed we won't adjust the landed ship's
  coordinate or speed, instead returning 1 if p1 crashed into p2, and vice versa.

  Returns 0 if no one crash landed.
*/
uint8_t handle_collision(uint8_t st)
{
    /* Collision, one ship is inside another, they're regarded as
       rectangular for this purpose */
    if (has_collision()) {
        if (st == 1)
            return 1;
        if (st == 2)
            return 2;

        if ((p1.x_speed ^ p2.x_speed) >> 31) {
            int16_t p1_xspeed = p1.x_speed;
            p1.x_speed = (-p1.x_speed >> 2) + (p2.x_speed >> 2);
            p2.x_speed = (-p2.x_speed >> 2) + (p1_xspeed >> 2);
        } else if (abs(p1.x_speed) > abs(p2.x_speed)){
            p2.x_speed += (p1.x_speed >> 2);
            p1.x_speed >> 2;
        } else {
            p1.x_speed += (p2.x_speed >> 2);
            p2.x_speed >> 2;
        }
        
        if ((p1.y_speed ^ p2.y_speed) >> 31) {
            int16_t p1_yspeed = p1.y_speed;
            p1.y_speed = (-p1.y_speed >> 2) + (p2.y_speed >> 2);
            p2.y_speed = (-p2.y_speed >> 2) + (p1_yspeed >> 2);
        } else if (abs(p1.y_speed) > abs(p2.y_speed)){
            p2.y_speed += (p1.y_speed >> 2);
            p1.y_speed >> 2;
        } else {
            p1.y_speed += (p2.y_speed >> 2);
            p2.y_speed >> 2;
        }
    }
    return 0;
}

void mp_mode()
{
    /* 1 = p1 reached ground, 2 = p2 ditto, 3 = both did */
    uint8_t st = state();
    if ((st != 1) && move(&p1))
        use_fuel(&(p1.fuel),FUEL_STEP);
    if ((st != 2) && move(&p2))
        use_fuel(&(p2.fuel), FUEL_STEP);

    if (ms++ >= 10) {
        ms = 0;

        check_quit_pause(clear_hud, reset_game);

        update_player_sprite(p1, clear_pixels);
        update_player_sprite(p2, clear_pixels);

        uint8_t collision_land = handle_collision(st);

        if (st != 3) {
            if (st != 1)
                update_player(&p1);
            if (st != 2)
                update_player(&p2);
        }

        update_player_sprite(p1, set_pixels);
        update_player_sprite(p2, set_pixels);

        if ((collision_land) || (st == 3)) { // Both landed
            uint8_t winner;
            if ((winner = check_winner()) != 0) {
                switch (collision_land) {
                case 1: winner = 2; break;
                case 2: winner = 1; break;
                }
                post_game(winner);
            }
            reset_game();
        }

        show_hud();
        display_update();
        clear_hud();
    }
}
