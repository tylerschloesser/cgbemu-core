#define FRAME_WIDTH 160
#define FRAME_HEIGHT 144

#include "globals.h"

enum {
    BUTTON_UP = 0,
    BUTTON_DOWN,
    BUTTON_RIGHT,
    BUTTON_LEFT,

    BUTTON_A,
    BUTTON_B,
    BUTTON_SELECT,
    BUTTON_START
};

void setup( void );
void get_next_frame( uint16_t frame[FRAME_HEIGHT][FRAME_WIDTH] );

void cgbemu_set_button_pressed(int button, bool pressed);

/* placeholder */
void cgbemu_run_to_vblank();

uint16_t* cgbemu_get_screen_buffer();

int cgbemu_load_cartridge(const uint8_t* buffer, int size);
void cgbemu_load_bios(uint8_t* buffer, int size);
