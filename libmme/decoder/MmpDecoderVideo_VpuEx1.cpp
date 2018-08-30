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

#include "MmpDecoderVideo_VpuEx1.hpp"
#include "MmpUtil.hpp"

#define EXTRA_BUFFER_COUNT 1

/////////////////////////////////////////////////////////////
//CMmpDecoderVideo_VpuEx1 Member Functions

static const enum MMP_FOURCC s_support_fourcc_in[]={
     
     MMP_FOURCC_VIDEO_H264,
     
     MMP_FOURCC_VIDEO_MPEG4,
     MMP_FOURCC_VIDEO_MPEG2,
     
     //MMP_FOURCC_VIDEO_WMV1, /* VPU does not support WMV1/2, but MXPlayer can not distinguish between WMV1/2 and WMV3. */
     //MMP_FOURCC_VIDEO_WMV2, /* Therefore, VPU must return ERROR when decode WMV1/2. */
     MMP_FOURCC_VIDEO_WMV3, /* VPU support only WMV3 SP/MP */
     MMP_FOURCC_VIDEO_WVC1, /* WMV3 Advanced Profile */
     
     MMP_FOURCC_VIDEO_RV30,
     MMP_FOURCC_VIDEO_RV40,
     MMP_FOURCC_VIDEO_RV,

     MMP_FOURCC_VIDEO_VP80,

     MMP_FOURCC_VIDEO_H263,
     
     MMP_FOURCC_VIDEO_FLV1, /* sorenson spark */
     MMP_FOURCC_VIDEO_THEORA,

     MMP_FOURCC_VIDEO_MSMPEG4V3, /* only support msmpeg4v3 */

     /* VPU Support Only WMV3 */
     //MMP_FOURCC_VIDEO_VC1,
     
     
     MMP_FOURCC_VIDEO_FFMPEG,
};

static const enum MMP_FOURCC s_support_fourcc_out[]={
     
     MMP_FOURCC_IMAGE_YUV420,
     //MMP_FOURCC_IMAGE_YUV420M,
     //MMP_FOURCC_IMAGE_YVU420,
     //MMP_FOURCC_IMAGE_YVU420M
};

class mmp_vpu_lock {

private:
	class mmp_vpu_if *m_p_vpu_if;
	
public:
	mmp_vpu_lock(class mmp_vpu_if *p_vpu_if) : m_p_vpu_if(p_vpu_if)
    {
        p_vpu_if->get_external_mutex()->lock();
        p_vpu_if->clk_enable();
    }

	~mmp_vpu_lock() {
        m_p_vpu_if->clk_disable();
        m_p_vpu_if->get_external_mutex()->unlock();
    }
};


CMmpDecoderVideo_VpuEx1::CMmpDecoderVideo_VpuEx1(struct CMmpDecoderVideo::create_config *p_create_config) : 

    CMmpDecoderVideo(p_create_config, "VPUEx1", 
                      s_support_fourcc_in, sizeof(s_support_fourcc_in)/sizeof(s_support_fourcc_in[0]),
                     s_support_fourcc_out, sizeof(s_support_fourcc_out)/sizeof(s_support_fourcc_out[0]))
    ,CLASS_DECODER_VPU(p_create_config->fourcc_in)

,m_bOpenVPU(MMP_FALSE)
,m_bDecodeDSI(MMP_FALSE)
,m_last_pts(-1)
,m_decode_dsi_fail_count(0)
{
    
}

CMmpDecoderVideo_VpuEx1::~CMmpDecoderVideo_VpuEx1()
{

}

MMP_RESULT CMmpDecoderVideo_VpuEx1::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;
    enum MMP_FOURCC fourcc_in = this->get_fourcc_in();

    MMPDEBUGMSG(m_DEBUG_MMEDec, (TEXT("[%s] ln=%d this=0x%08x ++++"), __func__, __LINE__, this ));

    /* Dec initially check */
    if(mmpResult == MMP_SUCCESS) {
        mmpResult=CMmpDecoderVideo::Open();
        if(mmpResult != MMP_SUCCESS) {
              MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_VpuEx1::Open] FAIL : CMmpDecoderVideo::Open()  fourcc_in=%c%c%c%c"), MMPGETFOURCCARG(fourcc_in) ));
        }
    }

    /* vpu_if 생성하고, streaming_ref_count를 체크한다.  ref_count가 Full이면 Error 리턴*/
    if(mmpResult == MMP_SUCCESS) {
        m_p_vpu_if = mmp_vpu_if::create_object();
        if(m_p_vpu_if == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_VpuEx1::Open] FAIL : mmp_vpu_if::create_object ")));
        }
        else {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("VPU Streaming Reference Count : %d "), m_p_vpu_if->streaming_ref_count() ));
            if(m_p_vpu_if->streaming_is_full() == MMP_TRUE) {
                mmpResult = MMP_FAILURE;
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("ERROR: VPU Streaming Reference Count is Full !! ")));
            }
        }
    }

    if(m_p_vpu_if!=NULL)   m_p_vpu_if->get_external_mutex()->lock();

    /* Streaming Start / PM-Domain On  */
    if(mmpResult == MMP_SUCCESS) {
        mmpResult = m_p_vpu_if->streaming_start((void*)this);
        if(mmpResult != MMP_SUCCESS) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_VpuEx1::Open] FAIL: VPU Streaming Start !! ")));
        }
    }
    
    /* VPU Open */   
    if(mmpResult == MMP_SUCCESS) {
        
        m_p_vpu_if->clk_enable();

        mmpResult=CLASS_DECODER_VPU::Open(m_p_vpu_if);
        if(mmpResult!=MMP_SUCCESS) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_VpuEx1::Open] FAIL: CLASS_DECODER_VPU::Open() ")));
        }

        m_p_vpu_if->clk_disable();
    }

    if(m_p_vpu_if!=NULL)   m_p_vpu_if->get_external_mutex()->unlock();
            
    MMPDEBUGMSG(m_DEBUG_MMEDec, (TEXT("[%s] ln=%d this=0x%08x ----"), __func__, __LINE__, this ));

    return mmpResult;
}


