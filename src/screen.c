#include <assert.h>

#include "screen.h"
#include "cpu.h"
#include "gameboy.h"
#include "memory.h"

#define MAX_TILES_PER_LINE 10

static void update_screen_buffer(int y_pixel, int x_pixel, uint16_t pallete_entry);
static void render_background(void);
static void render_sprites(void);

uint16_t screen_buffer[SCREEN_HEIGHT][SCREEN_WIDTH];

static uint8_t scanline_bg_priority[160];
static uint8_t scanline_obj_priority[160];

uint8_t lcd_enabled = 0x01;

static uint8_t window_enabled;
static uint8_t bg_enabled;
static uint8_t obj_enabled;
static uint16_t tile_data;
static uint16_t window_tile_map;
static uint16_t bg_tile_map;
static uint8_t sprite_height;

static uint16_t tile_data_offset;
static uint8_t signed_tiles;

// TODO document what this does when I'm less tired...
static uint8_t flip_table[] = {0,1,2,3,4,5,6,7,7,6,5,4,3,2,1,0};

static bool screen_initialized = false;

void initialize_screen() {
    assert( screen_initialized == false );
    memset(screen_buffer, 0, (SCREEN_HEIGHT * SCREEN_WIDTH) * sizeof(uint16_t));
    screen_initialized = true;
}

/**
 * Converts the gameboy color format to RGB565 and updates the screen buffer
 */
void update_screen_buffer( int y_pixel, int x_pixel, u16 pallete_entry ) 
{
    u8 r, g, b;
    
    r = (pallete_entry >> 0 ) & 0x1F;
    g = (pallete_entry >> 5 ) & 0x1F;
    b = (pallete_entry >> 10) & 0x1F;

    uint16_t rgb_565 = b | (g << 6) | (r << 11);
    screen_buffer[y_pixel][x_pixel] = rgb_565;
}

/**
 * Updates global variables that affect how the screen is rendered
 */
void update_lcd_control_register(uint8_t lcd_control)
{
    bg_enabled = (lcd_control & 0x01);
    obj_enabled = (lcd_control & 0x02);

    if(lcd_control & 0x04) {
        sprite_height = 16;
    } else {
        sprite_height = 8;
    }

    if(lcd_control & 0x08){
        bg_tile_map = 0x1C00;
    } else {
        bg_tile_map = 0x1800;
    }

    if(lcd_control & 0x10) {
        tile_data = 0x0000;
        signed_tiles = 0x00;
    } else {
        tile_data = 0x0800;
        signed_tiles = 0x01;
    }
    tile_data_offset = tile_data;

    window_enabled = (lcd_control & 0x20);

    if(lcd_control & 0x40) {
        window_tile_map = 0x1C00;
    } else {
        window_tile_map = 0x1800;
    }

    lcd_enabled = (lcd_control & 0x80);
}

/**
 * Renders the entire scanline
 */
void render_scanline(void)
{
    if(bg_enabled) {
        render_background();
    }
    if(obj_enabled) {
        render_sprites();
    }
}

/**
 * Renders the current background scanline. The priority is also recorded to
 * be used the the sprite rendering function
 */
