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


#include "MmpRendererVideo.hpp"

#include "MmpUtil.hpp"

#if (MMP_PLATFORM == MMP_PLATFORM_WIN32)
#include "MmpRenderer_OpenGLEx2.hpp"

#elif (MMP_PLATFORM == MMP_PLATFORM_ANDROID)

//#include "MmpRenderer_AndroidSurfaceEx1.hpp"
//#include "MmpRenderer_AndroidSurfaceEx2.hpp"
#include "MmpRenderer_AndroidSurfaceEx3.hpp"
#include "MmpRenderer_AndroidTinyAlsa.hpp"
#include "MmpRenderer_AndroidAudioFlinger.hpp"
#include "MmpRenderer_AndroidAudioTrack.hpp"

#elif (MMP_PLATFORM == MMP_PLATFORM_BUILDROOT)

#if (MMP_BOARD == MMP_BOARD_ODY_FPGA)
#include "MmpRenderer_OdyClientEx2.hpp"
#endif

#elif (MMP_PLATFORM == MMP_PLATFORM_TIZEN)

#include "MmpRenderer_ALSA.hpp"

#else
#error "ERROR : Select Platform"
#endif

#include "MmpRenderer_YUVWriter.hpp"
#include "MmpRenderer_DummyVideo.hpp"


/////////////////////////////////////////////////////////////
// create/destory object

CMmpRendererVideo* CMmpRendererVideo::CreateObject(struct CMmpRendererVideo::create_config* p_create_config) {
    
    CMmpRendererVideo* pObj = NULL;

    MMPDEBUGMSG(1, (TEXT("[CMmpRendererVideo::CreateObject] p_create_config->id=%d "), p_create_config->id));
        

    if(p_create_config->id == CMmpRendererVideo::ID_DISPLAY) {

        MMPDEBUGMSG(1, (TEXT("[CMmpRendererVideo::CreateObject] ln=%d DISPLAY p_create_config->id=%d "), __LINE__, p_create_config->id));

    #if (MMP_PLATFORM == MMP_PLATFORM_WIN32)
        pObj=new CMmpRenderer_OpenGLEx2(p_create_config);
        
    #elif (MMP_PLATFORM == MMP_PLATFORM_ANDROID)
        pObj=new CMmpRenderer_AndroidSurfaceEx3(p_create_config);
            
    #elif (MMP_PLATFORM == MMP_PLATFORM_BUILDROOT)
        pObj=new CMmpRenderer_OdyClientEx2(p_create_config);

    #elif (MMP_PLATFORM == MMP_PLATFORM_TIZEN)
        pObj=new CMmpRenderer_YUVWriter(p_create_config);

    #else
    #error "ERROR : Select Platform to create VideoRenderer"
    #endif

    }
    else if(p_create_config->id == CMmpRendererVideo::ID_YUV_WRITER) {

        MMPDEBUGMSG(1, (TEXT("[CMmpRendererVideo::CreateObject] ln=%d YUV p_create_config->id=%d "), __LINE__, p_create_config->id));

        pObj=new CMmpRenderer_YUVWriter(p_create_config);    
    }
    else if(p_create_config->id == CMmpRendererVideo::ID_DUMMY) {

        MMPDEBUGMSG(1, (TEXT("[CMmpRendererVideo::CreateObject] ln=%d DUMMY p_create_config->id=%d "), __LINE__, p_create_config->id));

        pObj=new CMmpRenderer_DummyVideo(p_create_config);    
    }


    if(pObj!=NULL) {
        if(pObj->Open() !=MMP_SUCCESS) {
            pObj->Close();
            delete pObj;
            pObj = NULL;
        }
    }

    return pObj;
}

/////////////////////////////////////////////////////////////
// class CMmpRendererVideo

CMmpRendererVideo::CMmpRendererVideo(struct CMmpRendererVideo::create_config* p_create_config, enum MMP_FOURCC fourcc_in) : CMmpRenderer(MMP_MEDIATYPE_VIDEO, fourcc_in)
    ,m_vf_count(0)
    ,m_pic_width(0)
    ,m_pic_height(0)
{
    MMP_S32 i;

    for(i = 0; i < MAX_VF_COUNT; i++) {
        m_p_vf[i] = NULL;
    }
        
}

