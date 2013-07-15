#ifndef GLOBALS_H_INCLUDED
#define GLOBALS_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#ifdef _WIN32
#define sleep Sleep
#endif

#define DEBUG

typedef signed char     s8;
typedef signed short    s16;
typedef signed int      s32;
typedef uint8_t         u8;
typedef uint16_t        u16;
typedef uint32_t        u32;
typedef uint64_t        u64;

#endif /* GLOBALS_H_INCLUDED */
