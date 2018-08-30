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

#include "MmpDemuxer.hpp"
#include "MmpDemuxer_Ffmpeg.hpp"
//#include "MmpDemuxer_FfmpegEx1.hpp"
#include "MmpDemuxer_FfmpegEx2.hpp"
#include "MmpDemuxer_ammf.hpp"
#include "MmpUtil.hpp"

/////////////////////////////////////////////////////////
// Create/Destroy Object

CMmpDemuxer* CMmpDemuxer::CreateObject(struct MmpDemuxerCreateConfig* pCreateConfig)
{
    CMmpDemuxer* pObj=NULL;
    MMP_CHAR szExt[32];

    CMmpUtil::SplitExt((MMP_CHAR*)pCreateConfig->filename, szExt);
    CMmpUtil::MakeLower(szExt);

    if(strcmp(szExt, "ammf") == 0) {
        pObj=new CMmpDemuxer_ammf(pCreateConfig);
    }
    else {
		       
        switch(pCreateConfig->type) {

          //case MMP_DEMUXER_TYPE_MJPEG_STREAM:
          //    pObj=new CMmpDemuxer_MjpegStream(pCreateConfig);
          //    break;

          case MMP_DEMUXER_TYPE_AMMF:
              pObj=new CMmpDemuxer_ammf(pCreateConfig);
              break;

          case MMP_DEMUXER_TYPE_FFMPEG:
          default:
#if (FFMPEG_VERSION == FFMPEG_VERSION_2_6_1)
              pObj=new CMmpDemuxer_Ffmpeg(pCreateConfig);
#elif (FFMPEG_VERSION == FFMPEG_VERSION_n3_1_1)
              //pObj=new CMmpDemuxer_FfmpegEx1(pCreateConfig);
              pObj=new CMmpDemuxer_FfmpegEx2(pCreateConfig);
#else
              pObj=NULL;
#endif
              break;
        }

    }

	if(pObj==NULL) {
        return (CMmpDemuxer*)NULL;
	}

    if( pObj->Open()!=MMP_SUCCESS )    
    {
        pObj->Close();
        delete pObj;
        return (CMmpDemuxer*)NULL;
    }

    return pObj;
}

MMP_RESULT CMmpDemuxer::DestroyObject(CMmpDemuxer* pObj)
{
    if(pObj)
    {
        pObj->Close();
        delete pObj;
    }
    return MMP_SUCCESS;
}

/////////////////////////////////////////////////////
// class

CMmpDemuxer::CMmpDemuxer(struct MmpDemuxerCreateConfig* pCreateConfig) :
m_create_config(*pCreateConfig)
{

}

CMmpDemuxer::~CMmpDemuxer()
{

}

MMP_RESULT CMmpDemuxer::Open()
{
    return MMP_FAILURE;
}

MMP_RESULT CMmpDemuxer::Close()
{
    return MMP_FAILURE;
}

struct mmp_audio_format CMmpDemuxer::get_audio_format() {

    struct mmp_audio_format af;

    memset(&af, 0x00, sizeof(af));

    af.fourcc = this->GetAudioFourcc();
    af.channels = this->GetAudioChannel();
    af.sample_rate = this->GetAudioSamplingRate();
    af.bits_per_sample = this->GetAudioBitsPerSample();
    af.bitrate = this->GetAudioBitrate();

    return af;
}

MMP_RESULT CMmpDemuxer::GetNextAudioData(MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size, MMP_S64* packet_pts, mmp_buffer_media::FLAG *flag) {

    return this->GetNextMediaData(MMP_MEDIATYPE_AUDIO, buffer, buf_max_size, buf_size, packet_pts, flag);
}

MMP_RESULT CMmpDemuxer::GetNextVideoData(MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size, MMP_S64* packet_pts, mmp_buffer_media::FLAG *flag) {

    return this->GetNextMediaData(MMP_MEDIATYPE_VIDEO, buffer, buf_max_size, buf_size, packet_pts, flag);
}

MMP_RESULT CMmpDemuxer::GetNextAudioData(class mmp_buffer_audiostream* p_buf_audiostream) {

    MMP_U8* buffer;
    MMP_S32 buf_max_size;
    MMP_S32 stream_size;
    MMP_S64 packet_pts;
    MMP_RESULT mmpResult; 
    mmp_buffer_media::FLAG flag;

    p_buf_audiostream->set_stream_size(0);

    buffer = (MMP_U8*)p_buf_audiostream->get_buf_vir_addr();
    buf_max_size = p_buf_audiostream->get_buf_size();

    mmpResult = this->GetNextMediaData(MMP_MEDIATYPE_AUDIO, buffer, (MMP_U32)buf_max_size, (MMP_U32*)&stream_size, &packet_pts, &flag);
    if(mmpResult == MMP_SUCCESS) {
        p_buf_audiostream->set_stream_size(stream_size);
        p_buf_audiostream->set_stream_offset(0);
        p_buf_audiostream->set_flag(mmp_buffer_media::FLAG_NULL);
        p_buf_audiostream->set_pts(packet_pts);
    }

    return mmpResult;
}

