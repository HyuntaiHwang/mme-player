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


#include "MmpPlayerTONE.hpp"
#include "MmpUtil.hpp"

/////////////////////////////////////////////////////////////
//CMmpPlayerTONE Member Functions



CMmpPlayerTONE::CMmpPlayerTONE(CMmpPlayerCreateProp* pPlayerProp) : CMmpPlayer(pPlayerProp, "TONE")

,m_pRendererAudio(NULL)
,m_pcm_buffer(NULL)
,m_pcm_buffer_max_size(1024*1024)
{
    

}

CMmpPlayerTONE::~CMmpPlayerTONE()
{
    
    
}

MMP_RESULT CMmpPlayerTONE::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;

    //CMmpRendererCreateProp RendererProp;
    //CMmpRendererCreateProp* pRendererProp=&RendererProp; 

    
    /* create pcm buffer */
    if(mmpResult == MMP_SUCCESS ) {
        
        m_pcm_buffer = (MMP_U8*)malloc(m_pcm_buffer_max_size);
        if(m_pcm_buffer == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else { /* Generater PCM Buffer */
            this->generate_pcm_buffer();        
        }
    }


    /* create audio render */
    if(mmpResult == MMP_SUCCESS ) {
        CMmpRendererAudio::create_config renderer_audio_create_config;
        m_pRendererAudio = CMmpRendererAudio::CreateObject(&renderer_audio_create_config);
        if(m_pRendererAudio == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {
    
     
        }
    }


    return mmpResult;
}


MMP_RESULT CMmpPlayerTONE::Close()
{
    CMmpPlayer::Close();

    if(m_pRendererAudio != NULL) {
        CMmpRenderer::DestroyObject(m_pRendererAudio);  
        m_pRendererAudio = NULL;

        CMmpUtil::Sleep(3000);
    }
    
    if(m_pcm_buffer!=NULL) 
    { 
        free(m_pcm_buffer); 
        m_pcm_buffer = NULL; 
    }
    
    return MMP_SUCCESS;
}

void CMmpPlayerTONE::generate_pcm_buffer() {

    
    #define PCM_32BIT_COUNT 48
//#define STREAM_FRAME_COUNT 20
//#define STREAM_BUF_SIZE ((PCM_32BIT_COUNT<<2)*STREAM_FRAME_COUNT)

    const MMP_U32 PCM_48kHz[PCM_32BIT_COUNT]=
    {
       0x00000000U, 0x08870887U, 0x10EA10EAU, 0x19031903U, 0x20AD20ADU, 0x27c927c9U, 0x2e372e37U, 0x33DA33DAU,
       0x389a389aU, 0x3c623c62U, 0x3f213f21U, 0x40cc40ccU, 0x415c415cU, 0x40cc40ccU, 0x3f213f21U, 0x3c623c62U,
       0x389a389aU, 0x33DA33DAU, 0x2e372e37U, 0x27c927c9U, 0x20AD20ADU, 0x19031903U, 0x10EA10EAU, 0x08870887U,
       0x00000000U, 0xF779F779U, 0xEF16EF16U, 0xE6FDE6FDU, 0xDF53DF53U, 0xD837D837U, 0xD1C9D1C9U, 0xCC26CC26U, 
       0xC766C766U, 0xC39EC39EU, 0xC0DFC0DFU, 0xBF34BF34U, 0xBEA4BEA4U, 0xBF34BF34U, 0xC0DFC0DFU, 0xC39EC39EU,
       0xC766C766U, 0xCC26CC26U, 0xD1C9D1C9U, 0xD837D837U, 0xDF53DF53U, 0xE6FDE6FDU, 0xEF16EF16U, 0xF779F779U
    };

    //MMP_S32 skip_count;
    MMP_S32 cur_size, desired_size;
    MMP_U32 *p_pcm_u32;
    MMP_S32 next_idx;

#if 0
    switch(this->m_create_config.audio_wf.nSamplesPerSec) {
    
        case 48000 :
            skip_count = 0;
            break;
    
        case 8000:
            skip_count = 5;
            break;

        default :
            skip_count = 0;
    }
#endif

    desired_size = PCM_32BIT_COUNT*100;//this->m_create_config.audio_wf.nAvgBytesPerSec/4;
    p_pcm_u32 = (MMP_U32*)m_pcm_buffer;
    next_idx = 0;
    for(cur_size = 0; cur_size < desired_size; cur_size++) {
        
        p_pcm_u32[cur_size] = PCM_48kHz[next_idx];
        next_idx += 1;
        next_idx %= PCM_32BIT_COUNT;
    }
    m_pcm_buffer_size = cur_size*4;

}

#define STREAM_FRAME_COUNT 20
#define STREAM_BUF_SIZE ((PCM_32BIT_COUNT<<2)*STREAM_FRAME_COUNT)

void CMmpPlayerTONE::Service()
{
    CMmpRenderer* pRendererAudio = m_pRendererAudio;
    MMP_S32 pcm_size=(PCM_32BIT_COUNT<<2)*100, pcm_index = 0;

    while(m_bServiceRun == MMP_TRUE) {

        //memset(m_pcm_buffer, 0xFF, pcm_size/2);
        //memset(&m_pcm_buffer[pcm_size/2], 0xFF, pcm_size/2);
#if 0
        pRendererAudio->RenderPCM(m_pcm_buffer, pcm_size);
#else
            CMmpUtil::Sleep(100);  
#endif

        pcm_index += pcm_size;

        if(pcm_index >= m_pcm_buffer_size) {
            pcm_index = 0;
        }
        CMmpUtil::Sleep(10);
    } /* endo fo while(m_bServiceRun == MMP_TRUE) { */
 
}

