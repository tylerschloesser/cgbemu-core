#include "gameboy.h"
#include "cpu.h"
#include "joypad.h"
#include "memory.h"

#include "cartridge.h"

#include "screen.h"

static bool gameboy_initialized = false;
GameboyColor* gb = NULL;

void gameboy_toggle_speed() {
    fullspeed = !fullspeed;
}

//TeMP
#include "hw_registers.h"


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
        int i;
        for(i = 0; i < 0x100; ++i) {
            swap(&cartridge->rom[i], &gb->bios[i]);
        }
        for(i = 0x200; i < 0x900; ++i) {
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
int initialize_gameboy(const char* bios_filepath)
{
    fprintf(stderr, "initialize_gameboy(bios_filepath=%s\n", bios_filepath);
    assert(gameboy_initialized == false);

    gb = (GameboyColor*)calloc(sizeof(GameboyColor), 1);

    assert(gb);
    if(!gb) return 1;

    gb->hw_registers = hw_registers;

    uint8_t* buffer = calloc(
              GAMEBOY_RAM_SIZE 
            + GAMEBOY_VRAM_SIZE 
            + GAMEBOY_OAM_SIZE 
            + GAMEBOY_BIOS_SIZE 
            + GAMEBOY_BG_PALLETE_SIZE 
            + GAMEBOY_OB_PALLETE_SIZE 
            + GAMEBOY_HRAM_SIZE, 1);

    gb->ram = buffer;
    gb->vram = (buffer += GAMEBOY_RAM_SIZE);
    gb->oam = (buffer += GAMEBOY_VRAM_SIZE);
    gb->bios = (buffer += GAMEBOY_OAM_SIZE);
    gb->bg_pallete = (buffer += GAMEBOY_BIOS_SIZE);
    gb->ob_pallete = (buffer += GAMEBOY_BG_PALLETE_SIZE);
    gb->hram = (buffer += GAMEBOY_HRAM_SIZE);
    
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
void gameboy_update_selected_ram()
{
    u32 ram_bank = gb->hw_registers[SVBK];
    gb->selected_ram = &gb->ram[ram_bank * 0x1000];
}

void gameboy_update_selected_vram()
{
    u32 vram_bank = gb->hw_registers[VBK];
    gb->selected_vram = &gb->vram[vram_bank * 0x2000];
}

int get_save_state_size() {
    // calculate memory required 
    int save_state_size = 0;
    
    save_state_size += sizeof(CpuState);
    /*
    save_state_size += cartridge_ram_size;
    save_state_size += cartridge_rom_size; //include ROM because
    */
    save_state_size += cartridge->ram_size;
    save_state_size += cartridge->rom_size; //include ROM because

    save_state_size += (0x40 + 0x40); //pallete and sprite pallete
    save_state_size += GAMEBOY_RAM_SIZE;
    save_state_size += GAMEBOY_VRAM_SIZE;
    save_state_size += GAMEBOY_OAM_SIZE;
    save_state_size += GAMEBOY_BIOS_SIZE;
    save_state_size += 0x7F; //zero page
    save_state_size += 1; //interrupt enabled
    save_state_size += 0x80; //hardware registers
    save_state_size += 1; //ime 
    save_state_size += 4; //mbc_control

    return save_state_size;
}

/**
 * memcpy increment destination
 */
void memcpy_id(uint8_t** dest_p, uint8_t* src, int size) {
    uint8_t* dest = (*dest_p);
    memcpy(dest, src, size);
    (*dest_p) += size;
}

/**
 * memcpy increment source
 */
void memcpy_is(uint8_t* dest, uint8_t** src_p, int size) {
    uint8_t* src = *src_p;
    memcpy(dest, src, size);
    (*src_p) += size;
}

void save_state(uint8_t** buffer) {
    
    int save_state_size = get_save_state_size();

    assert(save_state_size > 0);

    (*buffer) = (uint8_t*)malloc(save_state_size);
    uint8_t* buffer_original = (*buffer);    

    CpuState state = get_cpu_state();
    u8* state_raw = (u8*)&state;

    memcpy_id(buffer, state_raw, sizeof(CpuState));
    /*
    memcpy_id(buffer, cartridge_ram, cartridge_ram_size);
    memcpy_id(buffer, cartridge_rom, cartridge_rom_size);
    */

    memcpy_id(buffer, cartridge->ram, cartridge->ram_size);
    memcpy_id(buffer, cartridge->rom, cartridge->rom_size);

    memcpy_id(buffer, gb->bg_pallete, GAMEBOY_BG_PALLETE_SIZE);
    memcpy_id(buffer, gb->ob_pallete, GAMEBOY_OB_PALLETE_SIZE);
    memcpy_id(buffer, gb->ram, GAMEBOY_RAM_SIZE);
    memcpy_id(buffer, gb->vram, GAMEBOY_VRAM_SIZE);
    memcpy_id(buffer, gb->oam, GAMEBOY_OAM_SIZE);
    memcpy_id(buffer, gb->bios, GAMEBOY_BIOS_SIZE);
    memcpy_id(buffer, gb->hram, GAMEBOY_HRAM_SIZE);
    memcpy_id(buffer, &(gb->ie_register), 1);
    memcpy_id(buffer, gb->hw_registers, 0x80);
    memcpy_id(buffer, &(gb->ime_flag), 1);

    //TODO THIS NEEDS TO BE FIXED
    /*
    memcpy_id(buffer, mbc_control, 4);
    */

    /* reset the buffer pointer */
    (*buffer) = buffer_original;
}

void load_state(uint8_t* buffer, int size) {
    
    assert(buffer != NULL);
    assert(size > 0);
    assert(size == get_save_state_size());

    CpuState state;
    uint8_t* state_raw = (uint8_t*)&state;
    
    memcpy_is(state_raw, &buffer, sizeof(CpuState));
    /*
    memcpy_is(cartridge_ram, &buffer, cartridge_ram_size);
    memcpy_is(cartridge_rom, &buffer, cartridge_rom_size);
    */

    memcpy_is(cartridge->ram, &buffer, cartridge->ram_size);
    memcpy_is(cartridge->rom, &buffer, cartridge->rom_size);

    memcpy_is(gb->bg_pallete, &buffer, 0x40);
    memcpy_is(gb->ob_pallete, &buffer, 0x40);
    memcpy_is(gb->ram, &buffer, GAMEBOY_RAM_SIZE);
    memcpy_is(gb->vram, &buffer, GAMEBOY_VRAM_SIZE);
    memcpy_is(gb->oam, &buffer, GAMEBOY_OAM_SIZE);
    memcpy_is(gb->bios, &buffer, GAMEBOY_BIOS_SIZE);
    memcpy_is(gb->hram, &buffer, 0x7F);
    memcpy_is(&(gb->ie_register), &buffer, 1);
    memcpy_is(gb->hw_registers, &buffer, 0x80);
    memcpy_is(&(gb->ime_flag), &buffer, 1);
    /*
    memcpy_is(mbc_control, &buffer, 4);
    */
 
    set_cpu_state(state);

    cartridge_update_selected_rom();
    cartridge_update_selected_ram();
    gameboy_update_selected_ram();
    gameboy_update_selected_vram();
}

