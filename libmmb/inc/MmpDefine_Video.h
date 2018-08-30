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

#ifndef MMPDEFINE_VIDEO_H__
#define MMPDEFINE_VIDEO_H__

#if (MMP_OS == MMP_OS_WIN32)
#include <pshpack2.h>
#endif

typedef struct tagMMPBITMAPFILEHEADER {
        MMP_U16    bfType;
        MMP_U32   bfSize;
        MMP_U16    bfReserved1;
        MMP_U16    bfReserved2;
        MMP_U32   bfOffBits;

#if (MMP_OS == MMP_OS_WIN32)
}MMPBITMAPFILEHEADER;
#include <poppack.h>
#elif (MMP_OS == MMP_OS_LINUX)
} __attribute__((packed)) MMPBITMAPFILEHEADER;
#else
#error "ERROR : need attribute packed of OS "
#endif



typedef struct MMPBITMAPINFOHEADER
{
        MMP_U32      biSize;
        long       biWidth;
        long       biHeight;
        unsigned short       biPlanes;
        unsigned short       biBitCount;
        MMP_U32      biCompression;
        MMP_U32      biSizeImage;
        long       biXPelsPerMeter;
        long       biYPelsPerMeter;
        MMP_U32      biClrUsed;
        MMP_U32      biClrImportant;
}MMPBITMAPINFOHEADER;

#define MMP_BI_RGB        0L
#define MMP_BI_RLE8       1L
#define MMP_BI_RLE4       2L
#define MMP_BI_BITFIELDS  3L
#define MMP_BI_JPEG       4L
#define MMP_BI_PNG        5L


struct mmp_video_hw_codec_instance_info {

    MMP_S32 instance_index;
    MMP_S32 instance_max_count;

    MMP_S32 is_use;
    MMP_S32 is_decoder;

    MMPBITMAPINFOHEADER bih;
};

typedef enum MMP_VIDEO_CODINGTYPE {
    MMP_VIDEO_CodingUnused,     /**< Value when coding is N/A */
    MMP_VIDEO_CodingAutoDetect, /**< Autodetection of coding type */
    MMP_VIDEO_CodingMPEG2,      /**< AKA: H.262 */
    MMP_VIDEO_CodingH263,       /**< H.263 */
    MMP_VIDEO_CodingMPEG4,      /**< MPEG-4 */
    MMP_VIDEO_CodingWMV,        /**< all versions of Windows Media Video */
    MMP_VIDEO_CodingRV,         /**< all versions of Real Video */
    MMP_VIDEO_CodingAVC,        /**< H.264/AVC */
    MMP_VIDEO_CodingMJPEG,      /**< Motion JPEG */
    MMP_VIDEO_CodingVPX,        /**< Google VPX, formerly known as On2 VP8 */
    MMP_VIDEO_CodingKhronosExtensions = 0x6F000000, /**< Reserved region for introducing Khronos Standard Extensions */ 
    MMP_VIDEO_CodingVendorStartUnused = 0x7F000000, /**< Reserved region for introducing Vendor Extensions */
    MMP_VIDEO_CodingMax = 0x7FFFFFFF
} MMP_VIDEO_CODINGTYPE;


#define MMP_IMAGE_MAX_PLANE_COUNT 3

#define MMP_YUV420_PLAINE_INDEX_Y 0 /* if YUV420, Y index */  
#define MMP_YUV420_PLAINE_INDEX_U 1 /* if YUV420, U index */  
#define MMP_YUV420_PLAINE_INDEX_V 2 /* if YUV420, V index */  

#define MMP_NV12_PLAINE_INDEX_Y    0 /* if Y/CbCr NV12,, Y index */
#define MMP_NV12_PLAINE_INDEX_CbCr 1 /* if Y/CbCr NV12,, cbcr index */


#define MMP_YV12_FRAME_SIZE(w,h)  ((((w + 15) & (-16))  * ((h + 15) & (-16)) * 3) / 2)


