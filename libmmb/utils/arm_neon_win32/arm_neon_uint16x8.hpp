#ifndef ARM_NEON_UINT16X8_HPP__
#define ARM_NEON_UINT16X8_HPP__

#include "arm_neon_define.h"

class uint8x8_t;
class uint16x4_t;

class uint16x8_t {

public:
    enum {
        BIT = 16,
        SIZE = 8
    };

private:
    MMP_U16 m_d[SIZE];

public:
    uint16x8_t();
    uint16x8_t(MMP_U16 *p);
    uint16x8_t(uint8x8_t &mat8x8);
    uint16x8_t(MMP_U16 u16__);
    uint16x8_t(uint16x4_t &mat16x4_low, uint16x4_t &mat16x4_high);

    inline MMP_U16 get(MMP_S32 idx) { return m_d[idx]; }
    
    friend uint16x8_t operator*(uint16x8_t& a, uint16x8_t& b);

    uint16x8_t& operator+=(uint16x8_t &mat16x8);
    uint16x8_t& operator-=(uint16x8_t &mat16x8);
    uint16x8_t& operator*=(uint16x8_t &mat16x8);
    uint16x8_t& operator>>=(uint16x8_t &mat16x8);
    uint16x8_t& operator+=(MMP_U16 value);
    uint16x8_t& operator*=(MMP_U16 value);
    uint16x8_t& operator>>=(MMP_S32 value);

};

#endif
