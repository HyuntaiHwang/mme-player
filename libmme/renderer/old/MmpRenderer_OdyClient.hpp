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

#ifndef MMPRENDERER_ODYCLIENT_HPP__
#define MMPRENDERER_ODYCLIENT_HPP__

#include "MmpRenderer.hpp"

#if 1//(MMP_OS == MMP_OS_LINUX)

#include "gdm_fb.h"
#include "gdm-msgio.h"

struct ody_videofile {
	int fd;		/* file handler */
	int width;
	int height;
	int format;
};

struct ody_videoframe {
	int shared_fd[3];
	unsigned char *address[3];	/* virtual address */
	int size[3];
};

struct ody_framebuffer {
	struct gdm_dss_overlay request;
	struct gdm_dss_overlay_data buffer[2];
	int buffer_index;
};


struct ody_player {
    struct fb_var_screeninfo vi;
	struct ody_framebuffer fb_info;
	struct ody_videofile video_info;
	struct ody_videoframe frame[2];

	int release_fd;
	int buf_index;
	int stop;
};

class CMmpRenderer_OdyClient : public CMmpRenderer
{
friend class CMmpRenderer;

private:
    int m_sock_fd;
    int m_buf_idx;
    int m_luma_size, m_chroma_size;
    
    struct ody_player m_gplayer;

    struct gdm_dss_overlay m_req;
	struct gdm_dss_overlay_data m_req_data;


protected:
    CMmpRenderer_OdyClient(CMmpRendererCreateProp* pRendererProp);
    virtual ~CMmpRenderer_OdyClient();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

public:
    virtual MMP_RESULT Render(CMmpMediaSampleDecodeResult* pDecResult);
    virtual MMP_RESULT RenderYUV420Planar(MMP_U8* Y, MMP_U8* U, MMP_U8* V, MMP_U32 buffer_width, MMP_U32 buffer_height);
    virtual MMP_RESULT RenderYUV420Planar_Memory(MMP_U8* Y, MMP_U8* U, MMP_U8* V, MMP_U32 buffer_width, MMP_U32 buffer_height);
    virtual MMP_RESULT RenderYUV420Planar_Ion(MMP_U8* Y, MMP_U8* U, MMP_U8* V, MMP_U32 buffer_width, MMP_U32 buffer_height);
    
    
};


#endif
#endif
