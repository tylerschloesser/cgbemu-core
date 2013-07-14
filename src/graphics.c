#include "globals.h"

#include "graphics.h"
#include "gameboy.h"
#include "cpu.h"

//testing
#include "screen.h"

#define MAX_TILES_PER_LINE 10

#define SPRITE_MODE_8X8  0
#define SPRITE_MODE_8x16 1


static int render_background(u8 lcd_control);
static int render_sprites();

uint8_t scanline_bg_priority[160];

/* for CGB sprite priority
TODO handle regular gb sprite priority */
uint8_t scanline_obj_priority[160];

static bool graphics_initialized = false;

void initialize_graphics()
{
	assert( graphics_initialized == false );
	
	graphics_initialized = true;
}

void reinitialize_graphics()
{
	assert( graphics_initialized == true );

	graphics_initialized = false;
	initialize_graphics();
}

int render_scanline()
{
	assert( graphics_initialized == true );
	
	
	u8 lcd_control = gb->hw_registers[LCDC];

	if(lcd_control & BG_DISPLAY) {
		render_background(lcd_control);
	}
	
	if(lcd_control & OBJ_DISPLAY_ENABLE) {
		render_sprites();
	}
	
    return 0;
}

bool do_print_map =false;
bool printing = false;
void print_map()
{
    do_print_map = true;
}

int render_background(u8 lcd_control)
{
	assert( graphics_initialized == true );
	
	u16 tile_data_offset;
	u16 tile_map_offset;
	u16 tile_row;
	
	bool signed_tiles;
	bool render_window;
	
	u8 scanline = gb->hw_registers[LY];

    //TEMP
    if(scanline == 0 && do_print_map && !printing) {
        do_print_map = false;
        printing = true;
    } else if(scanline == 0 && printing) {
        printing = false;
    }
    if(printing) {
        if(scanline%8==0)
            printf("\n%i: ", scanline);
    }


	u8 scroll_y = gb->hw_registers[SCY];
	u8 scroll_x = gb->hw_registers[SCX];
	
	u8 window_y = gb->hw_registers[WY];
	u8 window_x = gb->hw_registers[WX];
	
	//u16 y_position;
	u8 y_position;
	u8 x_position;
	
	// deteremine whether we will render the overlaying window
	render_window = false;
	if(lcd_control & WINDOW_DISPLAY_ENABLE) {
		if(window_y <= scanline) {
			render_window = true;
		}
	}

    /* TODO we could intercept the changing of the LCD control
        register and remove the need for these checks */	
	signed_tiles = false;
	tile_data_offset = 0x0;
	if((lcd_control & BG_WINDOW_TILE_DATA_SELECT) == 0) {
		tile_data_offset = 0x800;
		signed_tiles = true;
	}
	
	//Determine which tile map is used
	y_position = 0;
	tile_map_offset = 0x1800;
	if(render_window) {
		x_position = window_x - 7; // not sure if this is right...
		y_position = scanline - window_y;
		
		if(lcd_control & WINDOW_TILE_MAP_DISPLAY_SELECT) {
			tile_map_offset = 0x1C00;
		}
		
	} else {
	
		x_position = scroll_x;
		y_position = scanline + scroll_y;
		
		if(lcd_control & BG_TILE_MAP_DISPLAY_SELECT) {
			tile_map_offset = 0x1C00;
		}
		
	}
	
	tile_row = y_position / 8;
	
	//render each pixel on the horizontal scanline
	u8 x_pixel;
	for( x_pixel = 0; x_pixel < SCREEN_WIDTH; ++x_pixel, ++x_position ) {
	
	
		if(render_window) {
			if(x_pixel >= window_x) {
				//x_position  = x_pixel - window_x;
			}
		}
		
		// test background wrapping!
		//if(x_position > 2
		// background wrapping will already take place because x_position is an unsigned byte? 
		
		u16 tile_column = x_position / 8;
		u16 tile_address = tile_map_offset + (tile_row * 32) + tile_column;
		
		
		u8 tile_number = gb->vram[tile_address];
		u8 tile_attributes = gb->vram[tile_address + 0x2000];
        
		int vram_offset = (tile_attributes & TILE_VRAM_BANK_NUMBER) ? 0x2000 : 0;

		vram_offset += tile_data_offset;

		
        u8 pallete_number = tile_attributes & BACKGROUND_PALETTE_NUMBER;
		u8 pallete_index = pallete_number * 8;
		
		bool x_flip = (tile_attributes & HORIZONTAL_FLIP) ? true : false;
		bool y_flip = (tile_attributes & VERTICAL_FLIP) ? true : false;
		
		int tile_x_pixel = x_position % 8;
		int tile_y_pixel = y_position % 8;
        
		//testing
		if(x_flip == true) {
			tile_x_pixel -= 7;
			tile_x_pixel *= -1;
		}
		//testing NOT SURE IF THIS WORKS!!!!
		if(y_flip == true) {
			tile_y_pixel -= 7;
			tile_y_pixel *= -1;
		}
		
		int tile_pixel = (tile_y_pixel * 8) + tile_x_pixel;
		
		int vram_index;
		if(signed_tiles) {
			vram_index = (((s8)tile_number + 128) * 16) + (tile_pixel / 8) * 2;
		} else {
			vram_index = tile_number * 16 + (tile_pixel / 8) * 2;
		}


        if(printing) {
            if((x_pixel % 8 == 0) && (scanline % 8 == 0)) printf("%04X ", (vram_index + vram_offset));
        }



		u8 high = gb->vram[vram_index + vram_offset + 1];
		u8 low  = gb->vram[vram_index + vram_offset + 0];
		u8 bit = 0x80 >> (tile_pixel % 8);
       
		u8 pallete_index_offset = 0;
        u16 pallete_entry;			
        //u8 r, g, b;			
		
        /*
		if((high & bit) && (low & bit)) {
			pallete_index_offset = 6;
		} else if(high & bit) {
			pallete_index_offset = 4;
		} else if(low & bit) {
			pallete_index_offset = 2;
		} else {
			pallete_index_offset = 0;
		}
        */
        if( high & bit ) {
            pallete_index_offset = 4;
        }
        if( low & bit ) {
            pallete_index_offset += 2;
        }
		
		pallete_entry = gb->bg_pallete[pallete_index + pallete_index_offset];
		pallete_entry |= (gb->bg_pallete[pallete_index + pallete_index_offset + 1] << 8);
		
		/*
		assert( scanline >= 0 && scanline <= 144 && x_pixel >= 0 && x_pixel <= 159 );
		if(scanline < 0 || scanline > 143 || x_pixel < 0 || x_pixel > 159) {
			continue;
		}
		*/
		
		scanline_bg_priority[x_pixel] = high || low; // checks whether high or low are 0
		
		update_screen_buffer( scanline, x_pixel, pallete_entry );
		
	}
    return 0;
}

