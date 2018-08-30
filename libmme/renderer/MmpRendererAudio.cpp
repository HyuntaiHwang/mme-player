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


#include "MmpRendererAudio.hpp"

#include "MmpUtil.hpp"

#if (MMP_PLATFORM == MMP_PLATFORM_WIN32)
//#include "MmpRenderer_WaveOutEx2.hpp"
//#include "MmpRenderer_WaveOutEx3.hpp"
#include "MmpRenderer_WaveOutEx4.hpp"
#include "MmpRenderer_AndroidTinyAlsa.hpp"
#include "MmpRenderer_AndroidAudioFlinger.hpp"

#elif (MMP_PLATFORM == MMP_PLATFORM_ANDROID)
#include "MmpRenderer_AndroidTinyAlsa.hpp"
#include "MmpRenderer_AndroidAudioFlinger.hpp"
#include "MmpRenderer_AndroidAudioTrack.hpp"

#elif (MMP_PLATFORM == MMP_PLATFORM_TIZEN)
#include "MmpRenderer_ALSA.hpp"

#else
#error "ERROR : Select OS"
#endif


/////////////////////////////////////////////////////////////
// create/destory object
CMmpRendererAudio* CMmpRendererAudio::CreateObject(struct CMmpRendererAudio::create_config *p_create_config) {

    CMmpRendererAudio* pObj = NULL;
        
#if (MMP_PLATFORM == MMP_PLATFORM_WIN32)
    //pObj = new CMmpRenderer_WaveOutEx2(p_create_config);
    //pObj = new CMmpRenderer_WaveOutEx3(p_create_config);
    pObj = new CMmpRenderer_WaveOutEx4(p_create_config);
            
#elif (MMP_PLATFORM == MMP_PLATFORM_ANDROID)
    pObj = new android::CMmpRenderer_AndroidAudioTrack(p_create_config);

#elif (MMP_PLATFORM == MMP_PLATFORM_BUILDROOT)
    pObj = NULL;

#elif (MMP_PLATFORM == MMP_PLATFORM_TIZEN)
    pObj = new CMmpRenderer_ALSA(p_create_config);

#else
#error "ERROR : Select Platform"
#endif

    if(pObj != NULL) {
        if(pObj->Open() != MMP_SUCCESS) {
            pObj->Close();
            delete pObj;
            pObj = NULL;
        }
    }

    return pObj;
}


/////////////////////////////////////////////////////////////
// class CMmpRendererAudio

CMmpRendererAudio::CMmpRendererAudio(CMmpRendererAudio::create_config *p_create_config, enum MMP_FOURCC fourcc_in) : 

CMmpRenderer(MMP_MEDIATYPE_AUDIO, fourcc_in)

,m_sr(p_create_config->sr)
,m_ch(p_create_config->ch)
,m_bps(p_create_config->bps)

{
    
}

CMmpRendererAudio::~CMmpRendererAudio() {
 
}

MMP_RESULT CMmpRendererAudio::Open() {

    MMP_RESULT mmpResult; 

    mmpResult = CMmpRenderer::Open();

    return mmpResult;
}

MMP_RESULT CMmpRendererAudio::Close() {

    MMP_RESULT mmpResult; 

    mmpResult = CMmpRenderer::Close();

    //this->vf_flush();
    return mmpResult;
}


void* mmp_render_audio_create(int sr, int ch, int bps)
{
    CMmpRendererAudio* pRendererAudio = NULL;
    CMmpRendererAudio::create_config renderer_audio_create_config;

    renderer_audio_create_config.sr = sr;
	renderer_audio_create_config.ch = ch;
	renderer_audio_create_config.bps = bps;
	pRendererAudio = CMmpRendererAudio::CreateObject(&renderer_audio_create_config);

    return (void*)pRendererAudio;
}

int mmp_render_audio_destroy(void* hdl) {

	CMmpRenderer* pRenderer = (CMmpRenderer*)hdl;
	
	if(pRenderer)
	{
		CMmpRenderer::DestroyObject(pRenderer);
	}

	return 0;
}

int mmp_render_audio_write(void* hdl, char* data, int datasize)
{
	CMmpRenderer* pRenderer = (CMmpRenderer*)hdl;
	class mmp_buffer_audioframe* p_buf_af;

    p_buf_af = mmp_buffer_mgr::get_instance()->attach_media_audioframe(MEDIA_ID_UNKNOWN, (MMP_ADDR)data, datasize);
    if(p_buf_af != NULL) {
        p_buf_af->set_data_size(datasize);
        if(pRenderer!=NULL) {
		    pRenderer->Render(p_buf_af);
	    }

        mmp_buffer_mgr::get_instance()->free_media_buffer(p_buf_af);
    }

	return 0;
}

