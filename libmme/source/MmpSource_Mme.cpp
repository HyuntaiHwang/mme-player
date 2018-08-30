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

#include "MmpSource_Mme.hpp"

#if (MMP_OS==MMP_OS_WINCE60 || MMP_OS==MMP_OS_WIN32 )

/////////////////////////////////////////////////////////////
//CMmpSource_Mme Member Functions

CMmpSource_Mme::CMmpSource_Mme(MMPCHAR* strFileName) : CMmpSource(strFileName)
,m_demuxer_hdl(NULL)
{

}

CMmpSource_Mme::~CMmpSource_Mme()
{

}

MMP_RESULT CMmpSource_Mme::Open()
{
    MMP_RESULT mmpResult;

    mmpResult=CMmpSource::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    MtekMmeIF_Create();

    m_demuxer_hdl=(*MpDemuxer_Create)(MPLAYER_DEMUXER_COM, m_strFileName);
    if(m_demuxer_hdl==NULL)
    {
        return MMP_FAILURE;
    }

    if((*MpDemuxer_GetAudioProp)(m_demuxer_hdl, (void*)&m_MpAudioInfo)!=MP_SUCCESS )
    {
        (*MpDemuxer_Destroy)(m_demuxer_hdl);
        m_demuxer_hdl=NULL;

        return MMP_FAILURE;
    }

    if((*MpDemuxer_GetVideoProp)(m_demuxer_hdl, (void*)&m_MpVideoInfo)!=MP_SUCCESS )
    {
        (*MpDemuxer_Destroy)(m_demuxer_hdl);
        m_demuxer_hdl=NULL;

        return MMP_FAILURE;
    }

    m_pMediaInfo[MMP_MEDIATYPE_AUDIO]->SetAudioInfo((MMPWAVEFORMATEX*)&m_MpAudioInfo.wf, m_MpAudioInfo.cb, m_MpAudioInfo.wf.cbSize);
    m_pMediaInfo[MMP_MEDIATYPE_VIDEO]->SetVideoInfo((MMPBITMAPINFOHEADER*)&m_MpVideoInfo.bih, m_MpVideoInfo.dsi, m_MpVideoInfo.dsiSize);

    return MMP_SUCCESS;
}


MMP_RESULT CMmpSource_Mme::Close()
{
    MMP_RESULT mmpResult;

    if(m_demuxer_hdl)
    {
        (*MpDemuxer_Destroy)(m_demuxer_hdl);
        m_demuxer_hdl=NULL;
    }

    MtekMmeIF_Destroy();

    mmpResult=CMmpSource::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpSource_Mme::GetNextData(unsigned char* data, int* dataSize, int maxInputSize, MMP_MEDIATYPE* mediaType, unsigned int* timeStamp)
{
    return (*MpDemuxer_GetNextData)(m_demuxer_hdl, data, dataSize, maxInputSize, (MPLAYER_MEDIA_TYPE*)mediaType, timeStamp);
}

MMP_RESULT CMmpSource_Mme::SetCurTimeStamp(unsigned int ts)
{
    return (*MpDemuxer_SetCurTimeStamp)(m_demuxer_hdl, ts);
}

unsigned int CMmpSource_Mme::GetCurTimeStamp()
{
    unsigned int ts;
    (*MpDemuxer_GetCurTimeStamp)(m_demuxer_hdl, &ts);
    return ts;
}

unsigned int CMmpSource_Mme::GetStopTimeStamp()
{
    unsigned int ts;
    (*MpDemuxer_GetStopTimeStamp)(m_demuxer_hdl, &ts);
    return ts;
}

MMP_BOOL CMmpSource_Mme::IsSeekable()
{
    MMP_BOOL mmpFlag;
    mmpFlag=(*MpDemuxer_IsSeekable)(m_demuxer_hdl)?MMP_TRUE:MMP_FALSE;
    return mmpFlag;
}

#endif //#if (MMP_OS==MMP_OS_WINCE || MMP_OS==MMP_OS_WIN32 )