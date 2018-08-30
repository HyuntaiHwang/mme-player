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

#include "MmpDemuxer_MjpegStream.hpp"



/////////////////////////////////////////////////////
// class

CMmpDemuxer_MjpegStream::CMmpDemuxer_MjpegStream(struct MmpDemuxerCreateConfig* pCreateConfig) : CMmpDemuxer(pCreateConfig) 
,m_fp(NULL)
{

}

CMmpDemuxer_MjpegStream::~CMmpDemuxer_MjpegStream()
{

}

MMP_RESULT CMmpDemuxer_MjpegStream::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;

    m_fp = fopen((const char*)m_create_config.filename, "rb");
    if(m_fp==NULL) {
        mmpResult = MMP_FAILURE;
    }

   
    return mmpResult;
}

MMP_RESULT CMmpDemuxer_MjpegStream::Close()
{
    if(m_fp!=NULL) {
        fclose(m_fp);
        m_fp = NULL;
    }
    
    return MMP_SUCCESS;
}

MMP_U32 CMmpDemuxer_MjpegStream::GetVideoFormat() {

     return 0;
}

MMP_U32 CMmpDemuxer_MjpegStream::GetVideoPicWidth() {

    return 0;
}

MMP_U32 CMmpDemuxer_MjpegStream::GetVideoPicHeight() {

    return 0;
}

MMP_RESULT CMmpDemuxer_MjpegStream::GetVideoExtraData(MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size)  {

    
    return MMP_FAILURE;
}

MMP_RESULT CMmpDemuxer_MjpegStream::GetNextVideoData(MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size) {

    return MMP_FAILURE;
}   

