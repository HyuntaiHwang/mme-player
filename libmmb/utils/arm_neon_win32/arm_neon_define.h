#ifndef ARM_NEON_DEFINE_H__
#define ARM_NEON_DEFINE_H__

#include "MmpDefine.h"

#if 0

NEON 벡터의 크기 

       NEON 벡터(변수) 는 레지스터와 연결되어 있고,  Neon함수는 레지스터를 제어하기 위해 사용된다. 

     DoubleWord(64bit)  NEON Vector( D Register )
                - 8개의 8bit 요소 
                - 4개의 16bit  요소
                - 2개의 32bit 요소
                - 1개의 64bit 요소 

      QuadWord(128bit)  NEON Vector( Q Register)    
                - 16개의 8bit 요소
                - 8개의  16bit 요소
                - 4개의  32bit 요소
                - 2개의 64bit 요소 

      DoubleWord Neon Vector는  64bit 연산을 지원하고,  QuardWord Neon Vector는 128bit 크기의 연산을 지원한다. 


NEON Register

         Q Register 16개와  D Register 32 개  포함  총 256byte의 Register를 사용해여 연산한다. 
         D Register는  VFP Register와 공유한다. 
                  VFPv2 는  D0~D15 까지 사용
                  VFPv3 는  D0~D31 까지 사용 
         Neon은  D Register를  VFP와 공유하므로  NEON연산과 VFP연산을 동시에 사용하면  레지스터가 충돌하여 예상치 못한 결과가 발생할 수 있다.

#endif

enum ARM_NEON_HL {
    ARM_NEON_LOW = 0,
    ARM_NEON_HIGH 
};

#endif

