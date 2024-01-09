/* display.c
   This file written 2015 by F Lundevall
   Some parts are original code written by Axel Isaksson
   Latest update 2019-12-09 by E All

   For copyright and licensing, see file LICENSE
*/

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "marslander.h"

#define DISPLAY_CHANGE_TO_COMMAND_MODE (PORTFCLR = 0x10)
#define DISPLAY_CHANGE_TO_DATA_MODE (PORTFSET = 0x10)

#define DISPLAY_ACTIVATE_RESET (PORTGCLR = 0x200)
#define DISPLAY_DO_NOT_RESET (PORTGSET = 0x200)

#define DISPLAY_ACTIVATE_VDD (PORTFCLR = 0x40)
#define DISPLAY_ACTIVATE_VBAT (PORTFCLR = 0x20)

#define DISPLAY_TURN_OFF_VDD (PORTFSET = 0x40)
#define DISPLAY_TURN_OFF_VBAT (PORTFSET = 0x20)

#define DISPLAY_PAGES 4
#define DISPLAY_BUFFER_SIZE DISPLAY_COLS * DISPLAY_PAGES
uint8_t display[DISPLAY_BUFFER_SIZE];

/* Declare array containing ground */
#define GROUND_LEN 512
#define GROUND_OFFSET 255
uint8_t ground_arr[GROUND_LEN]; // we create more ground than there is display
const uint8_t GROUND_SPRITE[1] = {75}; // 01001011

uint8_t spi_send_recv(uint8_t data)
{
    while(!(SPI2STAT & 0x08));
    SPI2BUF = data;
    while(!(SPI2STAT & 1));
    return SPI2BUF;
}

void display_init(void)
{
    DISPLAY_CHANGE_TO_COMMAND_MODE;
    sleep(10);
    DISPLAY_ACTIVATE_VDD;
    sleep(1000000);

    spi_send_recv(0xAE);
    DISPLAY_ACTIVATE_RESET;
    sleep(10);
    DISPLAY_DO_NOT_RESET;
    sleep(10);

    spi_send_recv(0x8D);
    spi_send_recv(0x14);

    spi_send_recv(0xD9);
    spi_send_recv(0xF1);

    DISPLAY_ACTIVATE_VBAT;
    sleep(10000000);

    spi_send_recv(0xA1);
    spi_send_recv(0xC8);

    spi_send_recv(0xDA);
    spi_send_recv(0x20);

    spi_send_recv(0xAF);
}

void led_init()
{
    TRISECLR = 0xff;
}

void set_leds()
{
    PORTESET = 0xff;
}

void set_pixels(const int col, const uint8_t bval)
{
    display[col] |= bval;
}

/* All bits set to 1 in bval are set to zero in the display buffer */
void clear_pixels(const int col, const uint8_t bval)
{
    display[col] &= ~bval;
}

int check_page(const uint8_t y)
{
    int page;
    if (y < 8)      // 8 Because a "page" consists of 8 pixels. Y because we use cartesian coordinates for the sprites position.
        page = 0;
    else if (y < 16)
        page = 1;
    else if (y < 24)
        page = 2;
    else
        page = 3;
    return page;
}

void clear_page(uint8_t page)
{
    int i;
    for (i = 0; i < DISPLAY_COLS; ++i)
        display[i+page*DISPLAY_COLS] = 0;
}

void clear_screen()
{
    int i;
    for (i = 0; i < DISPLAY_BUFFER_SIZE; ++i)
        display[i] = 0;
}

/*
  sprite_update manipulates the bits of the sprite byte arrray to address the
  situation where a sprite is between two pages, and then modifies the bits
  in the display buffer in that sprite according to the given pixels_func, x,
  and y-coordinates.

  TLDR this function lets you regard the display as a cartesian coordinate system
*/
void sprite_update(const uint8_t x, const uint8_t y, const uint8_t spr[], const int len,
                   void (*pixels_func)(int col, uint8_t bval))
{
    int page = check_page(y);

    uint8_t upper_cut, lower_cut;
    int i;
    for (i = 0; i < len; ++i) {
        upper_cut = spr[i] << (y & 7);
        /* The lower cut has to be shifted down as it's there it's */
        /* supposed to be written on the next page, so eg abcdefgh with y_pos */
        /*  of 5 becomes xxxxxabc[NEXTPAGE]dgefhxx */
        lower_cut = spr[i] >> (8 - (y & 7));
        pixels_func(x + i + DISPLAY_COLS*page, upper_cut);
        pixels_func(x + i + DISPLAY_COLS*(page + 1), lower_cut);
    }
}