enum MMP_ROTATE {
    MMP_ROTATE_0   = 0,
    MMP_ROTATE_90  = 90,
    MMP_ROTATE_180 = 180,
    MMP_ROTATE_270 = 270,
    MMP_ROTATE_HFLIP = 360,
    MMP_ROTATE_VFLIP = 450,
    MMP_ROTATE_RAND = 540,
    MMP_ROTATE_OFF  = 730
};



/** 
 * AVC profile types, each profile indicates support for various 
 * performance bounds and different annexes.
 */
typedef enum MMP_VIDEO_AVCPROFILETYPE {
    MMP_VIDEO_AVCProfileBaseline = 0x01,   /**< Baseline profile */
    MMP_VIDEO_AVCProfileMain     = 0x02,   /**< Main profile */
    MMP_VIDEO_AVCProfileExtended = 0x04,   /**< Extended profile */
    MMP_VIDEO_AVCProfileHigh     = 0x08,   /**< High profile */
    MMP_VIDEO_AVCProfileHigh10   = 0x10,   /**< High 10 profile */
    MMP_VIDEO_AVCProfileHigh422  = 0x20,   /**< High 4:2:2 profile */
    MMP_VIDEO_AVCProfileHigh444  = 0x40,   /**< High 4:4:4 profile */
    MMP_VIDEO_AVCProfileKhronosExtensions = 0x6F000000, /**< Reserved region for introducing Khronos Standard Extensions */ 
    MMP_VIDEO_AVCProfileVendorStartUnused = 0x7F000000, /**< Reserved region for introducing Vendor Extensions */
    MMP_VIDEO_AVCProfileMax      = 0x7FFFFFFF  
} MMP_VIDEO_AVCPROFILETYPE;

/** 
 * AVC level types, each level indicates support for various frame sizes, 
 * bit rates, decoder frame rates.  No need 
 */
typedef enum MMP_VIDEO_AVCLEVELTYPE {
    MMP_VIDEO_AVCLevel1   = 0x01,     /**< Level 1 */
    MMP_VIDEO_AVCLevel1b  = 0x02,     /**< Level 1b */
    MMP_VIDEO_AVCLevel11  = 0x04,     /**< Level 1.1 */
    MMP_VIDEO_AVCLevel12  = 0x08,     /**< Level 1.2 */
    MMP_VIDEO_AVCLevel13  = 0x10,     /**< Level 1.3 */
    MMP_VIDEO_AVCLevel2   = 0x20,     /**< Level 2 */
    MMP_VIDEO_AVCLevel21  = 0x40,     /**< Level 2.1 */
    MMP_VIDEO_AVCLevel22  = 0x80,     /**< Level 2.2 */
    MMP_VIDEO_AVCLevel3   = 0x100,    /**< Level 3 */
    MMP_VIDEO_AVCLevel31  = 0x200,    /**< Level 3.1 */
    MMP_VIDEO_AVCLevel32  = 0x400,    /**< Level 3.2 */
    MMP_VIDEO_AVCLevel4   = 0x800,    /**< Level 4 */
    MMP_VIDEO_AVCLevel41  = 0x1000,   /**< Level 4.1 */
    MMP_VIDEO_AVCLevel42  = 0x2000,   /**< Level 4.2 */
    MMP_VIDEO_AVCLevel5   = 0x4000,   /**< Level 5 */
    MMP_VIDEO_AVCLevel51  = 0x8000,   /**< Level 5.1 */
    MMP_VIDEO_AVCLevelKhronosExtensions = 0x6F000000, /**< Reserved region for introducing Khronos Standard Extensions */ 
    MMP_VIDEO_AVCLevelVendorStartUnused = 0x7F000000, /**< Reserved region for introducing Vendor Extensions */
    MMP_VIDEO_AVCLevelMax = 0x7FFFFFFF  
} MMP_VIDEO_AVCLEVELTYPE;

