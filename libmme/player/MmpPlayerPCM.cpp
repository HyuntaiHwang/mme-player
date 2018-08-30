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


#include "MmpPlayerPCM.hpp"
#include "MmpUtil.hpp"

/////////////////////////////////////////////////////////////
//CMmpPlayerPCM Member Functions

CMmpPlayerPCM::CMmpPlayerPCM(CMmpPlayerCreateProp* pPlayerProp) : CMmpPlayer(pPlayerProp, "PCM")

,m_pcm_hdl(NULL)
,m_pRendererAudio(NULL)
,m_p_buf_af(NULL)
{
    

}

CMmpPlayerPCM::~CMmpPlayerPCM()
{
    
    
}

MMP_RESULT CMmpPlayerPCM::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;

    //CMmpRendererCreateProp RendererProp;
    //CMmpRendererCreateProp* pRendererProp=&RendererProp; 

    /* create pcm file */
    if(mmpResult == MMP_SUCCESS ) {
        m_pcm_hdl = pcm_dump_read_create(this->m_create_config.filename);
        if(m_pcm_hdl == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {
            m_play_duration = pcm_dump_get_play_dur(m_pcm_hdl);
        }
    }
    
    /* create pcm buffer */
    if(mmpResult == MMP_SUCCESS ) {
        m_p_buf_af = mmp_buffer_mgr::get_instance()->alloc_media_audioframe((MMP_MEDIA_ID)this, mmp_buffer_audioframe::DEFAULT_MAX_FRAME_SIZE, mmp_buffer::HEAP);
        if(m_p_buf_af == NULL) {
            mmpResult = MMP_FAILURE;
        }
    }


    /* create audio render */
    if(mmpResult == MMP_SUCCESS ) {
        CMmpRendererAudio::create_config renderer_audio_create_config;
        renderer_audio_create_config.sr = pcm_dump_get_sr(m_pcm_hdl);
        renderer_audio_create_config.ch = pcm_dump_get_ch(m_pcm_hdl);
        renderer_audio_create_config.bps = pcm_dump_get_bps(m_pcm_hdl);
        m_pRendererAudio = CMmpRendererAudio::CreateObject(&renderer_audio_create_config);
        if(m_pRendererAudio == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {
    
     
        }
    }


    return mmpResult;
}


MMP_RESULT CMmpPlayerPCM::Close()
{
    CMmpPlayer::Close();

    if(m_pRendererAudio != NULL) {
        CMmpRenderer::DestroyObject(m_pRendererAudio);  
        m_pRendererAudio = NULL;

        CMmpUtil::Sleep(1000);
    }
    
    if(m_p_buf_af!=NULL)  { 
        mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_af);
        m_p_buf_af = NULL; 
    }
    
    if(m_pcm_hdl!=NULL) { 
        pcm_dump_destroy(m_pcm_hdl);
        m_pcm_hdl = NULL; 
    }
     
    return MMP_SUCCESS;

}

void CMmpPlayerPCM::Service()
{
    MMP_S64 start_tick, before_tick, cur_tick;
    
    CMmpRenderer* pRendererAudio = m_pRendererAudio;
        
    MMP_S32 readsz;

    MMP_S64 packet_pts=0, last_render_pts=0;
    MMP_S64 timestamp_weight = 0;

    
    start_tick = CMmpUtil::GetTickCountUS();
    before_tick = start_tick;
    while(m_bServiceRun == MMP_TRUE) {
    
        pcm_dump_read_data(m_pcm_hdl, (MMP_U8*)m_p_buf_af->get_buf_vir_addr(), 4096, &readsz);
        m_p_buf_af->set_data_size(readsz);
        if(readsz > 0) {
            pRendererAudio->Render(m_p_buf_af);
            /* Note!!  Audio need not go to  Sleep */
            //pcm_dur = ((MMP_S64)readsz*1000000L) / (MMP_S64)pcm_dump_get_abps(m_pcm_hdl);
            //packet_pts += pcm_dur;
            //CMmpUtil::Sleep( (MMP_U32)pcm_dur/1000); 
        }
        
        cur_tick = CMmpUtil::GetTickCountUS();
        if( (cur_tick - before_tick) > (1000LL*1000LL) ) {
        
            if(this->m_create_config.callback != NULL) {

                struct mmp_player_callback_playtime playtime_st;
                MMP_U32 msg;
                
                msg = CMmpPlayer::CALLBACK_PLAYTIME;
                playtime_st.media_duration  = this->m_play_duration;
                playtime_st.media_pts = packet_pts;

                (*this->m_create_config.callback)(this->m_create_config.callback_privdata, msg, (void*)&playtime_st, NULL);
            }

            before_tick = cur_tick;
        }
    } /* endo fo while(m_bServiceRun == MMP_TRUE) { */
 
}