/*
  Just sends the display buffer to the display.
*/
void display_update()
{
    int i, j, k;
    int c;
    for (i = 0; i < DISPLAY_PAGES; ++i) {
        DISPLAY_CHANGE_TO_COMMAND_MODE;

        spi_send_recv(0x22); // set page cmd
        spi_send_recv(i); // sets page nr
        spi_send_recv(0); // set low nbl of col
        spi_send_recv(0x10); // set hi nbl of col

        DISPLAY_CHANGE_TO_DATA_MODE;

        for (j = 0; j < DISPLAY_COLS; ++j)
            spi_send_recv(display[i * DISPLAY_COLS + j]);
    }
}

/*
  __NOT__ WRITTEN BY EDVARD ALL OR DAVID LJUNGGREN!
  CREDITS TO: F Lundevall and A Eriksson
  (excessive comment removed as we don't care how it works, also
  changed the style according to K&R)

  itoa

  Simple conversion routine
  Converts binary to decimal numbers
  Returns pointer to (static) char array
*/
#define ITOA_BUFSIZ ( 24 )
char* itoaconv(int num)
{
    register int i, sign;
    static char itoa_buffer[ ITOA_BUFSIZ ];
    static const char maxneg[] = "-2147483648";

    itoa_buffer[ ITOA_BUFSIZ - 1 ] = 0;   /* Insert the end-of-string marker. */
    sign = num;                           /* Save sign. */
    if (num < 0 && num - 1 > 0) {         /* Check for most negative integer */
        for( i = 0; i < sizeof( maxneg ); i += 1 )
            itoa_buffer[ i + 1 ] = maxneg[ i ];
        i = 0;
    }
    else {
        if( num < 0 ) num = -num;           /* Make number positive. */
        i = ITOA_BUFSIZ - 2;                /* Location for first ASCII digit. */
        do {
            itoa_buffer[ i ] = num % 10 + '0';/* Insert next digit. */
            num = num / 10;                   /* Remove digit from number. */
            i -= 1;                           /* Move index to next empty position. */
        } while( num > 0 )
            ;
        if( sign < 0 ) {
            itoa_buffer[ i ] = '-';
            i -= 1;
        }
    }
    /* Since the loop always sets the index i to the next empty position,
     * we must add 1 in order to return a pointer to the first occupied position. */
    return( &itoa_buffer[ i + 1 ] );
}

void clear_leds()
{
    PORTECLR = 0xff;
}
/*
  Max amount through 8 is what every LED is worth, so we subtract that from
  the current fuel to see how much fuel is left, and then we clear the LED PORT
  for that value.
*/
void update_fuel_leds(float fuel, const uint16_t max)
{
    uint8_t led = max >> 3;
    int leds_left = 0;
    while (fuel > led) {
        ++leds_left;
        fuel -= led;
    }
    PORTECLR = (1 << leds_left);
}

void display_ground()
{
    int x;
    for (x = 0; x < DISPLAY_COLS; ++x)
        sprite_update(x, ground_arr[x + GROUND_OFFSET], GROUND_SPRITE, 1, set_pixels);
}

uint8_t text_center(const uint8_t text_width)
{
    return (DISPLAY_COLS - text_width*8) / 2;
}

/* Basically just a more simple sprite_update concentrated on writing text */
void display_text(const char s[], uint8_t x, const uint8_t page,
                  void (*pixels_func)(int col, uint8_t bval))
{
    int i;
    char c;
    while ((c = *(s++)) != '\0') {
        for (i = 0; i < 8; ++i)
            pixels_func(x + i + DISPLAY_COLS*page, font[c * 8 + i]);
        x += 8;
    }
}
