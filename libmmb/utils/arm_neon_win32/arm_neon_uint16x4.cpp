#include "arm_neon_uint16x4.hpp"
#include "arm_neon_uint16x8.hpp"
#include "arm_neon_uint32x4.hpp"

uint16x4_t::uint16x4_t() {
    
    MMP_S32 i;
    for(i = 0; i < SIZE; i++) {
        m_d[i] = 0;
    }
}

uint16x4_t::uint16x4_t(MMP_U16 *pu16__) {

    MMP_S32 i;
    for(i = 0; i < SIZE; i++) {
        m_d[i] = pu16__[i];
    }
}

uint16x4_t::uint16x4_t(uint16x8_t &mat16x8, enum ARM_NEON_HL neon_hl) {
    
    MMP_S32 i;
    MMP_S32 sidx;

    if(neon_hl == ARM_NEON_LOW) {
        sidx = 0;
    }
    else {
        sidx = 4;
    }

    for(i = 0; i < SIZE; i++) {
        m_d[i] = mat16x8.get(sidx+i);
    }

}

uint16x4_t::uint16x4_t(uint32x4_t &mat32x4) {

    MMP_S32 i;
    MMP_U16 u16__;
    MMP_U32 u32__;

    for(i = 0; i < SIZE; i++) {
        u32__ = mat32x4.get(i);
        u16__ = (MMP_U16)u32__;
        m_d[i] = u16__;
    }
}