/** 
 * AVC loop filter modes 
 *
 * MMP_VIDEO_AVCLoopFilterEnable               : Enable
 * MMP_VIDEO_AVCLoopFilterDisable              : Disable
 * MMP_VIDEO_AVCLoopFilterDisableSliceBoundary : Disabled on slice boundaries
 */
typedef enum MMP_VIDEO_AVCLOOPFILTERTYPE {
    MMP_VIDEO_AVCLoopFilterEnable = 0,
    MMP_VIDEO_AVCLoopFilterDisable,
    MMP_VIDEO_AVCLoopFilterDisableSliceBoundary,
    MMP_VIDEO_AVCLoopFilterKhronosExtensions = 0x6F000000, /**< Reserved region for introducing Khronos Standard Extensions */ 
    MMP_VIDEO_AVCLoopFilterVendorStartUnused = 0x7F000000, /**< Reserved region for introducing Vendor Extensions */
    MMP_VIDEO_AVCLoopFilterMax = 0x7FFFFFFF
} MMP_VIDEO_AVCLOOPFILTERTYPE;


typedef struct MMP_VIDEO_PARAM_H264TYPE {
    MMP_U32 nSliceHeaderSpacing;  
    MMP_U32 nPFrames;     
    MMP_U32 nBFrames;     
    MMP_BOOL bUseHadamard;
    MMP_U32 nRefFrames;  
	MMP_U32 nRefIdx10ActiveMinus1;
	MMP_U32 nRefIdx11ActiveMinus1;
    MMP_BOOL bEnableUEP;  
    MMP_BOOL bEnableFMO;  
    MMP_BOOL bEnableASO;  
    MMP_BOOL bEnableRS;   
    MMP_VIDEO_AVCPROFILETYPE eProfile;
	MMP_VIDEO_AVCLEVELTYPE eLevel; 
    MMP_U32 nAllowedPictureTypes;  
	MMP_BOOL bFrameMBsOnly;        									
    MMP_BOOL bMBAFF;               
    MMP_BOOL bEntropyCodingCABAC;  
    MMP_BOOL bWeightedPPrediction; 
    MMP_U32 nWeightedBipredicitonMode; 
    MMP_BOOL bconstIpred ;
    MMP_BOOL bDirect8x8Inference;  
	MMP_BOOL bDirectSpatialTemporal;
	MMP_U32 nCabacInitIdc;
	MMP_VIDEO_AVCLOOPFILTERTYPE eLoopFilterMode;
} MMP_VIDEO_PARAM_H264TYPE;



/** 
 * H.263 profile types, each profile indicates support for various 
 * performance bounds and different annexes.
 *
 * ENUMS:
 *  Baseline           : Baseline Profile: H.263 (V1), no optional modes                                                    
 *  H320 Coding        : H.320 Coding Efficiency Backward Compatibility 
 *                       Profile: H.263+ (V2), includes annexes I, J, L.4
 *                       and T
 *  BackwardCompatible : Backward Compatibility Profile: H.263 (V1), 
 *                       includes annex F                                    
 *  ISWV2              : Interactive Streaming Wireless Profile: H.263+ 
 *                       (V2), includes annexes I, J, K and T                 
 *  ISWV3              : Interactive Streaming Wireless Profile: H.263++  
 *                       (V3), includes profile 3 and annexes V and W.6.3.8   
 *  HighCompression    : Conversational High Compression Profile: H.263++  
 *                       (V3), includes profiles 1 & 2 and annexes D and U   
 *  Internet           : Conversational Internet Profile: H.263++ (V3),  
 *                       includes profile 5 and annex K                       
 *  Interlace          : Conversational Interlace Profile: H.263++ (V3),  
 *                       includes profile 5 and annex W.6.3.11               
 *  HighLatency        : High Latency Profile: H.263++ (V3), includes  
 *                       profile 6 and annexes O.1 and P.5                       
 */
