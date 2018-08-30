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

#include "MmpPlayerVideoEx1.hpp"
#include "MmpUtil.hpp"

/////////////////////////////////////////////////////////////
//CMmpPlayerVideoEx1 Member Functions

CMmpPlayerVideoEx1::CMmpPlayerVideoEx1(CMmpPlayerCreateProp* pPlayerProp) : CMmpPlayer(pPlayerProp, "VideoEx1")
,m_pDemuxer(NULL)
,m_pDecoderVideo(NULL)
,m_pRendererVideo(NULL)

,m_buffer_stride(0)
,m_buffer_height(0)

,m_last_packet_pts(0)
,m_fps(0)

,m_p_buf_vs(NULL)

{
    
}

CMmpPlayerVideoEx1::~CMmpPlayerVideoEx1()
{
    
}

MMP_RESULT CMmpPlayerVideoEx1::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;
    struct MmpDemuxerCreateConfig demuxer_create_config;
    struct CMmpDecoderVideo::create_config decoder_create_config;
    struct CMmpRendererVideo::create_config renderer_video_create_config;
    
    MMP_S32 stream_buf_max_size;
    
    MMP_BOOL is_video_dec_reconfig = MMP_FALSE;

    /* create demuxer */
    if(mmpResult == MMP_SUCCESS ) {
        strcpy((char*)demuxer_create_config.filename, this->m_create_config.filename);
        m_pDemuxer = CMmpDemuxer::CreateObject(&demuxer_create_config);
        if(m_pDemuxer == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpPlayerVideoEx1::Open] FAIL:  CMmpDemuxer::CreateObject ")));
        }
        else {
            stream_buf_max_size = m_pDemuxer->GetVideoPicWidth()*m_pDemuxer->GetVideoPicHeight()*3/2;
        }
    }

    /* create stream buffer */
    if(mmpResult == MMP_SUCCESS ) {
        m_p_buf_vs = mmp_buffer_mgr::get_instance()->alloc_media_videostream((MMP_MEDIA_ID)this, stream_buf_max_size, mmp_buffer::ION);
        if(m_p_buf_vs == NULL) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpPlayerVideoEx1::Open] FAIL:  mmp_buffer_mgr::get_instance()->alloc_media_videostream(%d) "), stream_buf_max_size));
            mmpResult = MMP_FAILURE;   
        }
    }

    /* 
        create video renderer 
           **note: The VideoRenderer must be created before VideoDecoder for deciding  fourcc_out of VideoDecoder.
    */
    if(mmpResult == MMP_SUCCESS ) {
        
#if (MMP_OS == MMP_OS_WIN32)
        renderer_video_create_config.hRenderWnd = this->m_create_config.video_config.m_hRenderWnd;
        renderer_video_create_config.hRenderDC = this->m_create_config.video_config.m_hRenderDC;
#endif
        renderer_video_create_config.id = CMmpRendererVideo::ID_DISPLAY;
        //renderer_video_create_config.id = CMmpRendererVideo::ID_YUV_WRITER;
        m_pRendererVideo = CMmpRendererVideo::CreateObject(&renderer_video_create_config);
        if(m_pRendererVideo == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpPlayerVideoEx1::Open] FAIL:  CMmpRenderer::CreateVideoObject ")));
        }
    }

#if 1
    /* create video decoder */
    if(mmpResult == MMP_SUCCESS ) {

        memset(&decoder_create_config, 0x00, sizeof(decoder_create_config));
        decoder_create_config.fourcc_in = m_pDemuxer->GetVideoFourcc();
        decoder_create_config.fourcc_out = m_pRendererVideo->get_fourcc_in();

        m_pic_width = m_pDemuxer->GetVideoPicWidth();
        m_pic_height = m_pDemuxer->GetVideoPicHeight();

        m_buffer_stride = MMP_BYTE_ALIGN(m_pic_width, 16);
        m_buffer_height = m_pic_height;

        m_pDecoderVideo = CMmpDecoderVideo::CreateObject(&decoder_create_config, m_create_config.bForceSWCodec /*MMP_FALSE*/ /* MMP_TRUE: FFmpeg Force Use */);
        if(m_pDecoderVideo == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(0, (TEXT("[CMmpPlayerVideoEx1::Open] FAIL:  CMmpDecoder::CreateVideoObject  %c%c%c%c %dx%d"), 
                    MMPGETFOURCCARG(decoder_create_config.fourcc_in),
                    m_pic_width, m_pic_height
                ));
        }
        else {

            m_pDemuxer->GetVideoExtraData(m_p_buf_vs);
            if(m_p_buf_vs->get_stream_size() > 0) {

                m_pDecoderVideo->set_pic_width( m_pDemuxer->GetVideoPicWidth() );
                m_pDecoderVideo->set_pic_height( m_pDemuxer->GetVideoPicHeight() );

                m_p_buf_vs->set_flag(mmp_buffer_media::FLAG_CONFIGDATA);
                m_pDecoderVideo->DecodeAu(m_p_buf_vs, (class mmp_buffer_videoframe**)NULL, &is_video_dec_reconfig);
            }
        }

    }
