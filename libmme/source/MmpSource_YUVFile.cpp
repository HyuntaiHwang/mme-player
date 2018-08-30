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

#include "MmpSource_YUVFile.hpp"


/////////////////////////////////////////////////////////////
//CMmpSource_YUVFile Member Functions

CMmpSource_YUVFile::CMmpSource_YUVFile(struct MmpSourceCreateConfig* pCreateConfig) : CMmpSource(pCreateConfig)
,m_fp(NULL)
,m_source_file_size(0)
,m_pic_width(pCreateConfig->width)
,m_pic_height(pCreateConfig->height){
    
    MMP_S32 i;

    m_fp_cur_idx = 0;
    for(i = 0; i < YUVFILE_MAX_COUNT; i++) {
        m_fp_array[i] = NULL;
    }
}

CMmpSource_YUVFile::~CMmpSource_YUVFile()
{

}

MMP_RESULT CMmpSource_YUVFile::Open()
{
    MMP_RESULT mmpResult;
    MMP_S32 i;
    MMP_CHAR filename[512];


    mmpResult=CMmpSource::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    if(this->m_create_config.file_count > 1) {

        m_source_file_size = 0;
        for(i = 0; i < this->m_create_config.file_count; i++) {
            sprintf(filename, "%s%d", this->m_create_config.filename, (int)i);
            m_fp_array[i] = fopen(filename, "rb");
            if(m_fp_array[i] == NULL) {
                mmpResult = MMP_FAILURE;
                break;
            }
            else {
#if (MMP_OS == MMP_OS_LINUX)
                fseeko(m_fp_array[i], 0, SEEK_END);
                m_source_file_size += ftello(m_fp_array[i]);
                fseeko(m_fp_array[i], 0, SEEK_SET);
#elif (MMP_OS == MMP_OS_WIN32)
                _fseeki64(m_fp_array[i], 0, SEEK_END);
                m_source_file_size += _ftelli64(m_fp_array[i]);
                _fseeki64(m_fp_array[i], 0, SEEK_SET);
#else
#error "ERROR : Select OS"
#endif
            }
        }
    }
    else {
        m_fp = fopen((char*)this->m_create_config.filename, "rb");
        if(m_fp == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {
#if (MMP_OS == MMP_OS_LINUX)
            fseeko(m_fp, 0, SEEK_END);
            m_source_file_size += ftello(m_fp);
            fseeko(m_fp, 0, SEEK_SET);
#elif (MMP_OS == MMP_OS_WIN32)
            _fseeki64(m_fp, 0, SEEK_END);
            m_source_file_size = _ftelli64(m_fp);
            _fseeki64(m_fp, 0, SEEK_SET);
#else
#error "ERROR : Select OS"
#endif
        }
    }
    
    return mmpResult;
}


MMP_RESULT CMmpSource_YUVFile::Close()
{
    MMP_RESULT mmpResult;
    MMP_S32 i;

    mmpResult=CMmpSource::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    if(m_fp != NULL) {
        fclose(m_fp);
        m_fp = NULL;
    }
    
    for(i = 0; i < YUVFILE_MAX_COUNT; i++) {
        if(m_fp_array[i] != NULL) {
            fclose(m_fp_array[i]);
            m_fp_array[i] = NULL;
        }
    }

    return MMP_SUCCESS;
}



MMP_RESULT CMmpSource_YUVFile::GetNextData(MMP_U8* data, MMP_S32 max_data_len, MMP_S32* data_len)
{
    MMP_RESULT mmpResult = MMP_FAILURE;

    if(this->m_create_config.file_count > 1) {
        mmpResult = this->GetNextData_Multi(data, max_data_len, data_len);
    }
    else {
        mmpResult = this->GetNextData_Single(data, max_data_len, data_len);
    }

    return mmpResult;
}

MMP_RESULT CMmpSource_YUVFile::GetNextData_Single(MMP_U8* data, MMP_S32 max_data_len, MMP_S32* data_len)
{
    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_S32 yuvsize, readsize;

    yuvsize = m_pic_width*m_pic_height*3/2;
    if(data_len != NULL) *data_len = 0;

    if(m_fp != NULL) {
        readsize  = fread(data, 1, yuvsize, m_fp);
        if(readsize == yuvsize) {
            mmpResult = MMP_SUCCESS;
            if(data_len != NULL) *data_len = readsize;
        }
    }

    return mmpResult;
}

MMP_RESULT CMmpSource_YUVFile::GetNextData_Multi(MMP_U8* data, MMP_S32 max_data_len, MMP_S32* data_len)
{
    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_S32 yuvsize, readsize;
    FILE* fp;

    yuvsize = m_pic_width*m_pic_height*3/2;
    if(data_len != NULL) *data_len = 0;

    fp = m_fp_array[m_fp_cur_idx];
    while(fp != NULL) {

        readsize  = fread(data, 1, yuvsize, fp);
        if(readsize == yuvsize) {
            mmpResult = MMP_SUCCESS;
            if(data_len != NULL) *data_len = readsize;
            break;
        }
        else {
            m_fp_cur_idx++;
            fp = m_fp_array[m_fp_cur_idx];
        }
    }

    return mmpResult;
}
