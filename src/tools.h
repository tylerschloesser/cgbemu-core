#ifndef TOOLS_H_INCLUDED
#define TOOLS_H_INCLUDED

#include <stdarg.h>
#include <string.h>
#include <sys/time.h>

#include "globals.h"

extern unsigned int last_error;
extern bool debug_mode;

void tprintf( const char* format, ... );
unsigned int binary_read_file( char* filepath, u8* dest, int max_size );
void fatal_error();
char *get_last_error();

unsigned long get_elapsed_usec( struct timeval time_start, struct timeval time_end );

#endif /* TOOLS_H_INCLUDED */