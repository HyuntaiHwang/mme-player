#ifndef ARM_NEON_UINT16X4_HPP__
#define ARM_NEON_UINT16X4_HPP__

#include "arm_neon_define.h"

class uint16x8_t;
class uint32x4_t;

class uint16x4_t {

public:
    enum {
        BIT = 16,
        SIZE = 4
    };

private:
    MMP_U16 m_d[SIZE];

public:
    uint16x4_t();
    uint16x4_t(MMP_U16 *pu16__);
    uint16x4_t(uint16x8_t &mat16x8, enum ARM_NEON_HL neon_hl);
    uint16x4_t(uint32x4_t &mat32x4);
    
    inline MMP_U16 get(MMP_S32 idx) { return m_d[idx]; }
};

#endif