#endif


    if(mmpResult == MMP_SUCCESS ) {
        mmpResult = CMmpPlayer::Open();
    }


    if(mmpResult == MMP_SUCCESS) {
    
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpPlayerVideoEx1::Open] is_video_dec_reconfig=%d "), is_video_dec_reconfig));

            if(is_video_dec_reconfig == MMP_TRUE) {
                
                    MMP_S32 dbufcnt = m_pDecoderVideo->get_buf_req_count_actual();
                    MMP_S32 i2;
                    MMP_S32 pic_width, pic_height;
                       
                    pic_width = m_pDecoderVideo->get_pic_width();
                    pic_height = m_pDecoderVideo->get_pic_height();
                    
                    this->m_pRendererVideo->vf_config(dbufcnt, pic_width, pic_height); 
                    for(i2 = 0; i2 < this->m_pRendererVideo->vf_get_count(); i2++) {

                        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpPlayerVideoEx1::Open] ln=%d "), __LINE__));

                        class mmp_buffer_videoframe *p_vf_tmp = this->m_pRendererVideo->vf_get(i2);
                        m_pDecoderVideo->use_buf_vf(p_vf_tmp);
                        p_vf_tmp->set_own((MMP_MEDIA_ID)m_pDecoderVideo);
                        
                    }
            }

    }

    return mmpResult;
}


MMP_RESULT CMmpPlayerVideoEx1::Close()
{
    CMmpPlayer::Close();

    if(m_pDemuxer != NULL) {
        CMmpDemuxer::DestroyObject(m_pDemuxer);  
        m_pDemuxer = NULL;
    }


    if(m_pDecoderVideo != NULL) {
        CMmpDecoder::DestroyObject(m_pDecoderVideo);  
        m_pDecoderVideo = NULL;
    }


    if(m_pRendererVideo != NULL) {
        CMmpRenderer::DestroyObject(m_pRendererVideo);  
        m_pRendererVideo = NULL;
    }
    
    /* destoy stream buffer */
    if(m_p_buf_vs != NULL) {
        mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_vs);
        m_p_buf_vs = NULL;
    }

    return MMP_SUCCESS;
}

MMP_S64 CMmpPlayerVideoEx1::GetDuration() {

    MMP_S64 dur = 0;
    if(m_pDemuxer != NULL) {
       dur = m_pDemuxer->GetDuration();
    }
    return dur;
}
    
MMP_S32 CMmpPlayerVideoEx1::GetPlayFPS() {
    return m_fps;
}

MMP_S64 CMmpPlayerVideoEx1::GetPlayPosition() {
    return m_last_packet_pts;
}

MMP_S32 CMmpPlayerVideoEx1::GetVideoDecoderFPS() {

    MMP_S32 fps = 0;
    
    if(m_pDecoderVideo != NULL) {
        fps = m_pDecoderVideo->GetAvgFPS();
    }

    return fps;
}

MMP_S32 CMmpPlayerVideoEx1::GetVideoDecoderDur() {

    MMP_S32 dur = 0;
    
    if(m_pDecoderVideo != NULL) {
        dur = m_pDecoderVideo->GetAvgDur();
    }

    return dur;
}

MMP_S32 CMmpPlayerVideoEx1::GetVideoDecoderTotalDecFrameCount() {

    MMP_S32 cnt = 0;
    
    if(m_pDecoderVideo != NULL) {
        cnt = m_pDecoderVideo->GetTotalDecFrameCount();
    }

    return cnt;
}

const MMP_CHAR* CMmpPlayerVideoEx1::GetVideoDecoderClassName() {

    return m_pDecoderVideo->get_class_name();
}

MMP_S32 CMmpPlayerVideoEx1::GetVideoWidth() {
    MMP_S32 val = 0;
    if(m_pDemuxer != NULL) {
       val = m_pDemuxer->GetVideoPicWidth();
    }
    return val;
}

MMP_S32 CMmpPlayerVideoEx1::GetVideoHeight() {
    MMP_S32 val = 0;
    if(m_pDemuxer != NULL) {
       val = m_pDemuxer->GetVideoPicHeight();
    }
    return val;
}

enum MMP_FOURCC CMmpPlayerVideoEx1::GetVideoFourcc(){
    enum MMP_FOURCC fourcc = MMP_FOURCC_VIDEO_UNKNOWN;
    if(m_pDemuxer != NULL) {
       fourcc = m_pDemuxer->GetVideoFourcc();
    }
    return fourcc;
}

/* Video Renderer */
void CMmpPlayerVideoEx1::SetFirstVideoRenderer() {

    if(m_pRendererVideo != NULL) {
        m_pRendererVideo->SetFirstRenderer();
    }
}

MMP_BOOL CMmpPlayerVideoEx1::IsFirstVideoRenderer() {

    MMP_BOOL bFlag = MMP_FALSE;

    if(m_pRendererVideo != NULL) {
        bFlag = m_pRendererVideo->IsFirstRenderer();
    }

    return bFlag;
}

