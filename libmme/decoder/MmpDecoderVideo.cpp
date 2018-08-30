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

#include "MmpDecoderVideo.hpp"
#include "MmpUtil.hpp"
#include "mmp_lock.hpp"


/**********************************************************************************
  HW Codec Video Selection
***********************************************************************************/

#define MMP_HWCODEC_VIDEO_NONE                  0x00
#define MMP_HWCODEC_VIDEO_SW                    0x10
#define MMP_HWCODEC_VIDEO_VPU                   0x20
#define MMP_HWCODEC_VIDEO_EXYNOS4_MFC           0x30  /* Exynos4 Android 4.3 */
#define MMP_HWCODEC_VIDEO_EXYNOS4_MFC_ANDROID44 0x31  /* Exynos4 Android 4.4 */
#define MMP_HWCODEC_VIDEO_V4L2                  0x40  /* V4L2 Standard */
#define MMP_HWCODEC_VIDEO_V4L2_MFC              0x41  /* V4L2 MFC Exynsos4412 */

#if (MMP_SOC == MMP_SOC_GDM7243V)
#define MMP_HWCODEC_VIDEO MMP_HWCODEC_VIDEO_VPU
//#define MMP_HWCODEC_VIDEO MMP_HWCODEC_VIDEO_NONE

#elif (MMP_PLATFORM == MMP_PLATFORM_WIN32)
#define MMP_HWCODEC_VIDEO MMP_HWCODEC_VIDEO_NONE
//#define MMP_HWCODEC_VIDEO MMP_HWCODEC_VIDEO_VPU

#else
#define MMP_HWCODEC_VIDEO MMP_HWCODEC_VIDEO_NONE
#endif


#if (MMP_HWCODEC_VIDEO == MMP_HWCODEC_VIDEO_VPU)
#include "MmpDecoderVideo_VpuEx1.hpp"
#elif (MMP_HWCODEC_VIDEO == MMP_HWCODEC_VIDEO_V4L2_MFC)
#include "MmpDecoderVideo_V4L2_MFC.hpp"
#include "MmpDecoderVideo_V4L2_ION.hpp"
#elif ((MMP_HWCODEC_VIDEO == MMP_HWCODEC_VIDEO_EXYNOS4_MFC) || (MMP_HWCODEC_VIDEO == MMP_HWCODEC_VIDEO_EXYNOS4_MFC_ANDROID44) )
#include "MmpDecoderVideo_Mfc.hpp"
#endif

/* Video */
#if (FFMPEG_VERSION != FFMPEG_DISABLE)
#include "MmpDecoderVideo_Ffmpeg.hpp"
#include "MmpDecoderVideo_FfmpegEx1.hpp"

#if (FFMPEG_VERSION == FFMPEG_VERSION_2_6_1)
#define CMmpDecoderVideo_SWCodec CMmpDecoderVideo_Ffmpeg

#elif (FFMPEG_VERSION == FFMPEG_VERSION_n3_1_1)
#define CMmpDecoderVideo_SWCodec CMmpDecoderVideo_FfmpegEx1
#endif

#endif

/////////////////////////////////////////////////////////////
//CreateObject 

