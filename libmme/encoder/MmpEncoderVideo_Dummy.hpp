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

#ifndef MMPENCODERVIDEO_DUMMY_HPP__
#define MMPENCODERVIDEO_DUMMY_HPP__

#include "MmpEncoderVideo.hpp"

class CMmpEncoderVideo_Dummy : public CMmpEncoderVideo
{
friend class CMmpEncoderVideo;

private:
    MMP_U32 m_nEncodedStreamCount;
    MMP_BOOL m_bEncodeOpen;   
    MMP_BOOL m_bEncodeDSI; 
    
    
protected:
    CMmpEncoderVideo_Dummy(struct CMmpEncoderVideo::create_config *p_create_config);
    virtual ~CMmpEncoderVideo_Dummy();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    virtual const MMP_U8* GetClassName() { return (const MMP_U8*)"Dummy";}
    
public:
    virtual void set_pic_width(MMP_S32 w);
    virtual void set_pic_height(MMP_S32 h);
    
    virtual MMP_RESULT EncodeAu(class mmp_buffer_videoframe* p_buf_vf, class mmp_buffer_videostream* p_buf_vs);
    

};

#endif

