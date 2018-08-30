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

#ifndef MMPDEFINE_OMXCOMP_H__
#define MMPDEFINE_OMXCOMP_H__

#include "OMX_Audio.h"
#include "OMX_Video.h"

/******************************************************************************
   Component Name 
******************************************************************************/
/* Audio Decoder */
#define OMX_COMPNAME_AUDIO_MP1_DECODER        "OMX.mme.mpeg-L1.decoder"
#define OMX_COMPNAME_AUDIO_MP2_DECODER        "OMX.mme.mpeg-L2.decoder"
#define OMX_COMPNAME_AUDIO_MP3_DECODER        "OMX.mme.mp3.decoder"

#define OMX_COMPNAME_AUDIO_WMAV2_DECODER      "OMX.mme.wmav2.decoder"
#define OMX_COMPNAME_AUDIO_AC3_DECODER        "OMX.mme.ac3.decoder"
#define OMX_COMPNAME_AUDIO_AAC_DECODER        "OMX.mme.aac.decoder"
#define OMX_COMPNAME_AUDIO_FLAC_DECODER       "OMX.mme.flac.decoder"
#define OMX_COMPNAME_AUDIO_FLAC_MME_DECODER   "OMX.mme.flac-mme.decoder"
#define OMX_COMPNAME_AUDIO_ADPCM_MS_DECODER   "OMX.mme.adpcm-ms.decoder"
#define OMX_COMPNAME_AUDIO_PCM_DECODER        "OMX.mme.pcm.decoder"

#define OMX_COMPNAME_AUDIO_FFMPEG_DECODER     "OMX.mme.ffmpeg_a.decoder"

/* Video Decoder */
#define OMX_COMPNAME_VIDEO_H264_DECODER       "OMX.mme.h264.decoder"
#define OMX_COMPNAME_VIDEO_MPEG4_DECODER      "OMX.mme.mpeg4.decoder"
#define OMX_COMPNAME_VIDEO_MSMPEG4V3_DECODER  "OMX.mme.msmpeg4v3.decoder"
#define OMX_COMPNAME_VIDEO_MPEG2_DECODER      "OMX.mme.mpeg2.decoder"
#define OMX_COMPNAME_VIDEO_WMV3_DECODER       "OMX.mme.wmv3.decoder"
#define OMX_COMPNAME_VIDEO_WVC1_DECODER       "OMX.mme.wvc1.decoder"
#define OMX_COMPNAME_VIDEO_RV8_DECODER        "OMX.mme.rv8.decoder"
#define OMX_COMPNAME_VIDEO_RV9_DECODER        "OMX.mme.rv9.decoder"
#define OMX_COMPNAME_VIDEO_RV10_DECODER       "OMX.mme.rv10.decoder"
#define OMX_COMPNAME_VIDEO_RV_DECODER         "OMX.mme.rv.decoder"
#define OMX_COMPNAME_VIDEO_VP8_DECODER        "OMX.mme.vp8.decoder"
#define OMX_COMPNAME_VIDEO_AVS_DECODER        "OMX.mme.avs.decoder"
#define OMX_COMPNAME_VIDEO_H263_DECODER       "OMX.mme.h263.decoder"
#define OMX_COMPNAME_VIDEO_FLV1_DECODER       "OMX.mme.flv1.decoder" /* sorenson spark*/
#define OMX_COMPNAME_VIDEO_THEORA_DECODER     "OMX.mme.theora.decoder" 
#define OMX_COMPNAME_VIDEO_FFMPEG_DECODER     "OMX.mme.ffmpeg_v.decoder"

//#define OMX_COMPNAME_VIDEO_MSMPEG4V1_DECODER  "OMX.mme.msmpeg4v1.decoder"
//#define OMX_COMPNAME_VIDEO_MSMPEG4V2_DECODER  "OMX.mme.msmpeg4v2.decoder"
//#define OMX_COMPNAME_VIDEO_MSMPEG4V3_DECODER  "OMX.mme.msmpeg4v3.decoder"
//#define OMX_COMPNAME_VIDEO_WMV1_DECODER       "OMX.mme.wmv1.decoder"
//#define OMX_COMPNAME_VIDEO_WMV2_DECODER       "OMX.mme.wmv2.decoder"
//#define OMX_COMPNAME_VIDEO_WMV3_DECODER       "OMX.mme.wmv3.decoder"
//#define OMX_COMPNAME_VIDEO_VP6_DECODER        "OMX.mme.vp6.decoder"
//#define OMX_COMPNAME_VIDEO_VP6F_DECODER       "OMX.mme.vp6f.decoder"
//#define OMX_COMPNAME_VIDEO_VP6A_DECODER       "OMX.mme.vp6a.decoder"
//#define OMX_COMPNAME_VIDEO_SVQ1_DECODER       "OMX.mme.svq1.decoder"   /* sorenson 1 */
//#define OMX_COMPNAME_VIDEO_SVQ3_DECODER       "OMX.mme.svq3.decoder"   /* sorenson 3 */
//#define OMX_COMPNAME_VIDEO_MJPEG_DECODER      "OMX.mme.mjpeg.decoder" 