CMmpDecoderVideo* CMmpDecoderVideo::CreateObject(struct CMmpDecoderVideo::create_config *p_create_config, MMP_BOOL bForceFfmpeg) {

	CMmpDecoderVideo* pObj=NULL;

#if (FFMPEG_VERSION != FFMPEG_DISABLE)
    if( (p_create_config->fourcc_in == MMP_FOURCC_VIDEO_FFMPEG) 
       || (p_create_config->fourcc_in == MMP_FOURCC_VIDEO_WMV1) 
       || (p_create_config->fourcc_in == MMP_FOURCC_VIDEO_WMV2) 
       )
    {
        bForceFfmpeg = MMP_TRUE;
    }
#endif

    if( (bForceFfmpeg == MMP_TRUE) ) {
#if (FFMPEG_VERSION != FFMPEG_DISABLE)
        pObj=new CMmpDecoderVideo_SWCodec(p_create_config);
#endif
    }
    else { 

#if (MMP_HWCODEC_VIDEO == MMP_HWCODEC_VIDEO_VPU)
        pObj=new CMmpDecoderVideo_VpuEx1(p_create_config);
        
#elif (MMP_HWCODEC_VIDEO == MMP_HWCODEC_VIDEO_V4L2_MFC)
        pObj=new CMmpDecoderVideo_V4L2_MFC(p_create_config);

#elif (MMP_HWCODEC_VIDEO == MMP_HWCODEC_VIDEO_NONE)

#if (FFMPEG_VERSION == FFMPEG_DISABLE)
        pObj=NULL; 
#else
        pObj=new CMmpDecoderVideo_SWCodec(p_create_config);
#endif

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
//CMmpDecoderVideo Member Functions

CMmpDecoderVideo::CMmpDecoderVideo(struct CMmpDecoderVideo::create_config *p_create_config, MMP_CHAR* class_name,
                                   const enum MMP_FOURCC *support_fourcc_in_list, MMP_S32 support_fourcc_in_count,   
                                   const enum MMP_FOURCC *support_fourcc_out_list, MMP_S32 support_fourcc_out_count  ) : 

CMmpDecoder(p_create_config->fourcc_in, p_create_config->fourcc_out, class_name, 
            support_fourcc_in_list, support_fourcc_in_count,
            support_fourcc_out_list, support_fourcc_out_count) 

,m_nDecodingAvgFPS(0)
,m_nTotalDecDur(0)
,m_nTotalDecFrameCount(0)

#if 0
,m_demuxer_pic_width(MMP_VIDEO_DEC_MIN_WIDTH)
,m_demuxer_pic_height(MMP_VIDEO_DEC_MIN_HEIGHT)
,m_decoder_pic_width(MMP_VIDEO_DEC_MIN_WIDTH)
,m_decoder_pic_height(MMP_VIDEO_DEC_MIN_HEIGHT)
,m_decoder_buf_width( V4L2_VIDEO_FRAME_STRIDE_ALIGN(MMP_VIDEO_DEC_MIN_WIDTH) )
,m_decoder_buf_height( V4L2_VIDEO_FRAME_HEIGHT_ALIGN(MMP_VIDEO_DEC_MIN_HEIGHT) )
#else

,m_demuxer_pic_width(p_create_config->pic_width)
,m_demuxer_pic_height(p_create_config->pic_height)
,m_decoder_pic_width(p_create_config->pic_width)
,m_decoder_pic_height(p_create_config->pic_height)
,m_decoder_buf_width( V4L2_VIDEO_FRAME_STRIDE_ALIGN(p_create_config->pic_width) )
,m_decoder_buf_height( V4L2_VIDEO_FRAME_HEIGHT_ALIGN(p_create_config->pic_height) )

#endif

#if 0
,m_buf_req_count_min(MMP_VIDEO_DECODED_BUF_MIN_COUNT_DEFAULT)
,m_buf_req_count_actual(MMP_VIDEO_DECODED_BUF_ACTUAL_COUNT_DEFAULT)
#else
,m_buf_req_count_min(p_create_config->buf_req_count_min)
,m_buf_req_count_actual(p_create_config->buf_req_count_actual)

#endif

,m_call_count_init_buf_vf_from_demuxer(0)

,m_buf_vf_decoded_count(0)
,m_buf_vf_index(0)

#if 0
,m_is_android_buffer(MMP_FALSE)
#else
,m_is_android_buffer(p_create_config->is_android_buffer)
#endif

,m_mon_before_tick(0)
,m_mon_fps_sum(0)
,m_mon_fps_fail_sum(0)
,m_mon_coding_dur_sum(0)
,m_mon_postproc_dur_sum(0)
,m_mon_au_sum_size(0)
,m_mon_au_sum_size_total(0)

,m_DEBUG_MMEDec(MMP_FALSE)
{
    MMP_S32 i;

    for(i = 0; i < DECODED_BUF_MAX_COUNT; i++) {
        m_p_buf_vf_decoded[i] = NULL;
    }

    m_DEBUG_MMEDec = CMmpUtil::IsDebugEnable_MMEDec();


    m_display_crop_rect.left = 0;
    m_display_crop_rect.top = 0;
    m_display_crop_rect.right = m_demuxer_pic_width-1;
    m_display_crop_rect.bottom = m_demuxer_pic_height-1;
        
}

CMmpDecoderVideo::~CMmpDecoderVideo() {

}

MMP_RESULT CMmpDecoderVideo::Open()
{
    MMP_RESULT mmpResult;

    mmpResult = CMmpDecoder::Open();
    if(mmpResult == MMP_SUCCESS) {
        
        
    }


    return mmpResult;
}


MMP_RESULT CMmpDecoderVideo::Close()
{
    MMP_RESULT mmpResult;

    mmpResult=CMmpDecoder::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    
    return MMP_SUCCESS;
}

MMP_S32 CMmpDecoderVideo::GetAvgFPS() {
    
    MMP_S32 fps=0;
    MMP_S32 avgdur;

    if(m_nTotalDecFrameCount > 0)  {
        avgdur = (MMP_S32)(m_nTotalDecDur/m_nTotalDecFrameCount);
        if(avgdur == 0) {
            fps = 1000;
        }
        else {
            fps = 1000/avgdur;
        }
    }

    return fps;
}

MMP_S32 CMmpDecoderVideo::GetAvgDur() {

    MMP_S32 avgdur = 0;

    if(m_nTotalDecFrameCount > 0)  {
        avgdur = (MMP_S32)(m_nTotalDecDur/m_nTotalDecFrameCount);
    }

    return avgdur;
}


MMP_RESULT CMmpDecoderVideo::init_buf_vf_from_demuxer(MMP_S32 demuxer_pic_width, MMP_S32 demuxer_pic_height, enum MMP_FOURCC fourcc_out, MMP_S32 buf_req_cnt_min, MMP_S32 buf_req_cnt_actual) {

    MMP_S32 i;
    MMP_BOOL bflag = MMP_TRUE;
    MMP_RESULT mmpResult = MMP_FAILURE;
    enum MMP_FOURCC fourcc;

    fourcc = this->get_fourcc_out();

#if 1
    if(m_call_count_init_buf_vf_from_demuxer > 0) {
        
        MMP_S32 demuxer_pic_height1;
        demuxer_pic_height1 = mmp_buffer_videoframe::s_get_height(fourcc_out, m_demuxer_pic_height, MMP_BUFTYPE_HW); 
        if(demuxer_pic_height1 == demuxer_pic_height) {
            demuxer_pic_height = m_demuxer_pic_height;
        }   
    }
#endif
    
    /*
        2016,10,20 비록 값이 일치하더라도, 최소 1번은 init_buf 가 수행되도록 하는게 좋겠다. 
    */
    if(m_call_count_init_buf_vf_from_demuxer > 0) {

        if(  (demuxer_pic_width != m_demuxer_pic_width)
            || (demuxer_pic_height != m_demuxer_pic_height)
            || ( fourcc != fourcc_out)
            || (buf_req_cnt_min != m_buf_req_count_min)
            || (buf_req_cnt_actual != m_buf_req_count_actual)
            ) {
            /* Nothing to do */
        }
        else {
            bflag &= MMP_FALSE;
        }

    }


    if(buf_req_cnt_actual < buf_req_cnt_min) {
        bflag &= MMP_FALSE;
    }

    if(demuxer_pic_width < MMP_VIDEO_DEC_MIN_WIDTH) {
        bflag &= MMP_FALSE;
    }

    if(demuxer_pic_height < MMP_VIDEO_DEC_MIN_HEIGHT) {
        bflag &= MMP_FALSE;
    }

    if(bflag == MMP_TRUE)  {
        if(CMmpDecoder::set_fourcc_out(fourcc_out) != MMP_SUCCESS) {
            bflag &= MMP_FALSE;
        }
    }

    if(bflag == MMP_TRUE)  {
        
        /*
           VPU 기준으로 이 코드를 작성한다.
           
           Base. 
               1. decoder pic_width는 demuxer_width와 같다고 가정한다. 
               2. decoder pic_height는 반드시 16 align 이라고 가정한다. 
               3. display영역은  demuxer가 지정한 영역과 같다고 가정한다.

           2016, 10, 3 
                  CTS-media/android.media.cts.MediaPlayerTest/testLocalVideo_MP4_H264_480x360_1000kbps_25fps_AAC_Stereo_128kbps_44110Hz
                  위 기준2번은 demuxer_height와 decoder_height가 불일치가 발생한다. 
                  이 경우 CTS FAIL이 발생한다. 
                  decoder_height와 demuxer_height를 width처럼 같다고 가정하고 코드를 작성해야 한다. 
                   
           2016, 10, 7
                  10.3일 CTS때문에  decoder_pic_height를 demux_pic_height와 동일하게 만든 코드는  1080P 재생시, MALI에서 Chroma위치가 안맞아 색감이 맞지 않게 된다.
                  현재로서는 방법을 찾을 수 없다. 
                  원래 Base기준으로 돌아가고,  CTS는 다른 해결방법을 찾아보자. 
        */
        m_demuxer_pic_width = demuxer_pic_width;
        m_decoder_pic_width = m_demuxer_pic_width;
        m_decoder_buf_width = mmp_buffer_videoframe::s_get_stride(fourcc_out, m_demuxer_pic_width, MMP_BUFTYPE_HW);
        
#if 0 /* CTS통과, 1080P Chroma Align안맞음 */
        m_demuxer_pic_height = demuxer_pic_height;
        m_decoder_pic_height = m_demuxer_pic_height;//mmp_buffer_videoframe::s_get_height(fourcc_out, m_demuxer_pic_height, MMP_BUFTYPE_HW); 
        m_decoder_buf_height = mmp_buffer_videoframe::s_get_height(fourcc_out, m_demuxer_pic_height, MMP_BUFTYPE_HW);  //m_decoder_pic_height;
#else
        m_demuxer_pic_height = demuxer_pic_height;
        m_decoder_pic_height = mmp_buffer_videoframe::s_get_height(fourcc_out, m_demuxer_pic_height, MMP_BUFTYPE_HW); 
        m_decoder_buf_height = m_decoder_pic_height;
#endif

        m_display_crop_rect.left = 0;
        m_display_crop_rect.top = 0;
        m_display_crop_rect.right = m_demuxer_pic_width-1;
        m_display_crop_rect.bottom = m_demuxer_pic_height-1;
        
        m_buf_req_count_min = buf_req_cnt_min;
        m_buf_req_count_actual = buf_req_cnt_actual;
        m_buf_vf_index = 0;
        m_buf_vf_decoded_count = 0;
        for(i = 0; i < DECODED_BUF_MAX_COUNT; i++) {
            m_p_buf_vf_decoded[i] = NULL;
        }

        mmpResult = MMP_SUCCESS;
        MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[%s] ln=%d Success  : m_buf_vf_decoded_count=%d  Demuxr(%d %d) Dec(%d %d) Buf(%d %d) Req(%d %d) "),  __func__, __LINE__, m_buf_vf_decoded_count,
                      m_demuxer_pic_width,  m_demuxer_pic_height, 
                      m_decoder_pic_width, m_decoder_pic_height, 
                      m_decoder_buf_width, m_decoder_buf_height,
                      m_buf_req_count_min, m_buf_req_count_actual
                      ));
    }
    else {

        MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[%s] ln=%d FAIL  : m_buf_vf_decoded_count=%d  Demuxr(%d %d)/(%d %d)  Dec(%d %d) Buf(%d %d) Req(%d %d)/(%d %d)  "),  __func__, __LINE__, m_buf_vf_decoded_count,
                      m_demuxer_pic_width,  m_demuxer_pic_height, demuxer_pic_width,  demuxer_pic_height, 
                      m_decoder_pic_width, m_decoder_pic_height, 
                      m_decoder_buf_width, m_decoder_buf_height,
                      m_buf_req_count_min, m_buf_req_count_actual, buf_req_cnt_min, buf_req_cnt_actual
                      ));
    }

    m_call_count_init_buf_vf_from_demuxer++;

    return mmpResult;
}


