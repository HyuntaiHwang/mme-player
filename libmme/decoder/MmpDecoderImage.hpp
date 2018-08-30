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

#ifndef MMPDECODERIMAGE_HPP__
#define MMPDECODERIMAGE_HPP__

#include "MmpDecoder.hpp"

class CMmpDecoderImage : public CMmpDecoder
{
friend class CMmpDecoder;

public:
    static CMmpDecoderImage* CreateObject(struct MmpDecoderCreateConfig *pCreateConfig, MMP_BOOL bForceSW = MMP_FALSE);

protected:
    MMPBITMAPINFOHEADER* m_pbih;;
	MMPBITMAPINFOHEADER m_bih_in;
	MMPBITMAPINFOHEADER m_bih_out;

private:
    MMP_S32 m_nDecodingAvgFPS;
    
    MMP_U32 m_nTotalDecDur;
    MMP_U32 m_nTotalDecFrameCount;

protected:
    CMmpDecoderImage(struct MmpDecoderCreateConfig *pCreateConfig, MMP_CHAR* class_name);
    virtual ~CMmpDecoderImage();

    virtual MMP_RESULT Open();
	virtual MMP_RESULT Open(MMP_U8* pStream, MMP_U32 nStreamSize) {return MMP_FAILURE;}
    virtual MMP_RESULT Close();

    void DecodeMonitor(class mmp_buffer_videoframe* p_buf_videoframe);

    
public:
    virtual MMP_RESULT DecodeAu(class mmp_buffer_imagestream* p_buf_imagestream, class mmp_buffer_imageframe** pp_buf_imageframe) = 0;
    
    MMP_S32 GetPicWidth() { return m_bih_out.biWidth; }
    MMP_S32 GetPicHeight() { return m_bih_out.biHeight; }
    MMP_U32 GetPicFormat() { return m_bih_out.biCompression; }
};

#endif

