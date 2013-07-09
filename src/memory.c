#include "memory.h"

#include "joypad.h"

/*
#include "../gui/debugger.h"
*/

/*
#define INVALID_WRITE( location )															\
			fprintf( stderr, "attempted to write to invalid location: %04X\n", location ); 	\
			exit( EXIT_FAILURE )
			
#define INVALID_READ( location )															\
			fprintf( stderr, "attempted to read from invalid location: %04X\n", location ); 	\
			exit( EXIT_FAILURE )
*/
#define INVALID_WRITE( location )															\
			/*fprintf( stderr, "attempted to write to invalid location: %04X\n", location );*/ 	\
			return
			
#define INVALID_READ( location )															\
			/*fprintf( stderr, "attempted to read from invalid location: %04X\n", location );*/ 	\
			return 0
            
#define PALLETE_SIZE 0x40
#define SPRITE_PALLETE_SIZE 0x40

u8 cartridge_ram[CARTRIDGE_RAM_SIZE];
u8 cartridge_rom[CARTRIDGE_ROM_SIZE];


u8 pallete[PALLETE_SIZE];
u8 sprite_pallete[SPRITE_PALLETE_SIZE];

u8 gameboy_ram[GAMEBOY_RAM_SIZE];
u8 gameboy_vram[GAMEBOY_VRAM_SIZE];
u8 gameboy_oam[GAMEBOY_OAM_SIZE];

//u8 bios[BIOS_SIZE];
#include "bios.h"

u8 zero_page[0x7F];             //127B
u8 interrupt_enable;
u8 hardware_registers[0x80];    //128B


u8 IME; /* interrupt master enable flag */
	
u8 mbc_control[4];

static bool memory_initialized = false;
//static bool memory_debugger_enabled = false;
/*
static Debugger *debugger = NULL;
*/

/*
void set_memory_debugger_enabled( bool value ) {
#ifdef DEBUG
    char *val_str = (value ? "TRUE" : "FALSE");
    printf("memory_debugger_enabled set to %s\n", val_str);
#endif
    memory_debugger_enabled = value;
}
*/

/*TEMPTEMPTEMPTEMPTEMPTE*/
void dump_memory() {
    FILE *dump = fopen("ram_dump.bin", "wb");
    fwrite(gameboy_ram,1, GAMEBOY_RAM_SIZE, dump);
    fclose(dump);
    
    dump = fopen("zero_page_dump.bin", "wb");
    fwrite(zero_page,1, 0x7F, dump);
    fclose(dump);
    
    dump = fopen("hardware_registers_dump.bin", "wb");
    fwrite(hardware_registers,1, 0x80, dump);
    fclose(dump);
    
}

void initialize_memory( void ) 
{
	assert( memory_initialized == false );

	printf("CLEARING ALL MEMORY\n");
	int i;
	for(i = 0; i < CARTRIDGE_RAM_SIZE; ++i) {
		cartridge_ram[i] = 0;
	}
	for(i = 0; i < CARTRIDGE_ROM_SIZE; ++i) {
		cartridge_rom[i] = 0;
	}
	for(i = 0; i < GAMEBOY_RAM_SIZE; ++i) {
		gameboy_ram[i] = 0;
	}
	for(i = 0; i < GAMEBOY_VRAM_SIZE; ++i) {
		gameboy_vram[i] = 0;
	}
	for(i = 0; i < GAMEBOY_OAM_SIZE; ++i) {
		gameboy_oam[i] = 0;
	}
	
	printf("SKIPPING BIOS\n");
	/*
	for(i = 0; i < BIOS_SIZE; ++i) {
		bios[i] = 0;
	}
	*/
	// get rid of all these numbers...
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
	printf("DONE CLEARING MEMORY\n");
	
	/* do special memory stuff here */
    
	hardware_registers[SVBK] = 1; /* selected ram bank should never be 0 */
	hardware_registers[BLCK] = 0; /* enable bios */
    //fprintf(stderr, "disabling bios!\n");
    //hardware_registers[BLCK] = 0x11; /* enable bios */
	
	memory_initialized = true;
	
}

void reinitialize_memory( void )
{
	assert( memory_initialized == true );
	memory_initialized = false;
	initialize_memory();
}

