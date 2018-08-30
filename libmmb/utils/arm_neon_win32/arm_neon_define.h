#ifndef ARM_NEON_DEFINE_H__
#define ARM_NEON_DEFINE_H__

#include "MmpDefine.h"

#if 0

NEON ������ ũ�� 

       NEON ����(����) �� �������Ϳ� ����Ǿ� �ְ�,  Neon�Լ��� �������͸� �����ϱ� ���� ���ȴ�. 

     DoubleWord(64bit)  NEON Vector( D Register )
                - 8���� 8bit ��� 
                - 4���� 16bit  ���
                - 2���� 32bit ���
                - 1���� 64bit ��� 

      QuadWord(128bit)  NEON Vector( Q Register)    
                - 16���� 8bit ���
                - 8����  16bit ���
                - 4����  32bit ���
                - 2���� 64bit ��� 

      DoubleWord Neon Vector��  64bit ������ �����ϰ�,  QuardWord Neon Vector�� 128bit ũ���� ������ �����Ѵ�. 


NEON Register

         Q Register 16����  D Register 32 ��  ����  �� 256byte�� Register�� ����ؿ� �����Ѵ�. 
         D Register��  VFP Register�� �����Ѵ�. 
                  VFPv2 ��  D0~D15 ���� ���
                  VFPv3 ��  D0~D31 ���� ��� 
         Neon��  D Register��  VFP�� �����ϹǷ�  NEON����� VFP������ ���ÿ� ����ϸ�  �������Ͱ� �浹�Ͽ� ����ġ ���� ����� �߻��� �� �ִ�.

#endif

enum ARM_NEON_HL {
    ARM_NEON_LOW = 0,
    ARM_NEON_HIGH 
};

#endif

