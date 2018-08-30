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
#include "mmp_buffer_mgr.hpp"
#include "MmpUtil.hpp"
#include "v4l2_jpeg_api.h"

int mme_jpeg_decode_libjpeg(unsigned char* jpegdata, int jpegsize, struct mme_image_buffer* p_dst_buf) {
    
    MMP_S32 pic_width, pic_height;
    enum MMP_FOURCC fourcc_rgb, fourcc_dst;
    MMP_RESULT mmpResult;
    MMP_S32 i, iret = 0;
    MMP_U8* p_decoded_rgb = NULL;
    MMP_S32 dst_stride_arr[3];
    MMP_U8* p_dst_data[3];
    
    /* get jpeg width, height */
    mmpResult = CMmpImageTool::Jpeg_GetWidthHeightFourcc(jpegdata, jpegsize, &pic_width, &pic_height, NULL);
    if(mmpResult != MMP_SUCCESS) {
        iret = -1;
    }

    /* alloc rgb decoded buf */
    if(iret == 0) {
        p_decoded_rgb = (MMP_U8*)MMP_MALLOC(pic_width*pic_height*4);
        if(p_decoded_rgb == NULL) {
            iret = -1;
        }
    }
       
    /* do decoding */
    if(iret == 0) {
          mmpResult = CMmpImageTool::Jpeg_Decode_libjpeg_BGR(jpegdata, jpegsize, 
                                                            &pic_width, &pic_height, &fourcc_rgb, p_decoded_rgb);
          if(mmpResult != MMP_SUCCESS) {
              iret = -1;
          }
    }

    /* check param */
    if(pic_width != p_dst_buf->width) iret = -1;
    if(pic_height != p_dst_buf->height) iret = -1;
    
    /* color convert */
    if(iret == 0) {

        mmpResult = MMP_FAILURE;
        fourcc_dst = (enum MMP_FOURCC)p_dst_buf->fourcc;
        
        switch(p_dst_buf->type) {
            case MME_IMG_BUF_USERPTR:     
                p_dst_data[0] = p_dst_buf->m.data; 
                dst_stride_arr[0] = p_dst_buf->stride_userptr;
                break;
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

        if(fourcc_rgb == MMP_FOURCC_IMAGE_GREY) {
        
            memcpy((void*)p_dst_data[0], p_decoded_rgb, V4L2_GREY_STRIDE(pic_width)*pic_height);
            mmpResult = MMP_SUCCESS;
        }
        else {
            switch(fourcc_dst) {
            
                case MMP_FOURCC_IMAGE_NV16:    
                    mmpResult = CMmpImageTool::ConvertRGBtoNV16(p_decoded_rgb, pic_width, pic_height, fourcc_rgb, 
                                                                p_dst_data[0]);
                    break;

                case MMP_FOURCC_IMAGE_NV16V:    
                    mmpResult = CMmpImageTool::ConvertRGBtoNV16V(p_decoded_rgb, pic_width, pic_height, fourcc_rgb, 
                                                                p_dst_data[0]);
                    break;

                case MMP_FOURCC_IMAGE_YUV420:    
                    mmpResult = CMmpImageTool::ConvertRGBtoYUV420(p_decoded_rgb, pic_width, pic_height, fourcc_rgb, 
                                                                  p_dst_data[0]);
                    break;

                case MMP_FOURCC_IMAGE_YUV420M:    
                    mmpResult = CMmpImageTool::ConvertRGBtoYUV420M(p_decoded_rgb, pic_width, pic_height, fourcc_rgb, 
                                                                     p_dst_data[0], p_dst_data[1], p_dst_data[2], 
                                                                     dst_stride_arr[0],dst_stride_arr[1],dst_stride_arr[2]);
                    break;

    #if 0
                case MMP_FOURCC_IMAGE_YCbCr422_P2:
                    mmpResult = CMmpImageTool::ConvertRGBtoYCbCr422_P2(p_decoded_rgb, pic_width, pic_height, fourcc_rgb, 
                                                                     p_dst_data[0], p_dst_data[1], 
                                                                     dst_stride_arr[0], dst_stride_arr[1] );
                    break;
    #endif

                case MMP_FOURCC_IMAGE_YUV444_P1:
                    mmpResult = CMmpImageTool::ConvertRGBtoYUV444_P1(p_decoded_rgb, pic_width, pic_height, fourcc_rgb, 
                                                                     p_dst_data[0], dst_stride_arr[0]);
                    break;

                case MMP_FOURCC_IMAGE_YUV444M:
                    mmpResult = CMmpImageTool::ConvertRGBtoYUV444M(p_decoded_rgb, pic_width, pic_height, fourcc_rgb, 
                                                                     p_dst_data[0], p_dst_data[1], p_dst_data[2], 
                                                                     dst_stride_arr[0],dst_stride_arr[1],dst_stride_arr[2]);
                    break;

                case MMP_FOURCC_IMAGE_GREY:
                    mmpResult = CMmpImageTool::ConvertRGBtoGREY(p_decoded_rgb, pic_width, pic_height, fourcc_rgb, 
                                                                p_dst_data[0], dst_stride_arr[0]);
                    break;
            }
        }

        if(mmpResult != MMP_SUCCESS) {
            iret = -1;
        }
    }


    /* free rgb buf */
    if(p_decoded_rgb != NULL) {
        MMP_FREE(p_decoded_rgb);
    }

    return 0;
}


int mme_jpeg_encode_libjpeg(struct mme_image_buffer* p_src_buf, int quality  /* rage 0~100 */, unsigned char* jpegdata, int jpegdata_max_size, int* jpegsize) {


    int i, iret = 0;
    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U8 *p_src_data[MME_MAX_PLANE_COUNT] = {NULL, NULL, NULL};
    MMP_U8 *p_yuv444_alloc_data = NULL;
    MMP_S32 pic_width, pic_height;
    MMP_U8 *p_yuv444_data;
    enum MMP_FOURCC fourcc_src;
    MMP_S32 src_stride_arr[3], yuv444_stride;
    
    if(jpegsize != NULL) *jpegsize = 0;

    /* set src param */
    pic_width = p_src_buf->width;
    pic_height = p_src_buf->height;
    fourcc_src = (enum MMP_FOURCC)p_src_buf->fourcc;
    switch(p_src_buf->type) {
        case MME_IMG_BUF_USERPTR:     
            p_src_data[0] = p_src_buf->m.data; 
            src_stride_arr[0] = p_src_buf->stride_userptr;
            break;
        case MME_IMG_BUF_HEAP_PLANE :
            for(i = 0; i < p_src_buf->m.heap_plane->plane_count; i++) {
                p_src_data[i] = p_src_buf->m.heap_plane->p_buf[i];
                src_stride_arr[i] = p_src_buf->m.heap_plane->stride[i];
            }
            break;
        case MME_IMG_BUF_ION_PLANE :
            for(i = 0; i < p_src_buf->m.ion_plane->plane_count; i++) {
                p_src_data[i] = (MMP_U8*)p_src_buf->m.ion_plane->plane[i].vir_addr;
                src_stride_arr[i] = p_src_buf->m.ion_plane->plane[i].stride;
            }
            break;
        default:
            iret = -1;
    }

    yuv444_stride = pic_width*3;
    p_yuv444_alloc_data = (MMP_U8*)MMP_MALLOC(yuv444_stride *pic_height);
    if(p_yuv444_alloc_data != NULL) {

        if(p_src_buf->fourcc != V4L2_PIX_FMT_YUV444_P1) {
            p_yuv444_data = p_yuv444_alloc_data;
        }
            
        switch(p_src_buf->fourcc) {
    
            case V4L2_PIX_FMT_YUV444_P1:
                p_yuv444_data = p_src_data[0];
                yuv444_stride = src_stride_arr[0];
                break;

            case V4L2_PIX_FMT_UYVY : //              "UYVY"         1              1                    16         1               YUV 4:2:2 packed, CbYCrY      
            case V4L2_PIX_FMT_VYUY : //              "VYUY"         1              1                    16         1               YUV 4:2:2 packed, CrYCbY
            case V4L2_PIX_FMT_YUYV : //              "YUYV"         1              1                    16         1               YUV 4:2:2 packed, YCbYCr
            case V4L2_PIX_FMT_YVYU : //              "YVYU"         1              1                    16         1               YUV 4:2:2 packed, YCrYCb
                CMmpImageTool::ConvertYUYVtoYUV444P1(p_src_data[0], (enum MMP_FOURCC)p_src_buf->fourcc, 
                                                    pic_width, pic_height, 
                                                    p_yuv444_alloc_data, yuv444_stride);
                break;

            case V4L2_PIX_FMT_NV16 : //              "NV16"         1              2                    8,8        1,1            YUV 4:2:2 planar, Y/CbCr
                CMmpImageTool::ConvertNV16toYUV444P1(p_src_data[0],
                                                    pic_width, pic_height, 
                                                    p_yuv444_alloc_data, yuv444_stride);
                break;

            case V4L2_PIX_FMT_NV16V : //              "N16V"         1              2                    8,8        1,1            YUV 4:2:2 planar, Y/CbCr
                CMmpImageTool::ConvertNV16VtoYUV444P1(p_src_data[0],
                                                    pic_width, pic_height, 
                                                    p_yuv444_alloc_data, yuv444_stride);
                break;

            case V4L2_PIX_FMT_NV61 : //              "NV61"         1              2                    8,8        1,1            YUV 4:2:2 planar, Y/CrCb
                CMmpImageTool::ConvertNV61toYUV444P1(p_src_data[0],
                                                    pic_width, pic_height, 
                                                    p_yuv444_alloc_data, yuv444_stride);
                break;

            case V4L2_PIX_FMT_YUV422P : //         "422P"         1              3                    8,4,4     1,1,1          YUV 4:2:2 3-planar, Y/Cb/Cr
                CMmpImageTool::ConvertYUV422PtoYUV444P1(p_src_data[0],
                                                    pic_width, pic_height, 
                                                    p_yuv444_alloc_data, yuv444_stride);
                break;

            case V4L2_PIX_FMT_NV12 : //              "NV12"         1              2                    8,4        1,2            YUV 4:2:0 contiguous 2-planar, Y/CbCr
                CMmpImageTool::ConvertNV12toYUV444P1(p_src_data[0],
                                                    pic_width, pic_height, 
                                                    p_yuv444_alloc_data, yuv444_stride);
                break;

            case V4L2_PIX_FMT_NV12M : //           "NM12"         2              2                    8,4        1,2            YUV 4:2:0 non-contiguous 2-planar, Y/CbCr
                CMmpImageTool::ConvertNV12MtoYUV444P1(p_src_data[0], p_src_data[1], 
                                                    src_stride_arr[0], src_stride_arr[1],
                                                    pic_width, pic_height, 
                                                    p_yuv444_alloc_data, yuv444_stride);

                break;

            case V4L2_PIX_FMT_NV21 : //              "NV21"         1              2                    8,4        1,2            YUV 4:2:0 contiguous 2-planar, Y/CrCb
                CMmpImageTool::ConvertNV21toYUV444P1(p_src_data[0],
                                                    pic_width, pic_height, 
                                                    p_yuv444_alloc_data, yuv444_stride);
                break;

            case V4L2_PIX_FMT_NV21M : //           "NM21"         2              2                    8,4        1,2            YUV 4:2:0 non-contiguous 2-planar, Y/CrCb
                CMmpImageTool::ConvertNV21MtoYUV444P1(p_src_data[0], p_src_data[1], 
                                                    src_stride_arr[0], src_stride_arr[1],
                                                    pic_width, pic_height, 
                                                    p_yuv444_alloc_data, yuv444_stride);

                break;

            case V4L2_PIX_FMT_YUV420M : //        "YM12"         3              3                    8,2,2     1,2,2          YUV 4:2:0 non-contiguous 3-planar, Y/Cb/Cr
                CMmpImageTool::ConvertYUV420MtoYUV444P1(p_src_data[0], p_src_data[1], p_src_data[2], 
                                                        src_stride_arr[0], src_stride_arr[1], src_stride_arr[2],
                                                        pic_width, pic_height, 
                                                        p_yuv444_alloc_data, yuv444_stride);

                break;
                                                                                                                               
            case V4L2_PIX_FMT_YUV420 : //        "YU12"         3              3                    8,2,2     1,2,2          YUV 4:2:0 contiguous 3-planar, Y/Cb/Cr

                CMmpImageTool::ConvertYUV420toYUV444P1(p_src_data[0],
                                                       pic_width, pic_height, 
                                                       p_yuv444_alloc_data, yuv444_stride);

                break;

            case V4L2_PIX_FMT_GREY : //        "YU12"         3              3                    8,2,2     1,2,2          YUV 4:2:0 contiguous 3-planar, Y/Cb/Cr

                //CMmpImageTool::ConvertGREYtoYUV444P1(p_src_data[0],
                //                                     pic_width, pic_height, 
                //                                     p_yuv444_alloc_data, yuv444_stride);

                break;

            default:
                iret = -1;
                break;
        }

        if(p_src_buf->fourcc == V4L2_PIX_FMT_GREY) {
        
            mmpResult = CMmpImageTool::Jpeg_Encode_libjpeg_GREY(p_src_data[0], 
                                            pic_width, pic_height, V4L2_GREY_STRIDE(pic_width), quality,
                                            jpegdata, jpegdata_max_size, (MMP_S32*)jpegsize);
            if(mmpResult != MMP_SUCCESS) {
                iret = -1;
            }
        }
        else  if(iret == 0) {
            mmpResult = CMmpImageTool::Jpeg_Encode_libjpeg_YUV444_P1(p_yuv444_data, 
                                            pic_width, pic_height, yuv444_stride, quality,
                                            jpegdata, jpegdata_max_size, (MMP_S32*)jpegsize);
            if(mmpResult != MMP_SUCCESS) {
                iret = -1;
            }
        }
    }

    if(p_yuv444_alloc_data != NULL) {
        MMP_FREE(p_yuv444_alloc_data);
    }
    
    return iret;
}

extern MMP_RESULT CMmpEncodeImage_Jpu_EncodeAu(class mmp_buffer_imageframe* p_buf_imageframe, class mmp_buffer_imagestream* p_buf_imagestream);

int mme_jpeg_encode_jpu(struct mme_image_buffer* p_src_buf, int quality  /* rage 0~100 */, unsigned char* jpegdata, int jpegdata_max_size, int* jpegsize) {

#if (JPU_PLATFORM_V4L2_ENABLE == 1)
    if(jpegsize != NULL) *jpegsize = 0;

    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("!!!! JPU_PLATFORM_V4L2_ENABLE is ON ,  To use JPU directly, turn off this option. ")));

    return -1;