MMP_RESULT CMmpDecoderVideo_VpuEx1::Close()
{
    MMP_RESULT mmpResult;
    
    MMPDEBUGMSG(m_DEBUG_MMEDec, (TEXT("[%s] ln=%d this=0x%08x ++++"), __func__, __LINE__, this ));

    if(m_p_vpu_if != NULL) {
        m_p_vpu_if->get_external_mutex()->lock();

        m_p_vpu_if->clk_enable();
        CLASS_DECODER_VPU::Close(m_p_vpu_if);
        m_p_vpu_if->clk_disable();
        
        m_p_vpu_if->streaming_stop((void*)this);
        
        m_p_vpu_if->get_external_mutex()->unlock();

        mmp_vpu_if::destroy_object(m_p_vpu_if);
        m_p_vpu_if = NULL;
    }


    mmpResult=CMmpDecoderVideo::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_VpuEx1::Close] CMmpDecoderVideo::Close() \n\r")));
    }

    MMPDEBUGMSG(m_DEBUG_MMEDec, (TEXT("[%s] ln=%d this=0x%08x ----"), __func__, __LINE__, this ));

    return mmpResult;
}

MMP_RESULT CMmpDecoderVideo_VpuEx1::flush_buffer_in() {

    MMP_S64 pts;
    class mmp_vpu_lock autolock(m_p_vpu_if);

    MMPDEBUGMSG(m_DEBUG_MMEDec, (TEXT("[%s] ln=%d this=0x%08x ++++"), __func__, __LINE__, this ));

    while(!m_minheap_ts.IsEmpty()) {
        m_minheap_ts.Delete(pts);
    }

    m_last_pts = -1;

    //MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_VpuEx1::flush_buffer_in] ")));

    MMPDEBUGMSG(m_DEBUG_MMEDec, (TEXT("[%s] ln=%d this=0x%08x ----"), __func__, __LINE__, this ));

    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderVideo_VpuEx1::flush_buffer_out() {

    class mmp_vpu_lock autolock(m_p_vpu_if);

    MMPDEBUGMSG(m_DEBUG_MMEDec, (TEXT("[%s] ln=%d this=0x%08x ++++"), __func__, __LINE__, this ));

    CMmpDecoderVideo::flush_buffer_out();
    CLASS_DECODER_VPU::flush_buffer_out(m_p_vpu_if);
    
    MMPDEBUGMSG(m_DEBUG_MMEDec, (TEXT("[%s] ln=%d this=0x%08x ----"), __func__, __LINE__, this ));

    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderVideo_VpuEx1::clear_display_flag(class mmp_buffer_videoframe* p_buf_vf) {

    class mmp_vpu_lock autolock(m_p_vpu_if);
    MMP_RESULT mmpResult;

    MMPDEBUGMSG(m_DEBUG_MMEDec, (TEXT("[%s] ln=%d this=0x%08x ++++"), __func__, __LINE__, this ));

    mmpResult = CMmpDecoderVpuEx1::ClearDisplayFlag(m_p_vpu_if, p_buf_vf);

    MMPDEBUGMSG(m_DEBUG_MMEDec, (TEXT("[%s] ln=%d this=0x%08x ----"), __func__, __LINE__, this ));

    return mmpResult;
}

/*
    When run below CTS, the width/height of demuxer is larger than real pic width/height.

        Real Width/Height=(640, 480)  Test Width/Height =   (640+10, 480 + 10)
        android.media.cts.DecodeAccuracyTest#testH264SurfaceViewLargerHeightVideoDecode     WIDTH=640+10
        android.media.cts.DecodeAccuracyTest#testH264SurfaceViewLargerWidthVideoDecode      HEIGHT=480+10

    In this case, I notify to ACodec,  OUTPUT_CROP_CHANGED, 
        see omx_comp_vdec, this->callback_notify(OMX_EventPortSettingsChanged, OMX_DirOutput 1 , OMX_IndexConfigCommonOutputCrop, NULL);
    When ACodec recive this OMX_EventPortSettingsChanged(data2=OMX_IndexConfigCommonOutputCrop), it will tool new Crop-Rect Info.
*/
struct mmp_rect CMmpDecoderVideo_VpuEx1::get_display_crop_rect() {
    
    struct mmp_rect display_crop_rect;
    MMP_S32 vpu_pic_width, vpu_pic_height;
        
    
    display_crop_rect = CMmpDecoderVideo::get_display_crop_rect();

    if(m_bDecodeDSI == MMP_TRUE) {

        vpu_pic_width = CLASS_DECODER_VPU::GetVpuPicWidth();
        vpu_pic_height = CLASS_DECODER_VPU::GetVpuPicHeight();

        if(display_crop_rect.right > (vpu_pic_width-1) ) {
            display_crop_rect.right = (vpu_pic_width-1);
        }

        if(display_crop_rect.bottom > (vpu_pic_height-1) ) {
            display_crop_rect.bottom = (vpu_pic_height-1);
        }
    }
    
    return  display_crop_rect;
}

MMP_BOOL CMmpDecoderVideo_VpuEx1::check_reconfig(MMP_BOOL is_msg, MMP_CHAR* msg_name, enum ACTION* next_actoin) {

    MMP_BOOL bflag = MMP_FALSE;
    
    MMP_S32 demuxer_pic_width, demuxer_pic_height;
    MMP_S32 decoder_buf_width, decoder_buf_height;
    MMP_S32 vpu_pic_width, vpu_pic_height;
    MMP_S32 vpu_buf_width, vpu_buf_height;

    MMP_S32 actual_buf_cnt;
    MMP_S32 min_buf_cnt, vpu_min_buf_cnt;

    MMP_BOOL is_android_buf = this->is_android_buffer();

    demuxer_pic_width = this->get_demuxer_pic_width();
    demuxer_pic_height = this->get_demuxer_pic_height();
    decoder_buf_width = this->get_decoder_buf_width();
    decoder_buf_height = this->get_decoder_buf_height();
    vpu_pic_width = CLASS_DECODER_VPU::GetVpuPicWidth();
    vpu_pic_height = CLASS_DECODER_VPU::GetVpuPicHeight();
    vpu_buf_width = CLASS_DECODER_VPU::GetVpuBufWidth();
    vpu_buf_height = CLASS_DECODER_VPU::GetVpuBufHeight();
            
    min_buf_cnt = this->get_buf_req_count_min();
    actual_buf_cnt = this->get_buf_req_count_actual();
    vpu_min_buf_cnt = CLASS_DECODER_VPU::GetVpuMinFrameBufferCount();

    if(next_actoin) *next_actoin = ACTION_NONE;

    MMPDEBUGMSG(is_msg, (TEXT("[CMmpDecoderVideo_VpuEx1::check_reconfig] %s Demuxer(W:%d H:%d) Decoder(W:%d H:%d Bufc:%d) VPU(W:%d H:%d) VPUBuf(W:%d H:%d BufC:%d) AndBuf(%d) "),  
                msg_name, 
                demuxer_pic_width, demuxer_pic_height,
                decoder_buf_width, decoder_buf_height, min_buf_cnt,
                vpu_pic_width, vpu_pic_height, 
                vpu_buf_width, vpu_buf_height, vpu_min_buf_cnt,
                is_android_buf
        ));

    /*
        When run below CTS, the width/height of demuxer is larger than real pic width/height.

           Real Width/Height=(640, 480)  Test Width/Height =   (640+10, 480 + 10)
           android.media.cts.DecodeAccuracyTest#testH264SurfaceViewLargerHeightVideoDecode     WIDTH=640+10
           android.media.cts.DecodeAccuracyTest#testH264SurfaceViewLargerWidthVideoDecode      HEIGHT=480+10

        In this case, I just regarded this as RECONCIG STATUS, but it made sometimes CTS-FAIL with buffer Crash!!.
        So I replaced it to OUTPUT_CROP_CHANGED  to solve this problem.

    */
    m_decoder_reconfig_width = vpu_buf_width;//CLASS_DECODER_VPU::GetVpuReconfigWidth();
    m_decoder_reconfig_height = vpu_buf_height;//CLASS_DECODER_VPU::GetVpuReconfigHeight();
    if(min_buf_cnt < vpu_min_buf_cnt) {
        m_decoder_reconfig_min_buf_count = vpu_min_buf_cnt;
        m_decoder_reconfig_actual_buf_count = m_decoder_reconfig_min_buf_count+4;
    }
    else {
        /*
            In case omx_min_buf_cnt is larger than vpu_min_buf_cnt, it need not to change buf_count.
            If you make buff_cnt less, it fait to test below CTS
                android.media.cts.AdaptivePlaybackTest#testVP8_adaptiveDr
                android.media.cts.AdaptivePlaybackTest#testVP8_adaptiveSmallDrc
        */
        m_decoder_reconfig_min_buf_count = min_buf_cnt;
        m_decoder_reconfig_actual_buf_count = actual_buf_cnt;
    }
    
    /************************************************************************************
        check width/height
    *************************************************************************************/
    if( (decoder_buf_width == vpu_buf_width ) && (decoder_buf_height == vpu_buf_height) )  {
        /* Nothing to do */
    }
    else if( (decoder_buf_width >= vpu_buf_width )&&(decoder_buf_height >= vpu_buf_height) ) {
        
        bflag = MMP_TRUE;
        if(next_actoin) *next_actoin = ACTION_OUTPUT_CROP_CHANGED;

        MMPDEBUGMSG(is_msg, (TEXT("[CMmpDecoderVideo_VpuEx1::check_reconfig] %s OutputCropChanged decoder_buf(%d %d) vpu_buf(%d %d) "), 
                        msg_name, 
                        decoder_buf_width, decoder_buf_height,
                        vpu_buf_width, vpu_buf_height
                        ));
    }
    else  {
        bflag = MMP_TRUE;
        if(next_actoin) *next_actoin = ACTION_RECONFIG;

        MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVideo_VpuEx1::check_reconfig] %s Reconfig=TRUE reconfig_width=%d reconfig_height=%d "),  
                msg_name, 
                m_decoder_reconfig_width, m_decoder_reconfig_height
        ));
    }

    /************************************************************************************
        check min buf cnt
    *************************************************************************************/
    if(min_buf_cnt < vpu_min_buf_cnt) {
        bflag = MMP_TRUE;
        if(next_actoin) *next_actoin = ACTION_RECONFIG;
        MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVideo_VpuEx1::check_reconfig] %s Reconfig=TRUE min_buf_cnt=%d actual_buf_cnt=%d "),  
                msg_name, 
                m_decoder_reconfig_min_buf_count, m_decoder_reconfig_actual_buf_count
        ));
    }

    return bflag;
}

