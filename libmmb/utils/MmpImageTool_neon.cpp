/*
 * Copyright (c) 2014 Anapass Co., Ltd.
 *              http://www.anapass.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "MmpImageTool.hpp"
#include "MmpUtil.hpp"
#include <math.h>
#include <arm_neon.h>

MMP_RESULT CMmpImageTool::ConvertYUV420MtoRGB_neon(MMP_U8* yuv420_y, MMP_U8* yuv420_u, MMP_U8* yuv420_v, 
                                            MMP_S32 y_stride, MMP_S32 u_stride, MMP_S32 v_stride,
                                            MMP_S32 pic_width, MMP_S32 pic_height, 
                                            MMP_U8* rgb, enum MMP_FOURCC fourcc_rgb) {

                                                
    MMP_S32 w, h;
    MMP_S32 r,g,b,y,u,v;

    MMP_U8 *line_y, *line_u, *line_v;
    MMP_U8 *line_rgb;
    
    MMP_U8 *pix_y, *pix_u, *pix_v;
    MMP_U8 *pix_rgb;

    MMP_S32 r_idx, g_idx, b_idx, a_idx;
    MMP_S32 rgb_pix_width;
    MMP_S32 rgb_stride;

    switch(fourcc_rgb) {
        case MMP_FOURCC_IMAGE_RGB888:  a_idx=-1, r_idx=2, g_idx=1, b_idx=0; rgb_pix_width=3; break;
        case MMP_FOURCC_IMAGE_BGR888:  a_idx=-1, r_idx=0, g_idx=1, b_idx=2; rgb_pix_width=3; break;
        
        case MMP_FOURCC_IMAGE_ARGB8888:  a_idx=3, r_idx=2, g_idx=1, b_idx=0; rgb_pix_width=4; break;
        case MMP_FOURCC_IMAGE_ABGR8888:  a_idx=3, r_idx=0, g_idx=1, b_idx=2; rgb_pix_width=4; break;

        default:
            return MMP_FAILURE;
    }
    rgb_stride = MMP_BYTE_ALIGN(pic_width*rgb_pix_width,4); 
    line_y = yuv420_y;
    line_u = yuv420_u;
    line_v = yuv420_v;
    line_rgb = rgb;

    for(h = 0; h < pic_height;h++) {
        
        pix_y = line_y;
        pix_u = line_u;
        pix_v = line_v;

        pix_rgb = line_rgb;
        
        for(w = 0; w < pic_width; w+=8 ) {

             y=(int)(*pix_y); y&=0xff;
             u=(int)(*pix_u); u&=0xff;  u-=128;
             v=(int)(*pix_v); v&=0xff;  v-=128;
         
             r=y+(91881*u)/65536;
             g=y-((22554*u)/65536)-((46802*v)/65536);
             b=y+(116130*v)/65536;
             if(r<0) r=0; if(r>255) r=255;
             if(g<0) g=0; if(g>255) g=255;
             if(b<0) b=0; if(b>255) b=255;

             if(a_idx>=0) pix_rgb[a_idx]=0xFF;
             pix_rgb[r_idx] = (MMP_U8)r;
             pix_rgb[g_idx] = (MMP_U8)g;
             pix_rgb[b_idx] = (MMP_U8)b;

             pix_rgb+=rgb_pix_width;
             pix_y++;
             if( ((w+1)%2) == 0 ) {
                pix_u++;
                pix_v++;
             }
        }

        line_y += y_stride;
        if( ((h+1)%2) == 0 ) {
            line_u += u_stride;
            line_v += v_stride;
        }
        line_rgb += rgb_stride;
    }

    return MMP_SUCCESS;
}


#if 0

    yuv2rgb_neon.c 

/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include "yuv420sp2rgb.h"
#include <arm_neon.h>
#include <stdlib.h>

/* this source file should only be compiled by Android.mk when targeting
 * the armeabi-v7a ABI, and should be built in NEON mode
 */
void fir_filter_neon_intrinsics(short *output, const short* input, const short* kernel, int width, int kernelSize)
{
#if 1
  int nn, offset = -kernelSize / 2;

  for (nn = 0; nn < width; nn++)
  {
    int mm, sum = 0;
    int32x4_t sum_vec = vdupq_n_s32(0);
    for (mm = 0; mm < kernelSize / 4; mm++)
    {
      int16x4_t kernel_vec = vld1_s16(kernel + mm * 4);
      int16x4_t input_vec = vld1_s16(input + (nn + offset + mm * 4));
      sum_vec = vmlal_s16(sum_vec, kernel_vec, input_vec);
    }

    sum += vgetq_lane_s32(sum_vec, 0);
    sum += vgetq_lane_s32(sum_vec, 1);
    sum += vgetq_lane_s32(sum_vec, 2);
    sum += vgetq_lane_s32(sum_vec, 3);

    if (kernelSize & 3)
    {
      for (mm = kernelSize - (kernelSize & 3); mm < kernelSize; mm++)
        sum += kernel[mm] * input[nn + offset + mm];
    }

    output[nn] = (short)((sum + 0x8000) >> 16);
  }
#else /* for comparison purposes only */
  int nn, offset = -kernelSize/2;
  for (nn = 0; nn < width; nn++)
  {
    int sum = 0;
    int mm;
    for (mm = 0; mm < kernelSize; mm++)
    {
      sum += kernel[mm]*input[nn+offset+mm];
    }
    output[n] = (short)((sum + 0x8000) >> 16);
  }
#endif
}