MMP_RESULT CMmpDecoderVideo::init_buf_vf_from_decoder(MMP_S32 demuxer_pic_width, MMP_S32 demuxer_pic_height, enum MMP_FOURCC fourcc_out, MMP_S32 buf_req_cnt_min, MMP_S32 buf_req_cnt_actual) {

    MMP_S32 i;
    MMP_BOOL bflag = MMP_TRUE;
    MMP_RESULT mmpResult = MMP_FAILURE;
    enum MMP_FOURCC fourcc;

    fourcc = this->get_fourcc_out();

    if(  (demuxer_pic_width != m_demuxer_pic_width)
        || (demuxer_pic_height != m_demuxer_pic_height)
        || ( fourcc != fourcc_out)
        || (buf_req_cnt_min != m_buf_req_count_min)
        || (buf_req_cnt_actual != m_buf_req_count_actual)
        ) {
        /* Nothing to do */
    }
    else {
        bflag &= MMP_FALSE;
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s] ln=%d Error  if(  (demuxer_pic_width != m_demuxer_pic_width)...."),  __func__, __LINE__));
    }

    if(buf_req_cnt_actual < buf_req_cnt_min) {
        bflag &= MMP_FALSE;
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s] ln=%d Error if(buf_req_cnt_actual(%d) < buf_req_cnt_min(%d) )"),  __func__, __LINE__, buf_req_cnt_actual, buf_req_cnt_min ));
    }

    if(demuxer_pic_width < MMP_VIDEO_DEC_MIN_WIDTH) {
        bflag &= MMP_FALSE;
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s] ln=%d Error if(demuxer_pic_width(%d) < MMP_VIDEO_MIN_WIDTH(%d) )"),  __func__, __LINE__, demuxer_pic_width, MMP_VIDEO_DEC_MIN_WIDTH));
    }

    if(demuxer_pic_height < MMP_VIDEO_DEC_MIN_HEIGHT) {
        bflag &= MMP_FALSE;
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s] ln=%d Error if(demuxer_pic_height(%d) < MMP_VIDEO_MIN_HEIGHT(%d) )"),  __func__, __LINE__, demuxer_pic_height, MMP_VIDEO_DEC_MIN_HEIGHT));
    }

    if(bflag == MMP_TRUE)  {
        if(CMmpDecoder::set_fourcc_out(fourcc_out) != MMP_SUCCESS) {
            bflag &= MMP_FALSE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s] ln=%d Error fourcc_out=%c%c%c%c "),  __func__, __LINE__, MMPGETFOURCCARG(fourcc_out) ));
        }
    }

    if(bflag == MMP_TRUE)  {
        
        m_demuxer_pic_width = demuxer_pic_width;
        m_decoder_pic_width = m_demuxer_pic_width;
        m_decoder_buf_width = mmp_buffer_videoframe::s_get_stride(fourcc_out, m_demuxer_pic_width, MMP_BUFTYPE_HW);


        m_demuxer_pic_height = demuxer_pic_height;
        m_decoder_pic_height = m_demuxer_pic_height;
        m_decoder_buf_height = mmp_buffer_videoframe::s_get_height(fourcc_out, m_demuxer_pic_height, MMP_BUFTYPE_HW);
        
        m_buf_req_count_min = buf_req_cnt_min;
        m_buf_req_count_actual = buf_req_cnt_actual;
        m_buf_vf_index = 0;
        m_buf_vf_decoded_count = 0;
        for(i = 0; i < DECODED_BUF_MAX_COUNT; i++) {
            m_p_buf_vf_decoded[i] = NULL;
        }

        mmpResult = MMP_SUCCESS;
    }

    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[%s] ln=%d m_buf_vf_decoded_count=%d mmpResult=%d"),  __func__, __LINE__, m_buf_vf_decoded_count, mmpResult));

    return mmpResult;
}

