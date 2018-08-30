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

#include "MmpPlayerAVEx5.hpp"
#include "MmpUtil.hpp"

#define SOURCE_DEBUG 1 
#define AUDIO_DEC_DEBUG 1
#define VIDEO_DEC_DEBUG 1 
#define AUDIO_REN_DEBUG 1 
#define VIDEO_REN_DEBUG 1

/////////////////////////////////////////////////////////////
//CMmpPlayerAVEx5 Member Functions

CMmpPlayerAVEx5::CMmpPlayerAVEx5(CMmpPlayerCreateProp* pPlayerProp) : CMmpPlayer(pPlayerProp, "AVEx4" )

,m_is_player_run(MMP_FALSE)

,m_pDemuxer(NULL)
,m_pDecoderAudio(NULL)
,m_pDecoderVideo(NULL)
,m_pRendererAudio(NULL)
,m_pRendererVideo(NULL)

,m_p_task_video_dec(NULL)

,m_queue_video_stream(100)
,m_p_mutex_video_dec(NULL)
,m_p_cond_video_dec(NULL)

,m_queue_video_yuv(3)
,m_p_mutex_video_render(NULL)
,m_p_cond_video_render(NULL)


,m_p_task_audio_dec(NULL)

,m_queue_audio_stream(1000)
,m_p_mutex_audio_dec(NULL)
,m_p_cond_audio_dec(NULL)

,m_queue_audio_pcm(50)
,m_p_mutex_audio_render(NULL)
,m_p_cond_audio_render(NULL)

,m_bServiceRun_AudioRender(MMP_FALSE)
,m_p_task_audio_render(NULL)

,m_bServiceRun_VideoRender(MMP_FALSE)
,m_p_task_video_render(NULL)

,m_play_start_timestamp(0)
,m_seek_target_time(0)

,m_mon_vdec_count(0)
,m_mon_vdec_last_diff(0)

,m_mon_vren_count(0)
,m_mon_vren_last_diff(0)

,m_mon_adec_count(0)
,m_mon_adec_last_diff(0)

,m_mon_aren_count(0)
,m_mon_aren_last_diff(0)

{
    MMP_S32 i;

    for(i = 0; i < MMP_MEDIATYPE_MAX; i++) {
        m_is_media[i] = MMP_FALSE;
    }
}

CMmpPlayerAVEx5::~CMmpPlayerAVEx5()
{
    
    
}

