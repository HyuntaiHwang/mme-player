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

#include "MmpEncoderVideo.hpp"
#include "MmpUtil.hpp"
#include "MmpMpeg4Tool.hpp"

#define MMP_HWCODEC_VIDEO_SW     0x10
#define MMP_HWCODEC_VIDEO_VPU    0x20
#define MMP_HWCODEC_VIDEO_DUMMY  0x30

#if (MMP_SOC == MMP_SOC_GDM7243V)
//#define MMP_HWCODEC_VIDEO MMP_HWCODEC_VIDEO_SW
#define MMP_HWCODEC_VIDEO MMP_HWCODEC_VIDEO_VPU
//#define MMP_HWCODEC_VIDEO MMP_HWCODEC_VIDEO_DUMMY

#elif (MMP_PLATFORM == MMP_PLATFORM_WIN32)
#define MMP_HWCODEC_VIDEO MMP_HWCODEC_VIDEO_SW
//#define MMP_HWCODEC_VIDEO MMP_HWCODEC_VIDEO_NONE
//#define MMP_HWCODEC_VIDEO MMP_HWCODEC_VIDEO_VPU
//#define MMP_HWCODEC_VIDEO MMP_HWCODEC_VIDEO_VPU
//#define MMP_HWCODEC_VIDEO MMP_HWCODEC_VIDEO_DUMMY

#else
#define MMP_HWCODEC_VIDEO MMP_HWCODEC_VIDEO_NONE
#endif

#include "MmpEncoderVideo_Vpu.hpp"
#include "MmpEncoderVideo_Dummy.hpp"

#if (FFMPEG_VERSION != FFMPEG_DISABLE)
#include "MmpEncoderVideo_Ffmpeg.hpp"
#include "MmpEncoderVideo_FfmpegEx1.hpp"

#if (FFMPEG_VERSION == FFMPEG_VERSION_2_6_1)
#define CMmpEncoderVideo_SWCodec CMmpEncoderVideo_Ffmpeg

#elif (FFMPEG_VERSION == FFMPEG_VERSION_n3_1_1)
#define CMmpEncoderVideo_SWCodec CMmpEncoderVideo_FfmpegEx1
#endif

#endif

/////////////////////////////////////////////////////////////
//CreateObject 

