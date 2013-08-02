#ifndef SCREEN_H_
#define SCREEN_H_

#include "globals.h"

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144

extern uint16_t screen_buffer[SCREEN_HEIGHT][SCREEN_WIDTH];
extern uint8_t lcd_enabled;

void initialize_screen(void);
void render_scanline(void);
void update_lcd_control_register(uint8_t);

#endif // SCREEN_H_
