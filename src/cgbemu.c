#include <string.h>

#include "cgbemu.h"

#include "cpu.h"
#include "screen.h"
#include "gameboy.h"
#include "memory.h"
#include "cartridge.h"


void setup( void )
{
	gameboy_power_on();
}

void get_next_frame( uint16_t frame[FRAME_HEIGHT][FRAME_WIDTH] )
{
    //fprintf(stderr, "get_next_frame\n");

	start_cpu();
	emulate_cpu();
	//dprintf("after emulate_cpu()");
	/* convert the current screen from RGB888 to RGB565 */
	
	//u8 (*sb)[SCREEN_WIDTH][3] = screen_buffer;
	
	/*
	dprintf("In get_next_frame()\n\tscreen_buffer=%X\n\tsb=%x", screen_buffer, sb);
	dprintf("\tsb[0][0][0]=%X", sb[0][0][0]);
	*/
	
	int x, y;
	//int sum = 0;
	for(x = 0; x < FRAME_HEIGHT; ++x) {
		for(y = 0; y < FRAME_WIDTH; ++y) {
			/*
			uint8_t r, g, b;
			r = screen_buffer[x][y][0];
			g = screen_buffer[x][y][1];
			b = screen_buffer[x][y][2];
			
			//sum += (r + g + b);
			
			uint16_t rgb = (r << 11) | (g << 5) | b;
			frame[x][y] = rgb;
			*/
			frame[x][y] = screen_buffer[x][y];
		}
	}
	//dprintf("sum=%u\n", sum);
}

void cgbemu_run_to_vblank()
{
	
}

void cgbemu_set_button_pressed(int button, bool pressed)
{
	/* TODO too many layers of abstraction here....*/
	gameboy_toggle_button((Button)button, pressed);
}

uint16_t* cgbemu_get_screen_buffer()
{
	return (uint16_t*)screen_buffer;
}

int cgbemu_load_cartridge(const uint8_t* buffer, int size)
{
    return load_cartridge(buffer, size);
}

void cgbemu_load_bios(uint8_t* buffer, int size)
{
    gameboy_load_bios(buffer, size);
}


void cgbemu_save_state(uint8_t** buffer, int* size) {
    (*size) = get_save_state_size();
    save_state(buffer);
}

void cgbemu_load_state(uint8_t* buffer, int size) {
    load_state(buffer, size);
}

 









