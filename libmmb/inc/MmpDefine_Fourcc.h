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

#ifndef MMPDEFINE_FOURCC_H__
#define MMPDEFINE_FOURCC_H__

#include "v4l2_api_def.h"
//#include <linux/videodev2.h>

enum MMP_FOURCC {

    MMP_FOURCC_UNKNOWN = 0,

    MMP_FOURCC_AUDIO_UNKNOWN = MMPMAKEFOURCC('A','U','D','?'),
    MMP_FOURCC_AUDIO_PCM     = MMPMAKEFOURCC('P','C','M',' '),  /* PCM */
    MMP_FOURCC_AUDIO_CODEC_PCM_S16LE = MMPMAKEFOURCC('P','C','M','0'),  /* PCM Codec S16LE */
    MMP_FOURCC_AUDIO_MP1     = MMPMAKEFOURCC('M','P','1','A'),  /* Mpeg Layer 1 */
    MMP_FOURCC_AUDIO_MP2     = MMPMAKEFOURCC('M','P','2','A'),  /* Mpeg Layer 2 */
    MMP_FOURCC_AUDIO_MP3     = MMPMAKEFOURCC('M','P','3','A'),  /* Mpeg Layer 3 */
    MMP_FOURCC_AUDIO_VORBIS  = MMPMAKEFOURCC('V','O','R','B'),  /* Vorbis */
    MMP_FOURCC_AUDIO_BSAC    = MMPMAKEFOURCC('B','S','A','C'),  /* Korea TDMB Audio Bsac */
    MMP_FOURCC_AUDIO_AAC     = MMPMAKEFOURCC('A','A','C',' '),  /* AAC */
    MMP_FOURCC_AUDIO_AC3     = MMPMAKEFOURCC('A','C','3',' '),  /* AC3 */
    MMP_FOURCC_AUDIO_RA_COOK = MMPMAKEFOURCC('C','O','O','K'),  /* Real Audio COOK */
    MMP_FOURCC_AUDIO_RA_RAAC = MMPMAKEFOURCC('R','A','A','C'),  /* Real Audio RAAC */
    MMP_FOURCC_AUDIO_RA_SIPR = MMPMAKEFOURCC('S','I','P','R'),  /* Real Audio SIPR */
    MMP_FOURCC_AUDIO_DTS     = MMPMAKEFOURCC('D','T','S',' '),  /* Real Audio SIPR */
    MMP_FOURCC_AUDIO_WMA1    = MMPMAKEFOURCC('W','M','A','1'),  /* WMA 1 */
    MMP_FOURCC_AUDIO_WMA2    = MMPMAKEFOURCC('W','M','A','2'),  /* WMA 2 */
    MMP_FOURCC_AUDIO_WMA_LOSSLESS    = MMPMAKEFOURCC('W','M','A','L'),  /* WMA LossLess */
    MMP_FOURCC_AUDIO_WMA_PRO    = MMPMAKEFOURCC('W','M','A','P'),  /* WMA Pro */
    MMP_FOURCC_AUDIO_WMA_VOICE    = MMPMAKEFOURCC('W','M','A','V'),  /* WMA Voice */
    MMP_FOURCC_AUDIO_FLAC    = MMPMAKEFOURCC('F','L','A','C'),  /* WMA Voice */
    MMP_FOURCC_AUDIO_AMR_NB  = MMPMAKEFOURCC('A','M','R','N'),  /* AMR NB */
    MMP_FOURCC_AUDIO_AMR_WB  = MMPMAKEFOURCC('A','M','R','W'),  /* AMR NB */
    MMP_FOURCC_AUDIO_FFMPEG  = MMPMAKEFOURCC('F','F','M','A'),  /* Ffmpeg Audio */

    MMP_FOURCC_VIDEO_UNKNOWN = MMPMAKEFOURCC('V','I','D','?'),
    
