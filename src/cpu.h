#ifndef CPU_H_INCLUDED
#define CPU_H_INCLUDED

#include "globals.h"

typedef union
{
    uint16_t W;
    struct
    {
        uint8_t L, H;
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
    
} Cpu;

extern Cpu* cpu;

//TODO temp
void print_cpu_state();

//TEMP AND STUPID
#define hdma_source cpu->hdma_source
#define hdma_destination cpu->hdma_destination
#define hdma_transfer_length cpu->hdma_transfer_length
#define hdma_active cpu->hdma_active

void initialize_cpu(bool use_bios);

void emulate_cpu();

#endif /* CPU_H_INCLUDED */
