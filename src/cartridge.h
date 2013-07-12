#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include "globals.h"

int load_cartridge(const uint8_t* buffer, int size);
int verify_cartridge();

typedef enum {
    NONE,
    MBC1,
    MBC2,
} MemoryBankController;



typedef struct {
    MemoryBankController mbc;

    uint8_t* rom;
    uint8_t* ram;
    int ram_size;
    int rom_size;

    uint8_t* selected_rom;
    uint8_t* selected_ram;

    int selected_rom_bank;
    int selected_ram_bank;

    uint8_t (*read)(uint16_t);
    void (*write)(uint16_t, uint8_t);

    bool ram_enabled;
    uint8_t banking_mode;

} Cartridge;

extern Cartridge* cartridge;

#endif /* CARTRIDGE_H */
