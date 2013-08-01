#include "cartridge.h"
#include "memory.h"
#include <string.h>


Cartridge* cartridge = NULL;

static int verify_cartridge(void);

typedef struct {
    int header_value;
    MemoryBankController mbc;
    char name[32];
    int id;
} CartridgeType;

void cartridge_update_mbc(void) {
    assert(cartridge != NULL);

    switch(cartridge->mbc) {
        case NONE:
            break;
        case MBC1:
            cartridge->read = &mbc1_read;
            cartridge->write = &mbc1_write;
            break; 
        case MBC2:
            cartridge->read = &mbc2_read;
            cartridge->write = &mbc2_write;
            break; 
        case MBC3:
            cartridge->read = &mbc3_read;
            cartridge->write = &mbc3_write;
            break;
        case MBC4:
            cartridge->read = &mbc4_read;
            cartridge->write = &mbc4_write;
            break;
        case MBC5:
            cartridge->read = &mbc5_read;
            cartridge->write = &mbc5_write;
            break; 
        default:
            fprintf(stderr, "Unsupported MBC\n");
            assert(false);
            break;
    }
}
    
 

CartridgeType cartridge_types[] = {
    { 0x00, NONE, "ROM ONLY", ID_ROM_ONLY},
    { 0x01, MBC1, "MBC1", ID_MBC1},
    { 0x02, MBC1, "MBC1+RAM", ID_MBC1_RAM}, 
    { 0x03, MBC1, "MBC1+RAM+BATTERY", ID_MBC1_RAM_BATTERY },
    { 0x05, MBC2, "MBC2", ID_MBC2 },
    { 0x06, MBC2, "MBC2+BATTERY", ID_MBC2_BATTERY },
    { 0x08, NONE, "ROM+RAM", ID_ROM_RAM },
    { 0x09, NONE, "ROM+RAM+BATTERY", ID_ROM_RAM_BATTERY },
    { 0x0B, UNSUPPORTED, "MMM01", ID_MMM01 },
    { 0x0C, UNSUPPORTED, "MMM01+RAM", ID_MMM01_RAM },
    { 0x0D, UNSUPPORTED, "MMM01+RAM_BATTERY", ID_MMM01_RAM_BATTERY },
    { 0x0F, MBC3, "MBC3+TIMER+BATTERY", ID_MBC3_TIMER_BATTERY },
    { 0x10, MBC3, "MBC3+TIMER+RAM+BATTERY", ID_MBC3_TIMER_RAM_BATTERY },
    { 0x11, MBC3, "MBC3", ID_MBC3 },
    { 0x12, MBC3, "MBC3+RAM", ID_MBC3_RAM },
    { 0x13, MBC3, "MBC3+RAM+BATTERY", ID_MBC3_RAM_BATTERY },
    { 0x15, MBC4, "MBC4", ID_MBC4 },
    { 0x16, MBC4, "MBC4+RAM", ID_MBC4_RAM },
    { 0x17, MBC4, "MBC4+RAM+BATTERY", ID_MBC4_RAM_BATTERY },
    { 0x19, MBC5, "MBC5", ID_MBC5 },
    { 0x1A, MBC5, "MBC5+RAM", ID_MBC5_RAM },
    { 0x1B, MBC5, "MBC5+RAM+BATTERY", ID_MBC5_RAM_BATTERY },
    { 0x1C, MBC5, "MBC5+RUMBLE", ID_MBC5_RUMBLE },
    { 0x1D, MBC5, "MBC5+RUMBLE+RAM", ID_MBC5_RUMBLE_RAM },
    { 0x1E, MBC5, "MBC5+RUMBLE+RAM+BATTERY", ID_MBC5_RUMBLE_RAM_BATTERY },
    { 0xFC, UNSUPPORTED, "POCKET CAMERA", ID_POCKET_CAMERA },
    { 0xFD, UNSUPPORTED, "BANDAI TAMA5", ID_BANDAI_TAMA5 },
    { 0xFE, UNSUPPORTED, "HUC3", ID_HUC3 },
    { 0xFF, UNSUPPORTED, "HUC1+RAM+BATTERY", ID_HUC1_RAM_BATTERY }
};

bool get_has_battery(CartridgeType* cartridge_type) {
    assert(cartridge_type != NULL);

    switch(cartridge_type->id) {
        case ID_MBC1_RAM_BATTERY:
        case ID_MBC2_BATTERY:
        case ID_ROM_RAM_BATTERY:
        case ID_MMM01_RAM_BATTERY:
        case ID_MBC3_TIMER_BATTERY:
        case ID_MBC3_TIMER_RAM_BATTERY:
        case ID_MBC3_RAM_BATTERY:
        case ID_MBC4_RAM_BATTERY:
        case ID_MBC5_RAM_BATTERY:
        case ID_MBC5_RUMBLE_RAM_BATTERY:
        case ID_HUC1_RAM_BATTERY:
            return true;
    }
    return false;
}

