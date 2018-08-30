#include "arm_neon_uint16x8.hpp"
#include "arm_neon_uint8x8.hpp"
#include "arm_neon_uint16x4.hpp"

uint16x8_t::uint16x8_t() {
    MMP_S32 i;
    for(i = 0; i < SIZE; i++) {
        m_d[i] = 0;
    }
}

uint16x8_t::uint16x8_t(MMP_U16 *p) {

    MMP_S32 i;
    for(i = 0; i < SIZE; i++) {
        m_d[i] = p[i];
    }
}

uint16x8_t::uint16x8_t(uint8x8_t &mat8x8) {

    MMP_S32 i;
    MMP_U8 u8__;
    MMP_U16 u16__;

    for(i = 0; i < SIZE; i++) {
        u8__ = mat8x8.get(i);
        u16__ = ((MMP_U16)u8__)&0x00FF;
        
        m_d[i] = u16__;
    }
}

uint16x8_t::uint16x8_t(MMP_U16 u16__) {

    MMP_S32 i;
    for(i = 0; i < SIZE; i++) {
        m_d[i] = u16__;
    }
}

uint16x8_t::uint16x8_t(uint16x4_t &mat16x4_low, uint16x4_t &mat16x4_high) {

    MMP_S32 i;
    MMP_U16 u16__;

    for(i = 0; i < SIZE/2; i++) {
        u16__ = mat16x4_low.get(i);
        m_d[i] = u16__;
    }

    for(i = 0; i < SIZE/2; i++) {
        u16__ = mat16x4_high.get(i);
        m_d[4+i] = u16__;
    }

}

uint16x8_t operator*(uint16x8_t& a, uint16x8_t& b) {

    MMP_S32 i;
    uint16x8_t c;

    for(i = 0; i < uint16x8_t::SIZE; i++) {
        c.m_d[i] = a.m_d[i] * b.m_d[i];
    }
    
    return c;
}

uint16x8_t& uint16x8_t::operator+=(uint16x8_t &mat16x8) {
    
    MMP_S32 i;
    for(i = 0; i < SIZE; i++) {
        m_d[i] += mat16x8.get(i);
    }

    return *this;
}

uint16x8_t& uint16x8_t::operator-=(uint16x8_t &mat16x8){
    
    MMP_S32 i;
    for(i = 0; i < SIZE; i++) {
        m_d[i] -= mat16x8.get(i);
    }

    return *this;
}

uint16x8_t& uint16x8_t::operator*=(uint16x8_t &mat16x8){
    
    MMP_S32 i;
    for(i = 0; i < SIZE; i++) {
        m_d[i] *= mat16x8.get(i);
    }

    return *this;
}

uint16x8_t& uint16x8_t::operator>>=(uint16x8_t &mat16x8){
    
    MMP_S32 i;
    for(i = 0; i < SIZE; i++) {
        m_d[i] >>= mat16x8.get(i);
    }

    return *this;
}

uint16x8_t& uint16x8_t::operator+=(MMP_U16 value){
    
    MMP_S32 i;
    for(i = 0; i < SIZE; i++) {
        m_d[i] += value;
    }

    return *this;
}

uint16x8_t& uint16x8_t::operator*=(MMP_U16 value){
    
    MMP_S32 i;
    for(i = 0; i < SIZE; i++) {
        m_d[i] *= value;
    }

    return *this;
}

uint16x8_t& uint16x8_t::operator>>=(MMP_S32 value){
    
    MMP_S32 i;
    for(i = 0; i < SIZE; i++) {
        m_d[i] >>= value;
    }

    return *this;
}