MMP_RESULT CMmpDecoderVideo::use_buf_vf_from_renderer(class mmp_buffer_videoframe* p_buf_vf) {

    MMP_S32 i;
    MMP_S32 err_cnt = 0;
    MMP_RESULT mmpResult = MMP_FAILURE;

    if(p_buf_vf != NULL) {

        if(p_buf_vf->get_pic_width() != m_decoder_pic_width) {
            err_cnt++;
        }
        if(p_buf_vf->get_pic_height() != m_decoder_pic_height) {
            err_cnt++;
        }

        if(err_cnt == 0) {

            /* check if already use buf */
            for(i = 0; i < DECODED_BUF_MAX_COUNT; i++) {
                if(m_p_buf_vf_decoded[i] == p_buf_vf) {
                    break;
                }
            }

            if(i == DECODED_BUF_MAX_COUNT) {
                for(i = 0; i < DECODED_BUF_MAX_COUNT; i++) {

                    if(m_p_buf_vf_decoded[i] == NULL) {
                        m_p_buf_vf_decoded[i] = p_buf_vf;
                        m_buf_vf_decoded_count = i+1;
                        mmpResult = MMP_SUCCESS;
                        break;
                    }
                }
            }

        }

       // MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[%s] ln=%d m_buf_vf_decoded_count=(%d %d) (%d %d)  "),  __func__, __LINE__, 
       //    p_buf_vf->get_pic_width(), p_buf_vf->get_pic_height(),
       //    m_decoder_pic_width, m_decoder_pic_height
       //    ));
    }
    
    //MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[%s] ln=%d m_buf_vf_decoded_count=%d "),  __func__, __LINE__, m_buf_vf_decoded_count));

    return mmpResult;
}