    MMP_FOURCC_VIDEO_H264    = V4L2_PIX_FMT_H264,           // "H264" H264 with start codes  
    MMP_FOURCC_VIDEO_HEVC    = MMPMAKEFOURCC('H','E','V','C'),           
    MMP_FOURCC_VIDEO_H264_NO_SC  = V4L2_PIX_FMT_H264_NO_SC, //"AVC1"  H264 without start codes
    MMP_FOURCC_VIDEO_H264_MVC    = V4L2_PIX_FMT_H264_MVC,   // "M264"  H264 MVC

    MMP_FOURCC_VIDEO_H263    = V4L2_PIX_FMT_H263,  // "H263" 

    MMP_FOURCC_VIDEO_MPEG4   = V4L2_PIX_FMT_MPEG4, // "MPG4" MPEG-4 ES 
    MMP_FOURCC_VIDEO_MPEG2   = MMPMAKEFOURCC('M','P','G','2'),
    MMP_FOURCC_VIDEO_MPEG4_V2= MMPMAKEFOURCC('M','P','4','2'),
    MMP_FOURCC_VIDEO_XVID    = MMPMAKEFOURCC('X','V','I','D'),

    MMP_FOURCC_VIDEO_WVC1     = MMPMAKEFOURCC('W','V','C','1'),  //VC1 Advanced@L3
    MMP_FOURCC_VIDEO_WMV3     = MMPMAKEFOURCC('W','M','V','3'),  //VC1-WMV9 
    MMP_FOURCC_VIDEO_WMV2     = MMPMAKEFOURCC('W','M','V','2'),  //VC1-WMV8 
    MMP_FOURCC_VIDEO_WMV1     = MMPMAKEFOURCC('W','M','V','1'),  //VC1-WMV7 
    MMP_FOURCC_VIDEO_MSS1     = MMPMAKEFOURCC('M','S','S','1'),  
    MMP_FOURCC_VIDEO_MSS2     = MMPMAKEFOURCC('M','S','S','2'),  

    MMP_FOURCC_VIDEO_VP60     = MMPMAKEFOURCC('V','P','6','0'),  
    MMP_FOURCC_VIDEO_VP6F     = MMPMAKEFOURCC('V','P','6','F'),  
    MMP_FOURCC_VIDEO_VP6A     = MMPMAKEFOURCC('V','P','6','A'), 
    MMP_FOURCC_VIDEO_VP80     = MMPMAKEFOURCC('V','P','8','0'), 
    MMP_FOURCC_VIDEO_VP90     = MMPMAKEFOURCC('V','P','9','0'), 

    MMP_FOURCC_VIDEO_RV30     = MMPMAKEFOURCC('R','V','3','0'),  
    MMP_FOURCC_VIDEO_RV40     = MMPMAKEFOURCC('R','V','4','0'),  
    MMP_FOURCC_VIDEO_RV       = MMPMAKEFOURCC('R','V','X','0'),  

    MMP_FOURCC_VIDEO_SVQ1     = MMPMAKEFOURCC('S','V','Q','1'),   //Sorenson 1
    MMP_FOURCC_VIDEO_SVQ3     = MMPMAKEFOURCC('S','V','Q','3'),   //Sorenson 3
    MMP_FOURCC_VIDEO_FLV1     = MMPMAKEFOURCC('F','L','V','1'),   /* Sorenson Spark */

    MMP_FOURCC_VIDEO_THEORA   =   MMPMAKEFOURCC('T','H','E','O'),   
    MMP_FOURCC_VIDEO_MJPEG    = V4L2_PIX_FMT_MJPEG, // "MJPG"
    MMP_FOURCC_VIDEO_JPEG    =  V4L2_PIX_FMT_JPEG,  // "JPEG"
    
