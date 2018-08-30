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

#include "MmpDemuxer_ammf.hpp"
#include "MmpUtil.hpp"


/////////////////////////////////////////////////////
// class

CMmpDemuxer_ammf::CMmpDemuxer_ammf(struct MmpDemuxerCreateConfig* pCreateConfig) : CMmpDemuxer(pCreateConfig) 
,m_fp(NULL)
{
    int i;

    memset(&m_ammf_header, 0x00, sizeof(m_ammf_header));

    for(i = 0; i < MMP_MEDIATYPE_MAX; i++) {
        m_config_data[i] = NULL;
        m_config_data_size[i] = 0;

        m_next_index_id[i] = 0;
        m_index_count[i] = 0;
        m_ammf_index[i] = NULL;
    }
}

CMmpDemuxer_ammf::~CMmpDemuxer_ammf()
{
    int i;

    for(i = 0; i < MMP_MEDIATYPE_MAX; i++) {
        
        if(m_config_data[i]!=NULL) {
            free(m_config_data[i]);
        }
        m_config_data[i] = NULL;

        if(m_ammf_index[i] != NULL) {
            delete [] m_ammf_index[i];
            m_ammf_index[i] = NULL;
        }

    }
}

MMP_RESULT CMmpDemuxer_ammf::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_U32 i, j;
    CMmpAmmfIndex ammf_index;

    m_fp = fopen((const char*)m_create_config.filename, "rb");
    if(m_fp==NULL) {
        mmpResult = MMP_FAILURE;
    }

    /* read header */
    if(mmpResult == MMP_SUCCESS) {
    
        fseek(m_fp, 0, SEEK_SET);
        fread(&this->m_ammf_header, 1, sizeof(m_ammf_header), m_fp);

        if( (m_ammf_header.uiKey1 == MMP_AMF_FILE_KEY1) &&
            (m_ammf_header.uiKey2 == MMP_AMF_FILE_KEY2) ) {

            /* Nothing to do */
        }
        else {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDemuxer_ammf::Open] ERROR : Key Mismatch, Check Version cur_key=(0x%08x, 0x%08x) file_key=(0x%08x,0x%08x) "),
                                      m_ammf_header.uiKey1, m_ammf_header.uiKey2,
                                      MMP_AMF_FILE_KEY1, MMP_AMF_FILE_KEY2 ));
            mmpResult = MMP_FAILURE;
        }
    }

    /* read config */
    if(mmpResult == MMP_SUCCESS) {

        for(i = 0; i < MMP_MEDIATYPE_MAX; i++) {
            
            if(m_ammf_header.uiConfigDataSize[i] > 0) {
                
                m_config_data[i] = (MMP_U8*)malloc(m_ammf_header.uiConfigDataSize[i]);
                if(m_config_data[i] != NULL) {
                    
                    m_config_data_size[i] = m_ammf_header.uiConfigDataSize[i];

                    fseek(m_fp, m_ammf_header.uiConfigDataFileOffset[i], SEEK_SET);
                    fread(m_config_data[i], 1, m_config_data_size[i], m_fp);
                }
            }
        }
    }
    
    /* read index data */
    if(mmpResult == MMP_SUCCESS) {

        for(i = 0; i < MMP_MEDIATYPE_MAX; i++) {
            
            if(m_ammf_header.uiIndexCount[i] > 0) {

                m_index_count[i] = m_ammf_header.uiIndexCount[i];
                m_ammf_index[i] = new CMmpAmmfIndex[m_index_count[i]];
                
                fseek(m_fp, m_ammf_header.uiIndexFileOffset[i], SEEK_SET);

                for(j = 0; j < m_index_count[i]; j++) {

                    fread(&ammf_index, 1, sizeof(ammf_index), m_fp);
                    m_ammf_index[i][j] = ammf_index;
                }
                
            }
        }
    }

    return mmpResult;
}

MMP_RESULT CMmpDemuxer_ammf::Close()
{
    if(m_fp!=NULL) {
        fclose(m_fp);
        m_fp = NULL;
    }
    
    return MMP_SUCCESS;
}

enum MMP_FOURCC CMmpDemuxer_ammf::GetAudioFourcc() {

    enum MMP_FOURCC fourcc = MMP_FOURCC_AUDIO_UNKNOWN;

#if 0
    if(m_ammf_header.bIsMedia[MMP_MEDIATYPE_AUDIO] == MMP_TRUE) {
    
        format = (MMP_U32)m_ammf_header.wf.wFormatTag;
        format &= 0x0000ffff;
    }
#endif

    return fourcc;
}

MMP_U32 CMmpDemuxer_ammf::GetAudioChannel() {
    
    return 0;
}
    
MMP_U32 CMmpDemuxer_ammf::GetAudioSamplingRate() {

    return 0;
}

MMP_U32 CMmpDemuxer_ammf::GetAudioBitsPerSample() {

    return 0;
}
    

enum MMP_FOURCC CMmpDemuxer_ammf::GetVideoFourcc() {