MMP_RESULT CMmpDecoderVideo_VpuEx1::DecodeAu(class mmp_buffer_videostream* p_buf_vs, class mmp_buffer_videoframe** pp_buf_vf, enum ACTION *next_action) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U32 dec_start_tick, dec_end_tick;
    MMP_U32 postproc_start_tick, postproc_end_tick;
    class mmp_buffer_videoframe* p_buf_vf_vpu = NULL;
    class mmp_buffer_videoframe* p_buf_vf_decoded = NULL;
    MMP_BOOL is_android_buf = this->is_android_buffer();
    MMP_S64 pts;
    MMP_FOURCC fourcc_in, fourcc_out;
    
    MMP_S32 demuxer_pic_width, demuxer_pic_height;
    MMP_S32 actual_buf_cnt, vpu_real_buf_cnt;
    MMP_U32 vpu_result = 0;
    enum ACTION action;
        
    class mmp_vpu_lock autolock(m_p_vpu_if);

    MMPDEBUGMSG(m_DEBUG_MMEDec, (TEXT("[%s] ln=%d this=0x%08x ++++"), __func__, __LINE__, this ));

    fourcc_out = this->get_fourcc_out();
    
        
     /* Init Parmeter */
    if(pp_buf_vf != NULL) {
        *pp_buf_vf = NULL;
    }
    
    *next_action = ACTION_NONE;
           
    if(m_bDecodeDSI == MMP_FALSE) {

        demuxer_pic_width = this->get_demuxer_pic_width();
        demuxer_pic_height = this->get_demuxer_pic_height();

        p_buf_vs->set_pic_width(demuxer_pic_width);
        p_buf_vs->set_pic_height(demuxer_pic_height);

        
        mmpResult = CLASS_DECODER_VPU::DecodeDSI(m_p_vpu_if, p_buf_vs);
        if(mmpResult == MMP_SUCCESS) {
            
            m_bDecodeDSI = MMP_TRUE;

            /* Reconfig 조건이면, init_buf_vf_from_decoder를 수행하고 결과 리턴 */
            if(this->check_reconfig(MMP_TRUE, "check reconfig", &action) == MMP_TRUE) {
            
                if(action == ACTION_RECONFIG) {
                    *next_action = ACTION_RECONFIG;
                    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_VpuEx1::DecodeAu] occur reconfig ")));
                    
                    MMPDEBUGMSG(m_DEBUG_MMEDec, (TEXT("[%s] ln=%d this=0x%08x ----"), __func__, __LINE__, this ));

                    p_buf_vs->inc_stream_offset(p_buf_vs->get_stream_real_size());
                    return mmpResult;
                }
                else if(action == ACTION_OUTPUT_CROP_CHANGED) {
                    *next_action = action;
                }
            }

            if( (this->get_fourcc_in()==MMP_FOURCC_VIDEO_WMV3) 
              || (this->get_fourcc_in()==MMP_FOURCC_VIDEO_WVC1)) {
                  this->set_fourcc_in(CLASS_DECODER_VPU::GetFourccIn());
            }

            /*
                2016,8,29  WMV3는 ConfigData인 경우, DSI Return해야 한다. 
                           이 코드가 빠지면  Gallery Thumbnail이 비정상적으로 나옴 
                
                       VC1_002_1920x1080_WMV3_Mp_10Mbps_WMA2_44.1Khz_2ch_98kbps-(Mv) Miss A - Bad Girl Good Girl.wmv 참고
            */
            if( (p_buf_vs->get_flag()&mmp_buffer_media::FLAG_CONFIGDATA) 
                && (this->get_fourcc_in()==MMP_FOURCC_VIDEO_WMV3) 
                ) {

               MMPDEBUGMSG(m_DEBUG_MMEDec, (TEXT("[%s] ln=%d this=0x%08x ----"), __func__, __LINE__, this ));
               p_buf_vs->inc_stream_offset(p_buf_vs->get_stream_real_size());

               return MMP_SUCCESS;
            }

        }
        else if(mmpResult==MMP_ERROR_NOT_SUPPORT) {
            *next_action = ACTION_FAIL_DECODE_DSI;
             MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_VpuEx1::DecodeAu] Fail to DecodeDSI(Not Support), Notify DecodeDSI Err ")));
             MMPDEBUGMSG(m_DEBUG_MMEDec, (TEXT("[%s] ln=%d this=0x%08x ----"), __func__, __LINE__, this ));
             p_buf_vs->inc_stream_offset(p_buf_vs->get_stream_real_size());
             return mmpResult;
        }
        else {
            m_decode_dsi_fail_count++;
            if(m_decode_dsi_fail_count > 30) {
                *next_action = ACTION_FAIL_DECODE_DSI;
                MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_VpuEx1::DecodeAu] Fail to DecodeDSI, Notify DecodeDSI Err ")));
            }

            MMPDEBUGMSG(m_DEBUG_MMEDec, (TEXT("[%s] ln=%d this=0x%08x ----"), __func__, __LINE__, this ));
            p_buf_vs->inc_stream_offset(p_buf_vs->get_stream_real_size());
            return mmpResult;
        }
    }
     
    if((p_buf_vs->get_flag()&mmp_buffer_media::FLAG_RECONFIGDATA) != 0) {
        MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_VpuEx1::DecodeAu] Reconfig Data, return here ")));
        return MMP_SUCCESS;
    }
    /*  
      2016,8,29  이 코드가 왜 여기에 있는지 모르겠음...??? 
      2016,8,31  ConfigData로 설정되면 ffmepg의 extra_data 이므로 decoding할 필요 없다. 
                 DecodeDSI를 통해 Reconfig가 일어났는지 체크해본다. 
      2016,9,5  VLCPlayer는  H264 SPS/PPS를 분리해서 ConfigData Flag설정해서 보낸다. 
                ConfigData라고 해서 막으면 문제 발생 

      2016,9,5   AVC Data인 경우 ConfigData를 Decoder로 전달하면 안된다.  
    */
    if((p_buf_vs->get_flag()&mmp_buffer_media::FLAG_CONFIGDATA) != 0) {
        
        /* Reconfig 조건이면, init_buf_vf_from_decoder를 수행하고 결과 리턴 */
        
        if(this->check_reconfig(MMP_TRUE, "configdata", &action) == MMP_TRUE) {

            if(action == ACTION_RECONFIG) {
                *next_action = ACTION_RECONFIG;
                p_buf_vs->inc_stream_offset(p_buf_vs->get_stream_real_size());

                MMPDEBUGMSG(m_DEBUG_MMEDec, (TEXT("[%s] ln=%d this=0x%08x ----"), __func__, __LINE__, this ));
                return MMP_SUCCESS;
            }
        }

        fourcc_in = this->get_fourcc_in();
        switch(fourcc_in) {
            case MMP_FOURCC_VIDEO_H264:
                if(CMmpDecoderVpuEx1::m_is_avc) {
                    p_buf_vs->inc_stream_offset(p_buf_vs->get_stream_real_size());

                    MMPDEBUGMSG(m_DEBUG_MMEDec, (TEXT("[%s] ln=%d this=0x%08x H264(avc) Config Data return here ----"), __func__, __LINE__, this ));
                    return MMP_SUCCESS;
                }
                break;
#if 0
            case MMP_FOURCC_VIDEO_MPEG4:
            /*
                2016,12,26 comment by hthwang
                     CTS-media DecoderTest.testCodecBasicMpeg4
                       If you decode ConfigData hear, TimeStamp save to 'ts_heap'. 
                       So App wil receive TimeStamp ZERO(0) twice.
                       CTS-media DecoderTest.testCodecBasicMpeg4 will regard this as FAIL!!.
                2017,6,23
                       If you decode ConfigData hear, just I will decode this au.
                       But I will not add TimeStame to 'ts_heap'.
            */
                p_buf_vs->inc_stream_offset(p_buf_vs->get_stream_real_size());
                MMPDEBUGMSG(m_DEBUG_MMEDec, (TEXT("[%s] ln=%d this=0x%08x Mpeg4 CofingData, return here ----"), __func__, __LINE__, this ));
                return MMP_SUCCESS;
#endif
        }
    }
    
    actual_buf_cnt = this->get_buf_vf_count();
    vpu_real_buf_cnt = CLASS_DECODER_VPU::GetVpuRealFrameBufferCount();

    if(actual_buf_cnt == 0) {
         MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_VpuEx1::DecodeAu] ERROR : actual_buf_cnt=%d  vpu_real_buf_cnt=%d "),  
                  actual_buf_cnt, vpu_real_buf_cnt ));
	}
      
    if(is_android_buf == MMP_TRUE) {
        if(actual_buf_cnt != vpu_real_buf_cnt )  {
            mmpResult = CLASS_DECODER_VPU::SetVpuFrameBuffer(m_p_vpu_if, this->get_buf_vf_array(), actual_buf_cnt );
            if(mmpResult != MMP_SUCCESS) {
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_VpuEx1::DecodeAu] FAIL : CLASS_DECODER_VPU::SetVpuFrameBuffer  actual_buf_cnt=%d  vpu_real_buf_cnt=%d "),  
                  actual_buf_cnt, vpu_real_buf_cnt ));

                MMPDEBUGMSG(m_DEBUG_MMEDec, (TEXT("[%s] ln=%d this=0x%08x ----"), __func__, __LINE__, this ));

                *next_action = ACTION_FAIL_DECODE_SetVpuFrameBuffer;

                return mmpResult;
            }
        }
    }
    else if(vpu_real_buf_cnt == 0) {
        mmpResult = CLASS_DECODER_VPU::AllocVpuFrameBuffer(m_p_vpu_if); 
        if(mmpResult != MMP_SUCCESS) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_VpuEx1::DecodeAu] FAIL : CLASS_DECODER_VPU::AllocVpuFrameBuffer  actual_buf_cnt=%d  vpu_real_buf_cnt=%d "),  
                  actual_buf_cnt, vpu_real_buf_cnt ));

            MMPDEBUGMSG(m_DEBUG_MMEDec, (TEXT("[%s] ln=%d this=0x%08x ----"), __func__, __LINE__, this ));

            *next_action = ACTION_FAIL_DECODE_AllocVpuFrameBuffer;
            return mmpResult;
        }
    }
            
    dec_start_tick = CMmpUtil::GetTickCount();
    mmpResult = this->DecodeAu_PicEnd_Run(m_p_vpu_if, p_buf_vs);
    if(mmpResult == MMP_SUCCESS) {
        mmpResult = this->DecodeAu_PicEnd_WaitDone(m_p_vpu_if, p_buf_vs, &p_buf_vf_vpu, &vpu_result);
        
        MMP_BOOL is_add_pts = MMP_TRUE;
        enum mmp_buffer_media::FLAG vs_flag = p_buf_vs->get_flag();
                
        if((vs_flag&mmp_buffer_media::FLAG_VIDEO_CHUNK_REUSE) != 0) {
            is_add_pts = MMP_FALSE;
        }
        if((vs_flag&mmp_buffer_media::FLAG_CONFIGDATA) != 0) {  /* 2017,6,23, If configdata, I will not add TimeStamep to 'ts_heap' */
            is_add_pts = MMP_FALSE;
        }
        if((vpu_result&CLASS_DECODER_VPU::H264_SPS_PPS) != 0) {
            is_add_pts = MMP_FALSE;
        }

        MMPDEBUGMSG(m_DEBUG_MMEDec, (TEXT("[%s] ln=%d this=0x%08x vs_flag=0x%x is_add_pts=%d pts=%lld "), __func__, __LINE__, this, vs_flag, is_add_pts, p_buf_vs->get_pts() ));

        if(is_add_pts==MMP_TRUE) {
            if(m_minheap_ts.IsFull()) {
                m_minheap_ts.Delete(pts);
            }
            pts = p_buf_vs->get_pts();
            if(pts >= 0) {
                m_minheap_ts.Add(pts);
            }
        }
    }
    dec_end_tick = CMmpUtil::GetTickCount();
     
    postproc_start_tick = CMmpUtil::GetTickCount();
    /*  check reconfig 
        After decoding, may be changed WIDTH/HEIGHT

        Because of VPU  is HW, BUF_TYPE must be HW.
        In case not-android buffer, take care!!  
            
    */
