#include "cartridge.h"
#include "memory.h"
#include <string.h>


Cartridge* cartridge = NULL;

static int verify_cartridge(void);

typedef struct {
    int header_value;
    bool supported;
    char* name;
    int id;
} CartridgeType;
   
enum {
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
};

void set_memory_bank_controller(Cartridge* cartridge, CartridgeType* cartridge_type) {
    assert(cartridge != NULL);
    assert(cartridge_type != NULL);

    switch(cartridge_type->id) {
        case ID_ROM_ONLY:
        case ID_ROM_RAM:
        case ID_ROM_RAM_BATTERY:
            cartridge->mbc = NONE;
            
            break;
        case ID_MBC1:
        case ID_MBC1_RAM:
        case ID_MBC1_RAM_BATTERY:
            cartridge->mbc = MBC1;
            cartridge->read = &mbc1_read;
            cartridge->write = &mbc1_write;
            printf("cartridge set to MBC1\n");
            break; 
        case ID_MBC2:
        case ID_MBC2_BATTERY:
            cartridge->mbc = MBC2;
            cartridge->read = &mbc2_read;
            cartridge->write = &mbc2_write;
            break; 
        case ID_MBC3_TIMER_BATTERY:
        case ID_MBC3_TIMER_RAM_BATTERY:
        case ID_MBC3:
        case ID_MBC3_RAM:
        case ID_MBC3_RAM_BATTERY:
            cartridge->mbc = MBC3;
            cartridge->read = &mbc3_read;
            cartridge->write = &mbc3_write;
            //cartridge->read = &mbc1_read;
            //cartridge->write = &mbc1_write;


            break;
        case ID_MBC4:
        case ID_MBC4_RAM:
        case ID_MBC4_RAM_BATTERY:
            cartridge->mbc = MBC4;
            cartridge->read = &mbc4_read;
            cartridge->write = &mbc4_write;
            break;
        case ID_MBC5:
        case ID_MBC5_RAM:
        case ID_MBC5_RAM_BATTERY:
        case ID_MBC5_RUMBLE:
        case ID_MBC5_RUMBLE_RAM:
        case ID_MBC5_RUMBLE_RAM_BATTERY:
            cartridge->mbc = MBC5;
            cartridge->read = &mbc5_read;
            cartridge->write = &mbc5_write;
            printf("cartridge set to MBC5\n");
            break; 
        case ID_MMM01:
        case ID_MMM01_RAM:
        case ID_MMM01_RAM_BATTERY:
        case ID_POCKET_CAMERA:
        case ID_BANDAI_TAMA5:
        case ID_HUC3:
        case ID_HUC1_RAM_BATTERY:
        default:
            cartridge->mbc = UNSUPPORTED;
            break;
    }
}
    
 

CartridgeType cartridge_types[] = {
    { 0x00, true,  "ROM ONLY", ID_ROM_ONLY},
    { 0x01, false, "MBC1", ID_MBC1},
    { 0x02, false, "MBC1+RAM", ID_MBC1_RAM}, 
    { 0x03, false, "MBC1+RAM+BATTERY", ID_MBC1_RAM_BATTERY },
    { 0x05, false, "MBC2", ID_MBC2 },
    { 0x06, false, "MBC2+BATTERY", ID_MBC2_BATTERY },
    { 0x08, false, "ROM+RAM", ID_ROM_RAM },
    { 0x09, false, "ROM+RAM+BATTERY", ID_ROM_RAM_BATTERY },
    { 0x0B, false, "MMM01", ID_MMM01 },
    { 0x0C, false, "MMM01+RAM", ID_MMM01_RAM },
    { 0x0D, false, "MMM01+RAM_BATTERY", ID_MMM01_RAM_BATTERY },
    { 0x0F, false, "MBC3+TIMER+BATTERY", ID_MBC3_TIMER_BATTERY },
    { 0x10, false, "MBC3+TIMER+RAM+BATTERY", ID_MBC3_TIMER_RAM_BATTERY },
    { 0x11, false, "MBC3", ID_MBC3 },
    { 0x12, false, "MBC3+RAM", ID_MBC3_RAM },
    { 0x13, false, "MBC3+RAM+BATTERY", ID_MBC3_RAM_BATTERY },
    { 0x15, false, "MBC4", ID_MBC4 },
    { 0x16, false, "MBC4+RAM", ID_MBC4_RAM },
    { 0x17, false, "MBC4+RAM+BATTERY", ID_MBC4_RAM_BATTERY },
    { 0x19, false, "MBC5", ID_MBC5 },
    { 0x1A, false, "MBC5+RAM", ID_MBC5_RAM },
    { 0x1B, false, "MBC5+RAM+BATTERY", ID_MBC5_RAM_BATTERY },
    { 0x1C, false, "MBC5+RUMBLE", ID_MBC5_RUMBLE },
    { 0x1D, false, "MBC5+RUMBLE+RAM", ID_MBC5_RUMBLE_RAM },
    { 0x1E, false, "MBC5+RUMBLE+RAM+BATTERY", ID_MBC5_RUMBLE_RAM_BATTERY },
    { 0xFC, false, "POCKET CAMERA", ID_POCKET_CAMERA },
    { 0xFD, false, "BANDAI TAMA5", ID_BANDAI_TAMA5 },
    { 0xFE, false, "HUC3", ID_HUC3 },
    { 0xFF, false, "HUC1+RAM+BATTERY", ID_HUC1_RAM_BATTERY }
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

CartridgeType* cartridge_type = NULL;

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
    
    cartridge_type = get_cartridge_type(cartridge->rom[0x0147]);
    //TODO handle errors
    assert(cartridge_type != NULL);
    printf("cartridge type: %s\n", cartridge_type->name);


    set_memory_bank_controller(cartridge, cartridge_type);
    if(cartridge->mbc == UNSUPPORTED) {
        printf("This memory bank controller is not supported\n");
        return 1;
    }

    cartridge->has_battery = get_has_battery(cartridge_type);

    cartridge_update_selected_rom();
    cartridge_update_selected_ram();

    printf("cartridge loaded \n\tRAM: %i KiB\n\tROM: %i KiB\n", 
            cartridge->ram_size / 1024, cartridge->rom_size / 1024);

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


void cartridge_update_selected_rom() {

    cartridge->selected_rom = 
        &cartridge->rom[cartridge->selected_rom_bank * 0x4000]; 
     
}
void cartridge_update_selected_ram() {

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


