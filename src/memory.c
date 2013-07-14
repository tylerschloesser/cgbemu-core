#include "memory.h"

#include "joypad.h"
#include "cartridge.h"


#define INVALID_WRITE( location )															\
			/*fprintf( stderr, "attempted to write to invalid location: %04X\n", location );*/ 	\
			return
			
#define INVALID_READ( location )															\
			/*fprintf( stderr, "attempted to read from invalid location: %04X\n", location );*/ 	\
			return 0
            
#define PALLETE_SIZE 0x40
#define SPRITE_PALLETE_SIZE 0x40

/*
int cartridge_rom_size = 0;
int cartridge_ram_size = 0;

uint8_t* cartridge_rom = NULL;
uint8_t* cartridge_ram = NULL;
*/

/*
u8 pallete[PALLETE_SIZE];
u8 sprite_pallete[SPRITE_PALLETE_SIZE];

uint8_t* gameboy_ram = NULL;
uint8_t* gameboy_vram = NULL;
uint8_t* gameboy_oam = NULL;
uint8_t* gameboy_bios = NULL;
*/

void update_selected_cartridge_rom(Cartridge* cartridge);
void update_selected_cartridge_ram(Cartridge* cartridge);
//u8 bios[BIOS_SIZE];

/*
u8 zero_page[0x7F];             //127B
u8 interrupt_enable;
u8 hardware_registers[0x80];    //128B
*/


//u8 IME; /* interrupt master enable flag */
	
//u8 mbc_control[4];

static bool memory_initialized = false;

uint8_t MBC_read(uint16_t location);
void MBC_write(uint16_t location, uint8_t data);

uint8_t (*read_memory)(uint16_t);
void (*write_memory)(uint16_t, uint8_t);

/*
void update_selected_gameboy_ram_bank();
void update_selected_gameboy_vram_bank();
*/


void update_all_selected_banks() {
    gameboy_update_selected_vram_bank();
    gameboy_update_selected_ram_bank();
}


void update_selected_cartridge_banks() {
    update_selected_cartridge_ram(cartridge);
    update_selected_cartridge_rom(cartridge);
}

void hdma_transfer(void)
{
    assert(cartridge);
    assert((gb->hw_registers[HDMA5] & 0x80) == 0x00);
    /*
    fprintf(stderr, "hdma_transfer src=%04X dst=%04X len=%04X rom_bank=%X\n", 
            hdma_source,
            hdma_destination,
            hdma_transfer_length,
            cartridge->selected_rom_bank);
    */
    int i;
    for(i = 0; i < 0x10; ++i) {
        write_memory(hdma_destination++, read_memory(hdma_source++));
    } 
    hdma_transfer_length -= 0x10;

    /* this looks weird, but i assure you it makes sense */
    if(--gb->hw_registers[HDMA5] == 0xFF) {
        /* transfer is complete */
        hdma_active = false;
    }
}

void initialize_memory(GameboyModel model) 
{
	assert( memory_initialized == false );

    /*
    assert(gameboy_ram == NULL);
    assert(gameboy_vram == NULL);
    assert(gameboy_oam == NULL);
    assert(gameboy_bios == NULL);

    gameboy_ram = (uint8_t*)calloc(GAMEBOY_RAM_SIZE, 1);
    gameboy_vram = (uint8_t*)calloc(GAMEBOY_VRAM_SIZE, 1);
    gameboy_oam = (uint8_t*)calloc(GAMEBOY_OAM_SIZE, 1);
    gameboy_bios = (uint8_t*)calloc(GAMEBOY_BIOS_SIZE, 1);

    assert(gameboy_ram != NULL);
    assert(gameboy_vram != NULL);
    assert(gameboy_oam != NULL);
    assert(gameboy_bios != NULL);
    */

    write_memory = &MBC_write;
    read_memory = &MBC_read;
    
	// get rid of all these numbers...
    /*
    int i;
	for(i = 0; i < 0x7F; ++i) {
		zero_page[i] = 0;
	}
	interrupt_enable = 0;
	for(i = 0; i < 0x80; ++i) {
		hardware_registers[i] = 0;
	}
	
	for(i = 0; i < 0x40; ++i) {
		pallete[i] = 0;
	}
	for(i = 0; i < 4; ++i) {
		mbc_control[i] = 0;
	}
    */
	
	/* do special memory stuff here */
    
	//hardware_registers[SVBK] = 1; /* selected ram bank should never be 0 */
	//hardware_registers[BLCK] = 0; /* enable bios */
    //hardware_registers[VBK] = 0;

    update_all_selected_banks();
   
	memory_initialized = true;
}

