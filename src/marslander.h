/* marslander.h

   This file written by 2019-10-14 by D Ljunggren
   Last edited by E All 2019-12-09

   For copyright and licensing, see file LICENSE
*/

/* Won't compile without this :( */
void *stdin, *stdout, *stderr;

/* General game options, think they should be here as long as we're not
   planning on making these configurable settings in the game */
#define DISPLAY_COLS 128
#define SCALE 100 // one pixel corresponds to 100 "units"
#define CRASH_TRESHOLD 60
#define PLATFORM_SIZE 6
#define SHIP_SPRITE {0b1100,0b111,0b111,0b1100}
#define SHIP_SIZE 4
/*
   Clearly a spaceship:
   0110
   0110
   1111
   1001
*/

/* Declare array containing ground */
#define GROUND_LEN 512
#define GROUND_OFFSET 255
uint8_t ground_arr[GROUND_LEN]; // we create more ground than there is display

#define ONE_SEC 1<<22

// Handy delay function
void sleep(int cyc);

/* Declarations for player.c */
struct player
{
    const uint8_t button_set, sprite[4];
    uint8_t score;
    int16_t x_speed, y_speed;
    int32_t x_pos, y_pos;
    float fuel;
};
void update_player(struct player *p);
void update_player_sprite(struct player p, void (*pixels_func)(int col, uint8_t bval));
void use_fuel(float* fuel, const float step);
void reset_player(struct player *p, const float fuel_max,
                  const int32_t x, const int32_t y, const int8_t dir);
void update_speed(int16_t* speed, const int8_t thrust);
uint8_t move(struct player *p);

/* Declarations for game.c */
uint8_t ms;
uint8_t game_state;
uint8_t landed(struct player p);
uint8_t game_over(struct player p);

/* Declarations for modes */
void sp_mode(void);
void mp_mode(void);
void menu_mode(void);

/* Declarations for display.c */
void display_init(void);
void clear_leds(void);
void clear_leds(void);
void sprite_update(const uint8_t x, const uint8_t y, const uint8_t spr[], const int len,
                   void (*pixels_func)(int col, uint8_t bval));
void update_fuel_leds(float fuel, uint16_t max);
void display_text(const char s[], uint8_t x, const uint8_t page,
                  void (*pixels_func)(int col, uint8_t bval));
void display_update(void);
void set_pixels(int col, uint8_t bval);
void clear_pixels(int col, uint8_t bval);
void clear_screen();
char* itoaconv(int num);

/* Declare bitmap array containing font */
extern const uint8_t const font[128*8];
extern const uint8_t const happy_smiley[8];

/* Declarations for input.c */
void input_init(void);
int getbtns(void);

/* Declarations of highscore.c */
void init_hi_scores();
void display_list();