#if 1
    {
        MMP_RESULT mmpres = MMP_FAILURE;
        if(this->check_reconfig(MMP_FALSE /*is_msg*/, "Check After Dec", &action) == MMP_TRUE) {

            if(action==ACTION_RECONFIG) {
                *next_action = ACTION_RECONFIG;
                MMPDEBUGMSG(1, (TEXT("[%s] ln=%d this=0x%08x  NextAction is RECONFIG (Check After Dec)"), __func__, __LINE__, this ));
            }
        }
    }

#else
    pic_width = this->get_demuxer_pic_width();
    pic_height = this->get_demuxer_pic_height();
    buffer_width = mmp_buffer_videoframe::s_get_stride(fourcc_out, pic_width, MMP_BUFTYPE_HW);
    buffer_height = mmp_buffer_videoframe::s_get_height(fourcc_out, pic_height, MMP_BUFTYPE_HW);
    vpu_width = CLASS_DECODER_VPU::GetVpuPicWidth();
    vpu_height = CLASS_DECODER_VPU::GetVpuPicHeight();
    vpu_width = mmp_buffer_videoframe::s_get_stride(fourcc_out, vpu_width, MMP_BUFTYPE_HW);
    vpu_height = mmp_buffer_videoframe::s_get_height(fourcc_out, vpu_height, MMP_BUFTYPE_HW);

    min_buf_cnt = this->get_buf_req_count_min();
    vpu_min_buf_cnt = CLASS_DECODER_VPU::GetVpuMinFrameBufferCount();

    MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVideo_VpuEx1::DecodeAu] Check After Dec, Pic(W:%d H:%d) Buf(W:%d H:%d Bufc:%d) VPU(W:%d H:%d BufC:%d) AndBuf(%d) "),  
                        pic_width, pic_height,
                        buffer_width, buffer_height,  min_buf_cnt,
                        vpu_width, vpu_height, vpu_min_buf_cnt,
                        is_android_buf
                ));
    if( (buffer_width != vpu_width)  
        ||  (buffer_height != vpu_height)  ) {

        min_buf_cnt = this->get_buf_req_count_min();
        vpu_min_buf_cnt = CLASS_DECODER_VPU::GetVpuMinFrameBufferCount();

        mmpResult = this->cfg_buf_vf(vpu_width, vpu_height, vpu_min_buf_cnt, vpu_min_buf_cnt+EXTRA_BUFFER_COUNT );
        if(mmpResult == MMP_SUCCESS) {
            if(is_reconfig!=NULL) *is_reconfig = MMP_TRUE;

            MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_VpuEx1::DecodeAu] Reconfig in Play!! Pic(W:%d H:%d) Buf(W:%d H:%d Bufc:%d) VPU(W:%d H:%d BufC:%d) AndBuf(%d) "),  
                        pic_width, pic_height,
                        buffer_width, buffer_height,  min_buf_cnt,
                        vpu_width, vpu_height, vpu_min_buf_cnt,
                        is_android_buf
                ));
        }
    }

