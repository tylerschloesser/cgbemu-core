#ifndef SCREEN_H_
#define SCREEN_H_

#include "graphics.h"

/*
//temp
#include "../gui/gtksdl.h"
*/

//extern u8 screen_buffer[SCREEN_HEIGHT][SCREEN_WIDTH][3];
extern uint16_t screen_buffer[SCREEN_HEIGHT][SCREEN_WIDTH];

/*
extern SDL_Surface *surface;
*/

void update_screen();

void initialize_screen();
void reinitialize_screen();

void update_screen_buffer( int x_pixel, int y_pixel, u16 pallete_entry );

#endif // SCREEN_H_