/* Video Encoder */
#define OMX_COMPNAME_VIDEO_MPEG4_ENCODER      "OMX.mme.mpeg4.encoder"
#define OMX_COMPNAME_VIDEO_H264_ENCODER       "OMX.mme.h264.encoder"
#define OMX_COMPNAME_VIDEO_H263_ENCODER       "OMX.mme.h263.encoder"

/******************************************************************************
   Mime Name
******************************************************************************/
/* Mime Type : Audio */
#define OMX_MIMETYPE_AUDIO_MP3      "audio/mpeg"
#define OMX_MIMETYPE_AUDIO_AAC      "audio/mp4a-latm"
#define OMX_MIMETYPE_AUDIO_AAC_ADTS "audio/aac-adts"
#define OMX_MIMETYPE_AUDIO_VORBIS   "audio/vorbis"
#define OMX_MIMETYPE_AUDIO_MP2      "audio/mpeg-L2"  		
#define OMX_MIMETYPE_AUDIO_MP1      "audio/mpeg-L1"  		
#define OMX_MIMETYPE_AUDIO_QCELP    "audio/qcelp"
#define OMX_MIMETYPE_AUDIO_OPUS     "audio/opus"
#define OMX_MIMETYPE_AUDIO_G711_ALAW "audio/g711-alaw"
#define OMX_MIMETYPE_AUDIO_G711_MLAW "audio/g711-mlaw"
#define OMX_MIMETYPE_AUDIO_MSGSM     "audio/gsm"
#define OMX_MIMETYPE_AUDIO_WMAV2     "audio/x-ms-wma"
#define OMX_MIMETYPE_AUDIO_AC3       "audio/ac3"  		
#define OMX_MIMETYPE_AUDIO_EAC3      "audio/eac3"  		
//#define OMX_MIMETYPE_AUDIO_AAC_MAIN "audio/aac-main"  		
#define OMX_MIMETYPE_AUDIO_FLAC      "audio/flac"  		
//#define OMX_MIMETYPE_AUDIO_ADPCM_MS "audio/adpcm-ms"  		
#define OMX_MIMETYPE_AUDIO_RAW       "audio/raw"
#define OMX_MIMETYPE_AUDIO_PCM       OMX_MIMETYPE_AUDIO_RAW 
#define OMX_MIMETYPE_AUDIO_AMR_NB    "audio/3gpp"
#define OMX_MIMETYPE_AUDIO_AMR_WB    "audio/amr-wb"
#define OMX_MIMETYPE_AUDIO_FLAC      "audio/flac"
#define OMX_MIMETYPE_AUDIO_FLAC_MME  "audio/flac-mme"
#define OMX_MIMETYPE_AUDIO_FFMPEG    "audio/ffmpeg"
#define OMX_MIMETYPE_AUDIO_DISABLE   "audio/disable"

/* Mime Type : Video */
#define OMX_MIMETYPE_VIDEO_H264      "video/avc" 
#define OMX_MIMETYPE_VIDEO_HEVC      "video/hevc" 
#define OMX_MIMETYPE_VIDEO_H264_MVC  "video/mvc"
#define OMX_MIMETYPE_VIDEO_HEVC      "video/hevc"
#define OMX_MIMETYPE_VIDEO_MPEG4     "video/mp4v-es"
#define OMX_MIMETYPE_VIDEO_MSMPEG4V3 "video/mp43"
#define OMX_MIMETYPE_VIDEO_MPEG2     "video/mpeg2"
#define OMX_MIMETYPE_VIDEO_WMV3      "video/x-ms-wmv"    /* WMV3 */
#define OMX_MIMETYPE_VIDEO_RV        "video/vnd.rn-realvideo"   /* RV 8/9/10 */
#define OMX_MIMETYPE_VIDEO_VP8       "video/x-vnd.on2.vp8"
#define OMX_MIMETYPE_VIDEO_VP9       "video/x-vnd.on2.vp9"
#define OMX_MIMETYPE_VIDEO_AVS       "video/avs"
#define OMX_MIMETYPE_VIDEO_H263      "video/3gpp"
#define OMX_MIMETYPE_VIDEO_FLV1      "video/flv1"  /* sorenson spark */
#define OMX_MIMETYPE_VIDEO_THEORA    "video/theora"
#define OMX_MIMETYPE_VIDEO_FFMPEG    "video/ffmpeg"