CartridgeType* get_cartridge_type(int header_value) {
    for(int i = 0; i < (sizeof(cartridge_types) / sizeof(CartridgeType)); ++i) {
        if(cartridge_types[i].header_value == header_value) {
            return &cartridge_types[i];
        }
    }
    return NULL;
}

static int get_cartridge_ram_size(int header_value);

static int read_cartridge(const char* filepath) {
    assert(filepath);
    FILE* file = fopen(filepath, "rb");

    if(!file) {
        perror("fopen() failed");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    rewind(file);

    cartridge->rom = (uint8_t*)malloc(size);
    if(fread(cartridge->rom, 1, size, file) == 0) {
        perror("fread() failed");
        fclose(file);
        return 1;
    }

    cartridge->rom_size = size;

    return 0;
}

int initialize_cartridge(const char* cartridge_filepath) {
    
    fprintf(stderr, "initialize_cartridge(cartridge_filepath=%s)\n",
            cartridge_filepath);

    assert(cartridge_filepath);

    //TODO make this better
    assert(cartridge == NULL);
    cartridge = (Cartridge*)calloc(sizeof(Cartridge), 1);
    assert(cartridge != NULL);
    
    if(read_cartridge(cartridge_filepath) != 0) {
        return 1;
    }

    /* Cartridge RAM */
    cartridge->ram_size = get_cartridge_ram_size(cartridge->rom[0x0149]);
    //TODO handle errors
    assert(cartridge->ram_size >= 0);
    assert(cartridge->ram == NULL);

    if(cartridge->ram_size > 0) {
        cartridge->ram = (uint8_t*)malloc(cartridge->ram_size);
        assert(cartridge->ram != NULL); 
    } else {
        cartridge->ram = NULL;
    }

    /* TYPE */
    CartridgeType* cartridge_type = get_cartridge_type(cartridge->rom[0x0147]);
    //TODO handle errors
    assert(cartridge_type != NULL);
    cartridge->id = cartridge_type->id;
    memcpy(cartridge->name, cartridge_type->name, 32);
    cartridge->mbc = cartridge_type->mbc;

    printf("cartridge type: %s\n", cartridge_type->name);


    cartridge_update_mbc();

    cartridge->has_battery = get_has_battery(cartridge_type);

    cartridge_update_selected_rom();
    cartridge_update_selected_ram();

    printf("cartridge loaded \n\tRAM: %i KiB\n\tROM: %i KiB\n", 
            cartridge->ram_size / 1024, cartridge->rom_size / 1024);

    //TODO actually verify...
    verify_cartridge();

    return 0;
}

static int get_cartridge_ram_size(int header_value) {
    switch(header_value) {
        case 0:
            return 0; /* no cartridge ram */
        case 1:
            return 0x800; /* 2KiB */
        case 2:
            return 0x2000; /* 8KiB */
        case 3:
            return 0x8000; /* 32KiB */
    }
    return -1;
}

static int get_cartridge_rom_size(int header_value) {
    if(header_value >= 0 && header_value <= 7) {
        return(0x8000 << header_value);
    }
    return -1;
}


void cartridge_update_selected_rom(void) {

    cartridge->selected_rom = 
        &cartridge->rom[cartridge->selected_rom_bank * 0x4000]; 
     
}
void cartridge_update_selected_ram(void) {

    cartridge->selected_ram = 
        &cartridge->ram[cartridge->selected_ram_bank * 0x2000];
}




/* TODO finish this */
static int verify_cartridge(void) {
    
    char title[0x10];
    title[0x10 - 1] = '\0';
    for(int i = 0x134; i < 0x144; ++i) {
        title[i - 0x134] = cartridge->rom[i];
    }
    printf("Title: %s\n", title);

    int expected_cartridge_rom_size =
       get_cartridge_rom_size(cartridge->rom[0x148]);

    printf("expected ROM size: %i KiB\n", 
            expected_cartridge_rom_size / 1024);
    
    /* CHECKSUM */
    uint32_t sum = 0;
    for(int i = 0; i < cartridge->rom_size; ++i) {
        sum += cartridge->rom[i];
    }
    sum -= ((u16)cartridge->rom[0x14e] + cartridge->rom[0x14f]);

    sum &= 0x0000FFFF;

    u16 checksum = (cartridge->rom[0x014E] << 8) | cartridge->rom[0x014F];

    if(sum != checksum) {
        printf("WARNING: invalid checksum\n");
        printf("\t%X != %X\n", sum, checksum);
    }

    return 0;
}