MMP_RESULT CMmpDemuxer::GetNextAudioDataEx1(class mmp_buffer_audiostream* p_buf_as) {

    MMP_U8* buffer, *buffer1;
    MMP_S32 buf_max_size, i;
    MMP_S32 stream_size;
    MMP_S64 packet_pts;
    MMP_RESULT mmpResult; 
    mmp_buffer_media::FLAG flag;

    if(p_buf_as->get_stream_real_size() == 0) {

        p_buf_as->set_stream_size(0);
        p_buf_as->set_stream_offset(0);

        buffer = (MMP_U8*)p_buf_as->get_buf_vir_addr();
        buf_max_size = p_buf_as->get_buf_size();

        mmpResult = this->GetNextMediaData(MMP_MEDIATYPE_AUDIO, buffer, (MMP_U32)buf_max_size, (MMP_U32*)&stream_size, &packet_pts, &flag);
        if(mmpResult == MMP_SUCCESS) {
            p_buf_as->set_stream_size(stream_size);
            p_buf_as->set_flag(mmp_buffer_media::FLAG_NULL);
            p_buf_as->set_pts(packet_pts);
        }

    }
    else {
    
        buffer = (MMP_U8*)p_buf_as->get_stream_real_addr();
        buffer1 = (MMP_U8*)p_buf_as->get_buf_vir_addr();
        i = (int)p_buf_as->get_stream_real_size();

        memcpy(buffer1, buffer, i);
                
        p_buf_as->set_stream_size(i);
        p_buf_as->set_stream_offset(0);

        buffer = (MMP_U8*)p_buf_as->get_buf_vir_addr();
        buffer += i;
        buf_max_size = p_buf_as->get_buf_size();
        buf_max_size -= i;

        mmpResult = this->GetNextMediaData(MMP_MEDIATYPE_AUDIO, buffer, (MMP_U32)buf_max_size, (MMP_U32*)&stream_size, &packet_pts, &flag);
        if(mmpResult == MMP_SUCCESS) {
            p_buf_as->set_stream_size(stream_size+i);
            p_buf_as->set_stream_offset(0);
            p_buf_as->set_flag(mmp_buffer_media::FLAG_NULL);
            p_buf_as->set_pts(packet_pts);

            MMPDEBUGMSG(1, (TEXT("[CMmpDemuxer::GetNextAudioDataEx1] strsz:%d i=%d "), stream_size, i ));
        }
    }

    return mmpResult;
}

MMP_RESULT CMmpDemuxer::GetNextVideoData(class mmp_buffer_videostream* p_buf_videostream) {

    MMP_U8* buffer;
    MMP_S32 buf_max_size;
    MMP_S32 stream_size;
    MMP_S64 packet_pts;
    MMP_RESULT mmpResult; 
    mmp_buffer_media::FLAG flag;

    p_buf_videostream->set_stream_size(0);

    buffer = (MMP_U8*)p_buf_videostream->get_buf_vir_addr();
    buf_max_size = p_buf_videostream->get_buf_size();

    mmpResult = this->GetNextMediaData(MMP_MEDIATYPE_VIDEO, buffer, (MMP_U32)buf_max_size, (MMP_U32*)&stream_size, &packet_pts, &flag);
    if(mmpResult == MMP_SUCCESS) {
        p_buf_videostream->set_stream_size(stream_size);
        p_buf_videostream->set_pts(packet_pts);
        p_buf_videostream->set_flag(flag);
    }

    return mmpResult;
}
    
MMP_RESULT CMmpDemuxer::GetVideoExtraData(MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size) {

    return this->GetMediaExtraData(MMP_MEDIATYPE_VIDEO, buffer, buf_max_size, buf_size);
}

MMP_RESULT CMmpDemuxer::GetAudioExtraData(class mmp_buffer_audiostream* p_buf_audiotream) {

    class mmp_buffer_addr buf_addr;
    MMP_S32 stream_size = 0;
    MMP_RESULT mmpResult;

    buf_addr = p_buf_audiotream->get_buf_addr();

    mmpResult = this->GetMediaExtraData(MMP_MEDIATYPE_AUDIO, (MMP_U8*)buf_addr.m_vir_addr, buf_addr.m_size, (MMP_U32*)&stream_size);
    if(mmpResult == MMP_SUCCESS) {
        p_buf_audiotream->set_stream_size(stream_size);
        p_buf_audiotream->set_flag(mmp_buffer_media::FLAG_CONFIGDATA);
    }
    else {
        p_buf_audiotream->set_stream_size(0);
        p_buf_audiotream->set_flag(mmp_buffer_media::FLAG_NULL);
    }

    return mmpResult;
}

MMP_RESULT CMmpDemuxer::GetVideoExtraData(class mmp_buffer_videostream* p_buf_videstream) {

    class mmp_buffer_addr buf_addr;
    MMP_S32 stream_size = 0;
    MMP_RESULT mmpResult;

    buf_addr = p_buf_videstream->get_buf_addr();

    mmpResult = this->GetMediaExtraData(MMP_MEDIATYPE_VIDEO, (MMP_U8*)buf_addr.m_vir_addr, buf_addr.m_size, (MMP_U32*)&stream_size);
    if(mmpResult == MMP_SUCCESS) {
        p_buf_videstream->set_stream_size(stream_size);
        p_buf_videstream->set_flag(mmp_buffer_media::FLAG_CONFIGDATA);
    }
    else {
        p_buf_videstream->set_stream_size(0);
        p_buf_videstream->set_flag(mmp_buffer_media::FLAG_NULL);
    }

    return mmpResult;
}

MMP_RESULT CMmpDemuxer::GetAudioExtraData(MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size) {

    return this->GetMediaExtraData(MMP_MEDIATYPE_AUDIO, buffer, buf_max_size, buf_size);
}

class mmp_buffer_media* CMmpDemuxer::GetNextMediaBuffer() { 

    class mmp_buffer_media* p_buf_media = NULL;
    //class mmp_buffer_audiostream* p_buf_audiostream = NULL;
    //class mmp_buffer_videostream* p_buf_videostream = NULL;
    MMP_BOOL bforce_get = MMP_FALSE;
    MMP_S32 i;

    for(i = 0; i < 10; i++) {

        p_buf_media = this->queue_get(bforce_get);
        if(p_buf_media != NULL) {
            break;
        }

        this->queue_buffering();
    }
    
    return p_buf_media; 
}
    
