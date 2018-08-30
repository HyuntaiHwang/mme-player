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

#include "MmpBitExtractor.hpp"
#include "MmpUtil.hpp"


CMmpBitExtractor::CMmpBitExtractor() :
m_Data(NULL),
m_nDataSize(0),
m_nDataCurBitIndex(0),
m_bRemovePattern(false)
{

}

CMmpBitExtractor::~CMmpBitExtractor()
{
	m_Data=NULL;
    m_nDataSize=0;
    m_nDataCurBitIndex=0;
}


MMP_RESULT CMmpBitExtractor::Start( unsigned char* pAu, int auSize )
{
    m_Data=NULL;
    m_nDataSize=0;
    m_nDataCurBitIndex=0;

    m_Data=pAu;
    m_nDataSize=auSize;
    m_nDataCurBitIndex=0;

    return MMP_SUCCESS;
}



MMP_RESULT CMmpBitExtractor::Stop()
{
    return MMP_SUCCESS;
}

unsigned int CMmpBitExtractor::GetBit(int dataBitIndex)
{
    unsigned int byteIndex,  bitIndex;
    unsigned char c;
    unsigned int i;

    byteIndex=dataBitIndex/8;
    bitIndex=dataBitIndex%8;
    c=m_Data[byteIndex];
    
    i=c;
    i&=0xff;
    i>>=(7-bitIndex);

    return (i&0x01);
}

bool CMmpBitExtractor::CanGetBit(int dataBitIndex)
{
    if( (unsigned int)dataBitIndex/8 >= m_nDataSize ) return false;

    return true; 
}

unsigned long CMmpBitExtractor::Query_BitCode(unsigned long &pt, int length)
{
    int i;
    unsigned int bitv;
    unsigned int dataBitIndex;

    dataBitIndex=m_nDataCurBitIndex;

    pt=0;
    for( i=0; i<length; i++)
    {
       if( !this->CanGetBit(dataBitIndex) )
       {
           pt=0;
           break;
       }
       bitv=this->GetBit(dataBitIndex);

       pt|=(bitv<<(length-1-i));
       dataBitIndex++;
    }

	return pt;
}

unsigned long CMmpBitExtractor::Pop_BitCode(unsigned long &pt, int length)
{
    pt=this->Query_BitCode(pt, length);
    m_nDataCurBitIndex+=length;


    //m_NextQuery32=this->Query_BitCode((unsigned long&)m_NextQuery32, 32);
    
    return pt;
}

unsigned long CMmpBitExtractor::Pop_Bit(int bit) {
    unsigned long pt;
    return this->Pop_BitCode(pt, bit);
}

int CMmpBitExtractor::Pop_1Bit()
{
    unsigned long pt;
    pt=Pop_BitCode(pt,1);
    return (int)pt;
}

unsigned long CMmpBitExtractor::Query_32Bits()
{
    unsigned long pt;
    pt=Query_BitCode(pt,32);
    return pt;
}

void CMmpBitExtractor::FlushBits(unsigned int bitsToFlush)
{
    unsigned long pt;
    pt=Pop_BitCode(pt,bitsToFlush);
}

/*
      Exp-Golomb Code

            BitStreing Form            CodeNum
       --------------------------------------
                 1                       0
               0 1 0                     1
               0 1 1                     2
             0 0 1 0 0                   3
             0 0 1 0 1                   4
             0 0 1 1 0                   5
             0 0 1 1 1                   6
           0 0 0 1 0 0 0                 7
           0 0 0 1 0 0 1                 8
               - - -                     -
*/
unsigned int CMmpBitExtractor::ue_GolombCode()
{
    int i,j,k;
    unsigned int bitv;
    unsigned int dataBitIndex;
    unsigned int codeNum;
    unsigned long pt;

    dataBitIndex=m_nDataCurBitIndex;

    k=0;
    codeNum=0;
    for( i=0; ; i++)
    {
       if( !this->CanGetBit(dataBitIndex) )
       {
           while(1) 
           {
               MMPDEBUGMSG(1, (TEXT("ue_Golomb Code Error1 \n\r")));
           }
           break;
       }
       bitv=this->GetBit(dataBitIndex);

       codeNum<<=1;
       codeNum|=bitv;
       k++;
       dataBitIndex++;

       if( bitv==1 ) break;
    }

    for( j=0; j<i; j++)
    {
        if( !this->CanGetBit(dataBitIndex) )
        {
           while(1) 
           {
               MMPDEBUGMSG( 1, (TEXT("ue_Golomb Code Error1 \n\r")));
           }
           break;
        }
        bitv=this->GetBit(dataBitIndex);
        
        codeNum<<=1;
        codeNum|=bitv;
        k++;
       
        dataBitIndex++;
    }

    this->Pop_BitCode(pt, k);

	return (codeNum-1);
}
   
unsigned int CMmpBitExtractor::te_GolombCode()
{
    int bit;
    bit=this->Pop_1Bit();

    if (bit == 0)
       return 1;
    else
       return 0;   
}

int CMmpBitExtractor::se_GolombCode()
{
    unsigned int codeNum;
    int v;

    codeNum=this->ue_GolombCode();
    codeNum++;

    v = (int)( (codeNum & 0x01) ? (-1*(int)(codeNum >> 1)) : (codeNum >> 1) );  

    return v;
}
   
// check next bitcode if bitcode equal next bitcode of which size is length
bool CMmpBitExtractor::IsNextBits( unsigned int bitCode, int length )
{
   unsigned long v;
   v=this->Query_BitCode(v, length);
   return (v==bitCode)?true:false;
}