// only renders sprites on the current scanline
int render_sprites() {

//TODO
    memset(scanline_obj_priority, 0, 160);

	assert( graphics_initialized == true );

    u8 sprite_mode = SPRITE_MODE_8X8;
    if(gb->hw_registers[LCDC] & OBJ_SIZE) {
		sprite_mode = SPRITE_MODE_8x16;
	}
    
	//TODO limit sprites to 10 per line (refer to pandocs)
	
    u16 sprite_attribute_index = 0xFE00; // OAM memory location
    int sprite_count = 40;
    
	// loop through all 40 sprites
    int sprite;
    for(sprite = 0; sprite < sprite_count; ++sprite) {
    
        s16 y_position = ((s16)read_memory(sprite_attribute_index++)) - 16;
        s16 x_position = ((s16)read_memory(sprite_attribute_index++)) - 8;
        u8 tile_number = read_memory(sprite_attribute_index++);
        u8 tile_attributes = read_memory(sprite_attribute_index++);

        if(y_position == -16 || y_position >= 144)
            continue;
			
        if(x_position == -8 || x_position >= 160)
            continue;
        

        s16 scanline = gb->hw_registers[LY];
        //TODO temp
        //scanline -= 1;
        
		u8 sprite_height = 8;
		if(sprite_mode == SPRITE_MODE_8x16) {
			sprite_height = 16;
		}
		
        // check if the sprite is within the current scanline
        if((scanline >= y_position) && (scanline < (y_position + sprite_height))) {
      		
			u8 pallete_number = tile_attributes & 0x07;
		          
			u16 vram_offset = (tile_attributes & TILE_VRAM_BANK_NUMBER) ? 0x2000 : 0;

            bool x_flip = (tile_attributes & HORIZONTAL_FLIP) ? true : false;
            bool y_flip = (tile_attributes & VERTICAL_FLIP) ? true : false;

			u8 priority = tile_attributes & 0x80;
			
            s8 sprite_line = scanline - y_position;
            
			// ****TODO try vertical swap (experimental)
			if(y_flip == true) {
				sprite_line -= ( sprite_height - 1 );
				sprite_line *= -1;
			}
			
            //u16 sprite_address = vram_offset + (tile_number * 16) + (sprite_line * 2);
            u16 sprite_address = (tile_number * 16) + (sprite_line * 2);
            
            u8 high = gb->vram[vram_offset + sprite_address + 1];
            u8 low = gb->vram[vram_offset + sprite_address + 0];
            //u32 color;
            
            int sprite_pixel;

            for(sprite_pixel = 0; sprite_pixel < 8; ++sprite_pixel){
                
                s8 x_bit = sprite_pixel;
                if(x_flip == true) {
                    x_bit -= 7;
                    x_bit *= -1;
                }
                
                u8 bit = 0x80 >> x_bit;
                
				//temp
				u8 pallete_index = pallete_number * 8;
				u8 pallete_index_offset = 0;
				u16 pallete_entry;
				
                /*
                if((high & bit) && (low & bit)) {
					pallete_index_offset = 6;
                } else if(high & bit) {
					pallete_index_offset = 4;
                } else if(low & bit) {
					pallete_index_offset = 2;
                } else {
                    // white is transparant
                    continue;
                }
                */
                /* better speed ? */
                if( high & bit ) {
                    pallete_index_offset = 4;
                }
                if( low & bit ) {
                    pallete_index_offset += 2;
                }
                if( pallete_index_offset == 0 ) {
                    continue;
                }
                
				
				pallete_entry = gb->ob_pallete[pallete_index + pallete_index_offset];
                pallete_entry |= (gb->ob_pallete[pallete_index + pallete_index_offset + 1] << 8);

				int x_pixel = x_position + sprite_pixel;
				
				
				/* don't remove this you idiot! */
				if(scanline < 0 || scanline > 143 || x_pixel < 0 || x_pixel > 159) {
					continue;
				}

                /* TODO find a btter way of doing this */
                if(scanline_obj_priority[x_pixel]) continue;
                scanline_obj_priority[x_pixel] = 1;
				
				/*TODO find a better way of doing this */
				if(!priority || !(scanline_bg_priority[x_pixel]))
					update_screen_buffer( scanline, x_pixel, pallete_entry );

            }
        }        
    }
    return 0;
}