//TODO temporary
bool in_bios = true;
//i have yet to figure out the dif between cgb and gb bios

void reinitialize_memory(GameboyModel model)
{
	assert( memory_initialized == true );
	memory_initialized = false;
	initialize_memory(model);
}

/*
uint8_t* selected_cartridge_rom_bank = NULL;
uint8_t* selected_cartridge_ram_bank = NULL;
*/
/*
uint8_t* selected_gameboy_vram_bank = NULL;
uint8_t* selected_gameboy_ram_bank = NULL;
*/


void update_selected_cartridge_rom(Cartridge* cartridge) {

    assert(cartridge != NULL);
  
    
    /* TODO check endianess */
    cartridge->selected_rom = 
        &cartridge->rom[cartridge->selected_rom_bank * 0x4000]; 
     
}
void update_selected_cartridge_ram(Cartridge* cartridge) {
    assert(cartridge != NULL);

    cartridge->selected_ram = 
        &cartridge->ram[cartridge->selected_ram_bank * 0x2000];
}

/*
void update_selected_cartridge_ram_bank() {
    u32 ram_bank = mbc_control[RAM_BANK];

    selected_cartridge_ram_bank = 
        &cartridge_ram[ram_bank * 0x2000];
}
*/


void update_cartridge_banking(Cartridge* cartridge) {

    assert(cartridge != NULL);

    if(cartridge->banking_mode == 0x00) {

        cartridge->selected_rom_bank |= 
            (cartridge->selected_ram_bank << 5);
        update_selected_cartridge_rom(cartridge);

    } else {

        /* reset the selected rom bank */
        cartridge->selected_rom_bank &= 0x1F;
        update_selected_cartridge_rom(cartridge);
        update_selected_cartridge_ram(cartridge);

    }
}

void mbc0_write(uint16_t location, uint8_t data) {
    assert(false);           
}

uint8_t mbc0_read(uint16_t location) {
    assert(false);
    return 0;
}

void mbc1_write(uint16_t location, uint8_t data) {
    uint8_t location_hi = ((location & 0xF000) >> 12);
    switch(location_hi) {
        case 0x0:
        case 0x1:
            cartridge->ram_enabled = ((data & 0x0A) == 0x0A);
            break;
        case 0x2:
        case 0x3:
            data &= 0x1F;
            if(data == 0x00) data = 0x01;
            cartridge->selected_rom_bank = data;
            update_cartridge_banking(cartridge);
            break;
        case 0x4:
        case 0x5:
            cartridge->selected_ram_bank = (data & 0x03);
            update_cartridge_banking(cartridge);
            break;
        case 0x6:
        case 0x7:
            cartridge->banking_mode = (data & 0x01);
            update_cartridge_banking(cartridge);
            break;
        case 0x8:
        case 0x9:
            location &= 0x1FFF;
            gb->selected_vram[location] = data;
            //selected_gameboy_vram_bank[location] = data;
            break;
        case 0xA:
        case 0xB:
            //TODO do something different here...
            assert(cartridge->ram_enabled == true);

            location &= 0x1FFF;
            cartridge->selected_ram[location] = data;
            break;
        case 0xC:
            location &= 0x0FFF;
            gb->ram[location] = data;
            //gameboy_ram[location] = data;
            break;
        case 0xD:
            location &= 0x0FFF;
            //selected_gameboy_ram_bank[location] = data;
            gb->selected_ram[location] = data;
            break;

    } 
}

