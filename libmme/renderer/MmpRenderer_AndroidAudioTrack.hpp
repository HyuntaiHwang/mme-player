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

#ifndef MMPRENDERER_ANDROIDAUDIOTRACK_HPP__
#define MMPRENDERER_ANDROIDAUDIOTRACK_HPP__

#include "MmpRendererAudio.hpp"
#include "TemplateList.hpp"

#include <binder/IBinder.h>
#include <binder/IMemory.h>
#include <binder/IServiceManager.h>
#include "mmp_oal_mutex.hpp"
#include "mmp_oal_cond.hpp"

namespace android {

class AudioTrack;


class CMmpRenderer_AndroidAudioTrack : public CMmpRendererAudio
{
friend class CMmpRendererAudio;

private:
    struct renbuf {
        MMP_U8* data;
        MMP_S32 rdidx;
        MMP_S32 size;
        MMP_S64 pts;
    };
private:
    MMP_BOOL m_is_close;

    int m_SampleRate;
    int64_t m_LatencyUs;
    size_t m_FrameSize;
    
    sp<AudioTrack> m_pAudioTrack;
    class mmp_oal_mutex *m_p_mutex;
    class mmp_oal_cond *m_p_cond_ren;
    class mmp_oal_cond *m_p_cond_fill;
    TCircular_Queue<struct renbuf> m_queue_buf;

    //sp<IAudioTrack> m_i_audiotrack;
    //const sp<IMemory> m_sharedBuffer;
    //sp<StaticAudioTrackClientProxy> mStaticProxy;
    //sp<AudioTrackClientProxy>       mProxy;         // primary owner of the memory
    
protected:
    CMmpRenderer_AndroidAudioTrack(CMmpRendererAudio::create_config *p_create_config);
    virtual ~CMmpRenderer_AndroidAudioTrack();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    static void AudioCallbackStub(int _event, void *user, void *info);
    void AudioCallback(int _event, void *info);

    size_t fillBuffer(void *data, size_t size);

public:
    virtual MMP_RESULT Render(class mmp_buffer_audioframe* p_buf_af);

};
}

#endif

