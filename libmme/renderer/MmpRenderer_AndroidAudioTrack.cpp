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


#include "MmpRenderer_AndroidAudioTrack.hpp"
#include "MmpUtil.hpp"

#include <media/IAudioFlingerClient.h>
#include <media/IAudioFlinger.h>
#include <media/IAudioPolicyService.h>
#include <media/AudioTrack.h>
#include "private/media/AudioTrackShared.h"
#include "mmp_lock.hpp"
#include <typeinfo>

namespace android {

sp<AudioTrack> s_pAudioTrack;
/////////////////////////////////////////////////////////////
//CMmpRenderer_AndroidAudioTrack Member Functions

CMmpRenderer_AndroidAudioTrack::CMmpRenderer_AndroidAudioTrack(CMmpRendererAudio::create_config *p_create_config) :  CMmpRendererAudio(p_create_config)
    ,m_is_close(MMP_FALSE)
    ,m_p_mutex(NULL)
    ,m_p_cond_ren(NULL)
    ,m_p_cond_fill(NULL)
    ,m_queue_buf(100)
{
    
}

CMmpRenderer_AndroidAudioTrack::~CMmpRenderer_AndroidAudioTrack()
{
    struct renbuf rbuf;

    while(!m_queue_buf.IsEmpty()) {
        m_queue_buf.Delete(rbuf);
        free(rbuf.data);
    }
}

MMP_RESULT CMmpRenderer_AndroidAudioTrack::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;
    status_t err;
    audio_channel_mask_t audioMask = audio_channel_out_mask_from_count(this->get_ch());

    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpRenderer_AndroidAudioTrack::Open] rate=%d ch=%d audiomask=0x%x "), 
                     this->get_sr(),
                     this->get_ch(),
                     audioMask
                     ));

    if(mmpResult == MMP_SUCCESS) {
        m_p_mutex = mmp_oal_mutex::create_object();
        if(m_p_mutex == NULL) {
            mmpResult = MMP_FAILURE;
        }
    }

    if(mmpResult == MMP_SUCCESS) {
        m_p_cond_ren = mmp_oal_cond::create_object();
        if(m_p_cond_ren == NULL) {
            mmpResult = MMP_FAILURE;
        }
    }

    if(mmpResult == MMP_SUCCESS) {
        m_p_cond_fill = mmp_oal_cond::create_object();
        if(m_p_cond_fill == NULL) {
            mmpResult = MMP_FAILURE;
        }
    }

    if(mmpResult == MMP_SUCCESS) {

        m_SampleRate = this->get_sr();
                
        static int uiddd = 100;
        static int session_id = 100;

        m_pAudioTrack = new AudioTrack(
                    AUDIO_STREAM_MUSIC, 
                    m_SampleRate, 
                    AUDIO_FORMAT_PCM_16_BIT, 
                    audioMask,
                    0, //4096*10*3 /*frameCount*/, 
                    AUDIO_OUTPUT_FLAG_NONE, 
                    &AudioCallbackStub, 
                    this,
                    0 /*notificationFrames*/
                    //session_id, //AUDIO_SESSION_ALLOCATE, //int sessionId       = AUDIO_SESSION_ALLOCATE,
                    //AudioTrack::TRANSFER_DEFAULT, //AudioTrack::TRANSFER_SYNC, //(AudioTrack::transfer_type)0,//TRANSFER_DEFAULT, //transfer_type transferType = TRANSFER_DEFAULT,
                    //NULL, //const audio_offload_info_t *offloadInfo = NULL,
                    //uiddd, //-1, //int uid = -1,
                    //-1, //pid_t pid = -1,
                    //NULL //, const audio_attributes_t* pAttributes = NULL
                    );

        uiddd++;
        session_id++;

        err = m_pAudioTrack->initCheck();
        if(err!= OK) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_AndroidAudioTrack::Open] FAIL: new AudioTrack  ") ));
            m_pAudioTrack.clear();
            mmpResult = MMP_FAILURE;
        }
        else {
            m_LatencyUs = (int64_t)m_pAudioTrack->latency() * 1000;
            m_FrameSize = m_pAudioTrack->frameSize();
            m_pAudioTrack->start();
        }
    }

    return mmpResult;
}

MMP_RESULT CMmpRenderer_AndroidAudioTrack::Close()
{
    m_is_close = MMP_TRUE;

    printf("!!!!!!!!!!!!!!!! ln=%d \n", __LINE__ );

    if(m_p_cond_fill != NULL) {
        this->m_p_cond_fill->signal();
    }

    printf("!!!!!!!!!!!!!!!! ln=%d \n", __LINE__ );

    if(m_p_cond_ren != NULL) {
        this->m_p_cond_ren->signal();
    }

#if 1
    printf("!!!!!!!!!!!!!!!! ln=%d \n", __LINE__ );
    CMmpUtil::Sleep(10);
    printf("!!!!!!!!!!!!!!!! ln=%d \n", __LINE__ );
    if(m_pAudioTrack != NULL) {
        printf("!!!!!!!!!!!!!!!! try stop.....\n");
        m_pAudioTrack->stop();
        while(!m_pAudioTrack->stopped()) {
            printf("!!!!!!!!!!!!!!!! wait.......\n");
            CMmpUtil::Sleep(10);
        }
        printf("!!!!!!!!!!!!!!!! stopped...\n");
        m_pAudioTrack->flush();

        if(s_pAudioTrack == NULL) {
           // s_pAudioTrack = m_pAudioTrack;
        }
        else {
            m_pAudioTrack.clear();
        }
    }
#endif

    if(m_p_mutex != NULL) {
        mmp_oal_mutex::destroy_object(m_p_mutex);
        m_p_mutex = NULL;
    }

    if(m_p_cond_ren != NULL) {
        mmp_oal_cond::destroy_object(m_p_cond_ren);
        m_p_cond_ren = NULL;
    }

    if(m_p_cond_fill != NULL) {
        mmp_oal_cond::destroy_object(m_p_cond_fill);
        m_p_cond_fill = NULL;
    }
    

    return MMP_SUCCESS;
}

