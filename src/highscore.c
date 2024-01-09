/* game.c

   This file written 2019-09-24 by Edvard All

   For copyright and licensing, see file LICENSE
*/

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "marslander.h"

#define N_WIDTH 24
#define SC_WIDTH 12
#define NR_WIDTH 16
#define LIST_LEN 8
#define N_LEN 3
#define ALPHABET "abcdefghijklmnopqrstuvwxyz"
#define ALPHABET_LEN 25

struct hi_score {
    char name[4];
    uint8_t score;
} hi_arr[LIST_LEN];

/* Returns 1 if we issued a valid input, 0 otherwise */
uint8_t nav_alphabet(int *i, int *c)
{
    switch (get_btns()) {
    case 1:
        --*c;
        if (*c < 0)
            *c = ALPHABET_LEN;
        break;
    case 2:
        ++*c;
        if (*c > ALPHABET_LEN)
            *c = 0;
        break;
    case 4: 
        ++*i;
        sleep(ONE_SEC>>2);
        break;
    case 8:
        if (*i > 0)
            --*i;
        sleep(ONE_SEC>>2);
        break;
    default:
        return 0;
    }
    return 1;
}

void prompt_name(char *name)
{
    uint8_t text_pos = text_center(N_LEN);
    uint8_t ms = 0;
    int8_t blink_switch = 1;

    clear_screen();

    display_text("What's your", text_center(11), 0, set_pixels);
    display_text("name?", text_center(5), 1, set_pixels);
    display_text(name, text_pos, 3, set_pixels);

    int i, c, lock;
    i = c = lock = 0;
    uint8_t button;
    while (i < N_LEN)
        if ((IFS(0) & (1<<8))) {
            IFSCLR(0) = (1<<8);
            if (!lock && nav_alphabet(&i, &c)) {
                clear_page(3);
                name[i] = ALPHABET[c];
                display_text(name, text_pos, 3, set_pixels);
                lock = 1;
            }
            if ((ms % 35) == 0)
                lock = 0;
            if (++ms >= 50) {
                ms = 0;
                if (blink_switch++ & 1) {
                    display_text("_", (text_pos + i*8), 3, set_pixels);
                } else {
                    display_text("_", (text_pos + i*8), 3, clear_pixels);
                    display_text(&name[i], (text_pos + i*8), 3, set_pixels);
                }
            }
            display_update();
        }
}

void insert_item(const char s[4], uint8_t score, int i)
{
    int j = LIST_LEN;
    while (j-- > i)
        hi_arr[j+1] = hi_arr[j];
    for (j = 0; j < N_LEN; ++j)
        hi_arr[i].name[j] = s[j];
    hi_arr[i].score = score;
}

/*
  Returns index of the score if the score made the highscore list, -1 otherwise.
*/
uint8_t highscore(uint8_t score)
{
    int i;
    for (i = 0; i < LIST_LEN; ++i)
        if (score > hi_arr[i].score) {
            return i;
        }
    return -1;
}

void init_hi_scores()
{
    int i;
    for (i = 0; i < LIST_LEN; ++i) {
        hi_arr[i].score = 0;
    }
}

void display_list()
{
    uint8_t vert_line[1] = {0xff};
    uint8_t offset = 3;

    int i;
    /* Left hand side of list */
    for (i = 0; i < LIST_LEN >> 1; ++i) { 
        display_text(itoaconv(i+1), 0, i, set_pixels);
        display_text(".", offset, i, set_pixels);
        if (hi_arr[i].score) {
            display_text(hi_arr[i].name, NR_WIDTH, i, set_pixels);
            display_text(":", N_WIDTH+NR_WIDTH-3, i, set_pixels);
            display_text(itoaconv(hi_arr[i].score),
                         N_WIDTH+NR_WIDTH+offset, i, set_pixels);
        }
    }
    
    /* Draw a line in the center to separate the (columns) */
    int j;
    for (j = 0; j < 4; ++j)
        sprite_update(DISPLAY_COLS / 2, j*8, vert_line, 1, set_pixels);

    /* Right hand side of list */
    for (; i < LIST_LEN; ++i) { 
        uint8_t right = DISPLAY_COLS / 2 + offset;
        uint8_t page = i - (LIST_LEN>>1);
        display_text(itoaconv(i+1), right, page, set_pixels);
        display_text(".", (right + offset), page, set_pixels);
        if (hi_arr[i].score) {
            display_text(hi_arr[i].name, (right + NR_WIDTH), page, set_pixels);
            display_text(":", (right + N_WIDTH+NR_WIDTH-offset), page, set_pixels);
            display_text(itoaconv(hi_arr[i].score), (right + N_WIDTH+NR_WIDTH+offset),
                         page, set_pixels);
        }
    }
}
