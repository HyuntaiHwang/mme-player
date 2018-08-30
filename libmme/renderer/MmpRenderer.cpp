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


#include "MmpRenderer.hpp"

#include "MmpUtil.hpp"

//////////////////////////////////////////////////////////////
// CMmpRenderer CreateObject/DestroyObject

MMP_RESULT CMmpRenderer::DestroyObject(CMmpRenderer* pObj)
{
    if(pObj)
    {
        pObj->Close();
        delete pObj;
    }
    return MMP_SUCCESS;
}

#define ENC_STREAM_MAX_SIZE (1024*1024*2)
/////////////////////////////////////////////////////////////
//CMmpRenderer Member Functions

CMmpRenderer* CMmpRenderer::s_pFirstRenderer[MMP_MEDIATYPE_MAX] = { NULL, NULL };

CMmpRenderer::CMmpRenderer(enum MMP_MEDIATYPE mt, enum MMP_FOURCC fourcc_in) :

m_MediaType(mt)
,m_fourcc_in(fourcc_in)
,m_real_render_pts(-1000)

,m_pVideoEncoder(NULL)
,m_pMuxer(NULL)
,m_p_buf_videostream_enc(NULL)
{
	
    CMmpRenderer::s_pFirstRenderer[m_MediaType] = this;
}

CMmpRenderer::~CMmpRenderer()
{

}

MMP_RESULT CMmpRenderer::Open()
{
#if 0
    struct MmpEncoderCreateConfig *pEncoderCreateConfig;
    struct MmpMuxerCreateConfig muxer_create_config;

    if(m_pRendererProp->m_bVideoEncoder == MMP_TRUE) {

        /*Create Video Encoder */
        pEncoderCreateConfig = &m_pRendererProp->m_VideoEncoderCreateConfig;
        pEncoderCreateConfig->nPicWidth = this->m_pRendererProp->m_iPicWidth;
        pEncoderCreateConfig->nPicHeight = this->m_pRendererProp->m_iPicHeight;
        m_pVideoEncoder = (CMmpEncoderVideo*)CMmpEncoder::CreateVideoObject(pEncoderCreateConfig, m_pRendererProp->m_bVideoEncoderForceSWCodec);
                
        /*Create Muxer */
        if(m_pVideoEncoder != NULL) {

            memset(&muxer_create_config, 0x00, sizeof(muxer_create_config));
            strcpy((char*)muxer_create_config.filename, m_pRendererProp->m_VideoEncFileName);

            muxer_create_config.bMedia[MMP_MEDIATYPE_VIDEO] = MMP_TRUE;
            muxer_create_config.bih.biSize = sizeof(MMPBITMAPINFOHEADER);
            muxer_create_config.bih.biCompression = m_pVideoEncoder->GetFormat();
            muxer_create_config.bih.biWidth = m_pVideoEncoder->GetVideoPicWidth();
            muxer_create_config.bih.biHeight = m_pVideoEncoder->GetVideoPicHeight();
            muxer_create_config.video_bitrate = 1024*1024*4;
            muxer_create_config.video_fps = 24;
            muxer_create_config.video_idr_period = 10;
            
            m_pMuxer = CMmpMuxer::CreateObject(&muxer_create_config);
        }

        /* create stream buffer */
        m_p_buf_videostream_enc = mmp_buffer_mgr::get_instance()->alloc_media_videostream(ENC_STREAM_MAX_SIZE, mmp_buffer::HEAP);
    }
#endif

    return MMP_SUCCESS;
}


MMP_RESULT CMmpRenderer::Close()
{
    if(m_pMuxer != NULL) {
        CMmpMuxer::DestroyObject(m_pMuxer);
        m_pMuxer = NULL;
    }

    if(m_pVideoEncoder != NULL) {
        CMmpEncoder::DestroyObject(m_pVideoEncoder);
        m_pVideoEncoder = NULL;
    }

    /* destoy stream buffer */
    if(m_p_buf_videostream_enc != NULL) {
        mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_videostream_enc);
        m_p_buf_videostream_enc = NULL;
    }

    return MMP_SUCCESS;
}

void CMmpRenderer::set_real_render_pts(MMP_S64 pts) { 
      m_real_render_pts = pts; 
      m_real_render_pts_set_tick = CMmpUtil::GetTickCountUS(); 
}

MMP_S64 CMmpRenderer::get_real_render_pts() { 
    
#if 0
    MMP_S64 cur_tick = CMmpUtil::GetTickCountUS(); 
    MMP_S64 pts = -1000;

    if(m_real_render_pts >= 0) {
        pts =  m_real_render_pts + (cur_tick - m_real_render_pts_set_tick);
    }

    return pts;
#else

    return m_real_render_pts;
#endif
}

MMP_RESULT CMmpRenderer::EncodeAndMux(class mmp_buffer_videoframe* p_buf_videoframe) {

    MMP_S64 pts;
    const MMP_S64 SECOND = 1000000L;
    const MMP_S64 FPS = 24;
    const MMP_S64 DurPerSecond =  SECOND/FPS;
    
    if( (m_pVideoEncoder != NULL) && (m_pMuxer != NULL) && (m_p_buf_videostream_enc!=NULL) ) {
        
        pts = m_p_buf_videostream_enc->get_pts();
        pts += DurPerSecond;
        p_buf_videoframe->set_pts(pts);
        
        m_pVideoEncoder->EncodeAu(p_buf_videoframe, m_p_buf_videostream_enc);
        m_pMuxer->AddMediaData(m_p_buf_videostream_enc);
    }

    return MMP_SUCCESS;
}

