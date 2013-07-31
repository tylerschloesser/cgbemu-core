#include "cgbemu.h"
#include "cpu.h"
#include "screen.h"
#include "gameboy.h"
#include "memory.h"
#include "cartridge.h"
#include "joypad.h"

void cgbemu_run_to_vblank()
{
    start_cpu();
    emulate_cpu();
}

int initialize_cgbemu(const char* cartridge_filepath, const char* bios_filepath) {
    assert(cartridge_filepath);

    if(cartridge_filepath == NULL) {
        fprintf(stderr, "No cartridge filepath provided\n");
        return 1;
    }
    
    initialize_gameboy(bios_filepath);
    initialize_cartridge(cartridge_filepath);
    initialize_cpu();
    initialize_joypad();
    initialize_screen();

    //TODO check that all of these initialize functions were successful

    return 0;
}


void cgbemu_set_button_pressed(Button button, bool pressed)
{
    int gb_key;
    switch(button) {
        case UP:
            gb_key = JOYPAD_UP;
            break;
        case DOWN:
            gb_key = JOYPAD_DOWN;
            break;
        case RIGHT:
            gb_key = JOYPAD_RIGHT;
            break;
        case LEFT:
            gb_key = JOYPAD_LEFT;
            break;

        case A:
            gb_key = JOYPAD_A;
            break;
        case B:
            gb_key = JOYPAD_B;
            break;
        case SELECT:
            gb_key = JOYPAD_SELECT;
            break;
        case START:
            gb_key = JOYPAD_START;
            break;
        default:
            return;
    }

    if(pressed) {
        joypad_down(gb_key);
    } else {
        joypad_up(gb_key);
    }
}

uint16_t* cgbemu_get_screen_buffer()
{
    return get_screen_buffer();
}

void cgbemu_save_state(uint8_t** buffer, int* size) {
    (*size) = get_save_state_size();
    save_state(buffer);
}

void cgbemu_load_state(uint8_t* buffer, int size) {
    load_state(buffer, size);
}

