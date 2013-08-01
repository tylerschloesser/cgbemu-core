#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED

#include "globals.h"
#include "gameboy.h"
#include "cartridge.h"
#include "cpu.h"

uint8_t memory_read(uint16_t);
void memory_write(uint16_t, uint8_t);

void hdma_transfer(void);

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

#endif // MEMORY_H_INCLUDED
