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

/********************************************************** 
    Tool of Common
***********************************************************/

MMP_BOOL CMmpImageTool::IsRGB(enum MMP_FOURCC fourcc) {

    MMP_BOOL bflag;

    switch(fourcc) {
    
        case MMP_FOURCC_IMAGE_RGB565  : 
	    case MMP_FOURCC_IMAGE_RGB888  : 
	    case MMP_FOURCC_IMAGE_BGR888  : 
	    case MMP_FOURCC_IMAGE_ARGB8888: 
	    case MMP_FOURCC_IMAGE_ABGR8888:
            bflag = MMP_TRUE;
            break;

        default:
            bflag = MMP_FALSE;
    }

    return bflag;
}

MMP_BOOL CMmpImageTool::IsYUV(enum MMP_FOURCC fourcc) {

    MMP_BOOL bflag;

    switch(fourcc) {
    
        case MMP_FOURCC_IMAGE_YVU420M  : 
	    case MMP_FOURCC_IMAGE_YUV420M  : 
	        bflag = MMP_TRUE;
            break;

        default:
            bflag = MMP_FALSE;
    }

    return bflag;
}

#if 0
MMP_S32 CMmpImageTool::GetPlaneCount(enum MMP_FOURCC fourcc) {

    MMP_S32 plane_count = 0;

    switch(fourcc) {
    
        case MMP_FOURCC_IMAGE_RGB888  : 
	    case MMP_FOURCC_IMAGE_BGR888  : 
	    //case MMP_FOURCC_IMAGE_RGB565  : 
	    //case MMP_FOURCC_IMAGE_BGR565  : 
	    //case MMP_FOURCC_IMAGE_ARGB1555: 
	    //case MMP_FOURCC_IMAGE_BGRA1555: 
	    //case MMP_FOURCC_IMAGE_RGBA1555: 
	    //case MMP_FOURCC_IMAGE_ABGR1555: 
	    //case MMP_FOURCC_IMAGE_ARGB8888: 
//	    case MMP_FOURCC_IMAGE_BGRA8888: 
	    case MMP_FOURCC_IMAGE_ARGB8888: 
//	    case MMP_FOURCC_IMAGE_ABGR8888:
            plane_count = 1;
            break;

        case MMP_FOURCC_IMAGE_NV16YCbCr422_P2:
        case MMP_FOURCC_IMAGE_YCrCb422_P2:

        case MMP_FOURCC_IMAGE_NV12M:
        case MMP_FOURCC_IMAGE_NV21M:
            plane_count = 2;
            break;

        case MMP_FOURCC_IMAGE_YVU420M:
        case MMP_FOURCC_IMAGE_YUV420M:
            plane_count = 3;
            break;

        case MMP_FOURCC_IMAGE_YUV444_P1:
            plane_count = 1;
            break;

        default:
            plane_count = 0;
    }

    return plane_count;
}
#endif

MMP_S32 CMmpImageTool::GetPixelByte(enum MMP_FOURCC fourcc) {

    MMP_S32 pixel_byte = 1;

    switch(fourcc) {
    
        case MMP_FOURCC_IMAGE_RGB888  : 
	    case MMP_FOURCC_IMAGE_BGR888  : 
            pixel_byte = 3;
            break;

	    //case MMP_FOURCC_IMAGE_RGB565  : 
	    //case MMP_FOURCC_IMAGE_BGR565  : 
	    //case MMP_FOURCC_IMAGE_ARGB1555: 
	    //case MMP_FOURCC_IMAGE_BGRA1555: 
	    //case MMP_FOURCC_IMAGE_RGBA1555: 
	    //case MMP_FOURCC_IMAGE_ABGR1555: 
          //  pixel_byte = 2;
            //break;

	    ///case MMP_FOURCC_IMAGE_ARGB8888: 
	    //case MMP_FOURCC_IMAGE_BGRA8888: 
	    case MMP_FOURCC_IMAGE_ARGB8888: 
	    case MMP_FOURCC_IMAGE_ABGR8888:
            pixel_byte = 4;
            break;

        case MMP_FOURCC_IMAGE_YVU420M:
        case MMP_FOURCC_IMAGE_YUV420M:
        default:
            pixel_byte = 1;
    }

    return pixel_byte;
}

MMP_S32 CMmpImageTool::GetPicStride(enum MMP_FOURCC fourcc, MMP_S32 pic_width) {

    MMP_S32 stride;

    switch(fourcc) {
    
        case MMP_FOURCC_IMAGE_RGB888  : 
	    case MMP_FOURCC_IMAGE_BGR888  : 
            stride = MMP_BYTE_ALIGN(pic_width*3, 4); 
            break;

	    //case MMP_FOURCC_IMAGE_RGB565  : 
	    //case MMP_FOURCC_IMAGE_BGR565  : 
          //  stride = pic_width*2; 
            //break;

//	    case MMP_FOURCC_IMAGE_ARGB1555: 
//	    case MMP_FOURCC_IMAGE_BGRA1555: 
//	    case MMP_FOURCC_IMAGE_RGBA1555: 
//	    case MMP_FOURCC_IMAGE_ABGR1555: 
  //          stride = pic_width*2; 
    //        break;

	  //  case MMP_FOURCC_IMAGE_ARGB8888: 
	  //  case MMP_FOURCC_IMAGE_BGRA8888: 
	    case MMP_FOURCC_IMAGE_ARGB8888: 
	    case MMP_FOURCC_IMAGE_ABGR8888:
            stride = pic_width*4; 
            break;
            
        case MMP_FOURCC_IMAGE_YVU420M:
        case MMP_FOURCC_IMAGE_YUV420M:
        default:
            stride = pic_width;
    }

    return stride;
}

MMP_S32 CMmpImageTool::GetBufferStride(enum MMP_FOURCC fourcc, MMP_S32 pic_width) {

    MMP_S32 stride;

    switch(fourcc) {
    
        case MMP_FOURCC_IMAGE_RGB888  : 
	    case MMP_FOURCC_IMAGE_BGR888  : 
            stride = MMP_BYTE_ALIGN(pic_width*3, 4); 
            break;

	    //case MMP_FOURCC_IMAGE_RGB565  : 
	    //case MMP_FOURCC_IMAGE_BGR565  : 
          //  stride = pic_width*2; 
          //  break;

	    //case MMP_FOURCC_IMAGE_ARGB1555: 
	    //case MMP_FOURCC_IMAGE_BGRA1555: 
	    //case MMP_FOURCC_IMAGE_RGBA1555: 
	    //case MMP_FOURCC_IMAGE_ABGR1555: 
          //  stride = pic_width*2; 
          //  break;

	    //case MMP_FOURCC_IMAGE_ARGB8888: 
	    //case MMP_FOURCC_IMAGE_BGRA8888: 
	    case MMP_FOURCC_IMAGE_ARGB8888: 
	    //case MMP_FOURCC_IMAGE_ABGR8888:
            stride = pic_width*4; 
            break;
            
        case MMP_FOURCC_IMAGE_YVU420M:
        case MMP_FOURCC_IMAGE_YUV420M:
            stride = MMP_BYTE_ALIGN(pic_width, 16); 
            break;

        default:
            stride = pic_width;
    }

    return stride;
}

/********************************************************** 
        Tool of Impage Processing
***********************************************************/
MMP_RESULT CMmpImageTool::Flip_V(MMP_S32 pic_width, MMP_S32 pic_height, MMP_U8* p_image, enum MMP_FOURCC fourcc) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 stride;
    MMP_U8* tmpbuf = NULL;
    MMP_S32 h, flip_height;
    MMP_U8 *p_image_top, *p_image_bottom;

    /* check stride */
    switch(fourcc) {
    
        case MMP_FOURCC_IMAGE_BGR888:
        case MMP_FOURCC_IMAGE_RGB888: stride = MMP_BYTE_ALIGN(pic_width*3,4); break;

        case MMP_FOURCC_IMAGE_ARGB8888: stride = pic_width*4; break;
        default:
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("CMmpImageTool::%s] FAIL: not support format (%c%c%c%c) "), MMP_CLASS_FUNC, 
                                                     MMPGETFOURCCARG(fourcc)
                                                     ));
    }

    if(mmpResult == MMP_SUCCESS) {

        tmpbuf = (MMP_U8*)MMP_MALLOC(stride);
        if(tmpbuf != NULL) {
            
            flip_height = pic_height/2;
            p_image_top = p_image;
            p_image_bottom = p_image + stride*(pic_height-1);
            for(h = 0; h < flip_height; h++) {
                memcpy(tmpbuf, p_image_top, stride);
                memcpy(p_image_top, p_image_bottom, stride);
                memcpy(p_image_bottom, tmpbuf, stride);

                p_image_top += stride;
                p_image_bottom -= stride;
            }
        }
    }

    if(tmpbuf != NULL) {
        MMP_FREE(tmpbuf);
    }

    return mmpResult;
}

/* YUV 4:2:2 packed, CbYCrY */
MMP_RESULT CMmpImageTool::ConvertRGBtoYUYV(MMP_U8* p_image_rgb, MMP_S32 pic_width, MMP_S32 pic_height,enum MMP_FOURCC fourcc_rgb,
                                       MMP_U8* p_image_yuyv, enum MMP_FOURCC fourcc_yuyv) {

    MMP_U8 *p_image_y, *p_image_u, *p_image_v;
    MMP_S32 y_stride, u_stride, v_stride;
    MMP_RESULT mmpResult = MMP_SUCCESS;
    
    p_image_y = new MMP_U8[ V4L2_YUV422P_Y_FRAME_SIZE(pic_width, pic_height) ];
    p_image_u = new MMP_U8[ V4L2_YUV422P_U_FRAME_SIZE(pic_width, pic_height) ];
    p_image_v = new MMP_U8[ V4L2_YUV422P_V_FRAME_SIZE(pic_width, pic_height) ];

    y_stride = V4L2_YUV422P_Y_STRIDE(pic_width);
    u_stride = V4L2_YUV422P_U_STRIDE(pic_width);
    v_stride = V4L2_YUV422P_V_STRIDE(pic_width);

    if( (p_image_y!=NULL)  &&  (p_image_u!=NULL)  && (p_image_v!=NULL) ) {

        mmpResult = CMmpImageTool::ConvertRGBtoYUV422PM(p_image_rgb, pic_width, pic_height, fourcc_rgb,
                                                  p_image_y, p_image_u, p_image_v, 
                                                  y_stride, u_stride, v_stride);
        if(mmpResult == MMP_SUCCESS) {
    
            MMP_S32 yuyv_stride = V4L2_YUYV_STRIDE(pic_width);

            MMP_U8 *line_yuyv, *pix_yuyv;

            MMP_U8 *line_y, *line_u, *line_v;
            MMP_U8 *pix_y, *pix_u, *pix_v;
            MMP_S32 w, h;

            MMP_S32 y0_idx, y1_idx, u_idx, v_idx;

            switch(fourcc_yuyv) {
                
                case MMP_FOURCC_IMAGE_UYVY:
                    y0_idx = 1, y1_idx = 3;
                    u_idx = 0, v_idx = 2;
                    break;
                
                case MMP_FOURCC_IMAGE_VYUY:
                    y0_idx = 1, y1_idx = 3;
                    u_idx = 2, v_idx = 0;
                    break;
                
                case MMP_FOURCC_IMAGE_YVYU:
                    y0_idx = 0, y1_idx = 2;
                    u_idx = 3, v_idx = 1;
                    break;
                
                case MMP_FOURCC_IMAGE_YUYV:
                default:
                    y0_idx = 0, y1_idx = 2;
                    u_idx = 1, v_idx = 3;
                    break;
            }
    
            line_yuyv = p_image_yuyv;
            line_y = p_image_y;
            line_u = p_image_u;
            line_v = p_image_v;

            for(h = 0; h < pic_height; h++) {
                
                pix_yuyv = line_yuyv;

                pix_y = line_y;
                pix_u = line_u;
                pix_v = line_v;

                for(w = 0; w < pic_width; w+=2) {

                    pix_yuyv[y0_idx] = pix_y[0];
                    pix_yuyv[y1_idx] = pix_y[1];
                    pix_yuyv[u_idx] = pix_u[0];
                    pix_yuyv[v_idx] = pix_v[0];
                    
                    pix_yuyv += 4;
                    pix_y+=2;
                    pix_u++;
                    pix_v++;
                }
                                
                line_yuyv += yuyv_stride;
                line_y += y_stride;
                line_u += u_stride;
                line_v += v_stride;
            }

        }

    }

    if(p_image_y) delete p_image_y;
    if(p_image_u) delete p_image_u;
    if(p_image_v) delete p_image_v;

    return mmpResult;
}
    
MMP_RESULT CMmpImageTool::ConvertRGBtoGREY(MMP_U8* p_image_rgb, MMP_S32 pic_width, MMP_S32 pic_height,enum MMP_FOURCC fourcc_rgb,
                                       MMP_U8* p_image_grey) {

    MMP_U8 *p_image_y, *p_image_u, *p_image_v;
    MMP_S32 y_stride, u_stride, v_stride;
    MMP_RESULT mmpResult;
    
    p_image_y = p_image_grey;
    p_image_u = (MMP_U8*)MMP_MALLOC(V4L2_YU12_U_FRAME_SIZE(pic_width, pic_height));
    p_image_v = (MMP_U8*)MMP_MALLOC(V4L2_YU12_V_FRAME_SIZE(pic_width, pic_height));
    
    y_stride = V4L2_YU12_Y_STRIDE(pic_width);
    u_stride = V4L2_YU12_U_STRIDE(pic_width);
    v_stride = V4L2_YU12_V_STRIDE(pic_width);

    mmpResult = CMmpImageTool::ConvertRGBtoYUV420M(p_image_rgb, pic_width, pic_height, fourcc_rgb,
                                                p_image_y, p_image_u, p_image_v, 
                                                y_stride, u_stride, v_stride);

    MMP_FREE(p_image_u);
    MMP_FREE(p_image_v);

    return mmpResult;
}

MMP_RESULT CMmpImageTool::ConvertRGBtoYUV420(MMP_U8* p_image_rgb, MMP_S32 pic_width, MMP_S32 pic_height,enum MMP_FOURCC fourcc_rgb,
                                       MMP_U8* p_image_yu12) {

    MMP_U8 *p_image_y, *p_image_u, *p_image_v;
    MMP_S32 y_stride, u_stride, v_stride;
    
    p_image_y = p_image_yu12 + V4L2_YU12_Y_FRAME_OFFSET(pic_width, pic_height);
    p_image_u = p_image_yu12 + V4L2_YU12_U_FRAME_OFFSET(pic_width, pic_height);
    p_image_v = p_image_yu12 + V4L2_YU12_V_FRAME_OFFSET(pic_width, pic_height);

    y_stride = V4L2_YU12_Y_STRIDE(pic_width);
    u_stride = V4L2_YU12_U_STRIDE(pic_width);
    v_stride = V4L2_YU12_V_STRIDE(pic_width);

    return CMmpImageTool::ConvertRGBtoYUV420M(p_image_rgb, pic_width, pic_height, fourcc_rgb,
                                                p_image_y, p_image_u, p_image_v, 
                                                y_stride, u_stride, v_stride);
}

MMP_RESULT CMmpImageTool::ConvertRGBtoYVU420(MMP_U8* p_image_rgb, MMP_S32 pic_width, MMP_S32 pic_height,enum MMP_FOURCC fourcc_rgb,
                                       MMP_U8* p_image_yu12) {

    MMP_U8 *p_image_y, *p_image_u, *p_image_v;
    MMP_S32 y_stride, u_stride, v_stride;
    
    p_image_y = p_image_yu12 + V4L2_YV12_Y_FRAME_OFFSET(pic_width, pic_height);
    p_image_u = p_image_yu12 + V4L2_YV12_U_FRAME_OFFSET(pic_width, pic_height);
    p_image_v = p_image_yu12 + V4L2_YV12_V_FRAME_OFFSET(pic_width, pic_height);

    y_stride = V4L2_YV12_Y_STRIDE(pic_width);
    u_stride = V4L2_YV12_U_STRIDE(pic_width);
    v_stride = V4L2_YV12_V_STRIDE(pic_width);

    return CMmpImageTool::ConvertRGBtoYUV420M(p_image_rgb, pic_width, pic_height, fourcc_rgb,
                                                p_image_y, p_image_u, p_image_v, 
                                                y_stride, u_stride, v_stride);
}

MMP_RESULT CMmpImageTool::ConvertRGBtoYUV420M(MMP_U8* p_image_rgb, MMP_S32 pic_width, MMP_S32 pic_height,enum MMP_FOURCC fourcc_rgb,
                                                MMP_U8* p_image_y, MMP_U8* p_image_u, MMP_U8* p_image_v, 
                                                MMP_S32 y_stride, MMP_S32 u_stride, MMP_S32 v_stride) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 w, h;
    MMP_S32 rgb_stride, rgb_pixel_byte;
    
    MMP_U8 *p_image_src1, *p_image_src2;
    MMP_U8 *p_line_src1, *p_line_src2;
    
    MMP_U8 *p_image_dest_y, *p_image_dest_u, *p_image_dest_v;
    MMP_U8 *p_line_dest_y, *p_line_dest_u, *p_line_dest_v;

    MMP_S32 y;
    MMP_S32 r[4]={0}, g[4]={0}, b[4]={0};
    MMP_S32 u[4]={0}, v[4]={0};

    MMP_S32 i;
    MMP_BOOL is_end_col;

    if(CMmpImageTool::IsRGB(fourcc_rgb) != MMP_TRUE) {
        mmpResult = MMP_FAILURE;
    }
    
    rgb_stride = CMmpImageTool::GetPicStride(fourcc_rgb, pic_width);
    rgb_pixel_byte = CMmpImageTool::GetPixelByte(fourcc_rgb);

    /* convert Y */
    if(mmpResult == MMP_SUCCESS) {
        
        p_image_src1 = p_image_rgb;
        p_image_dest_y = p_image_y;
        for(h = 0; h < pic_height; h++) {
        
            p_line_src1 = p_image_src1;
            p_line_dest_y = p_image_dest_y;
            for(w = 0; w < pic_width; w++) {
                
                switch(fourcc_rgb) {

                    case MMP_FOURCC_IMAGE_ABGR8888: /* Byte0 => R */
                    case MMP_FOURCC_IMAGE_BGR888: /* Byte0 => R */
                        r[0] = p_line_src1[0];
                        g[0] = p_line_src1[1];
                        b[0] = p_line_src1[2];
                        break;
                    
                    case MMP_FOURCC_IMAGE_ARGB8888: /* Byte0 => B */
                    case MMP_FOURCC_IMAGE_RGB888:  /* Byte0 => B */
                        b[0] = p_line_src1[0];
                        g[0] = p_line_src1[1];
                        r[0] = p_line_src1[2];
                        break;

                    default: 
                        r[0] = g[0] = b[0] = 0;
                }

                r[0]&=0xff, g[0]&=0xff, b[0]&=0xff;
#if 1
                y= ((66*r[0]+129*g[0]+25*b[0]+128)>>8)+16;
#else
                float rf, gf, bf, yf;
                rf = (float)r[0];
                gf = (float)g[0];
                bf = (float)b[0];
                yf = 0.299f*rf + 0.587f*gf + 0.114f*bf;
                                
                y = (MMP_S32)yf;
#endif

                (*p_line_dest_y) = (MMP_U8)y;
            
                p_line_src1 += rgb_pixel_byte;
                p_line_dest_y++;
            }

            p_image_src1 += rgb_stride;
            p_image_dest_y += y_stride;
        }
    }
        
    /* convert U, V */
    /*
         u0, u1 
         u2  u3   ==> U
    */
    if(mmpResult == MMP_SUCCESS) {
        
        p_image_src1 = p_image_rgb;
        p_image_src2 = p_image_src1 + rgb_stride;
        p_image_dest_u = p_image_u;
        p_image_dest_v = p_image_v;
        for(h = 0; h < pic_height; h+=2) {
        
            p_line_src1 = p_image_src1;
            p_line_src2 = p_image_src2;
            
            /* if odd height, h is last line */
            if( ((pic_height%2)==1) && (h==(pic_height-1)) ){
                p_line_src2 = p_line_src1;
            }

            p_line_dest_u = p_image_dest_u;
            p_line_dest_v = p_image_dest_v;
            for(w = 0; w < pic_width; w+=2 ) {
             
                /* if odd height, h is last line */
                if( ((pic_width%2)==1) && (w==(pic_width-1)) ){
                    is_end_col = MMP_TRUE;
                }
                else {
                    is_end_col = MMP_FALSE;
                }

                /*
                       u0, u1 
                       u2  u3   ==> U
                 */
                switch(fourcc_rgb) {
                    case MMP_FOURCC_IMAGE_ABGR8888: /* Byte0 => R */
                        r[0] = p_line_src1[0], g[0] = p_line_src1[1], b[0] = p_line_src1[2];
                        r[2] = p_line_src2[0], g[2] = p_line_src2[1], b[2] = p_line_src2[2];
                        if(is_end_col == MMP_FALSE) {
                            r[1] = p_line_src1[4], g[1] = p_line_src1[5], b[1] = p_line_src1[6];
                            r[3] = p_line_src2[4], g[3] = p_line_src2[5], b[3] = p_line_src2[6];
                        }
                        else {
                            b[1] = b[0], g[1] = g[0], r[1] = r[0];
                            b[3] = b[2], g[3] = g[2], r[3] = r[2];
                        }
                        break;

                    case MMP_FOURCC_IMAGE_BGR888: /* Byte0 => R */
                        r[0] = p_line_src1[0], g[0] = p_line_src1[1], b[0] = p_line_src1[2];
                        r[2] = p_line_src2[0], g[2] = p_line_src2[1], b[2] = p_line_src2[2];
                        if(is_end_col == MMP_FALSE) {
                            r[1] = p_line_src1[3], g[1] = p_line_src1[4], b[1] = p_line_src1[5];
                            r[3] = p_line_src2[3], g[3] = p_line_src2[4], b[3] = p_line_src2[5];
                        }
                        else {
                            b[1] = b[0], g[1] = g[0], r[1] = r[0];
                            b[3] = b[2], g[3] = g[2], r[3] = r[2];
                        }
                        break;

                    case MMP_FOURCC_IMAGE_ARGB8888: /* Byte0 => B */
                        b[0] = p_line_src1[0], g[0] = p_line_src1[1], r[0] = p_line_src1[2];
                        b[2] = p_line_src2[0], g[2] = p_line_src2[1], r[2] = p_line_src2[2];
                        if(is_end_col == MMP_FALSE) {
                            b[1] = p_line_src1[4], g[1] = p_line_src1[5], r[1] = p_line_src1[6];
                            b[3] = p_line_src2[4], g[3] = p_line_src2[5], r[3] = p_line_src2[6];
                        }
                        else {
                            r[1] = r[0], g[1] = g[0], b[1] = b[0];
                            r[3] = r[2], g[3] = g[2], b[3] = b[2];
                        }
                        break;

                    case MMP_FOURCC_IMAGE_RGB888: /* Byte0 => B */
                        b[0] = p_line_src1[0], g[0] = p_line_src1[1], r[0] = p_line_src1[2];
                        b[2] = p_line_src2[0], g[2] = p_line_src2[1], r[2] = p_line_src2[2];
                        if(is_end_col == MMP_FALSE) {
                            b[1] = p_line_src1[3], g[1] = p_line_src1[4], r[1] = p_line_src1[5];
                            b[3] = p_line_src2[3], g[3] = p_line_src2[4], r[3] = p_line_src2[5];
                        }
                        else {
                            r[1] = r[0], g[1] = g[0], b[1] = b[0];
                            r[3] = r[2], g[3] = g[2], b[3] = b[2];
                        }
                        break;

                    default: 
                        r[0] = g[0] = b[0] = 0;
                        r[1] = g[1] = b[1] = 0;
                        r[2] = g[2] = b[2] = 0;
                        r[3] = g[3] = b[3] = 0;
                }

                for(i = 0; i < 4; i++) {
                    r[i]&=0xff, g[i]&=0xff, b[i]&=0xff;

#if 0
                    float rf, gf, bf, yf, uf, vf;
                    rf = (float)r[i];
                    gf = (float)g[i];
                    bf = (float)b[i];
                    yf = 0.299f*rf + 0.587f*gf + 0.114f*bf;
                    //uf = (bf-yf) * 0.492f;
                    //vf = -0.148f*(rf-yf) * 0.877f;
                    uf = (bf-yf) * 0.492f;
                    vf = (rf-yf) * 0.877f;
                    u[i] = (MMP_S32)(uf + 128.0f);
                    v[i] = (MMP_S32)(vf + 128.0f);
                                
#else                
                    u[i]= ((-38*r[i]-74*g[i]+112*b[i])>>8)+128;
                    v[i]= ((112*r[i]-94*g[i]-18*b[i])>>8)+128;
#endif
                }
         
#if 1
                (*p_line_dest_u) = (MMP_U8)((u[0] + u[1] + u[2] + u[3])/4);
                (*p_line_dest_v) = (MMP_U8)((v[0] + v[1] + v[2] + v[3])/4);
#else
                (*p_line_dest_u) = (MMP_U8)((u[0])/1);
                (*p_line_dest_v) = (MMP_U8)((v[0])/1);
#endif
            
                p_line_src1 += rgb_pixel_byte*2;
                p_line_src2 += rgb_pixel_byte*2;
                p_line_dest_u++;
                p_line_dest_v++;
            }

            p_image_src1 += rgb_stride*2;
            p_image_src2 += rgb_stride*2;

            p_image_dest_u += u_stride;
            p_image_dest_v += v_stride;
        }
    }

    return mmpResult;
}


