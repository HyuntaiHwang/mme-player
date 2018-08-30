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

#ifndef MMPRENDERER_ANDROIDAUDIOFLINGER_HPP__
#define MMPRENDERER_ANDROIDAUDIOFLINGER_HPP__

#include "MmpRenderer.hpp"
#include "TemplateList.hpp"
#include "tinyalsa/asoundlib.h"

#include <binder/IBinder.h>
#include <binder/IMemory.h>
#include <binder/IServiceManager.h>

#include <media/IAudioFlingerClient.h>
#include <media/IAudioFlinger.h>
#include <media/IAudioPolicyService.h>
#include "private/media/AudioTrackShared.h"

namespace android {
    
class CMmpRenderer_AndroidAudioFlinger : public CMmpRenderer
{
friend class CMmpRenderer;

private:

    sp<IAudioTrack> m_i_audiotrack;
    const sp<IMemory> m_sharedBuffer;
    sp<StaticAudioTrackClientProxy> mStaticProxy;
    sp<AudioTrackClientProxy>       mProxy;         // primary owner of the memory
    
protected:
    CMmpRenderer_AndroidAudioFlinger(CMmpRendererCreateProp* pRendererProp);
    virtual ~CMmpRenderer_AndroidAudioFlinger();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

public:
    virtual MMP_RESULT Render(class mmp_buffer_audioframe* p_buf_af);

};
}

#endif