CMmpRendererVideo::~CMmpRendererVideo() {
        
    this->vf_flush();
}

MMP_RESULT CMmpRendererVideo::Open() {

    MMP_RESULT mmpResult; 

    mmpResult = CMmpRenderer::Open();

    return mmpResult;
}

MMP_RESULT CMmpRendererVideo::Close() {

    MMP_RESULT mmpResult; 

    mmpResult = CMmpRenderer::Close();

    this->vf_flush();

    return mmpResult;
}

MMP_RESULT CMmpRendererVideo::vf_flush(void) {

    MMP_S32 i;

    for(i = 0; i < MAX_VF_COUNT; i++) {
        if(m_p_vf[i] != NULL) {
            mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_vf[i]);
            m_p_vf[i] = NULL;
        }
    }
    m_vf_count = 0;

    return MMP_SUCCESS;
}

MMP_RESULT CMmpRendererVideo::vf_add(class mmp_buffer_videoframe* p_vf) {

    MMP_RESULT mmpResult = MMP_FAILURE;

    if( m_vf_count < MAX_VF_COUNT ) {

        m_p_vf[m_vf_count] = p_vf;
        
        m_vf_count++;
        mmpResult = MMP_SUCCESS;
    }

    return mmpResult;
}

MMP_RESULT CMmpRendererVideo::vf_clear_own(MMP_S32 fd) {

    return this->vf_set_own(fd, MEDIA_ID_UNKNOWN);
}

