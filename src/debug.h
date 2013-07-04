#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

#include <stdbool.h>

#include "globals.h"
#include "memory.h"
#include "cpu.h"

#define DEBUG_MBC_READ 0
#define DEBUG_MBC_WRITE 1

#define DEBUG

extern bool debug_console_opened;

void display_cpu_values();

void display_debug_info();

void open_debug_console();
void close_debug_console();

#ifdef _WIN32
void dprintf( const char* format, ... );
#endif

void open_log();
void close_log();
void write_log( const char* format, ... );

extern char opcode[0x100][16];
extern char opcodeCB[0x100][16];

#endif /* DEBUG_H_INCLUDED */
