#ifndef ARM_NEON_UINT8X8_HPP__
#define ARM_NEON_UINT8X8_HPP__

#include "arm_neon_define.h"

class uint16x8_t;
class uint8x8_t {

public:
    enum {
        BIT = 8,
        SIZE = 8
    };

private:
    MMP_U8 m_d[SIZE];

public:
    uint8x8_t();
    uint8x8_t(MMP_U8 *p);
    uint8x8_t(uint16x8_t &mat16x8);

    inline MMP_U8 get(MMP_S32 idx) { return m_d[idx]; }
    inline void set(MMP_S32 idx, MMP_U8 v) { m_d[idx] = v; }
};

class uint8x8x4_t {
public:
    uint8x8_t val[4];
};

#endif