uint8_t mbc1_read(uint16_t location) {
    uint8_t location_hi = ((location & 0xF000) >> 12);
    switch(location_hi) {
        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3:
        {
            location &= 0x3FFF; /* ~0xC000 */

            //DIRTY HAX. FIX THIS
            if(in_bios && (location < 0x100 || (location > 0x1FF && location < 0x900)))
                return gb->bios[location];

            return cartridge->rom[location];
        }
        case 0x4: 
        case 0x5:
        case 0x6:
        case 0x7:
            location &= 0x3FFF;
            return cartridge->selected_rom[location];
        case 0x8:
        case 0x9:
            location &= 0x1FFF; /* ~0xE000 */
            return gb->selected_vram[location];
            //return selected_gameboy_vram_bank[location];
        case 0xA:
        case 0xB:
            location &= 0x1FFF;
            return cartridge->selected_ram[location];
        case 0xC:
            location &= 0x0FFF; /* ~0xF000 */
            //return gameboy_ram[location];
            return gb->ram[location];
        case 0xD:
            location &= 0x0FFF;
            //return selected_gameboy_ram_bank[location];
            return gb->selected_ram[location];
    }
     /* this shouldn't happen */
    assert(false);
    return 0; 

}

void mbc2_write(uint16_t location, uint8_t data) {

    assert(false);
}

uint8_t mbc2_read(uint16_t location) {

    assert(false);
}


void mbc3_write(uint16_t location, uint8_t data) {
    uint8_t location_hi = ((location & 0xF000) >> 12);
    switch(location_hi) {
        case 0x0:
        case 0x1:
            cartridge->ram_enabled = ((data & 0x0A) == 0x0A);
            break;
        case 0x2:
        case 0x3:
            data &= 0x7F;
            if(data == 0x00) data = 0x01;
            cartridge->selected_rom_bank = data;
            update_selected_cartridge_rom(cartridge);
            break;
        case 0x4:
        case 0x5:
            if(data > 0x03) {
                cartridge->rtc_register = data;
                break;
            }
            cartridge->rtc_register = 0x00;

            cartridge->selected_ram_bank = (data & 0x03);
            /* TODO handle rtc stuff */ 
            update_selected_cartridge_ram(cartridge);
            break;
        case 0x6:
        case 0x7:
            break;
        case 0x8:
        case 0x9:
            location &= 0x1FFF;
            //selected_gameboy_vram_bank[location] = data;
            gb->selected_vram[location] = data;
            break;
        case 0xA:
        case 0xB:
        {
            //TODO do something different here...
            assert(cartridge->ram_enabled == true);

            if(cartridge->rtc_register) {
                break;
            }


            location &= 0x1FFF;
            cartridge->selected_ram[location] = data;
            break;
        }
        case 0xC:
            location &= 0x0FFF;
            //gameboy_ram[location] = data;
            gb->ram[location] = data;
            break;
        case 0xD:
            location &= 0x0FFF;
            //selected_gameboy_ram_bank[location] = data;
            gb->selected_ram[location] = data;
            break;
    } 
}

uint8_t mbc3_read(uint16_t location) {
    uint8_t location_hi = ((location & 0xF000) >> 12);
    switch(location_hi) {
        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3:
        {
            location &= 0x3FFF; /* ~0xC000 */

            //DIRTY HAX. FIX THIS
            if(in_bios && (location < 0x100 || (location > 0x1FF && location < 0x900)))
                return gb->bios[location];

            return cartridge->rom[location];
        }
        case 0x4: 
        case 0x5:
        case 0x6:
        case 0x7:
            location &= 0x3FFF;
            return cartridge->selected_rom[location];
        case 0x8:
        case 0x9:
            location &= 0x1FFF; /* ~0xE000 */
            //return selected_gameboy_vram_bank[location];
            return gb->selected_vram[location];
        case 0xA:
        case 0xB:
            if(cartridge->rtc_register) {
                return 0x00;
            }
            location &= 0x1FFF;
            return cartridge->selected_ram[location];
        case 0xC:
            location &= 0x0FFF; /* ~0xF000 */
            return gb->ram[location];
        case 0xD:
            location &= 0x0FFF;
            return gb->selected_ram[location];
    }

    /* this shouldn't happen */
    assert(false);
    return 0; 
}