void render_background(void)
{
    uint16_t tile_map_offset;
    uint16_t tile_row;
    
    uint8_t scanline = gb->hw_registers[LY];

    uint8_t scroll_y = gb->hw_registers[SCY];
    uint8_t scroll_x = gb->hw_registers[SCX];
    
    uint8_t window_y = gb->hw_registers[WY];
    uint8_t window_x = gb->hw_registers[WX];
    
    uint8_t y_position;
    uint8_t x_position;
    
    // deteremine whether we will render the overlaying window
    bool render_window = false;
    if(window_enabled && (window_y <= scanline)) {
        render_window = true;
    }
    
    //Determine which tile map is used
    if(render_window) {
        x_position = window_x - 7; // not sure if this is right...
        y_position = scanline - window_y;
        
        tile_map_offset = window_tile_map;
    } else {
        x_position = scroll_x;
        y_position = scanline + scroll_y;
        
        tile_map_offset = bg_tile_map;
    }
    
    tile_row = y_position / 8;
    
    //render each pixel on the horizontal scanline
    for(uint8_t x_pixel = 0; x_pixel < SCREEN_WIDTH; ++x_pixel, ++x_position ) {
           
        uint16_t tile_column = x_position / 8;
        uint16_t tile_address = tile_map_offset + (tile_row * 32) + tile_column;
        
        uint8_t tile_number = gb->vram[tile_address];
        uint16_t tile_attributes = gb->vram[tile_address + 0x2000]; // This is 16 bits on purpose
        
        int vram_offset = ((tile_attributes & 0x08) << 10);
        vram_offset += tile_data_offset;
        
        uint8_t pallete_number = tile_attributes & BACKGROUND_PALETTE_NUMBER;
        uint8_t pallete_index = pallete_number * 8;
        
        uint8_t tile_x_pixel = flip_table[((tile_attributes & 0x20) >> 2) | (x_position % 8)];
        uint8_t tile_y_pixel = flip_table[((tile_attributes & 0x40) >> 3) | (y_position % 8)];

        int tile_pixel = (tile_y_pixel * 8) + tile_x_pixel;
        
        int vram_index = (((tile_number + (128 * signed_tiles)) & 0xFF) * 16);
        vram_index += vram_offset + (tile_pixel / 8) * 2;
                
        uint8_t low  = gb->vram[vram_index++];
        uint8_t high = gb->vram[vram_index];

        uint8_t shift = tile_x_pixel;
        uint8_t bit = 0x80 >> shift;
       
        uint8_t pallete_index_offset;
        uint16_t pallete_entry;          

        shift = 7 - shift;

        pallete_index_offset = ((low & bit) >> shift) << 1;
        pallete_index_offset |= ((high & bit) >> shift) << 2;

        pallete_index += pallete_index_offset;

        pallete_entry = gb->bg_pallete[pallete_index++];
        pallete_entry |= (gb->bg_pallete[pallete_index] << 8);
        
        scanline_bg_priority[x_pixel] = pallete_index_offset;
        
        update_screen_buffer(scanline, x_pixel, pallete_entry);
    }
}

/**
 * Renders sprites on the current scanline
 */
void render_sprites(void) {

    memset(scanline_obj_priority, 0, 160);

    //TODO limit sprites to 10 per line (refer to pandocs)
    
    uint16_t sprite_attribute_index = 0xFE00; // OAM memory location
    int sprite_count = 40;
    
    // loop through all 40 sprites
    for(int sprite = 0; sprite < sprite_count; ++sprite) {
    
        s16 y_position = ((s16)memory_read(sprite_attribute_index++)) - 16;
        s16 x_position = ((s16)memory_read(sprite_attribute_index++)) - 8;
        uint8_t tile_number = memory_read(sprite_attribute_index++);
        uint8_t tile_attributes = memory_read(sprite_attribute_index++);

        if(y_position == -16 || y_position >= 144)
            continue;
            
        if(x_position == -8 || x_position >= 160)
            continue;
        

        s16 scanline = gb->hw_registers[LY];
        
        // check if the sprite is within the current scanline
        if((scanline >= y_position) && (scanline < (y_position + sprite_height))) {
            
            uint8_t pallete_number = tile_attributes & 0x07;
                  
            uint16_t vram_offset = (tile_attributes & 0x08) << 10;

            uint8_t x_flip = (tile_attributes & 0x20) >> 2;
            uint8_t y_flip = (tile_attributes & 0x40) >> 2;

            uint8_t bg_priority = tile_attributes & 0x80;
            
            s8 sprite_line = scanline - y_position;
            
            if(y_flip) {
                sprite_line -= ( sprite_height - 1 );
                sprite_line *= -1;
            }
            
            uint16_t sprite_address = vram_offset + (tile_number * 16) + (sprite_line * 2);
            uint8_t low = gb->vram[sprite_address++];
            uint8_t high = gb->vram[sprite_address];
            
            for(int sprite_pixel = 0; sprite_pixel < 8; ++sprite_pixel){
 
                int x_pixel = x_position + sprite_pixel;
                // TODO shouldn't need to check scanline here right?
                if(x_pixel < 0 || x_pixel > 159 || scanline_obj_priority[x_pixel]) {
                    continue;
                }
                if(bg_priority && scanline_bg_priority[x_pixel]) {
                    continue;
                }

                scanline_obj_priority[x_pixel] = 0x01;

                uint8_t shift = flip_table[x_flip | sprite_pixel];
                uint8_t bit = 0x80 >> shift;
                shift = 7 - shift;
                
                uint8_t pallete_index_offset;
                
                pallete_index_offset = ((low & bit) >> shift) << 1;
                pallete_index_offset |= ((high & bit) >> shift) << 2;

                if(pallete_index_offset == 0) {
                    continue;
                }

                uint8_t pallete_index = pallete_index_offset + pallete_number * 8;
                uint16_t pallete_entry;

                pallete_entry = gb->ob_pallete[pallete_index++];
                pallete_entry |= (gb->ob_pallete[pallete_index] << 8);
                
                update_screen_buffer(scanline, x_pixel, pallete_entry);
            }
        }        
    }
}
