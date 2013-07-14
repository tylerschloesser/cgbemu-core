#ifndef GAMEBOY_H_
#define GAMEBOY_H_

#include <stdbool.h>
#include "globals.h"

void gameboy_power_on();
void gameboy_power_off();

#define GAMEBOY_BG_PALLETE_SIZE 0x40
#define GAMEBOY_OB_PALLETE_SIZE 0x40

#define GAMEBOY_HRAM_SIZE 0x7F
#define GAMEBOY_HW_REGISTERS_SIZE 0x80



void gameboy_load_cartridge(char* cartridge_filepath);
void gameboy_load_bios(char *bios_filepath);

void gameboy_save_state( char* save_state_filepath );
void gameboy_load_state( char* save_state_filepath );

void gameboy_toggle_speed();

void gameboy_update_selected_ram_bank();
void gameboy_update_selected_vram_bank();


typedef struct {
    uint8_t* ram;
    uint8_t* vram;
    uint8_t* oam;
    uint8_t* bios;
    
    uint8_t* selected_vram;
    uint8_t* selected_ram;

    uint8_t* pallete;
    uint8_t* bg_pallete; /* background pallete */
    uint8_t* ob_pallete; /* object (sprite) pallete */

    uint8_t* hram;
    uint8_t* hw_registers; /* hardware registers (I/O ports) */
    
    u8 ie_register; /* interrupt enable register */
    u8 ime_flag; /* interrupt master enable flag */

    u8 cgb_mode;
    
} GameboyColor;

extern GameboyColor* gb;



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

//TODO
int get_save_state_size();

typedef enum {
    CGB, /* gameboy color */
    MGB, /* gameboy pocket */
    DMG, /* gameboy */
} GameboyModel;


#endif // GAMEBOY_H_
