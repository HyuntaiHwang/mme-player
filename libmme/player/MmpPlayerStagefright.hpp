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

#ifndef _MMPPLAYERSTAGEFRIGHT_HPP__
#define _MMPPLAYERSTAGEFRIGHT_HPP__

#include "MmpPlayer.hpp"

#if (MMP_PLATFORM == MMP_PLATFORM_ANDROID || MMP_PLATFORM == MMP_PLATFORM_WIN32)

#include <binder/IMemory.h>
#include <binder/IServiceManager.h>

#include <media/IMediaPlayerService.h>

#include <gui/ISurfaceComposer.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/BufferItemConsumer.h>
#include <utils/String8.h>

#include <private/gui/ComposerService.h>
#include <binder/ProcessState.h>

#include <ui/GraphicBufferMapper.h>
#include <ui/DisplayInfo.h>

namespace android {

class CMmpPlayerStagefright : public CMmpPlayer
{
friend class CMmpPlayer;

private:
    int m_fd;

    struct DisplayInfo m_display_info;

    sp<IMediaPlayer> m_iMediaPlayer;
    
    sp<SurfaceComposerClient> m_SurfaceComposerClient;
    sp<SurfaceControl> m_SurfaceControl;
    
    //sp<Surface> m_Surface;
    //sp<IGraphicBufferProducer> iGraphicBufferProducer;
    
protected:
    CMmpPlayerStagefright(CMmpPlayerCreateProp* pPlayerProp);
    virtual ~CMmpPlayerStagefright();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    virtual void Service();

public:
    virtual MMP_RESULT PlayStart();
    virtual MMP_RESULT PlayStop();
    virtual MMP_RESULT PlayPause();
    virtual MMP_RESULT PlayResume();

    virtual MMP_RESULT Seek(MMP_S64 pts);

    virtual MMP_S64 GetDuration();
    virtual MMP_S64 GetPlayPosition();

    virtual MMP_U32 GetDurationMS();
    virtual MMP_U32 GetPlayPositionMS();

};

}

#endif

#endif