MMP_RESULT CMmpDecoderVideo::free_buf_vf_from_renderer(class mmp_buffer_videoframe* p_buf_vf) {

    MMP_S32 i, j;
    MMP_S32 err_cnt = 0;
    MMP_RESULT mmpResult = MMP_FAILURE;

    if(p_buf_vf != NULL) {

        if(p_buf_vf->get_pic_width() != m_decoder_pic_width) {
            err_cnt++;
        }
        if(p_buf_vf->get_pic_height() != m_decoder_pic_height) {
            err_cnt++;
        }

        if(err_cnt == 0) {
                        
            for(i = 0; i < DECODED_BUF_MAX_COUNT; i++) {

                if(m_p_buf_vf_decoded[i] == p_buf_vf) {

                    m_p_buf_vf_decoded[i] = NULL;

                    /* 중간 배열의 버퍼를 지웠으므로, 뒤 버퍼를 앞으로 한칸씩 당긴다. */
                    for(j = i; j < m_buf_vf_decoded_count; j++) {
                        m_p_buf_vf_decoded[j] = m_p_buf_vf_decoded[j+1];
                    }
                    m_buf_vf_decoded_count--;
                    break;
                }
            }

            mmpResult = MMP_SUCCESS;
        }
    }
    
    return mmpResult;
}

