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

#include "MmpDecoderImage.hpp"
#include "MmpUtil.hpp"
#include "MmpDecoderImage_JpegLib.hpp"
#include "MmpDecoderImage_V4L2.hpp"


/**********************************************************************************
  HW Codec Image Selection  ( Imgae : OpenMax IL Term)
***********************************************************************************/

#define MMP_HWCODEC_IMAGE_NONE               0x00
#define MMP_HWCODEC_IMAGE_SW                 0x10
#define MMP_HWCODEC_IMAGE_JPU                0x20

#if (MMP_PLATFORM == MMP_PLATFORM_BUILDROOT)
//#define MMP_HWCODEC_IMAGE MMP_HWCODEC_IMAGE_JPU
#define MMP_HWCODEC_IMAGE MMP_HWCODEC_IMAGE_NONE

#elif (MMP_PLATFORM == MMP_PLATFORM_ANDROID)
#define MMP_HWCODEC_IMAGE MMP_HWCODEC_IMAGE_NONE

#elif (MMP_PLATFORM == MMP_PLATFORM_WIN32)
#define MMP_HWCODEC_IMAGE MMP_HWCODEC_IMAGE_JPU
//#define MMP_HWCODEC_IMAGE MMP_HWCODEC_IMAGE_SW

#else
#define MMP_HWCODEC_IMAGE MMP_HWCODEC_IMAGE_NONE

#endif

/////////////////////////////////////////////////////////////////////////////
// CreateObject

CMmpDecoderImage* CMmpDecoderImage::CreateObject(struct MmpDecoderCreateConfig *pCreateConfig, MMP_BOOL bForceSW) {

	CMmpDecoderImage* pObj=NULL;
	
    if(bForceSW == MMP_TRUE) {
        pObj=new CMmpDecoderImage_JpegLib(pCreateConfig);
    }
    else { 

#if (MMP_HWCODEC_IMAGE == MMP_HWCODEC_IMAGE_JPU)

        pObj=new CMmpDecoderImage_V4L2(pCreateConfig);
        
        
#elif (MMP_HWCODEC_IMAGE == MMP_HWCODEC_IMAGE_SW)
        pObj=new CMmpDecoderImage_JpegLib(pCreateConfig);

#elif (MMP_HWCODEC_IMAGE == MMP_HWCODEC_IMAGE_NONE)
        pObj=new CMmpDecoderImage_JpegLib(pCreateConfig);

#else
#error "ERROR: Select HW Codec "    
#endif
    
    }


	if(pObj==NULL) {
    
        if( pObj->Open()!=MMP_SUCCESS )    {
            pObj->Close();
            delete pObj;
            pObj = NULL;
        }
    }

    return pObj;
}



/////////////////////////////////////////////////////////////
//CMmpDecoderImage Member Functions

static const enum MMP_FOURCC s_support_fourcc_in[]={
     MMP_FOURCC_VIDEO_JPEG
};

static const enum MMP_FOURCC s_support_fourcc_out[]={
     MMP_FOURCC_AUDIO_PCM
};


CMmpDecoderImage::CMmpDecoderImage(struct MmpDecoderCreateConfig *pCreateConfig, MMP_CHAR* class_name) : 

CMmpDecoder(pCreateConfig->fourcc, MMP_FOURCC_IMAGE_UNKNOWN, class_name, 
           s_support_fourcc_in, sizeof(s_support_fourcc_in)/sizeof(s_support_fourcc_in[0]),
           s_support_fourcc_out, sizeof(s_support_fourcc_out)/sizeof(s_support_fourcc_out[0])
           ) 

,m_nDecodingAvgFPS(0)
,m_nTotalDecDur(0)
,m_nTotalDecFrameCount(0)
{

	/* In format */
	m_bih_in.biSize = sizeof(MMPBITMAPINFOHEADER);
    m_bih_in.biWidth = pCreateConfig->nPicWidth;
    m_bih_in.biHeight = pCreateConfig->nPicHeight;
	m_bih_in.biPlanes = 1;
	m_bih_in.biBitCount = 24;
	m_bih_in.biCompression = MMP_FOURCC_VIDEO_MPEG4;
	m_bih_in.biSizeImage = 0;
	m_bih_in.biXPelsPerMeter = 0;
	m_bih_in.biYPelsPerMeter = 0;
	m_bih_in.biClrUsed = 0;
	m_bih_in.biClrImportant = 0;

	/* out format */
	m_bih_out.biSize = sizeof(MMPBITMAPINFOHEADER);
	m_bih_out.biWidth = m_bih_in.biWidth;
	m_bih_out.biHeight = m_bih_in.biHeight;
	m_bih_out.biPlanes = 3;
	m_bih_out.biBitCount = 12;
	m_bih_out.biCompression = MMP_FOURCC_IMAGE_YUV420M;
	m_bih_out.biSizeImage = MMP_YV12_FRAME_SIZE(m_bih_out.biWidth, m_bih_out.biHeight);
	m_bih_out.biXPelsPerMeter = 0;
	m_bih_out.biYPelsPerMeter = 0;
	m_bih_out.biClrUsed = 0;
	m_bih_out.biClrImportant = 0;

}


CMmpDecoderImage::~CMmpDecoderImage()
{

}

MMP_RESULT CMmpDecoderImage::Open()
{
    MMP_RESULT mmpResult;

    mmpResult=CMmpDecoder::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    return MMP_SUCCESS;
}


MMP_RESULT CMmpDecoderImage::Close()
{
    MMP_RESULT mmpResult;

    mmpResult=CMmpDecoder::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    return MMP_SUCCESS;
}



void CMmpDecoderImage::DecodeMonitor(class mmp_buffer_videoframe* p_buf_videoframe) {

    static MMP_U32 before_tick = 0, fps_sum=0, dur_sum=0;
    MMP_U32 start_tick = m_nClassStartTick, cur_tick;
    MMP_U32 dur_avg = 0;

    if(p_buf_videoframe != NULL) {

        fps_sum ++;
        dur_sum += p_buf_videoframe->get_coding_dur();

        m_nTotalDecFrameCount++;
        m_nTotalDecDur += p_buf_videoframe->get_coding_dur();
 
        cur_tick = CMmpUtil::GetTickCount();
        if( (cur_tick - before_tick) > 1000 ) {
        
            if(fps_sum != 0) {
                dur_avg = dur_sum/fps_sum;
            }
            
            MMPDEBUGMSG(0, (TEXT("[VideoDec %s %s %dx%d] %d. fps=%d dur=%d "), 
                this->get_class_name(),   this->get_fourcc_in_name(),  m_bih_out.biWidth, m_bih_out.biHeight,
                        (cur_tick-start_tick)/1000, fps_sum, dur_avg ));

            if(dur_avg > 0) {
                m_nDecodingAvgFPS = 1000/dur_avg;
            }
            else {
                m_nDecodingAvgFPS = 1000;
            }

            before_tick = cur_tick;
            fps_sum = 0;
            dur_sum = 0;
        }

    }
        
}