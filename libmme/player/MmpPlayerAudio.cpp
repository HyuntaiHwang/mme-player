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


#include "MmpPlayerAudio.hpp"
#include "MmpUtil.hpp"

/////////////////////////////////////////////////////////////
//CMmpPlayerAudio Member Functions

CMmpPlayerAudio::CMmpPlayerAudio(CMmpPlayerCreateProp* pPlayerProp) : CMmpPlayer(pPlayerProp, "Audio")
,m_pDemuxer(NULL)
,m_pDecoderAudio(NULL)
,m_pRendererAudio(NULL)

,m_p_buf_as(NULL)
,m_p_buf_af(NULL)

#if (CMmpPlayerAudio_PCM_DUMP == 1)
,m_pcm_dump_hdl(NULL)
#endif

,m_is_seek(MMP_FALSE)
,m_last_render_pts(0)
{
    

}

CMmpPlayerAudio::~CMmpPlayerAudio()
{
#if (CMmpPlayerAudio_PCM_DUMP == 1)
    pcm_dump_destroy(m_pcm_dump_hdl);
#endif

    
}

MMP_RESULT CMmpPlayerAudio::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;

    CMmpRendererCreateProp RendererProp;
    CMmpRendererCreateProp* pRendererProp=&RendererProp; 
    //MMP_BOOL is_reconfig;

    CMmpPlayer::Open();

    /* alloc audio stream buffer */
    if(mmpResult == MMP_SUCCESS) {
        m_p_buf_as = mmp_buffer_mgr::get_instance()->alloc_media_audiostream((MMP_MEDIA_ID)this, mmp_buffer_audiostream::DEFAULT_MAX_STREAM_SIZE, mmp_buffer::HEAP);
        if(m_p_buf_as == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT(MMP_CTAG"FAIL: alloc audio stream"), MMP_CNAME));
        }
    }

    /* alloc audio frame buffer */
    if(mmpResult == MMP_SUCCESS) {
        m_p_buf_af = mmp_buffer_mgr::get_instance()->alloc_media_audioframe((MMP_MEDIA_ID)this, mmp_buffer_audioframe::DEFAULT_MAX_FRAME_SIZE, mmp_buffer::HEAP);
        if(m_p_buf_as == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT(MMP_CTAG"FAIL: alloc audio stream"), MMP_CNAME));
        }
    }

    
        
    /* create demuxer */
    if(mmpResult == MMP_SUCCESS ) {
        m_pDemuxer = this->CreateDemuxer();
        if(m_pDemuxer == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT(MMP_CTAG"FAIL: CreateDemuxer"), MMP_CNAME));
        }
    }

    
    /* create audio decoder */
    if(mmpResult == MMP_SUCCESS ) {
        m_pDecoderAudio = this->CreateDecoderAudio(m_pDemuxer);
        if(m_pDecoderAudio == NULL) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT(MMP_CTAG"FAIL: CreateDecoderAudio"), MMP_CNAME));
            mmpResult = MMP_FAILURE;
        }
        else {
            mmpResult = m_pDemuxer->GetAudioExtraData(m_p_buf_as);
            if(mmpResult == MMP_SUCCESS) {
                m_p_buf_as->set_flag(mmp_buffer_media::FLAG_CONFIGDATA);
                mmpResult = m_pDecoderAudio->DecodeAu(m_p_buf_as, m_p_buf_af);
                MMPDEBUGMSG(1, (TEXT(MMP_CTAG" ln=%d  mmpResult=%d "), MMP_CNAME, __LINE__, mmpResult ));
            }
        }
    }

    /* create audio render */
    if(mmpResult == MMP_SUCCESS ) {
        m_pRendererAudio = this->CreateRendererAudio(m_pDecoderAudio);
        if(m_pRendererAudio == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT(MMP_CTAG"FAIL: CreateRendererAudio"), MMP_CNAME));
        }
    }

#if (CMmpPlayerAudio_PCM_DUMP == 1)
    if(mmpResult == MMP_SUCCESS ) {
        m_pcm_dump_hdl = pcm_dump_write_create(CMmpPlayerAudio_PCM_FILENAME, m_pRendererAudio->get_sr(), m_pRendererAudio->get_ch(), m_pRendererAudio->get_bps() );
    }
#endif

    return mmpResult;
}


MMP_RESULT CMmpPlayerAudio::Close()
{
    CMmpPlayer::Close();

    CMmpDemuxer::DestroyObject(m_pDemuxer);  m_pDemuxer = NULL;
    CMmpDecoder::DestroyObject(m_pDecoderAudio);  m_pDecoderAudio = NULL;
    CMmpRenderer::DestroyObject(m_pRendererAudio);  m_pRendererAudio = NULL;
    
    if(m_p_buf_as != NULL) { 
        mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_as);
        m_p_buf_as = NULL;
    }
    
    if(m_p_buf_af != NULL) { 
        mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_af);
        m_p_buf_af = NULL;
    }
    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpPlayerAudio::Seek(MMP_S64 pts) {

    //m_pDemuxer->Seek(pts);

    m_seek_pts = pts;
    m_is_seek = MMP_TRUE;

    return MMP_SUCCESS;
}

MMP_S64 CMmpPlayerAudio::GetDuration() {
    return m_pDemuxer->GetDuration();
}

MMP_S64 CMmpPlayerAudio::GetPlayPosition() {
    return m_last_render_pts;
}