#define OMX_MIMETYPE_VIDEO_RAW       "video/raw"

//#define OMX_MIMETYPE_VIDEO_MJPEG     "video/mjpeg"
//#define OMX_MIMETYPE_VIDEO_SVQ1      "video/svq1" /* Sorenson 1 */
//#define OMX_MIMETYPE_VIDEO_SVQ3      "video/svq3" /* Sorenson 3 */
//#define OMX_MIMETYPE_VIDEO_WMV1      "video/wmv1"
//#define OMX_MIMETYPE_VIDEO_WMV2      "video/wmv2"
//#define OMX_MIMETYPE_VIDEO_WMV3      "video/wmv3"
//#define OMX_MIMETYPE_VIDEO_VP6       "video/vp6"
//#define OMX_MIMETYPE_VIDEO_VP6F      "video/vp6f"
//#define OMX_MIMETYPE_VIDEO_VP6A      "video/vp6a"

//#define OMX_MIMETYPE_VIDEO_MSMPEG4V1 "video/msmpeg4v1"
//#define OMX_MIMETYPE_VIDEO_MSMPEG4V2 "video/msmpeg4v2"
//#define OMX_MIMETYPE_VIDEO_MSMPEG4V3 "video/msmpeg4v3"

/* Mime Type : Image */
#define OMX_MIMETYPE_IMAGE_JPEG      "image/jpeg"


/******************************************************************************
   ROLE Name
******************************************************************************/

/* android default role */

#define OMX_ROLENAME_AUDIO_DECODER_MP3     "audio_decoder.mp3"
#define OMX_ROLENAME_AUDIO_DECODER_MP2     "audio_decoder.mp2"
#define OMX_ROLENAME_AUDIO_DECODER_MP1     "audio_decoder.mp1"
#define OMX_ROLENAME_AUDIO_DECODER_AMR_NB  "audio_decoder.amrnb"
#define OMX_ROLENAME_AUDIO_DECODER_AMR_WB  "audio_decoder.amrwb"
#define OMX_ROLENAME_AUDIO_DECODER_AAC     "audio_decoder.aac"
#define OMX_ROLENAME_AUDIO_DECODER_VORBIS  "audio_decoder.vorbis"
#define OMX_ROLENAME_AUDIO_DECODER_OPUS    "audio_decoder.opus"
#define OMX_ROLENAME_AUDIO_DECODER_G711_MLAW   "audio_decoder.g711mlaw"
#define OMX_ROLENAME_AUDIO_DECODER_G711_ALAW   "audio_decoder.g711alaw"
#define OMX_ROLENAME_AUDIO_DECODER_RAW      "audio_decoder.raw"
#define OMX_ROLENAME_AUDIO_DECODER_FLAC     "audio_decoder.flac"
#define OMX_ROLENAME_AUDIO_DECODER_MSGSM    "audio_decoder.gsm"
#define OMX_ROLENAME_AUDIO_DECODER_AC3      "audio_decoder.ac3"

#define OMX_ROLENAME_AUDIO_ENCODER_MP3 "audio_encoder.mp3"
#define OMX_ROLENAME_AUDIO_ENCODER_MP2 "audio_encoder.mp2"
#define OMX_ROLENAME_AUDIO_ENCODER_MP1 "audio_encoder.mp1"
#define OMX_ROLENAME_AUDIO_ENCODER_AMR_NB "audio_encoder.amrnb"
#define OMX_ROLENAME_AUDIO_ENCODER_AMR_WB "audio_encoder.amrwb"
#define OMX_ROLENAME_AUDIO_ENCODER_AAC     "audio_encoder.aac"
#define OMX_ROLENAME_AUDIO_ENCODER_VORBIS  "audio_encoder.vorbis"
#define OMX_ROLENAME_AUDIO_ENCODER_OPUS    "audio_encoder.opus"
#define OMX_ROLENAME_AUDIO_ENCODER_G711_MLAW   "audio_encoder.g711mlaw"
#define OMX_ROLENAME_AUDIO_ENCODER_G711_ALAW   "audio_encoder.g711alaw"
#define OMX_ROLENAME_AUDIO_ENCODER_RAW      "audio_encoder.raw"
#define OMX_ROLENAME_AUDIO_ENCODER_FLAC     "audio_encoder.flac"
#define OMX_ROLENAME_AUDIO_ENCODER_MSGSM    "audio_encoder.gsm"
#define OMX_ROLENAME_AUDIO_ENCODER_AC3      "audio_encoder.ac3"