typedef enum MMP_VIDEO_H263PROFILETYPE {
    MMP_VIDEO_H263ProfileBaseline            = 0x01,        
    MMP_VIDEO_H263ProfileH320Coding          = 0x02,          
    MMP_VIDEO_H263ProfileBackwardCompatible  = 0x04,  
    MMP_VIDEO_H263ProfileISWV2               = 0x08,               
    MMP_VIDEO_H263ProfileISWV3               = 0x10,               
    MMP_VIDEO_H263ProfileHighCompression     = 0x20,     
    MMP_VIDEO_H263ProfileInternet            = 0x40,            
    MMP_VIDEO_H263ProfileInterlace           = 0x80,           
    MMP_VIDEO_H263ProfileHighLatency         = 0x100,         
    MMP_VIDEO_H263ProfileKhronosExtensions = 0x6F000000, /**< Reserved region for introducing Khronos Standard Extensions */ 
    MMP_VIDEO_H263ProfileVendorStartUnused = 0x7F000000, /**< Reserved region for introducing Vendor Extensions */
    MMP_VIDEO_H263ProfileMax                 = 0x7FFFFFFF  
} MMP_VIDEO_H263PROFILETYPE;


/** 
 * H.263 level types, each level indicates support for various frame sizes, 
 * bit rates, decoder frame rates.
 */
typedef enum MMP_VIDEO_H263LEVELTYPE {
    MMP_VIDEO_H263Level10  = 0x01,  
    MMP_VIDEO_H263Level20  = 0x02,      
    MMP_VIDEO_H263Level30  = 0x04,      
    MMP_VIDEO_H263Level40  = 0x08,      
    MMP_VIDEO_H263Level45  = 0x10,      
    MMP_VIDEO_H263Level50  = 0x20,      
    MMP_VIDEO_H263Level60  = 0x40,      
    MMP_VIDEO_H263Level70  = 0x80, 
    MMP_VIDEO_H263LevelKhronosExtensions = 0x6F000000, /**< Reserved region for introducing Khronos Standard Extensions */ 
    MMP_VIDEO_H263LevelVendorStartUnused = 0x7F000000, /**< Reserved region for introducing Vendor Extensions */
    MMP_VIDEO_H263LevelMax = 0x7FFFFFFF  
} MMP_VIDEO_H263LEVELTYPE;

/** 
 * H.263 Params 
 *
 * STRUCT MEMBERS:
 *  nSize                    : Size of the structure in bytes
 *  nVersion                 : OMX specification version information 
 *  nPortIndex               : Port that this structure applies to
 *  nPFrames                 : Number of P frames between each I frame
 *  nBFrames                 : Number of B frames between each I frame
 *  eProfile                 : H.263 profile(s) to use
 *  eLevel                   : H.263 level(s) to use
 *  bPLUSPTYPEAllowed        : Indicating that it is allowed to use PLUSPTYPE 
 *                             (specified in the 1998 version of H.263) to 
 *                             indicate custom picture sizes or clock 
 *                             frequencies 
 *  nAllowedPictureTypes     : Specifies the picture types allowed in the 
 *                             bitstream
 *  bForceRoundingTypeToZero : value of the RTYPE bit (bit 6 of MPPTYPE) is 
 *                             not constrained. It is recommended to change 
 *                             the value of the RTYPE bit for each reference 
 *                             picture in error-free communication
 *  nPictureHeaderRepetition : Specifies the frequency of picture header 
 *                             repetition
 *  nGOBHeaderInterval       : Specifies the interval of non-empty GOB  
 *                             headers in units of GOBs
 */
typedef struct MMP_VIDEO_PARAM_H263TYPE {
    //MMP_U32 nSize;
    //MMP_VERSIONTYPE nVersion;
    MMP_U32 nPortIndex;
    MMP_U32 nPFrames;
    MMP_U32 nBFrames;
    MMP_VIDEO_H263PROFILETYPE eProfile;
	MMP_VIDEO_H263LEVELTYPE eLevel;
    MMP_BOOL bPLUSPTYPEAllowed;
    MMP_U32 nAllowedPictureTypes;
    MMP_BOOL bForceRoundingTypeToZero;
    MMP_U32 nPictureHeaderRepetition;
    MMP_U32 nGOBHeaderInterval;
} MMP_VIDEO_PARAM_H263TYPE;


