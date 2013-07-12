#include "memory.h"

#include "joypad.h"


#define INVALID_WRITE( location )															\
			/*fprintf( stderr, "attempted to write to invalid location: %04X\n", location );*/ 	\
			return
			
#define INVALID_READ( location )															\
			/*fprintf( stderr, "attempted to read from invalid location: %04X\n", location );*/ 	\
			return 0
            
#define PALLETE_SIZE 0x40
#define SPRITE_PALLETE_SIZE 0x40

int cartridge_rom_size = 0;
int cartridge_ram_size = 0;

uint8_t* cartridge_rom = NULL;
uint8_t* cartridge_ram = NULL;

u8 pallete[PALLETE_SIZE];
u8 sprite_pallete[SPRITE_PALLETE_SIZE];

uint8_t* gameboy_ram = NULL;
uint8_t* gameboy_vram = NULL;
uint8_t* gameboy_oam = NULL;
uint8_t* gameboy_bios = NULL;

//u8 bios[BIOS_SIZE];

u8 zero_page[0x7F];             //127B
u8 interrupt_enable;
u8 hardware_registers[0x80];    //128B


u8 IME; /* interrupt master enable flag */
	
u8 mbc_control[4];

static bool memory_initialized = false;

uint8_t MBC_read(uint16_t location);
void MBC_write(uint16_t location, uint8_t data);

uint8_t (*read_memory)(uint16_t);
void (*write_memory)(uint16_t, uint8_t);

void update_selected_cartridge_rom_bank();
void update_selected_cartridge_ram_bank();
void update_selected_gameboy_ram_bank();
void update_selected_gameboy_vram_bank();


void update_all_selected_banks() {
    update_selected_gameboy_vram_bank();
    update_selected_gameboy_ram_bank();
    update_selected_cartridge_ram_bank();
    update_selected_cartridge_rom_bank();
}


void initialize_memory(GameboyModel model) 
{
	assert( memory_initialized == false );

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


    write_memory = &MBC_write;
    read_memory = &MBC_read;
    
	// get rid of all these numbers...
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
	
	/* do special memory stuff here */
	hardware_registers[SVBK] = 1; /* selected ram bank should never be 0 */
	hardware_registers[BLCK] = 0; /* enable bios */
    hardware_registers[VBK] = 0;

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

void mbc0_write(uint16_t location, uint8_t data);
uint8_t mbc0_read(uint16_t location);
void mbc1_write(uint16_t location, uint8_t data);
uint8_t mbc1_read(uint16_t location);
void mbc2_write(uint16_t location, uint8_t data);
uint8_t mbc2_read(uint16_t location);
void mbc3_write(uint16_t location, uint8_t data);
uint8_t mbc3_read(uint16_t location);
void mbc4_write(uint16_t location, uint8_t data);
uint8_t mbc4_read(uint16_t location);
void mbc5_write(uint16_t location, uint8_t data);
uint8_t mbc5_read(uint16_t location);

uint8_t* selected_cartridge_rom_bank = NULL;
uint8_t* selected_cartridge_ram_bank = NULL;
uint8_t* selected_gameboy_vram_bank = NULL;
uint8_t* selected_gameboy_ram_bank = NULL;


typedef enum {
    NONE,
    MBC1,
    MBC2,
} MemoryBankController;

void update_selected_cartridge_rom(Cartridge* cartridge) {

    assert(cartridge != NULL);

    
    /* TODO check endianess */
    cartridge->selected_rom = 
        &cartridge->rom[selected_rom_bank * 0x4000]; 
     
}

void update_selected_cartridge_rom_bank() {
    u32 rom_bank = mbc_control[ROM_BANK_LOW];
    rom_bank |= mbc_control[ROM_BANK_HIGH] << 8;

    /* TODO this should only happen in MBC1 (i think) */

    if(rom_bank == 0)
        rom_bank = 1;

    selected_cartridge_rom_bank = 
        &cartridge_rom[rom_bank * 0x4000];
}


typedef struct {
    MemoryBankController mbc;

    uint8_t* rom;
    uint8_t* ram;
    int ram_size;
    int rom_size;

    uint8_t* selected_rom;
    uint8_t* selected_ram;

    //int selected_rom_bank;
    int selected_rom_bank;
    int selected_ram_bank;

    uint8_t (*read)(uint16_t);
    void (*write)(uint16_t, uint8_t);

    bool ram_enabled;
    uint8_t banking_mode;

} Cartridge;

Cartridge* cartridge = NULL;

void update_cartridge_banking(Cartridge* cartridge) {

    assert(cartridge != NULL);

    if(cartridge->banking_mode == 0x00) {

        cartridge->selected_rom_bank |= 
            (cartridge->selected_ram_bank << 5);
        update_selected_rom_bank(cartridge);

    } else {

        cartridge->selected_rom_bank &= 0x1F;
        update_selected_ram_bank(cartridge);

    }
}

void mbc0_write(uint16_t location, uint8_t data) {
           
}

uint8_t mbc0_read(uint16_t location) {

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
            cartridge->selected_rom_bank = (data & 0x1F);
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
            selected_gameboy_vram_bank[location] = data;
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
            gameboy_ram[location] = data;
            break;
        case 0xD:
            location &= 0x0FFF;
            selected_gameboy_ram_bank[location] = data;
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
                return bios[location];

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
            return selected_gameboy_vram_bank[location];
        case 0xA:
        case 0xB:
            location &= 0x1FFF;
            return cartridge->selected_ram[location];
        case 0xC:
            location &= 0x0FFF; /* ~0xF000 */
            return gameboy_ram[location];
        case 0xD:
            location &= 0x0FFF;
            return selected_gameboy_ram_bank[location];
    }
 
}

