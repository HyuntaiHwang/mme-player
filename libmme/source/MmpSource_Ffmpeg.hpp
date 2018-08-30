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

#ifndef _MMPSOURCE_FFMPEG_HPP__
#define _MMPSOURCE_FFMPEG_HPP__

#include "MmpSource.hpp"

#if (MMP_OS==MMP_OS_WINCE60 || MMP_OS==MMP_OS_WIN32 )


class CMmpSource_Ffmpeg : public CMmpSource
{
friend class CMmpSource;

protected:
    

protected:
    CMmpSource_Ffmpeg(MMPCHAR* strFileName);
    virtual ~CMmpSource_Ffmpeg();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

public:
    virtual MMP_RESULT GetNextData(unsigned char* data, int* dataSize, int maxInputSize, MMP_MEDIATYPE* mediaType, unsigned int* timeStamp);

    virtual MMP_RESULT SetCurTimeStamp(unsigned int ts);
    virtual unsigned int GetCurTimeStamp();
    virtual unsigned int GetStopTimeStamp();
    virtual MMP_BOOL IsSeekable();
};

#endif 
#endif

