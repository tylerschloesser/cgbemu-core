#include <string.h>

#include "android_interface.h"

#include "cpu.h"
#include "screen.h"
#include "gameboy.h"
#include "memory.h"



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

void cgbemu_load_cartridge(uint8_t* buffer, int size)
{
	assert(cartridge_rom != NULL);
	assert(size > 0);
	
	if(size > CARTRIDGE_ROM_SIZE) {
		printf("invalid rom size\n");
		return;
	}
	
	memcpy(cartridge_rom, buffer, size);
	verify_cartridge();
}

void cgbemu_load_bios(uint8_t* buffer, int size)
{
	assert(cartridge_rom != NULL);
	assert(size > 0);
	
	if(size > BIOS_SIZE) {
		printf("invalid bios size\n");
		return;
	}
	
	memcpy(bios, buffer, size);
}












