#include "gameboy.h"
#include "cpu.h"
#include "joypad.h"
#include "memory.h"

#include "cartridge.h"

#include "screen.h"

static bool gameboy_initialized = false;
GameboyColor* gb = NULL;

// These are the values of the hardware registers after startup
static uint8_t hw_registers[] = {
  0x0f, 0x00, 0x00, 0x00, 0xcf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x01, 0x00, 0x80, 0xf3, 0xc1, 0x87, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x77, 0xf3, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
  0x00, 0xff, 0x00, 0xff, 0x91, 0x01, 0x00, 0x00, 0x91, 0x00, 0x00, 0xfc,
  0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
  0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x81, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void swap(uint8_t* a, uint8_t* b) {
    uint8_t c = *a;
    *a = *b;
    *b = c;
}

static void enable_bios()
{
    assert(gb);
    assert(gb->bios);
    assert(cartridge);
    assert(cartridge->rom);

    if(gb->use_bios) {
        for(int i = 0; i < 0x100; ++i) {
            swap(&cartridge->rom[i], &gb->bios[i]);
        }
        for(int i = 0x200; i < 0x900; ++i) {
            swap(&cartridge->rom[i], &gb->bios[i]);
        }
    }
}

void gameboy_disable_bios() {
    assert(gb);

    if(gb->use_bios)
        enable_bios();

}


static int read_bios(const char* filepath) {
    assert(gb);
    assert(gb->bios);
    fprintf(stderr, "read_bios(filepath=%s)\n", filepath);
    FILE* file = fopen(filepath, "rb");

    if(!file) {
        perror("fopen() failed");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    rewind(file);

    if(size != GAMEBOY_BIOS_SIZE) {
        fprintf(stderr, "Invalid BIOS\n");
        return 1;
    }

    if(fread(gb->bios, 1, GAMEBOY_BIOS_SIZE, file) == 0) {
        perror("fread() failed");
        fclose(file);
        return 1;
    }
    
    fclose(file);
    return 0;
}

/**
 * If bios_filepath is null, don't use the bios
 */
int initialize_gameboy(bool use_bios, const char* bios_filepath)
{
    fprintf(stderr, "initialize_gameboy(bios_filepath=%s\n", bios_filepath);
    assert(gameboy_initialized == false);

    gb = (GameboyColor*)calloc(sizeof(GameboyColor), 1);

    assert(gb);
    if(gb == NULL) {
        return 1;
    }

    if(use_bios == false) {
        memcpy(gb->hw_registers, hw_registers, GAMEBOY_HW_REGISTERS_SIZE);
    }

    if(bios_filepath) {
        if(read_bios(bios_filepath) != 0) {
            return 1;
        }
        gb->use_bios = true;
        gb->hw_registers[BLCK] = 0x00;
        enable_bios();
    } else {
        gb->use_bios = false;
        gb->hw_registers[BLCK] = 0x11;
    }

    gameboy_update_selected_ram();
    gameboy_update_selected_vram();

    gameboy_initialized = true;

    return 0;
}

/**
 * TODO is selected ram bank 0 allowed?
 */
void gameboy_update_selected_ram(void)
{
    u32 ram_bank = gb->hw_registers[SVBK];
    gb->selected_ram = &gb->ram[ram_bank * 0x1000];
}

void gameboy_update_selected_vram(void)
{
    u32 vram_bank = gb->hw_registers[VBK];
    gb->selected_vram = &gb->vram[vram_bank * 0x2000];
}

static int get_save_state_size() {
    return(sizeof(Cpu) + sizeof(Cartridge) + cartridge->ram_size + sizeof(GameboyColor));
}

int gameboy_save_state(const char* filepath) {

    assert(filepath);

    if(filepath == NULL) {
        fprintf(stderr, "No filepath specified\n");
        return 1;
    }

    FILE* file = fopen(filepath, "wb");
    if(file == NULL) {
        perror("fopen() failed");
        return 1;
    }
    
    fwrite((uint8_t*)cpu, 1, sizeof(Cpu), file);

    fwrite((uint8_t*)cartridge, 1, sizeof(Cartridge), file);
    fwrite(cartridge->ram, 1, cartridge->ram_size, file);

    fwrite((uint8_t*)gb, 1, sizeof(GameboyColor), file);

    fclose(file);

    fprintf(stderr, "State saved!\n");

    return 0;
}



int gameboy_load_state(const char* filepath) {
    assert(filepath);

    if(filepath == NULL) {
        fprintf(stderr, "No filepath specified\n");
        return 1;
    }

    FILE* file = fopen(filepath, "rb");
    if(file == NULL) {
        perror("fopen() failed");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    rewind(file);

    int expected_size = get_save_state_size();

    // Checks to see that the save state is the proper size
    if(size != expected_size) {
        fprintf(stderr, "Invalid save state\n");
        fclose(file);
        return 1;
    }

    fread((uint8_t*)cpu, 1, sizeof(Cpu), file);

    // Save the ram/rom pointers since their about to be overwritten
    uint8_t* cartridge_ram = cartridge->ram;
    uint8_t* cartridge_rom = cartridge->rom;

    fread((uint8_t*)cartridge, 1, sizeof(Cartridge), file);
    cartridge->ram = cartridge_ram;
    cartridge->rom = cartridge_rom;
    
    fread(cartridge->ram, 1, cartridge->ram_size, file);
    cartridge_update_mbc();
    cartridge_update_selected_rom();
    cartridge_update_selected_ram();

    fread((uint8_t*)gb, 1, sizeof(GameboyColor), file);
    gameboy_update_selected_ram();
    gameboy_update_selected_vram();

    fprintf(stderr, "State loaded!\n");

    return 0;
}

