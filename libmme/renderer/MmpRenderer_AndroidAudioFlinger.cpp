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


#include "MmpRenderer_AndroidAudioFlinger.hpp"
#include "MmpUtil.hpp"


namespace android {



/////////////////////////////////////////////////////////////
//CMmpRenderer_AndroidAudioFlinger Member Functions

CMmpRenderer_AndroidAudioFlinger::CMmpRenderer_AndroidAudioFlinger(CMmpRendererCreateProp* pRendererProp) :  CMmpRenderer(MMP_MEDIATYPE_AUDIO, pRendererProp)
{
    
}

CMmpRenderer_AndroidAudioFlinger::~CMmpRenderer_AndroidAudioFlinger()
{

}

/*
  audio_policy.conf 에  3개의 HW가 등록되어 있다.
   
         primary  - speaker 
         a2dp     - blutooth
         usb      - accessary 

  audio policy service가 초기화 할때,  audio_policy.conf 파일을 읽어  등록된 HW와  in/out prop를 설정한다.
  아래에서 getOutput을  하면  MUSIC에 관련된 적적한 audio_io_output ID를  return한다. 
  Audio Policy정책에 따라  speaker, a2dp 가 모두  Sound ON 될 수 도 있고, FAIL이 날 수 도 있다. 

*/
MMP_RESULT CMmpRenderer_AndroidAudioFlinger::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;
    audio_stream_type_t stream_type = AUDIO_STREAM_MUSIC;
    audio_io_handle_t audio_io_output = 0;

    sp<IAudioPolicyService> i_audio_policy;
    sp<IAudioFlinger> i_audioflinger;
        

    if(mmpResult == MMP_SUCCESS) {

        sp<IServiceManager> sm = defaultServiceManager();
        sp<IBinder> binder_audio_policy = sm->getService(String16("media.audio_policy"));
        i_audio_policy = interface_cast<IAudioPolicyService>(binder_audio_policy);
        if(i_audio_policy == NULL) {
            printf("ERROR: get audio policy \n\r");
            mmpResult = MMP_FAILURE;
        }
        else {
            printf("audio policy service .. OK \n\r");
        }

        sp<IBinder> binder = sm->getService(String16("media.audio_flinger"));
        i_audioflinger = interface_cast<IAudioFlinger>(binder);
        if(i_audioflinger == NULL) {
            printf("ERROR: get audioflinger \n\r");
            //return -1;
        }
        else {
            printf("audioflinger service .. OK \n\r");
        }

    }

    if(mmpResult == MMP_SUCCESS) {
        stream_type = AUDIO_STREAM_MUSIC;
        audio_io_output = i_audio_policy->getOutput(stream_type);
        if(audio_io_output == 0) {
            printf("ERROR: i_audio_policy->getOutput ln=%d \n\r", __LINE__ );
            return MMP_FAILURE;
            //mmpResult = MMP_FAILURE;
        }
        else {
            printf("audio_io_output(from audio_policy) .. OK \n\r");
        }
    }

#if 1
    uint32_t sample_rate = 0;
    audio_format_t afmt = AUDIO_FORMAT_INVALID;
    audio_channel_mask_t channel_mask = 0; 
    size_t frame_count = 0;
    uint32_t track_flags = 0;

    
    String8 track_name("TestAudioTrack");
    
    /* create Track */
    stream_type = AUDIO_STREAM_MUSIC;
    sample_rate = 44100;
    channel_mask = AUDIO_CHANNEL_OUT_STEREO;
    afmt = AUDIO_FORMAT_PCM_16_BIT;
    frame_count = 4096;

#if 1 //android 5.x
    m_i_audiotrack = i_audioflinger->createTrack(
                                stream_type, //audio_stream_type_t streamType,
                                sample_rate,  //uint32_t sampleRate,
                                afmt, //audio_format_t format,
                                channel_mask, //audio_channel_mask_t channelMask,
                                &frame_count, //size_t *pFrameCount,
                                &track_flags, //track_flags_t *flags,
                                m_sharedBuffer, //const sp<IMemory>& sharedBuffer,
                                // On successful return, AudioFlinger takes over the handle
                                // reference and will release it when the track is destroyed.
                                // However on failure, the client is responsible for release.
                                audio_io_output, //audio_io_handle_t output,
                                -1, //pid_t tid,  // -1 means unused, otherwise must be valid non-0
                                NULL, //int *sessionId,
                                0, //int clientUid,
                                NULL //status_t *status
                                );
#else
    m_i_audiotrack = i_audioflinger->createTrack(stream_type, //audio_stream_type_t streamType,
                                sample_rate,  //uint32_t sampleRate,
                                afmt, //audio_format_t format,
                                channel_mask, //audio_channel_mask_t channelMask,
                                frame_count, //size_t frameCount,
                                &track_flags, //track_flags_t *flags,
                                m_sharedBuffer, //const sp<IMemory>& sharedBuffer,
                                audio_io_output, //audio_io_handle_t output,
                                -1, //pid_t tid,  // -1 means unused, otherwise must be valid non-0
                                NULL, //int *sessionId,
                                // input: ignored
                                // output: server's description of IAudioTrack for display in logs.
                                // Don't attempt to parse, as the format could change.
                                track_name, //String8& name,
                                0, //int clientUid,
                                NULL);//status_t *status);
#endif