/*
 YUV 4:2:0 image with a plane of 8 bit Y samples followed by an interleaved
 U/V plane containing 8 bit 2x2 subsampled chroma samples.
 except the interleave order of U and V is reversed.

 H V
 Y Sample Period      1 1
 U (Cb) Sample Period 2 2
 V (Cr) Sample Period 2 2
 */

/*
 size of a char:
 find . -name limits.h -exec grep CHAR_BIT {} \;
 */

#ifndef max
#define max(a,b) ({typeof(a) _a = (a); typeof(b) _b = (b); _a > _b ? _a : _b; })
#define min(a,b) ({typeof(a) _a = (a); typeof(b) _b = (b); _a < _b ? _a : _b; })
#endif

#define bytes_per_pixel 2
#define LOAD_Y(i,j) (pY + i * width + j)
#define LOAD_V(i,j) (pUV + (i / 2) * width + bytes_per_pixel * (j / 2))
#define LOAD_U(i,j) (pUV + (i / 2) * width + bytes_per_pixel * (j / 2)+1)

const uint8_t ZEROS[8] = {220,220, 220, 220, 220, 220, 220, 220};
const uint8_t Y_SUBS[8] = {16, 16, 16, 16, 16, 16, 16, 16};
const uint8_t UV_SUBS[8] = {128, 128, 128, 128, 128, 128, 128, 128};

const uint32_t UV_MULS[] = {833, 400, 833, 400};

void color_convert_common(unsigned char *pY, unsigned char *pUV, int width, int height, unsigned char *buffer, int grey)
{

  int i, j;
  int nR, nG, nB;
  int nY, nU, nV;
  unsigned char *out = buffer;
  int offset = 0;

  uint8x8_t Y_SUBvec = vld1_u8(Y_SUBS);
  uint8x8_t UV_SUBvec = vld1_u8(UV_SUBS); // v,u,v,u v,u,v,u
  uint32x4_t UV_MULSvec = vld1q_u32(UV_MULS);
  uint8x8_t ZEROSvec =vld1_u8(ZEROS);

  uint32_t UVvec_int[8];
  if (grey)
  {
    memcpy(out, pY, width * height * sizeof(unsigned char));
  }
  else
    // YUV 4:2:0
    for (i = 0; i < height; i++)
    {
      for (j = 0; j < width; j += 8)
      {
        //        nY = *(pY + i * width + j);
        //        nV = *(pUV + (i / 2) * width + bytes_per_pixel * (j / 2));
        //        nU = *(pUV + (i / 2) * width + bytes_per_pixel * (j / 2) + 1);

        uint8x8_t nYvec = vld1_u8(LOAD_Y(i,j));
        uint8x8_t nUVvec = vld1_u8(LOAD_V(i,j)); // v,u,v,u v,u,v,u

        nYvec = vmul_u8(nYvec, vcle_u8(nYvec,ZEROSvec));

        // Yuv Convert
        //        nY -= 16;
        //        nU -= 128;
        //        nV -= 128;

        //        nYvec = vsub_u8(nYvec, Y_SUBvec);
        //        nUVvec = vsub_u8(nYvec, UV_SUBvec);

        uint16x8_t nYvec16 = vmovl_u8(vsub_u8(nYvec, Y_SUBvec));
        uint16x8_t nUVvec16 = vmovl_u8(vsub_u8(nYvec, UV_SUBvec));

        uint16x4_t Y_low4 = vget_low_u16(nYvec16);
        uint16x4_t Y_high4 = vget_high_u16(nYvec16);
        uint16x4_t UV_low4 = vget_low_u16(nUVvec16);
        uint16x4_t UV_high4 = vget_high_u16(nUVvec16);

        uint32x4_t UV_low4_int = vmovl_u16(UV_low4);
        uint32x4_t UV_high4_int = vmovl_u16(UV_high4);

        uint32x4_t Y_low4_int = vmull_n_u16(Y_low4, 1192);
        uint32x4_t Y_high4_int = vmull_n_u16(Y_high4, 1192);

        uint32x4x2_t UV_uzp = vuzpq_u32(UV_low4_int, UV_high4_int);

        uint32x2_t Vl = vget_low_u32(UV_uzp.val[0]);// vld1_u32(UVvec_int);
        uint32x2_t Vh = vget_high_u32(UV_uzp.val[0]);//vld1_u32(UVvec_int + 2);

        uint32x2x2_t Vll_ = vzip_u32(Vl, Vl);
        uint32x4_t* Vll = (uint32x4_t*)(&Vll_);

        uint32x2x2_t Vhh_ = vzip_u32(Vh, Vh);
        uint32x4_t* Vhh = (uint32x4_t*)(&Vhh_);

        uint32x2_t Ul =  vget_low_u32(UV_uzp.val[1]);
        uint32x2_t Uh =  vget_high_u32(UV_uzp.val[1]);

        uint32x2x2_t Ull_ = vzip_u32(Ul, Ul);
        uint32x4_t* Ull = (uint32x4_t*)(&Ull_);

        uint32x2x2_t Uhh_ = vzip_u32(Uh, Uh);
        uint32x4_t* Uhh = (uint32x4_t*)(&Uhh_);

        uint32x4_t B_int_low = vmlaq_n_u32(Y_low4_int, *Ull, 2066); //multiply by scalar accum
        uint32x4_t B_int_high = vmlaq_n_u32(Y_high4_int, *Uhh, 2066); //multiply by scalar accum
        uint32x4_t G_int_low = vsubq_u32(Y_low4_int, vmlaq_n_u32(vmulq_n_u32(*Vll, 833), *Ull, 400));
        uint32x4_t G_int_high = vsubq_u32(Y_high4_int, vmlaq_n_u32(vmulq_n_u32(*Vhh, 833), *Uhh, 400));
        uint32x4_t R_int_low = vmlaq_n_u32(Y_low4_int, *Vll, 1634); //multiply by scalar accum
        uint32x4_t R_int_high = vmlaq_n_u32(Y_high4_int, *Vhh, 1634); //multiply by scalar accum

        B_int_low = vshrq_n_u32 (B_int_low, 10);
        B_int_high = vshrq_n_u32 (B_int_high, 10);
        G_int_low = vshrq_n_u32 (G_int_low, 10);
        G_int_high = vshrq_n_u32 (G_int_high, 10);
        R_int_low = vshrq_n_u32 (R_int_low, 10);
        R_int_high = vshrq_n_u32 (R_int_high, 10);


        uint8x8x3_t RGB;
        RGB.val[0] = vmovn_u16(vcombine_u16(vqmovn_u32 (R_int_low),vqmovn_u32 (R_int_high)));
        RGB.val[1] = vmovn_u16(vcombine_u16(vqmovn_u32 (G_int_low),vqmovn_u32 (G_int_high)));
        RGB.val[2] = vmovn_u16(vcombine_u16(vqmovn_u32 (B_int_low),vqmovn_u32 (B_int_high)));

        vst3_u8 (out+i*width*3 + j*3, RGB);
      }
    }

}