MMP_RESULT CMmpEncoderVideo::gen_create_config(enum MMP_FOURCC fourcc_in,  enum MMP_FOURCC fourcc_out,
                                               MMP_S32 pic_width, MMP_S32 pic_height,
                                               MMP_S32 fr /*framerate*/, MMP_S32 ip /*idr_period*/, MMP_S32 br /*bitrate*/,
                                               MMP_S32 sw_codec_use,
                                               MMP_OUT struct CMmpEncoderVideo::create_config *p_create_config
                                               ) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
        
    memset(p_create_config, 0x00,  sizeof(*p_create_config) );

    if(sw_codec_use) {
        p_create_config->enc_type = CMmpEncoderVideo::ENC_SW;
    }
    
    /* basic info */
    p_create_config->fourcc_in = fourcc_in;
    p_create_config->fourcc_out = fourcc_out;
    
    /* Inport info */
    p_create_config->pic_width = pic_width;
    p_create_config->pic_height = pic_height;
    p_create_config->fr = fr;

    /* Outport info */
    p_create_config->ip = ip; /* idr period */
    p_create_config->br = br; /* bitrate */ 

    /* Set H264 Info */
    switch(fourcc_out) {
        case MMP_FOURCC_VIDEO_H264:
                p_create_config->codec.h264.nSliceHeaderSpacing = 0;
                p_create_config->codec.h264.nPFrames = 20;
                p_create_config->codec.h264.nBFrames = 0;
                p_create_config->codec.h264.bUseHadamard = MMP_TRUE;
                p_create_config->codec.h264.nRefFrames = 1;
	            p_create_config->codec.h264.nRefIdx10ActiveMinus1 = 0;
	            p_create_config->codec.h264.nRefIdx11ActiveMinus1 = 0;
                p_create_config->codec.h264.bEnableUEP = MMP_FALSE;
                p_create_config->codec.h264.bEnableFMO = MMP_FALSE;
                p_create_config->codec.h264.bEnableASO = MMP_FALSE;
                p_create_config->codec.h264.bEnableRS = MMP_FALSE;
                p_create_config->codec.h264.eProfile = MMP_VIDEO_AVCProfileMain;;
	            p_create_config->codec.h264.eLevel = MMP_VIDEO_AVCLevel4;
                p_create_config->codec.h264.nAllowedPictureTypes = (MMP_VIDEO_PictureTypeI | MMP_VIDEO_PictureTypeP);
	            p_create_config->codec.h264.bFrameMBsOnly = MMP_FALSE;
                p_create_config->codec.h264.bMBAFF = MMP_FALSE;
                p_create_config->codec.h264.bEntropyCodingCABAC = MMP_FALSE;
                p_create_config->codec.h264.bWeightedPPrediction = MMP_FALSE;
                p_create_config->codec.h264.nWeightedBipredicitonMode = MMP_FALSE;
                p_create_config->codec.h264.bconstIpred  = MMP_FALSE;
                p_create_config->codec.h264.bDirect8x8Inference = MMP_FALSE;
	            p_create_config->codec.h264.bDirectSpatialTemporal = MMP_FALSE;
	            p_create_config->codec.h264.nCabacInitIdc = 0;
	            p_create_config->codec.h264.eLoopFilterMode = MMP_VIDEO_AVCLoopFilterEnable;
                break;

        case MMP_FOURCC_VIDEO_H263: 
                //p_create_config->codec.h263.nSize = this->m_videotype.h263.nSize;
                //p_create_config->codec.h263.nVersion= this->m_videotype.h263.nVersion;
                p_create_config->codec.h263.nPortIndex= 1;
                p_create_config->codec.h263.nPFrames= 20;
                p_create_config->codec.h263.nBFrames= 0;
                p_create_config->codec.h263.eProfile= MMP_VIDEO_H263ProfileHighCompression;
	            p_create_config->codec.h263.eLevel= MMP_VIDEO_H263Level70;
                p_create_config->codec.h263.bPLUSPTYPEAllowed= MMP_FALSE;
                p_create_config->codec.h263.nAllowedPictureTypes= (MMP_VIDEO_PictureTypeI | MMP_VIDEO_PictureTypeP);
                p_create_config->codec.h263.bForceRoundingTypeToZero= MMP_FALSE;
                p_create_config->codec.h263.nPictureHeaderRepetition= 0;
                p_create_config->codec.h263.nGOBHeaderInterval= 0;
                break;

       case MMP_FOURCC_VIDEO_MPEG4: 
                p_create_config->codec.mpeg4.nPortIndex = 1;//this->m_videotype.mpeg4.nPortIndex;
                p_create_config->codec.mpeg4.nSliceHeaderSpacing = 0;//this->m_videotype.mpeg4.nSliceHeaderSpacing;
                p_create_config->codec.mpeg4.bSVH = MMP_FALSE;
                p_create_config->codec.mpeg4.bGov = MMP_FALSE;
                p_create_config->codec.mpeg4.nPFrames = 1;//this->m_videotype.mpeg4.nPFrames;
                p_create_config->codec.mpeg4.nBFrames = 0;//this->m_videotype.mpeg4.nBFrames;
                p_create_config->codec.mpeg4.nIDCVLCThreshold = 0;
                p_create_config->codec.mpeg4.bACPred = MMP_TRUE;
                p_create_config->codec.mpeg4.nMaxPacketSize = 256;
                p_create_config->codec.mpeg4.nTimeIncRes = 1000;
                p_create_config->codec.mpeg4.eProfile = MMP_VIDEO_MPEG4ProfileCore;
                p_create_config->codec.mpeg4.eLevel = MMP_VIDEO_MPEG4Level2;
                p_create_config->codec.mpeg4.nAllowedPictureTypes = (MMP_VIDEO_PictureTypeI | MMP_VIDEO_PictureTypeP);;
                p_create_config->codec.mpeg4.nHeaderExtension = 0;
                p_create_config->codec.mpeg4.bReversibleVLC = MMP_FALSE;
                break;
    }

    
    return mmpResult;

}