    MMP_FOURCC_VIDEO_MSMPEG4V1 =   MMPMAKEFOURCC('M','S','M','1'),   
    MMP_FOURCC_VIDEO_MSMPEG4V2 =   MMPMAKEFOURCC('M','S','M','2'),   
    MMP_FOURCC_VIDEO_MSMPEG4V3 =   MMPMAKEFOURCC('M','S','M','3'),   //Divx3

    MMP_FOURCC_VIDEO_INDEO2    = MMPMAKEFOURCC('I','N','D','2'),   /* Indeo2 */
    MMP_FOURCC_VIDEO_INDEO3    = MMPMAKEFOURCC('I','N','D','3'),   /* Indeo2 */
    MMP_FOURCC_VIDEO_INDEO4    = MMPMAKEFOURCC('I','N','D','4'),   /* Indeo2 */
    MMP_FOURCC_VIDEO_INDEO5    = MMPMAKEFOURCC('I','N','D','5'),   /* Indeo2 */

    MMP_FOURCC_VIDEO_TSCC      = MMPMAKEFOURCC('T','S','C','C'),   /* Indeo2 */
    MMP_FOURCC_VIDEO_FFMPEG  = MMPMAKEFOURCC('F','F','M','V'),  /* Ffmpeg Video */
    
    MMP_FOURCC_IMAGE_UNKNOWN = MMPMAKEFOURCC('I','M','G','?'),

    MMP_FOURCC_IMAGE_GREY      = V4L2_PIX_FMT_GREY, //v4l2_fourcc('G', 'R', 'E', 'Y') /*  8  Greyscale     */

    MMP_FOURCC_IMAGE_YUV444_P1   = V4L2_PIX_FMT_YUV444_P1, //MMPMAKEFOURCC('4','4','4','1'),  /* 24bit Y/U/V 444 Plane 1 Comp1*/
    MMP_FOURCC_IMAGE_YUV444M   = V4L2_PIX_FMT_YUV444M,     //MMPMAKEFOURCC('4','4','4','M'),  /* 24bit Y/U/V 444 Plane 1 Comp3*/
    //MMP_FOURCC_IMAGE_YCbCr422_P2 = MMPMAKEFOURCC('N','V','1','6'),  /* 16 bit Y/CbCr 4:2:2 Plane 2, V4L2_PIX_FMT_NV16 */
    //MMP_FOURCC_IMAGE_YCrCb422_P2 = MMPMAKEFOURCC('N','V','6','1'),  /* 16 bit Y/CrCb 4:2:2 Plane 2, V4L2_PIX_FMT_NV61 */
    

    MMP_FOURCC_IMAGE_UYVY       = V4L2_PIX_FMT_UYVY, // "UYVY"   YUV 4:2:2 packed, CbYCrY      
    MMP_FOURCC_IMAGE_VYUY       = V4L2_PIX_FMT_VYUY, //YUV 4:2:2 packed, CrYCbY
    MMP_FOURCC_IMAGE_YUYV       = V4L2_PIX_FMT_YUYV, //"YUYV" YUV 4:2:2 packed, YCbYCr
    MMP_FOURCC_IMAGE_YVYU       = V4L2_PIX_FMT_YVYU, //"YVYU"  YUV 4:2:2 packed, YCrYCb

    MMP_FOURCC_IMAGE_NV16       = V4L2_PIX_FMT_NV16, // "NV16" YUV 4:2:2 planar, Y/CbCr    16 bit Y/CbCr 4:2:2 Plane 2, V4L2_PIX_FMT_NV16
    MMP_FOURCC_IMAGE_NV16V      = V4L2_PIX_FMT_NV16V,// "N16V" YUV 2:2:4 planar, Y/CbCr    16 bit Y/CbCr 2:2:4 Plane 2, V4L2_PIX_FMT_NV16V (vertical)
    MMP_FOURCC_IMAGE_NV61       = V4L2_PIX_FMT_NV61, // "NV61" YUV 4:2:2 planar, Y/CrCb    16 bit Y/CrCb 4:2:2 Plane 2, V4L2_PIX_FMT_NV61