void CMmpPlayerAudio::Service()
{
    MMP_U32 start_tick, before_tick, cur_tick, dec_tick, t1, t2;
    MMP_RESULT mmpResult;
    
    CMmpDemuxer* pDemuxer = m_pDemuxer;
    CMmpDecoderAudio* pDecoderAudio = m_pDecoderAudio;
        
    MMP_U32 frame_count = 0, packet_count=0, packet_sub_index = 0;
    MMP_U32 render_size = 0, render_sub_size = 0;

    MMP_TICKS packet_pts=0;
    MMP_S32 render_create_try_count = 0;
    MMP_S32 dec_cnt = 0;
    
    //class mmp_media_audioframe* p_buf_af;

    m_p_buf_as->set_stream_size(0);
    m_p_buf_as->set_stream_offset(0);
    start_tick = CMmpUtil::GetTickCount();
    before_tick = start_tick;
    dec_tick = 0;
    while(m_bServiceRun == MMP_TRUE) {
    
        if(render_create_try_count > 3) {
            break;
        }

        if(m_is_seek == MMP_TRUE) {
            m_pDemuxer->Seek(m_seek_pts);    
            pDecoderAudio->flush_buffer();
            m_is_seek = MMP_FALSE;
        }

        mmpResult = pDemuxer->GetNextAudioDataEx1(m_p_buf_as);
        if(mmpResult == MMP_SUCCESS) {
        
            packet_sub_index = 0;
            render_sub_size = 0;

            while(m_p_buf_as->get_stream_real_size() > 0) {

                
                t1 = CMmpUtil::GetTickCount();
                pDecoderAudio->DecodeAu(m_p_buf_as, m_p_buf_af);
                t2 = CMmpUtil::GetTickCount();
                dec_tick += t2 - t1;
                dec_cnt++;

                static unsigned int decoded_sum = 0;

                decoded_sum += m_p_buf_af->get_data_size();

                MMPDEBUGMSG(1, (TEXT("[AudioPlayer] dec_cnt=%d pts = %lld strmsz=%d strmoffset=%d data_size=%d  sum=%d "), dec_cnt, m_p_buf_as->get_pts(), 
                    m_p_buf_as->get_stream_real_size(), m_p_buf_as->get_stream_offset(), m_p_buf_af->get_data_size(), decoded_sum ));

                if(m_p_buf_af->get_coding_result() == mmp_buffer_media::SUCCESS) {
      
#if (CMmpPlayerAudio_PCM_DUMP == 1)
                    pcm_dump_write_data(m_pcm_dump_hdl, (MMP_U8*)m_p_buf_af->get_buf_vir_addr(), m_p_buf_af->get_data_size() );    
#endif
                    if(m_pRendererAudio == NULL) {
                        m_pRendererAudio = this->CreateRendererAudio(pDecoderAudio);
                        render_create_try_count++;
                    }

                    if(render_create_try_count > 3) {
                        break;
                    }

                    if(m_pRendererAudio != NULL) {
                        m_pRendererAudio->Render(m_p_buf_af);
                    }

                    frame_count++;
                    render_size += m_p_buf_af->get_data_size();
                    render_sub_size += m_p_buf_af->get_data_size();
                    m_last_render_pts = m_p_buf_af->get_pts();

#if 0  /* debug monitoring */
                    cur_tick = CMmpUtil::GetTickCount();
                    MMPDEBUGMSG(1, (TEXT("[CMmpPlayerAudio::Service -- Dec Detail Info ] %d. pktcnt=(%d %d) frc=%d rnsz=(%d %d) pts=(%d %d)"),
                           (cur_tick-start_tick)/1000,
                           packet_count, packet_sub_index,
                           frame_count, 
                           render_size, render_sub_size, 
                           (unsigned int)(m_p_buf_af->get_pts()/1000), (unsigned int)(m_p_buf_af->get_pts()/1000)
                    ));

#endif
                }
                else if(m_p_buf_af->get_coding_result() == mmp_buffer_media::NEED_MORE_DATA) {
                    break;
                }

                packet_sub_index++;
            } /* end of while(stream_buf_size > 0) { */

            packet_count++;
        }
        else {
            m_pRendererAudio->end_of_stream();
        }

        cur_tick = CMmpUtil::GetTickCount();

        if( (cur_tick - before_tick) > 1000) {

            MMPDEBUGMSG(1, (TEXT("[CMmpPlayerAudio::Service] %d. dec_tick=%d frc=%d rnsz=%d pts=%d "),
                           (cur_tick-start_tick)/1000,
                           dec_tick, 
                           frame_count, render_size,
                           (unsigned int)(packet_pts/1000)
                ));

            
            if(this->m_create_config.callback != NULL) {
            
                struct mmp_player_callback_playtime playtime_st;
                MMP_U32 msg;
                void *data1 = NULL, *data2 = NULL;

                msg = CMmpPlayer::CALLBACK_PLAYTIME;
                playtime_st.media_duration  = m_pDemuxer->GetDuration();
                playtime_st.media_pts = m_pRendererAudio->get_real_render_pts();

                //MMPDEBUGMSG(1, (TEXT("[CMmpPlayerAudio::Service] callbac TimeSamp %lld "), playtime_st.media_pts ));

                (*this->m_create_config.callback)(this->m_create_config.callback_privdata, msg, (void*)&playtime_st, NULL);
            }
            before_tick = cur_tick;
            dec_tick = 0;
            frame_count = 0;
            render_size = 0;
        }

    } /* endo fo while(m_bServiceRun == MMP_TRUE) { */
        
}