MMP_RESULT CMmpPlayerAVEx5::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 demuxer_pic_width, demuxer_pic_height;
    MMP_S32 stream_buf_max_size;

    CMmpRendererCreateProp RendererProp;
    CMmpRendererCreateProp* pRendererProp=&RendererProp; 
    CMmpDecoderVideo::ACTION next_action;

    /* create demuxer */
    if(mmpResult == MMP_SUCCESS ) {
        m_pDemuxer = this->CreateDemuxer();
        if(m_pDemuxer == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpPlayerAVEx5::Open] FAIL: this->CreateDemuxer")));
        }
        else {
    
            if(m_pDemuxer->GetAudioFourcc() != MMP_FOURCC_AUDIO_UNKNOWN) {
                this->set_audio(MMP_TRUE);
            }
     
            if(m_pDemuxer->GetVideoFourcc() != MMP_FOURCC_VIDEO_UNKNOWN) {
                this->set_video(MMP_TRUE);
            }

            demuxer_pic_width = m_pDemuxer->GetVideoPicWidth();
            demuxer_pic_height = m_pDemuxer->GetVideoPicHeight();
            stream_buf_max_size = demuxer_pic_width*demuxer_pic_height*3/2;
        }
    }

    /* create audio decoder */
    if((mmpResult == MMP_SUCCESS) && (this->is_audio()==MMP_TRUE)) {
        m_pDecoderAudio = (CMmpDecoderAudio*)this->CreateDecoderAudio(m_pDemuxer);
        if(m_pDecoderAudio == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {
            this->DecodeAudioExtraData(m_pDemuxer, m_pDecoderAudio);
        }
    }

    if((mmpResult == MMP_SUCCESS) && (this->is_audio()==MMP_TRUE)) {
        m_pRendererAudio = this->CreateRendererAudio(m_pDecoderAudio);
        if(m_pRendererAudio == NULL) {
            mmpResult = MMP_FAILURE;
        }
    }

    /* create video render */
    if((mmpResult == MMP_SUCCESS) && (this->is_video()==MMP_TRUE))  {
        m_pRendererVideo = this->CreateRendererVideo();
        if(m_pRendererVideo == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {
    
     
        }
    }

    /* create vidoe decoder */
    if((mmpResult == MMP_SUCCESS) && (this->is_video()==MMP_TRUE))  {
        m_pDecoderVideo = (CMmpDecoderVideo*)this->CreateDecoderVideo(m_pDemuxer, m_pRendererVideo, m_create_config.bForceSWCodec /* bFFmpeg Use*/);
        if(m_pDecoderVideo == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {
            class mmp_buffer_videostream* p_buf_vs_temp;
            MMP_BOOL is_reconfig = MMP_FALSE;
            MMP_S32 dbufcnt;
            MMP_S32 i2;
            MMP_S32 pic_width, pic_height;
            struct mmp_rect display_crop_rect;
                       
            /* init buffer */
            m_pDecoderVideo->set_android_buffer(MMP_TRUE);
            m_pDecoderVideo->init_buf_vf_from_demuxer(demuxer_pic_width, demuxer_pic_height, 
                                                             m_pRendererVideo->get_fourcc_in(), 
                                                             m_pDecoderVideo->get_buf_req_count_min()+4, 
                                                             m_pDecoderVideo->get_buf_req_count_actual()+4 );

            /* config decoder */
            p_buf_vs_temp = mmp_buffer_mgr::get_instance()->alloc_media_videostream((MMP_MEDIA_ID)this, 1024, mmp_buffer::HEAP);
            if(p_buf_vs_temp != NULL) {
                m_pDemuxer->GetVideoExtraData(p_buf_vs_temp);
                if(p_buf_vs_temp->get_stream_real_size() > 0) {
                    p_buf_vs_temp->set_flag(mmp_buffer_media::FLAG_CONFIGDATA);
                    m_pDecoderVideo->DecodeAu(p_buf_vs_temp, (class mmp_buffer_videoframe**)NULL, &next_action);
                }
                mmp_buffer_mgr::get_instance()->free_media_buffer(p_buf_vs_temp);
            }
                    
            /* ready decoded buffer */
            dbufcnt = m_pDecoderVideo->get_buf_req_count_actual();
                        
            pic_width = m_pDecoderVideo->get_decoder_pic_width();
            pic_height = m_pDecoderVideo->get_decoder_pic_height();
            display_crop_rect = m_pDecoderVideo->get_display_crop_rect();
                    
            this->m_pRendererVideo->vf_config(dbufcnt, pic_width, pic_height, display_crop_rect); 
            for(i2 = 0; i2 < this->m_pRendererVideo->vf_get_count(); i2++) {
                class mmp_buffer_videoframe *p_vf_tmp = this->m_pRendererVideo->vf_get(i2);
                m_pDecoderVideo->use_buf_vf_from_renderer(p_vf_tmp);
                p_vf_tmp->set_own((MMP_MEDIA_ID)m_pDecoderVideo);
                        
            }

        }
    }


    
    /* create video decoding task */
    if((mmpResult == MMP_SUCCESS) && (this->is_video()==MMP_TRUE))  {

        /* create video dec cond */
        m_p_cond_video_dec = mmp_oal_cond::create_object();
        if(m_p_cond_video_dec == NULL) {
            mmpResult = MMP_FAILURE;
        }

        /* create video dec mutex */
        if(mmpResult == MMP_SUCCESS ) {
            m_p_mutex_video_dec = mmp_oal_mutex::create_object();
            if(m_p_mutex_video_dec == NULL) {
                mmpResult = MMP_FAILURE;
            }
        }

        /* create video rend cond */
        m_p_cond_video_render = mmp_oal_cond::create_object();
        if(m_p_cond_video_render == NULL) {
            mmpResult = MMP_FAILURE;
        }

        /* create video ren mutex */
        if(mmpResult == MMP_SUCCESS ) {
            m_p_mutex_video_render = mmp_oal_mutex::create_object();
            if(m_p_mutex_video_render == NULL) {
                mmpResult = MMP_FAILURE;
            }
        }

        /* create video dec task */
        if(mmpResult == MMP_SUCCESS ) {
            m_bServiceRun_VideoDec=MMP_TRUE;
            m_p_task_video_dec = mmp_oal_task::create_object(CMmpPlayerService::ServiceStub_VideoDec, this);
            if(m_p_task_video_dec == NULL)
            {
                mmpResult = MMP_FAILURE;
            }
        }
    }

    
    /* create audio decoding task */
    if((mmpResult == MMP_SUCCESS) && (this->is_audio()==MMP_TRUE))  {

        /* create audio dec cond */
        m_p_cond_audio_dec = mmp_oal_cond::create_object();
        if(m_p_cond_audio_dec == NULL) {
            mmpResult = MMP_FAILURE;
        }

        /* create audio dec mutex */
        if(mmpResult == MMP_SUCCESS ) {
            m_p_mutex_audio_dec = mmp_oal_mutex::create_object();
            if(m_p_mutex_audio_dec == NULL) {
                mmpResult = MMP_FAILURE;
            }
        }

        /* create audio rend cond */
        if(mmpResult == MMP_SUCCESS ) {
            m_p_cond_audio_render = mmp_oal_cond::create_object();
            if(m_p_cond_audio_render == NULL) {
                mmpResult = MMP_FAILURE;
            }
        }

        /* create audio ren mutex */
        if(mmpResult == MMP_SUCCESS ) {
            m_p_mutex_audio_render = mmp_oal_mutex::create_object();
            if(m_p_mutex_audio_render == NULL) {
                mmpResult = MMP_FAILURE;
            }
        }

        /* create audio dec task */
        if(mmpResult == MMP_SUCCESS ) {
            m_bServiceRun_AudioDec=MMP_TRUE;
            m_p_task_audio_dec = mmp_oal_task::create_object(CMmpPlayerService::ServiceStub_AudioDec, this);
            if(m_p_task_audio_dec == NULL)
            {
                mmpResult = MMP_FAILURE;
            }
        }
    }

    /* create audio render task */
    if((mmpResult == MMP_SUCCESS) && (this->is_audio()==MMP_TRUE))  {

        /* create audio render task */
        if(mmpResult == MMP_SUCCESS ) {
            m_bServiceRun_AudioRender=MMP_TRUE;
            m_p_task_audio_render = mmp_oal_task::create_object(CMmpPlayerService::ServiceStub_AudioRender, this);
            if(m_p_task_audio_render == NULL)
            {
                mmpResult = MMP_FAILURE;
            }
        }
    }

    /* create video render task */
    if((mmpResult == MMP_SUCCESS) && (this->is_video()==MMP_TRUE))  {

        /* create audio render task */
        if(mmpResult == MMP_SUCCESS ) {
            m_bServiceRun_VideoRender=MMP_TRUE;
            m_p_task_video_render = mmp_oal_task::create_object(CMmpPlayerService::ServiceStub_VideoRender, this);
            if(m_p_task_video_render == NULL)
            {
                mmpResult = MMP_FAILURE;
            }
        }
    }


    m_play_start_timestamp = CMmpUtil::GetTickCountUS();

    return mmpResult;
}


MMP_RESULT CMmpPlayerAVEx5::Close()
{    
#if 0
    if(m_p_task_audio_render != NULL)
    {
        m_bServiceRun_AudioRender=MMP_FALSE;
        m_p_cond_audio_render->signal(); //signal
        mmp_oal_task::destroy_object(m_p_task_audio_render);
        m_p_task_audio_render=NULL;
    }

    if(m_p_task_video_render != NULL)
    {
        m_bServiceRun_VideoRender=MMP_FALSE;
        m_p_cond_video_render->signal(); //signal
        mmp_oal_task::destroy_object(m_p_task_video_render);
        m_p_task_video_render=NULL;
    }

    if(m_p_task_video_dec != NULL)
    {
        m_bServiceRun_VideoDec=MMP_FALSE;
        m_p_cond_video_dec->signal(); //signal
        mmp_oal_task::destroy_object(m_p_task_video_dec);
        m_p_task_video_dec=NULL;
    }

    if(m_p_task_audio_dec != NULL)
    {
        m_bServiceRun_AudioDec=MMP_FALSE;
        m_p_cond_audio_dec->signal(); //signal
        mmp_oal_task::destroy_object(m_p_task_audio_dec);
        m_p_task_audio_dec=NULL;
    }
#else
    this->PlayStop();

#endif

    CMmpPlayer::Close();

    CMmpDemuxer::DestroyObject(m_pDemuxer);  m_pDemuxer = NULL;
    CMmpDecoder::DestroyObject(m_pDecoderAudio);  m_pDecoderAudio = NULL;
    CMmpDecoder::DestroyObject(m_pDecoderVideo);  m_pDecoderVideo = NULL;
    CMmpRenderer::DestroyObject(m_pRendererVideo);  m_pRendererVideo = NULL;
    
    if(m_pRendererAudio != NULL) {
        CMmpRenderer::DestroyObject(m_pRendererAudio);
        m_pRendererAudio = NULL;
    }

    /* delete video dec prop */
    if(m_p_cond_video_dec != NULL) {
        mmp_oal_cond::destroy_object(m_p_cond_video_dec);
        m_p_cond_video_dec = NULL;
    }
    
    if(m_p_mutex_video_dec != NULL) {
    
        mmp_oal_mutex::destroy_object(m_p_mutex_video_dec);
        m_p_mutex_video_dec = NULL;
    }

    if(m_p_cond_video_render != NULL) {
        mmp_oal_cond::destroy_object(m_p_cond_video_render);
        m_p_cond_video_render = NULL;
    }
    
    if(m_p_mutex_video_render != NULL) {
    
        mmp_oal_mutex::destroy_object(m_p_mutex_video_render);
        m_p_mutex_video_render = NULL;
    }

    this->flush_queue_video_stream();
    this->flush_queue_video_frame();
        
    /* delete audio dec prop */
    if(m_p_cond_audio_dec != NULL) {
        mmp_oal_cond::destroy_object(m_p_cond_audio_dec);
        m_p_cond_audio_dec = NULL;
    }
    
    if(m_p_mutex_audio_dec != NULL) {
    
        mmp_oal_mutex::destroy_object(m_p_mutex_audio_dec);
        m_p_mutex_audio_dec = NULL;
    }

    if(m_p_cond_audio_render != NULL) {
        mmp_oal_cond::destroy_object(m_p_cond_audio_render);
        m_p_cond_audio_render = NULL;
    }
    
    if(m_p_mutex_audio_render != NULL) {
    
        mmp_oal_mutex::destroy_object(m_p_mutex_audio_render);
        m_p_mutex_audio_render = NULL;
    }

    this->flush_queue_audio_stream();
    this->flush_queue_audio_frame();
    
    return MMP_SUCCESS;

}

MMP_RESULT CMmpPlayerAVEx5::PlayStop() {

    m_is_player_run = MMP_TRUE;

    //CMmpPlayerService::PlayStop();
    if(m_p_task != NULL)
    {
        m_bServiceRun=MMP_FALSE;
        mmp_oal_task::destroy_object(m_p_task);
        m_p_task=NULL;
    }


    if(m_p_task_video_dec != NULL)
    {
        m_bServiceRun_VideoDec=MMP_FALSE;
        m_p_cond_video_dec->signal(); //signal
        mmp_oal_task::destroy_object(m_p_task_video_dec);
        m_p_task_video_dec=NULL;
    }

    if(m_p_task_audio_dec != NULL)
    {
        m_bServiceRun_AudioDec=MMP_FALSE;
        m_p_cond_audio_dec->signal(); //signal
        mmp_oal_task::destroy_object(m_p_task_audio_dec);
        m_p_task_audio_dec=NULL;
    }

    if(m_p_task_video_render != NULL)
    {
        m_bServiceRun_VideoRender=MMP_FALSE;
        m_p_cond_video_render->signal(); //signal
        mmp_oal_task::destroy_object(m_p_task_video_render);
        m_p_task_video_render=NULL;
    }

    if(m_p_task_audio_render != NULL)
    {
        m_bServiceRun_AudioRender=MMP_FALSE;
        m_p_cond_audio_render->signal(); //signal
        mmp_oal_task::destroy_object(m_p_task_audio_render);
        m_p_task_audio_render=NULL;
    }


    return MMP_SUCCESS;
}

void CMmpPlayerAVEx5::flush_queue_audio_stream()  {

    class mmp_buffer_audiostream *p_buf_audiostream;

    while(!m_queue_audio_stream.IsEmpty()) {
        m_queue_audio_stream.Delete(p_buf_audiostream);
        if(p_buf_audiostream != NULL) {
            mmp_buffer_mgr::get_instance()->free_media_buffer(p_buf_audiostream);
        }
    }

}

void CMmpPlayerAVEx5::flush_queue_audio_frame() {

    class mmp_buffer_audioframe *p_buf_af;

    while(!m_queue_audio_pcm.IsEmpty()) {
        m_queue_audio_pcm.Delete(p_buf_af);
        if(p_buf_af != NULL) {
            mmp_buffer_mgr::get_instance()->free_media_buffer(p_buf_af);
        }
    }

}

void CMmpPlayerAVEx5::flush_queue_video_stream() {

    class mmp_buffer_videostream *p_buf_videostream;

    while(!m_queue_video_stream.IsEmpty()) {
        m_queue_video_stream.Delete(p_buf_videostream);
        if(p_buf_videostream != NULL) {
            mmp_buffer_mgr::get_instance()->free_media_buffer(p_buf_videostream);
        }
    }

}

void CMmpPlayerAVEx5::flush_queue_video_frame() {

    class mmp_buffer_videoframe *p_buf_vf;

    while(!m_queue_video_yuv.IsEmpty()) {
        m_queue_video_yuv.Delete(p_buf_vf);
    }
}


MMP_RESULT CMmpPlayerAVEx5::Seek(MMP_S64 pts) {

    this->PlayStop();
    CMmpUtil::Sleep(200);

    /* flush video stream queue */
    m_p_mutex_video_dec->lock();
    this->flush_queue_video_stream();
    m_p_mutex_video_dec->unlock();
    m_p_cond_video_dec->signal(); //signal

    /* flush video yuv queue */
    m_p_mutex_video_render->lock();
    this->flush_queue_video_frame();
    m_p_mutex_video_render->unlock();
    m_p_cond_video_render->signal(); //signal

    /* flush audio stream queue */
    m_p_mutex_audio_dec->lock();
    this->flush_queue_audio_stream();
    m_p_mutex_audio_dec->unlock();
    m_p_cond_audio_dec->signal(); //signal
    
    /* flush audio pcm queue */
    m_p_mutex_audio_render->lock();
    this->flush_queue_audio_frame();
    m_p_mutex_audio_render->unlock();
    m_p_cond_audio_render->signal(); //signal


    m_pDemuxer->Seek(pts);

    m_seek_target_time = pts;

    MMPDEBUGMSG(1, (TEXT("[CMmpPlayerAVEx5::Seek] pts=%d "), (unsigned int)(pts/1000) ));
                      

    this->PlayStart();

    return MMP_SUCCESS;
}


void CMmpPlayerAVEx5::Service_AudioDec() {

    MMP_U32 decoded_buf_max_size = 1024*1024;
    
    MMP_U8* stream_buf, *pbuf;
    MMP_TICKS anchor_timestamp;
    MMP_U32 decoded_size;
    MMP_U32 dec_frame_count = 0;

    MMP_U32 ch, sample_rate, bits_per_sample;
    MMP_S64 timestamp_weight;

    MMP_U32 t1, t2;

    class mmp_buffer_audiostream* p_buf_audiostream;
    class mmp_buffer_audioframe* p_buf_af;
    class mmp_buffer_audioframe* p_buf_af_temp;
 
    //p_omx_buffer_header_out->nTimeStamp = m_AnchorTimeUs + (m_NumSamplesOutput * 1000000ll) / m_pcm_parm.nSamplingRate;
    while(m_bServiceRun_AudioDec == MMP_TRUE) {

        m_p_mutex_audio_dec->lock();
        if( m_queue_audio_stream.IsEmpty() ) {
            m_p_cond_audio_dec->wait(m_p_mutex_audio_dec); //wait
        }
        
        p_buf_audiostream = NULL;
        if((!m_queue_audio_stream.IsEmpty()) && (m_bServiceRun_AudioDec == MMP_TRUE) )
	    {
		    m_queue_audio_stream.Delete(p_buf_audiostream);
	    }
        m_p_mutex_audio_dec->unlock();
    
        if(p_buf_audiostream != NULL) {
        
            stream_buf = (MMP_U8*)p_buf_audiostream->get_stream_real_addr();
            anchor_timestamp = p_buf_audiostream->get_pts();
            decoded_size = 0;
            ch = m_pDemuxer->GetAudioChannel();
            sample_rate = m_pDemuxer->GetAudioSamplingRate();
            bits_per_sample = m_pDemuxer->GetAudioBitsPerSample();
            timestamp_weight = 1000000LL*8LL/(MMP_S64)(ch*bits_per_sample*sample_rate);
            pbuf = stream_buf;
            while(p_buf_audiostream->get_stream_real_size() > 0) {

                if(m_bServiceRun_AudioDec != MMP_TRUE) {
                    break;
                }

                p_buf_audiostream->set_pts(anchor_timestamp + (MMP_S64)(decoded_size)*timestamp_weight);
                p_buf_af = mmp_buffer_mgr::get_instance()->alloc_media_audioframe((MMP_MEDIA_ID)this, mmp_buffer_audioframe::DEFAULT_MAX_FRAME_SIZE, mmp_buffer::HEAP);

                m_pDecoderAudio->DecodeAu(p_buf_audiostream, p_buf_af);
                if(p_buf_af->get_coding_result() == mmp_buffer_media::SUCCESS) {

                    t1 = CMmpUtil::GetTickCount();
                    t2 = t1;
                    while( (t2-t1) < 1000*1000) //Wait 100 ms
                    {
                        if(m_bServiceRun_AudioDec != MMP_TRUE) {
                            break;
                        }

                        if(!m_queue_audio_pcm.IsFull()) {
                            break;
                        }
                        CMmpUtil::Sleep(10);
                        t2 = CMmpUtil::GetTickCount();
                    }

#if (CMmpPlayerAVEx5_AUDIO_DEC_TASK_ADD_PCM_ENABLE == 1)
                    m_p_mutex_audio_render->lock();
                    if(m_queue_audio_pcm.IsFull()) {

                       MMPDEBUGMSG(AUDIO_DEC_DEBUG, (TEXT("[AV Player AudioDec] Render Task Busy Skip Audo")));
                       m_queue_audio_pcm.Delete(p_buf_af_temp);
                       mmp_buffer_mgr::get_instance()->free_media_buffer(p_buf_af_temp);
                    }
                    m_queue_audio_pcm.Add(p_buf_af);
                    m_p_mutex_audio_render->unlock();
#else
                    mmp_buffer_mgr::get_instance()->free_media_buffer(p_buf_af);
#endif

                    MMPDEBUGMSG(AUDIO_DEC_DEBUG, (TEXT("[AV Player AudioDec] %d. decsz=%d ts=(%d %d) pcm_q=%d "), 
                            dec_frame_count,
                            p_buf_af->get_data_size(),
                            (unsigned int)(p_buf_audiostream->get_pts()/1000), (unsigned int)(p_buf_af->get_pts()/1000),
                             m_queue_audio_pcm.GetSize()
                             ));

                    dec_frame_count++;
                    decoded_size +=  p_buf_af->get_data_size();
            
                    m_p_cond_audio_render->signal(); //signal
                }
                else {
                    mmp_buffer_mgr::get_instance()->free_media_buffer(p_buf_af);
                }

                //p_buf_audiostream->update_stream_offset(); /*  stream_offset += used_byte */
            }

            mmp_buffer_mgr::get_instance()->free_media_buffer(p_buf_audiostream);
        }

        CMmpUtil::Sleep(10);
    }

}

void CMmpPlayerAVEx5::Service_AudioRender() {

    MMP_TICKS cur_us, play_us, packet_pts = 0, diff;
    MMP_TICKS audio_eval_timestamp;
    MMP_U32 render_frame_count = 0;
    
    class mmp_buffer_audioframe* p_buf_af;
    
    while(m_bServiceRun_AudioRender == MMP_TRUE) {
    
        m_p_mutex_audio_render->lock();
        if( m_queue_audio_pcm.IsEmpty() ) {
            m_p_cond_audio_render->wait(m_p_mutex_audio_render); //wait
        }
        
        p_buf_af = NULL;
        if((!m_queue_audio_pcm.IsEmpty()) && (m_bServiceRun_AudioRender == MMP_TRUE) )
	    {
		    m_queue_audio_pcm.Delete(p_buf_af);		
	    }
        m_p_mutex_audio_render->unlock();
                
        while( (p_buf_af != NULL) && (m_bServiceRun_AudioRender == MMP_TRUE) )  {

            audio_eval_timestamp = p_buf_af->get_pts();

            cur_us = CMmpUtil::GetTickCountUS();
            play_us = cur_us - m_play_start_timestamp + m_seek_target_time;

            diff = audio_eval_timestamp - play_us;
            m_mon_aren_last_diff = (MMP_S32)(diff/1000);

            if(diff < 1000*1000*100) { 
            
                MMPDEBUGMSG(AUDIO_REN_DEBUG, (TEXT("[AV Player AudioRen] %d. decsz=%d ts=( %d %d, %d) decbuf=0x%08x "), 
                            render_frame_count,
                            p_buf_af->get_data_size(),
                             (unsigned int)(packet_pts/1000), (unsigned int)(audio_eval_timestamp/1000), (unsigned int)(play_us/1000),
                             p_buf_af->get_buf_vir_addr()
                             ));
                             
                if( (play_us - audio_eval_timestamp) < 1000*2000 ) {
                    if(m_pRendererAudio!=NULL)  m_pRendererAudio->Render(p_buf_af);
                    render_frame_count++;

                    m_mon_aren_count++;
                }
                else {
                    MMPDEBUGMSG(AUDIO_REN_DEBUG, (TEXT("[AV Player AudioRen] frame is too late, skip")));
                }

                mmp_buffer_mgr::get_instance()->free_media_buffer(p_buf_af);
                
                break;
            }
            else if( (play_us + 3*1000000L) < packet_pts ) {
            
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[AV Player AudioRen] %d. [frame is too fast, skip] decsz=%d ts=( %d %d, %d) decbuf=0x%08x "), 
                            render_frame_count,
                            p_buf_af->get_data_size(),
                             (unsigned int)(packet_pts/1000), (unsigned int)(audio_eval_timestamp/1000), (unsigned int)(play_us/1000),
                             p_buf_af->get_buf_vir_addr()
                             ));
                
                mmp_buffer_mgr::get_instance()->free_media_buffer(p_buf_af);
                break;
            }
            else {
                
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[AV Player AudioRen] %d. [too fast .. go to SLEEP] decsz=%d ts=( %d %d, %d) decbuf=0x%08x  diff=%d"), 
                            render_frame_count,
                            p_buf_af->get_data_size(),
                             (unsigned int)(packet_pts/1000), (unsigned int)(audio_eval_timestamp/1000), (unsigned int)(play_us/1000),
                             p_buf_af->get_buf_vir_addr(),
                             (int)(diff/1000)
                             ));
                if(diff>10) {
                    diff/=2;
                    CMmpUtil::Sleep((int)(diff/1000));
                }
                else {
                    CMmpUtil::Sleep(10);
                }
            }

        }

        CMmpUtil::Sleep(10);

    }

    

}