#endif

    /* copy image to decoded buffer */
    if(p_buf_vf_vpu != NULL)  {
        if(is_android_buf == MMP_TRUE) {
            p_buf_vf_decoded = p_buf_vf_vpu;
        }
        else {
            p_buf_vf_decoded = this->get_next_vf();
            this->do_csc_sw(p_buf_vf_vpu, p_buf_vf_decoded);
        }

        if(m_minheap_ts.IsEmpty()) {
            if(m_last_pts == -1) {
                CMmpDecoderVpuEx1::ClearDisplayFlag(m_p_vpu_if, p_buf_vf_decoded);
                p_buf_vf_decoded->set_own((MMP_MEDIA_ID)this);
                p_buf_vf_decoded = NULL;
            }
            else {
                pts = m_last_pts + 1000*10;
            }
        }
        else {
            m_minheap_ts.Delete(pts);
        }
        m_last_pts = pts;

        //MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_VpuEx1::DecodeAu] delete pts : %d  "), (unsigned int)(pts/1000) ));
        if(p_buf_vf_decoded != NULL) {
            postproc_end_tick = CMmpUtil::GetTickCount();

            p_buf_vf_decoded->set_coding_dur(dec_end_tick - dec_start_tick);
            p_buf_vf_decoded->set_postproc_dur(postproc_end_tick - postproc_start_tick);
            p_buf_vf_decoded->set_pts(pts);
            p_buf_vf_decoded->clear_own();
        }
    }

    /*
        In case of Mpeg4 Packed B-Frame,  VPU don't return any frame.
        Therefore, we must delete a Time-Stamp 
    */
    if( (vpu_result&SKIP_TYPE) != 0) {
    
        if(!m_minheap_ts.IsEmpty())  {
            m_minheap_ts.Delete(pts);
        }
    }

    /*
        If VPU request CHUNK_REUSE, notify to Player
    */
    if( (vpu_result&CHUNK_REUSE) != 0) {
        *next_action = ACTION_CHUNK_REUSE;
    }

    if( (vpu_result&SEQUENCE_CHANGE) != 0) {
        *next_action = ACTION_RECONFIG;
        m_decoder_reconfig_width = CLASS_DECODER_VPU::GetVpuReconfigWidth();
        m_decoder_reconfig_height = CLASS_DECODER_VPU::GetVpuReconfigHeight();

        MMPDEBUGMSG(1, (TEXT("[%s] ln=%d this=0x%08x  NextAction is RECONFIG reconfig_width(%d %d) vpu_pic(%d %d) vpu_buf(%d %d) "), __func__, __LINE__, this, 
                  m_decoder_reconfig_width, m_decoder_reconfig_height,
                  CLASS_DECODER_VPU::GetVpuPicWidth(), CLASS_DECODER_VPU::GetVpuPicHeight(), 
                  CLASS_DECODER_VPU::GetVpuBufWidth(), CLASS_DECODER_VPU::GetVpuBufHeight()
                  ));
    }

    if( (vpu_result&VPU_TIMEOUT) != 0) {
        *next_action = ACTION_FAIL_DECODE_TIMEOUT;
    }

    if(pp_buf_vf != NULL) {
        *pp_buf_vf = p_buf_vf_decoded;
    }

    p_buf_vs->inc_stream_offset(p_buf_vs->get_stream_real_size());
        
    MMPDEBUGMSG(m_DEBUG_MMEDec, (TEXT("[%s] ln=%d this=0x%08x ----"), __func__, __LINE__, this ));

	return MMP_SUCCESS; 
}


