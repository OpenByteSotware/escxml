#ifndef ESCX_DATA_TYPES_H
#define ESCX_DATA_TYPES_H

#include <limits.h>

#if UCHAR_MAX != 255U
#error ESCXML requires an 8-bit unsigned char
#endif

#if USHRT_MAX != 65535U
#error ESCXML requires a 16-bit unsigned short
#endif

typedef unsigned char ESCX_BOOL_T;
typedef unsigned char ESCX_BYTE_T;
typedef signed char ESCX_SBYTE_T;
typedef unsigned short ESCX_UINT16_T;
typedef signed short ESCX_INT16_T;

#define ESCX_FALSE ((ESCX_BOOL_T)0U)
#define ESCX_TRUE  ((ESCX_BOOL_T)1U)

#endif