void CMmpPlayerVideoEx1::SetVideoRotate(enum MMP_ROTATE rotate) {

    if(m_pRendererVideo != NULL) {
        m_pRendererVideo->SetRotate(rotate);
    }

}

void CMmpPlayerVideoEx1::Service()
{
    MMP_RESULT mmpResult_Dec;
    MMP_U32 frame_count = 0;
    FILE* dump_fp = NULL;
    MMP_S32 fps = 0;
    MMP_U32 cur_tick, before_tick, t1, t2;
    class mmp_buffer_videoframe* p_buf_vf_decoded;
    MMP_BOOL is_reconfig;

    MMP_S32 pic_width, pic_height;
    //MMP_S32 buffer_width, buffer_height;
    
#if 0
#if (MMP_OS == MMP_OS_WIN32)
    dump_fp = fopen("d:\\work\\h264_BP_FullHD.h264", "wb");
#endif
#endif
    

    before_tick = CMmpUtil::GetTickCount();

    while(m_bServiceRun == MMP_TRUE) {
                
        t1 = CMmpUtil::GetTickCount();
                
        m_pDemuxer->GetNextVideoData(this->m_p_buf_vs);
        this->m_p_buf_vs->set_stream_offset(0);

        while(1) {

            int stream_real_size;
            stream_real_size = this->m_p_buf_vs->get_stream_real_size();
            if(stream_real_size <= 0) {
                break;
            }

            if(dump_fp != NULL) {
                fwrite((void*)m_p_buf_vs->get_buf_vir_addr(), 1,  m_p_buf_vs->get_stream_size(), dump_fp);
            }

            m_p_buf_vs->set_flag(0);
            
            m_last_packet_pts = m_p_buf_vs->get_pts();
            p_buf_vf_decoded = NULL;

            this->mon_vdec_begin();
            mmpResult_Dec = m_pDecoderVideo->DecodeAu(m_p_buf_vs, &p_buf_vf_decoded, &is_reconfig);
            this->mon_vdec_end(p_buf_vf_decoded);

            if(mmpResult_Dec != MMP_SUCCESS) {
                break;
            }

            if(is_reconfig == MMP_TRUE) {
                MMP_S32 dbufcnt = m_pDecoderVideo->get_buf_req_count_actual();
                MMP_S32 i2, rbufcnt;
                                       
                pic_width = m_pDecoderVideo->get_pic_width();
                pic_height = m_pDecoderVideo->get_pic_height();
                    
                this->m_pRendererVideo->vf_config(dbufcnt, pic_width, pic_height); 
                rbufcnt = this->m_pRendererVideo->vf_get_count();
                for(i2 = 0; i2 < rbufcnt; i2++) {
                    class mmp_buffer_videoframe *p_vf_tmp = this->m_pRendererVideo->vf_get(i2);
                    m_pDecoderVideo->use_buf_vf(p_vf_tmp);
                    p_vf_tmp->set_own((MMP_MEDIA_ID)m_pDecoderVideo);
                }

                MMPDEBUGMSG(1, (TEXT("[CMmpPlayerVideoEx1::Service] Reconfig  pic(%d %d) renbufcnt=%d  "),
                        pic_width, pic_height, 
                        rbufcnt
                    ));
                p_buf_vf_decoded = NULL;
            }

            if(p_buf_vf_decoded != NULL) {

                this->mon_vren_begin();
                m_pRendererVideo->Render(p_buf_vf_decoded, (MMP_MEDIA_ID)m_pDecoderVideo);
                this->mon_vren_end(p_buf_vf_decoded);
            }

            fps++;
        }
        
        frame_count++;

        t2 = CMmpUtil::GetTickCount();
        
        cur_tick = t2;    

        if( (cur_tick - before_tick) > 1000 ) {
            m_fps = fps;
            fps = 0;
                    
            if(this->m_create_config.callback != NULL) {

                struct mmp_player_callback_playtime playtime_st;
                MMP_U32 msg;
                void *data1 = NULL, *data2 = NULL;

                msg = CMmpPlayer::CALLBACK_PLAYTIME;
                playtime_st.media_duration  = m_pDemuxer->GetDuration();
                playtime_st.media_pts = this->m_p_buf_vs->get_pts();

                (*this->m_create_config.callback)(this->m_create_config.callback_privdata, msg, (void*)&playtime_st, NULL);
            }

            this->mon_print_every_1sec_video(this->m_pDecoderVideo->get_class_name());
            this->mon_reset_every_1sec();
            
            before_tick = cur_tick;
        }
        
        if( (t2-t1) < 10 ) {
            CMmpUtil::Sleep(20);
        }
        else {
            CMmpUtil::Sleep(1);
        }

    } /* endo fo while(m_bServiceRun == MMP_TRUE) { */

    if(dump_fp != NULL) {
        fclose(dump_fp);
    }
    
    MMPDEBUGMSG(1, (TEXT("[CMmpPlayerVideoEx1::Service] Task Ended!!")));
}


