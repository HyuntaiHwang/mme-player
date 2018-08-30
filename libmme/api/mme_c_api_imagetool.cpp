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

#include "mme_c_api.h"
#include "MmpImageTool.hpp"
#include "v4l2_jpeg_api.h"

int mme_imagetool_bmp_get_info(char* bmp_filename, int* pic_width, int *pic_height, unsigned int *fourcc, int* stride) {
    
    int iret = 0;
    MMP_RESULT mmpResult;

    mmpResult = CMmpImageTool::Bmp_LoadInfo(bmp_filename, (MMP_S32*)pic_width, (MMP_S32*)pic_height, (enum MMP_FOURCC*)fourcc, (MMP_S32*)stride);
    if(mmpResult != MMP_SUCCESS) {
        iret = -1;
    }

    return iret;
}

int mme_imagetool_bmp_load_file(char* bmp_filename, unsigned char* p_data_rgb, int max_data_size) {
    
    int iret = 0;
    MMP_RESULT mmpResult;

    mmpResult = CMmpImageTool::Bmp_LoadFile(bmp_filename, p_data_rgb, max_data_size);
    if(mmpResult != MMP_SUCCESS) {
        iret = -1;
    }

    return iret;
}

int mme_imagetool_bmp_load_file_for_v4l2(char* bmp_filename, struct v4l2_ion_frame* p_v4l2_ion_frame) {

    int iret = 0;
    struct mme_image_buffer mme_img_buf_src;
    struct mme_image_buffer mme_img_buf_dst;
    struct mme_ion_plane dst_plane;
    int max_src_size;
    int i;
        
    memset(&mme_img_buf_src, 0x00, sizeof(mme_img_buf_src));
    memset(&mme_img_buf_dst, 0x00, sizeof(mme_img_buf_dst));
    memset(&dst_plane, 0x00, sizeof(dst_plane));

    
    /* Load bitmap info */
    if(iret == 0) {
        iret = mme_imagetool_bmp_get_info(bmp_filename, &mme_img_buf_src.width, &mme_img_buf_src.height, &mme_img_buf_src.fourcc, &mme_img_buf_src.stride_userptr);
        if(iret == 0) {
            if(mme_img_buf_src.width != p_v4l2_ion_frame->pic_width) iret = -1;
            if(mme_img_buf_src.height != p_v4l2_ion_frame->pic_height) iret = -1;
    
            mme_img_buf_src.type = MME_IMG_BUF_USERPTR;
            max_src_size = mme_img_buf_src.width*mme_img_buf_src.height*4;
    
            if(iret == 0) {
                mme_img_buf_src.m.data = (MMP_U8*)MMP_MALLOC(max_src_size);
                if(mme_img_buf_src.m.data == NULL) {
                    iret = -1;
                }
            }
        }
    }
    
    /* Load bitmap data */
    if(iret == 0) {
        iret = mme_imagetool_bmp_load_file(bmp_filename, mme_img_buf_src.m.data, max_src_size);
    }

    /* config mme_img_buf_dst */
    if(iret == 0) {
        
        mme_img_buf_dst.type = MME_IMG_BUF_ION_PLANE;
        mme_img_buf_dst.width = p_v4l2_ion_frame->pic_width;
        mme_img_buf_dst.height = p_v4l2_ion_frame->pic_height;
        mme_img_buf_dst.stride_userptr = 0;
        mme_img_buf_dst.fourcc = p_v4l2_ion_frame->fourcc;
        mme_img_buf_dst.m.ion_plane = &dst_plane;
        mme_img_buf_dst.m.ion_plane->plane_count = p_v4l2_ion_frame->plane_count;
        for(i = 0; i < mme_img_buf_dst.m.ion_plane->plane_count; i++) {
            mme_img_buf_dst.m.ion_plane->plane[i].shared_fd = p_v4l2_ion_frame->plane[i].shared_fd;
            mme_img_buf_dst.m.ion_plane->plane[i].buf_size = p_v4l2_ion_frame->plane[i].buf_size;
            mme_img_buf_dst.m.ion_plane->plane[i].vir_addr = p_v4l2_ion_frame->plane[i].vir_addr;
            mme_img_buf_dst.m.ion_plane->plane[i].mem_offset = p_v4l2_ion_frame->plane[i].mem_offset;
            mme_img_buf_dst.m.ion_plane->plane[i].stride = p_v4l2_ion_frame->plane[i].stride;
        }
    }

    /* color convert */
    if(iret == 0) {
        
        iret = mme_imagetool_convert_color(&mme_img_buf_src, &mme_img_buf_dst);
    }

    if(mme_img_buf_src.m.data != NULL) {
        MMP_FREE(mme_img_buf_src.m.data);
    }

    return iret;
}

