#ifndef ESCX_ROM_H
#define ESCX_ROM_H
#include "DataTypes.h"

/* Define ESCX_PLATFORM_AVR=1 for classic AVR Harvard-memory targets. */
#if defined(ESCX_PLATFORM_AVR) && (ESCX_PLATFORM_AVR != 0)
#include <avr/pgmspace.h>
#define ESCX_ROM PROGMEM
#define ESCX_ROM_READ_U8(address) ((ESCX_BYTE_T)pgm_read_byte(address))
#define ESCX_ROM_READ_U16(address) ((ESCX_UINT16_T)pgm_read_word(address))
#define ESCX_ROM_READ_PTR(address) ((void *)pgm_read_ptr(address))
#else
#define ESCX_ROM
#define ESCX_ROM_READ_U8(address) (*(address))
#define ESCX_ROM_READ_U16(address) (*(address))
#define ESCX_ROM_READ_PTR(address) (*(address))
#endif
#endif
