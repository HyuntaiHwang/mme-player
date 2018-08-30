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

#include "MmpPlayerVideoEx3.hpp"
#include "MmpUtil.hpp"

/////////////////////////////////////////////////////////////
//CMmpPlayerVideoEx3 Member Functions

CMmpPlayerVideoEx3::CMmpPlayerVideoEx3(CMmpPlayerCreateProp* pPlayerProp) : CMmpPlayer(pPlayerProp, "VideoEx3")
,m_pDemuxer(NULL)
,m_pDecoderVideo(NULL)
,m_pRendererVideo(NULL)

,m_last_packet_pts(0)
,m_fps(0)

,m_p_buf_vs(NULL)

#if (V4L2_JPEG_ENABLE == 1)
,m_v4l2_hdl(NULL)
,m_jpeg_id(0)
#endif

,m_is_seek(MMP_FALSE)
,m_seek_pts(0)
{
    
}

CMmpPlayerVideoEx3::~CMmpPlayerVideoEx3()
{
    
}

MMP_RESULT CMmpPlayerVideoEx3::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;
    struct MmpDemuxerCreateConfig demuxer_create_config;
    struct CMmpDecoderVideo::create_config decoder_create_config;
    struct CMmpRendererVideo::create_config renderer_video_create_config;
    
    MMP_S32 stream_buf_max_size;
    MMP_S32 demuxer_pic_width, demuxer_pic_height;
    
    enum CMmpDecoderVideo::ACTION next_action;

    /* create demuxer */
    if(mmpResult == MMP_SUCCESS ) {
        strcpy((char*)demuxer_create_config.filename, this->m_create_config.filename);
        m_pDemuxer = CMmpDemuxer::CreateObject(&demuxer_create_config);
        if(m_pDemuxer == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpPlayerVideoEx3::Open] FAIL:  CMmpDemuxer::CreateObject ")));
        }
        else {
            demuxer_pic_width = m_pDemuxer->GetVideoPicWidth();
            demuxer_pic_height = m_pDemuxer->GetVideoPicHeight();
            stream_buf_max_size = demuxer_pic_width*demuxer_pic_height*3/2;
        }
    }

    /* create stream buffer */
    if(mmpResult == MMP_SUCCESS ) {
        m_p_buf_vs = mmp_buffer_mgr::get_instance()->alloc_media_videostream((MMP_MEDIA_ID)this, stream_buf_max_size, mmp_buffer::ION);
        if(m_p_buf_vs == NULL) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpPlayerVideoEx3::Open] FAIL:  mmp_buffer_mgr::get_instance()->alloc_media_videostream(%d) "), stream_buf_max_size));
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
        //renderer_video_create_config.id = CMmpRendererVideo::ID_DISPLAY;
        renderer_video_create_config.id = (CMmpRendererVideo::ID)this->m_create_config.video_config.m_idRender;
        //renderer_video_create_config.id = CMmpRendererVideo::ID_YUV_WRITER;
        //renderer_video_create_config.id = CMmpRendererVideo::ID_DUMMY;
        m_pRendererVideo = CMmpRendererVideo::CreateObject(&renderer_video_create_config);
        if(m_pRendererVideo == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpPlayerVideoEx3::Open] FAIL:  CMmpRenderer::CreateVideoObject ")));
        }
    }