MMP_RESULT CMmpImageTool::ConvertRGBtoYUV444M(MMP_U8* p_image_rgb, MMP_S32 pic_width, MMP_S32 pic_height,enum MMP_FOURCC fourcc_rgb,
                                                MMP_U8* p_image_y, MMP_U8* p_image_u, MMP_U8* p_image_v, 
                                                MMP_S32 y_stride, MMP_S32 u_stride, MMP_S32 v_stride) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 w, h;
    MMP_S32 rgb_stride, rgb_pixel_byte;
    
    MMP_U8 *p_image_src1;
    MMP_U8 *p_line_src1;
    
    MMP_U8 *p_image_dest_y, *p_image_dest_u, *p_image_dest_v;
    MMP_U8 *p_line_dest_y, *p_line_dest_u, *p_line_dest_v;

    MMP_S32 y, u, v;
    MMP_S32 r, g, b;
    
    if(CMmpImageTool::IsRGB(fourcc_rgb) != MMP_TRUE) {
        mmpResult = MMP_FAILURE;
    }
    
    rgb_stride = CMmpImageTool::GetPicStride(fourcc_rgb, pic_width);
    rgb_pixel_byte = CMmpImageTool::GetPixelByte(fourcc_rgb);

    /* convert Y */
    if(mmpResult == MMP_SUCCESS) {
        
        p_image_src1 = p_image_rgb;
        p_image_dest_y = p_image_y;
        p_image_dest_u = p_image_u;
        p_image_dest_v = p_image_v;
        for(h = 0; h < pic_height; h++) {
        
            p_line_src1 = p_image_src1;
            p_line_dest_y = p_image_dest_y;
            p_line_dest_u = p_image_dest_u;
            p_line_dest_v = p_image_dest_v;
            for(w = 0; w < pic_width; w++) {
                
                switch(fourcc_rgb) {

                    case MMP_FOURCC_IMAGE_BGR888: /* Byte0 => R */
                        r = p_line_src1[0];
                        g = p_line_src1[1];
                        b = p_line_src1[2];
                        break;

                    case MMP_FOURCC_IMAGE_RGB888:  /* Byte0 => B */
                        b = p_line_src1[0];
                        g = p_line_src1[1];
                        r = p_line_src1[2];
                        break;

                    default: 
                        r = g = b = 0;
                }

                r&=0xff, g&=0xff, b&=0xff;
                y= ((66*r+129*g+25*b+128)>>8)+16;
                u= ((-38*r-74*g+112*b+128)>>8)+128;
                v= ((112*r-94*g-18*b+128)>>8)+128;

                (*p_line_dest_y) = (MMP_U8)y;
                (*p_line_dest_u) = (MMP_U8)u;
                (*p_line_dest_v) = (MMP_U8)v;
            
                p_line_src1 += rgb_pixel_byte;
                p_line_dest_y++;
                p_line_dest_u++;
                p_line_dest_v++;
            }

            p_image_src1 += rgb_stride;
            p_image_dest_y += y_stride;
            p_image_dest_u += u_stride;
            p_image_dest_v += v_stride;
        }
    }

    
    return mmpResult;
}


MMP_RESULT CMmpImageTool::ConvertRGBtoYUV422P(MMP_U8* p_image_rgb, MMP_S32 pic_width, MMP_S32 pic_height,enum MMP_FOURCC fourcc_rgb,
                                       MMP_U8* p_image_yuv422p) {

    MMP_U8 *p_image_y, *p_image_u, *p_image_v;
    MMP_S32 y_stride, u_stride, v_stride;

    p_image_y = p_image_yuv422p + V4L2_YUV422P_Y_FRAME_OFFSET(pic_width, pic_height);
    p_image_u = p_image_yuv422p + V4L2_YUV422P_U_FRAME_OFFSET(pic_width, pic_height);
    p_image_v = p_image_yuv422p + V4L2_YUV422P_V_FRAME_OFFSET(pic_width, pic_height);

    y_stride = V4L2_YUV422P_Y_STRIDE(pic_width);
    u_stride = V4L2_YUV422P_U_STRIDE(pic_width);
    v_stride = V4L2_YUV422P_V_STRIDE(pic_width);

    return CMmpImageTool::ConvertRGBtoYUV422PM(p_image_rgb, pic_width, pic_height, fourcc_rgb,
                                                p_image_y, p_image_u, p_image_v, 
                                                y_stride, u_stride, v_stride);
}

MMP_RESULT CMmpImageTool::ConvertRGBtoYUV422PM(MMP_U8* p_image_rgb, MMP_S32 pic_width, MMP_S32 pic_height,enum MMP_FOURCC fourcc_rgb,
                                                MMP_U8* p_image_y, MMP_U8* p_image_u, MMP_U8* p_image_v, 
                                                MMP_S32 y_stride, MMP_S32 u_stride, MMP_S32 v_stride) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 w, h;
    MMP_S32 rgb_stride, rgb_pixel_byte;
    
    MMP_U8 *p_image_src1;
    MMP_U8 *p_line_src1;
    
    MMP_U8 *p_image_dest_y, *p_image_dest_u, *p_image_dest_v;
    MMP_U8 *p_line_dest_y, *p_line_dest_u, *p_line_dest_v;

    MMP_S32 y;
    MMP_S32 r[4]={0}, g[4]={0}, b[4]={0};
    MMP_S32 u[4]={0}, v[4]={0};

    MMP_S32 i;
    MMP_BOOL is_end_col;

    if(CMmpImageTool::IsRGB(fourcc_rgb) != MMP_TRUE) {
        mmpResult = MMP_FAILURE;
    }
    
    rgb_stride = CMmpImageTool::GetPicStride(fourcc_rgb, pic_width);
    rgb_pixel_byte = CMmpImageTool::GetPixelByte(fourcc_rgb);

    /* convert Y */
    if(mmpResult == MMP_SUCCESS) {
        
        p_image_src1 = p_image_rgb;
        p_image_dest_y = p_image_y;
        for(h = 0; h < pic_height; h++) {
        
            p_line_src1 = p_image_src1;
            p_line_dest_y = p_image_dest_y;
            for(w = 0; w < pic_width; w++) {
                
                switch(fourcc_rgb) {

                    case MMP_FOURCC_IMAGE_BGR888: /* Byte0 => R */
                        r[0] = p_line_src1[0];
                        g[0] = p_line_src1[1];
                        b[0] = p_line_src1[2];
                        break;

                    case MMP_FOURCC_IMAGE_RGB888:  /* Byte0 => B */
                        b[0] = p_line_src1[0];
                        g[0] = p_line_src1[1];
                        r[0] = p_line_src1[2];
                        break;

                    default: 
                        r[0] = g[0] = b[0] = 0;
                }

                r[0]&=0xff, g[0]&=0xff, b[0]&=0xff;
                y= ((66*r[0]+129*g[0]+25*b[0]+128)>>8)+16;

                (*p_line_dest_y) = (MMP_U8)y;
            
                p_line_src1 += rgb_pixel_byte;
                p_line_dest_y++;
            }

            p_image_src1 += rgb_stride;
            p_image_dest_y += y_stride;
        }
    }

    /* convert U, V */
    /*
        u0, u1  ==> U
    */
    if(mmpResult == MMP_SUCCESS) {
        
        p_image_src1 = p_image_rgb;
        p_image_dest_u = p_image_u;
        p_image_dest_v = p_image_v;
        for(h = 0; h < pic_height; h++) {
        
            p_line_src1 = p_image_src1;
            p_line_dest_u = p_image_dest_u;
            p_line_dest_v = p_image_dest_v;
            for(w = 0; w < pic_width; w+=2 ) {
             
                /* if odd height, h is last line */
                if( ((pic_width%2)==1) && (w==(pic_width-1)) ){
                    is_end_col = MMP_TRUE;
                }
                else {
                    is_end_col = MMP_FALSE;
                }

                /*
                       u0, u1  ==> U
                 */
                switch(fourcc_rgb) {
                    case MMP_FOURCC_IMAGE_BGR888:  /* Byte0 => R */
                        r[0] = p_line_src1[0], g[0] = p_line_src1[1], b[0] = p_line_src1[2];
                        if(is_end_col == MMP_FALSE) {
                            r[1] = p_line_src1[3], g[1] = p_line_src1[4], b[1] = p_line_src1[5];
                        }
                        else {
                            b[1] = b[0], g[1] = g[0], r[1] = r[0];
                        }
                        break;

                    case MMP_FOURCC_IMAGE_RGB888:  /* Byte0 => B */
                        b[0] = p_line_src1[0], g[0] = p_line_src1[1], r[0] = p_line_src1[2];
                        if(is_end_col == MMP_FALSE) {
                            b[1] = p_line_src1[3], g[1] = p_line_src1[4], r[1] = p_line_src1[5];
                        }
                        else {
                            r[1] = r[0], g[1] = g[0], b[1] = b[0];
                        }
                        break;

                    default: 
                        r[0] = g[0] = b[0] = 0;
                        r[1] = g[1] = b[1] = 0;
                }

                for(i = 0; i < 2; i++) {
                    r[i]&=0xff, g[i]&=0xff, b[i]&=0xff;
                    u[i]= ((-38*r[i]-74*g[i]+112*b[i]+128)>>8)+128;
                    v[i]= ((112*r[i]-94*g[i]-18*b[i]+128)>>8)+128;
                }
         
                (*p_line_dest_u) = (MMP_U8)((u[0] + u[1] )/2);
                (*p_line_dest_v) = (MMP_U8)((v[0] + v[1] )/2);
            
                p_line_src1 += rgb_pixel_byte*2;
                p_line_dest_u++;
                p_line_dest_v++;
            }

            p_image_src1 += rgb_stride;
            p_image_dest_u += u_stride;
            p_image_dest_v += v_stride;
        }
    }

    return mmpResult;
}

MMP_RESULT CMmpImageTool::ConvertRGBtoNV16(MMP_U8* p_image_rgb, MMP_S32 pic_width, MMP_S32 pic_height,enum MMP_FOURCC fourcc_rgb,
                                           MMP_U8* p_image_nv16) {
    MMP_U8* p_image_y, *p_image_uv;
    MMP_S32 y_stride, uv_stride;
    MMP_S32 offset;

    p_image_y = p_image_nv16;
    offset = V4L2_NV16_UV_FRAME_OFFSET(pic_width, pic_height);
    p_image_uv= p_image_nv16 + offset;
    y_stride = V4L2_NV16_Y_STRIDE(pic_width);
    uv_stride = V4L2_NV16_UV_STRIDE(pic_width);

    return CMmpImageTool::ConvertRGBtoNV16M(p_image_rgb, pic_width, pic_height, fourcc_rgb,
                                            p_image_y, p_image_uv,
                                            y_stride, uv_stride);
}

MMP_RESULT CMmpImageTool::ConvertRGBtoNV16V(MMP_U8* p_image_rgb, MMP_S32 pic_width, MMP_S32 pic_height,enum MMP_FOURCC fourcc_rgb,
                                           MMP_U8* p_image_nv16v) {
    MMP_U8* p_image_y, *p_image_uv;
    MMP_S32 y_stride, uv_stride;
    MMP_S32 offset;

    p_image_y = p_image_nv16v;
    offset = V4L2_NV16V_UV_FRAME_OFFSET(pic_width, pic_height);
    p_image_uv= p_image_nv16v + offset;
    y_stride = V4L2_NV16V_Y_STRIDE(pic_width);
    uv_stride = V4L2_NV16V_UV_STRIDE(pic_width);

    return CMmpImageTool::ConvertRGBtoNV16MV(p_image_rgb, pic_width, pic_height, fourcc_rgb,
                                            p_image_y, p_image_uv,
                                            y_stride, uv_stride);
}

MMP_RESULT CMmpImageTool::ConvertRGBtoNV16M(MMP_U8* p_image_rgb, MMP_S32 pic_width, MMP_S32 pic_height,enum MMP_FOURCC fourcc_rgb,
                                                  MMP_U8* p_image_y, MMP_U8* p_image_uv,
                                                  MMP_S32 y_stride, MMP_S32 uv_stride) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 w, h;
    MMP_S32 rgb_stride, rgb_pixel_byte;
    
    MMP_U8 *p_image_src1;
    MMP_U8 *p_line_src1;
    
    MMP_U8 *p_image_dest_y;
    MMP_U8 *p_line_dest_y;

    MMP_S32 y;
    MMP_S32 r[2]={0}, g[2]={0}, b[2]={0};
    MMP_S32 u[2]={0}, v[2]={0};

    MMP_S32 i;
    MMP_BOOL is_end_col;

    if(CMmpImageTool::IsRGB(fourcc_rgb) != MMP_TRUE) {
        mmpResult = MMP_FAILURE;
    }

    rgb_stride = CMmpImageTool::GetPicStride(fourcc_rgb, pic_width);
    rgb_pixel_byte = CMmpImageTool::GetPixelByte(fourcc_rgb);

    /* convert Y */
    if(mmpResult == MMP_SUCCESS) {
        
        p_image_src1 = p_image_rgb;
        p_image_dest_y = p_image_y;
        for(h = 0; h < pic_height; h++) {
        
            p_line_src1 = p_image_src1;
            p_line_dest_y = p_image_dest_y;
            for(w = 0; w < pic_width; w++) {
                
                switch(fourcc_rgb) {
                    case MMP_FOURCC_IMAGE_BGR888: /* Byte0 => R */
                        r[0] = p_line_src1[0];
                        g[0] = p_line_src1[1];
                        b[0] = p_line_src1[2];
                        break;

                    case MMP_FOURCC_IMAGE_RGB888:  /* Byte0 => B */
                        b[0] = p_line_src1[0];
                        g[0] = p_line_src1[1];
                        r[0] = p_line_src1[2];
                        break;

                    default: 
                        r[0] = g[0] = b[0] = 0;
                }

                r[0]&=0xff, g[0]&=0xff, b[0]&=0xff;
                y= ((66*r[0]+129*g[0]+25*b[0]+128)>>8)+16;

                (*p_line_dest_y) = (MMP_U8)y;
            
                p_line_src1 += rgb_pixel_byte;
                p_line_dest_y++;
            }

            p_image_src1 += rgb_stride;
            p_image_dest_y += y_stride;
        }
    }

    /* convert U, V */
    /*
         [u0,v0] [u1,v1] ..
    */
    if(mmpResult == MMP_SUCCESS) {
        
        MMP_U8* src_line, *src_pix;
        MMP_U8* dst_line, *dst_pix;

        src_line = p_image_rgb;
        dst_line = p_image_uv;
        for(h = 0; h < pic_height; h++) {
        
            src_pix = src_line;
            dst_pix = dst_line;
                        
            for(w = 0; w < pic_width; w+=2 ) {
             
                /* if odd height, h is last line */
                if( ((pic_width%2)==1) && (w==(pic_width-1)) ){
                    is_end_col = MMP_TRUE;
                }
                else {
                    is_end_col = MMP_FALSE;
                }

                /*
                       u0, u1 
                       u2  u3   ==> U
                 */
                switch(fourcc_rgb) {
                    case MMP_FOURCC_IMAGE_BGR888:
                        r[0] = src_pix[0], g[0] = src_pix[1], b[0] = src_pix[2];
                        if(is_end_col == MMP_FALSE) {
                            r[1] = src_pix[3], g[1] = src_pix[4], b[1] = src_pix[5];
                        }
                        else {
                            r[1] = r[0], g[1] = g[0], b[1] = b[0];
                        }
                        break;

                    case MMP_FOURCC_IMAGE_RGB888: /* Byte0 -> B */
                    case MMP_FOURCC_IMAGE_ARGB8888:
                    default:
                        b[0] = src_pix[0], g[0] = src_pix[1], r[0] = src_pix[2];
                        if(is_end_col == MMP_FALSE) {
                            b[1] = src_pix[3], g[1] = src_pix[4], r[1] = src_pix[5];
                        }
                        else {
                            r[1] = r[0], g[1] = g[0], b[1] = b[0];
                        }
                        break;
                }

                for(i = 0; i < 2; i++) {
                    r[i]&=0xff, g[i]&=0xff, b[i]&=0xff;
                    u[i]= ((-38*r[i]-74*g[i]+112*b[i]+128)>>8)+128;
                    v[i]= ((112*r[i]-94*g[i]-18*b[i]+128)>>8)+128;
                }
                
                dst_pix[0] = (MMP_U8)((u[0] + u[1])/2);
                dst_pix[1] = (MMP_U8)((v[0] + v[1])/2);
            
                src_pix += rgb_pixel_byte*2;
                dst_pix += 2;
            }

            src_line += rgb_stride;
            dst_line += uv_stride;
        }
    }

    return mmpResult;
}