#endif


static MMP_RESULT convert_rgb2y_neon(MMP_U8* r, MMP_U8* g, MMP_U8* b, MMP_U8* y) {
    
    uint16x8_t mat16x8_src;
    uint16x8_t mat16x8_const;
    uint16x8_t mat16x8_dst;
    uint32x4_t mat32x4_low;
    uint32x4_t mat32x4_high;
    uint32x4_t mat32x4_dst_low;
    uint32x4_t mat32x4_dst_high;
    uint32x4_t mat32x4_const;
    
    //y= ((66*r[0]+129*g[0]+25*b[0]+128)>>8)+16;

    /*cal r */
    mat16x8_src = vmovl_u8(vld1_u8(r));
    mat16x8_const = vdupq_n_u16(66);
    mat16x8_dst = vmulq_u16(mat16x8_src, mat16x8_const);
    mat32x4_low = vmovl_u16(vget_low_u16(mat16x8_dst));
    mat32x4_high = vmovl_u16(vget_high_u16(mat16x8_dst));

    mat32x4_dst_low = mat32x4_low;
    mat32x4_dst_high = mat32x4_high;

    /*cal g */
    mat16x8_src = vmovl_u8(vld1_u8(g));
    mat16x8_const = vdupq_n_u16(129);
    mat16x8_dst = vmulq_u16(mat16x8_src, mat16x8_const);
    mat32x4_low = vmovl_u16(vget_low_u16(mat16x8_dst));
    mat32x4_high = vmovl_u16(vget_high_u16(mat16x8_dst));

    mat32x4_dst_low += mat32x4_low;
    mat32x4_dst_high += mat32x4_high;

    /*cal b */
    mat16x8_src = vmovl_u8(vld1_u8(b));
    mat16x8_const = vdupq_n_u16(25);
    mat16x8_dst = vmulq_u16(mat16x8_src, mat16x8_const);
    
    mat32x4_low = vmovl_u16(vget_low_u16(mat16x8_dst));
    mat32x4_high = vmovl_u16(vget_high_u16(mat16x8_dst));
        
    mat32x4_const = vdupq_n_u32(128);
    mat32x4_low += mat32x4_const;//128;
    mat32x4_high += mat32x4_const;//128;

    mat32x4_dst_low += mat32x4_low;
    mat32x4_dst_high += mat32x4_high;

    mat32x4_const = vdupq_n_u32(8);
    mat32x4_dst_low >>= mat32x4_const;//8;
    mat32x4_dst_high >>= mat32x4_const;//8;

    mat32x4_const = vdupq_n_u32(16);
    mat32x4_dst_low += mat32x4_const;//16;
    mat32x4_dst_high += mat32x4_const;//16;


    mat16x8_dst = vcombine_u16(vqmovn_u32(mat32x4_dst_low), vqmovn_u32(mat32x4_dst_high) );
    vst1_u8(y, vqmovn_u16(mat16x8_dst));

    return MMP_SUCCESS;
}