// static
void CMmpRenderer_AndroidAudioTrack::AudioCallbackStub(int _event, void *user, void *info) {

    static_cast<CMmpRenderer_AndroidAudioTrack *>(user)->AudioCallback(_event, info);
}

void CMmpRenderer_AndroidAudioTrack::AudioCallback(int _event, void *info) {
 
    switch(_event) {

    case AudioTrack::EVENT_MORE_DATA:
        {
            AudioTrack::Buffer *buffer = (AudioTrack::Buffer *)info;
            size_t numBytesWritten = fillBuffer(buffer->raw, buffer->size);

            //MMPDEBUGMSG(1, (TEXT("[AudioRend] Cbs numBytesWritten=%d buffer->size=%d "), numBytesWritten, buffer->size ));

            buffer->size = numBytesWritten;
        }
        break;

    case AudioTrack::EVENT_STREAM_END:
        //mReachedEOS = true;
        //notifyAudioEOS();
        break;
    }
}

size_t CMmpRenderer_AndroidAudioTrack::fillBuffer(void *data, size_t size) {
 
    struct renbuf rbuf;
    size_t size_done = 0;
    int size_remaining = (int)size;
    int rbuf_remaining;

    //MMPDEBUGMSG(1, (TEXT("[AudioRend] ln=%d "), __LINE__  ));

    while( (size_remaining > 0) && (m_is_close==MMP_FALSE) ) {
        
        //MMPDEBUGMSG(1, (TEXT("[AudioRend] ln=%d "), __LINE__  ));

        this->m_p_mutex->lock();

        //MMPDEBUGMSG(1, (TEXT("[AudioRend] ln=%d "), __LINE__  ));

        if(!this->m_queue_buf.IsEmpty()) {

          //  MMPDEBUGMSG(1, (TEXT("[AudioRend] ln=%d "), __LINE__  ));

            this->m_queue_buf.GetFirstItem(rbuf);

            rbuf_remaining = rbuf.size - rbuf.rdidx;
            if(rbuf_remaining > size_remaining) {

                memcpy( (void*)((unsigned int)data+size_done), (void*)((unsigned int)rbuf.data+rbuf.rdidx), size_remaining);

                size_done += size_remaining;
                rbuf.rdidx += size_remaining;
                size_remaining = 0;
                
                this->m_queue_buf.SetFirstItem(rbuf);
                //MMPDEBUGMSG(1, (TEXT("[AudioRend 1] pts=%d  rbuf.rdidx=%d "), (unsigned int)(rbuf.pts/1000), rbuf.rdidx ));
                this->set_real_render_pts(rbuf.pts);
            }
            else {
                this->m_queue_buf.Delete(rbuf);
                memcpy( (void*)((unsigned int)data+size_done), (void*)((unsigned int)rbuf.data+rbuf.rdidx), rbuf_remaining);
                free(rbuf.data);
                size_done += rbuf_remaining;
                size_remaining -= rbuf_remaining;

                //MMPDEBUGMSG(1, (TEXT("[AudioRend 2] pts=%d "), (unsigned int)(rbuf.pts/1000) ));
                this->set_real_render_pts(rbuf.pts);
            }
        }
        else {

            //MMPDEBUGMSG(1, (TEXT("[AudioRend] ln=%d "), __LINE__  ));

            this->m_p_cond_fill->wait(this->m_p_mutex);
        }
        
        //MMPDEBUGMSG(1, (TEXT("[AudioRend] ln=%d "), __LINE__  ));
        this->m_p_mutex->unlock();
    }

    //MMPDEBUGMSG(1, (TEXT("[AudioRend] ln=%d "), __LINE__  ));

    if(size_done > 0) {

        //MMPDEBUGMSG(1, (TEXT("[AudioRend] ln=%d "), __LINE__  ));
        this->m_p_cond_ren->signal();
    }
    
    //MMPDEBUGMSG(1, (TEXT("[AudioRend] ln=%d "), __LINE__  ));

    return size_done;
}

MMP_RESULT CMmpRenderer_AndroidAudioTrack::Render(class mmp_buffer_audioframe* p_buf_af)
{
    struct renbuf rbuf;

    this->m_p_mutex->lock();
    
    while( (this->m_queue_buf.IsFull()) && (m_is_close==MMP_FALSE) ) {
        this->m_p_cond_ren->wait(this->m_p_mutex);
    }

    if(m_is_close == MMP_FALSE) {

        rbuf.rdidx = 0;
        rbuf.size = p_buf_af->get_data_size();
        rbuf.data = (MMP_U8*)malloc(rbuf.size);
        rbuf.pts = p_buf_af->get_pts();
        if(rbuf.data != NULL) {
            memcpy(rbuf.data, (void*)p_buf_af->get_buf_vir_addr(), rbuf.size);
            this->m_queue_buf.Add(rbuf);
            //MMPDEBUGMSG(1, (TEXT("[AudioRend Add] pts=%d "), (unsigned int)(rbuf.pts/1000) ));
        }

    }

    this->m_p_mutex->unlock();

    if(this->m_queue_buf.GetSize() > 0) {
        this->m_p_cond_fill->signal();
    }

    return MMP_SUCCESS;
}

}