CMmpEncoderVideo* CMmpEncoderVideo::CreateObject(enum ENC_TYPE enc_type,
                                                enum MMP_FOURCC fourcc_in, enum MMP_FOURCC fourcc_out,
                                                MMP_S32 pic_width, MMP_S32 pic_height,
                                                MMP_S32 fr /*framerate*/, MMP_S32 ip /*idr_period*/, MMP_S32 br /*bitrate*/) {
    
    struct CMmpEncoderVideo::create_config create_config;
    MMP_RESULT mmpResult;
    CMmpEncoderVideo* pMmpEncoderVideo = NULL;

    mmpResult = CMmpEncoderVideo::gen_create_config(fourcc_in, fourcc_out, pic_width, pic_height, fr, ip, br, 0/*sw_codec_use*/,&create_config);
    if(mmpResult == MMP_SUCCESS) {
        create_config.enc_type = enc_type;
        pMmpEncoderVideo = CMmpEncoderVideo::CreateObject(&create_config);
    }

    return pMmpEncoderVideo;
}

CMmpEncoderVideo* CMmpEncoderVideo::CreateObject(struct CMmpEncoderVideo::create_config *p_create_config) {

    CMmpEncoderVideo* pObj=NULL;
    enum ENC_TYPE enc_type;
    
    enc_type = p_create_config->enc_type;

    switch(enc_type) {

        case ENC_SW:
#if (FFMPEG_VERSION != FFMPEG_DISABLE)
            pObj=new CMmpEncoderVideo_SWCodec(p_create_config);
#else
            pObj = NULL;
#endif 
            break;

        case ENC_HW:
            pObj=new CMmpEncoderVideo_Vpu(p_create_config);
            break;

        case ENC_DUMMY:
            pObj=new CMmpEncoderVideo_Dummy(p_create_config);
            break;

        default:
        #if (MMP_HWCODEC_VIDEO == MMP_HWCODEC_VIDEO_VPU)
            pObj=new CMmpEncoderVideo_Vpu(p_create_config);

        #elif (MMP_HWCODEC_VIDEO == MMP_HWCODEC_VIDEO_SW)
            pObj=new CMmpEncoderVideo_SWCodec(p_create_config);

        #elif (MMP_HWCODEC_VIDEO == MMP_HWCODEC_VIDEO_DUMMY)
            pObj=new CMmpEncoderVideo_Dummy(p_create_config);

        #else
        #error "ERROR: Select HW Codec "    
        #endif

    }
    
	if(pObj != NULL) {

        if( pObj->Open()!=MMP_SUCCESS ) {
            pObj->Close();
            delete pObj;
            pObj = NULL;
        }
    }

    return pObj;
}

/////////////////////////////////////////////////////////////
//CMmpEncoderVideo Member Functions

static const enum MMP_FOURCC s_support_fourcc_in[]={
     
     MMP_FOURCC_IMAGE_YUV420,
     
     MMP_FOURCC_IMAGE_NV12,
     MMP_FOURCC_IMAGE_ANDROID_OPAQUE
};


static const enum MMP_FOURCC s_support_fourcc_out[]={
     
     MMP_FOURCC_VIDEO_H264,
     MMP_FOURCC_VIDEO_MPEG4,
     MMP_FOURCC_VIDEO_H263,
};


CMmpEncoderVideo::CMmpEncoderVideo(struct CMmpEncoderVideo::create_config *p_create_config, MMP_CHAR* class_name) : 
     