    MMP_FOURCC_IMAGE_YUV422P    = V4L2_PIX_FMT_YUV422P, //  "422P"   YUV 4:2:2 3-planar, Y/Cb/Cr
    MMP_FOURCC_IMAGE_YUV422PM   = V4L2_PIX_FMT_YUV422PM,//  "42PM" Custom  YUV 4:2:2 3-planar, Y/Cb/Cr non-config
    
    MMP_FOURCC_IMAGE_NV12       = V4L2_PIX_FMT_NV12,  //MMPMAKEFOURCC('N','V','1','2'),  /* 12  Y/CbCr 4:2:0  2Plane contigous -- one Y, one Cr + Cb interleaved */
    MMP_FOURCC_IMAGE_NV12M      = V4L2_PIX_FMT_NV12M, //MMPMAKEFOURCC('N','M','1','2'),  /* 12  Y/CbCr 4:2:0  2Plane non-contigous */
    MMP_FOURCC_IMAGE_NV21       = V4L2_PIX_FMT_NV21,  //MMPMAKEFOURCC('N','V','2','1'),  /* 12  Y/CrCb 4:2:0  2Plane contigous -- one Y, one Cr + Cb interleaved*/
    MMP_FOURCC_IMAGE_NV21M      = V4L2_PIX_FMT_NV21M,  //MMPMAKEFOURCC('N','M','2','1'),  /* 12  Y/CrCb 4:2:0  2Plane non-contigous */
    
    MMP_FOURCC_IMAGE_YUV420     = V4L2_PIX_FMT_YUV420, //MMPMAKEFOURCC('Y','U','1','2'),  /* V4L2_PIX_FMT_YUV420 12  YUV 4:2:0  3P contigous */
    MMP_FOURCC_IMAGE_YUV420M    = V4L2_PIX_FMT_YUV420M, //MMPMAKEFOURCC('Y','M','1','2'),  /* V4L2_PIX_FMT_YUV420M 12  YUV 4:2:0  3P non contiguous planes - Y, Cb, Cr */
    
    MMP_FOURCC_IMAGE_YVU420     = V4L2_PIX_FMT_YVU420, //MMPMAKEFOURCC('Y','V','1','2'),  /* V4L2_PIX_FMT_YVU420 12  YVU 4:2:0  3P contigous */
    MMP_FOURCC_IMAGE_YVU420M    = V4L2_PIX_FMT_YVU420M, //MMPMAKEFOURCC('Y','M','2','1'),  /* V4L2_PIX_FMT_YVU420M 12  YUV 4:2:0  3P non contiguous planes - Y, Cb, Cr */

    MMP_FOURCC_IMAGE_NV12MT    = V4L2_PIX_FMT_NV12MT, //  v4l2_fourcc('T', 'M', '1', '2') /* 12  Y/CbCr 4:2:0 64x32 macroblocks */
    
    MMP_FOURCC_IMAGE_YUV420M_META = MMPMAKEFOURCC('Y','M','1','M'),  
    
    MMP_FOURCC_IMAGE_RGB565 = V4L2_PIX_FMT_RGB565,   /* RGB565 */
	MMP_FOURCC_IMAGE_RGB888 = V4L2_PIX_FMT_RGB24,   /* 24bit RGB */
	MMP_FOURCC_IMAGE_BGR888 = V4L2_PIX_FMT_BGR24,   /* 24bit BGR */
    MMP_FOURCC_IMAGE_ARGB8888 = V4L2_PIX_FMT_RGB32,   /* XRGB-8888, 32 bpp */
    MMP_FOURCC_IMAGE_ABGR8888 = V4L2_PIX_FMT_BGR32,   /* XBGR-8888, 32 bpp */

