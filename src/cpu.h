#ifndef CPU_H_INCLUDED
#define CPU_H_INCLUDED

#include "globals.h"

/*
//remember to change this, maybe a separate debugger file?
#include "../gui/debugger.h"
*/
//TEMP
extern bool cpu_step;
extern bool enable_breakpoints;
typedef union
{
    u16 W;
    struct
    {
        u8 L, H;
    } B;
} Z80Register;

typedef struct {
    bool running;
    bool waiting_for_interrupt;

    u16 hdma_source;
    u16 hdma_destination;
    u16 hdma_transfer_length;

    bool hdma_active;
    
    s32 timer_counter;
    s32 scanline_counter;
    
    u32 cycles;
    u32 mode_cycles;
    u32 divider_counter;
    
    Z80Register PC;
    Z80Register SP;
    Z80Register BC;
    Z80Register DE;
    Z80Register HL;
    Z80Register AF; 
    Z80Register IR;
    
} CpuState;
//TODO temp
void print_cpu_state();
extern CpuState cpu_state;
typedef enum {
    STOPPED,
    STARTED,
    PAUSED,
    EM_INVALID,
} EmulatorState;
void change_emulator_state( EmulatorState new_state );


//TEMP AND STUPID
#define hdma_source cpu_state.hdma_source
#define hdma_destination cpu_state.hdma_destination
#define hdma_transfer_length cpu_state.hdma_transfer_length
#define hdma_active cpu_state.hdma_active


//TEMPORARY
extern bool fullspeed;

/* 
 * "counter" is intentionally misspelled until the temporary
 *  cpu state definitions are removed
 */
void cpu_set_timer_countr( int timer_countr );

CpuState get_cpu_state();
void set_cpu_state( CpuState cpu_state );

void initialize_cpu(bool use_bios);

void emulate_cpu();

void start_cpu();
void stop_cpu();
void pause_cpu();

#endif /* CPU_H_INCLUDED */
