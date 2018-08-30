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

#ifndef MMPRENDERER_VIDEO_HPP__
#define MMPRENDERER_VIDEO_HPP__

#include "MmpRenderer.hpp"


class CMmpRendererVideo : public CMmpRenderer
{
public:
    enum ID {
        ID_DISPLAY=0,
        ID_YUV_WRITER,
        ID_DUMMY,
    };

    struct create_config {
#if (MMP_OS == MMP_OS_WIN32)
        void* hRenderWnd;
        void* hRenderDC;
#endif
        enum CMmpRendererVideo::ID id;
        
        public:
            create_config() : id(CMmpRendererVideo::ID_DISPLAY) {}
    };

public:
    static CMmpRendererVideo* CreateObject(struct CMmpRendererVideo::create_config* p_create_config);
    
protected:
    CMmpRendererVideo(struct CMmpRendererVideo::create_config* p_create_config, enum MMP_FOURCC fourcc_in = MMP_FOURCC_IMAGE_YUV420);
    virtual ~CMmpRendererVideo();

private:
    enum {
        MAX_VF_COUNT = 36
    };
        
    class mmp_buffer_videoframe* m_p_vf[MAX_VF_COUNT];
    MMP_S32 m_vf_count;

    MMP_S32 m_pic_width;
    MMP_S32 m_pic_height;

protected:
    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    virtual MMP_RESULT vf_config_internal(MMP_S32 bufcnt, MMP_S32 pic_width, MMP_S32 pic_height, struct mmp_rect display_crop_rect);
    MMP_RESULT vf_add(class mmp_buffer_videoframe* p_vf);
    MMP_RESULT vf_clear_own(MMP_S32 fd);
    MMP_RESULT vf_set_own(MMP_S32 fd, MMP_MEDIA_ID id);

private:

    MMP_RESULT vf_flush(void);

public:
    MMP_RESULT vf_config(MMP_S32 bufcnt, MMP_S32 pic_width, MMP_S32 pic_height);
    MMP_RESULT vf_config(MMP_S32 bufcnt, MMP_S32 pic_width, MMP_S32 pic_height, struct mmp_rect display_crop_rect);

    inline void vf_all_set_own(MMP_MEDIA_ID id) {
       class mmp_buffer_videoframe* p_vf = NULL; 
       for(int idx = 0; idx < m_vf_count; idx++) {
          m_p_vf[idx]->set_own(id);
       }
    }

    inline MMP_S32 vf_get_count(void) { return m_vf_count; }
    inline class mmp_buffer_videoframe* vf_get(MMP_S32 idx) {
       class mmp_buffer_videoframe* p_vf = NULL; 
       if( (idx >= 0) && (idx < m_vf_count) ) {
            p_vf = m_p_vf[idx];
       }
       return p_vf;
    }
    inline class mmp_buffer_videoframe* vf_next() {
       class mmp_buffer_videoframe* p_vf = NULL; 
       int idx;
       for(idx = 0; idx < m_vf_count; idx++) {
           if(m_p_vf[idx]->get_own() != (MMP_MEDIA_ID)this) {
               p_vf = m_p_vf[idx];
               break;
           }
       }
       return p_vf;
    }

    inline MMP_S32 get_pic_width() { return m_pic_width; }
    inline MMP_S32 get_pic_height() { return m_pic_height; }

    virtual MMP_RESULT Render(class mmp_buffer_videoframe* p_buf_videoframe, MMP_MEDIA_ID decoder_id) = 0;
    virtual MMP_RESULT Render(class mmp_buffer_imageframe* p_buf_imageframe) { return MMP_FAILURE; }
      

};

#ifdef __cplusplus
extern "C" {
#endif

void mmp_render_video_init(void* hwnd, void* hdc, 
							  int boardwidth, int boardheight, 
							  int scrx, int scry, int scrwidht, int scrheight,
							  int picwidht, int picheight,
                              int jpeg_dump);
void* mmp_render_video_create(int picwidht, int picheight, int rotationDegrees);
int mmp_render_video_destroy(void* hdl);
int mmp_render_video_write(void* hdl, char* data, int datasize);

#ifdef __cplusplus
}
#endif


#endif