void mbc4_write(uint16_t location, uint8_t data) {

    assert(false);
}

uint8_t mbc4_read(uint16_t location) {

    assert(false);
}

/*
void update_selected_cartridge_rom_bank() {
    u32 rom_bank = mbc_control[ROM_BANK_LOW];
    rom_bank |= mbc_control[ROM_BANK_HIGH] << 8;

    // TODO this should only happen in MBC1 (i think) 

    if(rom_bank == 0)
        rom_bank = 1;

    selected_cartridge_rom_bank = 
        &cartridge_rom[rom_bank * 0x4000];
}
*/

/*
void update_selected_cartridge_ram_bank() {
    u32 ram_bank = mbc_control[RAM_BANK];

    selected_cartridge_ram_bank = 
        &cartridge_ram[ram_bank * 0x2000];
}
*/

/*
void update_selected_gameboy_ram_bank() {

    u32 ram_bank = gb->hw_registers[SVBK];
    selected_gameboy_ram_bank = &gameboy_ram[ram_bank * 0x1000];

}

void update_selected_gameboy_vram_bank() {
    u32 vram_bank = gb->hw_registers[VBK];
    //printf("switching to vram_bank %i\n", vram_bank);

    selected_gameboy_vram_bank = 
        &gameboy_vram[vram_bank * 0x2000];
}
*/



void mbc5_write(uint16_t location, uint8_t data) {
    uint8_t location_hi = ((location & 0xF000) >> 12);
    switch(location_hi) {
        case 0x0:
        case 0x1:
            cartridge->ram_enabled = ((data & 0x0A) == 0x0A);
            break;
        case 0x2:
            cartridge->selected_rom_bank &= 0xFF00;
            cartridge->selected_rom_bank |= data;
            update_selected_cartridge_rom(cartridge);

            break;
        case 0x3:
            cartridge->selected_rom_bank &= 0x00FF;
            cartridge->selected_rom_bank |= ((data & 0x1) << 8);
            update_selected_cartridge_rom(cartridge);
            break;
        case 0x4:
        case 0x5:
            cartridge->selected_ram_bank = (data & 0x0F);
            update_selected_cartridge_ram(cartridge);
            break;
        case 0x6:
        case 0x7:
            break;
        case 0x8:
        case 0x9:
            location &= 0x1FFF;
            gb->selected_vram[location] = data;
            break;
        case 0xA:
        case 0xB:
            //TODO do something different here...
            assert(cartridge->ram_enabled == true);

            location &= 0x1FFF;
            cartridge->selected_ram[location] = data;
            break;
        case 0xC:
            location &= 0x0FFF;
            gb->ram[location] = data;
            break;
        case 0xD:
            location &= 0x0FFF;
            gb->selected_ram[location] = data;
            break;
    }
}

uint8_t mbc5_read(uint16_t location) {
    uint8_t location_hi = ((location & 0xF000) >> 12);
    switch(location_hi) {
        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3:
        {
            location &= 0x3FFF; /* ~0xC000 */

            //HAXXY HAX
            if(in_bios && (location < 0x100 || (location > 0x1FF && location < 0x900)))
                return gb->bios[location];

            return cartridge->rom[location];
        }
        case 0x4: 
        case 0x5:
        case 0x6:
        case 0x7:
            location &= 0x3FFF;
            return cartridge->selected_rom[location]; 
        case 0x8:
        case 0x9:
            location &= 0x1FFF; /* ~0xE000 */
            return gb->selected_vram[location];
        case 0xA:
        case 0xB:
            //TODO do something different here...
            assert(cartridge->ram_enabled == true);

            location &= 0x1FFF;
            return cartridge->selected_ram[location];
        case 0xC:
            location &= 0x0FFF; /* ~0xF000 */
            return gb->ram[location];
        case 0xD:
            location &= 0x0FFF;
            return gb->selected_ram[location];
    }
             
    assert(false);
    return 0;
}