#if 1
    /* create video decoder */
    if(mmpResult == MMP_SUCCESS ) {

        memset(&decoder_create_config, 0x00, sizeof(decoder_create_config));
        decoder_create_config.fourcc_in = m_pDemuxer->GetVideoFourcc();
        decoder_create_config.fourcc_out = m_pRendererVideo->get_fourcc_in();
        decoder_create_config.pic_width = MMP_VIDEO_DEC_MIN_WIDTH;
        decoder_create_config.pic_height = MMP_VIDEO_DEC_MIN_HEIGHT;
        decoder_create_config.buf_req_count_min = MMP_VIDEO_DECODED_BUF_MIN_COUNT_DEFAULT;
        decoder_create_config.buf_req_count_actual = MMP_VIDEO_DECODED_BUF_ACTUAL_COUNT_DEFAULT;
        decoder_create_config.is_android_buffer = MMP_FALSE;
        
	           
        m_pDecoderVideo = CMmpDecoderVideo::CreateObject(&decoder_create_config, m_create_config.bForceSWCodec /*MMP_FALSE*/ /* MMP_TRUE: FFmpeg Force Use */);
        if(m_pDecoderVideo == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(0, (TEXT("[CMmpPlayerVideoEx3::Open] FAIL:  CMmpDecoder::CreateVideoObject  %c%c%c%c %dx%d"), 
                    MMPGETFOURCCARG(decoder_create_config.fourcc_in),
                    demuxer_pic_width, demuxer_pic_height
                ));
        }
        else {

            m_pDecoderVideo->set_android_buffer(MMP_TRUE);
            m_pDecoderVideo->init_buf_vf_from_demuxer(demuxer_pic_width, demuxer_pic_height, m_pRendererVideo->get_fourcc_in(), m_pDecoderVideo->get_buf_req_count_min(), m_pDecoderVideo->get_buf_req_count_actual() );


            MMP_S32 dbufcnt = m_pDecoderVideo->get_buf_req_count_actual();
            MMP_S32 i2, j2;
            MMP_S32 pic_width, pic_height;
            struct mmp_rect display_crop_rect;
                       
            pic_width = m_pDecoderVideo->get_decoder_pic_width();
            pic_height = m_pDecoderVideo->get_decoder_pic_height();
            display_crop_rect = m_pDecoderVideo->get_display_crop_rect();
                    
            this->m_pRendererVideo->vf_config(dbufcnt, pic_width, pic_height, display_crop_rect); 
            for(i2 = 0; i2 < this->m_pRendererVideo->vf_get_count(); i2++) {
                class mmp_buffer_videoframe *p_vf_tmp = this->m_pRendererVideo->vf_get(i2);
                m_pDecoderVideo->use_buf_vf_from_renderer(p_vf_tmp);
                m_pDecoderVideo->clear_display_flag(p_vf_tmp);
                p_vf_tmp->set_own((MMP_MEDIA_ID)m_pDecoderVideo);
            }

            m_pDemuxer->GetVideoExtraData(m_p_buf_vs);
            if(m_p_buf_vs->get_stream_size() > 0) {
                m_p_buf_vs->set_flag(mmp_buffer_media::FLAG_CONFIGDATA);
                m_pDecoderVideo->DecodeAu(m_p_buf_vs, (class mmp_buffer_videoframe**)NULL, &next_action);
                if(next_action == CMmpDecoderVideo::ACTION_RECONFIG) {
                    dbufcnt = m_pDecoderVideo->get_buf_req_count_actual();
                    pic_width = m_pDecoderVideo->get_decoder_pic_width();
                    pic_height = m_pDecoderVideo->get_decoder_pic_height();
                    display_crop_rect = m_pDecoderVideo->get_display_crop_rect();

                    /* Renderer ReOpen */
                    CMmpRenderer::DestroyObject(m_pRendererVideo);  
                    m_pRendererVideo = this->CreateRendererVideo();
                    
                    this->m_pRendererVideo->vf_config(dbufcnt, pic_width, pic_height, display_crop_rect); 
                    j2 = this->m_pRendererVideo->vf_get_count();
                    for(i2 = 0; i2 < j2; i2++) {
                        class mmp_buffer_videoframe *p_vf_tmp = this->m_pRendererVideo->vf_get(i2);
                        m_pDecoderVideo->use_buf_vf_from_renderer(p_vf_tmp);
                        m_pDecoderVideo->clear_display_flag(p_vf_tmp);
                        p_vf_tmp->set_own((MMP_MEDIA_ID)m_pDecoderVideo);
                    }
                }
            }
        }

    }
#endif


    if(mmpResult == MMP_SUCCESS ) {
        mmpResult = CMmpPlayer::Open();
    }
        
    return mmpResult;
}


