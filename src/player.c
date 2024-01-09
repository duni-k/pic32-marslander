/* player.c

   This file written 2019-09-24 by Edvard All

   For copyright and licensing, see file LICENSE
*/

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "marslander.h"

#define GRAVITY 3
#define FRICTION 1
#define THRUST 1

/* Returns 1 if movement was made */
uint8_t move(struct player *p)
{
    uint8_t button = (get_btns() >> p->button_set) & 3;

    if (button && p->fuel >= 0) {
        switch (button) {
        case 1:
            update_speed(&(p->x_speed), -THRUST);
            break;
        case 2:
            update_speed(&(p->x_speed), THRUST);
            break;
        case 3:
            update_speed(&(p->y_speed), -THRUST);
            break;
        }
        return 1;
    }
    return 0;
}

void atmosphere_friction(int16_t* speed)
{
    if (*speed > 0) {
        *speed -= FRICTION;
    } else if (*speed < 0)
        *speed += FRICTION;
}

void update_speed(int16_t* speed, const int8_t thrust)
{
    *speed += (int16_t) thrust;
}

void update_position(int32_t *pos, const int16_t speed)
{
    *pos += (int32_t) speed;
}

void reset_player(struct player *p, const float fuel_max,
                  const int32_t x, const int32_t y, const int8_t dir)
{
    p->y_pos = y;
    p->y_speed = 0;
    p->x_pos = x;
    p->x_speed = dir*SCALE;
    p->fuel = fuel_max;
}

void use_fuel(float *fuel, float step)
{
    *fuel -= step;
}

void update_player(struct player *p)
{
    update_speed(&(p->y_speed), GRAVITY);
    atmosphere_friction(&(p->x_speed));
    atmosphere_friction(&(p->y_speed));
    update_position(&(p->x_pos), p->x_speed);
    update_position(&(p->y_pos), p->y_speed);
}

void update_player_sprite(struct player p, void (*pixels_func)(int col, uint8_t bval))
{
    int16_t x = p.x_pos / SCALE;
    int16_t y = p.y_pos / SCALE;

    /* Don't draw outside screen */
    if ((x <= -SHIP_SIZE) || (x + SHIP_SIZE >= DISPLAY_COLS) || (y <= -SHIP_SIZE))
        return;
    
    if ((x >= 0) && (x < (DISPLAY_COLS - SHIP_SIZE)) && (y >= 0)) {
        sprite_update(x, y, p.sprite, SHIP_SIZE, pixels_func);
    } else { // handle offscreen edgecases
        uint8_t y_edge = 0;
        if ((y < 0) && (y > -SHIP_SIZE))
            y_edge = -y;

        uint8_t temp_spr[SHIP_SIZE];
        int i;
        for (i = 0; i < SHIP_SIZE; ++i)
            temp_spr[i] = p.sprite[i] >> y_edge;

        uint8_t spr_len;
        if ((x < 0) && (x > -SHIP_SIZE)) {
            spr_len = SHIP_SIZE + x;
            uint8_t* fragment = temp_spr - x;
            sprite_update(0, y, fragment, spr_len, pixels_func);
        } else if ((x >= (DISPLAY_COLS - SHIP_SIZE)) && (x < DISPLAY_COLS)) {
            spr_len = SHIP_SIZE - (DISPLAY_COLS - x);
            sprite_update(x, y, temp_spr, spr_len, pixels_func);
        } else
            sprite_update(x, 0, temp_spr, SHIP_SIZE, pixels_func);
    }
}