void CMmpPlayerAVEx5::Service_VideoDec() {
    
    MMP_RESULT mmpResult;
    MMP_S32 pic_width, pic_height;
    MMP_U32 buffer_width, buffer_height;
        
    MMP_U32 t1, t2;

    class mmp_buffer_videostream *p_buf_videostream;
    class mmp_buffer_videoframe *p_buf_vf_decoded;
    class mmp_buffer_videoframe *p_buf_vf_temp;
    MMP_U8* p_au;

    CMmpDecoderVideo::ACTION next_action = CMmpDecoderVideo::ACTION_NONE;

    pic_width = m_pDemuxer->GetVideoPicWidth();
    pic_height = m_pDemuxer->GetVideoPicHeight();
    buffer_width = MMP_BYTE_ALIGN(pic_width, 16);
    buffer_height = MMP_BYTE_ALIGN(pic_height, 16);

    while(m_bServiceRun_VideoDec == MMP_TRUE) {

        m_p_mutex_video_dec->lock();
        if( m_queue_video_stream.IsEmpty() ) {
            m_p_cond_video_dec->wait(m_p_mutex_video_dec); //wait
        }
        
        p_buf_videostream = NULL;
        if((!m_queue_video_stream.IsEmpty()) && (m_bServiceRun_VideoDec == MMP_TRUE) )
	    {
		    m_queue_video_stream.Delete(p_buf_videostream);
	    }
        m_p_mutex_video_dec->unlock();
    
        while(p_buf_videostream != NULL) {

            int stream_real_size;

            if(next_action == CMmpDecoderVideo::ACTION_CHUNK_REUSE) {
                p_buf_videostream->set_flag(mmp_buffer_media::FLAG_VIDEO_CHUNK_REUSE);
                //MMPDEBUGMSG(1, (TEXT("[CMmpPlayerVideoEx3::Service] Chunk Reuse ")));
            }
            else {
                stream_real_size = p_buf_videostream->get_stream_real_size();
                if(stream_real_size <= 0) {
                    mmp_buffer_mgr::get_instance()->free_media_buffer(p_buf_videostream);
                    break;
                }
                p_buf_videostream->set_flag(mmp_buffer_media::FLAG_NULL);
            }

                    
            p_au =(MMP_U8*)p_buf_videostream->get_buf_vir_addr();

            MMPDEBUGMSG(VIDEO_DEC_DEBUG, (TEXT("[AV Player VideoDec] In TS=%d sz=%d stream(%02x %02x %02x %02x %02x %02x %02x %02x) "),
                (unsigned int)(p_buf_videostream->get_pts()/1000),
                p_buf_videostream->get_stream_real_size(),
                p_au[0], p_au[1], p_au[2], p_au[3], 
                p_au[4], p_au[5], p_au[6], p_au[7] 
                ));

            //p_buf_vf = mmp_buffer_mgr::get_instance()->alloc_media_videoframe(pic_width, pic_height);

            while(m_pDecoderVideo->get_buf_vf_count() > 0) {
                if(m_pDecoderVideo->is_next_vf() == MMP_TRUE) {
                    break;
                }
                CMmpUtil::Sleep(10);
                MMPDEBUGMSG(1, (TEXT("[AV Player VideoDec] Wait next vf ")));
            }

            p_buf_vf_decoded = NULL;    
            this->mon_vdec_begin();
            mmpResult = m_pDecoderVideo->DecodeAu(p_buf_videostream, &p_buf_vf_decoded, &next_action);
            this->mon_vdec_end(m_pDecoderVideo, p_buf_videostream, p_buf_vf_decoded);

            if(next_action == CMmpDecoderVideo::ACTION_RECONFIG) {
                
                MMP_S32 dbufcnt = m_pDecoderVideo->get_buf_req_count_actual();
                MMP_S32 i2;
                       
                pic_width = m_pDecoderVideo->get_decoder_pic_width();
                pic_height = m_pDecoderVideo->get_decoder_pic_height();
                 
                this->m_pRendererVideo->vf_config(dbufcnt, pic_width, pic_height); 
                for(i2 = 0; i2 < this->m_pRendererVideo->vf_get_count(); i2++) {
                    class mmp_buffer_videoframe *p_vf_tmp = this->m_pRendererVideo->vf_get(i2);
                    m_pDecoderVideo->use_buf_vf_from_renderer(p_vf_tmp);
                    p_vf_tmp->set_own((MMP_MEDIA_ID)m_pDecoderVideo);
                }

                p_buf_vf_decoded = NULL;
            }

            if(p_buf_vf_decoded != NULL) {
                                
                MMP_S64 decoded_pts;
                decoded_pts = p_buf_vf_decoded->get_pts();
                MMPDEBUGMSG(VIDEO_DEC_DEBUG, (TEXT("[AV Player VideoDec] Out pts=%02d:%02d:%02d "),
                                            CMmpUtil::Time_GetMin((unsigned int)(decoded_pts/1000)),
                                            CMmpUtil::Time_GetSec((unsigned int)(decoded_pts/1000)), 
                                            CMmpUtil::Time_GetMileSec((unsigned int)(decoded_pts/1000)) 
                                            ));
                

                while(1) {
                    m_p_mutex_video_render->lock();
                    if(m_queue_video_yuv.IsFull()) {

                        //MMPDEBUGMSG(VIDEO_DEC_DEBUG, (TEXT("[AV Player VideoDec] Render Task, Busy Skip Video")));
                        //MMPDEBUGMSG(1, (TEXT("[AV Player VideoDec] Render Task, Busy Skip Video")));
                        //m_queue_video_yuv.Delete(p_buf_vf_temp);
                        m_p_mutex_video_render->unlock();
                        CMmpUtil::Sleep(10);
                        continue;
                    }
                    m_queue_video_yuv.Add(p_buf_vf_decoded);
                    m_p_mutex_video_render->unlock();
                    m_p_cond_video_render->signal(); //signal
                    break;
                }
                
                m_mon_vdec_last_diff = (MMP_S32)( (p_buf_vf_decoded->get_pts() -   (CMmpUtil::GetTickCountUS()-m_play_start_timestamp) )/1000 );
                m_mon_vdec_count++;
            }
            else {
                //MMPDEBUGMSG(1, (TEXT("[AV Player VideoDec] p_buf_vf_decoded is NULL")));
            }
            
        }

        CMmpUtil::Sleep(10);
    }
}
    