/*
void common_write(u16 location, u8 data) {

    if(location < 0xFE00) {
        
    } else if(location < 0xFEA0) {

    }

*/


void MBC_write(uint16_t location, uint8_t data)
{
	assert( memory_initialized == true );

    if(location < 0xE000) {
        if(location == 0x83c0) {
            //printf("wrote %X to %X\n", data, location);
            //cpu_step = true;       
        }
        cartridge->write(location, data);
        return;
    }
	else if(location < 0x10000) /* E000-FFFF */
	{
        if(location < 0xFE00) 
		{
            /* E000-FDFF echo RAM (not usable) */
			INVALID_WRITE( location );	
        } 
		else if(location < 0xFEA0) 
		{
			/* FE00-FE9F oject attribute memory */
            int offset = 0xFE00;
            gb->oam[location - offset] = data;	
        }
		else if (location < 0xFF00) 
		{
			INVALID_WRITE( location );	
        }
		else if(location < 0xFF80)  
		{
			/* FF00-FF7F hardware registers */
            int offset = 0xFF00;
			
			switch(location - offset) {
                case P1:
                {
                    //joypad register
                    data &= 0x30;
                    
                    if((data & 0x30) == 0) {
                        // ? both button and dir keys selected?
                        gb->hw_registers[P1] |= (data & 0x0F);
                    } else if((data & 0x10) == 0) {
                        joypad_select_direction_keys();
                    } else if((data & 0x20) == 0) {
                        joypad_select_button_keys();
                    }
                    break;
                }
				case HDMA5:
				{
                    if(hdma_active == true) {
                        if((data & 0x80) == 0x00) {
                            /* cancel the DMA */
                            hdma_active = false;
                            gb->hw_registers[HDMA5] = 0xFF;
                        } else {
                            /* TODO not sure what this does... */
                            gb->hw_registers[HDMA5] = (data & 0x7F); 
                        }
                        return;
                    }

                    /* TODO this is stupid... fix this */
                    u8 *dma_registers = &gb->hw_registers[HDMA1];
                    hdma_source = (*dma_registers) << 8;
                    hdma_source |= (*(++dma_registers));
                    hdma_destination = (*(++dma_registers)) << 8;
                    hdma_destination |= (*(++dma_registers));

                    /* clear the 4 lsb */
                    hdma_destination &= ~0x000F;
                    //hdma_destination &= ~0x000F;
                    hdma_source &= 0xFFF0;
                    /* clear the 3 msb */
                    hdma_destination &= ~0xE000;
                    hdma_destination |= 0x8000;
                    
                    hdma_transfer_length = ((data & 0x7F) + 1) * 0x10;

                    if((data & 0x80) == 0x00) {
                        /* general purpose DMA. nothing special required */
                        printf("General DMA\n");
                        int i;
                        for(i = 0; i < hdma_transfer_length; ++i) {
                            write_memory(hdma_destination++, 
                                    read_memory(hdma_source++));
                        }

                        gb->hw_registers[HDMA1] = 0xFF;
                        gb->hw_registers[HDMA2] = 0xFF;
                        gb->hw_registers[HDMA3] = 0xFF;
                        gb->hw_registers[HDMA4] = 0xFF;
                        gb->hw_registers[HDMA5] = 0xFF;
                    } else {
                        /* hblank dma */

                        gb->hw_registers[HDMA5] = (data & 0x7F);
                        hdma_active = true;

                        /* TODO is this correct? */
                        u8 lcd_status = gb->hw_registers[STAT];
                        if((lcd_status & 0x03) == 0x00) {
                            /* currently in a hblank */
                            hdma_transfer();
                        }
                    }

					return;
				}
				case DMA:
				{
					//DMA transfer
					int dma_source = data << 8;
					int i;
					for(i = 0; i < 0xA0; ++i) {
							write_memory(0xFE00 + i, read_memory(dma_source + i));
					}
                    /* temp */
                    gb->hw_registers[DMA] = data;
					return;
				}
				case DIV:
				case LY:
					/* writing to these registers resets their value */
					data = 0;
					break;
				case TAC:
				{
                    //TODO TEMP

                    //if(data == 0x05) cpu_step = true;

					//update timer control
					if((gb->hw_registers[TAC] & 0x3) != (data & 0x3)) {
						//frequency has changed
						u32 timer_counter = 0;
						switch(data & 0x3) { case 0x0:
								timer_counter = 1024;
								break;
							case 0x1:
								timer_counter = 16;
								break;
							case 0x2:
								timer_counter = 64;
								break;
							case 0x3:
								timer_counter = 256;
								break;
						}
						if( timer_counter != 0 ) {
							cpu_set_timer_countr( timer_counter );
						}
					}
                    break;
				}	
				case SVBK: 
				{
					//assert(data < 8);
					/* writing 0 will select bank 1 */
					if(data == 0) {
						data = 1;
					}
                    gb->hw_registers[SVBK] = data;
                    gameboy_update_selected_ram_bank();
                    //update_selected_gameboy_ram_bank();
                    return;
				}
                case VBK:
                {
                    gb->hw_registers[VBK] = (data & 0x01);
                    //update_selected_gameboy_vram_bank();
                    gameboy_update_selected_vram_bank();
                    return;
                }
				case BCPD:
				{
					/* background pallete data */
					int pallete_index = gb->hw_registers[BCPS] & 0x3F;
					gb->bg_pallete[pallete_index] = data;
					if(gb->hw_registers[BCPS] & 0x80) {
						/* auto increment */
						++gb->hw_registers[BCPS];
					}
					gb->hw_registers[BCPS] &= 0xBF;
					// return because 
					return;
				}
				case OCPD:
				{
					// sprite pallete data
					int pallete_index = gb->hw_registers[OCPS] & 0x3F;
					gb->ob_pallete[pallete_index] = data;
					if(gb->hw_registers[OCPS] & 0x80) {
						++gb->hw_registers[OCPS];
					}
					gb->hw_registers[OCPS] &= 0xBF;
					return;
				}
				case BLCK:
				{
                    printf("WRITING %X to BLCK\n", data);
                    
					if(data == 0x11) {
						printf("disabling bios\n");
						output_opcodes = true;
					}
                    in_bios = false;
				}
			}
			
			gb->hw_registers[location - offset] = data;
        }
		else if(location < 0xFFFF) 
		{
            /* FF80-FFFE zero page */
            int offset = 0xFF80;
            gb->hram[location - offset] = data;
        } 
		else if(location == 0xFFFF) 
		{
            /* FFFF interrupt enable */
            gb->ie_register = data;
        } 
		else 
		{
			/* not possible */
        }
    } 
	else 
	{
        INVALID_WRITE( location );
    }
}

