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

#ifndef _MMPSOURCE_HPP__
#define _MMPSOURCE_HPP__

#include "MmpPlayerDef.h"
#include "MmpMediaInfo.hpp"


struct MmpSourceCreateConfig
{
    MMP_U32 type;
    MMP_U8 filename[256];
    MMP_S32 file_count;

    MMP_S32 width;
    MMP_S32 height;
};

class CMmpSource
{
public:
    static CMmpSource* CreateObject(struct MmpSourceCreateConfig* pCreateConfig);
    static MMP_RESULT DestroyObject(CMmpSource* pObj);

    static MMP_RESULT YUVFIle_DevideUnder2GB(MMP_CHAR* filename, MMP_S32 width, MMP_S32 height);

protected:
    struct MmpSourceCreateConfig m_create_config;
    

protected:
    CMmpSource(struct MmpSourceCreateConfig* pCreateConfig);
    virtual ~CMmpSource();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

public:

    virtual MMP_RESULT GetNextData(MMP_U8* data, MMP_S32 max_data_len, MMP_S32* data_len) = 0;
    virtual MMP_S64 GetSourceFileSize() = 0;
    
};

#endif

