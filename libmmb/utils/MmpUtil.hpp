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

#ifndef _HEADER_MMPUTIL_HPP
#define _HEADER_MMPUTIL_HPP

/******************************************************************************
 * INCLUDE FILES                                                           
 ******************************************************************************/
#include "MmpDefine.h"
#include "mmp_util.h"

/*******************************************************************************
 * LITERALS & DEFINITIONS                                              
 ******************************************************************************/

/*******************************************************************************
 * class CMmpSize
 ******************************************************************************/
class CMmpSize
{
public:
    int m_cx;
    int m_cy;

public:
    CMmpSize() { m_cx=m_cy=0;}
    CMmpSize(int w, int h) { m_cx=w, m_cy=h; }
};

/*******************************************************************************
 * class CMmpPoint
 ******************************************************************************/
class CMmpPoint
{
public:
    int m_x;
    int m_y;

public:
    CMmpPoint() { m_x=m_y=0;}
    CMmpPoint(int x, int y) { m_x=x, m_y=y; }

    void Set(int x, int y) { m_x=x, m_y=y; }
};

/*******************************************************************************
 * class CMediaSampleProp
 ******************************************************************************/
/*
class CMediaSampleProp
{
public:
    unsigned char* m_pMediaSample;
    unsigned int m_iSrcPhyAddr;
    int m_iMediaSampleSize;
    int m_iSampleNumber;
    int m_iDecodedFormat;
    unsigned int m_iDestLogAddr;
    unsigned int m_iDestPhyAddr;
};

*/


/*******************************************************************************
 * class CMmpUtil
 ******************************************************************************/
class CMmpUtil
{
public:

    static MMP_U32 GetTickCount();
    static MMP_S64 GetTickCountUS();
    static void Sleep(int milesec);

    static void* RegisterMap_Allocate( char* regName );
    static MMP_RESULT RegisterMap_Free( char* regName, void* pReg );

    static MMP_RESULT PhyMem_Alloc( unsigned int* logAddr, unsigned int* phyAddr, int allocSize, bool bCacheEnabled=false );
    static MMP_RESULT PhyMem_Free( unsigned int logAddr, unsigned int phyAddr, int allocSize, bool bCacheEnabled=false );
    static MMP_RESULT PhyMem_SetData( unsigned int logAddr, unsigned int phyAddr, unsigned char* pdata, int dataSize );
    static MMP_RESULT PhyMem_GetData( unsigned int logAddr, unsigned int phyAddr, unsigned char* pdata, int dataSize );

    static MMP_RESULT PhyMem_CopyToImageBuffer( unsigned int logAddr, unsigned int phyAddr, 
                                               unsigned char* pDestdata, 
                                               int imageWidth, int imageHeight,
                                               int imageFormat, int imageFlipUpDown );

    static MMP_RESULT PhyMem_VirtualCopy(  unsigned int virtualAddr,
                                          unsigned int logAddr, unsigned int phyAddr, 
                                          int allocSize );

    static MMP_RESULT CovertAnsiToUnicode( const char* strAnsi, WCHAR* strUnicode );
    static MMP_RESULT CovertUnicodeToAnsi( const WCHAR* strUnicode, char* strAnsi );

    
    static MMP_RESULT SplitDir(WCHAR* strFileName, WCHAR* wszDir);// 입력된 전체 파일명에서 디렉토리만 뽑아낸다. ex) "c:\mylib\aa.dat" => "c:\mylib\"
	static MMP_RESULT SplitFileName(MMP_CHAR* strFileName, MMP_CHAR* wszName); // 입력된 전체 파일명에서 파일이름만 뽑아낸다. ex) "c:\mylib\aa.dat" => "aa"
	static MMP_RESULT SplitExt(MMP_CHAR* strFileName, MMP_CHAR* wszExt);// 입력된 전체 파일명에서 확장자만 뽑아낸다. ex) "c:\mylib\aa.dat" => dat
	static MMP_RESULT MakeLower(MMP_CHAR* wszName);// 입력된 문자열을 모두 소문자로 만든다 ex) "AbcDe" => "abcde"
	static MMP_RESULT MakeUpper(MMP_CHAR* wszName);// 입력된 문자열을 모두 대문자로 만든다 ex) "AbcDe" => "ABCDE"
	