#define OMX_ROLENAME_VIDEO_DECODER_H264     "video_decoder.avc"
#define OMX_ROLENAME_VIDEO_DECODER_HEVC     "video_decoder.hevc"
#define OMX_ROLENAME_VIDEO_DECODER_MPEG4    "video_decoder.mpeg4"
#define OMX_ROLENAME_VIDEO_DECODER_MSMPEG4V3 "video_decoder.msmpeg4v3" /* DIV3*/
#define OMX_ROLENAME_VIDEO_DECODER_H263     "video_decoder.h263"
#define OMX_ROLENAME_VIDEO_DECODER_VP8      "video_decoder.vp8"
#define OMX_ROLENAME_VIDEO_DECODER_VP9      "video_decoder.vp9"
#define OMX_ROLENAME_VIDEO_DECODER_MPEG2    "video_decoder.mpeg2"

#define OMX_ROLENAME_VIDEO_ENCODER_H264     "video_encoder.avc"
#define OMX_ROLENAME_VIDEO_ENCODER_HEVC     "video_encoder.hevc"
#define OMX_ROLENAME_VIDEO_ENCODER_MPEG4    "video_encoder.mpeg4"
#define OMX_ROLENAME_VIDEO_ENCODER_H263     "video_encoder.h263"
#define OMX_ROLENAME_VIDEO_ENCODER_VP8      "video_encoder.vp8"
#define OMX_ROLENAME_VIDEO_ENCODER_VP9      "video_encoder.vp9"
#define OMX_ROLENAME_VIDEO_ENCODER_MPEG2    "video_encoder.mpeg2"

/* mme extension role */
#define OMX_ROLENAME_AUDIO_DECODER_WMA2    "audio_decoder.wma2" 
#define OMX_ROLENAME_AUDIO_DECODER_FFMPEG  "audio_decoder.ffmpeg" 

#define OMX_ROLENAME_VIDEO_DECODER_WMV3    "video_decoder.wmv3"
#define OMX_ROLENAME_VIDEO_DECODER_RV9     "video_decoder.rv9"
#define OMX_ROLENAME_VIDEO_DECODER_FLV1    "video_decoder.flv1" /* sorenson spark */
#define OMX_ROLENAME_VIDEO_DECODER_THEORA  "video_decoder.theora" 
#define OMX_ROLENAME_VIDEO_DECODER_FFMPEG  "video_decoder.ffmpeg" 

