#ifndef ARM_NEON_UINT32X4_HPP__
#define ARM_NEON_UINT32X4_HPP__

#include "arm_neon_define.h"

class uint16x4_t;

class uint32x4_t {

public:
    enum {
        BIT = 32,
        SIZE = 4
    };

private:
    MMP_U32 m_d[SIZE];

public:
    uint32x4_t();
    uint32x4_t(MMP_U32 *p);
    uint32x4_t(uint16x4_t &mat16x4);
    uint32x4_t(MMP_U32 u32__);

    inline MMP_U32 get(MMP_S32 idx) { return m_d[idx]; }

    friend uint32x4_t operator*(uint32x4_t& a, uint32x4_t& b);

    uint32x4_t& operator+=(uint32x4_t &mat32x4);
    uint32x4_t& operator-=(uint32x4_t &mat32x4);
    uint32x4_t& operator>>=(uint32x4_t &mat32x4);
    uint32x4_t& operator+=(MMP_U32 value);
    uint32x4_t& operator>>=(MMP_U32 value);

};

#endif