MMP_RESULT CMmpImageTool::ConvertRGBtoNV16MV(MMP_U8* p_image_rgb, MMP_S32 pic_width, MMP_S32 pic_height,enum MMP_FOURCC fourcc_rgb,
                                                  MMP_U8* p_image_y, MMP_U8* p_image_uv,
                                                  MMP_S32 y_stride, MMP_S32 uv_stride) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 w, h;
    MMP_S32 rgb_stride, rgb_pixel_byte;
    
    MMP_U8 *p_image_src1;
    MMP_U8 *p_line_src1;
    
    MMP_U8 *p_image_dest_y;
    MMP_U8 *p_line_dest_y;

    MMP_S32 y;
    MMP_S32 r[2]={0}, g[2]={0}, b[2]={0};
    MMP_S32 u[2]={0}, v[2]={0};

    MMP_S32 i;
    
    if(CMmpImageTool::IsRGB(fourcc_rgb) != MMP_TRUE) {
        mmpResult = MMP_FAILURE;
    }

    rgb_stride = CMmpImageTool::GetPicStride(fourcc_rgb, pic_width);
    rgb_pixel_byte = CMmpImageTool::GetPixelByte(fourcc_rgb);

    /* convert Y */
    if(mmpResult == MMP_SUCCESS) {
        
        p_image_src1 = p_image_rgb;
        p_image_dest_y = p_image_y;
        for(h = 0; h < pic_height; h++) {
        
            p_line_src1 = p_image_src1;
            p_line_dest_y = p_image_dest_y;
            for(w = 0; w < pic_width; w++) {
                
                switch(fourcc_rgb) {
                    case MMP_FOURCC_IMAGE_BGR888: /* Byte0 => R */
                        r[0] = p_line_src1[0];
                        g[0] = p_line_src1[1];
                        b[0] = p_line_src1[2];
                        break;

                    case MMP_FOURCC_IMAGE_RGB888:  /* Byte0 => B */
                        b[0] = p_line_src1[0];
                        g[0] = p_line_src1[1];
                        r[0] = p_line_src1[2];
                        break;

                    default: 
                        r[0] = g[0] = b[0] = 0;
                }

                r[0]&=0xff, g[0]&=0xff, b[0]&=0xff;
                y= ((66*r[0]+129*g[0]+25*b[0]+128)>>8)+16;

                (*p_line_dest_y) = (MMP_U8)y;
            
                p_line_src1 += rgb_pixel_byte;
                p_line_dest_y++;
            }

            p_image_src1 += rgb_stride;
            p_image_dest_y += y_stride;
        }
    }

    /* convert U, V */
    /*
         [u0,v0] [u1,v1] ..
    */
    if(mmpResult == MMP_SUCCESS) {
        
        MMP_U8* src_line0, *src_pix0;
        MMP_U8* src_line1, *src_pix1;
        MMP_U8* dst_line, *dst_pix;
        
        src_line0 = p_image_rgb;
        src_line1 = src_line0 + rgb_stride;
        dst_line = p_image_uv;
        for(h = 0; h < pic_height; h+=2) {
        
            src_pix0 = src_line0;
            src_pix1 = src_line1;
            dst_pix = dst_line;
                        
            for(w = 0; w < pic_width; w++ ) {

                switch(fourcc_rgb) {
                    case MMP_FOURCC_IMAGE_BGR888:
                        r[0] = src_pix0[0], g[0] = src_pix0[1], b[0] = src_pix0[2];
                        r[1] = src_pix1[0], g[1] = src_pix1[1], b[1] = src_pix1[2];
                        break;

                    case MMP_FOURCC_IMAGE_RGB888: /* Byte0 -> B */
                    case MMP_FOURCC_IMAGE_ARGB8888:
                    default:
                        b[0] = src_pix0[0], g[0] = src_pix0[1], r[0] = src_pix0[2];
                        b[1] = src_pix1[0], g[1] = src_pix1[1], r[1] = src_pix1[2];
                        break;
                }

                for(i = 0; i < 2; i++) {
                    r[i]&=0xff, g[i]&=0xff, b[i]&=0xff;
                    u[i]= ((-38*r[i]-74*g[i]+112*b[i]+128)>>8)+128;
                    v[i]= ((112*r[i]-94*g[i]-18*b[i]+128)>>8)+128;
                }
                
                dst_pix[0] = (MMP_U8)((u[0] + u[1])/2);
                dst_pix[1] = (MMP_U8)((v[0] + v[1])/2);
            
                src_pix0 += rgb_pixel_byte;
                src_pix1 += rgb_pixel_byte;
                dst_pix += 2;
            }

            src_line0 += rgb_stride*2;
            src_line1 += rgb_stride*2;
            dst_line += uv_stride;
        }
    }

    return mmpResult;
}

MMP_RESULT CMmpImageTool::ConvertRGBtoNV61(MMP_U8* p_image_rgb, MMP_S32 pic_width, MMP_S32 pic_height,enum MMP_FOURCC fourcc_rgb,
                                           MMP_U8* p_image_nv61) {
    MMP_U8* p_image_y, *p_image_uv;
    MMP_S32 y_stride, uv_stride;

    p_image_y = p_image_nv61;
    p_image_uv= p_image_nv61 + V4L2_NV61_UV_FRAME_OFFSET(pic_width, pic_height);
    y_stride = V4L2_NV61_Y_STRIDE(pic_width);
    uv_stride = V4L2_NV61_UV_STRIDE(pic_width);

    return CMmpImageTool::ConvertRGBtoNV61M(p_image_rgb, pic_width, pic_height, fourcc_rgb,
                                            p_image_y, p_image_uv,
                                            y_stride, uv_stride);
}

MMP_RESULT CMmpImageTool::ConvertRGBtoNV61M(MMP_U8* p_image_rgb, MMP_S32 pic_width, MMP_S32 pic_height,enum MMP_FOURCC fourcc_rgb,
                                                  MMP_U8* p_image_y, MMP_U8* p_image_uv,
                                                  MMP_S32 y_stride, MMP_S32 uv_stride) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 w, h;
    MMP_S32 rgb_stride, rgb_pixel_byte;
    
    MMP_U8 *p_image_src1;
    MMP_U8 *p_line_src1;
    
    MMP_U8 *p_image_dest_y;
    MMP_U8 *p_line_dest_y;

    MMP_S32 y;
    MMP_S32 r[2]={0}, g[2]={0}, b[2]={0};
    MMP_S32 u[2]={0}, v[2]={0};

    MMP_S32 i;
    MMP_BOOL is_end_col;

    if(CMmpImageTool::IsRGB(fourcc_rgb) != MMP_TRUE) {
        mmpResult = MMP_FAILURE;
    }

    rgb_stride = CMmpImageTool::GetPicStride(fourcc_rgb, pic_width);
    rgb_pixel_byte = CMmpImageTool::GetPixelByte(fourcc_rgb);

    /* convert Y */
    if(mmpResult == MMP_SUCCESS) {
        
        p_image_src1 = p_image_rgb;
        p_image_dest_y = p_image_y;
        for(h = 0; h < pic_height; h++) {
        
            p_line_src1 = p_image_src1;
            p_line_dest_y = p_image_dest_y;
            for(w = 0; w < pic_width; w++) {
                
                switch(fourcc_rgb) {
                    case MMP_FOURCC_IMAGE_BGR888: /* Byte0 -> R*/
                        r[0] = p_line_src1[0];
                        g[0] = p_line_src1[1];
                        b[0] = p_line_src1[2];
                        break;

                    case MMP_FOURCC_IMAGE_RGB888: /* Byte0 -> B*/
                        b[0] = p_line_src1[0];
                        g[0] = p_line_src1[1];
                        r[0] = p_line_src1[2];
                        break;

                    default: 
                        r[0] = g[0] = b[0] = 0;
                }

                r[0]&=0xff, g[0]&=0xff, b[0]&=0xff;
                y= ((66*r[0]+129*g[0]+25*b[0]+128)>>8)+16;

                (*p_line_dest_y) = (MMP_U8)y;
            
                p_line_src1 += rgb_pixel_byte;
                p_line_dest_y++;
            }

            p_image_src1 += rgb_stride;
            p_image_dest_y += y_stride;
        }
    }

    /* convert U, V */
    /*
         [v0,u0] [v1,u1] ..
    */
    if(mmpResult == MMP_SUCCESS) {
        
        MMP_U8* src_line, *src_pix;
        MMP_U8* dst_line, *dst_pix;

        src_line = p_image_rgb;
        dst_line = p_image_uv;
        for(h = 0; h < pic_height; h++) {
        
            src_pix = src_line;
            dst_pix = dst_line;
                        
            for(w = 0; w < pic_width; w+=2 ) {
             
                /* if odd height, h is last line */
                if( ((pic_width%2)==1) && (w==(pic_width-1)) ){
                    is_end_col = MMP_TRUE;
                }
                else {
                    is_end_col = MMP_FALSE;
                }

                /*
                       u0, u1 
                       u2  u3   ==> U
                 */
                switch(fourcc_rgb) {
                    case MMP_FOURCC_IMAGE_BGR888:
                        r[0] = src_pix[0], g[0] = src_pix[1], b[0] = src_pix[2];
                        if(is_end_col == MMP_FALSE) {
                            r[1] = src_pix[3], g[1] = src_pix[4], b[1] = src_pix[5];
                        }
                        else {
                            r[1] = r[0], g[1] = g[0], b[1] = b[0];
                        }
                        break;

                    case MMP_FOURCC_IMAGE_RGB888:
                    case MMP_FOURCC_IMAGE_ARGB8888:
                    default:
                        b[0] = src_pix[0], g[0] = src_pix[1], r[0] = src_pix[2];
                        if(is_end_col == MMP_FALSE) {
                            b[1] = src_pix[3], g[1] = src_pix[4], r[1] = src_pix[5];
                        }
                        else {
                            r[1] = r[0], g[1] = g[0], b[1] = b[0];
                        }
                        break;
                }

                for(i = 0; i < 2; i++) {
                    r[i]&=0xff, g[i]&=0xff, b[i]&=0xff;
                    u[i]= ((-38*r[i]-74*g[i]+112*b[i]+128)>>8)+128;
                    v[i]= ((112*r[i]-94*g[i]-18*b[i]+128)>>8)+128;
                }
         
                dst_pix[1] = (MMP_U8)((u[0] + u[1])/2);
                dst_pix[0] = (MMP_U8)((v[0] + v[1])/2);
            
                src_pix += rgb_pixel_byte*2;
                dst_pix += 2;
            }

            src_line += rgb_stride;
            dst_line += uv_stride;
        }
    }

    return mmpResult;
}


MMP_RESULT CMmpImageTool::ConvertRGBtoNV12(MMP_U8* p_image_rgb, MMP_S32 pic_width, MMP_S32 pic_height,enum MMP_FOURCC fourcc_rgb,
                                       MMP_U8* p_image_nv12) {

    MMP_U8* p_image_y, *p_image_uv;
    MMP_S32 y_stride, uv_stride;
    MMP_S32 offset;

    p_image_y = p_image_nv12;
    offset = V4L2_NV12_UV_FRAME_OFFSET(pic_width, pic_height);
    p_image_uv = p_image_nv12 + offset;
    y_stride = V4L2_NV12_Y_STRIDE(pic_width);
    uv_stride = V4L2_NV12_UV_STRIDE(pic_width);

    return CMmpImageTool::ConvertRGBtoNV12M(p_image_rgb, pic_width, pic_height,fourcc_rgb,
                                            p_image_y, p_image_uv, y_stride, uv_stride);
}

MMP_RESULT CMmpImageTool::ConvertRGBtoNV21(MMP_U8* p_image_rgb, MMP_S32 pic_width, MMP_S32 pic_height,enum MMP_FOURCC fourcc_rgb,
                                       MMP_U8* p_image_nv21) {

    MMP_U8* p_image_y, *p_image_uv;
    MMP_S32 y_stride, uv_stride;

    p_image_y = p_image_nv21;
    p_image_uv = p_image_nv21 + V4L2_NV21_UV_FRAME_OFFSET(pic_width, pic_height);
    y_stride = V4L2_NV21_Y_STRIDE(pic_width);
    uv_stride = V4L2_NV21_UV_STRIDE(pic_width);

    return CMmpImageTool::ConvertRGBtoNV21M(p_image_rgb, pic_width, pic_height,fourcc_rgb,
                                            p_image_y, p_image_uv, y_stride, uv_stride);
}
    
MMP_RESULT CMmpImageTool::ConvertRGBtoNV21M(MMP_U8* p_image_rgb, MMP_S32 pic_width, MMP_S32 pic_height,enum MMP_FOURCC fourcc_rgb,
                                                  MMP_U8* p_image_y, MMP_U8* p_image_uv,
                                                  MMP_S32 y_stride, MMP_S32 uv_stride
                                                  ) {

    return CMmpImageTool::ConvertRGBtoNV12M(p_image_rgb, pic_width, pic_height, fourcc_rgb,
                                            p_image_y, p_image_uv,
                                            y_stride, uv_stride,
                                            MMP_TRUE);
           
}


MMP_RESULT CMmpImageTool::ConvertRGBtoNV12M(MMP_U8* p_image_rgb, MMP_S32 pic_width, MMP_S32 pic_height,enum MMP_FOURCC fourcc_rgb,
                                                  MMP_U8* p_image_y, MMP_U8* p_image_uv,
                                                  MMP_S32 y_stride, MMP_S32 uv_stride,
                                                  MMP_BOOL is_NV21M
                                                  ) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 w, h;
    MMP_S32 rgb_stride, rgb_pixel_byte;
    
    MMP_U8 *p_image_src1;
    MMP_U8 *p_line_src1;
    
    MMP_U8 *p_image_dest_y;
    MMP_U8 *p_line_dest_y;

    MMP_S32 y;
    MMP_S32 r[4]={0}, g[4]={0}, b[4]={0};
    MMP_S32 u[4]={0}, v[4]={0};

    MMP_S32 i;
    MMP_BOOL is_end_col;

    MMP_S32 u_idx = 0;
    MMP_S32 v_idx = 1;

    if(is_NV21M == MMP_TRUE) {
        u_idx = 1;
        v_idx = 0;
    }

    if(CMmpImageTool::IsRGB(fourcc_rgb) != MMP_TRUE) {
        mmpResult = MMP_FAILURE;
    }

    rgb_stride = CMmpImageTool::GetPicStride(fourcc_rgb, pic_width);
    rgb_pixel_byte = CMmpImageTool::GetPixelByte(fourcc_rgb);

    /* convert Y */
    if(mmpResult == MMP_SUCCESS) {
        
        p_image_src1 = p_image_rgb;
        p_image_dest_y = p_image_y;
        for(h = 0; h < pic_height; h++) {
        
            p_line_src1 = p_image_src1;
            p_line_dest_y = p_image_dest_y;
            for(w = 0; w < pic_width; w++) {
                
                switch(fourcc_rgb) {
                    case MMP_FOURCC_IMAGE_BGR888: /* byte0 => R */
                        r[0] = p_line_src1[0];
                        g[0] = p_line_src1[1];
                        b[0] = p_line_src1[2];
                        break;

                    case MMP_FOURCC_IMAGE_RGB888:  /* byte0 => B */
                        b[0] = p_line_src1[0];
                        g[0] = p_line_src1[1];
                        r[0] = p_line_src1[2];
                        break;

                    default: 
                        r[0] = g[0] = b[0] = 0;
                }

                r[0]&=0xff, g[0]&=0xff, b[0]&=0xff;
                y= ((66*r[0]+129*g[0]+25*b[0]+128)>>8)+16;

                (*p_line_dest_y) = (MMP_U8)y;
            
                p_line_src1 += rgb_pixel_byte;
                p_line_dest_y++;
            }

            p_image_src1 += rgb_stride;
            p_image_dest_y += y_stride;
        }
    }

    /* convert U, V */
    /*
         u0, u1 
         u2  u3   ==> U
    */
    if(mmpResult == MMP_SUCCESS) {
        
        MMP_U8 *src_line0, *src_line1;
        MMP_U8 *src_pix0, *src_pix1;
        MMP_U8 *dst_line, *dst_pix;

        src_line0 = p_image_rgb;
        src_line1 = p_image_rgb + rgb_stride;
        dst_line = p_image_uv;
        for(h = 0; h < pic_height; h+=2) {
        
            src_pix0 = src_line0;
            src_pix1 = src_line1;
            
            /* if odd height, h is last line */
            if( ((pic_height%2)==1) && (h==(pic_height-1)) ){
                src_pix1 = src_pix0;
            }
            dst_pix = dst_line;

            for(w = 0; w < pic_width; w+=2 ) {
             
                /* if odd height, h is last line */
                if( ((pic_width%2)==1) && (w==(pic_width-1)) ){
                    is_end_col = MMP_TRUE;
                }
                else {
                    is_end_col = MMP_FALSE;
                }

                /*
                       u0, u1 
                       u2  u3   ==> U
                 */
                switch(fourcc_rgb) {
                    case MMP_FOURCC_IMAGE_BGR888:
                        b[0] = src_pix0[0], g[0] = src_pix0[1], r[0] = src_pix0[2];
                        b[2] = src_pix1[0], g[2] = src_pix1[1], r[2] = src_pix1[2];
                        if(is_end_col == MMP_FALSE) {
                            b[1] = src_pix0[3], g[1] = src_pix0[4], r[1] = src_pix0[5];
                            b[3] = src_pix1[3], g[3] = src_pix1[4], r[3] = src_pix1[5];
                        }
                        else {
                            b[1] = b[0], g[1] = g[0], r[1] = r[0];
                            b[3] = b[2], g[3] = g[2], r[3] = r[2];
                        }
                        break;

                    case MMP_FOURCC_IMAGE_RGB888:
                        b[0] = src_pix0[0], g[0] = src_pix0[1], r[0] = src_pix0[2];
                        b[2] = src_pix1[0], g[2] = src_pix1[1], r[2] = src_pix1[2];
                        if(is_end_col == MMP_FALSE) {
                            b[1] = src_pix0[3], g[1] = src_pix0[4], r[1] = src_pix0[5];
                            b[3] = src_pix1[3], g[3] = src_pix1[4], r[3] = src_pix1[5];
                        }
                        else {
                            r[1] = r[0], g[1] = g[0], b[1] = b[0];
                            r[3] = r[2], g[3] = g[2], b[3] = b[2];
                        }
                        break;

                    default: 
                        r[0] = g[0] = b[0] = 0;
                        r[1] = g[1] = b[1] = 0;
                        r[2] = g[2] = b[2] = 0;
                        r[3] = g[3] = b[3] = 0;
                }

                for(i = 0; i < 4; i++) {
                    r[i]&=0xff, g[i]&=0xff, b[i]&=0xff;
                    u[i]= ((-38*r[i]-74*g[i]+112*b[i]+128)>>8)+128;
                    v[i]= ((112*r[i]-94*g[i]-18*b[i]+128)>>8)+128;
                }
         
                dst_pix[u_idx] = (MMP_U8)((u[0] + u[1] + u[2] + u[3])/4);
                dst_pix[v_idx] = (MMP_U8)((v[0] + v[1] + v[2] + v[3])/4);
            
                src_pix0 += rgb_pixel_byte*2;
                src_pix1 += rgb_pixel_byte*2;
                
                dst_pix+=2;
            }

            src_line0 += rgb_stride*2;
            src_line1 += rgb_stride*2;

            dst_line += uv_stride;
        }
    }

    return mmpResult;
}



MMP_RESULT CMmpImageTool::ConvertRGBtoYUV444_P1(MMP_U8* p_rgb_data, MMP_S32 pic_width, MMP_S32 pic_height, enum MMP_FOURCC fourcc_rgb,
                                       MMP_U8* p_yuv_data, MMP_S32 yuv_stride) {
    
    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 rgb_stride;
    
    /* check fourcc */
    if(mmpResult == MMP_SUCCESS) {
        
        if(fourcc_rgb == MMP_FOURCC_IMAGE_RGB888) {
            rgb_stride = MMP_BYTE_ALIGN(pic_width*3, 4);
        }
        else if(fourcc_rgb == MMP_FOURCC_IMAGE_BGR888) {
            rgb_stride = MMP_BYTE_ALIGN(pic_width*3, 4);
        }
        else if(fourcc_rgb == MMP_FOURCC_IMAGE_ARGB8888) {
            rgb_stride = pic_width*4;
        }
        else {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpImageTool::%s] FAIL: not supportsrc RGB %c%c%c%c"), MMP_CLASS_FUNC, MMPGETFOURCCARG(fourcc_rgb) ));
        }
    }

    /* convert Y/U/V */
    if(mmpResult == MMP_SUCCESS) {
        
        MMP_S32 w, h;
        MMP_U8 *src_line, *dst_line;
        MMP_U8 *src_pixel, *dst_pixel;
        MMP_S32 r, g, b;
        MMP_S32 y, u, v;

        src_line = p_rgb_data;
        dst_line = p_yuv_data;
        for(h = 0; h < pic_height; h++) {
        
            src_pixel = src_line;
            dst_pixel = dst_line;
            for(w = 0; w < pic_width; w++) {
                
                switch(fourcc_rgb) {
                    case MMP_FOURCC_IMAGE_BGR888: /* Byte0 -> R */
                        r = (*src_pixel); src_pixel++;
                        g = (*src_pixel); src_pixel++;
                        b = (*src_pixel); src_pixel++;
                        break;

                    case MMP_FOURCC_IMAGE_ARGB8888:  /* Byte0 -> B */
                        b = (*src_pixel); src_pixel++;
                        g = (*src_pixel); src_pixel++;
                        r = (*src_pixel); src_pixel++;
                        src_pixel++;
                        break;

                    case MMP_FOURCC_IMAGE_RGB888:   /* Byte0 -> B */
                        b = (*src_pixel); src_pixel++;
                        g = (*src_pixel); src_pixel++;
                        r = (*src_pixel); src_pixel++;
                        break;

                    default: 
                        r = g = b = 0;
                }

                r&=0xff, g&=0xff, b&=0xff;
                y = ((66*r+129*g+25*b+128)>>8)+16;
                u = ((-38*r-74*g+112*b+128)>>8)+128;
                v = ((112*r-94*g-18*b+128)>>8)+128;

                (*dst_pixel) = (MMP_U8)y; dst_pixel++;
                (*dst_pixel) = (MMP_U8)u; dst_pixel++;
                (*dst_pixel) = (MMP_U8)v; dst_pixel++;
            }

            src_line += rgb_stride;
            dst_line += yuv_stride;
        }
    }

    
    return mmpResult;
}

