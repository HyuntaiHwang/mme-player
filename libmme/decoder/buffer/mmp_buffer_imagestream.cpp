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

#include "mmp_buffer_imagestream.hpp"


/**********************************************************
class member
**********************************************************/

mmp_buffer_imagestream::mmp_buffer_imagestream(MMP_MEDIA_ID producer_id) : mmp_buffer_media(IMAGE_STREAM, producer_id)

,m_p_mmp_buffer(NULL)
,m_stream_offset(0)
,m_stream_size(0)

,m_fp_imagefile(NULL)
{

}

mmp_buffer_imagestream::~mmp_buffer_imagestream() {
   
    if(m_fp_imagefile != NULL) {
        fclose(m_fp_imagefile);
        m_fp_imagefile = NULL;
    }

}

