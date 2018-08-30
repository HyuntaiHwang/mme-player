/*
 *
 *  Copyright (C) 2010-2011 TokiPlayer Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _MMPSOURCE_MME_HPP__
#define _MMPSOURCE_MME_HPP__

#include "MmpSource.hpp"

#if (MMP_OS==MMP_OS_WINCE60 || MMP_OS==MMP_OS_WIN32 )

#include "MtekMmeIF.h"

class CMmpSource_Mme : public CMmpSource
{
friend class CMmpSource;

protected:
    MPDEMUXER_HDL m_demuxer_hdl;
    MPLAYER_VIDEO_INFO m_MpVideoInfo;
    MPLAYER_AUDIO_INFO m_MpAudioInfo;


protected:
    CMmpSource_Mme(MMPCHAR* strFileName);
    virtual ~CMmpSource_Mme();

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