MMP_RESULT CMmpImageTool::ConvertRGBtoGREY(MMP_U8* p_rgb_data, MMP_S32 pic_width, MMP_S32 pic_height, enum MMP_FOURCC fourcc_rgb,
                                       MMP_U8* p_grey_data, MMP_S32 grey_stride) {
    
    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 rgb_stride;
    
    /* check fourcc */
    if(mmpResult == MMP_SUCCESS) {
        
        if(fourcc_rgb == MMP_FOURCC_IMAGE_RGB888) {
            rgb_stride = MMP_BYTE_ALIGN(pic_width*3, 4);
        }
        else if(fourcc_rgb == MMP_FOURCC_IMAGE_ARGB8888) {
            rgb_stride = pic_width*4;
        }
        else {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpImageTool::%s] FAIL: not supportsrc RGB %c%c%c%c"), MMP_CLASS_FUNC, MMPGETFOURCCARG(fourcc_rgb) ));
        }
    }

    /* convert Y/U/V */
    if(mmpResult == MMP_SUCCESS) {
        
        MMP_S32 w, h;
        MMP_U8 *src_line, *dst_line;
        MMP_U8 *src_pixel, *dst_pixel;
        MMP_S32 r, g, b;
        MMP_S32 y;

        src_line = p_rgb_data;
        dst_line = p_grey_data;
        for(h = 0; h < pic_height; h++) {
        
            src_pixel = src_line;
            dst_pixel = dst_line;
            for(w = 0; w < pic_width; w++) {
                
                switch(fourcc_rgb) {
                    case MMP_FOURCC_IMAGE_BGR888: /* byte0 -> R */
                        r = (*src_pixel); src_pixel++;
                        g = (*src_pixel); src_pixel++;
                        b = (*src_pixel); src_pixel++;
                        break;

                    case MMP_FOURCC_IMAGE_ARGB8888:  /* byte0 -> B */
                        b = (*src_pixel); src_pixel++;
                        g = (*src_pixel); src_pixel++;
                        r = (*src_pixel); src_pixel++;
                        src_pixel++;
                        break;

                    case MMP_FOURCC_IMAGE_RGB888:  /* byte0 -> B */
                        b = (*src_pixel); src_pixel++;
                        g = (*src_pixel); src_pixel++;
                        r = (*src_pixel); src_pixel++;
                        break;

                    default: 
                        r = g = b = 0;
                }

                r&=0xff, g&=0xff, b&=0xff;
                y = ((66*r+129*g+25*b+128)>>8)+16;
                
                (*dst_pixel) = (MMP_U8)y; dst_pixel++;
            }

            src_line += rgb_stride;
            dst_line += grey_stride;
        }
    }

    
    return mmpResult;
}