/*
         rgb00 rgb01 rgb02 rgb03 rgb04 rgb05 rgb06 rgb07
         rgb10 rgb11 rgb12 rgb13 rgb14 rgb15 rgb16 rgb17
      
         ==> 
             u0 u1 u2 u3   ,  v0  v1 v2 v3 

        u[i]= ((-38*r[i]-74*g[i]+112*b[i]+128)>>8)+128;
        v[i]= ((112*r[i]-94*g[i]-18*b[i]+128)>>8)+128;
*/
static void convert_rgb2uv_neon(MMP_U8* r1, MMP_U8* g1, MMP_U8* b1, 
                                MMP_U8* r2, MMP_U8* g2, MMP_U8* b2, 
                                MMP_U8* pixel_u, MMP_U8* pixel_v) {

    uint16x8_t mat16x8_src;
    uint16x8_t mat16x8_const;
    uint16x8_t mat16x8_dst;
    uint32x4_t mat32x4_low;
    uint32x4_t mat32x4_high;
    uint32x4_t mat32x4_dst_low;
    uint32x4_t mat32x4_dst_high;
    uint32x4_t mat32x4_const;

    MMP_S32 i;
    MMP_U8 d[8];
    
    
    //u[i]= ((-38*r[i]-74*g[i]+112*b[i]+128)>>8)+128;
    /*cal b */
    mat16x8_src = vmovl_u8(vld1_u8(b1));
    mat16x8_const = vdupq_n_u16(112);
    mat16x8_dst = vmulq_u16(mat16x8_src, mat16x8_const);
    mat32x4_low = vmovl_u16(vget_low_u16(mat16x8_dst));
    mat32x4_high = vmovl_u16(vget_high_u16(mat16x8_dst));

    mat32x4_dst_low = mat32x4_low;
    mat32x4_dst_high = mat32x4_high;

    mat32x4_const = vdupq_n_u32(128 + (128<<8));    
    mat32x4_dst_low += mat32x4_const;//128 + (128<<8);
    mat32x4_dst_high += mat32x4_const;//128 + (128<<8);

    /*cal g */
    mat16x8_src = vmovl_u8(vld1_u8(g1));
    mat16x8_const = vdupq_n_u16(74);
    mat16x8_dst = vmulq_u16(mat16x8_src, mat16x8_const);
    mat32x4_low = vmovl_u16(vget_low_u16(mat16x8_dst));
    mat32x4_high = vmovl_u16(vget_high_u16(mat16x8_dst));

    mat32x4_dst_low -= mat32x4_low;
    mat32x4_dst_high -= mat32x4_high;

    /*cal r */
    mat16x8_src = vmovl_u8(vld1_u8(r1));
    mat16x8_const = vdupq_n_u16(38);
    mat16x8_dst = vmulq_u16(mat16x8_src, mat16x8_const);
    
    mat32x4_low = vmovl_u16(vget_low_u16(mat16x8_dst));
    mat32x4_high = vmovl_u16(vget_high_u16(mat16x8_dst));
        
    mat32x4_dst_low -= mat32x4_low;
    mat32x4_dst_high -= mat32x4_high;

    /* devide */
    mat32x4_const = vdupq_n_u32(8);    
    mat32x4_dst_low >>= mat32x4_const;//8;
    mat32x4_dst_high >>= mat32x4_const;//8;

    mat16x8_dst = vcombine_u16(vqmovn_u32(mat32x4_dst_low), vqmovn_u32(mat32x4_dst_high) );
    vst1_u8(d, vqmovn_u16(mat16x8_dst));
    for(i=0; i<4; i++)  pixel_u[i] = d[i*2];
    
#if 1 //V
    //v[i]= ((112*r[i]-94*g[i]-18*b[i]+128)>>8)+128;
    /*cal r */
    mat16x8_src = vmovl_u8(vld1_u8(r1));
    mat16x8_const = vdupq_n_u16(112);
    mat16x8_dst = vmulq_u16(mat16x8_src, mat16x8_const);
    mat32x4_low = vmovl_u16(vget_low_u16(mat16x8_dst));
    mat32x4_high = vmovl_u16(vget_high_u16(mat16x8_dst));

    mat32x4_dst_low = mat32x4_low;
    mat32x4_dst_high = mat32x4_high;
    
    mat32x4_const = vdupq_n_u32(128 + (128<<8));    
    mat32x4_dst_low += mat32x4_const;//128 + (128<<8);
    mat32x4_dst_high += mat32x4_const;//128 + (128<<8);

    /*cal g */
    mat16x8_src = vmovl_u8(vld1_u8(g1));
    mat16x8_const = vdupq_n_u16(94);
    mat16x8_dst = vmulq_u16(mat16x8_src, mat16x8_const);
    mat32x4_low = vmovl_u16(vget_low_u16(mat16x8_dst));
    mat32x4_high = vmovl_u16(vget_high_u16(mat16x8_dst));

    mat32x4_dst_low -= mat32x4_low;
    mat32x4_dst_high -= mat32x4_high;

    /*cal r */
    mat16x8_src = vmovl_u8(vld1_u8(r1));
    mat16x8_const = vdupq_n_u16(18);
    mat16x8_dst = vmulq_u16(mat16x8_src, mat16x8_const);
    
    mat32x4_low = vmovl_u16(vget_low_u16(mat16x8_dst));
    mat32x4_high = vmovl_u16(vget_high_u16(mat16x8_dst));
        
    mat32x4_dst_low -= mat32x4_low;
    mat32x4_dst_high -= mat32x4_high;

    /* devide */
    mat32x4_const = vdupq_n_u32(8);    
    mat32x4_dst_low >>= mat32x4_const;//8;
    mat32x4_dst_high >>= mat32x4_const;//8;

    mat16x8_dst = vcombine_u16(vqmovn_u32(mat32x4_dst_low), vqmovn_u32(mat32x4_dst_high) );
    vst1_u8(d, vqmovn_u16(mat16x8_dst));
    for(i=0; i<4; i++)  pixel_v[i] = d[i*2];
#endif    

}


