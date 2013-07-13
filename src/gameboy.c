#include "gameboy.h"
#include "cpu.h"
#include "joypad.h"
#include "memory.h"
#include "graphics.h"

#include "cartridge.h"

void gameboy_toggle_speed() {
    fullspeed = !fullspeed;
}

void gameboy_power_on() 
{

	initialize_cpu();
	initialize_memory();
	initialize_joypad();
	initialize_screen();
	initialize_graphics();
	
	return;
}

void gameboy_power_off()
{
    printf("gameboy_power_off()\n");
	stop_cpu();
}


void gameboy_toggle_button( Button button, bool pressed )
{

	assert( button != INVALID );

	// temporary to allow for the new interface
	int gb_key = -1;
	switch( button )
	{
		case UP:
			gb_key = JOYPAD_UP;
			break;
		case DOWN:
			gb_key = JOYPAD_DOWN;
			break;
		case LEFT:
			gb_key = JOYPAD_LEFT;
			break;
		case RIGHT:
			gb_key = JOYPAD_RIGHT;
			break;
		case START:
			gb_key = JOYPAD_START;
			break;
		case SELECT:
			gb_key = JOYPAD_SELECT;
			break;
		case A:
			gb_key = JOYPAD_A;
			break;
		case B:
			gb_key = JOYPAD_B;
			break;
	}
	
	if( gb_key != -1 )
	{
		if( pressed == true ) {
			joypad_down( gb_key );
		} else {
			joypad_up( gb_key );
		}
	}
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

void memcpy_id(uint8_t** dest_p, uint8_t* src, int size) {
    uint8_t* dest = (*dest_p);
    memcpy(dest, src, size);
    (*dest_p) += size;
}

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

    memcpy_id(buffer, pallete, 0x40);
    memcpy_id(buffer, sprite_pallete, 0x40);
    memcpy_id(buffer, gameboy_ram, GAMEBOY_RAM_SIZE);
    memcpy_id(buffer, gameboy_vram, GAMEBOY_VRAM_SIZE);
    memcpy_id(buffer, gameboy_oam, GAMEBOY_OAM_SIZE);
    memcpy_id(buffer, gameboy_bios, GAMEBOY_BIOS_SIZE);
    memcpy_id(buffer, zero_page, 0x7F);
    memcpy_id(buffer, &interrupt_enable, 1);
    memcpy_id(buffer, hardware_registers, 0x80);
    memcpy_id(buffer, &IME, 1);
    memcpy_id(buffer, mbc_control, 4);

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

    memcpy_is(pallete, &buffer, 0x40);
    memcpy_is(sprite_pallete, &buffer, 0x40);
    memcpy_is(gameboy_ram, &buffer, GAMEBOY_RAM_SIZE);
    memcpy_is(gameboy_vram, &buffer, GAMEBOY_VRAM_SIZE);
    memcpy_is(gameboy_oam, &buffer, GAMEBOY_OAM_SIZE);
    memcpy_is(gameboy_bios, &buffer, GAMEBOY_BIOS_SIZE);
    memcpy_is(zero_page, &buffer, 0x7F);
    memcpy_is(&interrupt_enable, &buffer, 1);
    memcpy_is(hardware_registers, &buffer, 0x80);
    memcpy_is(&IME, &buffer, 1);
    memcpy_is(mbc_control, &buffer, 4);
 
    set_cpu_state(state);

    update_all_selected_banks(); 
}

