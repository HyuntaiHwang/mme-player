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


#include "MmpPlayer.hpp"
#include "MmpUtil.hpp"
//#include "MmpPlayerVideoEx2.hpp"
#include "MmpPlayerVideoEx3.hpp"
#include "MmpPlayerAudio.hpp"
#include "MmpPlayerPCM.hpp"
//#include "MmpPlayerAVEx4.hpp"
#include "MmpPlayerAVEx5.hpp"
#include "MmpPlayerTONE.hpp"
#include "MmpPlayerYUV.hpp"
#include "MmpPlayerRGB.hpp"
#include "MmpPlayerJPEG.hpp"
#include "MmpPlayerCompress.hpp"
#include "mmp_util.h"

#if (MMP_PLATFORM == MMP_PLATFORM_ANDROID)
#include "MmpPlayerStagefright.hpp"
#endif

//////////////////////////////////////////////////////////////
// CMmpPlayer CreateObject/DestroyObject

CMmpPlayer* CMmpPlayer::CreateObject(MMP_U32 playerID, CMmpPlayerCreateProp* pPlayerProp)
{
    CMmpPlayer* pObj = NULL;

    MMP_CHAR szext[16];
        
    CMmpUtil::SplitExtC(pPlayerProp->filename, szext);
    CMmpUtil::MakeLowerC(szext);

    if(strcmp(szext, "pcm") == 0) {
        pObj=new CMmpPlayerPCM(pPlayerProp);
    }
    else if( (strcmp(szext, "jpg") == 0) || (strcmp(szext, "jpeg") == 0) ) {
        pObj=new CMmpPlayerJPEG(pPlayerProp);
    }
    else if( strcmp(szext, "yuv") == 0 ) {
        pObj=new CMmpPlayerYUV(pPlayerProp);
    }
    else if( ( strcmp(szext, "argb8888") == 0 ) ||  ( strcmp(szext, "abgr8888") == 0 ) ) {
        pObj=new CMmpPlayerRGB(pPlayerProp);
    }
        
    if(pObj == NULL) {

        switch(playerID)
        {
            case CMmpPlayer::DEFAULT:
                pObj = new CMmpPlayerAVEx5(pPlayerProp);
                break;

            case CMmpPlayer::AUDIO_VIDEO:
                pObj = new CMmpPlayerAVEx5(pPlayerProp);
                break;

            case CMmpPlayer::VIDEO_ONLY:
                //pObj=new CMmpPlayerVideoEx2(pPlayerProp);
                pObj=new CMmpPlayerVideoEx3(pPlayerProp);
                break;

            case CMmpPlayer::AUDIO_ONLY:
                pObj=new CMmpPlayerAudio(pPlayerProp);
                break;

            case CMmpPlayer::TONEPLAYER:
                pObj=new CMmpPlayerTONE(pPlayerProp);
                break;

            case CMmpPlayer::YUVPLAYER:
                pObj=new CMmpPlayerYUV(pPlayerProp);
                break;

            case CMmpPlayer::JPEG:
                pObj=new CMmpPlayerJPEG(pPlayerProp);
                break;
#if 0
            case CMmpPlayer::ANDROID_COMPRESS:
                pObj=new CMmpPlayerCompress(pPlayerProp);
                break;
#endif                

    #if (MMP_PLATFORM == MMP_PLATFORM_ANDROID)
            case CMmpPlayer::STAGEFRIGHT:
                pObj=new android::CMmpPlayerStagefright(pPlayerProp);
                break;
    #endif
        }

    }

    if(pObj==NULL)
        return (CMmpPlayer*)NULL;

    if( pObj->Open()!=MMP_SUCCESS )    
    {
        pObj->Close();
        delete pObj;
        return (CMmpPlayer*)NULL;
    }

    return pObj;
}

MMP_RESULT CMmpPlayer::DestroyObject(CMmpPlayer* pObj)
{
    if(pObj)
    {
        pObj->Close();
        delete pObj;
    }
    return MMP_SUCCESS;
}

/////////////////////////////////////////////////////////////
//CMmpPlayer Member Functions

CMmpPlayer::CMmpPlayer(CMmpPlayerCreateProp* pPlayerProp, MMP_CHAR* player_name) :  CMmpPlayerService(player_name)

, m_create_config(*pPlayerProp)


{
    

}

CMmpPlayer::~CMmpPlayer()
{
}

MMP_RESULT CMmpPlayer::Open()
{

    CMmpPlayerService::Open();
    
    return MMP_SUCCESS;
}


MMP_RESULT CMmpPlayer::Close()
{
    CMmpPlayerService::Close();

    this->PlayStop();
   

    return MMP_SUCCESS;
}


CMmpDemuxer* CMmpPlayer::CreateDemuxer(void) {

    struct MmpDemuxerCreateConfig demuxer_create_config;
    CMmpDemuxer* pDemuxer = NULL;

    strcpy((char*)demuxer_create_config.filename, this->m_create_config.filename);
    pDemuxer = CMmpDemuxer::CreateObject(&demuxer_create_config);

    return pDemuxer;
}

CMmpDecoderAudio* CMmpPlayer::CreateDecoderAudio(CMmpDemuxer* pDemuxer) {
    
    CMmpDecoderAudio::create_config decoder_audio_create_config;
    CMmpDecoderAudio* pDecoderAudio = NULL;

    decoder_audio_create_config.fourcc_in = pDemuxer->GetAudioFourcc();
    decoder_audio_create_config.sr = pDemuxer->GetAudioSamplingRate();
    decoder_audio_create_config.ch = pDemuxer->GetAudioChannel();
    decoder_audio_create_config.bps = (unsigned short)pDemuxer->GetAudioBitsPerSample();

    pDecoderAudio = CMmpDecoderAudio::CreateObject(&decoder_audio_create_config);
    
    return pDecoderAudio;
}