MMP_RESULT CMmpImageTool::ConvertRGB32toRGB24(MMP_U8* p_rgb_src, MMP_S32 pic_width, MMP_S32 pic_height, enum MMP_FOURCC fourcc_src,
                                      MMP_U8* p_rgb_dst, enum MMP_FOURCC fourcc_dst) {

    MMP_S32 w, h;
    MMP_U8 *src_line;
    MMP_U8 *dst_line;

    MMP_U8 *src_pix;
    MMP_U8 *dst_pix;

    MMP_S32 src_stride, dst_stride;

    src_stride = pic_width*4;
    dst_stride = MMP_BYTE_ALIGN(pic_width*3, 4);
    src_line = p_rgb_src;
    dst_line = p_rgb_dst;
    for(h = 0; h < pic_height; h++) {
    
        src_pix = src_line;
        dst_pix = dst_line;
        for(w = 0; w < pic_width; w++) {

            dst_pix[0] = src_pix[0];
            dst_pix[1] = src_pix[1];
            dst_pix[2] = src_pix[2];
            
            src_pix += 4;
            dst_pix += 3;
        }

        src_line += src_stride;
        dst_line += dst_stride;
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpImageTool::ConvertGREYtoYUV444P1(MMP_U8* yuv420, 
                                                MMP_S32 pic_width, MMP_S32 pic_height, 
                                                MMP_U8* yuv444, MMP_S32 yuv444_stride) {
    MMP_U8 *yuv420_y, *yuv420_u, *yuv420_v;
    MMP_S32 y_stride, u_stride, v_stride;
    MMP_RESULT mmpResult;

    yuv420_y = yuv420;
    yuv420_u = (MMP_U8*)MMP_MALLOC( V4L2_YU12_U_FRAME_SIZE(pic_width, pic_height));
    yuv420_v = (MMP_U8*)MMP_MALLOC( V4L2_YU12_U_FRAME_SIZE(pic_width, pic_height));
    memset(yuv420_u, 128, V4L2_YU12_U_FRAME_SIZE(pic_width, pic_height) );
    memset(yuv420_v, 128, V4L2_YU12_V_FRAME_SIZE(pic_width, pic_height) );

    y_stride = V4L2_YU12_Y_STRIDE(pic_width);
    u_stride = V4L2_YU12_U_STRIDE(pic_width);
    v_stride = V4L2_YU12_V_STRIDE(pic_width);

    mmpResult = CMmpImageTool::ConvertYUV420MtoYUV444P1(yuv420_y, yuv420_u, yuv420_v, 
                                                   y_stride, u_stride, v_stride,
                                                   pic_width, pic_height, 
                                                   yuv444, yuv444_stride);
    MMP_FREE(yuv420_u);
    MMP_FREE(yuv420_v);

    return mmpResult;
}
    
MMP_RESULT CMmpImageTool::ConvertYUV420toYUV444P1(MMP_U8* yuv420, 
                                                MMP_S32 pic_width, MMP_S32 pic_height, 
                                                MMP_U8* yuv444, MMP_S32 yuv444_stride) {
    MMP_U8 *yuv420_y, *yuv420_u, *yuv420_v;
    MMP_S32 y_stride, u_stride, v_stride;

    yuv420_y = yuv420;
    yuv420_u = yuv420 + V4L2_YU12_U_FRAME_OFFSET(pic_width, pic_height);
    yuv420_v = yuv420 + V4L2_YU12_V_FRAME_OFFSET(pic_width, pic_height);

    y_stride = V4L2_YU12_Y_STRIDE(pic_width);
    u_stride = V4L2_YU12_U_STRIDE(pic_width);
    v_stride = V4L2_YU12_V_STRIDE(pic_width);

    return CMmpImageTool::ConvertYUV420MtoYUV444P1(yuv420_y, yuv420_u, yuv420_v, 
                                                   y_stride, u_stride, v_stride,
                                                   pic_width, pic_height, 
                                                   yuv444, yuv444_stride);
}

MMP_RESULT CMmpImageTool::ConvertYUV420MtoYUV444P1(MMP_U8* yuv420_y, MMP_U8* yuv420_u, MMP_U8* yuv420_v, 
                                                    MMP_S32 y_stride, MMP_S32 u_stride, MMP_S32 v_stride,
                                                MMP_S32 pic_width, MMP_S32 pic_height, 
                                                MMP_U8* yuv444, MMP_S32 yuv444_stride) {

    MMP_S32 w, h;
    MMP_U8 *src_line_y, *src_line_u, *src_line_v;
    MMP_U8 *dst_line;

    MMP_U8 *src_pix_y, *src_pix_u, *src_pix_v;
    MMP_U8 *dst_pix;

    src_line_y = yuv420_y;
    src_line_u = yuv420_u;
    src_line_v = yuv420_v;
    dst_line = yuv444;
    for(h = 0; h < pic_height; h++) {
    
        src_pix_y = src_line_y;
        src_pix_u = src_line_u;
        src_pix_v = src_line_v;
        dst_pix = dst_line;
        for(w = 0; w < pic_width; w++) {
         

            dst_pix[0] = (*src_pix_y);
            dst_pix[1] = (*src_pix_u);
            dst_pix[2] = (*src_pix_v);
            
            dst_pix += 3;
            src_pix_y++;
            if( ((w+1)%2) == 0) {
                src_pix_u++;
                src_pix_v++;
            }
        }

        src_line_y += y_stride;
        if( ((h+1)%2) == 0) {
            src_line_u += u_stride;
            src_line_v += v_stride;
        }
        dst_line += yuv444_stride;
    }

    return MMP_SUCCESS;
}


MMP_RESULT CMmpImageTool::ConvertNV21toYUV444P1(MMP_U8* p_image_src, 
                                            MMP_S32 pic_width, MMP_S32 pic_height, 
                                            MMP_U8* yuv444, MMP_S32 yuv444_stride)  {

    MMP_U8 *p_image_y, *p_image_uv;
    MMP_S32 y_stride, uv_stride;

    p_image_y = p_image_src + V4L2_NV21_Y_FRAME_OFFSET(pic_width, pic_height);
    p_image_uv = p_image_src + V4L2_NV21_UV_FRAME_OFFSET(pic_width, pic_height);

    y_stride = V4L2_NV21_Y_STRIDE(pic_width);
    uv_stride = V4L2_NV21_UV_STRIDE(pic_width);

    return CMmpImageTool::ConvertNV21MtoYUV444P1(p_image_y, p_image_uv, 
                                                y_stride, uv_stride,
                                                pic_width, pic_height, 
                                                yuv444, yuv444_stride);
}

MMP_RESULT CMmpImageTool::ConvertNV21MtoYUV444P1(MMP_U8* p_image_y, MMP_U8* p_image_uv, 
                                            MMP_S32 y_stride, MMP_S32 uv_stride,
                                            MMP_S32 pic_width, MMP_S32 pic_height, 
                                            MMP_U8* yuv444, MMP_S32 yuv444_stride
                                            ) {

    return CMmpImageTool::ConvertNV12MtoYUV444P1(p_image_y, p_image_uv, 
                                                y_stride, uv_stride,
                                                pic_width, pic_height, 
                                                yuv444, yuv444_stride,
                                                MMP_TRUE);

}


MMP_RESULT CMmpImageTool::ConvertNV12toYUV444P1(MMP_U8* p_image_src, 
                                                MMP_S32 pic_width, MMP_S32 pic_height, 
                                                MMP_U8* yuv444, MMP_S32 yuv444_stride) {

    MMP_U8 *p_image_y, *p_image_uv;
    MMP_S32 y_stride, uv_stride;

    p_image_y = p_image_src + V4L2_NV12_Y_FRAME_OFFSET(pic_width, pic_height);
    p_image_uv = p_image_src + V4L2_NV12_UV_FRAME_OFFSET(pic_width, pic_height);

    y_stride = V4L2_NV12_Y_STRIDE(pic_width);
    uv_stride = V4L2_NV12_UV_STRIDE(pic_width);

    return CMmpImageTool::ConvertNV12MtoYUV444P1(p_image_y, p_image_uv, 
                                                y_stride, uv_stride,
                                                pic_width, pic_height, 
                                                yuv444, yuv444_stride);
}
    
MMP_RESULT CMmpImageTool::ConvertNV12MtoYUV444P1(MMP_U8* p_image_y, MMP_U8* p_image_uv, 
                                                MMP_S32 y_stride, MMP_S32 uv_stride,
                                                MMP_S32 pic_width, MMP_S32 pic_height, 
                                                MMP_U8* yuv444, MMP_S32 yuv444_stride,
                                                MMP_BOOL is_NV21M) {

    MMP_S32 w, h;
    MMP_U8 *src_line_y, *src_line_uv;
    MMP_U8 *dst_line;

    MMP_U8 *src_pix_y, *src_pix_uv;
    MMP_U8 *dst_pix;

    MMP_S32 u_idx, v_idx;

    if(is_NV21M == MMP_FALSE) {
        u_idx = 0;
        v_idx = 1;
    }
    else {
        u_idx = 1;
        v_idx = 0;
    }

    src_line_y = p_image_y;
    src_line_uv = p_image_uv;
    dst_line = yuv444;
    for(h = 0; h < pic_height; h++) {
    
        src_pix_y = src_line_y;
        src_pix_uv = src_line_uv;
        dst_pix = dst_line;
        for(w = 0; w < pic_width; w++) {
         

            dst_pix[0] = (*src_pix_y);
            dst_pix[1] = (*(src_pix_uv+u_idx));
            dst_pix[2] = (*(src_pix_uv+v_idx));

            dst_pix += 3;
            src_pix_y++;
            if( ((w+1)%2) == 0) {
                src_pix_uv+=2;
            }
        }

        src_line_y += y_stride;
        if( ((h+1)%2) == 0) {
            src_line_uv += uv_stride;
        }
        dst_line += yuv444_stride;
    }

    return MMP_SUCCESS;
}

/* YUV 4:2:2 packed  */
MMP_RESULT CMmpImageTool::ConvertYUYVtoYUV444P1(MMP_U8* p_image_src, enum MMP_FOURCC fourcc_src,
                                                MMP_S32 pic_width, MMP_S32 pic_height, 
                                                MMP_U8* yuv444, MMP_S32 yuv444_stride) {
    
    MMP_U8 *p_image_y, *p_image_u, *p_image_v;
    MMP_S32 y_stride, u_stride, v_stride, yuyv_stride;
    MMP_S32 w, h;

    MMP_U8 *line_y, *line_u, *line_v;
    MMP_U8 *pix_y, *pix_u, *pix_v;

    MMP_U8 *line_src, *pix_src;

    MMP_S32 y0_idx, y1_idx, u_idx, v_idx;

    MMP_RESULT mmpResult = MMP_FAILURE;
    
    p_image_y = new MMP_U8[ V4L2_YUV422P_Y_FRAME_SIZE(pic_width, pic_height) ];
    p_image_u = new MMP_U8[ V4L2_YUV422P_U_FRAME_SIZE(pic_width, pic_height) ];
    p_image_v = new MMP_U8[ V4L2_YUV422P_V_FRAME_SIZE(pic_width, pic_height) ];
    
    y_stride = V4L2_YUV422P_Y_STRIDE(pic_width);
    u_stride = V4L2_YUV422P_U_STRIDE(pic_width);
    v_stride = V4L2_YUV422P_V_STRIDE(pic_width);

    yuyv_stride = V4L2_YUYV_STRIDE(pic_width);

    switch(fourcc_src) {
        case V4L2_PIX_FMT_UYVY:
            y0_idx=1, y1_idx=3;
            u_idx=0, v_idx=2;
            break;

        case V4L2_PIX_FMT_VYUY:
            y0_idx=1, y1_idx=3;
            u_idx=2, v_idx=0;
            break;

        case V4L2_PIX_FMT_YUYV:
            y0_idx=0, y1_idx=2;
            u_idx=1, v_idx=3;
            break;

        case V4L2_PIX_FMT_YVYU:
        default:
            y0_idx=0, y1_idx=2;
            u_idx=3, v_idx=1;
            break;
    }

    if( (p_image_y!=NULL) && (p_image_u!=NULL) && (p_image_v!=NULL) ) {

        line_src = p_image_src;
        line_y = p_image_y;
        line_u = p_image_u;
        line_v = p_image_v;
        for(h = 0; h < pic_height; h++) {
        
            pix_y = line_y;
            pix_u = line_u;
            pix_v = line_v;

            pix_src = line_src;

            for(w = 0; w < pic_width; w+=2) {

                *(pix_y + 0) = pix_src[y0_idx]; /* Y0 */
                *(pix_y + 1) = pix_src[y1_idx]; /* Y1 */
                *pix_u = pix_src[u_idx];
                *pix_v = pix_src[v_idx];

                pix_u++;
                pix_v++;
                pix_y+=2;

                pix_src += 4;
            }

            line_y += y_stride;
            line_u += u_stride;
            line_v += v_stride;

            line_src += yuyv_stride;
        }

        mmpResult = CMmpImageTool::ConvertYUV422PMtoYUV444P1(p_image_y, p_image_u, p_image_v, 
                                                        y_stride, u_stride, v_stride,
                                                        pic_width, pic_height, 
                                                        yuv444, yuv444_stride);

    }
    if(p_image_y) delete p_image_y;
    if(p_image_u) delete p_image_u;
    if(p_image_v) delete p_image_v;

    return mmpResult;
}

/* YUV 4:2:2 planar   */
MMP_RESULT CMmpImageTool::ConvertYUV422PtoYUV444P1(MMP_U8* p_yuv422p, 
                                                MMP_S32 pic_width, MMP_S32 pic_height, 
                                                MMP_U8* yuv444, MMP_S32 yuv444_stride) {

    MMP_U8 *p_image_y, *p_image_u, *p_image_v;
    MMP_S32 y_stride, u_stride, v_stride;

    p_image_y = p_yuv422p + V4L2_YUV422P_Y_FRAME_OFFSET(pic_width, pic_height);
    p_image_u = p_yuv422p + V4L2_YUV422P_U_FRAME_OFFSET(pic_width, pic_height);
    p_image_v = p_yuv422p + V4L2_YUV422P_V_FRAME_OFFSET(pic_width, pic_height);;

    y_stride = V4L2_YUV422P_Y_STRIDE(pic_width);
    u_stride = V4L2_YUV422P_U_STRIDE(pic_width);
    v_stride = V4L2_YUV422P_V_STRIDE(pic_width);

    return CMmpImageTool::ConvertYUV422PMtoYUV444P1(p_image_y, p_image_u, p_image_v, 
                                                    y_stride, u_stride, v_stride,
                                                    pic_width, pic_height, 
                                                    yuv444, yuv444_stride);
}
    
MMP_RESULT CMmpImageTool::ConvertYUV422PMtoYUV444P1(MMP_U8* p_image_y, MMP_U8* p_image_u, MMP_U8* p_image_v, 
                                                    MMP_S32 y_stride, MMP_S32 u_stride, MMP_S32 v_stride,
                                                    MMP_S32 pic_width, MMP_S32 pic_height, 
                                                    MMP_U8* yuv444, MMP_S32 yuv444_stride) {

    MMP_S32 w, h;
    MMP_U8 *src_line_y, *src_line_u, *src_line_v;
    MMP_U8 *dst_line;

    MMP_U8 *src_pix_y, *src_pix_u, *src_pix_v;
    MMP_U8 *dst_pix;

    src_line_y = p_image_y;
    src_line_u = p_image_u;
    src_line_v = p_image_v;
    dst_line = yuv444;
    for(h = 0; h < pic_height; h++) {
    
        src_pix_y = src_line_y;
        src_pix_u = src_line_u;
        src_pix_v = src_line_v;
        dst_pix = dst_line;
        for(w = 0; w < pic_width; w++) {
         

            dst_pix[0] = (*src_pix_y);
            dst_pix[1] = (*src_pix_u);
            dst_pix[2] = (*src_pix_v);

            dst_pix += 3;
            src_pix_y++;
            if( ((w+1)%2) == 0) {
                src_pix_u++;
                src_pix_v++;
            }
        }

        src_line_y += y_stride;
        src_line_u += u_stride;
        src_line_v += v_stride;
        
        dst_line += yuv444_stride;
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpImageTool::ConvertNV61toYUV444P1(MMP_U8* p_nv61, 
                                        MMP_S32 pic_width, MMP_S32 pic_height, 
                                        MMP_U8* yuv444, MMP_S32 yuv444_stride) {

    MMP_U8 *p_image_y, *p_image_uv;
    MMP_S32 y_stride, uv_stride;

    p_image_y = p_nv61;
    p_image_uv = p_nv61 + V4L2_NV61_UV_FRAME_OFFSET(pic_width, pic_height);;
    
    y_stride = V4L2_NV61_Y_STRIDE(pic_width);
    uv_stride = V4L2_NV61_UV_STRIDE(pic_width);
    
    return CMmpImageTool::ConvertNV61MtoYUV444P1(p_image_y, p_image_uv, 
                                                 y_stride, uv_stride,
                                                pic_width, pic_height, 
                                                yuv444, yuv444_stride);
}

MMP_RESULT CMmpImageTool::ConvertNV61MtoYUV444P1(MMP_U8* p_image_y, MMP_U8* p_image_uv, 
                                                MMP_S32 y_stride, MMP_S32 uv_stride,
                                                MMP_S32 pic_width, MMP_S32 pic_height, 
                                                MMP_U8* yuv444, MMP_S32 yuv444_stride
                                                ) {

    return CMmpImageTool::ConvertNV16MtoYUV444P1(p_image_y, p_image_uv, 
                                                y_stride, uv_stride,
                                                pic_width, pic_height, 
                                                yuv444, yuv444_stride,
                                                MMP_TRUE);
           
}

MMP_RESULT CMmpImageTool::ConvertNV16toYUV444P1(MMP_U8* p_nv16, 
                                        MMP_S32 pic_width, MMP_S32 pic_height, 
                                        MMP_U8* yuv444, MMP_S32 yuv444_stride) {

    MMP_U8 *p_image_y, *p_image_uv;
    MMP_S32 y_stride, uv_stride;

    p_image_y = p_nv16;
    p_image_uv = p_nv16 + V4L2_NV16_UV_FRAME_OFFSET(pic_width, pic_height);;
    
    y_stride = V4L2_NV16_Y_STRIDE(pic_width);
    uv_stride = V4L2_NV16_UV_STRIDE(pic_width);
    
    return CMmpImageTool::ConvertNV16MtoYUV444P1(p_image_y, p_image_uv, 
                                                 y_stride, uv_stride,
                                                pic_width, pic_height, 
                                                yuv444, yuv444_stride);
}
    
MMP_RESULT CMmpImageTool::ConvertNV16VtoYUV444P1(MMP_U8* p_nv16v, 
                                        MMP_S32 pic_width, MMP_S32 pic_height, 
                                        MMP_U8* yuv444, MMP_S32 yuv444_stride) {

    MMP_U8 *p_image_y, *p_image_uv;
    MMP_S32 y_stride, uv_stride;

    p_image_y = p_nv16v;
    p_image_uv = p_nv16v + V4L2_NV16V_UV_FRAME_OFFSET(pic_width, pic_height);;
    
    y_stride = V4L2_NV16V_Y_STRIDE(pic_width);
    uv_stride = V4L2_NV16V_UV_STRIDE(pic_width);
    
    return CMmpImageTool::ConvertNV16MVtoYUV444P1(p_image_y, p_image_uv, 
                                                 y_stride, uv_stride,
                                                 pic_width, pic_height, 
                                                 yuv444, yuv444_stride);
}

MMP_RESULT CMmpImageTool::ConvertNV16MtoYUV444P1(MMP_U8* p_image_y, MMP_U8* p_image_uv, 
                                                MMP_S32 y_stride, MMP_S32 uv_stride,
                                                MMP_S32 pic_width, MMP_S32 pic_height, 
                                                MMP_U8* yuv444, MMP_S32 yuv444_stride,
                                                MMP_BOOL is_NV61M
                                                ) {

    MMP_S32 w, h;
    MMP_U8 *src_line_y, *src_line_uv;
    MMP_U8 *dst_line;

    MMP_U8 *src_pix_y, *src_pix_uv;
    MMP_U8 *dst_pix;

    MMP_S32 u_idx, v_idx;

    if(is_NV61M == MMP_FALSE) {
        u_idx = 0;
        v_idx = 1;
    }
    else {
        u_idx = 1;
        v_idx = 0;
    }

    src_line_y = p_image_y;
    src_line_uv = p_image_uv;
    dst_line = yuv444;
    for(h = 0; h < pic_height; h++) {
    
        src_pix_y = src_line_y;
        src_pix_uv = src_line_uv;
        dst_pix = dst_line;
        for(w = 0; w < pic_width; w++) {
         

            dst_pix[0] = (*src_pix_y);
            dst_pix[1] = (*(src_pix_uv + u_idx));
            dst_pix[2] = (*(src_pix_uv + v_idx));

            dst_pix += 3;
            src_pix_y++;
            if( ((w+1)%2) == 0) {
                src_pix_uv+=2;
            }
        }

        src_line_y += y_stride;
        src_line_uv += uv_stride;
        dst_line += yuv444_stride;
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpImageTool::ConvertNV16MVtoYUV444P1(MMP_U8* p_image_y, MMP_U8* p_image_uv, 
                                                MMP_S32 y_stride, MMP_S32 uv_stride,
                                                MMP_S32 pic_width, MMP_S32 pic_height, 
                                                MMP_U8* yuv444, MMP_S32 yuv444_stride,
                                                MMP_BOOL is_NV61M
                                                ) {

    MMP_S32 w, h;
    MMP_U8 *src_line_y, *src_line_uv;
    MMP_U8 *dst_line;

    MMP_U8 *src_pix_y, *src_pix_uv;
    MMP_U8 *dst_pix;

    MMP_S32 u_idx, v_idx;

    if(is_NV61M == MMP_FALSE) {
        u_idx = 0;
        v_idx = 1;
    }
    else {
        u_idx = 1;
        v_idx = 0;
    }

    src_line_y = p_image_y;
    src_line_uv = p_image_uv;
    dst_line = yuv444;
    for(h = 0; h < pic_height; h++) {
    
        src_pix_y = src_line_y;
        src_pix_uv = src_line_uv;
        dst_pix = dst_line;
        for(w = 0; w < pic_width; w++) {
         

            dst_pix[0] = (*src_pix_y);
            dst_pix[1] = (*(src_pix_uv + u_idx));
            dst_pix[2] = (*(src_pix_uv + v_idx));

            dst_pix += 3;
            src_pix_y++;
            src_pix_uv+=2;
        }

        src_line_y += y_stride;
        if( ((h+1)%2) == 0) {
            src_line_uv += uv_stride;
        }
        dst_line += yuv444_stride;
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpImageTool::ConvertYUV444P1toYUV420P3(MMP_U8* yuv444, MMP_S32 yuv444_stride, 
                                                MMP_S32 pic_width, MMP_S32 pic_height, 
                                                MMP_U8* yuv420_y, MMP_U8* yuv420_u, MMP_U8* yuv420_v, 
                                                MMP_S32 y_stride, MMP_S32 u_stride, MMP_S32 v_stride
                                                ) {

    MMP_S32 w, h, i;
    MMP_U8 *src_line, *src_line1;
    MMP_U8 *dst_line_y, *dst_line_u, *dst_line_v;
    
    MMP_U8 *src_pix, *src_pix1;
    MMP_U8 *dst_pix_y, *dst_pix_u, *dst_pix_v;

    MMP_U32 u[4], v[4];
    const MMP_S32 yuv444_pix_byte = 3;
        
    /* Set Y */
    src_line = yuv444;
    dst_line_y = yuv420_y;
    for(h = 0; h < pic_height; h++) {
        
        src_pix = src_line;
        dst_pix_y = dst_line_y;
        for(w = 0; w < pic_width; w++) {
        
            (*dst_pix_y) = (*src_pix);

            src_pix += yuv444_pix_byte;
            dst_pix_y++;
        }

        src_line += yuv444_stride;
        dst_line_y += y_stride;
    }

    /* Set U, V */
    src_line = yuv444;
    src_line1 = yuv444 + yuv444_stride;
    dst_line_u = yuv420_u;
    dst_line_v = yuv420_v;
    for(h = 0; h < pic_height; h+=2) {
        
        if( ((h%2) != 0) 
            && (h == pic_height-1) ) {
            src_line1 = src_line;
        }

        src_pix = src_line;
        src_pix1 = src_line1;
        dst_pix_u = dst_line_u;
        dst_pix_v = dst_line_v;
        for(w = 0; w < pic_width; w+=2) {
        
            if( ((w%2) != 0) 
                && (w == pic_width-1) ) {

                u[0] = (MMP_U32)src_pix[1], u[1] = (MMP_U32)src_pix[1 + 0];
                u[2] = (MMP_U32)src_pix1[1], u[3] = (MMP_U32)src_pix1[1+ 0];

                v[0] = (MMP_U32)src_pix[2], v[1] = (MMP_U32)src_pix[2 + 0];
                v[2] = (MMP_U32)src_pix1[2], v[3] = (MMP_U32)src_pix1[2 + 0];
            }
            else {
            
                u[0] = (MMP_U32)src_pix[1], u[1] = (MMP_U32)src_pix[1 + yuv444_pix_byte];
                u[2] = (MMP_U32)src_pix1[1], u[3] = (MMP_U32)src_pix1[1+ yuv444_pix_byte];

                v[0] = (MMP_U32)src_pix[2], v[1] = (MMP_U32)src_pix[2 + yuv444_pix_byte];
                v[2] = (MMP_U32)src_pix1[2], v[3] = (MMP_U32)src_pix1[2 + yuv444_pix_byte];
            }

            for(i = 0; i < 4; i++) {
                u[i] &= 0xFF;
                v[i] &= 0xFF;
            }

            (*dst_pix_u)  =  (MMP_U8)((u[0]+u[1]+u[2]+u[3])/4);
            (*dst_pix_v)  =  (MMP_U8)((v[0]+v[1]+v[2]+v[3])/4);
            
            src_pix += yuv444_pix_byte;
            src_pix1 += yuv444_pix_byte;
            dst_pix_u++;
            dst_pix_v++;
        }

        src_line += yuv444_stride*2;
        src_line1 += yuv444_stride*2;
        dst_line_u += u_stride;
        dst_line_v += v_stride;
    }

    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpImageTool::ConvertYUV422P2toYUV420P3(MMP_U8* yuv422_y, MMP_U8* yuv422_uv, 
                                                MMP_S32 yuv422_y_stride,  MMP_S32 yuv422_uv_stride,
                                                MMP_S32 pic_width, MMP_S32 pic_height, 
                                                MMP_U8* yuv420_y, MMP_U8* yuv420_u, MMP_U8* yuv420_v, 
                                                MMP_S32 y_stride, MMP_S32 u_stride, MMP_S32 v_stride
                                                ) {

    MMP_S32 w, h, i;
    MMP_U8 *src_line, *src_line1;
    MMP_U8 *dst_line_y, *dst_line_u, *dst_line_v;
    
    MMP_U8 *src_pix, *src_pix1;
    MMP_U8 *dst_pix_y, *dst_pix_u, *dst_pix_v;

    MMP_U32 u[2], v[2];
        
    /* Set Y */
    src_line = yuv422_y;
    dst_line_y = yuv420_y;
    for(h = 0; h < pic_height; h++) {
        
        src_pix = src_line;
        dst_pix_y = dst_line_y;
        for(w = 0; w < pic_width; w++) {
        
            (*dst_pix_y) = (*src_pix);

            src_pix ++;;
            dst_pix_y++;
        }

        src_line += yuv422_y_stride;
        dst_line_y += y_stride;
    }

    /* Set U, V */
    src_line = yuv422_uv;
    src_line1 = yuv422_uv + yuv422_uv_stride;
    dst_line_u = yuv420_u;
    dst_line_v = yuv420_v;
    for(h = 0; h < pic_height; h+=2) {
        
        if( ((h%2) != 0) 
            && (h == pic_height-1) ) {
            src_line1 = src_line;
        }

        src_pix = src_line;
        src_pix1 = src_line1;
        dst_pix_u = dst_line_u;
        dst_pix_v = dst_line_v;
        for(w = 0; w < pic_width; w+=2) {
        
            
            u[0] = (MMP_U32)src_pix[0], v[0] = (MMP_U32)src_pix[1];
            u[1] = (MMP_U32)src_pix1[0], v[1] = (MMP_U32)src_pix1[1];
            
            for(i = 0; i < 2; i++) {
                u[i] &= 0xFF;
                v[i] &= 0xFF;
            }

            (*dst_pix_u)  =  (MMP_U8)((u[0]+u[1])/2);
            (*dst_pix_v)  =  (MMP_U8)((v[0]+v[1])/2);
            
            src_pix += 2;
            src_pix1 += 2;
            dst_pix_u++;
            dst_pix_v++;
        }

        src_line += yuv422_uv_stride*2;
        src_line1 += yuv422_uv_stride*2;
        dst_line_u += u_stride;
        dst_line_v += v_stride;
    }

    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpImageTool::ConvertColor(MMP_U8* p_src_data, MMP_S32 pic_width, MMP_S32 pic_height, enum MMP_FOURCC fourcc_src,
                                       MMP_U8* p_dst_data, enum MMP_FOURCC fourcc_dst, MMP_S32 dst_stride) {
    
    MMP_RESULT mmpResult = MMP_FAILURE;
    
    switch(fourcc_src) {
        case MMP_FOURCC_IMAGE_RGB888:
            if(fourcc_dst == MMP_FOURCC_IMAGE_YUV444_P1) {
                mmpResult = CMmpImageTool::ConvertRGBtoYUV444_P1(p_src_data, pic_width, pic_height, fourcc_src, p_dst_data, dst_stride);
            }
            break;
    }
    
    return mmpResult;
}

/********************************************************** 
    Tool of JPEG
***********************************************************/
MMP_RESULT CMmpImageTool::Jpeg_GetWidthHeight(MMP_U8* filename, MMP_OUT MMP_S32 *pic_width, MMP_OUT MMP_S32* pic_height) {

    FILE* fp;
    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U8* jpegdata = NULL;
    MMP_U32 jpegsize = 0;
    

    fp = fopen((const char*)filename, "rb");
    if(fp != NULL) {

        fseek(fp, 0, SEEK_END);
        jpegsize = (MMP_U32)ftell(fp);
        fseek(fp, 0, SEEK_SET);

        jpegdata = (MMP_U8*)malloc(jpegsize);
        if(jpegdata != NULL) {
            fread(jpegdata, 1, jpegsize, fp);
        }
        fclose(fp);
    }

    if(jpegdata != NULL) {

        mmpResult = CMmpImageTool::Jpeg_GetWidthHeightFourcc(jpegdata, jpegsize, pic_width, pic_height, NULL);
        free(jpegdata);
    }
    
    return mmpResult;
}

MMP_RESULT CMmpImageTool::Jpeg_GetWidthHeightFourcc(MMP_U8* jpegdata, MMP_S32 jpegsize, 
                                                    MMP_OUT MMP_S32 *pic_width, MMP_OUT MMP_S32* pic_height, enum MMP_FOURCC* pfourcc) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U32 sof_offset;
    MMP_U8* p;
    MMP_U16 w,h;
    MMP_U8 num_of_component, component_id, sampling_frequecy, component_spec; 
    MMP_S32 hSampFact, vSampFact;
    MMP_S32 sampleFactor;
    enum MMP_FOURCC fourcc;

    enum {
	    SAMPLE_420 = 0xA,
	    SAMPLE_H422 = 0x9,
	    SAMPLE_V422 = 0x6,
	    SAMPLE_444 = 0x5,
	    SAMPLE_400 = 0x1
    };

    if(pic_width){ *pic_width = 0; }
    if(pic_height){ *pic_height = 0; }

    mmpResult = CMmpImageTool::Jpeg_Get_SOF_Offset(jpegdata, jpegsize, &sof_offset);
    if(mmpResult == MMP_SUCCESS) {
        
        p = &jpegdata[sof_offset];
        p+=5; /* skip code,len,sampling precision */
        memcpy(&h, p, 2); p+=2; /*width*/
        memcpy(&w, p, 2); p+=2; /*height*/

        num_of_component = (*p);     p++; /* Numboer of Component */
        component_id = (*p);         p++;
        sampling_frequecy = (*p);    p++;
        component_spec = (*p); p++;

        hSampFact = ((MMP_S32)(sampling_frequecy>>4)) & 0xf;
        vSampFact = ((MMP_S32)sampling_frequecy)&0xf;

        if(num_of_component == 1) {
            sampleFactor = SAMPLE_400;
        }
        else {
            sampleFactor = ((hSampFact&3)<<2) | (vSampFact&3);
        }

        switch(sampleFactor) {
	        case SAMPLE_420:
		        fourcc = MMP_FOURCC_IMAGE_YUV420M; // FORMAT_420;
		        break;

            case SAMPLE_H422:
		        fourcc = MMP_FOURCC_IMAGE_NV16; //FORMAT_422;
		        break;

	        case SAMPLE_V422:
		        fourcc = MMP_FOURCC_IMAGE_NV16; //FORMAT_224;
		        break;

	        case SAMPLE_444:
		        fourcc = MMP_FOURCC_IMAGE_YUV444_P1; //FORMAT_444;
		        break;
	        
            default:	// 4:0:0
		        fourcc = MMP_FOURCC_IMAGE_GREY;//FORMAT_400;
	    }
        
        w = MMP_SWAP_U16(w);
        h = MMP_SWAP_U16(h);

        if(pic_width){ *pic_width = ((MMP_U32)w)&0xFFFF; }
        if(pic_height){ *pic_height = ((MMP_U32)h)&0xFFFF; }
        if(pfourcc!=NULL) { *pfourcc = fourcc; }
    }

    return mmpResult;
}

/* search SOF (Start of Frame) */
MMP_RESULT CMmpImageTool::Jpeg_Get_SOF_Offset(MMP_U8* jpegdata, MMP_U32 jpegsize, MMP_OUT MMP_U32 *sof_offset) {
    
    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_U8* p;
    MMP_U16 code, len;
    MMP_S32 remain_byte;
    MMP_U32 offset;
    

    p = jpegdata;
    memcpy(&code, p, 2);
    if(code ==  JPEG_SOI_CODE ) {
        p+=2;
    }
    else {
        mmpResult = MMP_FAILURE;
    }

    if(mmpResult == MMP_SUCCESS) {

        mmpResult = MMP_FAILURE;
        offset = 2;
        remain_byte = (MMP_S32)jpegsize;
        while(remain_byte > 4) {
            
            memcpy(&code, p, 2); p+=2;
            memcpy(&len, p, 2); 

            len = MMP_SWAP_U16(len);

            if(code == JPEG_SOF_CODE) {
                mmpResult = MMP_SUCCESS;
                if(sof_offset) *sof_offset = offset;
                break;
            }
            else {
                p += len;
                remain_byte -= 2+len;
                offset += 2+len;
            }
        }

    }

    return mmpResult;
}



/********************************************
Exit Def
********************************************/
#define EXIF_LOG2(x)                    (log((double)(x)) / log(2.0))
#define APEX_FNUM_TO_APERTURE(x)        ((int)(EXIF_LOG2((double)(x)) * 2 + 0.5))
#define APEX_EXPOSURE_TO_SHUTTER(x)     ((x) >= 1 ?                                 \
                                        (int)(-(EXIF_LOG2((double)(x)) + 0.5)) :    \
                                        (int)(-(EXIF_LOG2((double)(x)) - 0.5)))
#define APEX_ISO_TO_FILMSENSITIVITY(x)  ((int)(EXIF_LOG2((x) / 3.125) + 0.5))

#define NUM_SIZE                    2
#define IFD_SIZE                    12
#define OFFSET_SIZE                 4

#define NUM_0TH_IFD_TIFF            10
#define NUM_0TH_IFD_EXIF            22
#define NUM_0TH_IFD_GPS             10
#define NUM_1TH_IFD_TIFF            9

/* Type */
#define EXIF_TYPE_BYTE              1
#define EXIF_TYPE_ASCII             2
#define EXIF_TYPE_SHORT             3
#define EXIF_TYPE_LONG              4
#define EXIF_TYPE_RATIONAL          5
#define EXIF_TYPE_UNDEFINED         7
#define EXIF_TYPE_SLONG             9
#define EXIF_TYPE_SRATIONAL         10

#define EXIF_FILE_SIZE              28800

/* 0th IFD TIFF Tags */
#define EXIF_TAG_IMAGE_WIDTH                    0x0100
#define EXIF_TAG_IMAGE_HEIGHT                   0x0101
#define EXIF_TAG_MAKE                           0x010f
#define EXIF_TAG_MODEL                          0x0110
#define EXIF_TAG_ORIENTATION                    0x0112
#define EXIF_TAG_SOFTWARE                       0x0131
#define EXIF_TAG_DATE_TIME                      0x0132
#define EXIF_TAG_YCBCR_POSITIONING              0x0213
#define EXIF_TAG_EXIF_IFD_POINTER               0x8769
#define EXIF_TAG_GPS_IFD_POINTER                0x8825

/* 0th IFD Exif Private Tags */
#define EXIF_TAG_EXPOSURE_TIME                  0x829A
#define EXIF_TAG_FNUMBER                        0x829D
#define EXIF_TAG_EXPOSURE_PROGRAM               0x8822
#define EXIF_TAG_ISO_SPEED_RATING               0x8827
#define EXIF_TAG_EXIF_VERSION                   0x9000
#define EXIF_TAG_DATE_TIME_ORG                  0x9003
#define EXIF_TAG_DATE_TIME_DIGITIZE             0x9004
#define EXIF_TAG_SHUTTER_SPEED                  0x9201
#define EXIF_TAG_APERTURE                       0x9202
#define EXIF_TAG_BRIGHTNESS                     0x9203
#define EXIF_TAG_EXPOSURE_BIAS                  0x9204
#define EXIF_TAG_MAX_APERTURE                   0x9205
#define EXIF_TAG_METERING_MODE                  0x9207
#define EXIF_TAG_FLASH                          0x9209
#define EXIF_TAG_FOCAL_LENGTH                   0x920A
#define EXIF_TAG_USER_COMMENT                   0x9286
#define EXIF_TAG_COLOR_SPACE                    0xA001
#define EXIF_TAG_PIXEL_X_DIMENSION              0xA002
#define EXIF_TAG_PIXEL_Y_DIMENSION              0xA003
#define EXIF_TAG_EXPOSURE_MODE                  0xA402
#define EXIF_TAG_WHITE_BALANCE                  0xA403
#define EXIF_TAG_SCENCE_CAPTURE_TYPE            0xA406

/* 0th IFD GPS Info Tags */
#define EXIF_TAG_GPS_VERSION_ID                 0x0000
#define EXIF_TAG_GPS_LATITUDE_REF               0x0001
#define EXIF_TAG_GPS_LATITUDE                   0x0002
#define EXIF_TAG_GPS_LONGITUDE_REF              0x0003
#define EXIF_TAG_GPS_LONGITUDE                  0x0004
#define EXIF_TAG_GPS_ALTITUDE_REF               0x0005
#define EXIF_TAG_GPS_ALTITUDE                   0x0006
#define EXIF_TAG_GPS_TIMESTAMP                  0x0007
#define EXIF_TAG_GPS_PROCESSING_METHOD          0x001B
#define EXIF_TAG_GPS_DATESTAMP                  0x001D

/* 1th IFD TIFF Tags */
#define EXIF_TAG_COMPRESSION_SCHEME             0x0103
#define EXIF_TAG_X_RESOLUTION                   0x011A
#define EXIF_TAG_Y_RESOLUTION                   0x011B
#define EXIF_TAG_RESOLUTION_UNIT                0x0128
#define EXIF_TAG_JPEG_INTERCHANGE_FORMAT        0x0201
#define EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LEN    0x0202

typedef enum {
    EXIF_ORIENTATION_UP     = 1,
    EXIF_ORIENTATION_90     = 6,
    EXIF_ORIENTATION_180    = 3,
    EXIF_ORIENTATION_270    = 8,
} ExifOrientationType;

typedef enum {
    EXIF_SCENE_STANDARD,
    EXIF_SCENE_LANDSCAPE,
    EXIF_SCENE_PORTRAIT,
    EXIF_SCENE_NIGHT,
} CamExifSceneCaptureType;

typedef enum {
    EXIF_METERING_UNKNOWN,
    EXIF_METERING_AVERAGE,
    EXIF_METERING_CENTER,
    EXIF_METERING_SPOT,
    EXIF_METERING_MULTISPOT,
    EXIF_METERING_PATTERN,
    EXIF_METERING_PARTIAL,
    EXIF_METERING_OTHER     = 255,
} CamExifMeteringModeType;

typedef enum {
    EXIF_EXPOSURE_AUTO,
    EXIF_EXPOSURE_MANUAL,
    EXIF_EXPOSURE_AUTO_BRACKET,
} CamExifExposureModeType;

typedef enum {
    EXIF_WB_AUTO,
    EXIF_WB_MANUAL,
} CamExifWhiteBalanceType;

/* Values */
#define EXIF_DEF_MAKER          "SAMSUNG"
#define EXIF_DEF_MODEL          "SAMSUNG"
#define EXIF_DEF_SOFTWARE       "SAMSUNG"
#define EXIF_DEF_EXIF_VERSION   "0220"
#define EXIF_DEF_USERCOMMENTS   "User comments"

#define EXIF_DEF_YCBCR_POSITIONING  1   /* centered */
#define EXIF_DEF_FNUMBER_NUM        26  /* 2.6 */
#define EXIF_DEF_FNUMBER_DEN        10
#define EXIF_DEF_EXPOSURE_PROGRAM   3   /* aperture priority */
#define EXIF_DEF_FOCAL_LEN_NUM      278 /* 2.78mm */
#define EXIF_DEF_FOCAL_LEN_DEN      100
#define EXIF_DEF_FLASH              0   /* O: off, 1: on*/
#define EXIF_DEF_COLOR_SPACE        1
#define EXIF_DEF_EXPOSURE_MODE      EXIF_EXPOSURE_AUTO
#define EXIF_DEF_APEX_DEN           10

#define EXIF_DEF_COMPRESSION        6
#define EXIF_DEF_RESOLUTION_NUM     72
#define EXIF_DEF_RESOLUTION_DEN     1
#define EXIF_DEF_RESOLUTION_UNIT    2   /* inches */

typedef struct {
    MMP_U32 num;
    MMP_U32 den;
} rational_t;

typedef struct {
    MMP_S32 num;
    MMP_S32 den;
} srational_t;

typedef struct {
    bool enableGps;
    bool enableThumb;

    unsigned char maker[32];
    unsigned char model[32];
    unsigned char software[32];
    unsigned char exif_version[4];
    unsigned char date_time[20];
    unsigned char user_comment[150];

    MMP_U32 width;
    MMP_U32 height;
    MMP_U32 widthThumb;
    MMP_U32 heightThumb;

    MMP_U16 orientation;
    MMP_U16 ycbcr_positioning;
    MMP_U16 exposure_program;
    MMP_U16 iso_speed_rating;
    MMP_U16 metering_mode;
    MMP_U16 flash;
    MMP_U16 color_space;
    MMP_U16 exposure_mode;
    MMP_U16 white_balance;
    MMP_U16 scene_capture_type;

    rational_t exposure_time;
    rational_t fnumber;
    rational_t aperture;
    rational_t max_aperture;
    rational_t focal_length;

    srational_t shutter_speed;
    srational_t brightness;
    srational_t exposure_bias;

    unsigned char gps_latitude_ref[2];
    unsigned char gps_longitude_ref[2];

    MMP_U8 gps_version_id[4];
    MMP_U8 gps_altitude_ref;

    rational_t gps_latitude[3];
    rational_t gps_longitude[3];
    rational_t gps_altitude;
    rational_t gps_timestamp[3];
    unsigned char gps_datestamp[11];
    unsigned char gps_processing_method[100];

    rational_t x_resolution;
    rational_t y_resolution;
    MMP_U16 resolution_unit;
    MMP_U16 compression_scheme;
} exif_attribute_t;

inline void writeExifIfd(unsigned char **pCur,
                             unsigned short tag,
                             unsigned short type,
                             unsigned int count,
                             MMP_U32 value)
{
    memcpy(*pCur, &tag, 2);
    *pCur += 2;
    memcpy(*pCur, &type, 2);
    *pCur += 2;
    memcpy(*pCur, &count, 4);
    *pCur += 4;
    memcpy(*pCur, &value, 4);
    *pCur += 4;
}

inline void writeExifIfd(unsigned char **pCur,
                                         unsigned short tag,
                                         unsigned short type,
                                         unsigned int count,
                                         unsigned char *pValue)
{
    char buf[4] = { 0,};

    memcpy(buf, pValue, count);
    memcpy(*pCur, &tag, 2);
    *pCur += 2;
    memcpy(*pCur, &type, 2);
    *pCur += 2;
    memcpy(*pCur, &count, 4);
    *pCur += 4;
    memcpy(*pCur, buf, 4);
    *pCur += 4;
}

inline void writeExifIfd(unsigned char **pCur,
                                         unsigned short tag,
                                         unsigned short type,
                                         unsigned int count,
                                         unsigned char *pValue,
                                         unsigned int *offset,
                                         unsigned char *start)
{
    memcpy(*pCur, &tag, 2);
    *pCur += 2;
    memcpy(*pCur, &type, 2);
    *pCur += 2;
    memcpy(*pCur, &count, 4);
    *pCur += 4;
    memcpy(*pCur, offset, 4);
    *pCur += 4;
    memcpy(start + *offset, pValue, count);
    *offset += count;
}

inline void writeExifIfd(unsigned char **pCur,
                                         unsigned short tag,
                                         unsigned short type,
                                         unsigned int count,
                                         rational_t *pValue,
                                         unsigned int *offset,
                                         unsigned char *start)
{
    memcpy(*pCur, &tag, 2);
    *pCur += 2;
    memcpy(*pCur, &type, 2);
    *pCur += 2;
    memcpy(*pCur, &count, 4);
    *pCur += 4;
    memcpy(*pCur, offset, 4);
    *pCur += 4;
    memcpy(start + *offset, pValue, 8 * count);
    *offset += 8 * count;
}

static void setExifFixedAttribute(exif_attribute_t *exifInfo)
{
    //char property[PROPERTY_VALUE_MAX];

    //2 0th IFD TIFF Tags
    //3 Maker
    //property_get("ro.product.brand", property, EXIF_DEF_MAKER);
    //strncpy((char *)exifInfo->maker, property, sizeof(exifInfo->maker) - 1);
    //exifInfo->maker[sizeof(exifInfo->maker) - 1] = '\0';
    strcpy((char*)exifInfo->maker, "MAKER: hthwang@anapass.com");

    //3 Model
    //property_get("ro.product.model", property, EXIF_DEF_MODEL);
    //strncpy((char *)exifInfo->model, property, sizeof(exifInfo->model) - 1);
    //exifInfo->model[sizeof(exifInfo->model) - 1] = '\0';
    strcpy((char*)exifInfo->model, "MODEL: hthwang@anapass.com");

    //3 Software
    //property_get("ro.build.id", property, EXIF_DEF_SOFTWARE);
    //strncpy((char *)exifInfo->software, property, sizeof(exifInfo->software) - 1);
    //exifInfo->software[sizeof(exifInfo->software) - 1] = '\0';
    strcpy((char*)exifInfo->software, "SOFTWARE: hthwang@anapass.com");

    //3 YCbCr Positioning
    exifInfo->ycbcr_positioning = EXIF_DEF_YCBCR_POSITIONING;

    //2 0th IFD Exif Private Tags
    //3 F Number
    exifInfo->fnumber.num = EXIF_DEF_FNUMBER_NUM;
    exifInfo->fnumber.den = EXIF_DEF_FNUMBER_DEN;
    //3 Exposure Program
    exifInfo->exposure_program = EXIF_DEF_EXPOSURE_PROGRAM;
    //3 Exif Version
    memcpy(exifInfo->exif_version, EXIF_DEF_EXIF_VERSION, sizeof(exifInfo->exif_version));
    //3 Aperture
    MMP_U32 av = APEX_FNUM_TO_APERTURE((double)exifInfo->fnumber.num/exifInfo->fnumber.den);
    exifInfo->aperture.num = av*EXIF_DEF_APEX_DEN;
    exifInfo->aperture.den = EXIF_DEF_APEX_DEN;
    //3 Maximum lens aperture
    exifInfo->max_aperture.num = exifInfo->aperture.num;
    exifInfo->max_aperture.den = exifInfo->aperture.den;
    //3 Lens Focal Length
#if 0
    if (m_camera_id == CAMERA_ID_BACK)
        exifInfo->focal_length.num = BACK_CAMERA_FOCAL_LENGTH;
    else
        exifInfo->focal_length.num = FRONT_CAMERA_FOCAL_LENGTH;
    exifInfo->focal_length.den = EXIF_DEF_FOCAL_LEN_DEN;
#else
    exifInfo->focal_length.num = 1;
    exifInfo->focal_length.den = 1;
#endif

    
    //3 User Comments
    strcpy((char *)exifInfo->user_comment, EXIF_DEF_USERCOMMENTS);
    //3 Color Space information
    exifInfo->color_space = EXIF_DEF_COLOR_SPACE;
    //3 Exposure Mode
    exifInfo->exposure_mode = EXIF_DEF_EXPOSURE_MODE;

    //2 0th IFD GPS Info Tags
    unsigned char gps_version[4] = { 0x02, 0x02, 0x00, 0x00 };
    memcpy(exifInfo->gps_version_id, gps_version, sizeof(gps_version));

    //2 1th IFD TIFF Tags
    exifInfo->compression_scheme = EXIF_DEF_COMPRESSION;
    exifInfo->x_resolution.num = EXIF_DEF_RESOLUTION_NUM;
    exifInfo->x_resolution.den = EXIF_DEF_RESOLUTION_DEN;
    exifInfo->y_resolution.num = EXIF_DEF_RESOLUTION_NUM;
    exifInfo->y_resolution.den = EXIF_DEF_RESOLUTION_DEN;
    exifInfo->resolution_unit = EXIF_DEF_RESOLUTION_UNIT;
}

int CMmpImageTool::Jpeg_makeExif(unsigned char *exifOut,
                          unsigned char *thumb_buf,
                          unsigned int thumb_size,
                          unsigned int *size,
                          bool useMainbufForThumb)
{
    unsigned char *pCur, *pApp1Start, *pIfdStart, *pGpsIfdPtr, *pNextIfdOffset;
    unsigned int tmp, LongerTagOffest = 0;
    pApp1Start = pCur = exifOut;

    exif_attribute_t exifInfoObj;
    exif_attribute_t *exifInfo =&exifInfoObj;

    setExifFixedAttribute(exifInfo);

    //2 Exif Identifier Code & TIFF Header
    pCur += 4;  // Skip 4 Byte for APP1 marker and length
    unsigned char ExifIdentifierCode[6] = { 0x45, 0x78, 0x69, 0x66, 0x00, 0x00 };
    memcpy(pCur, ExifIdentifierCode, 6);
    pCur += 6;

    /* Byte Order - little endian, Offset of IFD - 0x00000008.H */
    unsigned char TiffHeader[8] = { 0x49, 0x49, 0x2A, 0x00, 0x08, 0x00, 0x00, 0x00 };
    memcpy(pCur, TiffHeader, 8);
    pIfdStart = pCur;
    pCur += 8;

    //2 0th IFD TIFF Tags
    if (exifInfo->enableGps)
        tmp = NUM_0TH_IFD_TIFF;
    else
        tmp = NUM_0TH_IFD_TIFF - 1;

    memcpy(pCur, &tmp, NUM_SIZE);
    pCur += NUM_SIZE;

    LongerTagOffest += 8 + NUM_SIZE + tmp*IFD_SIZE + OFFSET_SIZE;

    writeExifIfd(&pCur, EXIF_TAG_IMAGE_WIDTH, EXIF_TYPE_LONG,
                 1, exifInfo->width);
    writeExifIfd(&pCur, EXIF_TAG_IMAGE_HEIGHT, EXIF_TYPE_LONG,
                 1, exifInfo->height);
    writeExifIfd(&pCur, EXIF_TAG_MAKE, EXIF_TYPE_ASCII,
                 strlen((char *)exifInfo->maker) + 1, exifInfo->maker, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_MODEL, EXIF_TYPE_ASCII,
                 strlen((char *)exifInfo->model) + 1, exifInfo->model, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_ORIENTATION, EXIF_TYPE_SHORT,
                 1, exifInfo->orientation);
    writeExifIfd(&pCur, EXIF_TAG_SOFTWARE, EXIF_TYPE_ASCII,
                 strlen((char *)exifInfo->software) + 1, exifInfo->software, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_DATE_TIME, EXIF_TYPE_ASCII,
                 20, exifInfo->date_time, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_YCBCR_POSITIONING, EXIF_TYPE_SHORT,
                 1, exifInfo->ycbcr_positioning);
    writeExifIfd(&pCur, EXIF_TAG_EXIF_IFD_POINTER, EXIF_TYPE_LONG,
                 1, LongerTagOffest);
    if (exifInfo->enableGps) {
        pGpsIfdPtr = pCur;
        pCur += IFD_SIZE;   // Skip a ifd size for gps IFD pointer
    }

    pNextIfdOffset = pCur;  // Skip a offset size for next IFD offset
    pCur += OFFSET_SIZE;

    //2 0th IFD Exif Private Tags
    pCur = pIfdStart + LongerTagOffest;

    tmp = NUM_0TH_IFD_EXIF;
    memcpy(pCur, &tmp , NUM_SIZE);
    pCur += NUM_SIZE;

    LongerTagOffest += NUM_SIZE + NUM_0TH_IFD_EXIF*IFD_SIZE + OFFSET_SIZE;

    writeExifIfd(&pCur, EXIF_TAG_EXPOSURE_TIME, EXIF_TYPE_RATIONAL,
                 1, &exifInfo->exposure_time, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_FNUMBER, EXIF_TYPE_RATIONAL,
                 1, &exifInfo->fnumber, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_EXPOSURE_PROGRAM, EXIF_TYPE_SHORT,
                 1, exifInfo->exposure_program);
    writeExifIfd(&pCur, EXIF_TAG_ISO_SPEED_RATING, EXIF_TYPE_SHORT,
                 1, exifInfo->iso_speed_rating);
    writeExifIfd(&pCur, EXIF_TAG_EXIF_VERSION, EXIF_TYPE_UNDEFINED,
                 4, exifInfo->exif_version);
    writeExifIfd(&pCur, EXIF_TAG_DATE_TIME_ORG, EXIF_TYPE_ASCII,
                 20, exifInfo->date_time, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_DATE_TIME_DIGITIZE, EXIF_TYPE_ASCII,
                 20, exifInfo->date_time, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_SHUTTER_SPEED, EXIF_TYPE_SRATIONAL,
                 1, (rational_t *)&exifInfo->shutter_speed, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_APERTURE, EXIF_TYPE_RATIONAL,
                 1, &exifInfo->aperture, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_BRIGHTNESS, EXIF_TYPE_SRATIONAL,
                 1, (rational_t *)&exifInfo->brightness, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_EXPOSURE_BIAS, EXIF_TYPE_SRATIONAL,
                 1, (rational_t *)&exifInfo->exposure_bias, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_MAX_APERTURE, EXIF_TYPE_RATIONAL,
                 1, &exifInfo->max_aperture, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_METERING_MODE, EXIF_TYPE_SHORT,
                 1, exifInfo->metering_mode);
    writeExifIfd(&pCur, EXIF_TAG_FLASH, EXIF_TYPE_SHORT,
                 1, exifInfo->flash);
    writeExifIfd(&pCur, EXIF_TAG_FOCAL_LENGTH, EXIF_TYPE_RATIONAL,
                 1, &exifInfo->focal_length, &LongerTagOffest, pIfdStart);
    char code[8] = { 0x00, 0x00, 0x00, 0x49, 0x49, 0x43, 0x53, 0x41 };
    int commentsLen = strlen((char *)exifInfo->user_comment) + 1;
    memmove(exifInfo->user_comment + sizeof(code), exifInfo->user_comment, commentsLen);
    memcpy(exifInfo->user_comment, code, sizeof(code));
    writeExifIfd(&pCur, EXIF_TAG_USER_COMMENT, EXIF_TYPE_UNDEFINED,
                 commentsLen + sizeof(code), exifInfo->user_comment, &LongerTagOffest, pIfdStart);
    writeExifIfd(&pCur, EXIF_TAG_COLOR_SPACE, EXIF_TYPE_SHORT,
                 1, exifInfo->color_space);
    writeExifIfd(&pCur, EXIF_TAG_PIXEL_X_DIMENSION, EXIF_TYPE_LONG,
                 1, exifInfo->width);
    writeExifIfd(&pCur, EXIF_TAG_PIXEL_Y_DIMENSION, EXIF_TYPE_LONG,
                 1, exifInfo->height);
    writeExifIfd(&pCur, EXIF_TAG_EXPOSURE_MODE, EXIF_TYPE_LONG,
                 1, exifInfo->exposure_mode);
    writeExifIfd(&pCur, EXIF_TAG_WHITE_BALANCE, EXIF_TYPE_LONG,
                 1, exifInfo->white_balance);
    writeExifIfd(&pCur, EXIF_TAG_SCENCE_CAPTURE_TYPE, EXIF_TYPE_LONG,
                 1, exifInfo->scene_capture_type);
    tmp = 0;
    memcpy(pCur, &tmp, OFFSET_SIZE); // next IFD offset
    pCur += OFFSET_SIZE;

    //2 0th IFD GPS Info Tags
    if (exifInfo->enableGps) {
        writeExifIfd(&pGpsIfdPtr, EXIF_TAG_GPS_IFD_POINTER, EXIF_TYPE_LONG,
                     1, LongerTagOffest); // GPS IFD pointer skipped on 0th IFD

        pCur = pIfdStart + LongerTagOffest;

        if (exifInfo->gps_processing_method[0] == 0) {
            // don't create GPS_PROCESSING_METHOD tag if there isn't any
            tmp = NUM_0TH_IFD_GPS - 1;
        } else {
            tmp = NUM_0TH_IFD_GPS;
        }
        memcpy(pCur, &tmp, NUM_SIZE);
        pCur += NUM_SIZE;

        LongerTagOffest += NUM_SIZE + tmp*IFD_SIZE + OFFSET_SIZE;

        writeExifIfd(&pCur, EXIF_TAG_GPS_VERSION_ID, EXIF_TYPE_BYTE,
                     4, exifInfo->gps_version_id);
        writeExifIfd(&pCur, EXIF_TAG_GPS_LATITUDE_REF, EXIF_TYPE_ASCII,
                     2, exifInfo->gps_latitude_ref);
        writeExifIfd(&pCur, EXIF_TAG_GPS_LATITUDE, EXIF_TYPE_RATIONAL,
                     3, exifInfo->gps_latitude, &LongerTagOffest, pIfdStart);
        writeExifIfd(&pCur, EXIF_TAG_GPS_LONGITUDE_REF, EXIF_TYPE_ASCII,
                     2, exifInfo->gps_longitude_ref);
        writeExifIfd(&pCur, EXIF_TAG_GPS_LONGITUDE, EXIF_TYPE_RATIONAL,
                     3, exifInfo->gps_longitude, &LongerTagOffest, pIfdStart);
        writeExifIfd(&pCur, EXIF_TAG_GPS_ALTITUDE_REF, EXIF_TYPE_BYTE,
                     1, exifInfo->gps_altitude_ref);
        writeExifIfd(&pCur, EXIF_TAG_GPS_ALTITUDE, EXIF_TYPE_RATIONAL,
                     1, &exifInfo->gps_altitude, &LongerTagOffest, pIfdStart);
        writeExifIfd(&pCur, EXIF_TAG_GPS_TIMESTAMP, EXIF_TYPE_RATIONAL,
                     3, exifInfo->gps_timestamp, &LongerTagOffest, pIfdStart);
        tmp = strlen((char*)exifInfo->gps_processing_method);
        if (tmp > 0) {
            if (tmp > 100) {
                tmp = 100;
            }
            static const char ExifAsciiPrefix[] = { 0x41, 0x53, 0x43, 0x49, 0x49, 0x0, 0x0, 0x0 };
            unsigned char tmp_buf[100+sizeof(ExifAsciiPrefix)];
            memcpy(tmp_buf, ExifAsciiPrefix, sizeof(ExifAsciiPrefix));
            memcpy(&tmp_buf[sizeof(ExifAsciiPrefix)], exifInfo->gps_processing_method, tmp);
            writeExifIfd(&pCur, EXIF_TAG_GPS_PROCESSING_METHOD, EXIF_TYPE_UNDEFINED,
                         tmp+sizeof(ExifAsciiPrefix), tmp_buf, &LongerTagOffest, pIfdStart);
        }
        writeExifIfd(&pCur, EXIF_TAG_GPS_DATESTAMP, EXIF_TYPE_ASCII,
                     11, exifInfo->gps_datestamp, &LongerTagOffest, pIfdStart);
        tmp = 0;
        memcpy(pCur, &tmp, OFFSET_SIZE); // next IFD offset
        pCur += OFFSET_SIZE;
    }

    //2 1th IFD TIFF Tags

    unsigned char *thumbBuf = thumb_buf;
    unsigned int thumbSize = thumb_size;

    if (exifInfo->enableThumb && (thumbBuf != NULL) && (thumbSize > 0)) {
        tmp = LongerTagOffest;
        memcpy(pNextIfdOffset, &tmp, OFFSET_SIZE);  // NEXT IFD offset skipped on 0th IFD

        pCur = pIfdStart + LongerTagOffest;

        tmp = NUM_1TH_IFD_TIFF;
        memcpy(pCur, &tmp, NUM_SIZE);
        pCur += NUM_SIZE;

        LongerTagOffest += NUM_SIZE + NUM_1TH_IFD_TIFF*IFD_SIZE + OFFSET_SIZE;

        writeExifIfd(&pCur, EXIF_TAG_IMAGE_WIDTH, EXIF_TYPE_LONG,
                     1, exifInfo->widthThumb);
        writeExifIfd(&pCur, EXIF_TAG_IMAGE_HEIGHT, EXIF_TYPE_LONG,
                     1, exifInfo->heightThumb);
        writeExifIfd(&pCur, EXIF_TAG_COMPRESSION_SCHEME, EXIF_TYPE_SHORT,
                     1, exifInfo->compression_scheme);
        writeExifIfd(&pCur, EXIF_TAG_ORIENTATION, EXIF_TYPE_SHORT,
                     1, exifInfo->orientation);
        writeExifIfd(&pCur, EXIF_TAG_X_RESOLUTION, EXIF_TYPE_RATIONAL,
                     1, &exifInfo->x_resolution, &LongerTagOffest, pIfdStart);
        writeExifIfd(&pCur, EXIF_TAG_Y_RESOLUTION, EXIF_TYPE_RATIONAL,
                     1, &exifInfo->y_resolution, &LongerTagOffest, pIfdStart);
        writeExifIfd(&pCur, EXIF_TAG_RESOLUTION_UNIT, EXIF_TYPE_SHORT,
                     1, exifInfo->resolution_unit);
        writeExifIfd(&pCur, EXIF_TAG_JPEG_INTERCHANGE_FORMAT, EXIF_TYPE_LONG,
                     1, LongerTagOffest);
        writeExifIfd(&pCur, EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LEN, EXIF_TYPE_LONG,
                     1, thumbSize);

        tmp = 0;
        memcpy(pCur, &tmp, OFFSET_SIZE); // next IFD offset
        pCur += OFFSET_SIZE;

        memcpy(pIfdStart + LongerTagOffest, thumbBuf, thumbSize);
        LongerTagOffest += thumbSize;
    } else {
        tmp = 0;
        memcpy(pNextIfdOffset, &tmp, OFFSET_SIZE);  // NEXT IFD offset skipped on 0th IFD
    }

    unsigned char App1Marker[2] = { 0xff, 0xe1 };
    memcpy(pApp1Start, App1Marker, 2);
    pApp1Start += 2;

    *size = 10 + LongerTagOffest;
    tmp = *size - 2;    // APP1 Maker isn't counted
    unsigned char size_mm[2] = { (unsigned char)((tmp >> 8) & 0xFF), (unsigned char)(tmp & 0xFF) };
    memcpy(pApp1Start, size_mm, 2);

    //ALOGD("makeExif X");

    return 0;
}


extern "C" {
#include "jpeglib.h"
}
#include <setjmp.h>
struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */
  jmp_buf setjmp_buffer;	/* for return to caller */
};
typedef struct my_error_mgr * my_error_ptr;
METHODDEF(void) my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

#if (MMP_OS == MMP_OS_WIN32)
#define JPEG_DEC_TMP_FILE_NAME "d:\\work\\tmpdec.jpg"
#else
#define JPEG_DEC_TMP_FILE_NAME "/tmp/tmpdec.jpg"
#endif

MMP_RESULT CMmpImageTool::Jpeg_Decode_libjpeg_BGR(MMP_U8* jpegdata, MMP_S32 jpegsize, 
                                          MMP_S32 *pic_width, MMP_S32* pic_height, enum MMP_FOURCC *fourcc,  MMP_U8* p_decoded_data) {
    MMP_RESULT mmpResult;
    FILE* fp, *infile;
    
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;
    JSAMPARRAY buffer;		/* Output row buffer */
    MMP_S32 row_stride;
    MMP_S32 out_stride;
    MMP_U8* p_image;
    
    mmpResult = MMP_SUCCESS;
    fp =NULL, infile = NULL;
    
    /* write tmp jpg file */
    if(mmpResult == MMP_SUCCESS) {
        fp = fopen(JPEG_DEC_TMP_FILE_NAME, "wb");
        if(fp==NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {
            fwrite(jpegdata, 1, jpegsize, fp);
            fclose(fp);
        }
    }

    /* open tmp jpg file */
    if(mmpResult == MMP_SUCCESS) {
        infile = fopen(JPEG_DEC_TMP_FILE_NAME, "rb");
        if(infile == NULL) {
            mmpResult = MMP_FAILURE;
        }
    }

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    if (setjmp(jerr.setjmp_buffer)) {
        jpeg_destroy_decompress(&cinfo);
        return mmpResult;
    }
  
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    (void) jpeg_read_header(&cinfo, TRUE);
    (void) jpeg_start_decompress(&cinfo);

    row_stride = cinfo.output_width * cinfo.output_components;
    out_stride = MMP_BYTE_ALIGN(row_stride, 4);
    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
    p_image = p_decoded_data;
    while (cinfo.output_scanline < cinfo.output_height) {
        (void) jpeg_read_scanlines(&cinfo, buffer, 1);
        memcpy(p_image, (void*)(*((unsigned int*)buffer)), row_stride);
        p_image += out_stride;
    }
    (void) jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);


    if(infile != NULL) {
        fclose(infile);
    }

    if(pic_width!=NULL) *pic_width = cinfo.output_width;
    if(pic_height!=NULL) *pic_height = cinfo.output_height;
    if(fourcc!=NULL) {
        if(cinfo.output_components == 3) *fourcc = MMP_FOURCC_IMAGE_BGR888;
        else if(cinfo.output_components == 4) *fourcc = MMP_FOURCC_IMAGE_ABGR8888; 
        else if(cinfo.output_components == 1) *fourcc = MMP_FOURCC_IMAGE_GREY; 
        else *fourcc = MMP_FOURCC_VIDEO_UNKNOWN;
    }

    return mmpResult;
}

MMP_RESULT CMmpImageTool::Jpeg_Encode_libjpeg_YUV444_P1(MMP_U8* p_src_data, 
                                              MMP_S32 pic_width, MMP_S32 pic_height, MMP_S32 src_stride,  MMP_S32 quality,
                                              MMP_U8* jpegdata, MMP_S32 jpegdata_max_size, MMP_S32 *jpegsize) {
    

#if (MMP_OS == MMP_OS_WIN32)
#define JPEG_ENC_TMP_FILE_NAME "d:\\work\\tmpenc.jpg"
#else
#define JPEG_ENC_TMP_FILE_NAME "/tmp/tmpenc.jpg"
#endif

    MMP_RESULT mmpResult = MMP_SUCCESS;
    FILE* outfile = NULL, *fp = NULL;
    MMP_S32 filesz;
    
    if(mmpResult == MMP_SUCCESS) {
        outfile = fopen(JPEG_ENC_TMP_FILE_NAME, "wb");
        if(outfile == NULL) {
            mmpResult = MMP_FAILURE;
        }
    }
    
    if(mmpResult == MMP_SUCCESS) {
        
        struct jpeg_compress_struct cinfo;
        struct jpeg_error_mgr jerr;
        JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
        
        
        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_compress(&cinfo);
        jpeg_stdio_dest(&cinfo, outfile);

        cinfo.image_width = pic_width; 	/* image width and height, in pixels */
        cinfo.image_height = pic_height;
        cinfo.input_components = 3;		/* # of color components per pixel */
        cinfo.in_color_space = JCS_YCbCr; 	/* colorspace of input image */
      
        jpeg_set_defaults(&cinfo);

        if (quality < 0) quality = 0;
        else if(quality > 100) quality = 100;
        jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
        
        jpeg_start_compress(&cinfo, TRUE);

        while (cinfo.next_scanline < cinfo.image_height) {
            /* jpeg_write_scanlines expects an array of pointers to scanlines.
             * Here the array is only one element long, but you could pass
             * more than one scanline at a time if that's more convenient.
             */
            row_pointer[0] = &p_src_data[cinfo.next_scanline * src_stride];
            (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
        }

        jpeg_finish_compress(&cinfo);
        jpeg_destroy_compress(&cinfo);

        fclose(outfile);
        outfile = NULL;
        
        fp = fopen(JPEG_ENC_TMP_FILE_NAME, "rb");
        if(fp == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {
            fseek(fp, 0, SEEK_END);
            filesz = (MMP_S32)ftell(fp);
            fseek(fp, 0, SEEK_SET);
            if(filesz > jpegdata_max_size) {
                mmpResult = MMP_FAILURE;
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpImageTool::%s] FAIL: jpegbuff is too samll"), MMP_CLASS_FUNC));
            }
            else {
                fread(jpegdata, 1, filesz, fp);
                if(jpegsize!=NULL) *jpegsize = filesz;
            }
            fclose(fp);
        }
    }

    if(outfile != NULL) {
        fclose(outfile);
    }

    return mmpResult;
}

MMP_RESULT CMmpImageTool::Jpeg_Encode_libjpeg_GREY(MMP_U8* p_src_data, 
                                              MMP_S32 pic_width, MMP_S32 pic_height, MMP_S32 src_stride,  MMP_S32 quality,
                                              MMP_U8* jpegdata, MMP_S32 jpegdata_max_size, MMP_S32 *jpegsize) {
    

#if (MMP_OS == MMP_OS_WIN32)
#define JPEG_ENC_TMP_FILE_NAME "d:\\work\\tmpenc.jpg"
#else
#define JPEG_ENC_TMP_FILE_NAME "/tmp/tmpenc.jpg"
#endif

    MMP_RESULT mmpResult = MMP_SUCCESS;
    FILE* outfile = NULL, *fp = NULL;
    MMP_S32 filesz;
    
    if(mmpResult == MMP_SUCCESS) {
        outfile = fopen(JPEG_ENC_TMP_FILE_NAME, "wb");
        if(outfile == NULL) {
            mmpResult = MMP_FAILURE;
        }
    }
    
    if(mmpResult == MMP_SUCCESS) {
        
        struct jpeg_compress_struct cinfo;
        struct jpeg_error_mgr jerr;
        JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
        
        
        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_compress(&cinfo);
        jpeg_stdio_dest(&cinfo, outfile);

        cinfo.image_width = pic_width; 	/* image width and height, in pixels */
        cinfo.image_height = pic_height;
        cinfo.input_components = 1;		/* # of color components per pixel */
        cinfo.in_color_space = JCS_GRAYSCALE; 	/* colorspace of input image */
      
        jpeg_set_defaults(&cinfo);

        if (quality < 0) quality = 0;
        else if(quality > 100) quality = 100;
        jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
        
        jpeg_start_compress(&cinfo, TRUE);

        while (cinfo.next_scanline < cinfo.image_height) {
            /* jpeg_write_scanlines expects an array of pointers to scanlines.
             * Here the array is only one element long, but you could pass
             * more than one scanline at a time if that's more convenient.
             */
            row_pointer[0] = &p_src_data[cinfo.next_scanline * src_stride];
            (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
        }

        jpeg_finish_compress(&cinfo);
        jpeg_destroy_compress(&cinfo);

        fclose(outfile);
        outfile = NULL;
        
        fp = fopen(JPEG_ENC_TMP_FILE_NAME, "rb");
        if(fp == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {
            fseek(fp, 0, SEEK_END);
            filesz = (MMP_S32)ftell(fp);
            fseek(fp, 0, SEEK_SET);
            if(filesz > jpegdata_max_size) {
                mmpResult = MMP_FAILURE;
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpImageTool::%s] FAIL: jpegbuff is too samll"), MMP_CLASS_FUNC));
            }
            else {
                fread(jpegdata, 1, filesz, fp);
                if(jpegsize!=NULL) *jpegsize = filesz;
            }
            fclose(fp);
        }
    }

    if(outfile != NULL) {
        fclose(outfile);
    }

    return mmpResult;
}

/********************************************************** 
        Tool of BMP
***********************************************************/

MMP_CHAR* CMmpImageTool::Bmp_GetName(enum MMP_FOURCC fourcc, MMP_CHAR* buf) {
    
    
    switch(fourcc) {
        case MMP_FOURCC_IMAGE_RGB888  :  strcpy(buf, "RGB888");   break;
	    case MMP_FOURCC_IMAGE_BGR888  :  strcpy(buf, "BGR888");   break;
	    //case MMP_FOURCC_IMAGE_RGB565  :  strcpy(buf, "RGB565");   break;
	    //case MMP_FOURCC_IMAGE_BGR565  :  strcpy(buf, "BGR565");   break;
	    //case MMP_FOURCC_IMAGE_ARGB1555:  strcpy(buf, "ARGB1555");   break;
	    //case MMP_FOURCC_IMAGE_BGRA1555:  strcpy(buf, "BGRA1555");   break;
	    //case MMP_FOURCC_IMAGE_RGBA1555:  strcpy(buf, "RGBA1555");   break;
	    //case MMP_FOURCC_IMAGE_ABGR1555:  strcpy(buf, "ABGR1555");   break;
	    //case MMP_FOURCC_IMAGE_ARGB8888:  strcpy(buf, "ARGB8888");   break; 
	    //case MMP_FOURCC_IMAGE_BGRA8888:  strcpy(buf, "BGRA8888");   break; 
	    case MMP_FOURCC_IMAGE_ARGB8888:  strcpy(buf, "RGBA8888");   break; 
	    //case MMP_FOURCC_IMAGE_ABGR8888:  strcpy(buf, "ABGR8888");   break; 

        default:  strcpy(buf, "---");   break;

    }

    return buf;
}

MMP_RESULT CMmpImageTool::Bmp_SaveFile(MMP_CHAR* bmp_filename, MMP_S32 pic_width, MMP_S32 pic_height, MMP_U8* p_image, enum MMP_FOURCC fourcc) {
    
    MMP_S32 rgb_stride;
    MMP_RESULT mmpResult = MMP_SUCCESS;

    /* check stride */
    switch(fourcc) {
    
        case MMP_FOURCC_IMAGE_BGR888:
        case MMP_FOURCC_IMAGE_RGB888: 
              rgb_stride = MMP_BYTE_ALIGN(pic_width*3,4); 
              break;
        case MMP_FOURCC_IMAGE_ARGB8888: 
        case MMP_FOURCC_IMAGE_ABGR8888: 
              rgb_stride = pic_width*4; 
              break;
        default:
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("CMmpImageTool::%s] FAIL: not support format (%c%c%c%c) "), MMP_CLASS_FUNC, 
                                                     MMPGETFOURCCARG(fourcc)
                                                     ));
    }


    if(mmpResult == MMP_SUCCESS) {
        mmpResult = CMmpImageTool::Bmp_SaveFile(bmp_filename, pic_width, pic_height, rgb_stride, p_image, fourcc);
    }

    return mmpResult;
}

MMP_RESULT CMmpImageTool::Bmp_SaveFile(MMP_CHAR* bmp_filename, MMP_S32 pic_width, MMP_S32 pic_height, MMP_S32 rgb_real_stride, MMP_U8* p_image, enum MMP_FOURCC fourcc) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    FILE* fp = NULL;
    MMPBITMAPFILEHEADER BmpFileHeader;
    MMPBITMAPINFOHEADER BmpInfoHeader;
    MMP_S32 wrsz, normal_stride;
    
    /* file open */
    if(mmpResult == MMP_SUCCESS) {
        fp = fopen(bmp_filename, "wb");
        if(fp == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpImageTool::%s] FAIL: open file(%s)"), MMP_CLASS_FUNC, bmp_filename ));
        }
    }

    /* check stride */
    switch(fourcc) {
        case MMP_FOURCC_IMAGE_BGR888:
        case MMP_FOURCC_IMAGE_RGB888: 
              normal_stride = MMP_BYTE_ALIGN(pic_width*3,4); 
              break;
        case MMP_FOURCC_IMAGE_ARGB8888: 
        case MMP_FOURCC_IMAGE_ABGR8888: 
              normal_stride = pic_width*4; 
              break;
        default:
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("CMmpImageTool::%s] FAIL: not support format (%c%c%c%c) "), MMP_CLASS_FUNC, 
                                                     MMPGETFOURCCARG(fourcc)
                                                     ));
    }

    
    /* write file header */
    if(mmpResult == MMP_SUCCESS) {
        BmpFileHeader.bfType=BMP_HEADER_MARKER;
        BmpFileHeader.bfSize=sizeof(MMPBITMAPFILEHEADER)+sizeof(MMPBITMAPINFOHEADER)+ normal_stride*pic_height;
        BmpFileHeader.bfOffBits=sizeof(MMPBITMAPFILEHEADER)+sizeof(MMPBITMAPINFOHEADER);
        wrsz = fwrite(&BmpFileHeader, 1, sizeof(MMPBITMAPFILEHEADER), fp);
        if(wrsz != sizeof(MMPBITMAPFILEHEADER)) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpImageTool::%s] FAIL: write bmp file header"), MMP_CLASS_FUNC ));
        }
    }

    /* write infoheader */
    if(mmpResult == MMP_SUCCESS) {
        memset(&BmpInfoHeader, 0, sizeof(BmpInfoHeader) );
        BmpInfoHeader.biSize = sizeof(BmpInfoHeader);
        BmpInfoHeader.biWidth = pic_width;
        BmpInfoHeader.biHeight = -pic_height; /* Default : Top=>Bottom */
        
        switch(fourcc) {
            case MMP_FOURCC_IMAGE_BGR888:
            case MMP_FOURCC_IMAGE_RGB888:
                BmpInfoHeader.biPlanes = 1;
                BmpInfoHeader.biBitCount = 24;
                BmpInfoHeader.biCompression = MMP_BI_RGB;
                break;

            case MMP_FOURCC_IMAGE_ABGR8888:
            case MMP_FOURCC_IMAGE_ARGB8888:
                BmpInfoHeader.biPlanes = 1;
                BmpInfoHeader.biBitCount = 32;
                BmpInfoHeader.biCompression = MMP_BI_RGB;
                break;
        }

        BmpInfoHeader.biSizeImage = normal_stride*pic_height;
        wrsz = fwrite(&BmpInfoHeader, 1, sizeof(MMPBITMAPINFOHEADER), fp);
        if(wrsz != sizeof(MMPBITMAPINFOHEADER)) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpImageTool::%s] FAIL: write bmp info header"), MMP_CLASS_FUNC ));
        }
    }
    
    /* write image data */
    if(mmpResult == MMP_SUCCESS) {

        switch(fourcc) {

            case MMP_FOURCC_IMAGE_BGR888: /* Bitmap File B/G/R.. B/G/R..  Ref.C:\MediaSample\Bmp\BLUE_RGB888.bmp.bin  RED_RGB888.bmp.bin */
                {
                    MMP_U8 r, g, b;
                    MMP_S32 w, h, ip;
                    MMP_U8* line_dest = (MMP_U8*)MMP_MALLOC(normal_stride), *line_dest1;
                    MMP_U8* line_src = p_image, *line_src1;
                    if(line_dest != NULL) {
                        for(h = 0; h < pic_height; h++) {
                            line_dest1 = line_dest;
                            line_src1 = line_src;
                            for(ip = 0, w = 0; w < pic_width; w++) {
                                r= *line_src1; line_src1++;
                                g = *line_src1; line_src1++;
                                b = *line_src1; line_src1++;

                                /* set B/G/R order in case of Bitmap File */
                                *line_dest1 = b; line_dest1++;
                                *line_dest1 = g; line_dest1++;
                                *line_dest1 = r; line_dest1++;
                            }

                            line_src += rgb_real_stride;
                            fwrite(line_dest, 1, normal_stride, fp);
                        }
                    
                        MMP_FREE(line_dest);
                    }
                    else {
                        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpImageTool::%s] FAIL: alloc BGR888   normal_stride=%d rgb_real_stride=%d"), MMP_CLASS_FUNC, normal_stride, rgb_real_stride ));
                    }
                }
                break;

           case MMP_FOURCC_IMAGE_ARGB8888:
                {
                    MMP_U8 r, g, b;
                    MMP_S32 w, h, ip;
                    MMP_U8* line_dest = (MMP_U8*)MMP_MALLOC(normal_stride), *line_dest1;
                    MMP_U8* line_src = p_image, *line_src1;

                    if(line_dest != NULL) {
                        for(h = 0; h < pic_height; h++) {
                            line_dest1 = line_dest;
                            line_src1 = line_src;
                            for(ip = 0, w = 0; w < pic_width; w++) {
                                b= *line_src1; line_src1++;
                                g = *line_src1; line_src1++;
                                r = *line_src1; line_src1++;
                                line_src1++;

                                /* set B/G/R order in case of Bitmap File */
                                *line_dest1 = b; line_dest1++;
                                *line_dest1 = g; line_dest1++;
                                *line_dest1 = r; line_dest1++;
                                line_dest1++;
                            }

                            line_src += rgb_real_stride;
                            fwrite(line_dest, 1, normal_stride, fp);
                        }
                    
                        MMP_FREE(line_dest);
                    }
                    else {
                        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpImageTool::%s] FAIL: alloc ARGB8888   normal_stride=%d rgb_real_stride=%d"), MMP_CLASS_FUNC, normal_stride, rgb_real_stride ));
                    }
                }
                break;
 
            case MMP_FOURCC_IMAGE_ABGR8888:
                {
                    MMP_U8 r, g, b;
                    MMP_S32 w, h, ip;
                    MMP_U8* line_dest = (MMP_U8*)MMP_MALLOC(normal_stride), *line_dest1;
                    MMP_U8* line_src = p_image, *line_src1;
                    if(line_dest != NULL) {
                        for(h = 0; h < pic_height; h++) {
                            line_dest1 = line_dest;
                            line_src1 = line_src;
                            for(ip = 0, w = 0; w < pic_width; w++) {
                                r= *line_src1; line_src1++;
                                g = *line_src1; line_src1++;
                                b = *line_src1; line_src1++;
                                line_src1++;

                                /* set B/G/R order in case of Bitmap File */
                                *line_dest1 = b; line_dest1++;
                                *line_dest1 = g; line_dest1++;
                                *line_dest1 = r; line_dest1++;
                                line_dest1++;
                            }

                            line_src += rgb_real_stride;
                            fwrite(line_dest, 1, normal_stride, fp);
                        }
                    
                        MMP_FREE(line_dest);
                    }
                    else {
                        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpImageTool::%s] FAIL: alloc ABGR8888   normal_stride=%d rgb_real_stride=%d"), MMP_CLASS_FUNC, normal_stride, rgb_real_stride ));
                    }

                }
                break;
 
            default:
                wrsz = fwrite(p_image, 1, BmpInfoHeader.biSizeImage, fp);
                if(wrsz != (MMP_S32)BmpInfoHeader.biSizeImage) {
                    mmpResult = MMP_FAILURE;
                    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpImageTool::%s] FAIL: write image data (wrtten_sz = %d / %d"), MMP_CLASS_FUNC, wrsz,  BmpInfoHeader.biSizeImage));
                }
        }
        
    }

    if(fp != NULL) {
       fclose(fp);
    }

   return mmpResult;
}

