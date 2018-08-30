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

#ifndef MMPRENDERER_ODYFPGADISPLAY_HPP__
#define MMPRENDERER_ODYFPGADISPLAY_HPP__

#include "MmpRenderer.hpp"

#if (MMP_OS == MMP_OS_LINUX)


#include <time.h>
#include <inttypes.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <linux/types.h>
#include <linux/stat.h>
#include <ion_api.h>
#include <fcntl.h>

//#include "define.h"
#include "gdm_fb.h"
//#include "ipc_msg.h"

#define FRAMEBUFFER_NAME	"/dev/fb0"

#define VIDEO_WIDTH		1280
#define VIDEO_HEIGHT		 720
#define VIDEO_FORMAT		GDMFB_YUV420P3

#define FRAMEBUF_MAX_COUNT 2

struct ody_videofile {
	int fd;		/* file handler */
	int width;
	int height;
	int format;
};

struct ody_videoframe {
	int shared_fd;
	unsigned char *address;	/* virtual address */
	int size;
};


struct ody_framebuffer {
	int fd;
	struct fb_var_screeninfo vi;
	struct fb_fix_screeninfo fi;

	struct gdm_dss_overlay request;
	struct gdm_dss_overlay_data buffer[FRAMEBUF_MAX_COUNT];

	int buffer_index;
};


struct ody_player {
	struct ody_framebuffer fb_info;
	struct ody_videofile video_info;
	struct ody_videoframe frame[FRAMEBUF_MAX_COUNT];

	int buf_index;

	int renderer_stop;
	int stop;
};

#define MMPRENDERER_ODYFPGA_DUMP 0
class CMmpRenderer_OdyFpgaDisplay : public CMmpRenderer
{
friend class CMmpRenderer;

private:
	MMP_S32 m_fd_framebuffer;

	struct ody_player gplayer;
	int m_buf_ndx;

	int m_luma_size;
	int m_chroma_size;

	struct gdm_dss_overlay_data m_req_data;
	struct gdm_dss_overlay m_request;

    FILE* m_fp_dump;

protected:
    CMmpRenderer_OdyFpgaDisplay(CMmpRendererCreateProp* pRendererProp);
    virtual ~CMmpRenderer_OdyFpgaDisplay();

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