/** 
 * MPEG-4 profile types, each profile indicates support for various 
 * performance bounds and different annexes.
 * 
 * ENUMS:
 *  - Simple Profile, Levels 1-3
 *  - Simple Scalable Profile, Levels 1-2
 *  - Core Profile, Levels 1-2
 *  - Main Profile, Levels 2-4
 *  - N-bit Profile, Level 2
 *  - Scalable Texture Profile, Level 1
 *  - Simple Face Animation Profile, Levels 1-2
 *  - Simple Face and Body Animation (FBA) Profile, Levels 1-2
 *  - Basic Animated Texture Profile, Levels 1-2
 *  - Hybrid Profile, Levels 1-2
 *  - Advanced Real Time Simple Profiles, Levels 1-4
 *  - Core Scalable Profile, Levels 1-3
 *  - Advanced Coding Efficiency Profile, Levels 1-4
 *  - Advanced Core Profile, Levels 1-2
 *  - Advanced Scalable Texture, Levels 2-3
 */
typedef enum MMP_VIDEO_MPEG4PROFILETYPE {
    MMP_VIDEO_MPEG4ProfileSimple           = 0x01,        
    MMP_VIDEO_MPEG4ProfileSimpleScalable   = 0x02,    
    MMP_VIDEO_MPEG4ProfileCore             = 0x04,              
    MMP_VIDEO_MPEG4ProfileMain             = 0x08,             
    MMP_VIDEO_MPEG4ProfileNbit             = 0x10,              
    MMP_VIDEO_MPEG4ProfileScalableTexture  = 0x20,   
    MMP_VIDEO_MPEG4ProfileSimpleFace       = 0x40,        
    MMP_VIDEO_MPEG4ProfileSimpleFBA        = 0x80,         
    MMP_VIDEO_MPEG4ProfileBasicAnimated    = 0x100,     
    MMP_VIDEO_MPEG4ProfileHybrid           = 0x200,            
    MMP_VIDEO_MPEG4ProfileAdvancedRealTime = 0x400,  
    MMP_VIDEO_MPEG4ProfileCoreScalable     = 0x800,      
    MMP_VIDEO_MPEG4ProfileAdvancedCoding   = 0x1000,    
    MMP_VIDEO_MPEG4ProfileAdvancedCore     = 0x2000,      
    MMP_VIDEO_MPEG4ProfileAdvancedScalable = 0x4000,
    MMP_VIDEO_MPEG4ProfileAdvancedSimple   = 0x8000,
    MMP_VIDEO_MPEG4ProfileKhronosExtensions = 0x6F000000, /**< Reserved region for introducing Khronos Standard Extensions */ 
    MMP_VIDEO_MPEG4ProfileVendorStartUnused = 0x7F000000, /**< Reserved region for introducing Vendor Extensions */
    MMP_VIDEO_MPEG4ProfileMax              = 0x7FFFFFFF  
} MMP_VIDEO_MPEG4PROFILETYPE;

/** 
 * MPEG-4 level types, each level indicates support for various frame 
 * sizes, bit rates, decoder frame rates.  No need 
 */
