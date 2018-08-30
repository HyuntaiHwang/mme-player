#include "arm_neon_class.hpp"


/*************************************************************
Load/Save/Get
*************************************************************/
uint8x8_t vld1_u8(MMP_U8*__p0) {

    uint8x8_t mat8x8(__p0);
    return mat8x8;
}

uint8x8x4_t vld4_u8(MMP_U8* p) {

    uint8x8x4_t mat8x8x4;
    MMP_S32 i;

    for(i = 0; i < 8; i++) {
        mat8x8x4.val[0].set(i, *p); p++;
        mat8x8x4.val[1].set(i, *p); p++;
        mat8x8x4.val[2].set(i, *p); p++;
        mat8x8x4.val[3].set(i, *p); p++;
    }

    return mat8x8x4;
}

void vst1_u8(MMP_U8* p, uint8x8_t &mat8x8) {
    MMP_S32 i;
    for(i = 0; i < uint8x8_t::SIZE; i++) {
        p[i] = mat8x8.get(i);
    }
}

uint16x4_t vget_low_u16(uint16x8_t &mat16x8) {
    uint16x4_t mat16x4(mat16x8, ARM_NEON_LOW);
    return mat16x4;
}

uint16x4_t vget_high_u16(uint16x8_t &mat16x8) {
    uint16x4_t mat16x4(mat16x8, ARM_NEON_HIGH);
    return mat16x4;
}

MMP_U8 vget_lane_u8(uint8x8_t &mat8x8, MMP_S32 lane) {
    MMP_U8 u8__;
    u8__ = mat8x8.get(lane);
    return u8__;
}

/*************************************************************
Constant Load
*************************************************************/

uint16x8_t vdupq_n_u16(MMP_U16 u16__) {
    uint16x8_t mat16x8(u16__);
    return mat16x8;
}

uint32x4_t vdupq_n_u32(MMP_U32 u32__) {
    uint32x4_t mat32x4(u32__);
    return mat32x4;
}

/*************************************************************
Lane Extention
*************************************************************/

uint16x8_t vmovl_u8(uint8x8_t &mat8x8) {
    uint16x8_t mat16x8(mat8x8);
    return mat16x8;
}

uint32x4_t vmovl_u16(uint16x4_t &max16x4) {
    uint32x4_t mat32x4(max16x4);
    return mat32x4;
}

uint8x8_t  vqmovn_u16(uint16x8_t &mat16x8) {
    uint8x8_t mat8x8(mat16x8);
    return mat8x8;
}

uint16x4_t vqmovn_u32(uint32x4_t &mat32x4) {
    uint16x4_t mat16x4(mat32x4);
    return mat16x4;
}

uint16x8_t vcombine_u16(uint16x4_t &mat16x4_low, uint16x4_t &mat16x4_high) {
    uint16x8_t mat16x8(mat16x4_low, mat16x4_high);
    return mat16x8;
}

/*************************************************************
Basic Operator  +,-,/, x
*************************************************************/

uint16x8_t vmulq_u16(uint16x8_t &a, uint16x8_t &b) {
    
    uint16x8_t c;
    c = a * b;
    return c;
}