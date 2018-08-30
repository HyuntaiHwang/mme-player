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

#ifndef MMPDECODERIMAGE_JPEGLIB_HPP__
#define MMPDECODERIMAGE_JPEGLIB_HPP__

#include "MmpDecoderImage.hpp"
#include "mmp_buffer_imageframe.hpp"

class CMmpDecoderImage_JpegLib : public CMmpDecoderImage
{
friend class CMmpDecoderImage;

private:
    class mmp_buffer_imageframe* m_p_buf_imageframe_yuv;
    class mmp_buffer_imageframe* m_p_buf_imageframe_rgb;
        
protected:
    CMmpDecoderImage_JpegLib(struct MmpDecoderCreateConfig *pCreateConfig);
    virtual ~CMmpDecoderImage_JpegLib();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

public:
    virtual MMP_RESULT DecodeAu(class mmp_buffer_imagestream* p_buf_imagestream, class mmp_buffer_imageframe** pp_buf_imageframe);
};

#endif

