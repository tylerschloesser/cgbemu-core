#ifndef GAMEBOY_H_
#define GAMEBOY_H_

#include <stdbool.h>

void gameboy_power_on();
void gameboy_power_off();

void gameboy_load_cartridge(char* cartridge_filepath);
void gameboy_load_bios(char *bios_filepath);

void gameboy_save_state( char* save_state_filepath );
void gameboy_load_state( char* save_state_filepath );

void gameboy_toggle_speed();

bool verify_cartridge(); //not finished

typedef enum {
	UP,
	DOWN,
	RIGHT,
	LEFT,
	A,
	B,
	START,
	SELECT,

	INVALID,
} Button;

void gameboy_toggle_button( Button button, bool pressed );

#endif // GAMEBOY_H_