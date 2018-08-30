#include "arm_neon_uint32x4.hpp"
#include "arm_neon_uint16x4.hpp"

uint32x4_t::uint32x4_t() {
    MMP_S32 i;
    for(i = 0; i < SIZE; i++) {
        m_d[i] = 0;
    }
}

uint32x4_t::uint32x4_t(MMP_U32 *p) {

    MMP_S32 i;
    for(i = 0; i < SIZE; i++) {
        m_d[i] = p[i];
    }
}

uint32x4_t::uint32x4_t(uint16x4_t &mat16x4) {

    MMP_S32 i;
    MMP_U16 u16__;
    MMP_U32 u32__;

    for(i = 0; i < SIZE; i++) {
        u16__ = mat16x4.get(i);
        u32__ = ((MMP_U32)u16__)&0xFFFF;
        m_d[i] = u32__;
    }
}

uint32x4_t::uint32x4_t(MMP_U32 u32__) {

    MMP_S32 i;
    for(i = 0; i < SIZE; i++) {
        m_d[i] = u32__;
    }
}

uint32x4_t& uint32x4_t::operator+=(uint32x4_t &mat32x4) {
    MMP_S32 i;
    for(i = 0; i < SIZE; i++) {
        m_d[i] += mat32x4.get(i);
    }
    return *this; 
}

uint32x4_t& uint32x4_t::operator-=(uint32x4_t &mat32x4) {
    MMP_S32 i;
    for(i = 0; i < SIZE; i++) {
        m_d[i] -= mat32x4.get(i);
    }
    return *this; 
}

uint32x4_t& uint32x4_t::operator>>=(uint32x4_t &mat32x4) {
    MMP_S32 i;
    for(i = 0; i < SIZE; i++) {
        m_d[i] >>= mat32x4.get(i);
    }
    return *this; 
}

uint32x4_t& uint32x4_t::operator+=(MMP_U32 value) {
    MMP_S32 i;
    for(i = 0; i < SIZE; i++) {
        m_d[i] += value;
    }
    return *this; 
}

uint32x4_t& uint32x4_t::operator>>=(MMP_U32 value) {
    MMP_S32 i;
    for(i = 0; i < SIZE; i++) {
        m_d[i] >>= value;
    }
    return *this; 
}

uint32x4_t operator*(uint32x4_t& a, uint32x4_t& b) {

    MMP_S32 i;
    uint32x4_t c;

    for(i = 0; i < uint32x4_t::SIZE; i++) {
        c.m_d[i] = a.m_d[i] * b.m_d[i];
    }
    
    return c;
}

