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

#ifndef _MMPRENDERER_HPP__
#define _MMPRENDERER_HPP__

#include "MmpDefine.h"
#include "MmpPlayerDef.h"
#include "MmpEncoderVideo.hpp"
#include "MmpMuxer.hpp"
#include "mmp_buffer_videoframe.hpp"
#include "mmp_buffer_mgr.hpp"

class CMmpRenderer
{
public:
    enum {
        AUDIO_RENDER_DEFUALT = 0,
        AUDIO_RENDER_WIN32_WAVE_OUT
    };
public:
    
	
    static MMP_RESULT DestroyObject(CMmpRenderer* pObj);

protected:
    static CMmpRenderer* s_pFirstRenderer[MMP_MEDIATYPE_MAX];

protected:
    enum MMP_MEDIATYPE m_MediaType;
        
private:
    enum MMP_FOURCC m_fourcc_in;
    
    MMP_S64 m_real_render_pts;
    MMP_S64 m_real_render_pts_set_tick;
    
    CMmpEncoderVideo* m_pVideoEncoder;
    CMmpMuxer* m_pMuxer;
    class mmp_buffer_videostream* m_p_buf_videostream_enc;
    
protected:
    CMmpRenderer(enum MMP_MEDIATYPE mt, enum MMP_FOURCC fourcc_in);
    virtual ~CMmpRenderer();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

protected:
    
        
protected:
    MMP_RESULT EncodeAndMux(class mmp_buffer_videoframe* p_buf_videoframe);
    void set_real_render_pts(MMP_S64 pts);

public:
    inline enum MMP_FOURCC get_fourcc_in() { return m_fourcc_in; }
    MMP_S64 get_real_render_pts();
    virtual void end_of_stream() { };

    virtual MMP_RESULT OnSize(int cx, int cy) { return MMP_FAILURE; }

    virtual void SetFirstRenderer() { CMmpRenderer::s_pFirstRenderer[m_MediaType] = this; }
    MMP_BOOL IsFirstRenderer() { return (CMmpRenderer::s_pFirstRenderer[m_MediaType] == this)?MMP_TRUE:MMP_FALSE; }
    virtual void SetRotate(enum MMP_ROTATE rotate) {};
    virtual enum MMP_ROTATE GetRotate() { return MMP_ROTATE_0; }
    
    
    virtual MMP_RESULT Render(class mmp_buffer_audioframe* p_buf_audioframe) { return MMP_FAILURE; }
    
};



#endif

