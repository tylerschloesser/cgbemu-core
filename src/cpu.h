#ifndef CPU_H_INCLUDED
#define CPU_H_INCLUDED

#include "globals.h"

/*
//remember to change this, maybe a separate debugger file?
#include "../gui/debugger.h"
*/

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

typedef enum {
	STOPPED,
	STARTED,
	PAUSED,
	EM_INVALID,
} EmulatorState;
void change_emulator_state( EmulatorState new_state );

//TEMPORARY
extern bool output_opcodes;
extern bool fullspeed;

/* 
 * "counter" is intentionally misspelled until the temporary
 *  cpu state definitions are removed
 */
void cpu_set_timer_countr( int timer_countr );

CpuState get_cpu_state();
void set_cpu_state( CpuState cpu_state );

void reinitialize_cpu();
void initialize_cpu();

void emulate_cpu();

void start_cpu();
void stop_cpu();
void pause_cpu();

/*
void cpu_attach_debugger( Debugger* );
Debugger *cpu_detach_debugger( void ); 
*/

#endif /* CPU_H_INCLUDED */
