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

#include "MmpSource_Ffmpeg.hpp"

#if (MMP_OS==MMP_OS_WINCE60 || MMP_OS==MMP_OS_WIN32 )

/////////////////////////////////////////////////////////////
//CMmpSource_Ffmpeg Member Functions

CMmpSource_Ffmpeg::CMmpSource_Ffmpeg(MMPCHAR* strFileName) : CMmpSource(strFileName)
{

}

CMmpSource_Ffmpeg::~CMmpSource_Ffmpeg()
{

}

MMP_RESULT CMmpSource_Ffmpeg::Open()
{
    MMP_RESULT mmpResult;

    mmpResult=CMmpSource::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    
    return MMP_SUCCESS;
}


MMP_RESULT CMmpSource_Ffmpeg::Close()
{
    MMP_RESULT mmpResult;


    mmpResult=CMmpSource::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpSource_Ffmpeg::GetNextData(unsigned char* data, int* dataSize, int maxInputSize, MMP_MEDIATYPE* mediaType, unsigned int* timeStamp)
{
    return MMP_FAILURE;
}

MMP_RESULT CMmpSource_Ffmpeg::SetCurTimeStamp(unsigned int ts)
{
    return MMP_FAILURE;
}

unsigned int CMmpSource_Ffmpeg::GetCurTimeStamp()
{
    return 0;
}

unsigned int CMmpSource_Ffmpeg::GetStopTimeStamp()
{
    return 0;
}

MMP_BOOL CMmpSource_Ffmpeg::IsSeekable()
{
    return 0;
}

#endif //#if (MMP_OS==MMP_OS_WINCE || MMP_OS==MMP_OS_WIN32 )