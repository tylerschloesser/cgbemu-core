#ifndef SCREEN_H_
#define SCREEN_H_

#include "globals.h"

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144

extern uint16_t screen_buffer[SCREEN_HEIGHT][SCREEN_WIDTH];
void update_screen();

void initialize_screen();
void reinitialize_screen();

void update_screen_buffer( int x_pixel, int y_pixel, u16 pallete_entry );
int render_screen();
int render_scanline();

void update_lcd_control_register(uint8_t);


#endif // SCREEN_H_