typedef enum MMP_VIDEO_MPEG4LEVELTYPE {
    MMP_VIDEO_MPEG4Level0  = 0x01,   /**< Level 0 */   
    MMP_VIDEO_MPEG4Level0b = 0x02,   /**< Level 0b */   
    MMP_VIDEO_MPEG4Level1  = 0x04,   /**< Level 1 */ 
    MMP_VIDEO_MPEG4Level2  = 0x08,   /**< Level 2 */ 
    MMP_VIDEO_MPEG4Level3  = 0x10,   /**< Level 3 */ 
    MMP_VIDEO_MPEG4Level4  = 0x20,   /**< Level 4 */  
    MMP_VIDEO_MPEG4Level4a = 0x40,   /**< Level 4a */  
    MMP_VIDEO_MPEG4Level5  = 0x80,   /**< Level 5 */  
    MMP_VIDEO_MPEG4LevelKhronosExtensions = 0x6F000000, /**< Reserved region for introducing Khronos Standard Extensions */ 
    MMP_VIDEO_MPEG4LevelVendorStartUnused = 0x7F000000, /**< Reserved region for introducing Vendor Extensions */
    MMP_VIDEO_MPEG4LevelMax = 0x7FFFFFFF  
} MMP_VIDEO_MPEG4LEVELTYPE;

/** 
 * MPEG-4 configuration.  This structure handles configuration options
 * which are specific to MPEG4 algorithms
 *
 * STRUCT MEMBERS:
 *  nSize                : Size of the structure in bytes
 *  nVersion             : OMX specification version information
 *  nPortIndex           : Port that this structure applies to
 *  nSliceHeaderSpacing  : Number of macroblocks between slice header (H263+ 
 *                         Annex K). Put zero if not used
 *  bSVH                 : Enable Short Video Header mode
 *  bGov                 : Flag to enable GOV
 *  nPFrames             : Number of P frames between each I frame (also called 
 *                         GOV period)
 *  nBFrames             : Number of B frames between each I frame
 *  nIDCVLCThreshold     : Value of intra DC VLC threshold
 *  bACPred              : Flag to use ac prediction
 *  nMaxPacketSize       : Maximum size of packet in bytes.
 *  nTimeIncRes          : Used to pass VOP time increment resolution for MPEG4. 
 *                         Interpreted as described in MPEG4 standard.
 *  eProfile             : MPEG-4 profile(s) to use.
 *  eLevel               : MPEG-4 level(s) to use.
 *  nAllowedPictureTypes : Specifies the picture types allowed in the bitstream
 *  nHeaderExtension     : Specifies the number of consecutive video packet
 *                         headers within a VOP
 *  bReversibleVLC       : Specifies whether reversible variable length coding 
 *                         is in use
 */
typedef struct MMP_VIDEO_PARAM_MPEG4TYPE {
    //MMP_U32 nSize;
    //MMP_VERSIONTYPE nVersion;
    MMP_U32 nPortIndex;
    MMP_U32 nSliceHeaderSpacing;
    MMP_BOOL bSVH;
    MMP_BOOL bGov;
    MMP_U32 nPFrames;
    MMP_U32 nBFrames;
    MMP_U32 nIDCVLCThreshold;
    MMP_BOOL bACPred;
    MMP_U32 nMaxPacketSize;
    MMP_U32 nTimeIncRes;
    MMP_VIDEO_MPEG4PROFILETYPE eProfile;
    MMP_VIDEO_MPEG4LEVELTYPE eLevel;
    MMP_U32 nAllowedPictureTypes;
    MMP_U32 nHeaderExtension;
    MMP_BOOL bReversibleVLC;
} MMP_VIDEO_PARAM_MPEG4TYPE;


/** 
 * Specifies the picture type. These values should be OR'd to signal all 
 * pictures types which are allowed.
 *
 * ENUMS:
 *  Generic Picture Types:          I, P and B
 *  H.263 Specific Picture Types:   SI and SP
 *  H.264 Specific Picture Types:   EI and EP
 *  MPEG-4 Specific Picture Types:  S
 */