void MBC_write(u16 location, u8 data)
{
	assert( memory_initialized == true );

    /*
    if( memory_debugger_enabled ) {
        debug_MBC_write(location, data);
    } 
    */
	
	/*
    if( debugger != NULL ) {
        debugger->debug_write( location, data );
    }
	*/
    
    if( location < 0x2000 ) 
	{ 
		/* 0000-1FFF external RAM bank enable */
        mbc_control[RAM_ENABLE] = data;
    }
	else if( location < 0x3000 ) 
	{
        /* 2000-2FFF ROM bank select LSB */
        mbc_control[ROM_BANK_LOW] = data;
    }
	else if( location < 0x4000 ) 
	{
        /* 3000-3FFF ROM bank select MSB */
		//assert( data == 0 || data == 1 );
		
        if(data != 0 || data != 1) {
            fprintf(stderr,"invalid ROM bank select MSB value\n");
            return;
        }
		
		data = data & 0x01;
        mbc_control[ROM_BANK_HIGH] = data;
    }
	else if( location < 0x6000 ) 
	{
        /* 4000-5FFF external RAM bank select */
        mbc_control[RAM_BANK] = data;
    }
	else if( location < 0x8000 ) 
	{
        /* 6000-7FFF RAM/ROM select (only in mbc1) */
		//fprintf( stderr, "invalid write: location=%X\tdata=%X\n", location, data );
    }
	else if( location < 0xA000 ) 
	{
		/* 8000-9FFF VRAM */
		u16 vram_bank = hardware_registers[VBK];
		assert( vram_bank == 0 || vram_bank == 1 );
		u32 offset = 0x8000 - ( vram_bank * 0x2000 );
		u32 vram_location = location - offset;
		gameboy_vram[vram_location] = data;
	}
	else if( location < 0xC000 ) 
	{
		/* A000-BFFF switchable cartridge RAM */
		if(mbc_control[RAM_ENABLE] == 0) {
			assert( false );
			return;		
		}
		u32 offset = 0xA000;
		u32 ram_bank = mbc_control[RAM_BANK];
		u32 ram_location = ( ram_bank * 0x2000 ) + location - offset;
        if(ram_location >= CARTRIDGE_RAM_SIZE) {
            return;
        }
		cartridge_ram[ram_location] = data;
	}
	else if( location < 0xD000 ) 
	{
		/* C000-CFFF internal RAM bank 0 */
		u32 offset = 0xC000;
		u32 ram_location = location - offset;
		gameboy_ram[ram_location] = data;
	}
	else if( location < 0xE000 ) 
	{
		/* D000-DFFF internal RAM bank 1-7 */
		int offset = 0xD000;
		u32 ram_bank = hardware_registers[SVBK];
		assert(ram_bank>0);
		u32 ram_location = ( ram_bank * 0x1000 ) + location - offset;
		gameboy_ram[ram_location] = data;
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
            gameboy_oam[location - offset] = data;	
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
                        hardware_registers[P1] |= (data & 0x0F);
                    } else if((data & 0x10) == 0) {
                        joypad_select_direction_keys();
                    } else if((data & 0x20) == 0) {
                        joypad_select_button_keys();
                    }
                    break;
                }
				case HDMA5:
				{
					u32 dma_source, dma_destination;
					u8 *dma_registers = &hardware_registers[HDMA1];
					dma_source = (*dma_registers) << 8;
					dma_source |= (*(++dma_registers));
					dma_destination = (*(++dma_registers)) << 8;
					dma_destination |= (*(++dma_registers));
					
					u32 dma_transfer_length = ((data & 0x7F) + 1) * 0x10;

					int i;
					for(i = 0; i < dma_transfer_length; ++i) {
							MBC_write(dma_destination + i, MBC_read(dma_source + i));
					}
					/* indicate that the DMA transfer is inactive */
					hardware_registers[HDMA5] |= 0x80;
					return;
				}
				case DMA:
				{
					//DMA transfer
					int dma_source = data << 8;
					int i;
					for(i = 0; i < 0xA0; ++i) {
							MBC_write(0xFE00 + i, MBC_read(dma_source + i));
					}
					return;
				}
				case DIV:
				case LY:
					/* writing to these registers resets their value */
					data = 0;
					break;
				case TAC:
				{
					//update timer control
					if((hardware_registers[TAC] & 0x3) != (data & 0x3)) {
						//frequency has changed
						u32 timer_counter = 0;
						switch(data & 0x3) {
							case 0x0:
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
				}	
				case SVBK: 
				{
					//assert(data < 8);
					/* writing 0 will select bank 1 */
					if(data == 0) {
						data = 1;
					}
					break;
				}
				case BCPD:
				{
					/* background pallete data */
					int pallete_index = hardware_registers[BCPS] & 0x3F;
					pallete[pallete_index] = data;
					if(hardware_registers[BCPS] & 0x80) {
						/* auto increment */
						++hardware_registers[BCPS];
					}
					hardware_registers[BCPS] &= 0xBF;
					// return because 
					return;
				}
				case OCPD:
				{
					// sprite pallete data
					int pallete_index = hardware_registers[OCPS] & 0x3F;
					sprite_pallete[pallete_index] = data;
					if(hardware_registers[OCPS] & 0x80) {
						++hardware_registers[OCPS];
					}
					hardware_registers[OCPS] &= 0xBF;
					return;
				}
				case BLCK:
				{
					if(data == 0x11) {
						printf("disabling bios\n");
						output_opcodes = true;
					}
				}
			}
			
			hardware_registers[location - offset] = data;
        }
		else if(location < 0xFFFF) 
		{
            /* FF80-FFFE zero page */
            int offset = 0xFF80;
            zero_page[location - offset] = data;
        } 
		else if(location == 0xFFFF) 
		{
            /* FFFF interrupt enable */
            interrupt_enable = data;
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

u8 MBC_read(u16 location)
{
	assert( memory_initialized == true );
	
    /*
    if( memory_debugger_enabled ) {
        debug_MBC_read(location);
    } 
    */
	/*
    if( debugger != NULL ) {
        debugger->debug_read( location );
    } 
*/	
    
    if( location < 0x4000 ) 
	{
		/* 0000-3FFF ROM bank 0 */
		if( hardware_registers[BLCK] != 0x11 ) 
		{
            /* BIOS is enabled */
            if( location < 0x100 || ( location > 0x1FF && location < 0x900 ) )
            {
                return bios[location];
            }
		}
        return cartridge_rom[location];
    } 
	else if(location < 0x8000) 
	{
		/* 4000-7FFF ROM bank 0 to n */
        u32 rom_bank = mbc_control[ROM_BANK_LOW];
        rom_bank |= mbc_control[ROM_BANK_HIGH] << 8;
        u32 rom_location = ( rom_bank * 0x4000 ) + ( location - 0x4000 );
		return cartridge_rom[rom_location];
    } 
	else if (location < 0xA000) 
	{
        /* 8000-9FFF VRAM */
		u8 vram_bank = hardware_registers[VBK];
		//assert( vram_bank == 0 && vram_bank == 1 );
		u32 offset = 0x8000 - (vram_bank * 0x2000);
		u32 vram_location = location - offset;
		return gameboy_vram[vram_location];	
    } 
	else if (location < 0xC000) 
	{
		/* A000-BFFF switchable cartridge RAM */
		if(mbc_control[RAM_ENABLE] == 0) 
		{
			//printf( "attempted to read from disabled ram\n");
			//exit( EXIT_FAILURE );
            //fatal_error();
            return 0;
		}
		u32 offset = 0xA000;
        u32 ram_bank = mbc_control[RAM_BANK];
        u32 ram_location = ( ram_bank * 0x2000 ) + location - offset;
		return cartridge_ram[ram_location];
    }
	else if (location < 0xD000) 
	{
		/* C000-CFFF internal RAM bank 0 */
        int offset = 0xC000;
        int ram_location = location - offset;
		return gameboy_ram[ram_location];
    }
	else if (location < 0xE000) 
	{
		/* D000-DFFF internal RAM bank 1-7 */
        int offset = 0xD000;
        u32 ram_bank = hardware_registers[SVBK];
        u32 ram_location = ram_bank * 0x1000 + location - offset;
		return gameboy_ram[ram_location];
    } 
	else if (location < 0x10000) /* E000-FFFF */
	{
        if(location < 0xFE00) 
		{
            /* E000-FDFF Echo RAM (maps to C000-DDFF) */
			return MBC_read( location - 0x2000 );	
        } 
		else if(location < 0xFEA0) 
		{
            /* FE00-FE9F object attribute memory */
            int offset = 0xFE00;
            return gameboy_oam[location - offset];
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
            return hardware_registers[location - offset];
        } 
		else if(location < 0xFFFF) 
		{
            /* FF80-FFFE zero page */
            int offset = 0xFF80;
            return zero_page[location - offset];
        } 
		else if (location == 0xFFFF) 
		{
            /* FFFF interrupt enable */
            return interrupt_enable;
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

/*
void memory_attach_debugger( Debugger *_debugger)
{
   assert( debugger == NULL ); 
   assert( _debugger != NULL );

   debugger = _debugger;
}

Debugger *memory_detach_debugger( void )
{
    assert( debugger != NULL );
    Debugger *_debugger = debugger;

    debugger = NULL;
    return _debugger;
}
*/