#else

    int i, iret = 0;
    class mmp_buffer_imageframe* p_buf_imageframe = NULL;
    class mmp_buffer_imagestream* p_buf_imagestream = NULL;

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_S32 src_shared_ion_fd[MME_MAX_PLANE_COUNT] = {-1, -1, -1};
    MMP_S32 src_ion_memoffset[MME_MAX_PLANE_COUNT] = {0, 0, 0};
    MMP_S32 pic_width, pic_height;
    enum MMP_FOURCC fourcc_src;
    MMP_S32 src_stride;
    
    if(jpegsize != NULL) *jpegsize = 0;

    /* set src param */
    pic_width = p_src_buf->width;
    pic_height = p_src_buf->height;
    fourcc_src = (enum MMP_FOURCC)p_src_buf->fourcc;
    src_stride = p_src_buf->stride;
    switch(p_src_buf->type) {
        case MME_IMG_BUF_USERPTR:     
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[mme_jpeg_encode_jpu] FAIL: not support MME_IMG_BUF_USERPTR (only MME_IMG_BUF_ION_PLANE) ")));
            iret = -1;
            break;
        case MME_IMG_BUF_HEAP_PLANE :
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[mme_jpeg_encode_jpu] FAIL: not support MME_IMG_BUF_HEAP_PLANE (only MME_IMG_BUF_ION_PLANE) ")));
            iret = -1;
            break;
        case MME_IMG_BUF_ION_PLANE :

            for(i = 0; i < p_src_buf->m.ion_plane->plane_count; i++) {
                src_shared_ion_fd[i] = p_src_buf->m.ion_plane->plane[i].shared_fd;
                src_ion_memoffset[i] = p_src_buf->m.ion_plane->plane[i].mem_offset; 
            }

            /* attach images frame */
            p_buf_imageframe = mmp_buffer_mgr::get_instance()->attach_media_imageframe(src_shared_ion_fd, src_ion_memoffset,
                                                                                       pic_width, pic_height, fourcc_src);
            if(p_buf_imageframe == NULL) {
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[mme_jpeg_encode_jpu] FAIL: attach image frame")));
                iret = -1;
            }   

            /* alloc image stream */
            p_buf_imagestream = mmp_buffer_mgr::get_instance()->alloc_media_imagestream( MMP_BYTE_ALIGN(jpegdata_max_size, 1024), mmp_buffer::ION);
            if(p_buf_imagestream == NULL) {
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[mme_jpeg_encode_jpu] FAIL: alloc image stream")));
                iret = -1;
            }
            
            break;
        default:
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[mme_jpeg_encode_jpu] FAIL: unknown MME_IMNG_BUF TYPE ")));
            iret = -1;
    }

    /* encoddng */
    if(iret == 0) {
        mmpResult = CMmpEncodeImage_Jpu_EncodeAu(p_buf_imageframe, p_buf_imagestream);
        if(mmpResult != MMP_SUCCESS) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[mme_jpeg_encode_jpu] FAIL: CMmpEncodeImage_Jpu_EncodeAu")));
            iret = -1;
        }
        else {
            if(p_buf_imagestream->get_stream_real_size() > 0) {
                memcpy(jpegdata, p_buf_imagestream->get_stream_real_ptr(), p_buf_imagestream->get_stream_real_size());
                if(jpegsize != NULL) *jpegsize = p_buf_imagestream->get_stream_real_size();
            }
        }
    }


    /* free image frame */
    if(p_buf_imageframe != NULL) {
        mmp_buffer_mgr::get_instance()->free_media_buffer(p_buf_imageframe);
    }

    /* free image stream */
    if(p_buf_imagestream != NULL) {
        mmp_buffer_mgr::get_instance()->free_media_buffer(p_buf_imagestream);
    }

    return iret;
