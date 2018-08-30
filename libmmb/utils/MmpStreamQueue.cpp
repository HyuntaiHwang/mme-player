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

#include "MmpStreamQueue.hpp"

CMmpStreamQueue::CMmpStreamQueue() :
m_queue(30)
{

}

CMmpStreamQueue::~CMmpStreamQueue()
{
    CMmpStreamObject* pObj;

    while(!m_queue.IsEmpty())
    {
        m_queue.Delete(pObj);
        delete pObj;
    }

    
}

bool CMmpStreamQueue::IsEmpty()
{
    return m_queue.IsEmpty()?true:false;
}

MMP_RESULT CMmpStreamQueue::Add(unsigned char* data, int dataSize)
{
    CMmpStreamObject* pObj;

    if(dataSize>MMPSTREAMOBJECT_BUFSIZE)
        return MMP_FAILURE;

    if(m_queue.IsFull())
        return MMP_FAILURE;

    pObj=new CMmpStreamObject(data, dataSize);
    if(!pObj)
        return MMP_FAILURE;

    m_queue.Add(pObj);

    return MMP_SUCCESS;
}

MMP_RESULT CMmpStreamQueue::Delete(unsigned char* data)
{
    CMmpStreamObject* pObj;

    if(m_queue.IsEmpty())
        return MMP_FAILURE;

    m_queue.GetFirstItem(pObj);
    
    *data=pObj->Delete();
    if(pObj->IsEmpty())
    {
        m_queue.Delete(pObj);
        delete pObj;
    }

    return MMP_SUCCESS;
}

