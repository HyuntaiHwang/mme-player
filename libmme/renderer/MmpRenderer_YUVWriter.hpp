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

#ifndef MMPRENDERER_YUVWRITER_HPP__
#define MMPRENDERER_YUVWRITER_HPP__

#include "MmpRendererVideo.hpp"

class CMmpRenderer_YUVWriter : public CMmpRendererVideo
{
friend class CMmpRendererVideo;

private:
    FILE* m_fp;
    static MMP_U32 m_render_file_id;
	
protected:
    CMmpRenderer_YUVWriter(struct CMmpRendererVideo::create_config* p_create_config);
    virtual ~CMmpRenderer_YUVWriter();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

private:
    virtual MMP_RESULT vf_config_internal(MMP_S32 bufcnt, MMP_S32 pic_width, MMP_S32 pic_height, struct mmp_rect display_crop_rect);
    MMP_RESULT Init_Renderer(MMP_S32 pic_width, MMP_S32 pic_height);


protected:
    //virtual MMP_RESULT Render_Ion(CMmpMediaSampleDecodeResult* pDecResult);

public:
    virtual void SetFirstRenderer();
    virtual void SetRotate(enum MMP_ROTATE rotate);

    virtual MMP_RESULT Render(class mmp_buffer_videoframe* p_buf_vf, MMP_MEDIA_ID decoder_id);
    virtual MMP_RESULT Render(class mmp_buffer_imageframe* p_buf_imageframe);

};


#endif
