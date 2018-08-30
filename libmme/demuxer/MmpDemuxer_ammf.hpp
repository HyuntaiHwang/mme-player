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

#ifndef MMPDEMUXER_AMMF_HPP__
#define MMPDEMUXER_AMMF_HPP__

#include "MmpDemuxer.hpp"
#include "TemplateList.hpp"

class CMmpDemuxer_ammf : public CMmpDemuxer
{
friend class CMmpDemuxer;

private:
    CMmpAmmfHeader m_ammf_header;
    FILE* m_fp;

    MMP_U8* m_config_data[MMP_MEDIATYPE_MAX];
    MMP_U32 m_config_data_size[MMP_MEDIATYPE_MAX];

    CMmpAmmfIndex *m_ammf_index[MMP_MEDIATYPE_MAX];
    MMP_U32 m_next_index_id[MMP_MEDIATYPE_MAX];
    MMP_U32 m_index_count[MMP_MEDIATYPE_MAX];

protected:
    CMmpDemuxer_ammf(struct MmpDemuxerCreateConfig* pCreateConfig);
    virtual ~CMmpDemuxer_ammf();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    virtual void queue_buffering(void);

public:
    virtual MMP_RESULT GetNextVideoData(MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size, MMP_S64* packt_pts);
    virtual MMP_RESULT GetNextMediaData(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size, MMP_S64* packt_pts, mmp_buffer_media::FLAG *flag);
    

    virtual MMP_RESULT GetVideoExtraData(MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size);
    virtual MMP_RESULT GetMediaExtraData(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size);

    
    virtual enum MMP_FOURCC GetVideoFourcc();
    virtual MMP_U32 GetVideoPicWidth();
    virtual MMP_U32 GetVideoPicHeight();

    virtual enum MMP_FOURCC GetAudioFourcc();
    virtual MMP_U32 GetAudioChannel();
    virtual MMP_U32 GetAudioSamplingRate();
    virtual MMP_U32 GetAudioBitsPerSample();
    
    virtual MMP_S64 GetDuration();
    virtual MMP_RESULT Seek(MMP_S64 pts);
};


#endif //#ifndef _MMPDEMUXER_HPP__

