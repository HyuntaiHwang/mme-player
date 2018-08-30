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

#ifndef MMPRENDERER_ANDROIDSURFACEEX2_HPP__
#define MMPRENDERER_ANDROIDSURFACEEX2_HPP__

#include "MmpRendererVideo.hpp"

#if 1//(MMP_OS == MMP_OS_LINUX)

#include <binder/IMemory.h>
#include <gui/ISurfaceComposer.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
//#include <gui/BufferItemConsumer.h>
#include <utils/String8.h>

#include <private/gui/ComposerService.h>
#include <binder/ProcessState.h>

#include <ui/GraphicBufferMapper.h>
#include <ui/DisplayInfo.h>

using namespace android;


class CMmpRenderer_AndroidSurfaceEx2 : public CMmpRendererVideo
{
friend class CMmpRendererVideo;

private:

    sp<Surface> m_Surface;
    sp<SurfaceComposerClient> m_ComposerClient;
    sp<SurfaceControl> m_SurfaceControl;
    sp<ANativeWindow> m_NativeWindow;
    struct DisplayInfo m_display_info;

    int m_buf_idx;
    int m_buf_count;
    ANativeWindowBuffer *m_buf_native[32];
    
protected:
    CMmpRenderer_AndroidSurfaceEx2(struct CMmpRendererVideo::create_config* p_create_config);
    virtual ~CMmpRenderer_AndroidSurfaceEx2();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    virtual MMP_RESULT vf_config_internal(MMP_S32 bufcnt, MMP_S32 pic_width, MMP_S32 pic_height, struct mmp_rect display_crop_rect);

public:
    virtual MMP_RESULT Render(class mmp_buffer_videoframe* p_buf_videoframe, MMP_MEDIA_ID decoder_id);
};


#endif
#endif
