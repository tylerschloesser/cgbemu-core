#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

#include "globals.h"

void open_log();
void close_log();
void write_log( const char* format, ... );

extern char opcode[0x100][16];
extern char opcodeCB[0x100][16];

#endif /* DEBUG_H_INCLUDED */