MMP_RESULT CMmpImageTool::ConvertRGBtoYUV420M_neon(MMP_U8* p_image_rgb, MMP_S32 pic_width, MMP_S32 pic_height,enum MMP_FOURCC fourcc_rgb,
                                                MMP_U8* p_image_y, MMP_U8* p_image_u, MMP_U8* p_image_v, 
                                                MMP_S32 y_stride, MMP_S32 u_stride, MMP_S32 v_stride) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 w, h, i;
    MMP_S32 rgb_stride, rgb_pixel_byte;
    
    MMP_U8 *line_rgb1, *line_rgb2;
    MMP_U8 *pixel_rgb1, *pixel_rgb2;
    
    MMP_U8 *line_y, *line_u, *line_v;
    MMP_U8 *pixel_y, *pixel_u, *pixel_v;

    MMP_U8 r1[8]={0}, g1[8]={0}, b1[8]={0};
    MMP_U8 r2[8]={0}, g2[8]={0}, b2[8]={0};
    
    MMP_S32 pic_width_align;// = MMP_BYTE_ALIGN(pic_width, 8);

    if( (pic_width%8) == 0 ) {
        pic_width_align = pic_width;
    }
    else {
        pic_width_align = MMP_BYTE_ALIGN(pic_width, 8) - 8;
    }

    if(CMmpImageTool::IsRGB(fourcc_rgb) != MMP_TRUE) {
        mmpResult = MMP_FAILURE;
    }
    
    rgb_stride = CMmpImageTool::GetPicStride(fourcc_rgb, pic_width);
    rgb_pixel_byte = CMmpImageTool::GetPixelByte(fourcc_rgb);

    /* convert Y */
    line_rgb1 = p_image_rgb;
    line_rgb2 = line_rgb1 + rgb_stride;

    line_y = p_image_y;
    line_u = p_image_u;
    line_v = p_image_v;

    for(h = 0; h < pic_height; h++) {
        
        pixel_rgb1 = line_rgb1;
        pixel_rgb2 = line_rgb2;
            
        pixel_y = line_y;
        pixel_u = line_u;
        pixel_v = line_v;
            
        for(w = 0; w < pic_width_align; w+=8 ) {
                
            for(i = 0; i < 8; i++) {
                switch(fourcc_rgb) {

                    case MMP_FOURCC_IMAGE_ABGR8888: /* Byte0 => R */
                    case MMP_FOURCC_IMAGE_BGR888: /* Byte0 => R */
                        r1[i] = pixel_rgb1[rgb_pixel_byte*i + 0];
                        g1[i] = pixel_rgb1[rgb_pixel_byte*i + 1];
                        b1[i] = pixel_rgb1[rgb_pixel_byte*i + 2];
                        if((h%2)==0) {
                            r2[i] = pixel_rgb2[rgb_pixel_byte*i + 0];
                            g2[i] = pixel_rgb2[rgb_pixel_byte*i + 1];
                            b2[i] = pixel_rgb2[rgb_pixel_byte*i + 2];
                        }
                        break;
                    
                    case MMP_FOURCC_IMAGE_ARGB8888: /* Byte0 => B */
                    case MMP_FOURCC_IMAGE_RGB888:  /* Byte0 => B */
                        b1[i] = pixel_rgb1[rgb_pixel_byte*i + 0];
                        g1[i] = pixel_rgb1[rgb_pixel_byte*i + 1];
                        r1[i] = pixel_rgb1[rgb_pixel_byte*i + 2];
                        if((h%2)==0) {
                            b2[i] = pixel_rgb2[rgb_pixel_byte*i + 0];
                            g2[i] = pixel_rgb2[rgb_pixel_byte*i + 1];
                            r2[i] = pixel_rgb2[rgb_pixel_byte*i + 2];
                        }
                        break;

                    default: 
                        r1[i] = g1[i] = b1[i] = 0;
                }
            }

            convert_rgb2y_neon(r1, g1, b1, pixel_y);
            pixel_rgb1 += rgb_pixel_byte*8;
            pixel_y+=8;
                
            if((h%2)==0) {
                convert_rgb2uv_neon(r1, g1, b1, r2, g2, b2, pixel_u, pixel_v);
                pixel_rgb2 += rgb_pixel_byte*8;
                pixel_u+=4;
                pixel_v+=4;
            }
        }

        line_rgb1 += rgb_stride;
        line_rgb2 += rgb_stride;
        line_y += y_stride;
        if((h%2)==0) {
            line_u += u_stride;
            line_v += v_stride;
        }
    }
    
    return MMP_SUCCESS;
}