    enum MMP_FOURCC fourcc = MMP_FOURCC_VIDEO_UNKNOWN;

    if(m_ammf_header.bIsMedia[MMP_MEDIATYPE_VIDEO] == MMP_TRUE) {
    
        fourcc = (enum MMP_FOURCC)m_ammf_header.bih.biCompression;
    }

    return fourcc;
}

MMP_U32 CMmpDemuxer_ammf::GetVideoPicWidth() {

    MMP_U32 v = 0;

    if(m_ammf_header.bIsMedia[MMP_MEDIATYPE_VIDEO] == MMP_TRUE) {
    
        v = m_ammf_header.bih.biWidth;
    }

    return v;
}

MMP_U32 CMmpDemuxer_ammf::GetVideoPicHeight() {

    MMP_U32 v = 0;

    if(m_ammf_header.bIsMedia[MMP_MEDIATYPE_VIDEO] == MMP_TRUE) {
    
        v = m_ammf_header.bih.biHeight;
    }

    return v;
}

MMP_RESULT CMmpDemuxer_ammf::GetVideoExtraData(MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size)  {

    MMP_RESULT mmpResult = MMP_FAILURE;

    if(buf_size) *buf_size = 0;

    if( (m_config_data_size[MMP_MEDIATYPE_VIDEO] > 0)   
        && (m_config_data_size[MMP_MEDIATYPE_VIDEO] <= buf_max_size)   
        ) {

        memcpy(buffer, m_config_data[MMP_MEDIATYPE_VIDEO], m_config_data_size[MMP_MEDIATYPE_VIDEO] );
        if(buf_size) *buf_size = m_config_data_size[MMP_MEDIATYPE_VIDEO];
        mmpResult = MMP_SUCCESS;
    }
    
    return mmpResult;
}

MMP_RESULT CMmpDemuxer_ammf::GetMediaExtraData(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size)  {

    MMP_RESULT mmpResult = MMP_FAILURE;

    if(buf_size) *buf_size = 0;

    if( (m_config_data_size[mediatype] > 0)   
        && (m_config_data_size[mediatype] <= buf_max_size)   
        ) {

        memcpy(buffer, m_config_data[mediatype], m_config_data_size[mediatype] );
        if(buf_size) *buf_size = m_config_data_size[mediatype];
        mmpResult = MMP_SUCCESS;
    }
    
    return mmpResult;
}

MMP_RESULT CMmpDemuxer_ammf::GetNextVideoData(MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size, MMP_S64* packt_pts) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    CMmpAmmfIndex ammf_index;
    MMP_U32 next_index_id, index_count;

    if(buf_size) *buf_size = 0;

    next_index_id = m_next_index_id[MMP_MEDIATYPE_VIDEO];
    index_count = m_index_count[MMP_MEDIATYPE_VIDEO];
    ammf_index = m_ammf_index[MMP_MEDIATYPE_VIDEO][next_index_id];

    if( (ammf_index.uiStreamType == MMP_MEDIATYPE_VIDEO) 
        && (ammf_index.uiStreamSize  <= buf_max_size)   
        && (next_index_id < index_count)
        ) {

        fseek(m_fp, ammf_index.uiFileOffset, SEEK_SET);
        fread(buffer, 1, ammf_index.uiStreamSize, m_fp);
        if(buf_size) *buf_size = ammf_index.uiStreamSize;
        mmpResult = MMP_SUCCESS;

        m_next_index_id[MMP_MEDIATYPE_VIDEO]++;
    }
    
    return mmpResult;
}   

MMP_RESULT CMmpDemuxer_ammf::GetNextMediaData(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size, MMP_S64* packt_pts, mmp_buffer_media::FLAG *flag) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    CMmpAmmfIndex ammf_index;
    MMP_U32 next_index_id, index_count;

    if(buf_size) *buf_size = 0;

    next_index_id = m_next_index_id[mediatype];
    index_count = m_index_count[mediatype];
    ammf_index = m_ammf_index[mediatype][next_index_id];

    if( (ammf_index.uiStreamType == mediatype) 
        && (ammf_index.uiStreamSize  <= buf_max_size)   
        && (next_index_id < index_count)
        ) {

        fseek(m_fp, ammf_index.uiFileOffset, SEEK_SET);
        fread(buffer, 1, ammf_index.uiStreamSize, m_fp);
        if(buf_size) *buf_size = ammf_index.uiStreamSize;
        if(packt_pts) *packt_pts = (MMP_S64)ammf_index.uiTimeStamp*1000L;
        mmpResult = MMP_SUCCESS;

        m_next_index_id[mediatype]++;
    }
    
    return mmpResult;
}   

void CMmpDemuxer_ammf::queue_buffering(void) {

}

MMP_S64 CMmpDemuxer_ammf::GetDuration() {

    return (MMP_S64)m_ammf_header.uiPlayDuration*1000L;
}

MMP_RESULT CMmpDemuxer_ammf::Seek(MMP_S64 pts) {

    return MMP_FAILURE;
}