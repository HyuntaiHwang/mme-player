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

#ifndef _MMPPLAYERDEF_H__
#define _MMPPLAYERDEF_H__

#include "MmpDefine.h"


struct MmpEncoderCreateConfig {

    /* basic info */
    MMP_U32 nFormat;
    MMP_U32 nStreamType;
    
    /* Inport info */
    MMP_U32 nPicWidth;
    MMP_U32 nPicHeight;
    MMP_U32 nFrameRate;
    
    /* Outport info */
    MMP_U32 nIDRPeriod;
    MMP_U32 nBitRate;

    /* mutex handle */
    void* hw_sync_mutex_hdl;
    
    union {
        MMP_VIDEO_PARAM_H264TYPE h264;
        MMP_VIDEO_PARAM_H263TYPE h263;
        MMP_VIDEO_PARAM_MPEG4TYPE mpeg4;
    }enc_info;
    
};

/*

   -------------------------------------------------------------
   |                                                           |     
   |     |----------------------------------------|            |     
   |     |                                        |            |     
   |     |    |------------------------------|    |            |     
   |     |    |                              |    |            |     
   |     |    |                              |    |            |     
   |     |    |Screen                        |    |            |     
   |     |    |------------------------------|    |            |     
   |     |Window                                  |            |     
   |     |----------------------------------------|            |     
   |LCD                                                        | 
   ------------------------------------------------------------   
*/

#define MMP_RENDERER_TYPE_NORMAL     0x100
#define MMP_RENDERER_TYPE_DUMMY      0x101
#define MMP_RENDERER_TYPE_YUVWRITER  0x102
#define MMP_RENDERER_TYPE_DEFAULT    MMP_RENDERER_TYPE_NORMAL

typedef struct CMmpRenderCreateProp_st
{
    //Render Type
    MMP_U32 m_renderer_type;

    //Audio Renderer
    MMPWAVEFORMATEX m_wf;

    //Video Render
    void* m_hRenderWnd; //rendering window handle
    void* m_hRenderDC;  //rendering window DC
    int m_idRender;     //Render-ID   DISPLAY(0) YUV_WRITER(1) DUMMY(2) 
    
    int m_iBoardWidth;  //for example, lcd width or Monitor Width
    int m_iBoardHeight; //for example, lcd height or Monitor Height

    int m_iScreenPosX;
    int m_iScreenPosY;
    int m_iScreenWidth;
    int m_iScreenHeight;

    int m_iPicWidth;    // real picture width
    int m_iPicHeight;   // real picture height
    MMP_U32 pic_format;
    
    //Video Encoder Option
    MMP_BOOL m_bVideoEncoder;
    MMP_CHAR m_VideoEncFileName[MMP_FILENAME_MAX_LEN];
    MMP_BOOL m_bVideoEncoderForceSWCodec;
    struct MmpEncoderCreateConfig m_VideoEncoderCreateConfig;

}CMmpRendererCreateProp;


struct mmp_player_option_yuv {
    MMP_S32 width;
    MMP_S32 height;
    enum MMP_FOURCC fourcc;
};

typedef struct CMmpPlayerCreateProp_st
{
    //Source
    MMP_CHAR filename[MMP_FILENAME_MAX_LEN];

    //Video Config
    CMmpRendererCreateProp video_config;  
    MMP_BOOL bForceSWCodec;

    //Audio Config
    MMPWAVEFORMATEX audio_wf;

    union {
        struct mmp_player_option_yuv yuv;
    }option;

    void* callback_privdata;
    void (*callback)(void* priv, MMP_U32 msg, void* data1, void* data2);
}CMmpPlayerCreateProp;

#define MMP_MEDIASAMPLE_BUFFER_TYPE_HEAP_MEM 0x10
#define MMP_MEDIASAMPLE_BUFFER_TYPE_PHY_MEM  0x11
#define MMP_MEDIASAMPLE_BUFFER_TYPE_ION_FD   0x12
#define MMP_MEDIASAMPLE_BUFFER_TYPE_VIDEO_FRAME  0x13


#define MMP_MEDIASAMPLE_BUF_PCM 0

#if 0
typedef struct CMmpMediaSample_st
{
    MMP_U32 uiMediaType;
    MMP_U8* pAu;           //inbuf[2]  (input) stream au log addr (must)  ( enc: YUV  dec:stream)
    MMP_U32 uiAuSize;                  //inbuf[3]  (input) stream au size     (must)  ( enc: YUV size  dec:stream size) 
    MMP_U32 uiAuMaxSize;                  //inbuf[3]  (input) stream au size     (must)  ( enc: YUV size  dec:stream size) 
    MMP_U32 uiSampleNumber;            //inbuf[4]  (input) sample number for debugging
    MMP_S64 uiTimeStamp;
    MMP_U32 uiFlag;
}CMmpMediaSample;
#endif

