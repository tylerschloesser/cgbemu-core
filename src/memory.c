#include "memory.h"

#include "joypad.h"
#include "cartridge.h"
#include "gameboy.h"

#include "screen.h"


#define INVALID_WRITE( location )                                                           \
            /*fprintf( stderr, "attempted to write to invalid location: %04X\n", location );*/  \
            return
            
#define INVALID_READ( location )                                                            \
            /*fprintf( stderr, "attempted to read from invalid location: %04X\n", location );*/     \
            return 0
            
int get_checksum(uint8_t* buffer, int size) {
    int sum = 0;
    for(int i = 0; i < size; ++i) {
        sum += buffer[i];
    }
    return sum;
}

void dump(char* filename, uint8_t* buffer, int size) {
    FILE* file = fopen(filename, "w");
    if(!file) return;
    fwrite(buffer, 1, size, file);
    fclose(file);
}


void print_after_bios() {
    int c = get_checksum(gb->hw_registers, GAMEBOY_HW_REGISTERS_SIZE);
    printf("hr_registers checkum: %X\n", c);

    dump("hw_registers.bin", gb->hw_registers, GAMEBOY_HW_REGISTERS_SIZE);

/*        
    c = get_checksum(gb->hram, GAMEBOY_HRAM_SIZE);
    printf("hram checkum: %X\n\n", c);

    c = get_checksum(gb->ram, GAMEBOY_RAM_SIZE);
    printf("ram checkum: %X\n\n", c);

    */
    print_cpu_state();
}

void hdma_transfer(void)
{
    assert(cartridge);
    
    for(int i = 0; i < 0x10; ++i) {
        memory_write(hdma_destination++, memory_read(hdma_source++));
    } 
    hdma_transfer_length -= 0x10;

    /* this looks weird, but i assure you it makes sense */
    if(--gb->hw_registers[HDMA5] == 0xFF) {
        /* transfer is complete */
        hdma_active = false;
    }
}


/* TODO move this to cartridge.c */
void update_cartridge_banking(Cartridge* cartridge) {

    assert(cartridge != NULL);

    if(cartridge->banking_mode == 0x00) {

        cartridge->selected_rom_bank |= 
            (cartridge->selected_ram_bank << 5);

        //update_selected_cartridge_rom(cartridge);
        cartridge_update_selected_rom();

    } else {

        /* reset the selected rom bank */
        cartridge->selected_rom_bank &= 0x1F;

        cartridge_update_selected_rom();
        cartridge_update_selected_ram();

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

uint8_t mbc1_read(uint16_t location) {
    uint8_t location_hi = ((location & 0xF000) >> 12);
    switch(location_hi) {
        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3:
        {
            location &= 0x3FFF; /* ~0xC000 */
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
            cartridge_update_selected_rom();
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
            cartridge_update_selected_ram();
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
            gb->ram[location] = data;
            break;
        case 0xD:
            location &= 0x0FFF;
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
            cartridge_update_selected_rom();

            break;
        case 0x3:
            cartridge->selected_rom_bank &= 0x00FF;
            cartridge->selected_rom_bank |= ((data & 0x1) << 8);
            cartridge_update_selected_rom();
            break;
        case 0x4:
        case 0x5:
            cartridge->selected_ram_bank = (data & 0x0F);
            cartridge_update_selected_ram();
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

void memory_write(uint16_t location, uint8_t data)
{
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
                case LCDC:
                    update_lcd_control_register(data);
                    break;
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
                        for(int i = 0; i < hdma_transfer_length; ++i) {
                            memory_write(hdma_destination++, 
                                    memory_read(hdma_source++));
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
                    for(int i = 0; i < 0xA0; ++i) {
                            memory_write(0xFE00 + i, memory_read(dma_source + i));
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
                    gameboy_update_selected_ram();
                    //update_selected_gameboy_ram_bank();
                    return;
                }
                case VBK:
                {
                    gb->hw_registers[VBK] = (data & 0x01);
                    //update_selected_gameboy_vram_bank();
                    gameboy_update_selected_vram();
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
                    if(data == 0x11) {
                        if(gb->use_bios) {
                            gameboy_disable_bios();
                        }
                    }
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

uint8_t memory_read(uint16_t location) {

    if(location < 0xE000) {
        return cartridge->read(location);
    }
    else if (location < 0x10000) /* E000-FFFF */
    {
        if(location < 0xFE00) 
        {
            /* E000-FDFF Echo RAM (maps to C000-DDFF) */
            return memory_read( location - 0x2000 );    
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