/*
Ref. Android5.1  libstagefright  OMXCodec.cpp 

static const MimeToRole kMimeToRole[] = {
        { MEDIA_MIMETYPE_AUDIO_MPEG,
            "audio_decoder.mp3", "audio_encoder.mp3" },
        { MEDIA_MIMETYPE_AUDIO_MPEG_LAYER_I,
            "audio_decoder.mp1", "audio_encoder.mp1" },
        { MEDIA_MIMETYPE_AUDIO_MPEG_LAYER_II,
            "audio_decoder.mp2", "audio_encoder.mp2" },
        { MEDIA_MIMETYPE_AUDIO_AMR_NB,
            "audio_decoder.amrnb", "audio_encoder.amrnb" },
        { MEDIA_MIMETYPE_AUDIO_AMR_WB,
            "audio_decoder.amrwb", "audio_encoder.amrwb" },
        { MEDIA_MIMETYPE_AUDIO_AAC,
            "audio_decoder.aac", "audio_encoder.aac" },
        { MEDIA_MIMETYPE_AUDIO_VORBIS,
            "audio_decoder.vorbis", "audio_encoder.vorbis" },
        { MEDIA_MIMETYPE_AUDIO_OPUS,
            "audio_decoder.opus", "audio_encoder.opus" },
        { MEDIA_MIMETYPE_AUDIO_G711_MLAW,
            "audio_decoder.g711mlaw", "audio_encoder.g711mlaw" },
        { MEDIA_MIMETYPE_AUDIO_G711_ALAW,
            "audio_decoder.g711alaw", "audio_encoder.g711alaw" },
        { MEDIA_MIMETYPE_VIDEO_AVC,
            "video_decoder.avc", "video_encoder.avc" },
        { MEDIA_MIMETYPE_VIDEO_HEVC,
            "video_decoder.hevc", "video_encoder.hevc" },
        { MEDIA_MIMETYPE_VIDEO_MPEG4,
            "video_decoder.mpeg4", "video_encoder.mpeg4" },
        { MEDIA_MIMETYPE_VIDEO_H263,
            "video_decoder.h263", "video_encoder.h263" },
        { MEDIA_MIMETYPE_VIDEO_VP8,
            "video_decoder.vp8", "video_encoder.vp8" },
        { MEDIA_MIMETYPE_VIDEO_VP9,
            "video_decoder.vp9", "video_encoder.vp9" },
        { MEDIA_MIMETYPE_AUDIO_RAW,
            "audio_decoder.raw", "audio_encoder.raw" },
        { MEDIA_MIMETYPE_AUDIO_FLAC,
            "audio_decoder.flac", "audio_encoder.flac" },
        { MEDIA_MIMETYPE_AUDIO_MSGSM,
            "audio_decoder.gsm", "audio_encoder.gsm" },
        { MEDIA_MIMETYPE_VIDEO_MPEG2,
            "video_decoder.mpeg2", "video_encoder.mpeg2" },
        { MEDIA_MIMETYPE_AUDIO_AC3,
            "audio_decoder.ac3", "audio_encoder.ac3" },
    };
*/
/******************************************************************************
   Android Extension Index 
******************************************************************************/

#define OMX_INDEX_PARAM_ENABLE_ANB "OMX.google.android.index.enableAndroidNativeBuffers"
#define OMX_INDEX_PARAM_GET_ANB_USAGE "OMX.google.android.index.getAndroidNativeBufferUsage"
#define OMX_INDEX_PARAM_USE_ANB "OMX.google.android.index.useAndroidNativeBuffer"
#define OMX_INDEX_PARAM_ENABLE_THUMBNAILMODE "OMX.anapass.android.index.setThumbnailMode"
#define OMX_INDEX_PARAM_STORE_METADATA_IN_BUFFERS "OMX.google.android.index.storeMetaDataInBuffers"
#define OMX_INDEX_PARAM_DESCRIBE_COLOR_FORMAT "OMX.google.android.index.describeColorFormat"

typedef enum _OMX_VENDOR_INDEXTYPE
{
    //OMX_IndexVendorThumbnailMode        = 0x7F000001,
    //OMX_IndexConfigVideoIntraPeriod     = 0x7F000002,

    /* for Android Native Window */
    OMX_IndexParamEnableAndroidBuffers    = 0x7F000011,
    OMX_IndexParamGetAndroidNativeBufferUsage  = 0x7F000012,
    OMX_IndexParamUseAndroidNativeBuffer  = 0x7F000013,
    /* for Android Store Metadata Inbuffer */
    OMX_IndexParamStoreMetaDataInBuffers     = 0x7F000014,

    OMX_IndexParamEnableThumbnailMode = 0x7F000015,
    OMX_IndexParamDescribeColorFormat = 0x7F000016,

    /* for Android PV OpenCore*/
    //OMX_COMPONENT_CAPABILITY_TYPE_INDEX = 0xFF7A347
} OMX_VENDOR_INDEXTYPE;


/******************************************************************************
   MME Extention Type for OMX 
******************************************************************************/

