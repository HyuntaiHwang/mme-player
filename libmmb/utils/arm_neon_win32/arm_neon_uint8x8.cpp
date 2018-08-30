#include "arm_neon_uint8x8.hpp"
#include "arm_neon_uint16x8.hpp"

uint8x8_t::uint8x8_t() {
    MMP_S32 i;
    for(i = 0; i < SIZE; i++) {
        m_d[i] = 0;
    }
}

uint8x8_t::uint8x8_t(MMP_U8 *p) {

    MMP_S32 i;
    for(i = 0; i < SIZE; i++) {
        m_d[i] = p[i];
    }
}

uint8x8_t::uint8x8_t(uint16x8_t &mat16x8) {

    MMP_S32 i;
    MMP_U16 u16__;
    MMP_U8 u8__;

    for(i = 0; i < SIZE; i++) {
        u16__ = mat16x8.get(i);
        u8__ = (MMP_U8)u16__;
        m_d[i] = u8__;
    }
}
