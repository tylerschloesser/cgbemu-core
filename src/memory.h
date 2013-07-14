#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED

#include "globals.h"
#include "debug.h"
#include "gameboy.h"
//temp
#include "cpu.h"

#define GAMEBOY_RAM_SIZE 0x8000
#define GAMEBOY_VRAM_SIZE 0x4000
#define GAMEBOY_OAM_SIZE 0x20000  //128KB

#define GAMEBOY_BIOS_SIZE 0x900

extern int cartridge_ram_size;
extern int cartridge_rom_size;

extern uint8_t (*read_memory)(uint16_t);
extern void (*write_memory)(uint16_t, uint8_t);

void update_all_selected_banks();
void update_selected_cartridge_banks();

void hdma_transfer(void);

//temp
extern uint8_t* selected_gameboy_vram_bank;

//TODO TEMP
bool in_bios;

enum MBC_control {
    RAM_ENABLE,
    ROM_BANK_LOW,
    ROM_BANK_HIGH,
    RAM_BANK
};


void mbc0_write(uint16_t location, uint8_t data);
uint8_t mbc0_read(uint16_t location);
void mbc1_write(uint16_t location, uint8_t data);
uint8_t mbc1_read(uint16_t location);
void mbc2_write(uint16_t location, uint8_t data);
uint8_t mbc2_read(uint16_t location);
void mbc3_write(uint16_t location, uint8_t data);
uint8_t mbc3_read(uint16_t location);
void mbc4_write(uint16_t location, uint8_t data);
uint8_t mbc4_read(uint16_t location);
void mbc5_write(uint16_t location, uint8_t data);
uint8_t mbc5_read(uint16_t location);


enum hardware_register {
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


extern uint8_t* cartridge_ram;
extern uint8_t* cartridge_rom;

extern u8 pallete[0x40]; //8 palletes - 4 colors per pallete - 2 bytes per color
extern u8 sprite_pallete[0x40];

extern uint8_t* gameboy_ram;
extern uint8_t* gameboy_vram;
extern uint8_t* gameboy_oam;
extern uint8_t* gameboy_bios;

//extern u8 bios[BIOS_SIZE];


extern u8 zero_page[0x7F];             //127B
extern u8 interrupt_enable;
extern u8 hardware_registers[0x80];    //128B

extern u8 IME;							//Interrupt Master Enable Flag (used by EI, DI, RETI, <INT>)
	
extern u8 mbc_control[4];

enum lcd_control {
	LCD_DISPLAY_ENABLE = 0x80,
	WINDOW_TILE_MAP_DISPLAY_SELECT = 0x40,
	WINDOW_DISPLAY_ENABLE = 0x20,
	BG_WINDOW_TILE_DATA_SELECT = 0x10,
	BG_TILE_MAP_DISPLAY_SELECT = 0x08,
	OBJ_SIZE = 0x04,
	OBJ_DISPLAY_ENABLE = 0x02,
	BG_DISPLAY = 0x01
};

enum bg_map_attributes {
	BACKGROUND_PALETTE_NUMBER = 0x07,
	TILE_VRAM_BANK_NUMBER = 0x08,
	HORIZONTAL_FLIP = 0x20,
	VERTICAL_FLIP = 0x40,
	BG_TO_OAM_PRIORITY = 0x80
};


void initialize_memory();
void reinitialize_memory();

#endif // MEMORY_H_INCLUDED