static MMP_RESULT convert_rgb2y_neon_ABGR8888(uint8x8x4_t mat8x8x4_abgr8888, MMP_U8* y) {
    
    uint8x8_t mat8x8;
    uint16x8_t mat16x8;
    uint16x8_t mat16x8_res;
    uint16x8_t mat16x8_const;
    
    //mat8x8x4_abgr8888 = vld4_u8(pARGB8888);

    //y= ((66*r[0]+129*g[0]+25*b[0])>>8)+16;
    
    //yf = 0.299f*rf + 0.587f*gf + 0.114f*bf;
    //y= (76.5*r[0]+150.272*g[0]+29.184b[0])>>8
    //y= (76*r[0]+ 150*g[0]+ 29 b[0])>>8
    /*cal r */
    mat16x8 = vmovl_u8(mat8x8x4_abgr8888.val[0]);
    //mat16x8 *= 66;
    mat16x8_const = vdupq_n_u16(76);
    mat16x8 *= mat16x8_const;//76;
    //mat16x8 >>= 8;
    mat16x8_res = mat16x8;
    //mat16x8_res += 4096;
    
    /*cal g */
    mat16x8 = vmovl_u8(mat8x8x4_abgr8888.val[1]);
    //mat16x8 *= 129;
    mat16x8_const = vdupq_n_u16(150);
    mat16x8 *= mat16x8_const;//150;
    //mat16x8 >>= 8;
    mat16x8_res += mat16x8;
    
    /*cal b */
    mat16x8 = vmovl_u8(mat8x8x4_abgr8888.val[2]);
    //mat16x8 *= 25;
    mat16x8_const = vdupq_n_u16(29);
    mat16x8 *= mat16x8_const;//29;
    //mat16x8 >>= 8;
    mat16x8_res += mat16x8;
    
    mat16x8_const = vdupq_n_u16(8);
    mat16x8_res >>= mat16x8_const;//8;
    //mat16x8_res += 16;

#if 0
    vst1_u8(y, vqmovn_u16(mat16x8_res));

#else
    mat8x8 = vqmovn_u16(mat16x8_res);
    *y = vget_lane_u8(mat8x8, 0); y++;
    *y = vget_lane_u8(mat8x8, 1); y++;
    *y = vget_lane_u8(mat8x8, 2); y++;
    *y = vget_lane_u8(mat8x8, 3); y++;
    *y = vget_lane_u8(mat8x8, 4); y++;
    *y = vget_lane_u8(mat8x8, 5); y++;
    *y = vget_lane_u8(mat8x8, 6); y++;
    *y = vget_lane_u8(mat8x8, 7); y++;
     
#endif

    return MMP_SUCCESS;
}


/*
         rgb00 rgb01 rgb02 rgb03 rgb04 rgb05 rgb06 rgb07
         rgb10 rgb11 rgb12 rgb13 rgb14 rgb15 rgb16 rgb17
      
         ==> 
             u0 u1 u2 u3   ,  v0  v1 v2 v3 

        u[i]= ((-38*r[i]-74*g[i]+112*b[i]+128)>>8)+128;
        v[i]= ((112*r[i]-94*g[i]-18*b[i]+128)>>8)+128;
*/
static void convert_rgb2uv_neon_ABGR88886(uint8x8x4_t mat8x8x4_abgr8888, MMP_U8* pixel_u, MMP_U8* pixel_v) {

    //uint8x8x4_t mat8x8x4_abgr8888;
    uint8x8_t mat8x8;
    uint16x8_t mat16x8;
    uint16x8_t mat16x8_res;
    uint16x8_t mat16x8_const;

    //mat8x8x4_abgr8888 = vld4_u8(pARGB8888);
    
    MMP_S32 i;
    MMP_U8 d[8], *d1;
    
    
    //////////////////////////////////////////////////////////////////
    // Chroma U
    //u[i]= ((-38*r[i]-74*g[i]+112*b[i])>>8)+128;
    /*cal b */
    mat16x8 = vmovl_u8(mat8x8x4_abgr8888.val[2]);
    mat16x8_const = vdupq_n_u16(112);
    mat16x8 *= mat16x8_const;//112;
    //mat16x8 += (128<<8);
    //mat16x8 >>= 8;
    mat16x8_res = mat16x8;
    mat16x8_const = vdupq_n_u16(32768);
    mat16x8_res += mat16x8_const;//32768;
        
    /*cal g */
    mat16x8 = vmovl_u8(mat8x8x4_abgr8888.val[1]);
    mat16x8_const = vdupq_n_u16(74);
    mat16x8 *= mat16x8_const;//74;
    //mat16x8 >>= 8;
    mat16x8_res -= mat16x8;
    
    
    /*cal r */
    mat16x8 = vmovl_u8(mat8x8x4_abgr8888.val[0]);
    mat16x8_const = vdupq_n_u16(38);
    mat16x8 *= mat16x8_const;//38;
    //mat16x8 >>= 8;
    mat16x8_res -= mat16x8;
        
    mat16x8_const = vdupq_n_u16(8);    
    mat16x8_res >>= mat16x8_const;//8;
    //mat16x8_res += 128;

    mat8x8 = vqmovn_u16(mat16x8_res);
    *pixel_u = vget_lane_u8(mat8x8, 0); pixel_u++;
    *pixel_u = vget_lane_u8(mat8x8, 2); pixel_u++;
    *pixel_u = vget_lane_u8(mat8x8, 4); pixel_u++;
    *pixel_u = vget_lane_u8(mat8x8, 6); pixel_u++;
    //vst1_u8(pixel_u, vrev16_u8(mat8x8));
    
    //////////////////////////////////////////////////////////////////
    // Chroma V
    //v[i]= ((112*r[i]-94*g[i]-18*b[i])>>8)+128;
    /*cal r */
    mat16x8 = vmovl_u8(mat8x8x4_abgr8888.val[0]);
    mat16x8_const = vdupq_n_u16(112);
    mat16x8 *= mat16x8_const;//112;
    //mat16x8 += (128<<8);
    //mat16x8 >>= 8;
    mat16x8_res = mat16x8;
    mat16x8_const = vdupq_n_u16(32768);
    mat16x8_res += mat16x8_const;//32768;
    
    /*cal g */
    mat16x8 = vmovl_u8(mat8x8x4_abgr8888.val[1]);
    mat16x8_const = vdupq_n_u16(94);
    mat16x8 *= mat16x8_const;
    //mat16x8 >>= 8;
    mat16x8_res -= mat16x8;
    
    /*cal b */
    mat16x8 = vmovl_u8(mat8x8x4_abgr8888.val[2]);
    mat16x8_const = vdupq_n_u16(18);
    mat16x8 *= mat16x8_const;//18;
    //mat16x8 >>= 8;
    mat16x8_res -= mat16x8;

    mat16x8_const = vdupq_n_u16(8);
    mat16x8_res >>= mat16x8_const;//8;
    //mat16x8_res += 128;
    
    mat8x8 = vqmovn_u16(mat16x8_res);
    *pixel_v = vget_lane_u8(mat8x8, 0); pixel_v++;
    *pixel_v = vget_lane_u8(mat8x8, 2); pixel_v++;
    *pixel_v = vget_lane_u8(mat8x8, 4); pixel_v++;
    *pixel_v = vget_lane_u8(mat8x8, 6); pixel_v++;

}