MMP_RESULT CMmpImageTool::Bmp_LoadInfo(MMP_CHAR* bmp_filename, MMP_S32* pic_width, MMP_S32* pic_height, enum MMP_FOURCC* fourcc, MMP_S32* stride) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    FILE* fp = NULL;
    MMPBITMAPFILEHEADER BmpFileHeader;
    MMPBITMAPINFOHEADER BmpInfoHeader;
    MMP_S32 rdsz;

    if(pic_width!=NULL) *pic_width = 0;
    if(pic_height!=NULL) *pic_height = 0;
    if(stride!=NULL) *stride = 0;
    if(fourcc!=NULL) *fourcc = MMP_FOURCC_VIDEO_UNKNOWN;
    
    /* file open */
    if(mmpResult == MMP_SUCCESS) {
        fp = fopen(bmp_filename, "rb");
        if(fp == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpImageTool::%s] FAIL: open file(%s)"), MMP_CLASS_FUNC, bmp_filename ));
        }
    }

    /* read file header */
    if(mmpResult == MMP_SUCCESS) {
        rdsz = fread(&BmpFileHeader, 1, sizeof(MMPBITMAPFILEHEADER), fp);
        if(rdsz != sizeof(MMPBITMAPFILEHEADER)) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpImageTool::%s] FAIL: read bmp file header"), MMP_CLASS_FUNC ));
        }
    }

    /* read info header */
    if(mmpResult == MMP_SUCCESS) {
        rdsz = fread(&BmpInfoHeader, 1, sizeof(MMPBITMAPINFOHEADER), fp);
        if(rdsz != sizeof(MMPBITMAPINFOHEADER)) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpImageTool::%s] FAIL: read bmp info header"), MMP_CLASS_FUNC ));
        }
    }

    /* check info */
    if(mmpResult == MMP_SUCCESS) {
    
        if(BmpFileHeader.bfType != BMP_HEADER_MARKER) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpImageTool::%s] FAIL: Bmp Marker Error"), MMP_CLASS_FUNC ));
        }
        else {
            if(pic_width!=NULL) *pic_width = BmpInfoHeader.biWidth;
            if(pic_height!=NULL) {
                if(BmpInfoHeader.biHeight < 0)
                    *pic_height = BmpInfoHeader.biHeight * (-1);
                else 
                    *pic_height = BmpInfoHeader.biHeight;
            }
            if(fourcc!=NULL) {
                if(BmpInfoHeader.biBitCount == 24) {
                    *fourcc = MMP_FOURCC_IMAGE_RGB888;
                    if(stride!=NULL) *stride = MMP_BYTE_ALIGN(BmpInfoHeader.biWidth*3, 4);
                }
                else if(BmpInfoHeader.biBitCount == 32) {
                    *fourcc = MMP_FOURCC_IMAGE_ARGB8888;
                    if(stride!=NULL) *stride = MMP_BYTE_ALIGN(BmpInfoHeader.biWidth*4, 4);
                }
                else {
                    mmpResult = MMP_FAILURE;
                    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpImageTool::%s] FAIL: Not Support Bitmap Bit Format"), MMP_CLASS_FUNC ));
                }
            }
        }
    }

    if(mmpResult != MMP_SUCCESS) {
        if(pic_width!=NULL) *pic_width = 0;
        if(pic_height!=NULL) *pic_height = 0;
        if(stride!=NULL) *stride = 0;
        if(fourcc!=NULL) *fourcc = MMP_FOURCC_VIDEO_UNKNOWN;
    }

    if(fp != NULL) {
        fclose(fp);
    }
    
    return mmpResult;
}
    