CMmpDecoderVideo* CMmpPlayer::CreateDecoderVideo(CMmpDemuxer* pDemuxer, CMmpRendererVideo* pRendererVideo, MMP_BOOL bFfmpegUse) {
    
    struct CMmpDecoderVideo::create_config decoder_create_config;
    CMmpDecoderVideo* pDecoderVideo = NULL;

    memset(&decoder_create_config, 0x00, sizeof(decoder_create_config));

    decoder_create_config.fourcc_in = pDemuxer->GetVideoFourcc();
    decoder_create_config.fourcc_out = pRendererVideo->get_fourcc_in();
        
    pDecoderVideo = CMmpDecoderVideo::CreateObject(&decoder_create_config, bFfmpegUse);

    return pDecoderVideo;
}


CMmpRendererAudio* CMmpPlayer::CreateRendererAudio(CMmpDecoderAudio* pDecoderAudio) {

    CMmpRendererAudio* pRendererAudio = NULL;
    CMmpRendererAudio::create_config renderer_audio_create_config;

    renderer_audio_create_config.sr = pDecoderAudio->get_sample_rate();
	renderer_audio_create_config.ch = pDecoderAudio->get_channels();
	renderer_audio_create_config.bps = pDecoderAudio->get_bits_per_sample();
	pRendererAudio = CMmpRendererAudio::CreateObject(&renderer_audio_create_config);

    return pRendererAudio;
}


CMmpRendererVideo* CMmpPlayer::CreateRendererVideo(CMmpRendererVideo::ID vid_ren_id) {

    CMmpRendererVideo* pRendererVideo = NULL;
    struct CMmpRendererVideo::create_config renderer_video_create_config;

#if (MMP_OS == MMP_OS_WIN32)    
    renderer_video_create_config.hRenderWnd = this->m_create_config.video_config.m_hRenderWnd;
    renderer_video_create_config.hRenderDC = this->m_create_config.video_config.m_hRenderDC;
#endif
    renderer_video_create_config.id = vid_ren_id;
    //renderer_video_create_config.id = CMmpRendererVideo::ID_YUV_WRITER;

    pRendererVideo = CMmpRendererVideo::CreateObject(&renderer_video_create_config);
      
    return pRendererVideo;
}

#if 0
MMP_RESULT CMmpPlayer::DecodeMediaExtraData(MMP_U32 mediatype, 
                                    CMmpDemuxer* pDemuxer, CMmpDecoder* pDecoder)  {

    CMmpMediaSample *pMediaSample = &m_MediaSampleObj;
    CMmpMediaSampleDecodeResult* pDecResult = &m_DecResultObj;
    MMP_U32 stream_buf_size;
    MMP_U8* stream_buf = NULL; 
    MMP_U32 stream_buf_maxsize = 1024*1024;

    MMP_RESULT mmpResult = MMP_FAILURE;

    stream_buf = (MMP_U8*)malloc(stream_buf_maxsize);

    if(stream_buf != NULL) {
        stream_buf_size = 0;
        pDemuxer->GetMediaExtraData(mediatype, stream_buf, stream_buf_maxsize, &stream_buf_size);
        if(stream_buf_size > 0) {
        
            pMediaSample->pAu = stream_buf;
            pMediaSample->uiAuSize = stream_buf_size;
            pMediaSample->uiSampleNumber = 0;
            pMediaSample->uiTimeStamp = 0;
            pMediaSample->uiFlag = mmp_buffer_media::FLAG_CONFIGDATA;

            memset((void*)pDecResult, 0x00, sizeof(CMmpMediaSampleDecodeResult));
                    
            mmpResult = pDecoder->DecodeAu(pMediaSample, pDecResult);
        }
    }

    if(stream_buf) free(stream_buf);

    return mmpResult;
}
#endif

MMP_RESULT CMmpPlayer::DecodeAudioExtraData(CMmpDemuxer* pDemuxer, CMmpDecoderAudio* pDecoder) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    class mmp_buffer_audiostream* p_buf_audiostream;

    p_buf_audiostream = mmp_buffer_mgr::get_instance()->alloc_media_audiostream((MMP_MEDIA_ID)this, mmp_buffer_audiostream::DEFAULT_MAX_STREAM_SIZE, mmp_buffer::HEAP);
    if(p_buf_audiostream != NULL) {
        pDemuxer->GetAudioExtraData(p_buf_audiostream);
        if(p_buf_audiostream->get_stream_real_size() > 0) {
            mmpResult = pDecoder->DecodeAu(p_buf_audiostream, NULL);
        }
        mmp_buffer_mgr::get_instance()->free_media_buffer(p_buf_audiostream);
    }

    return mmpResult;
}

MMP_RESULT CMmpPlayer::DecodeVideoExtraData(CMmpDemuxer* pDemuxer, CMmpDecoderVideo* pDecoder) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    class mmp_buffer_videostream* p_buf_videostream;
    CMmpDecoderVideo::ACTION next_action;

    p_buf_videostream = mmp_buffer_mgr::get_instance()->alloc_media_videostream((MMP_MEDIA_ID)this, 1024, mmp_buffer::HEAP);
    if(p_buf_videostream != NULL) {
        pDemuxer->GetVideoExtraData(p_buf_videostream);
        if(p_buf_videostream->get_stream_real_size() > 0) {
            mmpResult = pDecoder->DecodeAu(p_buf_videostream, (class mmp_buffer_videoframe**)NULL, &next_action);
        }
        mmp_buffer_mgr::get_instance()->free_media_buffer(p_buf_videostream);
    }

    return mmpResult;
}