MMP_RESULT CMmpImageTool::ConvertRGBtoYUV420M_neon_ex1(MMP_U8* p_image_rgb, 
                                                MMP_S32 pic_width, MMP_S32 pic_height, 
                                                enum MMP_FOURCC fourcc_rgb,
                                                MMP_U8* p_image_y, MMP_U8* p_image_u, MMP_U8* p_image_v, 
                                                MMP_S32 y_stride, MMP_S32 u_stride, MMP_S32 v_stride) {

    MMP_S32 buf_stride;

    buf_stride = CMmpImageTool::GetPicStride(fourcc_rgb, pic_width);

    return CMmpImageTool::ConvertRGBtoYUV420M_neon_ex1(p_image_rgb, 
                                                pic_width, pic_height, buf_stride, 
                                                fourcc_rgb,
                                                p_image_y, p_image_u, p_image_v, 
                                                y_stride, u_stride, v_stride);
}

MMP_RESULT CMmpImageTool::ConvertRGBtoYUV420M_neon_ex1(MMP_U8* p_image_rgb, 
                                                MMP_S32 pic_width, MMP_S32 pic_height, MMP_S32 buf_stride, 
                                                enum MMP_FOURCC fourcc_rgb,
                                                MMP_U8* p_image_y, MMP_U8* p_image_u, MMP_U8* p_image_v, 
                                                MMP_S32 y_stride, MMP_S32 u_stride, MMP_S32 v_stride) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 w, h, i;
    MMP_S32 rgb_stride, rgb_pixel_byte, rgb_pixel_byte_neon;
    
    MMP_U8 *line_rgb1;
    MMP_U8 *pixel_rgb1;
    
    MMP_U8 *line_y, *line_u, *line_v;
    MMP_U8 *pixel_y, *pixel_u, *pixel_v;


    uint8x8_t mat8x8;
    uint16x8_t mat16x8;
    uint16x8_t mat16x8_res;
    uint16x8_t mat16x8_const;
    

    //MMP_U16 r[8]={0}, g[8]={0}, b[8]={0};
    //MMP_U16 *r1, *g1, *b1;

    uint8x8x4_t mat8x8x4_abgr8888;
    
    MMP_S32 pic_width_align;// = MMP_BYTE_ALIGN(pic_width, 8);

    if( (pic_width%8) == 0 ) {
        pic_width_align = pic_width;
    }
    else {
        pic_width_align = MMP_BYTE_ALIGN(pic_width, 8) - 8;
    }

    if(CMmpImageTool::IsRGB(fourcc_rgb) != MMP_TRUE) {
        mmpResult = MMP_FAILURE;
    }
    
    rgb_stride = buf_stride;
    rgb_pixel_byte = CMmpImageTool::GetPixelByte(fourcc_rgb);
    rgb_pixel_byte_neon = rgb_pixel_byte*8;

    /* convert Y */
    line_rgb1 = p_image_rgb;
    
    line_y = p_image_y;
    line_u = p_image_u;
    line_v = p_image_v;

    for(h = 0; h < pic_height; h++) {
        
        pixel_rgb1 = line_rgb1;
            
        pixel_y = line_y;
        pixel_u = line_u;
        pixel_v = line_v;
            
        for(w = 0; w < pic_width_align; w+=8 ) {
            
            mat8x8x4_abgr8888 = vld4_u8(pixel_rgb1);

#if 0
            convert_rgb2y_neon_ABGR8888(mat8x8x4_abgr8888, pixel_y);
#else
            //  y= ((66*r[0]+129*g[0]+25*b[0]+128)>>8)+16;
            //  y= ((66*r[0]+129*g[0]+25*b[0]+128 )>>8)+16;
            //   66*256 + 129*256 + 25*256 + 128 = 56448
            /*cal r */
            mat16x8 = vmovl_u8(mat8x8x4_abgr8888.val[0]);
            mat16x8_const = vdupq_n_u16(66);
            mat16x8 *= mat16x8_const;//66;
            mat16x8_res = mat16x8;
            
            /*cal g */
            mat16x8 = vmovl_u8(mat8x8x4_abgr8888.val[1]);
            mat16x8_const = vdupq_n_u16(129);
            mat16x8 *= mat16x8_const;//129;
            mat16x8_res += mat16x8;
    
            /*cal b */
            mat16x8 = vmovl_u8(mat8x8x4_abgr8888.val[2]);
            mat16x8_const = vdupq_n_u16(25);
            mat16x8 *= mat16x8_const;//25;
            mat16x8_res += mat16x8;
    
            mat16x8_const = vdupq_n_u16(128);
            mat16x8_res += mat16x8_const;//128;

            mat16x8_const = vdupq_n_u16(8);    
            mat16x8_res >>= mat16x8_const;
            
            mat16x8_const = vdupq_n_u16(16);    
            mat16x8_res += mat16x8_const;//16;

            vst1_u8(pixel_y, vqmovn_u16(mat16x8_res));
#endif


            pixel_y+=8;
            
            if( (h%2) == 0 ) {

#if 0
                convert_rgb2uv_neon_ABGR88886(mat8x8x4_abgr8888, pixel_u, pixel_v);
                pixel_u+=4;
                pixel_v+=4;
#else
                //////////////////////////////////////////////////////////////////
                // Chroma U
                //u[i]= ((-38*r[i]-74*g[i]+112*b[i])>>8)+128;
                //u[i]= ((-38*r[i]-74*g[i]+112*b[i] + 32768 )>>8);
                /*cal b */
                mat16x8 = vmovl_u8(mat8x8x4_abgr8888.val[2]);
                mat16x8_const = vdupq_n_u16(112);    
                mat16x8 *= mat16x8_const;//112;
                mat16x8_res = mat16x8;
                mat16x8_const = vdupq_n_u16(32768);
                mat16x8_res += mat16x8_const;//32768;
        
                /*cal g */
                mat16x8 = vmovl_u8(mat8x8x4_abgr8888.val[1]);
                mat16x8_const = vdupq_n_u16(74);
                mat16x8 *= mat16x8_const;//74;
                mat16x8_res -= mat16x8;
        
                /*cal r */
                mat16x8 = vmovl_u8(mat8x8x4_abgr8888.val[0]);
                mat16x8_const = vdupq_n_u16(38);
                mat16x8 *= mat16x8_const;//38;
                mat16x8_res -= mat16x8;
        
                mat16x8_const = vdupq_n_u16(8);
                mat16x8_res >>= mat16x8_const;//8;
                
                mat8x8 = vqmovn_u16(mat16x8_res);
                *pixel_u = vget_lane_u8(mat8x8, 0); pixel_u++;
                *pixel_u = vget_lane_u8(mat8x8, 2); pixel_u++;
                *pixel_u = vget_lane_u8(mat8x8, 4); pixel_u++;
                *pixel_u = vget_lane_u8(mat8x8, 6); pixel_u++;
                //vst1_u8(pixel_u, vrev16_u8(mat8x8));
    
                //////////////////////////////////////////////////////////////////
                // Chroma V
                //v[i]= ((112*r[i]-94*g[i]-18*b[i])>>8)+128;
                //v[i]= ((112*r[i]-94*g[i]-18*b[i] + 32768)>>8);
                /*cal r */
                mat16x8 = vmovl_u8(mat8x8x4_abgr8888.val[0]);
                mat16x8_const = vdupq_n_u16(112);
                mat16x8 *= mat16x8_const;//112;
                mat16x8_res = mat16x8;
                mat16x8_const = vdupq_n_u16(32768);
                mat16x8_res += mat16x8_const;//32768;
    
                /*cal g */
                mat16x8 = vmovl_u8(mat8x8x4_abgr8888.val[1]);
                mat16x8_const = vdupq_n_u16(94);
                mat16x8 *= mat16x8_const; //94
                mat16x8_res -= mat16x8;
    
                /*cal b */
                mat16x8 = vmovl_u8(mat8x8x4_abgr8888.val[2]);
                mat16x8_const = vdupq_n_u16(18);
                mat16x8 *= mat16x8_const;//18;
                mat16x8_res -= mat16x8;

                mat16x8_const = vdupq_n_u16(8);
                mat16x8_res >>= mat16x8_const;//8;
                
                mat8x8 = vqmovn_u16(mat16x8_res);
                *pixel_v = vget_lane_u8(mat8x8, 0); pixel_v++;
                *pixel_v = vget_lane_u8(mat8x8, 2); pixel_v++;
                *pixel_v = vget_lane_u8(mat8x8, 4); pixel_v++;
                *pixel_v = vget_lane_u8(mat8x8, 6); pixel_v++;

#endif
                
            }

            pixel_rgb1 += rgb_pixel_byte_neon;
            
        }

        line_rgb1 += rgb_stride;
        line_y += y_stride;
        if( (h%2) == 0 ) {
            line_u += u_stride;
            line_v += v_stride;
        }
    }
    
    return MMP_SUCCESS;
}