#define MMP_DECODED_BUF_PCM MMP_MEDIASAMPLE_BUF_PCM
#define MMP_DECODED_BUF_Y MMP_YUV420_PLAINE_INDEX_Y
#define MMP_DECODED_BUF_U MMP_YUV420_PLAINE_INDEX_U
#define MMP_DECODED_BUF_V MMP_YUV420_PLAINE_INDEX_V
#define MMP_DECODED_BUF_VIDEO_FRAME 0
#if 0
typedef struct CMmpMediaSampleDecodeResult_st
{
    MMP_U32 uiResultType;
    
    MMP_BOOL bImage;                     //outbuf[0]
	MMP_BOOL bReconfig; 
	
    MMP_U32 uiDecodedBufferPhyAddr[MMP_IMAGE_MAX_PLANE_COUNT];
    MMP_U32 uiDecodedBufferLogAddr[MMP_IMAGE_MAX_PLANE_COUNT];
    MMP_U32 uiDecodedBufferStride[MMP_IMAGE_MAX_PLANE_COUNT];       //Buffer Stride
    MMP_U32 uiDecodedBufferAlignHeight[MMP_IMAGE_MAX_PLANE_COUNT];  //Buffer Align Height ex) if Mali, height = (height+16 - 15)*16/16
    MMP_U32 uiDecodedBufferMaxSize; 
    
    MMP_U32 uiDecodedSize;                       //outbuf[7]  reserve
    MMP_U32 uiDecodedDuration;  //outbuf[10]  
	MMP_S64 uiTimeStamp;  //outbuf[10]  
	
	MMP_U32 uiAuUsedByte;
	
	MMP_U32 uiAudioSampleRate;
	MMP_U32 uiAudioFrameCount;
	MMP_U32 uiAudioUpSamplingFactor;

}CMmpMediaSampleDecodeResult;
#endif

typedef struct CMmpMediaSampleEncode_st
{
    MMP_U32 uiSampleType;

    MMP_U32 uiBufferPhyAddr[MMP_IMAGE_MAX_PLANE_COUNT];
    MMP_U32 uiBufferLogAddr[MMP_IMAGE_MAX_PLANE_COUNT];
    MMP_U32 uiBufferStride[MMP_IMAGE_MAX_PLANE_COUNT];       //Buffer Stride
    MMP_U32 uiBufferAlignHeight[MMP_IMAGE_MAX_PLANE_COUNT];  //Buffer Align Height ex) if Mali, height = (height+16 - 15)*16/16
    MMP_U32 uiBufferMaxSize; 

    MMP_U32 uiSampleNumber;            //inbuf[4]  (input) sample number for debugging
	MMP_U32 uiTimeStamp;
    MMP_U32 uiFlag;

    MMP_PIXELFORMAT pixelformat;
}CMmpMediaSampleEncode;

#define  MMP_MEDIASAMPLE_RESULT_ENCBUF_COUNT 2
#define MMP_ENCODED_BUF_STREAM 0
#define MMP_DECODED_BUF_DSI    1

typedef struct CMmpMediaSampleEncodeResult_st
{
    MMP_U32 uiEncodedBufferPhyAddr[MMP_MEDIASAMPLE_RESULT_ENCBUF_COUNT];
    MMP_U32 uiEncodedBufferLogAddr[MMP_MEDIASAMPLE_RESULT_ENCBUF_COUNT];
    MMP_U32 uiEncodedBufferMaxSize[MMP_MEDIASAMPLE_RESULT_ENCBUF_COUNT];
    MMP_U32 uiEncodedStreamSize[MMP_MEDIASAMPLE_RESULT_ENCBUF_COUNT];
    
    MMP_U32 uiEncodedDuration;  
    MMP_U32 uiTimeStamp;     
    MMP_U32 uiFlag;     
    
}CMmpMediaSampleEncodeResult;


/*  
    Ananpass Multimedia Format Define

    ----------------------

        header  (4096 byte)

    ----------------------


        stream data ( Video, Audio...)

    -----------------------

        config data

    -----------------------

        Index Data 
    ------------------------

*/
#define MMP_AMF_FILE_KEY1 0xCC00CC04
#define MMP_AMF_FILE_KEY2 0xCC44CC8C
#define MMP_AMF_MAX_MEDIA_COUNT     16
#define MMP_AMF_HEADER_SIZE 4096
#define MMP_AMF_STREAM_START_OFFSET MMP_AMF_HEADER_SIZE
typedef struct CMmpAmmfHeader_st {
    MMP_U32  uiKey1;
    MMP_U32  uiKey2;
    MMP_BOOL bIsMedia[MMP_AMF_MAX_MEDIA_COUNT];

    MMP_U32 uiFileSize;
    MMP_U32 uiPlayDuration;
    MMP_U32 uiRes1;
    MMP_U32 uiRes2;

    MMP_U32 uiHeaderDataFileOffset;
    MMP_U32 uiHeaderDataSize;

    MMP_U32 uiStreamDataFileOffset;
    MMP_U32 uiStreamDataSize;

    MMP_U32 uiConfigDataFileOffset[MMP_AMF_MAX_MEDIA_COUNT];
    MMP_U32 uiConfigDataSize[MMP_AMF_MAX_MEDIA_COUNT];

    MMP_U32 uiIndexFileOffset[MMP_AMF_MAX_MEDIA_COUNT];
    MMP_U32 uiIndexCount[MMP_AMF_MAX_MEDIA_COUNT];

    MMPWAVEFORMATEX wf;
    MMPBITMAPINFOHEADER bih;
    
    MMP_U8 reserve[32];
}CMmpAmmfHeader;

typedef struct CMmpAmmfIndex_st {
    MMP_U32 uiStreamType;
    MMP_U32 uiStreamSize;
    MMP_U32 uiFileOffset;
    MMP_U32 uiTimeStamp;
    MMP_U32 uiFlag;
}CMmpAmmfIndex;

#endif