    static MMP_RESULT SplitDirC(MMP_CHAR* strFileName, MMP_CHAR* wszDir);// 입력된 전체 파일명에서 디렉토리만 뽑아낸다. ex) "c:\mylib\aa.dat" => "c:\mylib\"
	//static MMP_RESULT SplitFileNameC(MMP_CHAR* strFileName, MMP_CHAR* wszName); // 입력된 전체 파일명에서 파일이름만 뽑아낸다. ex) "c:\mylib\aa.dat" => "aa"
	static MMP_RESULT SplitExtC(MMP_CHAR* strFileName, MMP_CHAR* wszExt);// 입력된 전체 파일명에서 확장자만 뽑아낸다. ex) "c:\mylib\aa.dat" => dat
	static MMP_RESULT MakeLowerC(MMP_CHAR* wszName);// 입력된 문자열을 모두 소문자로 만든다 ex) "AbcDe" => "abcde"

    static MMP_RESULT HexStringToInt(MMP_CHAR* szHex, MMP_S32* value); // Hex String을 숫자로 만든다. 
	
    //Color Convert
    static MMP_RESULT ColorConvertYUV420PlanarToRGB565(int picWidth, int picHeight, 
                                                              unsigned char* Y, unsigned char* U, unsigned char* V,
                                                              unsigned short* RGB565,
                                                              bool bTopDown
                                                              );
	
   
   static MMP_RESULT ColorConvertYUV420PlanarToRGB32( int picWidth, int picHeight, 
                                                                   unsigned char* Y, unsigned char* U, unsigned char* V,
                                                                    unsigned int* RGB32,
                                                                    bool bTopDown
                                                              );
   
   static MMP_RESULT ColorConvertYUV420PlanarToRGB24( int picWidth, int picHeight, 
                                                                   unsigned char* Y, unsigned char* U, unsigned char* V,
                                                                    unsigned char* RGB24
                                                              );
   
   static MMP_RESULT ColorConvertRGB32ToYUV420Planar(int picWidth, int picHeight, int stride, 
                                                     unsigned int* RGB32,
                                                     unsigned char* Y, unsigned char* U, unsigned char* V,
                                                     bool bTopDown
                                                    );

   static MMP_RESULT ColorConvertRGB24ToYUV420Planar(int picWidth, int picHeight, int stride_byte, 
                                                     unsigned char* RGB24,
                                                     unsigned char* Y, unsigned char* U, unsigned char* V, int luma_stride, int chroma_stride 
                                                    );

   static MMP_RESULT ColorConvertRGB8Pallete32ToYUV420Planar(int picWidth, int picHeight, int stride, 
                                                     unsigned char* rgb, unsigned int* pallete,
                                                     unsigned char* Y, unsigned char* U, unsigned char* V, int luma_stride, int chroma_stride 
                                                    );

   static MMP_RESULT ColorConvertYUV420PlanarToYUV420SemiPlanar(int picWidth, int picHeight, 
                                                                             unsigned char* Y, unsigned char* U, unsigned char* V, int luma_stride, int chroma_stride,
                                                                             unsigned char* Y1, unsigned char* UV1
                                                                             );

   
   static unsigned int  MakeUpperFourcc(unsigned int fourcc);
   static MMP_BOOL  CheckVideoFourcc(unsigned int fourcc, unsigned char* formatName);
   static MMP_BOOL  CheckAudioFormatTag(enum MMP_FOURCC fourcc, unsigned char* formatName);
    
   static MMP_BOOL IsMediaFileExtValid(WCHAR* wszName);

    static void* YSR_Create(char* fileName, int picWidth, int picHeight, bool bRead);
    static void  YSR_Write(void* handle, unsigned char* data);
    static void  YSR_Destroy(void* handle);


    //Time
    static int Time_GetHour(unsigned int timeStamp); //Unit: milesec
    static int Time_GetMin(unsigned int timeStamp);  //Unit: milesec
    static int Time_GetSec(unsigned int timeStamp);  //Unit: milesec 
    static int Time_GetMileSec(unsigned int timeStamp); //Unit: milesec

