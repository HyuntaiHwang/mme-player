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


#include "MmpPlayerTestDemuxer.hpp"
#include "MmpUtil.hpp"

/////////////////////////////////////////////////////////////
//CMmpPlayerTestDemuxer Member Functions

CMmpPlayerTestDemuxer::CMmpPlayerTestDemuxer(CMmpPlayerCreateProp* pPlayerProp) : CMmpPlayer(pPlayerProp, "TestDemuxer")
{
    

}

CMmpPlayerTestDemuxer::~CMmpPlayerTestDemuxer()
{
    
}

MMP_RESULT CMmpPlayerTestDemuxer::Open()
{
    MMP_RESULT mmpResult;

    mmpResult=CMmpPlayer::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    
    return MMP_SUCCESS;
}


MMP_RESULT CMmpPlayerTestDemuxer::Close()
{
    MMP_RESULT mmpResult;


    mmpResult=CMmpPlayer::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    return MMP_SUCCESS;
}

void CMmpPlayerTestDemuxer::Service()
{
#if 0
    unsigned char* pAu=NULL;
    unsigned char* pDecodedBuffer=NULL;
    const int MAXBUFSIZE=1920*1088*3/2;
    int auSize;
    MMP_MEDIATYPE mediaType;
    unsigned int timeStamp;
    MMP_RESULT mmpResult;
    CMmpMediaSample mediaSample;
    CMmpMediaSampleDecodeResult decResult;
    int iSampleNumber;

    MMPDEBUGMSG(1, (TEXT("[CMmpPlayerTestDemuxer::Service] Start Service.. \n\r")));

    pAu=new unsigned char[MAXBUFSIZE];
    pDecodedBuffer=new unsigned char[MAXBUFSIZE];

    iSampleNumber=0;
    while(m_bServiceRun)
    {
        //m_pMmpDemuxer->GetNextData()
        //m_pMmpDecoder->DecodeAu();
        //m_pMmpRenderer->Render();
        mmpResult=m_pMmpSource->GetNextData(pAu, &auSize, MAXBUFSIZE, &mediaType, &timeStamp);
        if(mmpResult!=MMP_SUCCESS)
        {
            continue;
        }

        MMPDEBUGMSG(1, (TEXT("[Player] %d (%x %x %x %x %x %x %x %x ) ts(%d) \n\r"),
                    auSize,
                    pAu[0],pAu[1],pAu[2],pAu[3],pAu[4],pAu[5],pAu[6],pAu[7],
                    timeStamp ));

        memset(&mediaSample, 0x00, sizeof(mediaSample));
        memset(&decResult, 0x00, sizeof(decResult));
        mediaSample.m_pAu=pAu;
        mediaSample.m_iAuSize=auSize;
        mediaSample.m_iSampleNumber=iSampleNumber;
        mediaSample.m_iDestLogAddr=(unsigned int)pDecodedBuffer;
        
        iSampleNumber++;
        CMmpOAL_Sleep(10);
    }

    if(pAu)
    {
        delete [] pAu;
    }

    delete [] pDecodedBuffer;

    MMPDEBUGMSG(1, (TEXT("[CMmpPlayerTestDemuxer::Service] Service Ended!! \n\r")));
#endif
}