CMmpEncoder(p_create_config->fourcc_in, p_create_config->fourcc_out, class_name, 
            s_support_fourcc_in, sizeof(s_support_fourcc_in)/sizeof(s_support_fourcc_in[0]),
            s_support_fourcc_out, sizeof(s_support_fourcc_out)/sizeof(s_support_fourcc_out[0])
            ) 

,m_create_config(*p_create_config)

,m_mon_before_tick(0)
,m_mon_fps_sum(0)
,m_mon_bitrate_sum(0)
,m_mon_pre_dur_sum(0)
,m_mon_enc_dur_sum(0)

,m_fourcc_last_input(MMP_FOURCC_VIDEO_UNKNOWN)
,m_DEBUG_MMEEnc(MMP_FALSE)
{

	m_DEBUG_MMEEnc = CMmpUtil::IsDebugEnable_MMEEnc();
}


CMmpEncoderVideo::~CMmpEncoderVideo()
{

}

MMP_RESULT CMmpEncoderVideo::Open()
{
    MMP_RESULT mmpResult;

    mmpResult=CMmpEncoder::Open();
    
    return mmpResult;
}


MMP_RESULT CMmpEncoderVideo::Close()
{
    MMP_RESULT mmpResult;
    
    mmpResult=CMmpEncoder::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    
    return MMP_SUCCESS;
}

void CMmpEncoderVideo::print_status(class mmp_buffer_videostream* p_buf_vs) {

    MMP_U32 before_tick = m_mon_before_tick, fps_sum=m_mon_fps_sum, enc_dur_sum=m_mon_enc_dur_sum, pre_dur_sum=m_mon_pre_dur_sum;
    MMP_U32 bitrate_sum = m_mon_bitrate_sum;
    MMP_U32 start_tick = m_nClassStartTick, cur_tick;
    MMP_U32 enc_dur_avg = 0, pre_dur_avg=0;
    //struct mmp_system_meminfo meminfo;

    if(p_buf_vs != NULL) {

        if(p_buf_vs->get_stream_real_size() > 0) {

            fps_sum ++;
            pre_dur_sum += p_buf_vs->get_preproc_dur();
            enc_dur_sum += p_buf_vs->get_coding_dur();

            bitrate_sum += p_buf_vs->get_stream_real_size();
        }
    }
    
    cur_tick = CMmpUtil::GetTickCount();
    if( (cur_tick - before_tick) > 1000 ) {
        
        if(fps_sum != 0) {
            pre_dur_avg = pre_dur_sum/fps_sum;
            enc_dur_avg = enc_dur_sum/fps_sum;
        }

        //CMmpUtil::system_meminfo(&meminfo);
            
        MMPDEBUGMSG(1, (TEXT("[MME-VEnc %s %s(%c%c%c%c) %s %dx%d] %d. fps=%d br=%dkbps dur=(pre:%d enc:%d)  "), 
                    this->get_class_name(),  
                    this->get_fourcc_in_name(),  MMPGETFOURCCARG(m_fourcc_last_input), 
                    this->get_fourcc_out_name(),  this->get_pic_width(), this->get_pic_height(),
                    (cur_tick-start_tick)/1000, 
                    fps_sum, bitrate_sum*8/1000, 
                    pre_dur_avg, enc_dur_avg
                    //meminfo.MemFree>>10 /* meminfo.MemFree/1024, */, meminfo.MemTotal>>10 /* meminfo.MemTotal/1024 */
                    ));

        
        before_tick = cur_tick;
        fps_sum = 0;
        bitrate_sum = 0;
        pre_dur_sum = 0;
        enc_dur_sum = 0;
    }

    m_mon_before_tick = before_tick;
    m_mon_fps_sum = fps_sum;
    m_mon_bitrate_sum = bitrate_sum;
    m_mon_pre_dur_sum = pre_dur_sum;
    m_mon_enc_dur_sum = enc_dur_sum;
}

