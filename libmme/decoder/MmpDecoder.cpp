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


#include "MmpDefine.h"
#include "MmpDecoder.hpp"
#include "MmpUtil.hpp"

//////////////////////////////////////////////////////////////
// CMmpDecoder CreateObject/DestroyObject

MMP_RESULT CMmpDecoder::DestroyObject(CMmpDecoder* pObj)
{
    if(pObj)
    {
        pObj->Close();
        delete pObj;
    }
    return MMP_SUCCESS;
}

/////////////////////////////////////////////////////////////
//CMmpDecoder Member Functions

CMmpDecoder::CMmpDecoder(enum MMP_FOURCC fourcc_in, enum MMP_FOURCC fourcc_out, MMP_CHAR* class_name,
                          const enum MMP_FOURCC *support_fourcc_in_list, MMP_S32 support_fourcc_in_count,
                          const enum MMP_FOURCC *support_fourcc_out_list, MMP_S32 support_fourcc_out_count
                        ) :
m_fourcc_in(fourcc_in)
,m_fourcc_out(fourcc_out)

,m_support_fourcc_in_list(support_fourcc_in_list)
,m_support_fourcc_in_count(support_fourcc_in_count)

,m_support_fourcc_out_list(support_fourcc_out_list)
,m_support_fourcc_out_count(support_fourcc_out_count)

{
	m_nClassStartTick = CMmpUtil::GetTickCount();

    sprintf((char*)m_fourcc_in_name, "%c%c%c%c", MMPGETFOURCCARG(m_fourcc_in) );
    sprintf((char*)m_fourcc_out_name, "%c%c%c%c", MMPGETFOURCCARG(m_fourcc_out) );

    strcpy(m_class_name, class_name);
}

CMmpDecoder::~CMmpDecoder()
{
    
}

MMP_RESULT CMmpDecoder::Open()
{
    MMP_BOOL bflag;
    MMP_RESULT mmpResult = MMP_SUCCESS;

    bflag = MMP_TRUE;
    bflag &= this->is_support_fourcc_in(m_fourcc_in);
    /*
        The 'fourcc_out' may be  'MMP_FOURCC_IMAGE_UNKNOWN' because Player may not know that when init.
        
        bflag &= this->is_support_this_fourcc_out(m_fourcc_out);
    */
    if(bflag != MMP_TRUE) {
        mmpResult = MMP_FAILURE;
    }

    return mmpResult;
}

MMP_RESULT CMmpDecoder::Close()
{
    return MMP_SUCCESS;
}

MMP_BOOL CMmpDecoder::is_support_fourcc_in(enum MMP_FOURCC fourcc) {

    enum MMP_FOURCC fourcc_support;
    MMP_BOOL bflag = MMP_FALSE;
    MMP_S32 i;

    for(i = 0; i < m_support_fourcc_in_count; i++) {
        fourcc_support = m_support_fourcc_in_list[i];
        if(fourcc_support == fourcc) {
            bflag = MMP_TRUE;
            break;
        }
    }

    return bflag;
}


MMP_BOOL CMmpDecoder::is_support_fourcc_out(enum MMP_FOURCC fourcc) {

    enum MMP_FOURCC fourcc_support;
    MMP_BOOL bflag = MMP_FALSE;
    MMP_S32 i;

    for(i = 0; i < m_support_fourcc_out_count; i++) {
        fourcc_support = m_support_fourcc_out_list[i];
        if(fourcc_support == fourcc) {
            bflag = MMP_TRUE;
            break;
        }
    }

    return bflag;
}

MMP_RESULT CMmpDecoder::set_fourcc_in(enum MMP_FOURCC fourcc) {
    
    MMP_BOOL bflag;
    MMP_RESULT mmpResult = MMP_FAILURE;

    bflag = this->is_support_fourcc_in(fourcc);
    if(bflag == MMP_TRUE) {
        m_fourcc_in = fourcc;
        sprintf((char*)m_fourcc_in_name, "%c%c%c%c", MMPGETFOURCCARG(m_fourcc_in) );
        mmpResult = MMP_SUCCESS;
    }

    return mmpResult;
}


MMP_RESULT CMmpDecoder::set_fourcc_out(enum MMP_FOURCC fourcc) {
    
    MMP_BOOL bflag;
    MMP_RESULT mmpResult = MMP_FAILURE;

    bflag = this->is_support_fourcc_out(fourcc);
    if(bflag == MMP_TRUE) {
        m_fourcc_out = fourcc;
        sprintf((char*)m_fourcc_out_name, "%c%c%c%c", MMPGETFOURCCARG(m_fourcc_out) );
        mmpResult = MMP_SUCCESS;
    }

    return mmpResult;
}