MMP_RESULT CMmpPlayerVideoEx3::Close()
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

#if (V4L2_JPEG_ENABLE == 1)
    if(m_v4l2_hdl != NULL) {
        v4l2_jpeg_enc_close(m_v4l2_hdl);
        v4l2_ion_free_buffer(&m_v4l2_ion_jpeg_dst);
        m_v4l2_hdl = NULL;
    }
#endif

    return MMP_SUCCESS;
}

MMP_S64 CMmpPlayerVideoEx3::GetDuration() {

    MMP_S64 dur = 0;
    if(m_pDemuxer != NULL) {
       dur = m_pDemuxer->GetDuration();
    }
    return dur;
}
    
MMP_S32 CMmpPlayerVideoEx3::GetPlayFPS() {
    return m_fps;
}

MMP_S64 CMmpPlayerVideoEx3::GetPlayPosition() {
    return m_last_packet_pts;
}

MMP_S32 CMmpPlayerVideoEx3::GetVideoDecoderFPS() {

    MMP_S32 fps = 0;
    
    if(m_pDecoderVideo != NULL) {
        fps = m_pDecoderVideo->GetAvgFPS();
    }

    return fps;
}

MMP_S32 CMmpPlayerVideoEx3::GetVideoDecoderDur() {

    MMP_S32 dur = 0;
    
    if(m_pDecoderVideo != NULL) {
        dur = m_pDecoderVideo->GetAvgDur();
    }

    return dur;
}

MMP_S32 CMmpPlayerVideoEx3::GetVideoDecoderTotalDecFrameCount() {

    MMP_S32 cnt = 0;
    
    if(m_pDecoderVideo != NULL) {
        cnt = m_pDecoderVideo->GetTotalDecFrameCount();
    }

    return cnt;
}

const MMP_CHAR* CMmpPlayerVideoEx3::GetVideoDecoderClassName() {

    return m_pDecoderVideo->get_class_name();
}

MMP_S32 CMmpPlayerVideoEx3::GetVideoPicWidth() {
    MMP_S32 val = 0;
    if(m_pDemuxer != NULL) {
       val = m_pDemuxer->GetVideoPicWidth();
    }
    return val;
}

MMP_S32 CMmpPlayerVideoEx3::GetVideoPicHeight() {
    MMP_S32 val = 0;
    if(m_pDemuxer != NULL) {
       val = m_pDemuxer->GetVideoPicHeight();
    }
    return val;
}

enum MMP_FOURCC CMmpPlayerVideoEx3::GetVideoFourcc(){
    enum MMP_FOURCC fourcc = MMP_FOURCC_VIDEO_UNKNOWN;
    if(m_pDemuxer != NULL) {
       fourcc = m_pDemuxer->GetVideoFourcc();
    }
    return fourcc;
}

/* Video Renderer */
void CMmpPlayerVideoEx3::SetFirstVideoRenderer() {

    if(m_pRendererVideo != NULL) {
        m_pRendererVideo->SetFirstRenderer();
    }
}

MMP_BOOL CMmpPlayerVideoEx3::IsFirstVideoRenderer() {

    MMP_BOOL bFlag = MMP_FALSE;

    if(m_pRendererVideo != NULL) {
        bFlag = m_pRendererVideo->IsFirstRenderer();
    }

    return bFlag;
}

void CMmpPlayerVideoEx3::SetVideoRotate(enum MMP_ROTATE rotate) {

    if(m_pRendererVideo != NULL) {
        m_pRendererVideo->SetRotate(rotate);
    }

}

MMP_RESULT CMmpPlayerVideoEx3::Seek(MMP_S64 pts) {

    //m_pDemuxer->Seek(pts);

    m_seek_pts = pts;
    m_is_seek = MMP_TRUE;

    return MMP_SUCCESS;
}

