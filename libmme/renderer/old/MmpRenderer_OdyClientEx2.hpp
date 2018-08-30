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

#ifndef MMPRENDERER_ODYCLIENTEX2_HPP__
#define MMPRENDERER_ODYCLIENTEX2_HPP__

#include "MmpRenderer.hpp"
#include "mmp_msg_proc.hpp"
#include "mmp_oal_mutex.hpp"

/*
    add rotate function 
*/

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
	//struct ody_videoframe frame[2];

	int release_fd;
	int buf_index;
	int stop;
};

class CMmpRenderer_OdyClientEx2 : public CMmpRenderer
{
friend class CMmpRenderer;

private:
    enum {
        ROTATE_BUF_COUNT = 2
    };

private:
    class mmp_oal_mutex* m_p_mutex;

    int m_sock_fd;
    int m_buf_idx;
    int m_luma_size, m_chroma_size;
	#if ROTATOR_FEATURE_ENABLE
    int m_rend_rot_buf_idx;
	#endif
    struct ody_player m_gplayer;

    struct gdm_dss_overlay m_req;
	struct gdm_dss_overlay_data m_req_data;

    class mmp_buffer* m_p_buf_rotate[ROTATE_BUF_COUNT];
    MMP_S32 m_rend_rot_buf_idx;
    MMP_ROTATE m_rotate;
	MMP_BOOL m_roate_rand;

#if (MMP_OS==MMP_OS_WIN32)
    class mmp_msg_res m_msg_res;
    static void service_render_stub(void* parm);
    void service_render();
    int dss_overlay_queue_win32(int sockfd, struct gdm_dss_overlay_data *req_data);
#endif

protected:
    CMmpRenderer_OdyClientEx2(CMmpRendererCreateProp* pRendererProp);
    virtual ~CMmpRenderer_OdyClientEx2();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();


public:
    virtual void SetFirstRenderer();
    virtual void SetRotate(enum MMP_ROTATE rotate) { m_rotate = rotate; }
    virtual enum MMP_ROTATE GetRotate() { return m_rotate; }

    virtual MMP_RESULT Render(class mmp_buffer_videoframe* p_buf_videoframe);
    virtual MMP_RESULT Render(class mmp_buffer_imageframe* p_buf_imageframe);

    
    
};


#endif