    /*File Tool */
    static MMP_RESULT File_Make(char* filename, MMP_U8* buffer, MMP_U32 buf_size);

    /* Jpeg */
    static MMP_RESULT Jpeg_SW_YUV420Planar_Enc(unsigned char* Y, unsigned char* U, unsigned char* V, 
                               int image_width, int image_height,
                               char* jpegfilename, int quality);

#define MMPUTIL_JPEG_ENC_FORMAT_YUV444_PACKED_YUV  0     
#define MMPUTIL_JPEG_ENC_FORMAT_YUV422_PACKED_YUYV 1     
#define MMPUTIL_JPEG_ENC_FORMAT_YUV411_PACKED      2     
#define MMPUTIL_JPEG_ENC_FORMAT_GRAY               3     
#define MMPUTIL_JPEG_ENC_FORMAT_RGB24BIT           4     
    static MMP_RESULT Jpeg_SW_YUV420Planar_Enc(unsigned char* Y, unsigned char* U, unsigned char* V, 
                               int image_width, int image_height,
                               char* jpegfilename, int quality, 
                               int enc_format /* 0:yuv444  1:yuv422  2:yuv411  3:gray */
                               );
    
    static int GetFileList(char* path, char* ext, int extcnt, int ext_size_max, char* file_array, int file_array_max, int file_size_max);
    static int CompareByAlphanumeric(const char *str1, const char* str2); /* 0 : equal  -1 : str1 front  1 : str1 rear */

    static MMP_RESULT system_meminfo(struct mmp_system_meminfo* p_meminfo);
    static MMP_U32 GetSystemJiffies(void);
    static MMP_RESULT system_cpu_usage(struct mmp_system_cpu_usage* p_cpu_usage);

    static MMP_BOOL IsVideoDecFrameValid(MMP_S32 width, MMP_S32 height);
    static MMP_BOOL IsVideoEncFrameValid(MMP_S32 width, MMP_S32 height);

    static MMP_BOOL IsDebugEnable(const char* name);
    static MMP_BOOL IsDebugEnable_VPUDec(void);
    static MMP_BOOL IsDebugEnable_VPUEnc(void);
    static MMP_BOOL IsDebugEnable_MMEDec(void);
    static MMP_BOOL IsDebugEnable_MMEEnc(void);
    static MMP_BOOL IsDebugEnable_OMXBase(void);
    static MMP_BOOL IsDebugEnable_OMXVideoDec(void);
    static MMP_BOOL IsDebugEnable_OMXVideoEnc(void);
    static MMP_BOOL IsDebugEnable_OMXAudioDec(void);
    static MMP_BOOL IsDebugEnable_OMXAudioEnc(void);


    /**********************************************************
    CTS 
    **********************************************************/
    static MMP_BOOL CTS_CheckSurfaceFrame_RGB32(MMP_S32 frameIndex, 
                                                MMP_S32 width, MMP_S32 height, MMP_PTR data,
                                                MMP_S32 TEST_R0, MMP_S32 TEST_G0, MMP_S32 TEST_B0,
                                                MMP_S32 TEST_R1, MMP_S32 TEST_G1, MMP_S32 TEST_B1,
                                                MMP_BOOL TopDown
                                                );

};

#ifdef __cplusplus
extern "C" {
#endif

MMP_RESULT CMmpUtil_Printf(char* lpszFormat, ... );

#ifdef __cplusplus
}
#endif

    //static MMP_RESULT Printf( WCHAR* lpszFormat, ... );

//#define MMP_SWAP_I16(v) (short)((((v)>>8)  & 0xff) | (((v)&0xff) << 8))
//#define MMP_SWAP_I32(v) (int)((((v)>>24) & 0xff) | (((v)>>8) & 0xff00) | (((v)&0xff00) << 8) | (((v)&0xff) << 24))
//#define MMP_SWAP_U16(v) (unsigned short)((((v)>>8)  & 0xff) | (((v)&0xff) << 8))
//#define MMP_SWAP_U32(v) (unsigned int)((((v)>>24) & 0xff) | (((v)>>8) & 0xff00) | (((v)&0xff00) << 8) | (((v)&0xff) << 24))

#endif //#ifndef _MTUTIL_H__



