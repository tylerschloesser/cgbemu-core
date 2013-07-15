#ifndef GAMEBOY_H_
#define GAMEBOY_H_

#include <stdbool.h>
#include "globals.h"

void gameboy_power_on();
void gameboy_power_off();

#define GAMEBOY_RAM_SIZE 0x8000
#define GAMEBOY_VRAM_SIZE 0x4000
/* TODO this is nNOT the correct OAM size */
#define GAMEBOY_OAM_SIZE 0x20000  //128KB
#define GAMEBOY_BIOS_SIZE 0x900

#define GAMEBOY_BG_PALLETE_SIZE 0x40
#define GAMEBOY_OB_PALLETE_SIZE 0x40

#define GAMEBOY_HRAM_SIZE 0x7F
#define GAMEBOY_HW_REGISTERS_SIZE 0x80



void gameboy_enable_bios(void);
void gameboy_disable_bios(void);

void gameboy_load_cartridge(char* cartridge_filepath);
void gameboy_load_bios(uint8_t* buffer, int size);

void gameboy_save_state( char* save_state_filepath );
void gameboy_load_state( char* save_state_filepath );

void gameboy_toggle_speed();

void gameboy_update_selected_ram();
void gameboy_update_selected_vram();


typedef struct {
    uint8_t* ram;
    uint8_t* vram;
    uint8_t* oam;
    uint8_t* bios;
    
    uint8_t* selected_vram;
    uint8_t* selected_ram;

    uint8_t* bg_pallete; /* background pallete */
    uint8_t* ob_pallete; /* object (sprite) pallete */

    uint8_t* hram;
    uint8_t* hw_registers; /* hardware registers (I/O ports) */
    
    u8 ie_register; /* interrupt enable register */
    u8 ime_flag; /* interrupt master enable flag */

    bool cgb_mode;
    bool use_bios;
    
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

enum hw_register {
    P1      = 0x00, //read joypad info
    SB      = 0x01, //serial transfer data
    SC      = 0x02, //serial i/o control

    DIV     = 0x04, //timer divider
    TIMA    = 0x05, //timer counter
    TMA     = 0x06, //timer modulo
    TAC     = 0x07, //timer control
    IF      = 0x0F, //interrupt flag
    NR10    = 0x10, //audio sweep
    NR11    = 0x11, //audio channel #1
    NR12    = 0x12, //envelope channel #1
    NR13    = 0x13, //sound frequency #1
    NR14    = 0x14, //sound frequency #1

    NR21    = 0x16, //audio channel #2
    NR22    = 0x17, //envelope channel #2
    NR23    = 0x18, //sound frequency #2
    NR24    = 0x19, //sound frequency #2

    NR30    = 0x1A, //audio channel #3
    NR31    = 0x1B, //sound length #3
    NR32    = 0x1C, //volume #3
    NR33    = 0x1D, //sound frequency #3
    NR34    = 0x1E, //sound frequency #3

    NR41    = 0x20, //sound length #4
    NR42    = 0x21, //envelope channel #4
    NR43    = 0x22, //audio counter
    NR44    = 0x23, //audio counter

    NR50    = 0x24, //channel control
    NR51    = 0x25, //sound output
    NR52    = 0x26, //sound on/off
    //AUD3WAVERAM FF30- FF3F
    LCDC    = 0x40, //lcd control
    STAT    = 0x41, //lcd status
    SCY     = 0x42, //scroll screen y
    SCX     = 0x43, //scroll screen x
    LY      = 0x44, //lcdc y-coord
    LYC     = 0x45, //ly compare
    DMA     = 0x46, //dma transfer
    BGP     = 0x47, //bg pallette data

    OBP0    = 0x48, //object palette 0 data
    OBP1    = 0x49, //object palette 1 data
    WY      = 0x4A, //window y position
    WX      = 0x4B, //window x position
    KEY1    = 0x4D, //cpu speed select
    VBK     = 0x4F, //vram bank select

    BLCK    = 0x50, //enable/disable bios
    HDMA1   = 0x51, //hbl general dma
    HDMA2   = 0x52, //
    HDMA3   = 0x53, // "     "     "
    HDMA4   = 0x54, //
    HDMA5   = 0x55, // "     "     "

    RP      = 0x56, //infared comms
    BCPS    = 0x68, //bg colour index
    BCPD    = 0x69, //bg colour data
    OCPS    = 0x6A, //object colour index
    OCPD    = 0x6B, //object colour data

    SVBK    = 0x70, //ram bank select
    IE      = 0xFF  //interrupt enable
};


#endif // GAMEBOY_H_