typedef enum OMX_VENDOR_VIDEO_CODINGTYPE {

#if 0
    OMX_VIDEO_CodingUnused,     /**< Value when coding is N/A */
    OMX_VIDEO_CodingAutoDetect, /**< Autodetection of coding type */
    OMX_VIDEO_CodingMPEG2,      /**< AKA: H.262 */
    OMX_VIDEO_CodingH263,       /**< H.263 */
    OMX_VIDEO_CodingMPEG4,      /**< MPEG-4 */
    OMX_VIDEO_CodingWMV,        /**< all versions of Windows Media Video */
    OMX_VIDEO_CodingRV,         /**< all versions of Real Video */
    OMX_VIDEO_CodingAVC,        /**< H.264/AVC */
    OMX_VIDEO_CodingMJPEG,      /**< Motion JPEG */
    OMX_VIDEO_CodingVPX,        /**< Google VPX, formerly known as On2 VP8 */
    OMX_VIDEO_CodingKhronosExtensions = 0x6F000000, /**< Reserved region for introducing Khronos Standard Extensions */ 
    OMX_VIDEO_CodingVendorStartUnused = 0x7F000000, /**< Reserved region for introducing Vendor Extensions */
    OMX_VIDEO_CodingMax = 0x7FFFFFFF
#endif

    //OMX_VIDEO_CodingAnapass = 0x7F000001,
    OMX_VIDEO_CodingFLV1 = (OMX_VIDEO_CodingVendorStartUnused+1), /* Sorenson Spark */
    OMX_VIDEO_CodingTHEORA,
    OMX_VIDEO_CodingMSMPEG4V3,
    OMX_VIDEO_CodingFFMpeg,

} OMX_VENDOR_VIDEO_CODINGTYPE;

typedef enum OMX_VENDOR_AUDIO_CODINGTYPE {

     //OMX_AUDIO_CodingVendorStartUnused = 0x7F000000, /**< Reserved region for introducing Vendor Extensions */
    OMX_AUDIO_CodingAC3 = (OMX_AUDIO_CodingVendorStartUnused+1),
    OMX_AUDIO_CodingFFMpeg,
    OMX_AUDIO_CodingMP2,
    OMX_AUDIO_CodingMP1

} OMX_VENDOR_AUDIO_CODINGTYPE;

typedef enum OMX_VENDOR_COLOR_FORMATTYPE {

    /*Samsung Color Format */
    OMX_Samsung_COLOR_FormatNV12TPhysicalAddress = 0x7F000001, /**< Reserved region for introducing Vendor Extensions */
    OMX_Samsung_COLOR_FormatNV12LPhysicalAddress = 0x7F000002,
    OMX_Samsung_COLOR_FormatNV12LVirtualAddress = 0x7F000003,
    //OMX_Samsung_COLOR_FormatNV12Tiled            = 0x7FC00002,  /* 0x7FC00002 */

#if 0 //def S3D_SUPPORT
    OMX_SEC_COLOR_FormatNV12Tiled_SBS_LR     = 0x7FC00003,  /* 0x7FC00003 */
    OMX_SEC_COLOR_FormatNV12Tiled_SBS_RL     = 0x7FC00004,  /* 0x7FC00004 */
    OMX_SEC_COLOR_FormatNV12Tiled_TB_LR     = 0x7FC00005,  /* 0x7FC00005 */
    OMX_SEC_COLOR_FormatNV12Tiled_TB_RL   = 0x7FC00006,  /* 0x7FC00006 */
    OMX_SEC_COLOR_FormatYUV420SemiPlanar_SBS_LR     = 0x7FC00007,  /* 0x7FC00007 */
    OMX_SEC_COLOR_FormatYUV420SemiPlanar_SBS_RL     = 0x7FC00008,  /* 0x7FC00008 */
    OMX_SEC_COLOR_FormatYUV420SemiPlanar_TB_LR     = 0x7FC00009,  /* 0x7FC00009 */
    OMX_SEC_COLOR_FormatYUV420SemiPlanar_TB_RL   = 0x7FC0000A,  /* 0x7FC0000A */
    OMX_SEC_COLOR_FormatYUV420Planar_SBS_LR     = 0x7FC0000B,  /* 0x7FC0000B */
    OMX_SEC_COLOR_FormatYUV420Planar_SBS_RL     = 0x7FC0000C,  /* 0x7FC0000C */
    OMX_SEC_COLOR_FormatYUV420Planar_TB_LR     = 0x7FC0000D,  /* 0x7FC0000D */
    OMX_SEC_COLOR_FormatYUV420Planar_TB_RL   = 0x7FC0000E,  /* 0x7FC0000E */
#endif

    //OMX_SEC_COLOR_FormatNV21LPhysicalAddress = 0x7F000010,
    //OMX_SEC_COLOR_FormatNV21Linear           = 0x7F000011,

    //* for Android Native Window */
    //OMX_SEC_COLOR_FormatANBYUV420SemiPlanar  = 0x100,
    /* for Android SurfaceMediaSource*/
    //OMX_COLOR_FormatAndroidOpaque            = 0x7F000789


}SEC_OMX_COLOR_FORMATTYPE;


#endif