MMP_S32 CMmpDecoderVideo::get_buf_vf_count_own(MMP_MEDIA_ID id) {

    MMP_S32 idx, cnt = 0;
    class mmp_buffer_videoframe* p_buf_vf = NULL;
        
    for(idx = 0; idx < m_buf_vf_decoded_count; idx++) {
        p_buf_vf = m_p_buf_vf_decoded[idx];
        if(p_buf_vf != NULL) {
            if(p_buf_vf->is_own(id) == MMP_TRUE) {
                cnt++;
            }
        }
    }
    
    return cnt;
}

class mmp_buffer_videoframe* CMmpDecoderVideo::get_buf_vf(MMP_S32 id) {

    class mmp_buffer_videoframe* p_buf_vf = NULL;
    
    if( (id >= 0) && (id < m_buf_vf_decoded_count) ) {
        p_buf_vf = m_p_buf_vf_decoded[id];
    }

    return p_buf_vf;
}

MMP_BOOL CMmpDecoderVideo::is_next_vf() {

    MMP_BOOL bFlag = MMP_FALSE;
    MMP_S32 idx;
    class mmp_buffer_videoframe* p_buf_vf = NULL;
    class mmp_buffer_videoframe* p_buf_vf_ret = NULL;

    for(idx = 0; idx < m_buf_vf_decoded_count; idx++) {
        p_buf_vf = m_p_buf_vf_decoded[idx];
        if(p_buf_vf != NULL) {
            if(p_buf_vf->is_own((MMP_MEDIA_ID)this) == MMP_TRUE) {
                bFlag = MMP_TRUE;
                break;
            }
        }
    }
    
    return bFlag;
}

class mmp_buffer_videoframe* CMmpDecoderVideo::get_next_vf(MMP_BOOL auto_update_id) {

#if 0

    class mmp_buffer_videoframe* p_buf_vf = NULL;
    class mmp_lock autolock(m_p_mutex_buffer);

    if(m_buf_vf_decoded_count > 0) {
        if( (m_buf_vf_index >= 0) && (m_buf_vf_index < m_buf_vf_decoded_count) ) {
            p_buf_vf = m_p_buf_vf_decoded[m_buf_vf_index];
            if(auto_update_id == MMP_TRUE) {
                m_buf_vf_index = (m_buf_vf_index+1)%m_buf_vf_decoded_count;
            }
        }
    }

