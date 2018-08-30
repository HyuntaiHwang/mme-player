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

#include "MmpMuxer_ammf.hpp"
#include "MmpUtil.hpp"


/////////////////////////////////////////////////////
// class

CMmpMuxer_ammf::CMmpMuxer_ammf(struct MmpMuxerCreateConfig* pCreateConfig) : CMmpMuxer(pCreateConfig) 
,m_fp(NULL)
{
    int i;

    memset(&m_ammf_header, 0x00, sizeof(m_ammf_header));

    m_ammf_header.uiKey1 = MMP_AMF_FILE_KEY1;
    m_ammf_header.uiKey2 = MMP_AMF_FILE_KEY2;
    for(i = 0; i < MMP_MEDIATYPE_MAX; i++) {
        m_ammf_header.bIsMedia[i] = pCreateConfig->bMedia[i];    
    }

    m_ammf_header.uiHeaderDataFileOffset = 0;
    m_ammf_header.uiHeaderDataSize = MMP_AMF_HEADER_SIZE;

    m_ammf_header.uiStreamDataFileOffset = MMP_AMF_STREAM_START_OFFSET;
    memcpy(&m_ammf_header.wf, &pCreateConfig->wf,  sizeof(MMPWAVEFORMATEX) );
    memcpy(&m_ammf_header.bih, &pCreateConfig->bih,  sizeof(MMPBITMAPINFOHEADER) );


    for(i = 0; i < MMP_MEDIATYPE_MAX; i++) {
        m_config_data[i] = NULL;
        m_config_data_size[i] = 0;
    }
}

CMmpMuxer_ammf::~CMmpMuxer_ammf()
{
    int i;

    for(i = 0; i < MMP_MEDIATYPE_MAX; i++) {
        if(m_config_data[i]!=NULL) {
            free(m_config_data[i]);
        }
        m_config_data[i] = NULL;
    }
}

MMP_RESULT CMmpMuxer_ammf::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_U8* buffer = NULL;

    buffer = (MMP_U8*)malloc(m_ammf_header.uiStreamDataFileOffset);
    if(buffer != NULL) {
        memset(buffer, 0x00, m_ammf_header.uiStreamDataFileOffset);
    }

    m_fp = fopen((const char*)m_create_config.filename, "wb");
    if(m_fp==NULL) {
        mmpResult = MMP_FAILURE;
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpMuxer_ammf::Open] FAIL: file open (%s) "), m_create_config.filename ));
    }
    else {
        fwrite(buffer, 1, m_ammf_header.uiStreamDataFileOffset, m_fp);
    }


    if(buffer) {
        free(buffer);
    }
   
    return mmpResult;
}

MMP_RESULT CMmpMuxer_ammf::Close()
{   
    int i;
    TList<CMmpAmmfIndex> *pListIndex;
    CMmpAmmfIndex index;
    bool bflag;

    if( (m_fp!=NULL) && (m_ammf_header.uiStreamDataSize > 0) ) {
    
        //Write Config Data
        for(i = 0; i < MMP_MEDIATYPE_MAX; i++) {
            
            if(m_config_data_size[i] > 0) {
                m_ammf_header.uiConfigDataFileOffset[i] = (MMP_U32)ftell(m_fp);
                m_ammf_header.uiConfigDataSize[i] = m_config_data_size[i];

                fwrite(this->m_config_data[i], 1, m_config_data_size[i], m_fp );
            }
        }

        //Write Index Data
        for(i = 0; i < MMP_MEDIATYPE_MAX; i++) {
            
            pListIndex = &m_index_list[i];

            if(pListIndex->GetSize() >  0) {
                m_ammf_header.uiIndexFileOffset[i] = (MMP_U32)ftell(m_fp);
                m_ammf_header.uiIndexCount[i] = pListIndex->GetSize();

                bflag = pListIndex->GetFirst(index);
                while(bflag==true) {
                    fwrite(&index, 1, sizeof(index), m_fp);
                    bflag = pListIndex->GetNext(index);
                }
            }
        }

        m_ammf_header.uiFileSize = (MMP_U32)ftell(m_fp);

        fseek(m_fp, 0, SEEK_SET);
        fwrite(&m_ammf_header, 1, sizeof(m_ammf_header), m_fp);
    }
    
    if(m_fp!=NULL) {
        fclose(m_fp);
        m_fp = NULL;
    }

    return MMP_SUCCESS;
}


MMP_RESULT CMmpMuxer_ammf::AddMediaConfig(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_size) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    
    if(m_config_data[mediatype] != NULL) {
        free(m_config_data[mediatype]);
        m_config_data[mediatype] = NULL;
    }

    m_config_data[mediatype] = (MMP_U8*)malloc(buf_size);
    if(m_config_data[mediatype] != NULL) {
        memcpy(m_config_data[mediatype], buffer, buf_size);
        m_config_data_size[mediatype] = buf_size;
        mmpResult = MMP_SUCCESS;
    }

    return mmpResult;
}

MMP_RESULT CMmpMuxer_ammf::AddMediaData(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_size, MMP_U32 flag, MMP_S64 pts) {

    TList<CMmpAmmfIndex>* pListIndex;
    CMmpAmmfIndex index;

    if( (flag&mmp_buffer_media::FLAG_CONFIGDATA) != 0 ) {
        this->AddMediaConfig(mediatype, buffer, buf_size);
    }
    else {

        pListIndex = &m_index_list[mediatype];

        index.uiStreamType = mediatype;
        index.uiStreamSize = buf_size;
        index.uiFileOffset = (MMP_U32)ftell(m_fp);
        index.uiTimeStamp = (MMP_U32)(pts/1000L);
        index.uiFlag = flag;
        pListIndex->Add(index);

        fwrite(buffer, 1, buf_size, m_fp);

        m_ammf_header.uiStreamDataSize += buf_size;
        if(m_ammf_header.uiPlayDuration < (index.uiTimeStamp + 30) ) {
            m_ammf_header.uiPlayDuration = index.uiTimeStamp + 30;
        }
    }

    return MMP_SUCCESS;
}