MMP_RESULT CMmpRendererVideo::vf_set_own(MMP_S32 fd, MMP_MEDIA_ID id) {

    MMP_S32 i;

    for(i = 0; i < m_vf_count; i++) {
        if(m_p_vf[i] != NULL) {
            if(m_p_vf[i]->get_buf_shared_fd() == fd) {
                m_p_vf[i]->set_own(id);
                break;
            }
        }
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpRendererVideo::vf_config_internal(MMP_S32 bufcnt, MMP_S32 pic_width, MMP_S32 pic_height, struct mmp_rect display_crop_rect) {
    
    MMP_S32 i;
    class mmp_buffer_videoframe* p_buf_vf;
        
    for(i = 0; i < bufcnt; i++) {
        p_buf_vf = mmp_buffer_mgr::get_instance()->alloc_media_videoframe((MMP_MEDIA_ID)this, pic_width, pic_height, this->get_fourcc_in(), mmp_buffer::ION, mmp_buffer::FLAG_NONE ); 
        if(p_buf_vf != NULL) {
            this->vf_add(p_buf_vf);
        }
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpRendererVideo::vf_config(MMP_S32 bufcnt, MMP_S32 pic_width, MMP_S32 pic_height)  {

    struct mmp_rect display_crop_rect;
    display_crop_rect.left = 0;
    display_crop_rect.top = 0;
    display_crop_rect.right = pic_width;
    display_crop_rect.bottom = pic_height;
    return this->vf_config(bufcnt, pic_width, pic_height, display_crop_rect);
}

MMP_RESULT CMmpRendererVideo::vf_config(MMP_S32 bufcnt, MMP_S32 pic_width, MMP_S32 pic_height, struct mmp_rect display_crop_rect) { 

    MMP_RESULT mmpResult;  
    this->vf_flush();     
    mmpResult = this->vf_config_internal(bufcnt, pic_width, pic_height, display_crop_rect);
    if(mmpResult == MMP_SUCCESS) {
        m_pic_width = pic_width;
        m_pic_height = pic_height;
    }
    return mmpResult;
}


#ifdef WIN32

static CMmpRendererCreateProp s_last_renderprop;
static int s_jpeg_dump = 0;
static int s_jpeg_index = 0;
#define JPEG_FILE_PATH "d:\\work\\jpeg"

void mmp_render_video_init(void* hwnd, void* hdc, 
					  int boardwidth, int boardheight, 
					  int scrx, int scry, int scrwidht, int scrheight, 
					  int picwidht, int picheight,
                      int jpeg_dump
                      )
{
	CMmpRendererCreateProp renderprop;
	
    s_jpeg_dump = jpeg_dump;
    s_jpeg_index = 0;
	
	renderprop.m_hRenderWnd = hwnd;
	renderprop.m_hRenderDC = hdc;
	renderprop.m_iBoardWidth = boardwidth;
	renderprop.m_iBoardHeight = boardheight;
	renderprop.m_iScreenPosX = scrx;
	renderprop.m_iScreenPosY = scry;
	renderprop.m_iScreenWidth = scrwidht;
	renderprop.m_iScreenHeight = scrheight;
	renderprop.m_iPicWidth = picwidht;
	renderprop.m_iPicHeight = picheight;
	renderprop.pic_format = MMP_FOURCC_IMAGE_YUV420M;
	
	s_last_renderprop = renderprop;
	
}


void* mmp_render_video_create(int pic_width, int pic_height, int rotate_degree)
{
    CMmpRendererVideo* pRendererVideo;
    CMmpRendererVideo::create_config renderer_video_create_config;
    struct mmp_rect rect;

#if (MMP_OS == MMP_OS_WIN32)
    renderer_video_create_config.hRenderWnd = s_last_renderprop.m_hRenderWnd;
    renderer_video_create_config.hRenderDC = s_last_renderprop.m_hRenderDC;
#endif
    renderer_video_create_config.id = CMmpRendererVideo::ID_DISPLAY;
    //renderer_video_create_config.id = CMmpRendererVideo::ID_YUV_WRITER;

    pRendererVideo = CMmpRendererVideo::CreateObject(&renderer_video_create_config);
    if(pRendererVideo) {
        rect.left = 0;
        rect.top = 0;
        rect.right = pic_width;
        rect.bottom = pic_height;
        pRendererVideo->vf_config(10, pic_width, pic_height, rect); 
    }

	return (void*)pRendererVideo;
}


int mmp_render_video_destroy(void* hdl)
{
	CMmpRenderer* pRenderer = (CMmpRenderer*)hdl;
	
	if(pRenderer)
	{
		CMmpRenderer::DestroyObject(pRenderer);
	}
	return 0;
}

int mmp_render_video_write(void* hdl, char* data, int datasize)
{
#if 1
	CMmpRendererVideo* pRenderer = (CMmpRendererVideo*)hdl;
    MMP_ADDR frame_addr;
    MMP_S32 pic_width, pic_height, frame_stride, frame_alignheight;
    class mmp_buffer_videoframe* p_buf_videoframe;

    if(pRenderer != NULL) {
        static int cnt = 0;
        //MMPDEBUGMSG(1, (TEXT("wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww cnt=%d ts=%d \n"), cnt , cnt/30 ));
        cnt++;

        frame_addr = (MMP_ADDR)data;
        pic_width = pRenderer->get_pic_width();
        pic_height = pRenderer->get_pic_height();
        frame_stride = pic_width;
        frame_alignheight = pic_height;

        p_buf_videoframe = mmp_buffer_mgr::get_instance()->attach_media_videoframe(0, (MMP_ADDR)data, pic_width, pic_height, MMP_FOURCC_IMAGE_YUV420, MMP_BUFTYPE_SW);

	    pRenderer->Render(p_buf_videoframe, 0);

#if 0
        if(s_jpeg_dump) {
            char jpegfilename[256];
            unsigned char* Y, *U, *V;
            
            Y = (unsigned char*)data;
            U = Y + (pic_width*pic_height);
            V = U + (pic_width*pic_height)/4;

            sprintf(jpegfilename, "%s\\mydump%02d.jpg", JPEG_FILE_PATH, s_jpeg_index);
            CMmpUtil::Jpeg_SW_YUV420Planar_Enc(Y, U, V, pic_width, pic_height, jpegfilename, 100);

            s_jpeg_index++;
        }
#endif

        mmp_buffer_mgr::get_instance()->free_media_buffer(p_buf_videoframe);

    }

    
#endif

	return 0;
}


#endif
