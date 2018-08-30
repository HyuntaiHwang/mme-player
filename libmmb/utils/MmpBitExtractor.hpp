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


#ifndef	__MMPBITEXTRACTOR_HPP
#define	__MMPBITEXTRACTOR_HPP

#include "MmpDefine.h"

class CMmpBitExtractor
{
private:
   unsigned char* m_Data;
   unsigned int m_nDataSize;
   unsigned int m_nDataCurBitIndex;
   bool m_bRemovePattern;
   unsigned int m_nRemovePatternValue;
   unsigned int m_NextQuery32;
   
private:
   unsigned int GetBit(int dataBitIndex);
   bool EnablePatternRemoval(int pattern);

public:   
   CMmpBitExtractor();
   ~CMmpBitExtractor();

   MMP_RESULT Start( unsigned char* pAu, int auSize );
   MMP_RESULT Stop();

    __inline static long Query_1Bit()
    {
    return 0;   // not implemented yet
    }

    __inline static long FlushLastAndQuery_1Bit()
    {
    return 0;   // not implemented yet
    }

#if 0
    __inline static long Query_BitCode(int lenght)
    {
        
        return 0;   // not implemented yet
    }
#endif

    __inline void FlushLast ()
    {
    
    } 

    inline unsigned int GetCurByteIndex() { return m_nDataCurBitIndex>>3; }
    inline unsigned int GetCurBitIndex() { return m_nDataCurBitIndex; }
    inline unsigned int GetRemainBitSize() { return (m_nDataSize*8 - m_nDataCurBitIndex); }

   
public:
   unsigned long Query_BitCode(unsigned long &pt, int length);
   unsigned long Pop_BitCode(unsigned long &pt, int length);
   unsigned long Pop_Bit(int bit);
   bool IsNextBits( unsigned int bitCode, int length ); // check next bitcode if bitcode equal next bitcode of which size is length

   unsigned int ue_GolombCode();
   unsigned int te_GolombCode();
   int se_GolombCode();
   
   int Pop_1Bit();
   unsigned long Query_32Bits();

   void FlushBits(unsigned int bitsToFlush);

   int ByteAligned();
   void NextStartCode();

   MMP_RESULT GetCurPos( unsigned int* bytePos, unsigned int* bitPos );

   MMP_RESULT Decode_NextStartCodePrefix(); //default 00 00 01
   MMP_RESULT Decode_NextStartCodePrefix4(); //00 00 00 01
   MMP_RESULT Decode_NextStartCodePrefix3or4(int* startCodeOffset); //00 00 00 01 or 00 00 01
   
   int GetDataSize() { return m_nDataSize;}
   bool CanGetBit(int dataBitIndex);
    
};


#endif // __MEDIAPLAYER_HPP