MMP_RESULT CMmpImageTool::Bmp_LoadFile(MMP_CHAR* bmp_filename, MMP_U8* p_image, MMP_S32 image_max_size) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    FILE* fp = NULL;
    MMPBITMAPFILEHEADER BmpFileHeader;
    MMPBITMAPINFOHEADER BmpInfoHeader;
    MMP_S32 rdsz, stride;

    MMP_S32 pic_width, pic_height;
    enum MMP_FOURCC fourcc;

   
    /* file open */
    if(mmpResult == MMP_SUCCESS) {
        fp = fopen(bmp_filename, "rb");
        if(fp == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpImageTool::%s] FAIL: open file(%s)"), MMP_CLASS_FUNC, bmp_filename ));
        }
    }

    /* read file header */
    if(mmpResult == MMP_SUCCESS) {
        rdsz = fread(&BmpFileHeader, 1, sizeof(MMPBITMAPFILEHEADER), fp);
        if(rdsz != sizeof(MMPBITMAPFILEHEADER)) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpImageTool::%s] FAIL: read bmp file header"), MMP_CLASS_FUNC ));
        }
    }

    /* read info header */
    if(mmpResult == MMP_SUCCESS) {
        rdsz = fread(&BmpInfoHeader, 1, sizeof(MMPBITMAPINFOHEADER), fp);
        if(rdsz != sizeof(MMPBITMAPINFOHEADER)) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpImageTool::%s] FAIL: read bmp info header"), MMP_CLASS_FUNC ));
        }
    }

    /* check info */
    if(mmpResult == MMP_SUCCESS) {
    
        if(BmpFileHeader.bfType != BMP_HEADER_MARKER) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpImageTool::%s] FAIL: Bmp Marker Error"), MMP_CLASS_FUNC ));
        }
        else {

            pic_width = BmpInfoHeader.biWidth;
            if(BmpInfoHeader.biHeight < 0)
                pic_height = BmpInfoHeader.biHeight * (-1);
            else 
                pic_height = BmpInfoHeader.biHeight;
                    
            if(BmpInfoHeader.biBitCount == 24) {
                fourcc = MMP_FOURCC_IMAGE_RGB888;
                stride = MMP_BYTE_ALIGN(pic_width*3, 4);

            }
            else if(BmpInfoHeader.biBitCount == 32) {
                fourcc = MMP_FOURCC_IMAGE_ARGB8888;
                stride = pic_width*4;
            }
            else {
                stride = 0;
                mmpResult = MMP_FAILURE;
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpImageTool::%s] FAIL: Not Support Bitmap Bit Format"), MMP_CLASS_FUNC ));
            }

            if(image_max_size < stride*pic_height) {
                mmpResult = MMP_FAILURE;
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpImageTool::%s] FAIL: image buffer is too smaall (bufsz=%d imagesz=%d (%dx%d bpp%d)"), MMP_CLASS_FUNC,
                          image_max_size,  stride*pic_height, pic_width, pic_height, BmpInfoHeader.biBitCount
                    ));
            }
        }
    }


    /* Load Data */
    if(mmpResult == MMP_SUCCESS) {
        
        MMP_S32 h;
        MMP_U8 *src_line, *dst_line;
        MMP_U8 *src_data, *dst_data;

        dst_data = p_image;
        src_data = (MMP_U8*)MMP_MALLOC(stride*pic_height);
        if(src_data == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpImageTool::%s] FAIL: malloc src_data"), MMP_CLASS_FUNC ));
        }
        else {

            rdsz = fread(src_data, 1, stride*pic_height, fp);
            if(rdsz != (stride*pic_height) ) {
                mmpResult = MMP_FAILURE;
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpImageTool::%s] FAIL: read data"), MMP_CLASS_FUNC ));
            }
            else {

                if(BmpInfoHeader.biHeight > 0)  { src_line = src_data + (pic_height-1)*stride; }
                else   {  src_line = src_data; }
                
                dst_line = dst_data;

                for(h = 0; h < pic_height; h++) {
                    memcpy(dst_line, src_line, stride);

                    if(BmpInfoHeader.biHeight > 0) { src_line -= stride; }
                    else { src_line += stride; } 
                    dst_line += stride;
                }

            }

            MMP_FREE(src_data);
        }
    }

    if(fp != NULL) {
        fclose(fp);
    }
    
    
    return mmpResult;
}