int mme_imagetool_yuv_load_file_for_v4l2(char* yuv_filename, struct v4l2_ion_frame* p_v4l2_ion_frame) {

    MMP_RESULT mmpResult;
    int iret = -1;

    mmpResult = CMmpImageTool::YUV_LoadFile_YUV420P3(yuv_filename, 
                                         p_v4l2_ion_frame->pic_width, p_v4l2_ion_frame->pic_height, 
                                         (MMP_U8*)p_v4l2_ion_frame->plane[0].vir_addr, 
                                         (MMP_U8*)p_v4l2_ion_frame->plane[1].vir_addr, 
                                         (MMP_U8*)p_v4l2_ion_frame->plane[2].vir_addr, 
                                         p_v4l2_ion_frame->plane[0].stride,
                                         p_v4l2_ion_frame->plane[1].stride,
                                         p_v4l2_ion_frame->plane[2].stride
                                         );

    if(mmpResult == MMP_SUCCESS) {
        iret = 0;
    }
    
    return iret;
}

int mme_imagetool_convert_color(struct mme_image_buffer *p_src_buf, struct mme_image_buffer* p_dst_buf) {

    int i, iret = 0;
    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U8 *p_src_data[MME_MAX_PLANE_COUNT] = {NULL, NULL, NULL};
    MMP_U8 *p_dst_data[MME_MAX_PLANE_COUNT] = {NULL, NULL, NULL};
    MMP_S32 pic_width, pic_height;
    enum MMP_FOURCC fourcc_src, fourcc_dst;
    MMP_S32 dst_stride_arr[3];

    /* set src param */
    pic_width = p_src_buf->width;
    pic_height = p_src_buf->height;
    fourcc_src = (enum MMP_FOURCC)p_src_buf->fourcc;
    switch(p_src_buf->type) {
        case MME_IMG_BUF_USERPTR:     p_src_data[0] = p_src_buf->m.data; break;
        case MME_IMG_BUF_HEAP_PLANE :
            for(i = 0; i < p_src_buf->m.heap_plane->plane_count; i++) {
                p_src_data[i] = p_src_buf->m.heap_plane->p_buf[i];
            }
            break;
        case MME_IMG_BUF_ION_PLANE :
            for(i = 0; i < p_src_buf->m.ion_plane->plane_count; i++) {
                p_src_data[i] = (MMP_U8*)p_src_buf->m.ion_plane->plane[i].vir_addr;
            }
            break;
        default:
            iret = -1;
    }

    /* set dst param */
    fourcc_dst = (enum MMP_FOURCC)p_dst_buf->fourcc;
    switch(p_dst_buf->type) {
        case MME_IMG_BUF_USERPTR:     p_dst_data[0] = p_dst_buf->m.data; break;
        case MME_IMG_BUF_HEAP_PLANE :
            for(i = 0; i < p_dst_buf->m.heap_plane->plane_count; i++) {
                p_dst_data[i] = p_dst_buf->m.heap_plane->p_buf[i];
                dst_stride_arr[i] = p_dst_buf->m.heap_plane->stride[i];
            }
            break;
        case MME_IMG_BUF_ION_PLANE :
            for(i = 0; i < p_dst_buf->m.ion_plane->plane_count; i++) {
                p_dst_data[i] = (MMP_U8*)p_dst_buf->m.ion_plane->plane[i].vir_addr;
                dst_stride_arr[i] = p_dst_buf->m.ion_plane->plane[i].stride;
            }
            break;
        default:
            iret = -1;
    }

    if(iret == 0 ) {

        iret = -1;
        mmpResult = MMP_FAILURE;

        switch(fourcc_src) {
        
            case MMP_FOURCC_IMAGE_RGB888:
            case MMP_FOURCC_IMAGE_ARGB8888:

                switch(fourcc_dst) {

                    case MMP_FOURCC_IMAGE_YUYV: 
                    case MMP_FOURCC_IMAGE_YVYU: 
                    case MMP_FOURCC_IMAGE_UYVY: 
                    case MMP_FOURCC_IMAGE_VYUY: 
                        mmpResult = CMmpImageTool::ConvertRGBtoYUYV(p_src_data[0], pic_width, pic_height, fourcc_src,
                                                                    p_dst_data[0], fourcc_dst);
                        break;

                    case MMP_FOURCC_IMAGE_NV16: // "NV16" YUV 4:2:2 planar, Y/CbCr    16 bit Y/CbCr 4:2:2 Plane 2, V4L2_PIX_FMT_NV16
                        mmpResult = CMmpImageTool::ConvertRGBtoNV16(p_src_data[0], pic_width, pic_height, fourcc_src,
                                                                     p_dst_data[0]);
                        break;
                    
                    case MMP_FOURCC_IMAGE_NV16V: // "N16V" YUV 2:2:4 (Vertical) planar, Y/CbCr    16 bit Y/CbCr 4:2:2 Plane 2, V4L2_PIX_FMT_N16V
                        mmpResult = CMmpImageTool::ConvertRGBtoNV16V(p_src_data[0], pic_width, pic_height, fourcc_src,
                                                                     p_dst_data[0]);
                        break;
                    
                    case MMP_FOURCC_IMAGE_NV61: 
                        mmpResult = CMmpImageTool::ConvertRGBtoNV61(p_src_data[0], pic_width, pic_height, fourcc_src,
                                                                     p_dst_data[0]);
                        break;
                
                    case MMP_FOURCC_IMAGE_YUV422P: //V4L2_PIX_FMT_YUV422P "422P" Plane1 Comp3  Bpp(8,4,4)  YUV 4:2:2 3-planar, Y/Cb/Cr
                        mmpResult = CMmpImageTool::ConvertRGBtoYUV422P(p_src_data[0], pic_width, pic_height, fourcc_src,
                                                                     p_dst_data[0]);
                        break;
                
                    case MMP_FOURCC_IMAGE_NV12:
                        mmpResult = CMmpImageTool::ConvertRGBtoNV12(p_src_data[0], pic_width, pic_height, fourcc_src,
                                                                     p_dst_data[0]);
                        break;
                
                    case MMP_FOURCC_IMAGE_NV12M:
                        mmpResult = CMmpImageTool::ConvertRGBtoNV12M(p_src_data[0], pic_width, pic_height, fourcc_src,
                                                                     p_dst_data[0], p_dst_data[1], 
                                                                     dst_stride_arr[0], dst_stride_arr[1]);
                        break;
                
                    case MMP_FOURCC_IMAGE_NV21:
                        mmpResult = CMmpImageTool::ConvertRGBtoNV21(p_src_data[0], pic_width, pic_height, fourcc_src,
                                                                     p_dst_data[0]);
                        break;

                    case MMP_FOURCC_IMAGE_NV21M:
                        mmpResult = CMmpImageTool::ConvertRGBtoNV21M(p_src_data[0], pic_width, pic_height, fourcc_src,
                                                                     p_dst_data[0], p_dst_data[1], 
                                                                     dst_stride_arr[0], dst_stride_arr[1]);
                        break;
                
                    case MMP_FOURCC_IMAGE_YUV420M : /* Non Contiguous */
                        mmpResult = CMmpImageTool::ConvertRGBtoYUV420M(p_src_data[0], pic_width, pic_height, fourcc_src,
                                                                     p_dst_data[0], p_dst_data[1], p_dst_data[2], 
                                                                     dst_stride_arr[0],dst_stride_arr[1],dst_stride_arr[2]);
                        break;
                
                    case MMP_FOURCC_IMAGE_YUV420 :
                        mmpResult = CMmpImageTool::ConvertRGBtoYUV420(p_src_data[0], pic_width, pic_height, fourcc_src,
                                                                p_dst_data[0] ); 
                        break;
                
                    case MMP_FOURCC_IMAGE_YVU420 :
                        mmpResult = CMmpImageTool::ConvertRGBtoYVU420(p_src_data[0], pic_width, pic_height, fourcc_src,
                                                                p_dst_data[0] ); 
                        break;
                
                    case MMP_FOURCC_IMAGE_YUV444_P1 :
                        mmpResult = CMmpImageTool::ConvertRGBtoYUV444_P1(p_src_data[0], pic_width, pic_height, fourcc_src,
                                                                         p_dst_data[0], dst_stride_arr[0]);
                        break;
                
                    case MMP_FOURCC_IMAGE_GREY :
                        mmpResult = CMmpImageTool::ConvertRGBtoGREY(p_src_data[0], pic_width, pic_height, fourcc_src,
                                                                    p_dst_data[0], dst_stride_arr[0]);
                        break;
                
                    default:
                         mmpResult = MMP_FAILURE;
                }
                
                break;

        }

        if(mmpResult == MMP_SUCCESS) {
            iret = 0;
        }

    }

    return iret;
}

int mme_imagetool_jpeg_get_width_height_fourcc(char* jpegfilename, int* pic_width, int* pic_height, unsigned int* v4l2_fourcc) {
    
    FILE* fp;
    int iret = 0;
    int jpegsize = 0;
    MMP_U8* jpegdata = NULL;
    MMP_RESULT mmpResult;
    MMP_FOURCC mmp_fourcc;

    if(iret == 0) {
        fp = fopen(jpegfilename, "rb");
        if(fp == NULL) {
            iret = -1;
        }
        else {
            fseek(fp, 0, SEEK_END);
            jpegsize = ftell(fp);
            fseek(fp, 0, SEEK_SET);

            jpegdata = (MMP_U8*)MMP_MALLOC(jpegsize);
            if(jpegdata!=NULL) {
                fread(jpegdata, 1, jpegsize, fp);
            }
            else {
                iret = -1;
            }
            fclose(fp);
        }
    }

    mmpResult = CMmpImageTool::Jpeg_GetWidthHeightFourcc(jpegdata, jpegsize, (MMP_S32*)pic_width, (MMP_S32*)pic_height, &mmp_fourcc);

    if(jpegdata != NULL) {
        MMP_FREE(jpegdata);
    }

    return iret;
}