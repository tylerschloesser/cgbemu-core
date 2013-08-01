#include <stdint.h>
#include <stdbool.h>

#define FRAME_WIDTH 160
#define FRAME_HEIGHT 144

typedef enum {
    UP,
    DOWN,
    RIGHT,
    LEFT,
    A,
    B,
    SELECT,
    START
} Button;

int initialize_cgbemu(const char* cartridge_filepath, const char* bios_filepath);

uint16_t* cgbemu_get_screen_buffer();
void cgbemu_run_to_vblank();

void cgbemu_set_button_pressed(Button button, bool pressed);

void cgbemu_save_state(const char* filepath);
void cgbemu_load_state(const char* filepath);