void CMmpPlayerAVEx5::Service_VideoRender() {

    MMP_S64 now_pts, video_pts;
    MMP_TICKS diff;
    class mmp_buffer_videoframe *p_buf_vf;
    const MMP_S64 threshold = 30*1000;
    int skip_count = 0;
    
    while(m_bServiceRun_VideoRender == MMP_TRUE) {
    
        m_p_mutex_video_render->lock();
        if( m_queue_video_yuv.IsEmpty() ) {
            m_p_cond_video_render->wait(m_p_mutex_video_render); //wait
        }
        
        p_buf_vf = NULL;
        if((!m_queue_video_yuv.IsEmpty()) && (m_bServiceRun_VideoRender == MMP_TRUE) )
	    {
		    m_queue_video_yuv.Delete(p_buf_vf);
	    }
        m_p_mutex_video_render->unlock();

        if( (p_buf_vf != NULL) && (skip_count > 0) ) {
            p_buf_vf->set_own((MMP_MEDIA_ID)this->m_pDecoderVideo);
            m_pDecoderVideo->clear_display_flag(p_buf_vf);
            skip_count--;
            continue;
        }
        

        while( (p_buf_vf != NULL)  && (m_bServiceRun_VideoRender == MMP_TRUE)) {

            video_pts = p_buf_vf->get_pts();
            now_pts = CMmpUtil::GetTickCountUS() - m_play_start_timestamp + m_seek_target_time;
            
            diff =  (MMP_TICKS)(video_pts - now_pts);
            m_mon_vren_last_diff = (MMP_S32)(diff/1000);

            if(video_pts < now_pts) { 
                    unsigned int t1, t2;

                    t1 = CMmpUtil::GetTickCount();
                    this->mon_vren_begin();
                    m_pRendererVideo->Render(p_buf_vf, (MMP_MEDIA_ID)this->m_pDecoderVideo);
                    this->mon_vren_end(p_buf_vf);
                    t2 = CMmpUtil::GetTickCount();

                    m_mon_vren_count++;

                    MMPDEBUGMSG(VIDEO_REN_DEBUG, (TEXT("[AV Player VideoRen]  ts=%d/%d ren_dur=%d "), 
                             (unsigned int)(video_pts/1000), 
                             (unsigned int)(now_pts/1000),
                             t2-t1
                             ));
                  break;
            }
            else {

                MMPDEBUGMSG(VIDEO_REN_DEBUG, (TEXT("[AV Player VideoRen]  ts=%d /%d Wait 10ms"), 
                             (unsigned int)(video_pts/1000), 
                             (unsigned int)(now_pts/1000)
                             ));
                CMmpUtil::Sleep(10);
            }

        }

        //CMmpUtil::Sleep(10);
    }

}

