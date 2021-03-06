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

#ifndef _MMPDEMUXER_FFMPEG_HPP__
#define _MMPDEMUXER_FFMPEG_HPP__

#include "MmpDemuxer.hpp"

#if (FFMPEG_VERSION == FFMPEG_VERSION_2_6_1)

extern "C"
{
#include "libavformat/avformat.h"
#include "libavformat/url.h"
}

#define MMP_DEMUXER_FILE_BUFFER_MAX_SIZE (3*1024*1024)
class CMmpDemuxer_Ffmpeg : public CMmpDemuxer
{
friend class CMmpDemuxer;

private:
    FILE* m_fp;
    MMP_U8* m_IoBuffer;
    URLContext* m_Url;
    AVIOContext* m_pAVIOContext;
    AVFormatContext *m_pAvformatCtx;
    int64_t m_iFileSize;
    MMP_S32 m_nStreamIndex[MMP_MEDIATYPE_MAX];

    MMP_U8 m_FileBuffer[MMP_DEMUXER_FILE_BUFFER_MAX_SIZE];
    MMP_S32 m_nFileBufferIndex;
    MMP_S32 m_nFileBufferSize;

protected:
    CMmpDemuxer_Ffmpeg(struct MmpDemuxerCreateConfig* pCreateConfig);
    virtual ~CMmpDemuxer_Ffmpeg();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    static int IORead_C_Stub(void *opaque, uint8_t *buf, int buf_size);
    static int64_t IOSeek_C_Stub(void *opaque, int64_t offset, int whence);

    int IORead_C(void *opaque, uint8_t *buf, int buf_size);
    int64_t IOSeek_C(void *opaque, int64_t offset, int whence);

    virtual void queue_buffering(void);

public:
    virtual class mmp_buffer_media* GetNextMediaBuffer();

    //virtual MMP_RESULT GetNextVideoData(MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size, MMP_S64* packt_pts);
    virtual MMP_RESULT GetNextMediaData(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size, MMP_S64* packt_pts, mmp_buffer_media::FLAG *flag);
    

    virtual MMP_RESULT GetVideoExtraData(MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size);
    virtual MMP_RESULT GetMediaExtraData(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size);
    
    virtual MMP_RESULT GetAudioExtraData(class mmp_buffer_audiostream* p_buf_audiostream);
    virtual MMP_RESULT GetVideoExtraData(class mmp_buffer_videostream* p_buf_videstream);

    
    virtual enum MMP_FOURCC GetVideoFourcc();
    virtual MMP_U32 GetVideoPicWidth();
    virtual MMP_U32 GetVideoPicHeight();
    virtual MMP_S32 GetVideoBitrate();
    virtual MMP_S32 GetVideoFPS();

    virtual enum MMP_FOURCC GetAudioFourcc();
    virtual MMP_U32 GetAudioChannel();
    virtual MMP_U32 GetAudioSamplingRate();
    virtual MMP_U32 GetAudioBitsPerSample();
    virtual MMP_S32 GetAudioBitrate();
    virtual MMP_S64 GetDuration();

    virtual MMP_RESULT Seek(MMP_S64 pts);
   
};


#endif //#ifndef _MMPDEMUXER_HPP__
#endif