/********************************************************** 
        Tool of YUV File
***********************************************************/
    
MMP_RESULT CMmpImageTool::YUV_LoadFile_YUV420P3(MMP_CHAR* yuv_filename, MMP_S32 pic_width, MMP_S32 pic_height, 
                                                MMP_U8* p_data_y, MMP_U8* p_data_u, MMP_U8* p_data_v, 
                                                MMP_S32 y_stride, MMP_S32 u_stride, MMP_S32 v_stride) {
    
    FILE* fp = NULL;
    MMP_RESULT mmpResult = MMP_SUCCESS;

    MMP_S32 h;
    MMP_S32 src_height;
    MMP_S32 src_stride, dst_stride;

    MMP_U8* dst_line;
    MMP_S32 rdsz;
    
    /* File Open */
    fp = fopen(yuv_filename, "rb");
    if(fp == NULL) {
        mmpResult = MMP_FAILURE;
    }
    
    /* Load Y */
    if(mmpResult == MMP_SUCCESS) {
    
        src_stride = pic_width;
        src_height = pic_height;
        dst_stride = y_stride;
        dst_line = p_data_y;
        for(h = 0; h < src_height; h++) {
            
            rdsz = fread(dst_line, 1, src_stride, fp);
            if(rdsz != src_stride) {
                mmpResult = MMP_FAILURE;
                break;
            }

            dst_line += dst_stride;
        }
    }

    /* Load U */
    if(mmpResult == MMP_SUCCESS) {
    
        src_stride = pic_width/2;
        src_height = pic_height/2;

        dst_stride = u_stride;
        dst_line = p_data_u;

        for(h = 0; h < src_height; h++) { 
            rdsz = fread(dst_line, 1, src_stride, fp);
            if(rdsz != src_stride) {
                mmpResult = MMP_FAILURE;
                break;
            }

            dst_line += dst_stride;
        }
    }

    /* Load V */
    if(mmpResult == MMP_SUCCESS) {
    
        src_stride = pic_width/2;
        src_height = pic_height/2;

        dst_stride = v_stride;
        dst_line = p_data_v;

        for(h = 0; h < src_height; h++) { 
            rdsz = fread(dst_line, 1, src_stride, fp);
            if(rdsz != src_stride) {
                mmpResult = MMP_FAILURE;
                break;
            }

            dst_line += dst_stride;
        }
    }

    if(fp != NULL) {
        fclose(fp);
    }

    return mmpResult;
}

MMP_RESULT CMmpImageTool::ConvertYUV420toRGB(MMP_U8* yuv420,
                                        MMP_S32 stride, 
                                        MMP_S32 pic_width, MMP_S32 pic_height, 
                                        MMP_U8* rgb, enum MMP_FOURCC fourcc_rgb) {
    MMP_U8 *yuv420_y, *yuv420_u, *yuv420_v;
    MMP_S32 y_stride, u_stride, v_stride;
    
    yuv420_y = yuv420 + V4L2_YU12_Y_FRAME_OFFSET(pic_width, pic_height);
    yuv420_u = yuv420 + V4L2_YU12_U_FRAME_OFFSET(pic_width, pic_height);
    yuv420_v = yuv420 + V4L2_YU12_V_FRAME_OFFSET(pic_width, pic_height);

    y_stride = stride;
    u_stride = V4L2_YU12_U_STRIDE(stride);
    v_stride = V4L2_YU12_V_STRIDE(stride);

    return CMmpImageTool::ConvertYUV420MtoRGB(yuv420_y, yuv420_u, yuv420_v, 
                                          y_stride, u_stride, v_stride,
                                          pic_width, pic_height,
                                          rgb, fourcc_rgb);

}

MMP_RESULT CMmpImageTool::ConvertYUV420MtoRGB(MMP_U8* yuv420_y, MMP_U8* yuv420_u, MMP_U8* yuv420_v, 
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
        
        for(w = 0; w < pic_width; w++ ) {

             y=(int)(*pix_y); y&=0xff;  
             u=(int)(*pix_u); u&=0xff;  
             v=(int)(*pix_v); v&=0xff;  
  
             /*
                Y = (0.257 * R) + (0.504 * G) + (0.098 * B) + 16 
                Cr = V = (0.439 * R) - (0.368 * G) - (0.071 * B) + 128 
                Cb = U = -(0.148 * R) - (0.291 * G) + (0.439 * B) + 128 

                B = 1.164(Y - 16) + 2.018(U - 128) 
                G = 1.164(Y - 16) - 0.813(V - 128) - 0.391(U - 128) 
                R = 1.164(Y - 16) + 1.596(V - 128) 
            */
#if 0
             y-=16;
             u-=128;
             v-=128;
             b=y+(91881*u)/65536;
             g=y-((22554*u)/65536)-((46802*v)/65536);
             r=y+(116130*v)/65536;
#else
             b = 1164*(y - 16) + 2018*(u - 128);
             g = 1164*(y - 16) - 813*(v - 128) - 391*(u - 128) ;
             r = 1164*(y - 16) + 1596*(v - 128);

             b/=1000;
             g/=1000;
             r/=1000;
#endif
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

MMP_RESULT CMmpImageTool::ConvertNV12MtoRGB(MMP_U8* yuv420_y, MMP_U8* yuv420_uv,
                                            MMP_S32 y_stride, MMP_S32 uv_stride, 
                                            MMP_S32 pic_width, MMP_S32 pic_height, 
                                            MMP_U8* rgb, enum MMP_FOURCC fourcc_rgb) {


    MMP_S32 w, h;
    MMP_S32 r,g,b,y,u,v;

    MMP_U8 *line_y, *line_uv;
    MMP_U8 *line_rgb;
    
    MMP_U8 *pix_y, *pix_uv;
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
    line_uv = yuv420_uv;
    line_rgb = rgb;

    for(h = 0; h < pic_height;h++) {
        
        pix_y = line_y;
        pix_uv = line_uv;
        
        pix_rgb = line_rgb;
        
        for(w = 0; w < pic_width; w++ ) {

             y=(int)(*pix_y); y&=0xff;
             u=(int)(pix_uv[0]); u&=0xff;  u-=128;
             v=(int)(pix_uv[1]); v&=0xff;  v-=128;
        
             b=y+(91881*u)/65536;
             g=y-((22554*u)/65536)-((46802*v)/65536);
             r=y+(116130*v)/65536;
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
                pix_uv+=2;
             }
        }

        line_y += y_stride;
        if( ((h+1)%2) == 0 ) {
            line_uv += uv_stride;
        }
        line_rgb += rgb_stride;
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpImageTool::ConvertYUV444MtoRGB(MMP_U8* yuv420_y, MMP_U8* yuv420_u, MMP_U8* yuv420_v, 
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
        
        for(w = 0; w < pic_width; w++ ) {

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
             pix_u++;
             pix_v++;
        }

        line_y += y_stride;
        line_u += u_stride;
        line_v += v_stride;
        line_rgb += rgb_stride;
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpImageTool::ConvertNV16toRGB(MMP_U8* nv16, 
                                       MMP_S32 pic_width, MMP_S32 pic_height, 
                                       MMP_U8* rgb, enum MMP_FOURCC fourcc_rgb) {
    MMP_U8 *nv16_y, *nv16_uv;
    MMP_S32 y_stride, uv_stride;

    nv16_y = nv16;
    nv16_uv = nv16 + V4L2_NV16_UV_FRAME_OFFSET(pic_width, pic_height);
    y_stride = V4L2_NV16_Y_STRIDE(pic_width);
    uv_stride = V4L2_NV16_UV_STRIDE(pic_width);

    return CMmpImageTool::ConvertNV16MtoRGB(nv16_y, nv16_uv, 
                                        y_stride, uv_stride, 
                                        pic_width, pic_height, 
                                        rgb,  fourcc_rgb);
}

MMP_RESULT CMmpImageTool::ConvertNV16VtoRGB(MMP_U8* nv16v, 
                                       MMP_S32 pic_width, MMP_S32 pic_height, 
                                       MMP_U8* rgb, enum MMP_FOURCC fourcc_rgb) {
    MMP_U8 *nv16v_y, *nv16v_uv;
    MMP_S32 y_stride, uv_stride;

    nv16v_y = nv16v;
    nv16v_uv = nv16v + V4L2_NV16V_UV_FRAME_OFFSET(pic_width, pic_height);
    y_stride = V4L2_NV16V_Y_STRIDE(pic_width);
    uv_stride = V4L2_NV16V_UV_STRIDE(pic_width);

    return CMmpImageTool::ConvertNV16MtoRGB(nv16v_y, nv16v_uv, 
                                        y_stride, uv_stride, 
                                        pic_width, pic_height, 
                                        rgb,  fourcc_rgb);
}

MMP_RESULT CMmpImageTool::ConvertNV16MtoRGB(MMP_U8* nv16_y, MMP_U8* nv16_uv, 
                                        MMP_S32 y_stride, MMP_S32 uv_stride, 
                                        MMP_S32 pic_width, MMP_S32 pic_height, 
                                        MMP_U8* rgb, enum MMP_FOURCC fourcc_rgb) {

    MMP_S32 w, h;
    MMP_S32 r,g,b,y,u,v;

    MMP_U8 *line_y, *line_uv;
    MMP_U8 *line_rgb;
    
    MMP_U8 *pix_y, *pix_uv;
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
    line_y = nv16_y;
    line_uv = nv16_uv;
    line_rgb = rgb;

    for(h = 0; h < pic_height;h++) {
        
        pix_y = line_y;
        pix_uv = line_uv;
        
        pix_rgb = line_rgb;
        
        for(w = 0; w < pic_width; w++ ) {

             y=(int)(*pix_y); y&=0xff;
             u=(int)(*(pix_uv+0)); u&=0xff;  u-=128;
             v=(int)(*(pix_uv+1)); v&=0xff;  v-=128;
         
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
                pix_uv+=2;
             }
        }

        line_y += y_stride;
        line_uv += uv_stride;
        line_rgb += rgb_stride;
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpImageTool::ConvertYUV4441toRGB(MMP_U8* yuv444, 
                                        MMP_S32 yuv_stride, 
                                        MMP_S32 pic_width, MMP_S32 pic_height, 
                                        MMP_U8* rgb, enum MMP_FOURCC fourcc_rgb) {
    MMP_S32 w, h;
    MMP_S32 r,g,b,y,u,v;

    MMP_U8 *line_yuv;
    MMP_U8 *line_rgb;
    
    MMP_U8 *pix_yuv;
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
    line_yuv = yuv444;
    line_rgb = rgb;

    for(h = 0; h < pic_height;h++) {
        
        pix_yuv = line_yuv;
        pix_rgb = line_rgb;
        
        for(w = 0; w < pic_width; w++ ) {

             y=(int)(*pix_yuv); y&=0xff;
             u=(int)(*(pix_yuv+1)); u&=0xff;  u-=128;
             v=(int)(*(pix_yuv+2)); v&=0xff;  v-=128;
         
             r=y+(91881*u)/65536;
             g=y-((22554*u)/65536)-((46802*v)/65536);
             b=y+(116130*v)/65536;
             if(r<0) r=0; if(r>255) r=255;
             if(g<0) g=0; if(g>255) g=255;
             if(b<0) b=0; if(b>255) b=255;

             //RGB32[k]=0xFF000000|r|((g<<16)&0xff00)|((b<<24)&0xff0000);

             if(a_idx>=0) pix_rgb[a_idx]=0xFF;
             pix_rgb[r_idx]=(MMP_U8)r;
             pix_rgb[g_idx]=(MMP_U8)g;
             pix_rgb[b_idx]=(MMP_U8)b;

             pix_rgb+=rgb_pix_width;
             pix_yuv+=3;
        }

        line_yuv += yuv_stride;
        line_rgb += rgb_stride;
    }

    return MMP_SUCCESS;
}


MMP_RESULT CMmpImageTool::ConvertGREYtoRGB(MMP_U8* grey, 
                                        MMP_S32 grey_stride, 
                                        MMP_S32 pic_width, MMP_S32 pic_height, 
                                        MMP_U8* rgb, enum MMP_FOURCC fourcc_rgb) {
    MMP_S32 w, h;
    MMP_S32 r,g,b,y;

    MMP_U8 *line_y;
    MMP_U8 *line_rgb;
    
    MMP_U8 *pix_y;
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
    line_y = grey;
    line_rgb = rgb;

    for(h = 0; h < pic_height;h++) {
        
        pix_y = line_y;
        pix_rgb = line_rgb;
        
        for(w = 0; w < pic_width; w++ ) {

             y=(int)(*pix_y); y&=0xff;
             
             r=y;
             g=y;
             b=y;
             if(r<0) r=0; if(r>255) r=255;
             if(g<0) g=0; if(g>255) g=255;
             if(b<0) b=0; if(b>255) b=255;

             if(a_idx>=0) pix_rgb[a_idx]=0xFF;
             pix_rgb[r_idx]=(MMP_U8)r;
             pix_rgb[g_idx]=(MMP_U8)g;
             pix_rgb[b_idx]=(MMP_U8)b;

             pix_rgb+=rgb_pix_width;
             pix_y++;
        }

        line_y += grey_stride;
        line_rgb += rgb_stride;
    }

    return MMP_SUCCESS;
}
