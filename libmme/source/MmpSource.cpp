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

#include "MmpSource.hpp"
#include "MmpSource_YUVFile.hpp"
#include "MmpOAL.hpp"
#include "MmpUtil.hpp"

//////////////////////////////////////////////////////////////
// CMmpSource CreateObject/DestroyObject

CMmpSource* CMmpSource::CreateObject(struct MmpSourceCreateConfig* pCreateConfig)
{
    CMmpSource* pObj = NULL;

    MMP_CHAR szExt[32];

    CMmpUtil::SplitExt((MMP_CHAR*)pCreateConfig->filename, szExt);
    CMmpUtil::MakeLower(szExt);

    if(strcmp(szExt, "yuv") == 0) {
        pObj = new class CMmpSource_YUVFile(pCreateConfig);
    }

    
    if(pObj==NULL)
        return (CMmpSource*)NULL;

    if( pObj->Open()!=MMP_SUCCESS )    
    {
        pObj->Close();
        delete pObj;
        return (CMmpSource*)NULL;
    }

    return pObj;
}

MMP_RESULT CMmpSource::DestroyObject(CMmpSource* pObj)
{
    if(pObj)
    {
        pObj->Close();
        delete pObj;
    }
    return MMP_SUCCESS;
}

/////////////////////////////////////////////////////////////
//CMmpSource Member Functions

CMmpSource::CMmpSource(struct MmpSourceCreateConfig* pCreateConfig) :
m_create_config(*pCreateConfig)
{
    

}

CMmpSource::~CMmpSource()
{

}

MMP_RESULT CMmpSource::Open()
{
    
    return MMP_SUCCESS;
}


MMP_RESULT CMmpSource::Close()
{
    
    return MMP_SUCCESS;
}


MMP_RESULT CMmpSource::YUVFIle_DevideUnder2GB(MMP_CHAR* filename, MMP_S32 width, MMP_S32 height) {
    
    int err_cnt = 0;
    CMmpSource* pSource = NULL;
    struct MmpSourceCreateConfig  source_create_config;
    MMP_U8* yuvdata;
    MMP_S32 datalen;

    MMP_S32 filecnt = 0;
    MMP_U32 filesize = 0, readsize = 0;
    MMP_CHAR newfilename[256];
    FILE* fp;

    yuvdata = new MMP_U8[width*height*3];

    strcpy((char*)source_create_config.filename, filename);
    source_create_config.width = width;
    source_create_config.height = height;
    pSource = CMmpSource::CreateObject(&source_create_config);
    if(pSource == NULL) {
        err_cnt++;
    }

    sprintf(newfilename, "%s%d", filename, filecnt);
    fp = fopen(newfilename, "wb");
    
    while(fp!=NULL) {
        
        datalen = 0;
        pSource->GetNextData(yuvdata, width*height*3, &datalen);
        if(datalen == 0) {
            break;
        }

        fwrite(yuvdata, 1, datalen, fp);
        filesize+=datalen;
        readsize+=datalen;
        if(filesize > (1024*1024*(1024+256*3))) {
            
            fclose(fp);
            
            filecnt++;
            filesize = 0;

            sprintf(newfilename, "%s%d", filename, filecnt);
            fp = fopen(newfilename, "wb");
        }

        MMPDEBUGMSG(1, (TEXT("[YUVFile Div] fileIdx(%d) fsz(%d)  readsz(%d/%d) "), filecnt, filesize, readsize, pSource->GetSourceFileSize() ));

    }

    fclose(fp);

    if(pSource != NULL) {
        CMmpSource::DestroyObject(pSource);
    }

    delete [] yuvdata;

    return MMP_SUCCESS;
}