int CMmpBitExtractor::ByteAligned()
{
    int bitIndex;

    bitIndex = m_nDataCurBitIndex % 8;
    
    return (bitIndex==0);
}

void CMmpBitExtractor::NextStartCode()
{
    unsigned long pt;
    unsigned int bitPosition;

    bitPosition=m_nDataCurBitIndex%8;
   
    if( bitPosition==0 )
    {
       pt=this->Query_BitCode( pt, 8 );
       if( pt==0x7F )
       {
            pt=this->Pop_BitCode( pt, 8 );
       }
    }
    else
    {
       bitPosition=8-bitPosition;
       pt=this->Pop_BitCode( pt, bitPosition );
    }

}

MMP_RESULT CMmpBitExtractor::GetCurPos( unsigned int* bytePos, unsigned int* bitPos )
{
    unsigned int byteIndex, bitIndex;

    byteIndex=m_nDataCurBitIndex/8;
    bitIndex=m_nDataCurBitIndex%8;

    if(bytePos) *bytePos=byteIndex;
    if(bitPos) *bitPos=bitIndex;

    return MMP_SUCCESS;
}

bool CMmpBitExtractor::EnablePatternRemoval(int pattern)
{
    m_bRemovePattern=true;
    m_nRemovePatternValue=pattern;
    return m_bRemovePattern;
}


MMP_RESULT CMmpBitExtractor::Decode_NextStartCodePrefix()
{
   unsigned long dummy;
   unsigned int codes[3];
   int count;

   // Locating the byte-alligned start code prefix 0x000001
   // -----------------------------------------------------
   codes[0] = (unsigned int)this->Pop_BitCode(dummy, 8);
   codes[1] = (unsigned int)this->Pop_BitCode(dummy, 8);
   codes[2] = (unsigned int)this->Pop_BitCode(dummy, 8);
   
   count=0;
   while (!((codes[0] == 0x00) && (codes[1] == 0x00) && (codes[2] == 0x01)))
   {
      //if (codes[0] == 0xAB && codes[1] == 0xCD && codes[2] == 0xEF )
      //   return MMP_FAILURE;

      codes[0] = codes[1];
      codes[1] = codes[2];
      codes[2] = (unsigned int)this->Pop_BitCode(dummy, 8);

      count++;
      if( count>100) return MMP_FAILURE;
   }
   
   return MMP_SUCCESS;
}

MMP_RESULT CMmpBitExtractor::Decode_NextStartCodePrefix4()
{
   unsigned long dummy;
   unsigned int codes[4];
   int count;

   // Locating the byte-alligned start code prefix 0x000001
   // -----------------------------------------------------
   codes[0] = (unsigned int)this->Pop_BitCode(dummy, 8);
   codes[1] = (unsigned int)this->Pop_BitCode(dummy, 8);
   codes[2] = (unsigned int)this->Pop_BitCode(dummy, 8);
   codes[3] = (unsigned int)this->Pop_BitCode(dummy, 8);
   
   count=0;
   while (!((codes[0] == 0x00) && (codes[1] == 0x00) && (codes[2] == 0x00) && (codes[3] == 0x01) ))
   {
      //if (codes[0] == 0xAB && codes[1] == 0xCD && codes[2] == 0xEF )
      //   return MMP_FAILURE;

      if( (m_nDataCurBitIndex>>3)>= m_nDataSize-4 )
          return MMP_FAILURE;

      codes[0] = codes[1];
      codes[1] = codes[2];
      codes[2] = codes[3];
      codes[3] = (unsigned int)this->Pop_BitCode(dummy, 8);

      //count++;
      //if( count>100) return MMP_FAILURE;
   }
   
   return MMP_SUCCESS;
}

MMP_RESULT CMmpBitExtractor::Decode_NextStartCodePrefix3or4(int* startCodeOffset)
{
   unsigned long dummy;
   unsigned int codes[4];

   codes[0] = (unsigned int)this->Pop_BitCode(dummy, 8);
   codes[1] = (unsigned int)this->Pop_BitCode(dummy, 8);
   codes[2] = (unsigned int)this->Pop_BitCode(dummy, 8);

   if( (codes[0] == 0x00) && (codes[1] == 0x00) && (codes[2] == 0x01) )
   {
       if(startCodeOffset) *startCodeOffset=3;
       return MMP_SUCCESS;
   }

   // Locating the byte-alligned start code prefix 0x000001
   // -----------------------------------------------------
   while(1)
   {
       codes[3] = (unsigned int)this->Pop_BitCode(dummy, 8);

       if( (codes[0] == 0x00) && (codes[1] == 0x00) && (codes[2] == 0x00) && (codes[3] == 0x01))
       {
           if(startCodeOffset) *startCodeOffset=4;
            return MMP_SUCCESS;
       }

   
       if( (m_nDataCurBitIndex>>3)>= m_nDataSize-4 )
       {
           if(startCodeOffset) *startCodeOffset=0;
          return MMP_FAILURE;
       }

       codes[0] = codes[1];
       codes[1] = codes[2];
       codes[2] = codes[3];
       
       if( (codes[0] == 0x00) && (codes[1] == 0x00) && (codes[2] == 0x01) )
       {
           if(startCodeOffset) *startCodeOffset=3;
            return MMP_SUCCESS;
       }
   }
   
   return MMP_SUCCESS;
}

