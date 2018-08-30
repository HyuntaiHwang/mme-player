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

#ifndef MMPRENDERER_ANDROIDSURFACEEX1_HPP__
#define MMPRENDERER_ANDROIDSURFACEEX1_HPP__

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


class CMmpRenderer_AndroidSurfaceEx1 : public CMmpRendererVideo
{
friend class CMmpRendererVideo;

private:

    sp<Surface> m_Surface;
    sp<SurfaceComposerClient> m_ComposerClient;
    sp<SurfaceControl> m_SurfaceControl;
    struct DisplayInfo m_display_info;
    
protected:
    CMmpRenderer_AndroidSurfaceEx1(CMmpRendererCreateProp* pRendererProp);
    virtual ~CMmpRenderer_AndroidSurfaceEx1();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

public:
    virtual MMP_RESULT Render(class mmp_buffer_videoframe* p_buf_videoframe);

    //virtual MMP_RESULT Render(CMmpMediaSampleDecodeResult* pDecResult);
    //virtual MMP_RESULT RenderYUV420Planar(MMP_U8* Y, MMP_U8* U, MMP_U8* V);
    //virtual MMP_RESULT RenderYUV420Planar(MMP_U8* Y, MMP_U8* U, MMP_U8* V, MMP_U32 buffer_width, MMP_U32 buffer_height);
    //virtual MMP_RESULT OnSize(int cx, int cy) 
    //{ 
      //  return MMP_SUCCESS;
    //}
    
};


#endif
#endif