#endif
}

int mme_jpeg_encode_v4l2(struct mme_image_buffer* p_src_buf, int quality  /* rage 0~100 */, unsigned char* jpegdata, int jpegdata_max_size, int* jpegsize) {

#if (JPU_PLATFORM_V4L2_ENABLE != 1)
    if(jpegsize != NULL) *jpegsize = 0;

    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("!!!! JPU_PLATFORM_V4L2_ENABLE is OFF ,  To use JPU-V4L2, turn on this option. ")));

    return -1;
#else

#if 0
    int iret = 0, i;
    struct v4l2_ion_frame ion_frame;

    ion_frame.width = p_src_buf->width;
    ion_frame.height = p_src_buf->height;
    ion_frame.stride = p_src_buf->stride;
    switch(p_src_buf->fourcc) {
    
        case MME_FOURCC_IMAGE_YUV444_P1:
           ion_frame.pix_fmt = V4L2_PIX_FMT_YUV444_P1;
           break;

        default:
            iret = -1;
    }
    
    switch(p_src_buf->type) {
        case MME_IMG_BUF_USERPTR:     
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[mme_jpeg_encode_v4l2] FAIL: not support MME_IMG_BUF_USERPTR (only MME_IMG_BUF_ION_PLANE) ")));
            iret = -1;
            break;
        case MME_IMG_BUF_HEAP_PLANE :
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[mme_jpeg_encode_v4l2] FAIL: not support MME_IMG_BUF_HEAP_PLANE (only MME_IMG_BUF_ION_PLANE) ")));
            iret = -1;
            break;
        case MME_IMG_BUF_ION_PLANE :

            ion_frame.plane_count = p_src_buf->m.ion_plane->plane_count;
            for(i = 0; i < p_src_buf->m.ion_plane->plane_count; i++) {
                ion_frame.plane[i].shared_fd = p_src_buf->m.ion_plane->plane[i].shared_fd;
                ion_frame.plane[i].buf_size = p_src_buf->m.ion_plane->plane[i].buf_size;
                ion_frame.plane[i].vir_addr = p_src_buf->m.ion_plane->plane[i].vir_addr;
                ion_frame.plane[i].mem_offset = p_src_buf->m.ion_plane->plane[i].mem_offset;
            }
            break;

        default:
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[mme_jpeg_encode_v4l2] FAIL: unknown MME_IMNG_BUF TYPE ")));
            iret = -1;
    }


    if(iret == 0) {
        iret = v4l2_jpeg_encode_ion_heap(&ion_frame, quality, jpegdata, jpegdata_max_size, jpegsize);
    }

    return iret;
#else
    return -1;
#endif

#endif
}