    return p_buf_vf;

#else
    MMP_S32 idx;
    class mmp_buffer_videoframe* p_buf_vf = NULL;
    class mmp_buffer_videoframe* p_buf_vf_ret = NULL;

    for(idx = 0; idx < m_buf_vf_decoded_count; idx++) {
        p_buf_vf = m_p_buf_vf_decoded[idx];
        if(p_buf_vf != NULL) {
            if(p_buf_vf->is_own((MMP_MEDIA_ID)this) == MMP_TRUE) {
                p_buf_vf_ret = p_buf_vf;
                break;
            }
        }
    }
    
    return p_buf_vf_ret;
#endif
}

MMP_RESULT CMmpDecoderVideo::flush_buffer_out() { 

    MMP_S32 idx;
    class mmp_buffer_videoframe* p_buf_vf = NULL;
        
    for(idx = 0; idx < DECODED_BUF_MAX_COUNT; idx++) {
        p_buf_vf = m_p_buf_vf_decoded[idx];
        if(p_buf_vf != NULL) {
            p_buf_vf->clear_own();
        }
    }
    
    return MMP_SUCCESS;
}

void CMmpDecoderVideo::print_status(class mmp_buffer_videoframe* p_buf_vf, int au_size) {

    MMP_U32 before_tick = m_mon_before_tick, fps_sum=m_mon_fps_sum, coding_dur_sum=m_mon_coding_dur_sum, postproc_dur_sum=m_mon_postproc_dur_sum;
    MMP_U32 au_sum_size = m_mon_au_sum_size;
    MMP_U32 au_sum_size_total = m_mon_au_sum_size_total;
    MMP_U32 fps_fail_sum = m_mon_fps_fail_sum;
    MMP_U32 start_tick = m_nClassStartTick, cur_tick;
    MMP_U32 coding_dur_avg = 0, postproc_dur_avg=0;
    //struct mmp_system_meminfo meminfo;

    if(p_buf_vf != NULL) {

        if(p_buf_vf->get_coding_result() != mmp_buffer_media::SUCCESS) 
		  fps_fail_sum++;
		  
        fps_sum ++;

        coding_dur_sum += p_buf_vf->get_coding_dur();
        postproc_dur_sum += p_buf_vf->get_postproc_dur();

        m_nTotalDecFrameCount++;
        m_nTotalDecDur += p_buf_vf->get_coding_dur();

        au_sum_size += au_size;
        au_sum_size_total += au_size;

    }
    else {

    }
 
    cur_tick = CMmpUtil::GetTickCount();
    if( (cur_tick - before_tick) > 1000 ) {
        
        if(fps_sum != 0) {
            coding_dur_avg = coding_dur_sum/fps_sum;
            postproc_dur_avg = postproc_dur_sum/fps_sum;
        }

        //CMmpUtil::system_meminfo(&meminfo);

        MMP_U32 cur_sec;
            
        cur_sec = (cur_tick-start_tick)/1000;
        
        MMPDEBUGMSG(1, (TEXT("[MME-VDec %s %s(%s) %dx%d %dx%d] %d. fps=(%d %d) dur=(%d, %d) ausz(%d %d) AndBuf(%d) "), 
                    this->get_class_name(),  
                    this->get_fourcc_in_name(),  this->get_fourcc_out_name(),  
                    m_demuxer_pic_width, m_demuxer_pic_height,
                    m_decoder_buf_width, m_decoder_buf_height,
                    (cur_tick-start_tick)/1000, fps_sum, fps_fail_sum, 
                    coding_dur_avg, postproc_dur_avg,
                    au_sum_size*8, (cur_sec==0)?au_sum_size_total*8:au_sum_size_total*8/cur_sec,
                    m_is_android_buffer 
                    //meminfo.MemFree>>10 /* meminfo.MemFree/1024, */, meminfo.MemTotal>>10 /* meminfo.MemTotal/1024 */
                    ));

        if( (coding_dur_avg + postproc_dur_avg) > 0) {
            m_nDecodingAvgFPS = 1000/(coding_dur_avg + postproc_dur_avg);
        }
        else {
            m_nDecodingAvgFPS = 1000;
        }

        before_tick = cur_tick;
        fps_sum = 0;
        fps_fail_sum = 0;
        coding_dur_sum = 0;
        postproc_dur_sum = 0;
        au_sum_size = 0;
    }

    m_mon_before_tick = before_tick;
    m_mon_fps_sum = fps_sum;
    m_mon_fps_fail_sum = fps_fail_sum;
    m_mon_coding_dur_sum = coding_dur_sum;
    m_mon_postproc_dur_sum = postproc_dur_sum;
    m_mon_au_sum_size = au_sum_size;
    m_mon_au_sum_size_total = au_sum_size_total;
        
}

