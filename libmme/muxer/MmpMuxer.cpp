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

#include "MmpMuxer.hpp"
#include "MmpMuxer_ammf.hpp"
#include "MmpMuxer_Ffmpeg.hpp"
#include "MmpMuxer_FfmpegEx2.hpp"
#include "MmpMuxer_rawstream.hpp"
#include "MmpUtil.hpp"

/////////////////////////////////////////////////////////
// Create/Destroy Object

CMmpMuxer* CMmpMuxer::CreateObject(struct MmpMuxerCreateConfig* pCreateConfig)
{
    CMmpMuxer* pObj=NULL;
    MMP_CHAR szExt[32];

    CMmpUtil::SplitExt((MMP_CHAR*)pCreateConfig->filename, szExt);
    CMmpUtil::MakeLower(szExt);

    if(strcmp(szExt, "ammf") == 0) {
        pObj=new CMmpMuxer_ammf(pCreateConfig);
    }
    else if(strcmp(szExt, "h264") == 0) {
        pObj=new CMmpMuxer_rawstream(pCreateConfig);
    }
    else {
#if (FFMPEG_VERSION == FFMPEG_VERSION_2_6_1)
        pObj=new CMmpMuxer_Ffmpeg(pCreateConfig);
#elif (FFMPEG_VERSION == FFMPEG_VERSION_n3_1_1)
        pObj=new CMmpMuxer_FfmpegEx2(pCreateConfig);
#else
        pObj = NULL;
#endif
    }
    
	if(pObj==NULL) {
        return (CMmpMuxer*)NULL;
	}

    if( pObj->Open()!=MMP_SUCCESS )    
    {
        pObj->Close();
        delete pObj;
        return (CMmpMuxer*)NULL;
    }

    return pObj;
}

MMP_RESULT CMmpMuxer::DestroyObject(CMmpMuxer* pObj)
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

CMmpMuxer::CMmpMuxer(struct MmpMuxerCreateConfig* pCreateConfig) :
m_create_config(*pCreateConfig)
,m_last_input_pts(0)
{

}

CMmpMuxer::~CMmpMuxer()
{

}

MMP_RESULT CMmpMuxer::Open()
{
    return MMP_FAILURE;
}

MMP_RESULT CMmpMuxer::Close()
{
    return MMP_FAILURE;
}

MMP_RESULT CMmpMuxer::AddVideoConfig(MMP_U8* buffer, MMP_U32 buf_size) {

    return this->AddMediaConfig(MMP_MEDIATYPE_VIDEO, buffer, buf_size);
}

MMP_RESULT CMmpMuxer::AddVideoData(MMP_U8* buffer, MMP_U32 buf_size, MMP_U32 flag, MMP_S64 pts) {

    return this->AddMediaData(MMP_MEDIATYPE_VIDEO, buffer, buf_size, flag, pts);
}

MMP_RESULT CMmpMuxer::AddMediaData(class mmp_buffer_videostream* p_buf_videostream) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U8* p_stream;
    MMP_S32 stream_size;
    MMP_U32 flag;
    MMP_S64 pts;
    
    if(p_buf_videostream->get_dsi_size() > 0) {
        mmpResult = this->AddMediaConfig(MMP_MEDIATYPE_VIDEO, (MMP_U8*)p_buf_videostream->get_dsi_buffer(), p_buf_videostream->get_dsi_size());
    }

    if(p_buf_videostream->get_stream_real_size() > 0) {

        p_stream = p_buf_videostream->get_stream_real_ptr();
        stream_size = p_buf_videostream->get_stream_real_size();
        flag = p_buf_videostream->get_flag();
        pts = p_buf_videostream->get_pts();

        mmpResult = this->AddMediaData(MMP_MEDIATYPE_VIDEO, p_stream, stream_size, flag, pts);
    }

    return MMP_SUCCESS;
}
