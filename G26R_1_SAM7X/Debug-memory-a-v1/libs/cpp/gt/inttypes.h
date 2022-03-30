#ifndef GT__INTTYPES_H__INCLUDED
#define GT__INTTYPES_H__INCLUDED

typedef signed char schar_t;
typedef unsigned char uchar_t;
typedef unsigned short ushort_t;
typedef unsigned int uint_t;
typedef unsigned long ulong_t;

typedef schar_t int8_t;
typedef short int16_t;
typedef int int32_t;

typedef uchar_t uint8_t;
typedef ushort_t uint16_t;
typedef uint_t uint32_t;

//typedef uint32_t bool;
#define false ((uint32_t)(0))
#define true  ((uint32_t)(1))

#endif//GT__INTTYPES_H__INCLUDED
