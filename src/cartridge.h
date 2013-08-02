#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include "globals.h"

typedef enum {
    NONE,
    MBC1,
    MBC2,
    MBC3,
    MBC4,
    MBC5,
    UNSUPPORTED 
} MemoryBankController;

typedef enum {
    ID_ROM_ONLY,
    ID_MBC1,
    ID_MBC1_RAM,
    ID_MBC1_RAM_BATTERY,
    ID_MBC2,
    ID_MBC2_BATTERY,
    ID_ROM_RAM,
    ID_ROM_RAM_BATTERY,
    ID_MMM01,
    ID_MMM01_RAM,
    ID_MMM01_RAM_BATTERY,
    ID_MBC3_TIMER_BATTERY,
    ID_MBC3_TIMER_RAM_BATTERY,
    ID_MBC3,
    ID_MBC3_RAM,
    ID_MBC3_RAM_BATTERY,
    ID_MBC4,
    ID_MBC4_RAM,
    ID_MBC4_RAM_BATTERY,
    ID_MBC5,
    ID_MBC5_RAM,
    ID_MBC5_RAM_BATTERY,
    ID_MBC5_RUMBLE,
    ID_MBC5_RUMBLE_RAM,
    ID_MBC5_RUMBLE_RAM_BATTERY,
    ID_POCKET_CAMERA,
    ID_BANDAI_TAMA5,
    ID_HUC3,
    ID_HUC1_RAM_BATTERY
} CartridgeId;

typedef struct {
    CartridgeId id;
    char name[32];
    MemoryBankController mbc;

    uint8_t* rom;
    uint8_t* ram;
    uint32_t  ram_size;
    uint32_t rom_size;

    uint8_t* selected_rom;
    uint8_t* selected_ram;

    uint16_t selected_rom_bank;
    uint16_t selected_ram_bank;

    uint8_t (*read)(uint16_t);
    void (*write)(uint16_t, uint8_t);

    bool ram_enabled;
    uint8_t banking_mode;

    bool has_battery;
    uint8_t rtc_register;

} Cartridge;

extern Cartridge* cartridge;

int initialize_cartridge(const char* cartridge_filepath);
void cartridge_update_mbc(void);
void cartridge_update_selected_rom(void);
void cartridge_update_selected_ram(void);

#endif // CARTRIDGE_H
