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

#ifndef MMPENCODERVIDEO_FFMPEGEX1_HPP__
#define MMPENCODERVIDEO_FFMPEGEX1_HPP__

#include "MmpEncoderVideo.hpp"
#include "MmpEncoderFfmpegEx1.hpp"

#if (FFMPEG_VERSION == FFMPEG_VERSION_n3_1_1)

#ifdef WIN32
#define CMmpEncoderVideo_FfmpegEx1_YUV_DUMP 0
#else
#define CMmpEncoderVideo_FfmpegEx1_YUV_DUMP 0
#endif

class CMmpEncoderVideo_FfmpegEx1 : public CMmpEncoderVideo, CMmpEncoderFfmpegEx1
{
friend class CMmpEncoderVideo;

private:
    MMP_U32 m_nEncodedStreamCount;
    MMP_BOOL m_is_dsi;

#if (CMmpEncoderVideo_FfmpegEx1_YUV_DUMP == 1)
    void* m_yuv_dump_hdl;
#endif

    MMP_U8* m_ColorConvertFrameBuffer;
    
protected:
    CMmpEncoderVideo_FfmpegEx1(struct CMmpEncoderVideo::create_config *p_create_config);
    virtual ~CMmpEncoderVideo_FfmpegEx1();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();
        
public:
    virtual MMP_RESULT EncodeAu(class mmp_buffer_videoframe* p_buf_vf, class mmp_buffer_videostream* p_buf_vs);

};

#endif 
#endif

