
#include "globals.h"

#include "memory.h"
#include <string.h>

enum {
    ROM_ONLY,
    MBC1,
    MBC1_RAM,
    MBC1_RAM_BATTERY,
    MBC2,
    MBC2_BATTERY,
    ROM_RAM,
    ROM_RAM_BATTERY,
    MMM01,
    MMM01_RAM,
    MMM01_RAM_BATTERY,
    MBC3_TIMER_BATTERY,
    MBC3_TIMER_RAM_BATTERY,
    MBC3,
    MBC3_RAM,
    MBC3_RAM_BATTERY,
    MBC4,
    MBC4_RAM,
    MBC4_RAM_BATTERY,
    MBC5,
    MBC5_RAM,
    MBC5_RAM_BATTERY,
    MBC5_RUMBLE,
    MBC5_RUMBLE_RAM,
    MBC5_RUMBLE_RAM_BATTERY,
    POCKET_CAMERA,
    BANDAI_TAMA5,
    HUC3,
    HUC1_RAM_BATTERY
};
    

typedef struct {
    int header_value;
    bool supported;
    char* name;
    int id;
} CartridgeType;
    

CartridgeType cartridge_types[] = {
    { 0x00, true,  "ROM ONLY", ROM_ONLY},
    { 0x01, false, "MBC1", MBC1},
    { 0x02, false, "MBC1+RAM", MBC1_RAM}, 
    { 0x03, false, "MBC1+RAM+BATTERY", MBC1_RAM_BATTERY },
    { 0x05, false, "MBC2", MBC2 },
    { 0x06, false, "MBC2+BATTERY", MBC2_BATTERY },
    { 0x08, false, "ROM+RAM", ROM_RAM },
    { 0x09, false, "ROM+RAM+BATTERY", ROM_RAM_BATTERY },
    { 0x0B, false, "MMM01", MMM01 },
    { 0x0C, false, "MMM01+RAM", MMM01_RAM },
    { 0x0D, false, "MMM01+RAM_BATTERY", MMM01_RAM_BATTERY },
    { 0x0F, false, "MBC3+TIMER+BATTERY", MBC3_TIMER_BATTERY },
    { 0x10, false, "MBC3+TIMER+RAM+BATTERY", MBC3_TIMER_RAM_BATTERY },
    { 0x11, false, "MBC3", MBC3 },
    { 0x12, false, "MBC3+RAM", MBC3_RAM },
    { 0x13, false, "MBC3+RAM+BATTERY", MBC3_RAM_BATTERY },
    { 0x15, false, "MBC4", MBC4 },
    { 0x16, false, "MBC4+RAM", MBC4_RAM },
    { 0x17, false, "MBC4+RAM+BATTERY", MBC4_RAM_BATTERY },
    { 0x19, false, "MBC5", MBC5 },
    { 0x1A, false, "MBC5+RAM", MBC5_RAM },
    { 0x1B, false, "MBC5+RAM+BATTERY", MBC5_RAM_BATTERY },
    { 0x1C, false, "MBC5+RUMBLE", MBC5_RUMBLE },
    { 0x1D, false, "MBC5+RUMBLE+RAM", MBC5_RUMBLE_RAM },
    { 0x1E, false, "MBC5+RUMBLE+RAM+BATTERY", MBC5_RUMBLE_RAM_BATTERY },
    { 0xFC, false, "POCKET CAMERA", POCKET_CAMERA },
    { 0xFD, false, "BANDAI TAMA5", BANDAI_TAMA5 },
    { 0xFE, false, "HUC3", HUC3 },
    { 0xFF, false, "HUC1+RAM+BATTERY", HUC1_RAM_BATTERY }
};

bool has_battery = false;

