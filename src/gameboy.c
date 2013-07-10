#include "gameboy.h"
#include "cpu.h"
#include "joypad.h"
#include "memory.h"
#include "graphics.h"

/*
#include "../tools.h"

#include <SDL.h>
#include <SDL_thread.h>
*/


/*
SDL_Surface *surface;
*/

/*
static void key_pressed(SDLKey key);
static void key_unpressed(SDLKey key);
*/

void gameboy_toggle_speed() {
    fullspeed = !fullspeed;
}

//TODO deprecated
/*
void gameboy_load_cartridge(char* cartridge_filepath) {

	printf("Opening %s...\n", cartridge_filepath);
    
	int bytes_read = binary_read_file(cartridge_filepath, cartridge_rom, CARTRIDGE_ROM_SIZE);
	if(bytes_read == 0) {
		fprintf(stderr, "gameboy_load_cartridge() failed\n");
		fatal_error();
	}
	
	verify_cartridge();
	
	//printf("Done (%s)\n", size_to_string(bytes_read));
}
*/

void gameboy_load_bios(char *bios_filepath) {
	printf("Opening %s...\n", bios_filepath);
    int bytes_read = binary_read_file(bios_filepath, bios, BIOS_SIZE);
	if(bytes_read == 0) {
		fprintf(stderr, "gameboy_load_bios() failed\n");
		fatal_error();
	}
    //printf("Done (%s)\n", size_to_string(bytes_read));
}


void gameboy_power_on() 
{

	initialize_cpu();
	initialize_memory();
	initialize_joypad();
	initialize_screen();
	initialize_graphics();
	
	/* ANDROID TESTING */
	return;
	
#ifdef _WIN32
	char game_filepath[] = "resources\\roms\\pokemon_yellow.gbc";
    //char game_filepath[] = "resources\\roms\\cpu_instrs.gb";
    
	//char game_filepath[] = "resources\\roms\\pokemon_red.gb";
	char bios_filepath[] = "resources\\bios\\gbc_bios.bin";
#else

	//char game_filepath[] = "resources/roms/pokemon_yellow.gbc";
    char game_filepath[] = "resources/roms/cpu_instrs.gb";
	//char game_filepath[] = "resources\\roms\\pokemon_red.gb";
	char bios_filepath[] = "resources/bios/gbc_bios.bin";
#endif
	
    gameboy_load_bios(bios_filepath);
    gameboy_load_cartridge(game_filepath);
	
    fprintf( stderr, "starting cpu\n" );
    
	start_cpu();
}

void gameboy_power_off()
{
    printf("gameboy_power_off()\n");
	stop_cpu();
}

//temp for debugging
void cpu_debug() {
	//display_cpu_values();
	//show_opcodes = true;
}

