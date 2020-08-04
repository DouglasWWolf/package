//============================================================================
// typedefs.h - Creates convenient typedefs for all of the integer types
//
//============================================================================
#pragma once
#include <stdint.h>

typedef int8_t    s8;
typedef int16_t   s16;
typedef int32_t   s32;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef   signed long long s64;
typedef unsigned long long u64;


//============================================================================
// This is a u16, stored in big-endian order
//============================================================================
class u16be
{
public:

    u16be& operator=(u16 value)
    {
        m_octet[0] = (value >> 8);
        m_octet[1] = (value & 0xFF);
        return *this;
    }

    operator u16()
    {
        return m_octet[0] << 8 | m_octet[1];
    }


    u8  m_octet[2];
};
//============================================================================


//============================================================================
// This is a u32, stored in big-endian order
//============================================================================
class u32be
{
public:

    u32be& operator=(u32 value)
    {
        m_octet[0] = (value >> 24);
        m_octet[1] = (value >> 16);
        m_octet[2] = (value >>  8);
        m_octet[3] = (value      );
        return *this;
    }

    operator u32()
    {
        return m_octet[0] << 24
             | m_octet[1] << 16
             | m_octet[2] <<  8
             | m_octet[3];
    }

    u8  m_octet[4];
};
//============================================================================