void CMmpPlayerAVEx5::Service() {

    MMP_BOOL bFreeAu;
    MMP_U32 stream_buf_max_size = 1024*1024;
    MMP_TICKS cur_tick, before_tick;
    MMP_TICKS packet_pts = 0;
    
    MMP_U32 packet_count = 0;
        
    m_play_start_timestamp = CMmpUtil::GetTickCountUS();
    before_tick = m_play_start_timestamp;

    class mmp_buffer_media *p_buf_media;
    
    while(m_bServiceRun == MMP_TRUE) {

        if( //((m_queue_video_stream.GetSize() < 3)  || (m_queue_audio_stream.GetSize() < 5))
             (!m_queue_audio_stream.IsFull())
            && (!m_queue_video_stream.IsFull())
            ) 
        {
            bFreeAu = MMP_TRUE;
            
            p_buf_media = m_pDemuxer->GetNextMediaBuffer();
            if(p_buf_media != NULL) {
            
                packet_pts = p_buf_media->get_pts();

                if(packet_pts == 533000LL) {
                    printf("Break");
                }

                int hour, min, sec, milesec;

                hour = CMmpUtil::Time_GetHour((unsigned int)(packet_pts/1000LL));
                min = CMmpUtil::Time_GetMin((unsigned int)(packet_pts/1000LL));
                sec = CMmpUtil::Time_GetSec((unsigned int)(packet_pts/1000LL)); 
                milesec = CMmpUtil::Time_GetMileSec((unsigned int)(packet_pts/1000LL));

                MMPDEBUGMSG(SOURCE_DEBUG, (TEXT("[Source Task] pktcnt=%d %s pts=%lld %02d:%02d:%02d (%04dms) "),
                                            packet_count, 
                                            (p_buf_media->is_video()==MMP_TRUE)?"Video":"Audio",
                                            packet_pts, 
                                            hour, min, sec, milesec
                                            ));
                if(packet_count == 0) {
                    m_play_start_timestamp = CMmpUtil::GetTickCountUS();
                }
                
                if(packet_pts < m_seek_target_time) {
                    m_seek_target_time = packet_pts;
                }

                if(p_buf_media->is_video() == MMP_TRUE) {
    
#if (CMmpPlayerAVEx5_DEMUX_TASK_ADD_VIDEO_DATA_ENABLE == 1)
                    m_p_mutex_video_dec->lock();
                    m_queue_video_stream.Add((class mmp_buffer_videostream*)p_buf_media);
                    m_p_mutex_video_dec->unlock();
                    m_p_cond_video_dec->signal(); //signal
#else
                    mmp_buffer_mgr::get_instance()->free_media_buffer(p_buf_media);
#endif 
                    bFreeAu = MMP_FALSE;
                }
                else if(p_buf_media->is_audio() == MMP_TRUE) {

#if (CMmpPlayerAVEx5_DEMUX_TASK_ADD_AUDIO_DATA_ENABLE == 1)
                    m_p_mutex_audio_dec->lock();
                    m_queue_audio_stream.Add((class mmp_buffer_audiostream*)p_buf_media);
                    m_p_mutex_audio_dec->unlock();
                    m_p_cond_audio_dec->signal(); //signal
                    bFreeAu = MMP_FALSE;
#else
                    mmp_buffer_mgr::get_instance()->free_media_buffer(p_buf_media);    
#endif
                }
                else {
                    mmp_buffer_mgr::get_instance()->free_media_buffer(p_buf_media);
                }
                packet_count++;
            }
        
        } /* if( (m_queue_video_yuv.GetSize() < 3)  || (m_queue_audio_pcm.GetSize() < 5) )  */
        else {
            CMmpUtil::Sleep(10);
        }

        cur_tick = CMmpUtil::GetTickCountUS();
        if( (cur_tick - before_tick) > (1000LL*1000LL) ) {
        
            if(this->m_create_config.callback != NULL) {

                struct mmp_player_callback_playtime playtime_st;
                MMP_U32 msg;
                void *data1 = NULL, *data2 = NULL;

                msg = CMmpPlayer::CALLBACK_PLAYTIME;
                playtime_st.media_duration  = m_pDemuxer->GetDuration();
                playtime_st.media_pts = packet_pts;
                (*this->m_create_config.callback)(this->m_create_config.callback_privdata, msg, (void*)&playtime_st, NULL);
            }


            //MMPDEBUGMSG(1, (TEXT("[Player] VD=(%d t=%d) VR=(%d t=%d)  AD=(%d t=%d) AR=(%d t=%d) "), 
              //             m_mon_vdec_count, m_mon_vdec_last_diff,  m_mon_vren_count, m_mon_vren_last_diff ,
                //           m_mon_adec_count, m_mon_adec_last_diff,  m_mon_aren_count, m_mon_aren_last_diff 
                  //         ));
            this->mon_print_every_1sec(this->m_pDecoderVideo->get_class_name());
            this->mon_reset_every_1sec();

            m_mon_vdec_count = 0;
            m_mon_vren_count = 0;

            m_mon_adec_count = 0;
            m_mon_aren_count = 0;

            before_tick = cur_tick;
        }
    } /* while(m_bServiceRun == MMP_TRUE) {  */
            
    MMPDEBUGMSG(1, (TEXT(MMP_CTAG" Stop, Byet!!"), MMP_CNAME));
}