void CMmpPlayerVideoEx3::Service()
{
    MMP_RESULT mmpResult_Dec;
    MMP_U32 frame_count = 0;
    FILE* dump_fp = NULL;
    MMP_S32 fps = 0;
    MMP_U32 cur_tick, before_tick, t1, t2;
    class mmp_buffer_videoframe* p_buf_vf_decoded;
    CMmpDecoderVideo::ACTION next_action;

    MMP_S32 pic_width, pic_height;
    //MMP_S32 buffer_width, buffer_height;
    
#if 0
#if (MMP_OS == MMP_OS_WIN32)
    dump_fp = fopen("d:\\work\\h264_BP_FullHD.h264", "wb");
#endif
#endif
    

    m_pDecoderVideo->set_android_buffer(MMP_TRUE);
    before_tick = CMmpUtil::GetTickCount();
        
#if 0
    /* Thumbnail Test */
    m_pDemuxer->Seek(10LL*1000000LL);
#endif

    while(m_bServiceRun == MMP_TRUE) {
                
        t1 = CMmpUtil::GetTickCount();

        if(m_is_seek == MMP_TRUE) {
            m_pDemuxer->Seek(m_seek_pts);
            m_is_seek = MMP_FALSE;
        }

                
        next_action = CMmpDecoderVideo::ACTION_NONE;
        m_pDemuxer->GetNextVideoData(this->m_p_buf_vs);
        this->m_p_buf_vs->set_stream_offset(0);

        while(1) {

            int stream_real_size;

            if(next_action == CMmpDecoderVideo::ACTION_CHUNK_REUSE) {
                this->m_p_buf_vs->set_flag(mmp_buffer_media::FLAG_VIDEO_CHUNK_REUSE);
                //MMPDEBUGMSG(1, (TEXT("[CMmpPlayerVideoEx3::Service] Chunk Reuse ")));
            }
            else {
                stream_real_size = this->m_p_buf_vs->get_stream_real_size();
                if(stream_real_size <= 0) {
                    break;
                }
                this->m_p_buf_vs->set_flag(mmp_buffer_media::FLAG_NULL);
            }

            if(dump_fp != NULL) {
                fwrite((void*)m_p_buf_vs->get_buf_vir_addr(), 1,  m_p_buf_vs->get_stream_size(), dump_fp);
            }

            m_last_packet_pts = m_p_buf_vs->get_pts();
            p_buf_vf_decoded = NULL;

            this->mon_vdec_begin();
            mmpResult_Dec = m_pDecoderVideo->DecodeAu(m_p_buf_vs, &p_buf_vf_decoded, &next_action);
            this->mon_vdec_end(m_pDecoderVideo, m_p_buf_vs, p_buf_vf_decoded);

            if(mmpResult_Dec != MMP_SUCCESS) {
                break;
            }

            if(next_action == CMmpDecoderVideo::ACTION_RECONFIG) {
                MMP_S32 dbufcnt = m_pDecoderVideo->get_buf_req_count_actual();
                MMP_S32 i2, rbufcnt;
                                       
                pic_width = m_pDecoderVideo->get_decoder_pic_width();
                pic_height = m_pDecoderVideo->get_decoder_pic_height();
                    
                this->m_pRendererVideo->vf_config(dbufcnt, pic_width, pic_height); 
                rbufcnt = this->m_pRendererVideo->vf_get_count();
                for(i2 = 0; i2 < rbufcnt; i2++) {
                    class mmp_buffer_videoframe *p_vf_tmp = this->m_pRendererVideo->vf_get(i2);
                    m_pDecoderVideo->use_buf_vf_from_renderer(p_vf_tmp);
                    p_vf_tmp->set_own((MMP_MEDIA_ID)m_pDecoderVideo);
                }

                MMPDEBUGMSG(0, (TEXT("[CMmpPlayerVideoEx3::Service] Reconfig  pic(%d %d) renbufcnt=%d  "),
                        pic_width, pic_height, 
                        rbufcnt
                    ));
                p_buf_vf_decoded = NULL;
            }

            if(p_buf_vf_decoded != NULL) {

                static MMP_TICKS last_pts = 0;

#if 0
                //if(last_pts > p_buf_vf_decoded->get_pts()) {

                    pic_width = m_pDecoderVideo->get_decoder_pic_width();
                    pic_height = m_pDecoderVideo->get_decoder_pic_height();
                
                    MMPDEBUGMSG(1, (TEXT("[CMmpPlayerVideoEx3::Service]  pic(%d %d) last_ts=%lld ts=%lld "),     pic_width, pic_height,  
                        last_pts,
                        p_buf_vf_decoded->get_pts()  ));
                //}
#endif

                last_pts = p_buf_vf_decoded->get_pts();
                
#if 0
#ifdef WIN32
                p_buf_vf_decoded->save_bmp("d:\\work\\a.bmp");
#else
                p_buf_vf_decoded->save_bmp("/data/a.bmp");
#endif
#endif

                this->mon_vren_begin();
                m_pRendererVideo->Render(p_buf_vf_decoded, (MMP_MEDIA_ID)m_pDecoderVideo);
                this->mon_vren_end(p_buf_vf_decoded);
                m_pDecoderVideo->clear_display_flag((class mmp_buffer_videoframe*)p_buf_vf_decoded);
                
#if (V4L2_JPEG_ENABLE == 1)
                int jpegsize;

                if(m_v4l2_hdl == NULL) {
                    m_v4l2_hdl = v4l2_jpeg_enc_open();
                    if(m_v4l2_hdl != 0) {

                        MMP_S32 pic_width, pic_height;
                        //unsigned int pix_fmt_src = (unsigned int)m_pDecoderVideo->get_fourcc_out();
                        //unsigned int pix_fmt_src = (unsigned int)V4L2_PIX_FMT_YUV420;
                        unsigned int pix_fmt_src = (unsigned int)V4L2_PIX_FMT_YVU420;

                        pic_width = m_pDecoderVideo->get_decoder_pic_width();
                        pic_height = m_pDecoderVideo->get_decoder_pic_height();

                        m_jpeg_enc_quality = 100;
                        memset(&m_v4l2_ion_frame_src, 0x00, sizeof(m_v4l2_ion_frame_src));
                        m_v4l2_ion_frame_src.pic_width = pic_width;
                        m_v4l2_ion_frame_src.pic_height = pic_height;
                        m_v4l2_ion_frame_src.fourcc = pix_fmt_src;
                        m_v4l2_ion_frame_src.plane_count = 1;
                        m_v4l2_ion_frame_src.plane[0].stride = V4L2_VIDEO_YVU420_Y_STRIDE(pic_width);
                        m_v4l2_ion_frame_src.plane[0].buf_size = V4L2_VIDEO_YVU420_FRAME_SIZE(pic_width, pic_height);

                        v4l2_ion_alloc_buffer(v4l2_jpeg_get_enc_max_stream_size(pic_width, pic_height, pix_fmt_src), &m_v4l2_ion_jpeg_dst, V4L2_ION_CACHED);

                        v4l2_jpeg_enc_prepare(m_v4l2_hdl, &m_v4l2_ion_frame_src, m_jpeg_enc_quality, &m_v4l2_ion_jpeg_dst);
                    }
                }

                
                if(m_jpeg_id < 60) {
                    m_v4l2_ion_frame_src.plane[0].shared_fd = p_buf_vf_decoded->get_buf_shared_fd();
                    if(v4l2_jpeg_enc_exe(m_v4l2_hdl, &m_v4l2_ion_frame_src, &m_v4l2_ion_jpeg_dst, &jpegsize) == 0) {
                      
                          char jpegname[256];
                          FILE* fp_jpeg;
                          sprintf(jpegname, "/data/mytmp/myrec%d.jpeg", m_jpeg_id);
                          m_jpeg_id++;
                          fp_jpeg = fopen(jpegname, "wb");
                          if(fp_jpeg) {
                              fwrite((const void*)m_v4l2_ion_jpeg_dst.vir_addr, 1, jpegsize, fp_jpeg);
                              fclose(fp_jpeg);
                          }
                    }
                }

#endif
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
    
    MMPDEBUGMSG(1, (TEXT("[CMmpPlayerVideoEx3::Service] Task Ended!!")));
}


