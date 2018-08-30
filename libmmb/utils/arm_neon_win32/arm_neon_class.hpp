#ifndef ARM_NEON_CLASS_H__
#define ARM_NEON_CLASS_H__




/****************************************
D Register (64bit)
****************************************/
#include "arm_neon_uint8x8.hpp"
#include "arm_neon_uint16x4.hpp"

/****************************************
Q Register 
****************************************/
#include "arm_neon_uint16x8.hpp"
#include "arm_neon_uint32x4.hpp"


uint8x8_t vld1_u8(MMP_U8*__p0);
uint8x8x4_t vld4_u8(MMP_U8* p);

void vst1_u8(MMP_U8* p, uint8x8_t &mat8x8);

uint16x4_t vget_low_u16(uint16x8_t &mat16x8);
uint16x4_t vget_high_u16(uint16x8_t &mat16x8);
MMP_U8 vget_lane_u8(uint8x8_t &mat8x8, MMP_S32 lane); 

uint16x8_t vmovl_u8(uint8x8_t &mat8x8);
uint32x4_t vmovl_u16(uint16x4_t &max16x4);
uint8x8_t  vqmovn_u16(uint16x8_t &mat16x8);
uint16x4_t vqmovn_u32(uint32x4_t &mat32x4);
uint16x8_t vcombine_u16(uint16x4_t &mat16x4_low, uint16x4_t &mat16x4_high);

uint16x8_t vdupq_n_u16(MMP_U16 c);
uint16x8_t vmulq_u16(uint16x8_t &a, uint16x8_t &b);

uint32x4_t vdupq_n_u32(MMP_U32 c);

#endif



