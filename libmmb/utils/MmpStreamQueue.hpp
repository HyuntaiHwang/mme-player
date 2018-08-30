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

#ifndef _MMPSTREAMQUEUE_HPP__
#define _MMPSTREAMQUEUE_HPP__

#include "MmpDefine.h"
#include "TemplateList.hpp"

#define MMPSTREAMOBJECT_BUFSIZE 4096

class CMmpStreamObject
{
private:
    unsigned char m_Buffer[MMPSTREAMOBJECT_BUFSIZE];
    int m_iDataSize;
    int m_iDataReadIndex;

public:
    CMmpStreamObject()
    {
        m_iDataSize=0;
        m_iDataReadIndex=0; 
    }

    CMmpStreamObject(unsigned char* data, int dataSize) 
    {  
        memcpy(m_Buffer, data, dataSize); 
        m_iDataSize=dataSize;
        m_iDataReadIndex=0; 
    }

    inline void Set(unsigned char* data, int dataSize)
    {
        memcpy(m_Buffer, data, dataSize); 
        m_iDataSize=dataSize;
        m_iDataReadIndex=0; 
    }

    inline bool IsEmpty() { return (m_iDataReadIndex>=m_iDataSize)?true:false; }
    inline unsigned char Delete() 
    {
        char ch;
        ch=m_Buffer[m_iDataReadIndex];
        m_iDataReadIndex++;
        return ch;
    }
};

class CMmpStreamQueue
{
private:
    TCircular_Queue<CMmpStreamObject*> m_queue;
    

public:
    CMmpStreamQueue();
    ~CMmpStreamQueue();

    MMP_RESULT Add(unsigned char* data, int dataSize);
    MMP_RESULT Delete(unsigned char* data);
    bool IsEmpty();

};


#endif