    /**<Reserved android opaque colorformat. Tells the encoder that
     * the actual colorformat will be  relayed by the
     * Gralloc Buffers.
     * FIXME: In the process of reserving some enum values for
     * Android-specific OMX IL colorformats. Change this enum to
     * an acceptable range once that is done.
     * */
    MMP_FOURCC_IMAGE_ANDROID_OPAQUE  = MMPMAKEFOURCC('A','O','P','A'), //Android Gralloc depend,  Ref OMX_COLOR_FormatAndroidOpaque = 0x7F000789 in OMX_IVCommon.h
};

#if 0 
No.     Format                                  fourcc        planes      components     bpp     vdiv              desc
--------------------------------------------------------------------------------------------------------------------------
1.       V4L2_PIX_FMT_UYVY              "UYVY"         1              1                    16         1               YUV 4:2:2 packed, CbYCrY      
2.       V4L2_PIX_FMT_VYUY              "VYUY"         1              1                    16         1               YUV 4:2:2 packed, CrYCbY
3.       V4L2_PIX_FMT_YUYV              "YUYV"         1              1                    16         1               YUV 4:2:2 packed, YCbYCr
4.       V4L2_PIX_FMT_YVYU              "YVYU"         1              1                    16         1               YUV 4:2:2 packed, YCrYCb

5.       V4L2_PIX_FMT_NV16              "NV16"         1              2                    8,8        1,1            YUV 4:2:2 planar, Y/CbCr
6.       V4L2_PIX_FMT_NV61              "NV61"         1              2                    8,8        1,1            YUV 4:2:2 planar, Y/CrCb

7.       V4L2_PIX_FMT_YUV422P         "422P"         1              3                    8,4,4     1,1,1          YUV 4:2:2 3-planar, Y/Cb/Cr

8.       V4L2_PIX_FMT_NV12              "NV12"         1              2                    8,4        1,2            YUV 4:2:0 contiguous 2-planar, Y/CbCr
9.       V4L2_PIX_FMT_NV12M           "NM12"         2              2                    8,4        1,2            YUV 4:2:0 non-contiguous 2-planar, Y/CbCr
10.     V4L2_PIX_FMT_NV21              "NV21"         1              2                    8,4        1,2            YUV 4:2:0 contiguous 2-planar, Y/CrCb
11.     V4L2_PIX_FMT_NV21M           "NM21"         2              2                    8,4        1,2            YUV 4:2:0 non-contiguous 2-planar, Y/CrCb

12.     V4L2_PIX_FMT_YUV420M        "YM12"         3              3                    8,2,2     1,2,2          YUV 4:2:0 non-contiguous 3-planar, Y/Cb/Cr

13.     V4L2_PIX_FMT_RGB32             "RGB4"        1              1                    32          1               XRGB-8888, 32 bpp
14.     V4L2_PIX_FMT_BGR32             "BGR4"        1              1                    32          1               XBGR-8888, 32 bpp
15.     V4L2_PIX_FMT_RGB565           "RGBP"        1              1                    16          1              RGB565
#endif



#define MMP_VIDEO_FRAME_STRIDE_ALIGN  V4L2_VIDEO_FRAME_STRIDE_ALIGN
#define MMP_VIDEO_FRAME_HEIGHT_ALIGN  V4L2_VIDEO_FRAME_HEIGHT_ALIGN 
#define MMP_VIDEO_STREAM_SIZE_ALIGN   V4L2_VIDEO_STREAM_SIZE_ALIGN


#define MMP_IMAGE_FRAME_STRIDE_ALIGN  V4L2_IMAGE_FRAME_STRIDE_ALIGN
#define MMP_IMAGE_FRAME_HEIGHT_ALIGN  V4L2_IMAGE_FRAME_HEIGHT_ALIGN
#define MMP_IMAGE_STREAM_SIZE_ALIGN   V4L2_IMAGE_STREAM_SIZE_ALIGN 

#define MMP_IMAGE_ANDROID_OPAQUE_SIZE 256 

#endif
