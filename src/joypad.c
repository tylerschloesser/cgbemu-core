#include <assert.h>

#include "joypad.h"
#include "memory.h"
#include "cpu.h"

#include "debug.h"


static u8 joypad_state;
/* 1 = not pressed
 * 0 = pressed
 */
 
static bool initialized = false;

void initialize_joypad() 
{
    
    assert( initialized == false );
    
    joypad_state = 0xFF;
    gb->hw_registers[P1] |= 0xF;
    
    initialized = true;
}

void reinitialize_joypad()
{
    assert( initialized == true );
    initialized = false;
    initialize_joypad();
}


u8 get_joypad_state() {

    assert( initialized == true );

    u8 joypad_select = gb->hw_registers[P1] & 0x30;
    
    if((joypad_select & 0x30) == 0) {
        // invalid setting (select both button and dir keys?)
        return (joypad_select | 0x0F);
    } else if((joypad_select & 0x10) == 0) {
        // direction keys
        return (joypad_select | ((joypad_state >> 0) & 0x0F));
    } else if((joypad_select & 0x20) == 0) {
        // button keys
        return (joypad_select | ((joypad_state >> 4) & 0x0F));
    }
    return 0xFF;
}


void joypad_down(int gb_key) {
  
    assert( initialized );
    assert( gb_key < 8 );
    
    /*
    u8 bit = 1 << gb_key;
    if( joypad_state ^ bit == 0 ) {
        fprintf(stderr,"alreadypressed\n");
        return;
    }
    */
    
    joypad_state &= ~(1 << gb_key);
    
    gb->hw_registers[P1] |= (get_joypad_state() & 0x0F);
    
    //cpu_interrupt(JOYPAD_INTERRUPT);
}

void joypad_up(int gb_key) {

    assert( initialized == true );
    assert(gb_key < 8); 

    joypad_state |= (1 << gb_key);
     
    gb->hw_registers[P1] |= (get_joypad_state() & 0x0F);
}

/**
 * Set the joypad to recognize the joypad buttons (A, B, start, select)
 */
void joypad_select_button_keys() {

    assert( initialized == true );
    
    // clear last 4 bits
    //gb->hw_registers[P1] &= 0xF0;
    gb->hw_registers[P1] = ~0x20;
    // update the joypad register with the current state of the buttons
    gb->hw_registers[P1] |= ((joypad_state >> 4) & 0x0F);

}

/**
 * Set the joypad to recognize the joypad direction keys (down, up, left, right)
 */
void joypad_select_direction_keys() {

    assert( initialized == true );
    
    // clear the last 4 bits
    //gb->hw_registers[P1] &= 0xF0;
    gb->hw_registers[P1] = ~0x10;
    
    gb->hw_registers[P1] |= ((joypad_state >> 0) & 0x0F);  
}