void gameboy_toggle_button( Button button, bool pressed )
{

	assert( button != INVALID );

	// temporary to allow for the new interface
	int gb_key = -1;
	switch( button )
	{
		case UP:
			gb_key = JOYPAD_UP;
			break;
		case DOWN:
			gb_key = JOYPAD_DOWN;
			break;
		case LEFT:
			gb_key = JOYPAD_LEFT;
			break;
		case RIGHT:
			gb_key = JOYPAD_RIGHT;
			break;
		case START:
			gb_key = JOYPAD_START;
			break;
		case SELECT:
			gb_key = JOYPAD_SELECT;
			break;
		case A:
			gb_key = JOYPAD_A;
			break;
		case B:
			gb_key = JOYPAD_B;
			break;
	}
	
	if( gb_key != -1 )
	{
		if( pressed == true ) {
			joypad_down( gb_key );
		} else {
			joypad_up( gb_key );
		}
	}
}
//TODO deprecated
/*
void gameboy_save_state( char* save_state_filepath )
{
	FILE* file = fopen( save_state_filepath, "wb+" );
	
	if( file == NULL ) {
		fprintf( stderr, "unable to open file for save state: %s\n", save_state_filepath );
		return;
	}
	
	pause_cpu();
	
	CpuState state = get_cpu_state();
	int state_size = sizeof( CpuState );
	
	fprintf(stdout,"Saving state...\n");
	//cpu_print_state();
	
	u8* state_raw = (u8*)&state;
	
	fwrite( state_raw, sizeof( state_raw[0] ), state_size, file );
	fwrite( cartridge_ram, sizeof( cartridge_ram[0] ), CARTRIDGE_RAM_SIZE, file );
	fwrite( cartridge_rom, sizeof( cartridge_rom[0] ), CARTRIDGE_ROM_SIZE, file );
	fwrite( pallete, sizeof( pallete[0] ), 0x40, file );
	fwrite( sprite_pallete, sizeof( sprite_pallete[0] ), 0x40, file );
	fwrite( gameboy_ram, sizeof( gameboy_ram[0] ), GAMEBOY_RAM_SIZE, file );
	fwrite( gameboy_vram, sizeof( gameboy_vram[0] ), GAMEBOY_VRAM_SIZE, file );
	fwrite( gameboy_oam, sizeof( gameboy_oam[0] ), GAMEBOY_OAM_SIZE, file );
	fwrite( bios, sizeof( bios[0] ), BIOS_SIZE, file );
	fwrite( zero_page, sizeof( zero_page[0] ), 0x7F, file );
	fwrite( &interrupt_enable, sizeof( interrupt_enable ), 1, file );
	fwrite( hardware_registers, sizeof( hardware_registers[0] ), 0x80, file );
	fwrite( &IME, sizeof( IME ), 1, file );
	fwrite( mbc_control, sizeof( mbc_control[0] ), 4, file );
	
	fclose( file );
	
	start_cpu();
}

void gameboy_load_state( char* save_state_filepath )
{
	FILE* file = fopen( save_state_filepath, "rb" );
	
	if( file == NULL ) {
		fprintf( stderr, "unable to open file for save state: %s\n", save_state_filepath );
		return;
	}
	
	pause_cpu();
	
	reinitialize_cpu();
	reinitialize_memory();
	reinitialize_joypad();
	reinitialize_screen();
	reinitialize_graphics();
	
	CpuState state;
	int state_size = sizeof( CpuState );
	u8* state_raw = (u8*)&state;
	
	fread( state_raw, sizeof( state_raw[0] ), state_size, file );
	fread( cartridge_ram, sizeof( cartridge_ram[0] ), CARTRIDGE_RAM_SIZE, file );
	fread( cartridge_rom, sizeof( cartridge_rom[0] ), CARTRIDGE_ROM_SIZE, file );
	fread( pallete, sizeof( pallete[0] ), 0x40, file );
	fread( sprite_pallete, sizeof( sprite_pallete[0] ), 0x40, file );
	fread( gameboy_ram, sizeof( gameboy_ram[0] ), GAMEBOY_RAM_SIZE, file );
	fread( gameboy_vram, sizeof( gameboy_vram[0] ), GAMEBOY_VRAM_SIZE, file );
	fread( gameboy_oam, sizeof( gameboy_oam[0] ), GAMEBOY_OAM_SIZE, file );
	fread( bios, sizeof( bios[0] ), BIOS_SIZE, file );
	fread( zero_page, sizeof( zero_page[0] ), 0x7F, file );
	fread( &interrupt_enable, sizeof( interrupt_enable ), 1, file );
	fread( hardware_registers, sizeof( hardware_registers[0] ), 0x80, file );
	fread( &IME, sizeof( IME ), 1, file );
	fread( mbc_control, sizeof( mbc_control[0] ), 4, file );
	
	fclose( file );
	
	set_cpu_state( state );
	
	fprintf(stdout,"Loading state...\n");
	//cpu_print_state();
	
	start_cpu();
}
*/
