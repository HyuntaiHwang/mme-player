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

#include "MmpMuxer_rawstream.hpp"
#include "MmpUtil.hpp"


/////////////////////////////////////////////////////
// class

CMmpMuxer_rawstream::CMmpMuxer_rawstream(struct MmpMuxerCreateConfig* pCreateConfig) : CMmpMuxer(pCreateConfig) 
,m_fp(NULL)
{
 

}

CMmpMuxer_rawstream::~CMmpMuxer_rawstream()
{
   
}

MMP_RESULT CMmpMuxer_rawstream::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;

    m_fp = fopen((const char*)m_create_config.filename, "wb");
    if(m_fp==NULL) {
        mmpResult = MMP_FAILURE;
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpMuxer_rawstream::Open] FAIL: file open (%s) "), m_create_config.filename ));
    }
    else {
    
    }

    return mmpResult;
}

MMP_RESULT CMmpMuxer_rawstream::Close()
{   
    
    if(m_fp!=NULL) {
        fclose(m_fp);
        m_fp = NULL;
    }

    return MMP_SUCCESS;
}


MMP_RESULT CMmpMuxer_rawstream::AddMediaConfig(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_size) {

    return MMP_SUCCESS;
}

MMP_RESULT CMmpMuxer_rawstream::AddMediaData(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_size, MMP_U32 flag, MMP_S64 pts) {

    
    fwrite(buffer, 1, buf_size, m_fp);
    
    return MMP_SUCCESS;
}

