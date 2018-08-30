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

#ifndef _MMPSOURCE_YUVFILE_HPP__
#define _MMPSOURCE_YUVFILE_HPP__

#include "MmpSource.hpp"

#define YUVFILE_MAX_COUNT 1024
class CMmpSource_YUVFile : public CMmpSource
{
friend class CMmpSource;

protected:
    FILE* m_fp;
    FILE* m_fp_array[YUVFILE_MAX_COUNT];
    MMP_S32 m_fp_cur_idx;

    MMP_S64 m_source_file_size;

    MMP_S32 m_pic_width;
    MMP_S32 m_pic_height;
    
protected:
    CMmpSource_YUVFile(struct MmpSourceCreateConfig* pCreateConfig);
    virtual ~CMmpSource_YUVFile();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    virtual MMP_RESULT GetNextData_Single(MMP_U8* data, MMP_S32 max_data_len, MMP_S32* data_len);
    virtual MMP_RESULT GetNextData_Multi(MMP_U8* data, MMP_S32 max_data_len, MMP_S32* data_len);

public:
    virtual MMP_RESULT GetNextData(MMP_U8* data, MMP_S32 max_data_len, MMP_S32* data_len);
    virtual MMP_S64 GetSourceFileSize() { return m_source_file_size; }
    
};

#endif