uint8_t MBC_read(uint16_t location) {

	assert( memory_initialized == true );

    if(location < 0xE000) {
        return cartridge->read(location);
    }
   	else if (location < 0x10000) /* E000-FFFF */
	{
        if(location < 0xFE00) 
		{
            /* E000-FDFF Echo RAM (maps to C000-DDFF) */
			return read_memory( location - 0x2000 );	
        } 
		else if(location < 0xFEA0) 
		{
            /* FE00-FE9F object attribute memory */
            int offset = 0xFE00;
            return gb->oam[location - offset];
        } 
		else if(location < 0xFF00) 
		{
			INVALID_READ( location );
        } 
		else if(location < 0xFF80) 
		{
			/* FF00-FF7F hardware registers */
            int offset = 0xFF00;
			/* handle special cases */
			switch(location - offset) 
			{
				case P1: /* joypad register */
                {
                    u8 joypad_state = get_joypad_state();
                    return joypad_state;
                }
			}
            return gb->hw_registers[location - offset];
        } 
		else if(location < 0xFFFF) 
		{
            /* FF80-FFFE zero page */
            int offset = 0xFF80;
            return gb->hram[location - offset];
        } 
		else if (location == 0xFFFF) 
		{
            /* FFFF interrupt enable */
            return gb->ie_register; 
        } 
		else 
		{
            /* not possible */
        }
    } 
	else 
	{
		INVALID_READ( location );
    }
	return 0;
}