MMP_RESULT CMmpDecoderVideo_VpuEx1::DecodeAu(class mmp_buffer_videostream* p_buf_vs, class mmp_buffer_videoframe* p_buf_vf) {

#if 1
    return MMP_FAILURE;
#else
    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U32 dec_start_tick, dec_end_tick;
    class mmp_buffer_videoframe* p_buf_vf_vpu = NULL;
         
    if(p_buf_vf!=NULL) p_buf_vf->set_coding_result(mmp_buffer_media::FAIL);
           
    if(m_bDecodeDSI == MMP_FALSE) {
        mmpResult = CLASS_DECODER_VPU::DecodeDSI(p_buf_vs);
        if(mmpResult == MMP_SUCCESS) {
            m_bDecodeDSI = MMP_TRUE;
            if( (this->get_demuxer_pic_width() != CLASS_DECODER_VPU::GetVpuPicWidth())  
                ||  (this->get_demuxer_pic_height() != CLASS_DECODER_VPU::GetVpuPicHeight()) ) {
		            if(p_buf_vf!=NULL) p_buf_vf->set_coding_result(mmp_buffer_media::RECONFIG);
                    this->cfg_buf_vf(CLASS_DECODER_VPU::GetVpuPicWidth(), CLASS_DECODER_VPU::GetVpuPicHeight());
                    return MMP_SUCCESS;
            }
        }
        else {
            return mmpResult;
        }
    }

    
    if((p_buf_vs->get_flag()&mmp_buffer_media::FLAG_CONFIGDATA) != 0) {
        if(p_buf_vf!=NULL) p_buf_vf->set_coding_result(mmp_buffer_media::RECONFIG);
        return MMP_SUCCESS;
    }

    dec_start_tick = CMmpUtil::GetTickCount();
    mmpResult = this->DecodeAu_PicEnd(p_buf_vs, &p_buf_vf_vpu);
    dec_end_tick = CMmpUtil::GetTickCount();

    /* check reconfig */
    if((this->get_demuxer_pic_width() != CLASS_DECODER_VPU::GetVpuPicWidth())  ||  (this->get_demuxer_pic_height() != CLASS_DECODER_VPU::GetVpuPicHeight())  ) {
        if(p_buf_vf!=NULL) p_buf_vf->set_coding_result(mmp_buffer_media::RECONFIG);
        this->cfg_buf_vf(CLASS_DECODER_VPU::GetVpuPicWidth(), CLASS_DECODER_VPU::GetVpuPicHeight());
        return MMP_SUCCESS;

    }
    /* copy image to decoded buffer */
    else if( (p_buf_vf_vpu != NULL) && (p_buf_vf != NULL) )  {
            
            this->do_csc_sw(p_buf_vf_vpu, p_buf_vf);

            p_buf_vf->set_coding_dur(dec_end_tick - dec_start_tick);
            p_buf_vf->set_pts(p_buf_vs->get_pts());
            p_buf_vf->clear_own();
            p_buf_vf->set_coding_result(mmp_buffer_media::SUCCESS);
    }

    p_buf_vs->inc_stream_offset(p_buf_vs->get_stream_real_size());

	return mmpResult; 
#endif
}