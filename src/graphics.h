#ifndef GRAPHICS_H_INCLUDED
#define GRAPHICS_H_INCLUDED

#ifdef _WIN32
#include <windows.h>
#endif

#include <fcntl.h>
#include <errno.h>

#include "globals.h"
/*
#include <SDL.h>
#include <SDL_thread.h>
*/
#include "memory.h"

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144

void print_map();

/*
extern SDL_Surface *surface;
*/

int render_screen();

void initialize_graphics();
void reinitialize_graphics();

//temp
u8* get_background_map(u16 vram_address, bool signed_tiles);

#endif //GRAPHICS_H_INCLUDED
