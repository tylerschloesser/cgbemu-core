#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include "globals.h"


int initialize_cartridge(const char* cartridge_filepath);

typedef enum {
    NONE,
    MBC1,
    MBC2,
    MBC3,
    MBC4,
    MBC5,
    UNSUPPORTED 
} MemoryBankController;

void cartridge_update_selected_rom();
void cartridge_update_selected_ram();

typedef struct {
    MemoryBankController mbc;

    uint8_t* rom;
    uint8_t* ram;
    int ram_size;
    int rom_size;

    uint8_t* selected_rom;
    uint8_t* selected_ram;

    u16 selected_rom_bank;
    u16 selected_ram_bank;

    uint8_t (*read)(uint16_t);
    void (*write)(uint16_t, uint8_t);

    bool ram_enabled;
    uint8_t banking_mode;

    bool has_battery;
    uint8_t rtc_register;

} Cartridge;

extern Cartridge* cartridge;

#endif /* CARTRIDGE_H */