    if(m_i_audiotrack == 0) {
        printf("FAIL: i_audioflinger->createTrack) \n\r");
        return MMP_FAILURE;
    }
    else {
        m_i_audiotrack->start();
        printf("i_audiotrack ...  OK frame_count=%d \n\r", frame_count);

#if 0
        //m_i_audiotrack->allocateTimedBuffer(size_t size,   sp<IMemory>* buffer) = 0;
        void* buffers;
        sp<IMemory> iMem = m_i_audiotrack->getCblk();
        audio_track_cblk_t* cblk = static_cast<audio_track_cblk_t*>(iMem->pointer());

        buffers = (char*)cblk + sizeof(audio_track_cblk_t);

        size_t temp = frame_count;//cblk->frameCount_;
        size_t frameCount = temp;
        size_t mFrameSizeAF = 2 * sizeof(int16_t);

        mStaticProxy = new StaticAudioTrackClientProxy(cblk, buffers, frameCount, mFrameSizeAF);
        mProxy = mStaticProxy;
        if(mProxy == NULL) {
            printf("FAIL: new StaticAudioTrackClientProxy \n\r");
            return MMP_FAILURE;
        }
#endif

    }


#endif

    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_AndroidAudioFlinger::Close()
{
    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_AndroidAudioFlinger::Render(class mmp_buffer_audioframe* p_buf_af)
{
    
#if 1

    void* buffers;
    sp<IMemory> iMem = m_i_audiotrack->getCblk();
    audio_track_cblk_t* cblk = static_cast<audio_track_cblk_t*>(iMem->pointer());

    buffers = (char*)cblk + sizeof(audio_track_cblk_t);

    //m_i_audiotrack->write(p_buf_af->get_data_size(), p_buf_af->get_data_size(), 1);
    return MMP_SUCCESS;

#endif

#if 0

  MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_AndroidAudioFlinger::Render] ln=%d "), __LINE__ ));

    //sp<IMemory> iMem = m_i_audiotrack->getCblk();
    //audio_track_cblk_t* cblk = static_cast<audio_track_cblk_t*>(iMem->pointer());

    //if(

    if(m_sharedBuffer != 0) {
    
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_AndroidAudioFlinger::Render] sz = %d"), p_buf_af->get_data_size()));

        memcpy(m_sharedBuffer->pointer(), (void*)p_buf_af->get_buf_vir_addr(), p_buf_af->get_data_size());
        m_i_audiotrack->signal();
    }
    else {
    
        
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_AndroidAudioFlinger::Render] ln=%d "), __LINE__ ));
        
        //memcpy(buffers, (void*)p_buf_af->get_buf_vir_addr(), p_buf_af->get_data_size());

        //m_i_audiotrack->signal();
        status_t status = NO_ERROR;
        //struct timespec requested;
        //struct timespec elapsed;
        AudioTrackClientProxy::Buffer buffer;

        buffer.mFrameCount = p_buf_af->get_data_size()/4;
        buffer.mNonContig = 0;

        sp<IMemory> iMem = m_i_audiotrack->getCblk();
        audio_track_cblk_t* cblk = static_cast<audio_track_cblk_t*>(iMem->pointer());

        //MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_AndroidAudioFlinger::Render] m_sharedBuffer is NULL  datasize=%d flag=0x%08x  "), p_buf_af->get_data_size(), cblk->mFlags ));

        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_AndroidAudioFlinger::Render] ln=%d "), __LINE__ ));

        status = mProxy->obtainBuffer(&buffer);//, requested, elapsed);
        if(status == NO_ERROR) {
               
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("+++++++++++A asdf ")));

            memcpy(buffer.mRaw, (void*)p_buf_af->get_buf_vir_addr(), p_buf_af->get_data_size());

            buffer.mFrameCount = p_buf_af->get_data_size()/4;
            buffer.mNonContig = 0;
            mProxy->releaseBuffer(&buffer);
        }

    }

#else
    sp<IMemory> buf;
    m_i_audiotrack->allocateTimedBuffer(p_buf_af->get_data_size(), &buf);

    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_AndroidAudioFlinger::Render] ln=%d "), __LINE__ ));

    if(buf != NULL) {

        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_AndroidAudioFlinger::Render] ln=%d "), __LINE__ ));

        memcpy(buf->pointer(), (void*)p_buf_af->get_buf_vir_addr(), p_buf_af->get_data_size());
        m_i_audiotrack->queueTimedBuffer(buf, p_buf_af->get_pts());
    }

    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_AndroidAudioFlinger::Render] ln=%d "), __LINE__ ));
#endif

    return MMP_SUCCESS;
}

}