void mbc2_write(uint16_t location, uint8_t data) {

}

uint8_t mbc2_read(uint16_t location) {

}

void mbc3_write(uint16_t location, uint8_t data) {

}

uint8_t mbc3_read(uint16_t location) {

}

void mbc4_write(uint16_t location, uint8_t data) {

}

uint8_t mbc4_read(uint16_t location) {

}

void update_selected_cartridge_rom_bank() {
    u32 rom_bank = mbc_control[ROM_BANK_LOW];
    rom_bank |= mbc_control[ROM_BANK_HIGH] << 8;

    /* TODO this should only happen in MBC1 (i think) */

    if(rom_bank == 0)
        rom_bank = 1;

    selected_cartridge_rom_bank = 
        &cartridge_rom[rom_bank * 0x4000];
}

void update_selected_cartridge_ram_bank() {
    u32 ram_bank = mbc_control[RAM_BANK];

    selected_cartridge_ram_bank = 
        &cartridge_ram[ram_bank * 0x2000];
}

void update_selected_gameboy_ram_bank() {

    u32 ram_bank = hardware_registers[SVBK];
    selected_gameboy_ram_bank = &gameboy_ram[ram_bank * 0x1000];

}

void update_selected_gameboy_vram_bank() {
    u32 vram_bank = hardware_registers[VBK];

    selected_gameboy_vram_bank = 
        &gameboy_vram[vram_bank * 0x2000];
}


void mbc5_write(uint16_t location, uint8_t data) {
    uint8_t location_hi = ((location & 0xF000) >> 12);
    switch(location_hi) {
        case 0x0:
        case 0x1:
            mbc_control[RAM_ENABLE] = data;
            break;
        case 0x2:
            mbc_control[ROM_BANK_LOW] = data;
            update_selected_cartridge_rom_bank();
            break;
        case 0x3:
            mbc_control[ROM_BANK_HIGH] = (data & 0x1);
            update_selected_cartridge_rom_bank();
            break;
        case 0x4:
        case 0x5:
            mbc_control[RAM_BANK] = data;
            update_selected_cartridge_ram_bank();
            break;
        case 0x6:
        case 0x7:
            break;
        case 0x8:
        case 0x9:
            location &= 0x1FFF;
            selected_gameboy_vram_bank[location] = data;
            break;
        case 0xA:
        case 0xB:
            //TODO do something different here...
            assert(mbc_control[RAM_ENABLE] != 0);

            location &= 0x1FFF;
            selected_cartridge_ram_bank[location] = data;
            break;
        case 0xC:
            location &= 0x0FFF;
            gameboy_ram[location] = data;
            break;
        case 0xD:
            location &= 0x0FFF;
            selected_gameboy_ram_bank[location] = data;
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
                return gameboy_bios[location];

            return cartridge_rom[location];
        }
        case 0x4: 
        case 0x5:
        case 0x6:
        case 0x7:
            location &= 0x3FFF;
            return selected_cartridge_rom_bank[location]; 
        case 0x8:
        case 0x9:
            location &= 0x1FFF; /* ~0xE000 */
            return selected_gameboy_vram_bank[location];
        case 0xA:
        case 0xB:
            location &= 0x1FFF;
            return selected_cartridge_ram_bank[location];
        case 0xC:
            location &= 0x0FFF; /* ~0xF000 */
            return gameboy_ram[location];
        case 0xD:
            location &= 0x0FFF;
            return selected_gameboy_ram_bank[location];
    }
             

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
        mbc5_write(location, data);
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
							write_memory(dma_destination + i, read_memory(dma_source + i));
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
							write_memory(0xFE00 + i, read_memory(dma_source + i));
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
                    //TODO TEMP

                    //if(data == 0x05) cpu_step = true;

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
                    break;
				}	
				case SVBK: 
				{
					//assert(data < 8);
					/* writing 0 will select bank 1 */
					if(data == 0) {
						data = 1;
					}
                    hardware_registers[SVBK] = data;
                    update_selected_gameboy_ram_bank();
                    return;
				}
                case VBK:
                {
                    hardware_registers[VBK] = data;
                    update_selected_gameboy_vram_bank();
                    return;
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
                    printf("WRITING %X to BLCK\n", data);
                    
					if(data == 0x11) {
						printf("disabling bios\n");
						output_opcodes = true;
					}
                    in_bios = false;
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

uint8_t MBC_read(uint16_t location) {

	assert( memory_initialized == true );

    if(location < 0xE000) {
        return mbc5_read(location);
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