MMP_RESULT CMmpDecoderVideo::do_csc_sw(class mmp_buffer_videoframe* p_buf_vf_src, class mmp_buffer_videoframe* p_buf_vf_dst) {
    
#if 1
    MMP_U8 *line_src, *line_dst;
    MMP_S32 stride_src, stride_dst, stride;
    MMP_S32 i, h;
    MMP_S32 h_dst;
    enum MMP_FOURCC fourcc_src, fourcc_dst;

    fourcc_src = p_buf_vf_src->get_fourcc();
    fourcc_dst = p_buf_vf_dst->get_fourcc();

    if( (fourcc_src == MMP_FOURCC_IMAGE_YUV420) 
        || (fourcc_src == MMP_FOURCC_IMAGE_YVU420) 
        || (fourcc_dst == MMP_FOURCC_IMAGE_YUV420) 
        || (fourcc_dst == MMP_FOURCC_IMAGE_YVU420) )
    {
        MMP_U8* image_src[3];
        MMP_U8* image_dst[3];

        image_src[0] = (MMP_U8*)p_buf_vf_src->get_buf_vir_addr_y();
        image_src[1] = (MMP_U8*)p_buf_vf_src->get_buf_vir_addr_u();
        image_src[2] = (MMP_U8*)p_buf_vf_src->get_buf_vir_addr_v();
    
        image_dst[0] = (MMP_U8*)p_buf_vf_dst->get_buf_vir_addr_y();
        image_dst[1] = (MMP_U8*)p_buf_vf_dst->get_buf_vir_addr_u();
        image_dst[2] = (MMP_U8*)p_buf_vf_dst->get_buf_vir_addr_v();

        for(i = 0; i < 3; i++) {

            line_src = image_src[i];
            line_dst = image_dst[i];
               
            stride_src = p_buf_vf_src->get_buf_stride(i);
            stride_dst = p_buf_vf_dst->get_buf_stride(i);
            if(stride_src>stride_dst) stride = stride_dst;
            else  stride = stride_src;

            h_dst = p_buf_vf_dst->get_buf_height(i);

            for(h = 0; h < h_dst; h++) {
                memcpy(line_dst, line_src, stride);
                line_src += stride_src;
                line_dst += stride_dst;
            }

        }

    }

#else

    enum MMP_FOURCC fourcc_src, fourcc_dst;

    fourcc_src = p_buf_vf_src->get_fourcc();
    fourcc_dst = p_buf_vf_dst->get_fourcc();

    MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVideo::do_csc_sw] fourcc_src=%c%c%c%c fourcc_dst=%c%c%c%c strideS(%d %d %d) strideD(%d %d %d) hS(%d %d %d) hD(%d %d %d) "), 
                            MMPGETFOURCCARG(fourcc_src), MMPGETFOURCCARG(fourcc_dst),
                            p_buf_vf_src->get_buf_stride(0), p_buf_vf_src->get_buf_stride(1), p_buf_vf_src->get_buf_stride(2),
                            p_buf_vf_dst->get_buf_stride(0), p_buf_vf_dst->get_buf_stride(1), p_buf_vf_dst->get_buf_stride(2), 
                            
                            p_buf_vf_src->get_buf_height(0), p_buf_vf_src->get_buf_height(1), p_buf_vf_src->get_buf_height(2),
                            p_buf_vf_dst->get_buf_height(0), p_buf_vf_dst->get_buf_height(1), p_buf_vf_dst->get_buf_height(2)

                            ));

#endif

    return MMP_SUCCESS;
}
    