void set_has_battery(CartridgeType* cartridge_type) {
    assert(cartridge_type != NULL);
    switch(cartridge_type->id) {
        case MBC1_RAM_BATTERY:
        case MBC2_BATTERY:
        case ROM_RAM_BATTERY:
        case MMM01_RAM_BATTERY:
        case MBC3_TIMER_BATTERY:
        case MBC3_TIMER_RAM_BATTERY:
        case MBC3_RAM_BATTERY:
        case MBC4_RAM_BATTERY:
        case MBC5_RAM_BATTERY:
        case MBC5_RUMBLE_RAM_BATTERY:
        case HUC1_RAM_BATTERY:
            has_battery = true;
            printf("cartridge has battery\n");
            return;
    }
    has_battery = false;
}

CartridgeType* cartridge_type = NULL;

CartridgeType* get_cartridge_type(int header_value) {
    int i;
    for(i = 0; i < (sizeof(cartridge_types) / sizeof(CartridgeType)); ++i) {
        if(cartridge_types[i].header_value == header_value) {
            return &cartridge_types[i];
        }
    }
    return NULL;
}

static int get_cartridge_ram_size(int header_value);
/* The contents of "buffer" are not needed after
   this function is called
   TODO
   check for max size
 */

int load_cartridge(const uint8_t* buffer, int size) {
    assert(buffer != NULL);
    assert(size > 0);
    
    //TODO make this better
    assert(cartridge_rom == NULL);

    cartridge_rom = (uint8_t*)malloc(size);
    
    //TODO better null check
    assert(cartridge_rom != NULL);
    memcpy(cartridge_rom, buffer, size);

    cartridge_rom_size = size;

    /* Cartridge RAM */
    cartridge_ram_size = get_cartridge_ram_size(cartridge_rom[0x0149]);
    //TODO handle errors
    assert(cartridge_ram_size >= 0);
    assert(cartridge_ram == NULL);

    if(cartridge_ram_size > 0) {
        cartridge_ram = (uint8_t*)malloc(cartridge_ram_size);
        assert(cartridge_ram != NULL); 
    } else {
        printf("skipping allocation of cartridge ram\n");
    }

    /* TYPE */
    
    cartridge_type = get_cartridge_type(cartridge_rom[0x0147]);
    //TODO handle errors
    assert(cartridge_type != NULL);
    printf("cartridge type: %s\n", cartridge_type->name);


    set_has_battery(cartridge_type);

    printf("cartridge loaded \n\tRAM: %i KiB\n\tROM: %i KiB\n", 
            cartridge_ram_size / 1024, size / 1024);

    //TODO actually verify...
    verify_cartridge();


    //TODO this is only temporary
    int save_state_size = get_save_state_size();
    printf("Save State size: %i B\n", save_state_size);

    return 0;
}

static int get_cartridge_ram_size(int header_value) {
    switch(header_value) {
        case 0:
            return 0; // no cartridge ram
        case 1:
            return 0x800; // 2KiB
        case 2:
            return 0x2000; // 8KiB
        case 3:
            return 0x8000; // 32KiB
    }
    return -1;
}

static int get_cartridge_rom_size(int header_value) {
    if(header_value >= 0 && header_value <= 7) {
        return(0x8000 << header_value);
    }
    return -1;
}





/* TODO finish this */
int verify_cartridge() {
	
	char title[0x10];
	title[0x10 - 1] = '\0';
	int i;
	for(i = 0x134; i < 0x144; ++i) {
		title[i - 0x134] = cartridge_rom[i];
	}
	printf("Title: %s\n", title);

    int expected_cartridge_rom_size =
       get_cartridge_rom_size(cartridge_rom[0x148]);

	printf("expected ROM size: %i KiB\n", 
            expected_cartridge_rom_size / 1024);
	
    /* CHECKSUM */
	u32 sum = 0;
	for(i = 0; i < cartridge_rom_size; ++i) {
		sum += cartridge_rom[i];
	}
	sum -= ((u16)cartridge_rom[0x14e] + cartridge_rom[0x14f]);

    sum &= 0x0000FFFF;

    u16 checksum = (cartridge_rom[0x014E] << 8) | cartridge_rom[0x014F];

    if(sum != checksum) {
        printf("WARNING: invalid checksum\n");
        printf("\t%X != %X\n", sum, checksum);
    }

	return 0;
}