typedef enum MMP_VIDEO_PICTURETYPE {
    MMP_VIDEO_PictureTypeI   = 0x01,
    MMP_VIDEO_PictureTypeP   = 0x02,
    MMP_VIDEO_PictureTypeB   = 0x04,
    MMP_VIDEO_PictureTypeSI  = 0x08,
    MMP_VIDEO_PictureTypeSP  = 0x10,
    MMP_VIDEO_PictureTypeEI  = 0x11,
    MMP_VIDEO_PictureTypeEP  = 0x12,
    MMP_VIDEO_PictureTypeS   = 0x14,
    MMP_VIDEO_PictureTypeKhronosExtensions = 0x6F000000, /**< Reserved region for introducing Khronos Standard Extensions */ 
    MMP_VIDEO_PictureTypeVendorStartUnused = 0x7F000000, /**< Reserved region for introducing Vendor Extensions */
    MMP_VIDEO_PictureTypeMax = 0x7FFFFFFF
} MMP_VIDEO_PICTURETYPE;

/* Ref. cnm-coda960-datasheet-Anapass_v4.8.0.pdf Page 126 */
#define MMP_VIDEO_ENC_MIN_WIDTH  160
#define MMP_VIDEO_ENC_MIN_HEIGHT 128

#define MMP_VIDEO_DEC_MIN_WIDTH  160
#define MMP_VIDEO_DEC_MIN_HEIGHT 128

#define MMP_VIDEO_MAX_WIDTH  1920
#define MMP_VIDEO_MAX_HEIGHT 1088

#define MMP_VIDEO_THEORA_MAX_WIDTH  1280  
#define MMP_VIDEO_THEORA_MAX_HEIGHT 720


/* 
 * Vidoe Packet header

     start addr => ------------------------------

                     mmp_video_packet_header
      
        dsi_pos => -----------------------------
                   
                        dsi data

 ffmpeg_hdr_pos => -----------------------------
                       
                        ffmpeg hdr data
                  
                   -----------------------------

 */


#define MMP_VIDEO_PACKET_HEADER_KEY 0x0ABBCCDDEE101000L

struct mmp_video_packet_header {
    MMP_U64 key;
    MMP_S32 packet_hdr_size;
    MMP_S32 packet_size; /* packet_hdr_size + dsi_size + ffmpeg_hdr_size */ 

    enum MMP_FOURCC fourcc;
    MMP_S32 pic_width;
    MMP_S32 pic_height;

    MMP_S32 fps; 
    MMP_S32 bitrate;
 
    MMP_S32 dsi_size;
    MMP_S32 dsi_pos;

    MMP_S32 ffmpeg_hdr_size;
    MMP_S32 ffmpeg_hdr_pos;
};


/*
    Video RV
*/
#define MMP_RV_EXTRA_DATA_KEY 0x8888982A

/* mmp video meta struct */
#define MMP_IMAGE_META_KEY 0xCCCC0000
#define MMP_IMAGE_META_ALLOC_SIZE 1024
struct mmp_image_meta {
    MMP_U32 key;
    enum MMP_FOURCC fourcc;
    MMP_S32 plane_count; 
    MMP_S32 pic_width;
    MMP_S32 pic_height;
    MMP_S32 buf_width[3]; /* stride */
    MMP_S32 buf_height[3];
    MMP_ADDR buf_phyaddr[3]; /* buf physical address */
    MMP_ADDR buf_viraddr[3]; /* buf virtual address */
    MMP_U32 reserve[32];
};
#define MMP_IMAGE_META_INIT(pobj, fourcc_, plane_count_) \
    pobj->key=MMP_IMAGE_META_KEY, \
    pobj->fourcc = fourcc_,\
    pobj->plane_count = plane_count_

/* Video Buffer Definition */
#define MMP_VIDEO_STREAM_BUF_EXTRA_SIZE 512  

#define MMP_VIDEO_DECODED_BUF_MIN_COUNT_DEFAULT 8
#define MMP_VIDEO_DECODED_BUF_ACTUAL_COUNT_DEFAULT 12
//#define MMP_VIDEO_DECODED_BUF_ACTUAL_MAX_COUNT 32
#define MMP_VIDEO_DECODED_BUF_ACTUAL_MAX_COUNT 64

#endif
