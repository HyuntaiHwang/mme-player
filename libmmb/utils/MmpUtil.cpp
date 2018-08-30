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

/*******************************************************************************
 * INCLUDE FILES                                                           
 ******************************************************************************/
#include "MmpDefine.h"
#include "MmpUtil.hpp"
#include "TemplateList.hpp"

#ifdef __OMX_PLATFORM_ANDROID
#if (MMP_OS == MMP_OS_LINUX)
extern "C"  {
#include "color_space_convertor.h"
}
#endif
#endif

#if (MMP_OS == MMP_OS_LINUX)
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#include <cutils/log.h>
#include <cutils/properties.h>


//#if (MMP_OS!=MMP_OS_WIN32 )
//#include "libyuv.h"
//#endif

#if (MMP_OS==MMP_OS_WIN32 )
MMP_U32 CMmpUtil::GetTickCount() {
    return ::GetTickCount();
}

MMP_S64 CMmpUtil::GetTickCountUS() {
    return (MMP_S64)(::GetTickCount())*1000;
}

void CMmpUtil::Sleep(int milesec) {
    ::Sleep(milesec);
}

#elif (MMP_OS==MMP_OS_LINUX )

#if 0
class __GET_TICK_COUNT
{
public:
    __GET_TICK_COUNT()
    {
        gettimeofday(&tv_, NULL);
    }
    timeval tv_;
};
static __GET_TICK_COUNT timeStart;
#endif


MMP_U32 CMmpUtil::GetTickCount() {

  #if 0  
    static time_t   secStart    = timeStart.tv_.tv_sec;
    static time_t   usecStart   = timeStart.tv_.tv_usec;
    timeval tv;

    gettimeofday(&tv, NULL);
    return (tv.tv_sec - secStart) * 1000 + (tv.tv_usec - usecStart) / 1000;
    #else

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (MMP_U32)(now.tv_sec*1000LL + now.tv_nsec/1000000LL);


    #endif
}

MMP_S64 CMmpUtil::GetTickCountUS() {
    
#if 0    
    static time_t   secStart    = timeStart.tv_.tv_sec;
    static time_t   usecStart   = timeStart.tv_.tv_usec;
                    
    timeval tv;

    gettimeofday(&tv, NULL);
    return (MMP_S64)(tv.tv_sec - secStart) * 1000000 + (MMP_U64)(tv.tv_usec - usecStart);
#else

   struct timespec now;
   clock_gettime(CLOCK_MONOTONIC, &now);
   return (now.tv_sec*1000000LL + now.tv_nsec/1000LL);

#endif    
}

void CMmpUtil::Sleep(int milesec) {
    
     usleep(1000*milesec);
}

#else
#error "ERROR : Set CMmpUtil::GetTickCount"
#endif

#if (MMP_OS==MMP_OS_NUCLEUS )
MMP_RESULT CMmpUtil::CovertAnsiToUnicode( const char* strAnsi, WCHAR* strUnicode )
{
    strcpy( strUnicode, strAnsi );
    return MMP_SUCCESS;
}

MMP_RESULT CMmpUtil::CovertUnicodeToAnsi( const WCHAR* strUnicode, char* strAnsi )
{
    strcpy( strAnsi, strUnicode);
    return MMP_SUCCESS;
}

#elif (MMP_OS==MMP_OS_LINUX )
MMP_RESULT CMmpUtil::CovertAnsiToUnicode( const char* strAnsi, WCHAR* strUnicode )
{
    strcpy( strUnicode, strAnsi );
    return MMP_SUCCESS;
}

MMP_RESULT CMmpUtil::CovertUnicodeToAnsi( const WCHAR* strUnicode, char* strAnsi )
{
    strcpy( strAnsi, strUnicode);
    return MMP_SUCCESS;
}

#else
MMP_RESULT CMmpUtil::CovertAnsiToUnicode( const char* strAnsi, WCHAR* strUnicode )
{
    MultiByteToWideChar( CP_ACP, 0, strAnsi, -1, strUnicode, MAX_PATH );
    return MMP_SUCCESS;
}

MMP_RESULT CMmpUtil::CovertUnicodeToAnsi( const WCHAR* strUnicode, char* strAnsi )
{
    WideCharToMultiByte( CP_ACP, 0, strUnicode, -1,  strAnsi, MAX_PATH, NULL, NULL);
    return MMP_SUCCESS;
}
#endif

// extract DIR form given full filename 
// ex) "c:\mylib\aa.dat" => "c:\mylib\"
MMP_RESULT CMmpUtil::SplitDir(WCHAR* strFileName, WCHAR* wszDir)
{
   WCHAR ch;
   int i,j;
   int strLength;

#if (MMP_OS==MMP_OS_NUCLEUS)
    strLength=strlen(strFileName);
#elif (MMP_OS==MMP_OS_LINUX)
    strLength=strlen(strFileName);
#else
   strLength=wcslen(strFileName);
#endif

   for( i=strLength-1,j=0; i>=0; i--, j++ )
   {
	  ch=strFileName[i];
      if( ch==L'\\' )
	  {
		  break;
	  }
   }

   for( i=0; i<strLength-j; i++ )
   {
	  ch=strFileName[i];
      wszDir[i]=ch;
   }
   wszDir[i]=L'\0';

   return MMP_SUCCESS;
}

// extract only FILENAME form given full filename 
// ex) "c:\mylib\aa.dat" => "aa"
MMP_RESULT CMmpUtil::SplitFileName(MMP_CHAR* strFileName, MMP_CHAR* szName)
{
    MMP_CHAR* strFile;
    MMP_CHAR ch;
    MMP_S32 i, strLength;
    
    strLength=strlen(strFileName);

    for(i = strLength-1; i >= 0; i-- )  {
	    ch = strFileName[i];
        if(ch == MMP_FILE_DEVIDER) {
		    break;
	    }
    }
    strFile = &strFileName[i+1];

    strcpy(szName, strFile);

    strLength = strlen(szName);
    for(i = strLength-1; i >= 0; i-- )  {
	    ch=szName[i];
        if( ch == '.' ) {
            szName[i] = '\0';
            break;
        }
    }
    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpUtil::SplitExt(MMP_CHAR* strFileName, MMP_CHAR* wszExt)
{
   MMP_CHAR* strExt;
   MMP_CHAR ch;
   int i;
   int strLength;

   strLength=strlen(strFileName);
   for( i=strLength-1; i>=0; i-- )
   {
	  ch=strFileName[i];
	  if( ch=='.' )
	  {
		  break;
	  }
   }
   strExt=&strFileName[i+1];
   strcpy(wszExt, strExt);
   
   return MMP_SUCCESS;
}

MMP_RESULT CMmpUtil::MakeLower(MMP_CHAR* wszName)
{
   MMP_CHAR ch;
   int i;
   int strLength;

   strLength=strlen(wszName);
   for( i=strLength-1; i>=0; i-- )
   {
	  ch=wszName[i];
	  if( ch>='A' && ch<='Z')
	  {
		  ch-='A';
          ch+='a';
          wszName[i]=ch;
	  }
   }

   return MMP_SUCCESS;
}

MMP_RESULT CMmpUtil::MakeUpper(MMP_CHAR* wszName)
{
   MMP_CHAR ch;
   int i;
   int strLength;

   strLength=strlen(wszName);
   for( i=strLength-1; i>=0; i-- )
   {
	  ch=wszName[i];
	  if( ch>='a' && ch<='z')
	  {
		  ch-='a';
          ch+='A';
          wszName[i]=ch;
	  }
   }

   return MMP_SUCCESS;
}

// �Էµ� ��ü ���ϸ����� �����丮�� �̾Ƴ���.
// ex) "c:\mylib\aa.dat" => "c:\mylib\"
MMP_RESULT CMmpUtil::SplitDirC(MMP_CHAR* strFileName, MMP_CHAR* wszDir)
{
   MMP_CHAR ch;
   int i,j;
   int strLength;

   strLength=strlen(strFileName);
   for( i=strLength-1,j=0; i>=0; i--, j++ )
   {
	  ch=strFileName[i];
      if( ch=='\\' )
	  {
		  break;
	  }
   }

   for( i=0; i<strLength-j; i++ )
   {
	  ch=strFileName[i];
      wszDir[i]=ch;
   }
   wszDir[i]='\0';

   return MMP_SUCCESS;
}

#if 0
// �Էµ� ��ü ���ϸ����� �����̸��� �̾Ƴ���.
// ex) "c:\mylib\aa.dat" => "aa"
MMP_RESULT CMmpUtil::SplitFileNameC(MMP_CHAR* strFileName, MMP_CHAR* wszName)
{
    MMP_CHAR* strFile;
    MMP_CHAR ch;
    int i;
    int strLength;

    strLength=strlen(strFileName);

    for( i=strLength-1; i>=0; i-- )
    {
	    ch=strFileName[i];
        if( ch=='\\' )
	    {
		  break;
	    }
    }
    strFile=&strFileName[i+1];


    strLength=strlen(strFile);
    for( i=0; i<strLength; i++ )
    {
	    ch=strFile[i];
        if( ch=='.' )
        {
              break;
        }
        wszName[i]=ch;
    }
    wszName[i]='\0';

    return MMP_SUCCESS;
}
#endif

MMP_RESULT CMmpUtil::SplitExtC(MMP_CHAR* strFileName, MMP_CHAR* wszExt)
{
   MMP_CHAR* strExt;
   MMP_CHAR ch='\0';
   int i;
   int strLength;

   strLength=strlen(strFileName);
   for( i=strLength-1; i>=0; i-- )
   {
	  ch=strFileName[i];
	  if( ch=='.' )
	  {
		  break;
	  }
   }
   strExt=&strFileName[i+1];
   strcpy(wszExt, strExt);
   
   return MMP_SUCCESS;
}

MMP_RESULT CMmpUtil::MakeLowerC(MMP_CHAR* wszName)
{
   MMP_CHAR ch;
   int i;
   int strLength;

   strLength=strlen(wszName);
   for( i=strLength-1; i>=0; i-- )
   {
	  ch=wszName[i];
	  if( ch>='A' && ch<='Z')
	  {
		  ch-='A';
          ch+='a';
          wszName[i]=ch;
	  }
   }

   return MMP_SUCCESS;
}

MMP_RESULT CMmpUtil::HexStringToInt(MMP_CHAR* szHex, MMP_S32* value) // Hex String�� ���ڷ� ������. 
{
    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 i, j, szlen;
    MMP_CHAR c;
    MMP_S32 h, v; 

    if(value != NULL) *value = 0;
    v = 0;
    szlen = strlen(szHex);
    for(i = szlen-1, j=0; i>=0; i--, j++) {
        c = szHex[i];
        if( (c >='0') &&  (c<='9') )  {  h=(MMP_S32)(c-'0'); h&=0xff; }
        else if( (c >='A') &&  (c<='Z') )  {  h=(MMP_S32)(c-'A'); h&=0xff; h+=10; }
        else if( (c >='a') &&  (c<='z') )  {  h=(MMP_S32)(c-'a'); h&=0xff; h+=10; }
        else {
            mmpResult = MMP_FAILURE;
            break;
        }

        h <<= (4*j);
        v += h;
    }

    if(mmpResult == MMP_SUCCESS) {
        if(value != NULL) *value = v;
    }

    return mmpResult;
}

#if (MMP_PLATFORM == MMP_PLATFORM_ANDROID)

#include <utils/Log.h>

#if 0 //(MMP_ANDROID_VERSION == 51)

MMP_RESULT CMmpUtil_Printf(char* lpszFormat, ... )
{
    char *szBuffer = new char[2048];  // Large buffer for very long filenames (like with HTTP)
    va_list args ;

	if(szBuffer != NULL) {
		//memset( szBuffer, 0x00, sizeof(TCHAR)*1024);
		va_start(args, lpszFormat) ;
		vsprintf(szBuffer, lpszFormat, args) ;
		va_end(args) ;

		printf("%s\n\r", szBuffer);
	}
    
	delete [] szBuffer;

    return MMP_SUCCESS;
}

#else
//MMP_RESULT CMmpUtil::Printf( TCHAR* msg, ... )
MMP_RESULT CMmpUtil_Printf(char* msg, ... )
{
    
    va_list argptr;

    va_start(argptr, msg);
    __android_log_vprint(ANDROID_LOG_INFO, "MME", msg, argptr);
    va_end(argptr);

    return MMP_SUCCESS;
}
#endif

#elif (MMP_OS == MMP_OS_LINUX)

MMP_RESULT CMmpUtil_Printf(char* lpszFormat, ... )
{
    char *szBuffer = new char[2048];  // Large buffer for very long filenames (like with HTTP)
    va_list args ;

	if(szBuffer != NULL) {
		//memset( szBuffer, 0x00, sizeof(TCHAR)*1024);
		va_start(args, lpszFormat) ;
		vsprintf(szBuffer, lpszFormat, args) ;
		va_end(args) ;

		printf("%s\n\r", szBuffer);
	}
    
	delete [] szBuffer;

    return MMP_SUCCESS;
}

#elif (MMP_OS == MMP_OS_WIN32)

MMP_RESULT CMmpUtil_Printf(char* lpszFormat, ... )
{
    int nBuf ;
    TCHAR szBuffer[2048];  // Large buffer for very long filenames (like with HTTP)
    va_list args ;

    static int cs_init = 0;
    static CRITICAL_SECTION cs;
    if(cs_init == 0) {
        InitializeCriticalSection(&cs);
        cs_init = 1;
    }
    
    EnterCriticalSection(&cs);

    memset( szBuffer, 0x00, sizeof(TCHAR)*1024);

    va_start(args, lpszFormat) ;
	
    nBuf = _vstprintf(szBuffer, lpszFormat, args) ;

#if  (MMP_OS==MMP_OS_WIN32)
	OutputDebugString(szBuffer) ;
    OutputDebugString("\n") ;
#else //if (MMP_OS==MMP_OS_WINCE60)
    RETAILMSG(1, (TEXT("%s"), szBuffer));
#endif

#if 0
    FILE* fp = NULL;
    if(fp == NULL) {
        fp = fopen("d:\\my_mme.log", "a");
    }
    if(fp != NULL) {
        fprintf(fp, "%s\n", szBuffer);
        fclose(fp);
    }
#endif

    va_end(args) ;

    LeaveCriticalSection(&cs);

    return MMP_SUCCESS;
}
#else
#error "Select OS for CMmpUtil::Printf"
#endif


MMP_RESULT CMmpUtil::ColorConvertYUV420PlanarToRGB565(int picWidth, int picHeight, 
                                                unsigned char* Y, unsigned char* U, unsigned char* V,
                                                unsigned short* RGB565,
                                                bool bTopDown
                                              )
{
    int k,ix,iy,r,g,b;
    int y,u,v;
    int lumaIndex, chromaIndex;
    unsigned short r1, g1, b1;

    lumaIndex=0;
    k=0;
    for(iy=0;iy<picHeight;iy++)
    {
        if(!bTopDown)
        {
           k=(picHeight-1-iy)*(picWidth);
        }
        
        for(ix=0;ix<picWidth;ix++, k++ )
        {
             lumaIndex=iy*picWidth+ix;
             chromaIndex=(iy>>1)*(picWidth>>1)+(ix>>1);

             y=(int)Y[lumaIndex];   y&=0xff;
             u=(int)U[chromaIndex]; u&=0xff;  u-=128;
             v=(int)V[chromaIndex]; v&=0xff;  v-=128;
             
             r=y+(91881*u)/65536;
             g=y-((22554*u)/65536)-((46802*v)/65536);
             b=y+(116130*v)/65536;
             if(r<0) r=0; if(r>255) r=255;
             if(g<0) g=0; if(g>255) g=255;
             if(b<0) b=0; if(b>255) b=255;

             r1=(unsigned short)(r>>3);
             g1=(unsigned short)(g>>2);
             b1=(unsigned short)(b>>3);

             RGB565[k]=(b1<<11)|(g1<<5)|r1;
        }
    }

    return MMP_SUCCESS;
}

#if 1//(MMP_OS == MMP_OS_WIN32)
MMP_RESULT CMmpUtil::ColorConvertYUV420PlanarToRGB32(int picWidth, int picHeight, 
                                                     unsigned char* Y, unsigned char* U, unsigned char* V,
                                                     unsigned int* RGB32,
                                                     bool bTopDown
                                                    )
{
    int k,ix,iy,r,g,b;
    int y,u,v;
    int lumaIndex, chromaIndex;
    
    lumaIndex=0;
    k=0;
    for(iy=0;iy<picHeight;iy++)
    {
        if(!bTopDown)
        {
           k=(picHeight-1-iy)*(picWidth);
        }
        
        for(ix=0;ix<picWidth;ix++, k++ )
        {
             lumaIndex=iy*picWidth+ix;
             chromaIndex=(iy>>1)*(picWidth>>1)+(ix>>1);

             y=(int)Y[lumaIndex];   y&=0xff;
             u=(int)U[chromaIndex]; u&=0xff;  u-=128;
             v=(int)V[chromaIndex]; v&=0xff;  v-=128;
         
             r=y+(91881*u)/65536;
             g=y-((22554*u)/65536)-((46802*v)/65536);
             b=y+(116130*v)/65536;
             if(r<0) r=0; if(r>255) r=255;
             if(g<0) g=0; if(g>255) g=255;
             if(b<0) b=0; if(b>255) b=255;

#if (MMP_OS==MMP_OS_NUCLEUS)
             RGB32[k]=r|((g<<16)&0xff00)|((g<<24)&0xff0000);
#elif (MMP_OS==MMP_OS_LINUX)
             //RGB32[k]=0xFF000000|r|((g<<16)&0xff00)|((b<<24)&0xff0000);

             RGB32[k]=0xFF000000;
             RGB32[k] |= b;
             RGB32[k] |= (g<<8)&0xff00;
             RGB32[k] |= (r<<16)&0xff0000;
                          
             //byte 0 : R
             //byte 1 : G
             //byte 2 : B
             //byte 3 : A
#else
             RGB32[k]=RGB(r, g, b);
#endif
        }
    }

    return MMP_SUCCESS;
}
#else

MMP_RESULT CMmpUtil::ColorConvertYUV420PlanarToRGB32(int picWidth, int picHeight, 
                                                     unsigned char* Y, unsigned char* U, unsigned char* V,
                                                     unsigned int* RGB32,
                                                     bool bTopDown
                                                    )
{
    
    return MMP_SUCCESS;
}

#endif

MMP_RESULT CMmpUtil::ColorConvertYUV420PlanarToRGB24(int picWidth, int picHeight, 
                                                     unsigned char* Y, unsigned char* U, unsigned char* V,
                                                     unsigned char* RGB24
                                                    )
{
    int k,ix,iy,r,g,b;
    int y,u,v;
    int lumaIndex, chromaIndex;
    
    lumaIndex=0;
    k=0;
    for(iy=0;iy<picHeight;iy++)
    {
        for(ix=0;ix<picWidth;ix++)
        {
             lumaIndex=iy*picWidth+ix;
             chromaIndex=(iy>>1)*(picWidth>>1)+(ix>>1);

             y=(int)Y[lumaIndex];   y&=0xff;
             u=(int)U[chromaIndex]; u&=0xff;  u-=128;
             v=(int)V[chromaIndex]; v&=0xff;  v-=128;
         
             r=y+(91881*u)/65536;
             g=y-((22554*u)/65536)-((46802*v)/65536);
             b=y+(116130*v)/65536;
             if(r<0) r=0; if(r>255) r=255;
             if(g<0) g=0; if(g>255) g=255;
             if(b<0) b=0; if(b>255) b=255;

             RGB24[k++]=b;
             RGB24[k++]=g;
             RGB24[k++]=r;
        }
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpUtil::ColorConvertRGB32ToYUV420Planar(int picWidth, int picHeight, int stride, 
                                                     unsigned int* RGB32,
                                                     unsigned char* Y, unsigned char* U, unsigned char* V,
                                                     bool bTopDown
                                                    )
{
   int k,ix,iy,r,g,b;
   int y,u,v;
   int luma_index, chroma_index;
   unsigned char* pbuf;

   k=picWidth%4;
   
   //Luma
   pbuf = (unsigned char*)RGB32;
   luma_index=0;
   for(iy=0;iy<picHeight;iy++)
   {
      //k=(picHeight-1-iy)*(picWidth)*4;
      k=iy*stride*4;
      for(ix=0;ix<picWidth;ix++, k+=4 )
      {
         b=pbuf[k];
         g=pbuf[k+1];
         r=pbuf[k+2];
         r&=0xff, g&=0xff, b&=0xff;


         y= ((66*r+129*g+25*b+128)>>8)+16;
         Y[luma_index]=(unsigned char)y;
         
         luma_index++;
      }
   }

   pbuf = (unsigned char*)RGB32;
   chroma_index=0;
   for(iy=0;iy<picHeight;iy+=2)
   {
      k=iy*stride*4;
      for(ix=0;ix<picWidth;ix+=2, k+=8 )
      {
         b=pbuf[k];
         g=pbuf[k+1];
         r=pbuf[k+2];
         r&=0xff, g&=0xff, b&=0xff;

         u= ((-38*r-74*g+112*b+128)>>8)+128;
         v= ((112*r-94*g-18*b+128)>>8)+128;
         
         U[chroma_index] = u;
         V[chroma_index] = v;
         
         chroma_index++;
      }

   }


   return MMP_SUCCESS;
}

MMP_RESULT CMmpUtil::ColorConvertRGB24ToYUV420Planar(int picWidth, int picHeight, int stride_byte, 
                                                     unsigned char* RGB24,
                                                     unsigned char* Y, unsigned char* U, unsigned char* V,
                                                     int luma_stride, int chroma_stride 
                                                     ) {

   int k,ix,iy,r,g,b;
   int y,u,v;
   int luma_index, chroma_index;
   unsigned char* pbuf;

   k=picWidth%4;
   
   //Luma
   pbuf = (unsigned char*)RGB24;
   luma_index=0;
   for(iy=0;iy<picHeight;iy++)
   {
      k=iy*stride_byte;
      luma_index = iy*luma_stride;
      for(ix=0;ix<picWidth;ix++, k+=3 )
      {
         r=pbuf[k];
         g=pbuf[k+1];
         b=pbuf[k+2];
         r&=0xff, g&=0xff, b&=0xff;


         y= ((66*r+129*g+25*b+128)>>8)+16;
         Y[luma_index]=(unsigned char)y;
         
         luma_index++;
      }
   }

   pbuf = (unsigned char*)RGB24;
   chroma_index=0;
   for(iy=0;iy<picHeight;iy+=2)
   {
      k=iy*stride_byte;
      chroma_index = (iy>>1)*chroma_stride;
      for(ix=0;ix<picWidth;ix+=2, k+=6 )
      {
         r=pbuf[k];
         g=pbuf[k+1];
         b=pbuf[k+2];
         r&=0xff, g&=0xff, b&=0xff;

         u= ((-38*r-74*g+112*b+128)>>8)+128;
         v= ((112*r-94*g-18*b+128)>>8)+128;
         
         U[chroma_index] = u;
         V[chroma_index] = v;
         
         chroma_index++;
      }

   }

   return MMP_SUCCESS;
}

MMP_RESULT CMmpUtil::ColorConvertRGB8Pallete32ToYUV420Planar(int picWidth, int picHeight, int stride, 
                                                     unsigned char* rgb, unsigned int* pallete,
                                                     unsigned char* Y, unsigned char* U, unsigned char* V, int luma_stride, int chroma_stride 
                                                     ) {
   int k,ix,iy,r,g,b;
   int y,u,v;
   int luma_index, chroma_index;
   unsigned int pal_index, rgb32col;

   k=picWidth%4;
   
   //Luma
   luma_index=0;
   for(iy=0;iy<picHeight;iy++)
   {
      k=iy*stride;
      luma_index = iy*luma_stride;
      for(ix=0;ix<picWidth;ix++, k++ )
      {
         pal_index = rgb[k];
         pal_index &= 0xFF;

         rgb32col = pallete[pal_index];

         b=rgb32col&0xff;
         g=(rgb32col>>8)&0xff;
         r=(rgb32col>>16)&0xff;

         y= ((66*r+129*g+25*b+128)>>8)+16;
         Y[luma_index]=(unsigned char)y;
         
         luma_index++;
      }
   }

   chroma_index=0;
   for(iy=0;iy<picHeight;iy+=2)
   {
      k=iy*stride;
      chroma_index = (iy>>1)*chroma_stride;
      for(ix=0;ix<picWidth;ix+=2, k+=2 )
      {
         pal_index = rgb[k];
         pal_index &= 0xFF;

         rgb32col = pallete[pal_index];

         b=rgb32col&0xff;
         g=(rgb32col>>8)&0xff;
         r=(rgb32col>>16)&0xff;

         u= ((-38*r-74*g+112*b+128)>>8)+128;
         v= ((112*r-94*g-18*b+128)>>8)+128;
         
         U[chroma_index] = u;
         V[chroma_index] = v;
         
         chroma_index++;
      }

   }

   return MMP_SUCCESS;
}

MMP_RESULT CMmpUtil::ColorConvertYUV420PlanarToYUV420SemiPlanar(int picWidth, int picHeight, 
                                                                 unsigned char* Y, unsigned char* U, unsigned char* V, int luma_stride, int chroma_stride ,
                                                                 unsigned char* Y1, unsigned char* UV1
                                                                 ) {

#ifdef __OMX_PLATFORM_ANDROID
       csc_linear_to_tiled_y_neon(
            (unsigned char *)Y1,
            (unsigned char *)Y, 
            picWidth,
            picHeight);

       csc_linear_to_tiled_uv_neon(
            (unsigned char *)UV1,
            (unsigned char *)U,
            (unsigned char *)V, //outputInfo.CVirAddr,
            picWidth/2,
            picHeight/2);
#endif

       return MMP_SUCCESS;
}


unsigned int  CMmpUtil::MakeUpperFourcc(unsigned int fourcc)
{
    char* pcf;
    char c;
    int i;

    pcf=(char*)&fourcc;

    for(i=0;i<4;i++)
    {
        c=pcf[i];
        if(c>='a'&&c<='z') { c=c-'a'+'A'; }
        pcf[i]=c;
    }

    return fourcc;
}

MMP_BOOL  CMmpUtil::CheckVideoFourcc(unsigned int fourcc, unsigned char* formatName)
{
    unsigned char name[8];
    MMP_BOOL bRet=MMP_TRUE;

    fourcc=CMmpUtil::MakeUpperFourcc(fourcc);

    switch(fourcc)
    {
        case MMPMAKEFOURCC('M','P','G','1'): strcpy((char*)name, "Mpg1"); break;
        case MMPMAKEFOURCC('M','P','G','2'): strcpy((char*)name, "Mpg2"); break;
        
        case MMPMAKEFOURCC('M','P','4','3'): strcpy((char*)name, "Mp43"); break;
        case MMPMAKEFOURCC('M','P','4','V'): strcpy((char*)name, "Mp4v"); break;
        case MMPMAKEFOURCC('X','V','I','D'): strcpy((char*)name, "Xvid"); break;

        case MMPMAKEFOURCC('D','I','V','X'): strcpy((char*)name, "Divx"); break;
        //case MMPMAKEFOURCC('D','I','V','3'): strcpy((char*)name, "Div3"); break;
        case MMPMAKEFOURCC('D','X','5','0'): strcpy((char*)name, "Dx50"); break;
        case MMPMAKEFOURCC('M','P','4','2'): strcpy((char*)name, "Mp42"); break; //Mpeg4 v2
            
        //case MMPMAKEFOURCC('D','I','V','3'):
        //case MMPMAKEFOURCC('d','i','v','3'):
        //    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CDavinci::CreateObject] Divx3 Decoder Select(%c%c%c%c) \n\r"), videoFormat>>0, videoFormat>>8, videoFormat>>16, videoFormat>>24));
        //    iCodecID=CODEC_DECODER_MPEG4;
        //    break;
    
        case MMPMAKEFOURCC('H','2','6','4'):  strcpy((char*)name, "H264"); break;
        case MMPMAKEFOURCC('A','V','C','1'):  strcpy((char*)name, "Avc1"); break;
        case MMPMAKEFOURCC('X','2','6','4'):  strcpy((char*)name, "X264"); break;
      
        case MMPMAKEFOURCC('H','2','6','3'):  strcpy((char*)name, "H263"); break; 
    
        case MMPMAKEFOURCC('R','V','4','0'):  strcpy((char*)name, "Rv40"); break;
        
        case MMPMAKEFOURCC('F','L','V','1'):  strcpy((char*)name, "Flv1"); break;
        
        case MMPMAKEFOURCC('W','M','V','3'):  strcpy((char*)name, "Wmv3"); break;
        
        case MMPMAKEFOURCC('V','P','6','F'):  strcpy((char*)name, "Vp6f"); break;
        
        default:
            bRet=MMP_FALSE;
            strcpy((char*)name, "????"); 
            break;
            
    }

    if(formatName)
    {
        strcpy((char*)formatName, (const char*)name);
    }

    return bRet;
}

MMP_BOOL  CMmpUtil::CheckAudioFormatTag(enum MMP_FOURCC fourcc, unsigned char* formatName)
{
    unsigned char name[8];
    MMP_BOOL bRet=MMP_TRUE;

    switch(fourcc)
    {
        case MMP_FOURCC_AUDIO_PCM       : strcpy((char*)name, " PCM"); break;//             0x0001

        case MMP_FOURCC_AUDIO_MP2       : strcpy((char*)name, " Mp2"); break;//             0x0055 // ISO/MPEG Layer3 Format Tag 
        case MMP_FOURCC_AUDIO_MP3       : strcpy((char*)name, " Mp3"); break;//             0x0055 // ISO/MPEG Layer3 Format Tag 
        case MMP_FOURCC_AUDIO_BSAC      : strcpy((char*)name, "Bsac"); break;//                   0xD000 // Korea TDMB Audio Bsac

        case MMP_FOURCC_AUDIO_AAC       : strcpy((char*)name, " Aac"); break; //                    0x00FF // China TDMB Audio AAXC
        
        case MMP_FOURCC_AUDIO_RA_COOK   : strcpy((char*)name, "Cook"); break;//             0x6F63 //Real Audio  Cook 
        case MMP_FOURCC_AUDIO_RA_RAAC   : strcpy((char*)name, "Raac"); break;//             0x504d //Real Audio  Raac
        case MMP_FOURCC_AUDIO_RA_SIPR   : strcpy((char*)name, "Sipr"); break;//             0x6973 //Real Audio  SIPR

        case MMP_FOURCC_AUDIO_AC3       : strcpy((char*)name, " Ac3"); break;//           0x2000 
        case MMP_FOURCC_AUDIO_DTS       : strcpy((char*)name, " Dts"); break;//           0x2000 
        case MMP_FOURCC_AUDIO_WMA1      : strcpy((char*)name, "Wma1"); break;//          
        case MMP_FOURCC_AUDIO_WMA2      : strcpy((char*)name, "Wma2"); break;//          
    
        default:
            bRet=MMP_FALSE;
            strcpy((char*)name, "????"); 
            return bRet;
    }

    if(formatName)
    {
        strcpy((char*)formatName, (const char*)name);
    }

    return bRet;
}


MMP_BOOL CMmpUtil::IsMediaFileExtValid(WCHAR* wszName)
{
#if 1
    return MMP_FALSE;
#else
    WCHAR wszExt[16];

    MMP_BOOL bRet=MMP_FALSE;

    CMmpUtil::SplitExt(wszName, wszExt);
    CMmpUtil::MakeLower(wszExt);

    if( 
        wcscmp(wszExt, L"avi")==0 ||
        wcscmp(wszExt, L"divx")==0 ||
        
        wcscmp(wszExt, L"mkv")==0 ||   
        
        wcscmp(wszExt, L"mp4")==0 ||
        wcscmp(wszExt, L"mov")==0 ||
        wcscmp(wszExt, L"3gp")==0 ||
        
        wcscmp(wszExt, L"wmv")==0 ||
        wcscmp(wszExt, L"asf")==0 ||
        
        wcscmp(wszExt, L"mpg")==0 ||
        wcscmp(wszExt, L"vob")==0 ||
        wcscmp(wszExt, L"mpeg")==0 ||
        
        wcscmp(wszExt, L"flv")==0 ||
        
        wcscmp(wszExt, L"rm")==0 ||
        wcscmp(wszExt, L"rmvb")==0 ||
        
        wcscmp(wszExt, L"dat")==0 ||
        wcscmp(wszExt, L"ts")==0 ||
        wcscmp(wszExt, L"tp")==0 
     )
    {
         bRet=MMP_TRUE;
    }
/*
    else if(  wcscmp(wszExt, L"xls"))==0 ||
              wcscmp(wszExt, L"pdf"))==0 ||  
              wcscmp(wszExt, L"db"))==0 ||
             wcscmp(wszExt, L"smi"))==0 ||
             wcscmp(wszExt, L"ini"))==0 
        )
    {
        bRet=FALSE;    
    }
    else
    {
        strMsg.Format(TEXT("ERROR: Unknown Ext : %s "), strFile);
        AfxMessageBox(strMsg);
        while(1) Sleep(1000);    
    }
*/
    return bRet;
#endif
}


#define YSRFILEHEADER_KEY 0x9829
typedef struct _YSRFileHeader
{
    int key;
    int width;
    int height;
    int res1;
    int res2;
}YSRFileHeader;

typedef struct _SYSRHandle
{
    FILE* fp;
    YSRFileHeader header;
}SYSRHandle;

void* CMmpUtil::YSR_Create(char* fileName, int picWidth, int picHeight, bool bRead)
{
    SYSRHandle* pYSRHandle=NULL;
    size_t resultByte;

    pYSRHandle=new SYSRHandle;
    if(!pYSRHandle)
    {
        goto action_fail;
    }
    memset(pYSRHandle, 0x00, sizeof(SYSRHandle) );

    if(bRead)
    {
        pYSRHandle->fp=fopen(fileName, "rb");
        if(!pYSRHandle->fp)
        {
            goto action_fail;
        }

        fread(&pYSRHandle->header, 1, sizeof(YSRFileHeader), pYSRHandle->fp);
        if(pYSRHandle->header.key!=YSRFILEHEADER_KEY)
        {
            goto action_fail;
        }

    }
    else //write mode
    {
        pYSRHandle->fp=fopen(fileName, "wb");
        if(!pYSRHandle->fp)
        {
            goto action_fail;
        }

        pYSRHandle->header.key=YSRFILEHEADER_KEY;
        pYSRHandle->header.width=picWidth;
        pYSRHandle->header.height=picHeight;
        
        resultByte=fwrite(&pYSRHandle->header, 1, sizeof(YSRFileHeader), pYSRHandle->fp);
        if(resultByte!=sizeof(YSRFileHeader))
        {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpUtil::YSR_Create] FAIL: Write Header \n\r")));
            goto action_fail;
        }
    }

    return (void*)pYSRHandle;

action_fail:

    if(pYSRHandle)
    {
        if(pYSRHandle->fp)
        {
            fclose(pYSRHandle->fp);
        }
        delete pYSRHandle;
    }

    return (void*)0;
}

void  CMmpUtil::YSR_Write(void* handle, unsigned char* data)
{
    SYSRHandle* pYSRHandle=(SYSRHandle*)handle;
    int w,h,size;

    if(pYSRHandle)
    {
        if(pYSRHandle->fp)
        {
            w=pYSRHandle->header.width;
            h=pYSRHandle->header.height;//((pYSRHandle->header.height+16-1)>>4)<<4;
            size=((w*h)*3)>>1;
            fwrite(data, 1, size, pYSRHandle->fp);
        }
    }
}

void  CMmpUtil::YSR_Destroy(void* handle)
{
    SYSRHandle* pYSRHandle=(SYSRHandle*)handle;

    if(pYSRHandle)
    {
        if(pYSRHandle->fp)
        {
            fclose(pYSRHandle->fp);
        }
        delete pYSRHandle;
    }
}

int CMmpUtil::Time_GetHour(unsigned int timeStamp)
{
    int t;
    t=timeStamp/1000;
    t/=3600;
    return t;
}

int CMmpUtil::Time_GetMin(unsigned int timeStamp)
{
    int t;
    t=timeStamp/1000;
    t%=3600;
    t/=60;
    return t;
}

int CMmpUtil::Time_GetSec(unsigned int timeStamp)
{
    int t;
        
    t=timeStamp/1000;
    t%=3600;
    t%=60;
    return t;
}

int CMmpUtil::Time_GetMileSec(unsigned int timeStamp)
{
    int t;
    t=timeStamp%1000;
    return t;
}

#if (MMP_OS==MMP_OS_WIN32)

static MMP_U8 s_debug_buffer[2048];

void mmp_debug_printf(MMP_U8* lpszFormat, ... )
{
    int nBuf;

    va_list args ;
    va_start(args, lpszFormat) ;
	
    nBuf = vsnprintf((char*)s_debug_buffer, 2048, (const char*)lpszFormat, args) ;

#if 0
    len=strlen((const char*)s_debug_buffer);
    if(s_debug_buffer[len-1] == '\r') 
    {
        s_debug_buffer[len-1]='\0';
    }
#endif
	
    //usx_dbgmsg_que_add_msg(s_debug_buffer, 2048);
    OutputDebugString((char*)s_debug_buffer) ;

    va_end(args) ;
}

#endif

MMP_RESULT CMmpUtil::File_Make(char* filename, MMP_U8* buffer, MMP_U32 buf_size) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    FILE* fp;

    fp = fopen(filename, "wb");
    if(fp != NULL) {
        fwrite(buffer, 1, buf_size, fp);
        fclose(fp);
        mmpResult = MMP_SUCCESS;
    }

    return mmpResult;
}

int CMmpUtil::CompareByAlphanumeric(const char *str1, const char* str2) /* 0 : equal  -1 : str1 front  1 : str1 rear */
{
    int i, len1, len2, minlen;
    char c1, c2;
    int ret;

    len1 = (int)strlen((char*)str1);
    len2 = (int)strlen((char*)str2);

    minlen = len1;
    if(minlen > len2 ) minlen = len2;

    ret = 0;
    for(i = 0; i < minlen; i++)
    {
        c1 = str1[i];
        c2 = str2[i];
        if(c1 < c2 ) ret = -1;
        else if(c1 > c2 ) ret = 1;

        if(ret != 0)
        {
            break;
        }
    }

    if(ret == 0 )
    {
        if(len1 < len2 ) 
        {
            ret = -1;    
        }
        else if(len1 > len2 )
        {
            ret = 1;
        }
    }

    return ret;
}

#if (MMP_OS == MMP_OS_WIN32) 

int CMmpUtil::GetFileList(char* path, char* ext, int extcnt, int ext_size_max, char* file_array, int file_array_max, int file_size_max)
{

    HANDLE hList;
    WIN32_FIND_DATA FileData;
    int i, file_cnt;

#if 0
    MMP_U8 szDir[MAX_PATH+1];
    MMP_U8 szname[256], szext[32];
#else

    MMP_U8* szDir=NULL;
    MMP_U8* szname =NULL, *szext = NULL;
#endif

    //MMP_U8 *str1, *str2, *str3;

    szDir = (MMP_U8*)malloc(MAX_PATH+1);
    szname = (MMP_U8*)malloc(256);
    szext = (MMP_U8*)malloc(256);
   
    file_cnt = 0;

    strcpy((char*)szDir, path);
    strcat((char*)szDir, "*.*");
    hList = FindFirstFile((LPCSTR)szDir, &FileData);
    if(hList == INVALID_HANDLE_VALUE)
    {
        free(szDir);
        free(szname);
        free(szext);

        return file_cnt;
    }

    do
    {
        if(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if(strcmp(FileData.cFileName, ".") != 0 
              && strcmp(FileData.cFileName, "..") != 0
              && strcmp(FileData.cFileName, "$RECYCLE.BIN") != 0
                )
            {
                strcpy((char*)szDir, path);
                strcat((char*)szDir, FileData.cFileName);
                strcat((char*)szDir, "\\");
                file_cnt += CMmpUtil::GetFileList((char*)szDir, ext, extcnt, ext_size_max, &file_array[file_cnt*file_size_max], file_array_max-file_cnt, file_size_max);
            }
        }
        else
        {
            strcpy((char*)szname, FileData.cFileName);
            for(i=0; i<extcnt; i++)
            {
                szext[0]=0;
                //usx_util_split_ext(szname, szext);

                szext[0]=0;
                CMmpUtil::SplitExtC((MMP_CHAR*)szname, (MMP_CHAR*)szext);
                //USXDEBUGMSG(1, (TEXT("filename : %s \n\r"), szname));
                CMmpUtil::MakeLower((MMP_CHAR*)szext);

                if(strcmp((const char*)szext, &ext[i*ext_size_max]) == 0)
                {
                    sprintf(&file_array[file_cnt*file_size_max], "%s%s", path, szname);
                    file_cnt++;
                    if(file_cnt>=file_array_max)
                    {
                        break;
                    }
                }
            
            }
        }

        if(file_cnt>=file_array_max)
        {
            break;
        }
    
    }while(FindNextFile(hList, &FileData) == TRUE);


    FindClose(hList);

    free(szDir);
    free(szname);
    free(szext);
    
#if 0
    for(i = 0; i < file_cnt; i++)
    {
        str1 = &file_array[i*file_size_max];
        for( j = i; j < file_cnt ;j++)
        {
            str2 = &file_array[j*file_size_max];
            comp_ret = usx_util_string_compare_by_alphanumeric(str1, str2);
            if(comp_ret > 0)
            {
                str3 = &file_array[file_cnt*file_size_max];
                strcpy(str3, str1);
                strcpy(str1, str2);
                strcpy(str2, str3);
            }
        }
    }
#endif
    
    return file_cnt;
}

#elif (MMP_OS == MMP_OS_LINUX)

int CMmpUtil::GetFileList(char* path, char* ext, int extcnt, int ext_size_max, char* file_array, int file_array_max, int file_size_max)
{
    DIR* dir;
    struct dirent* p_file;
    char szext[32];
    char szpath[256];
    int i, j, file_cnt, comp_ret;
    struct stat st;
    char*str1, *str2, *str3;

    file_cnt = 0;

    //printf("TRY: oepndir (%s) \n", path);
    dir = opendir(path);
    if(dir==NULL)
    {
        printf("FAIL: oepndir (%s) \n", path);
        return file_cnt;
    }
    //printf("SUCCESS: oepndir (%s) \n", path);

    do
    {
        p_file = readdir(dir);
        if(p_file)
        {
            sprintf(szpath, "%s%s", path, p_file->d_name);
            stat(szpath, &st);

            if(st.st_mode & S_IFDIR)
            {
#if 1
                if( strcmp(p_file->d_name, ".") != 0 && strcmp(p_file->d_name, "..") != 0 )
                {
                    sprintf(szpath, "%s%s/", path, p_file->d_name);
                    //USXDEBUGMSG(1, (TEXT("DIR: d_ino(%d) d_name(%s)  %s \n\r"), p_file->d_ino, p_file->d_name, szpath));
                    file_cnt += CMmpUtil::GetFileList(szpath, ext, extcnt, ext_size_max, &file_array[file_cnt*file_size_max], file_array_max-file_cnt, file_size_max);
                }
#endif
            }
            else
            {
                //USXDEBUGMSG(1, (TEXT("FILE: d_ino(%d) path(%s) d_name(%s) \n\r"), p_file->d_ino, path, p_file->d_name ));
                //printf("extcnt:%d  path(%s) szpath(%s) d_name(%s) \n", 
                //               extcnt, 
                //               path, szpath, 
                //               p_file->d_name);
                for(i=0; i<extcnt; i++)
                {
                    szext[0]=0;
                    CMmpUtil::SplitExtC(p_file->d_name, szext);
                    CMmpUtil::MakeLower((MMP_CHAR*)szext);
                    if(strcmp(szext, &ext[i*ext_size_max]) == 0)
                    {
                        sprintf(&file_array[file_cnt*file_size_max], "%s%s", path, p_file->d_name);
                        file_cnt++;
                        if(file_cnt>=file_array_max)
                        {
                            break;
                        }
                    }
                }
                //USXDEBUGMSG(1, (TEXT("FILE1: d_ino(%d) path(%s) d_name(%s) \n\r"), p_file->d_ino, path, p_file->d_name ));
            }
        }

        if(file_cnt>=file_array_max)
        {
            break;
        }
        
    }while(p_file);

    closedir(dir);
    
    for(i = 0; i < file_cnt; i++)
    {
        str1 = &file_array[i*file_size_max];
        for( j = i; j < file_cnt ;j++)
        {
            str2 = &file_array[j*file_size_max];
            comp_ret = CMmpUtil::CompareByAlphanumeric(str1, str2);
            if(comp_ret > 0)
            {
                str3 = &file_array[file_cnt*file_size_max];
                strcpy(str3, str1);
                strcpy(str1, str2);
                strcpy(str2, str3);
            }
        }
    }
    

    return file_cnt;
}

#endif

#if (MMP_OS == MMP_OS_WIN32) 
MMP_RESULT CMmpUtil::system_meminfo(struct mmp_system_meminfo* p_meminfo) {

#if 0
    typedef struct _MEMORYSTATUS {   
        DWORD dwLength;   
        DWORD dwMemoryLoad; 
        DWORD dwTotalPhys;   
        DWORD dwAvailPhys;   
        DWORD dwTotalPageFile; 
        DWORD dwAvailPageFile;   
        DWORD dwTotalVirtual;   
        DWORD dwAvailVirtual; 
} MEMORYSTATUS, *LPMEMORYSTATUS; 
#endif
    MEMORYSTATUS meminfo_win32;

    GlobalMemoryStatus(&meminfo_win32); 

    p_meminfo->MemTotal = meminfo_win32.dwTotalPhys/1024;
    p_meminfo->MemFree = meminfo_win32.dwAvailPhys/1024;

    return MMP_FAILURE;
}
#elif (MMP_OS == MMP_OS_LINUX) 

extern "C" {

    /* obsolete */
unsigned long kb_main_shared;
/* old but still kicking -- the important stuff */
unsigned long kb_main_buffers;
unsigned long kb_main_cached;
unsigned long kb_main_free;
unsigned long kb_main_total;
unsigned long kb_swap_free;
unsigned long kb_swap_total;
/* recently introduced */
unsigned long kb_high_free;
unsigned long kb_high_total;
unsigned long kb_low_free;
unsigned long kb_low_total;
/* 2.4.xx era */
unsigned long kb_active;
unsigned long kb_inact_laundry;
unsigned long kb_inact_dirty;
unsigned long kb_inact_clean;
unsigned long kb_inact_target;
unsigned long kb_swap_cached;  /* late 2.4 and 2.6+ only */
/* derived values */
unsigned long kb_swap_used;
unsigned long kb_main_used;
/* 2.5.41+ */
unsigned long kb_writeback;
unsigned long kb_slab;
unsigned long nr_reversemaps;
unsigned long kb_committed_as;
unsigned long kb_dirty;
unsigned long kb_inactive;
unsigned long kb_mapped;
unsigned long kb_pagetables;

    void mmp_system_meminfo(struct mmp_system_meminfo* p_meminfo);
}

#if 0
{"Active",       &kb_active},       // important
  {"AnonPages",    &kb_anon_pages},
  {"Bounce",       &kb_bounce},
  {"Buffers",      &kb_main_buffers}, // important
  {"Cached",       &kb_main_cached},  // important
  {"CommitLimit",  &kb_commit_limit},
  {"Committed_AS", &kb_committed_as},
  {"Dirty",        &kb_dirty},        // kB version of vmstat nr_dirty
  {"HighFree",     &kb_high_free},
  {"HighTotal",    &kb_high_total},
  {"Inact_clean",  &kb_inact_clean},
  {"Inact_dirty",  &kb_inact_dirty},
  {"Inact_laundry",&kb_inact_laundry},
  {"Inact_target", &kb_inact_target},
  {"Inactive",     &kb_inactive},     // important
  {"LowFree",      &kb_low_free},
  {"LowTotal",     &kb_low_total},
  {"Mapped",       &kb_mapped},       // kB version of vmstat nr_mapped
  {"MemFree",      &kb_main_free},    // important
  {"MemShared",    &kb_main_shared},  // important, but now gone!
  {"MemTotal",     &kb_main_total},   // important
  {"NFS_Unstable", &kb_nfs_unstable},
  {"PageTables",   &kb_pagetables},   // kB version of vmstat nr_page_table_pages
  {"ReverseMaps",  &nr_reversemaps},  // same as vmstat nr_page_table_pages
  {"SReclaimable", &kb_swap_reclaimable}, // "swap reclaimable" (dentry and inode structures)
  {"SUnreclaim",   &kb_swap_unreclaimable},
  {"Slab",         &kb_slab},         // kB version of vmstat nr_slab
  {"SwapCached",   &kb_swap_cached},
  {"SwapFree",     &kb_swap_free},    // important
  {"SwapTotal",    &kb_swap_total},   // important
  {"VmallocChunk", &kb_vmalloc_chunk},
  {"VmallocTotal", &kb_vmalloc_total},
  {"VmallocUsed",  &kb_vmalloc_used},
  {"Writeback",    &kb_writeback},    // kB version of vmstat nr_writeback
#endif

MMP_RESULT CMmpUtil::system_meminfo(struct mmp_system_meminfo* p_meminfo) {

    
    mmp_system_meminfo(p_meminfo);

    return MMP_SUCCESS;
}

#endif


#if 0
http://www.dreamy.pe.kr/zbxe/CodeClip/163339

CPU사용률은 /proc/stat를 참고하면 된다. [root@coco /root]# cat /proc/stat

cpu번호      user모드      nice user모드            system모드           idle 상태
-------------------------------------------------------------------------------
 cpu           1714278               9666                      631901             135528477
 cpu0           842765               5302                      372331               67721763
 cpu1           871513               4364                      259570               67806714

...

CPU사용률로써 부팅후 지금까지 소모된 jiffies의 크기를 나타낸다. 
5개의 필드로 구성되어 있으며, 
첫번째 필드는 CPU번호, 두번째 필드는 user 모드, 세번째 필드는 low priority(nice상태)의 user모드를 나타낸다. 
네번째 필드는 system 모드 마지막은 idle 테스크의 jiffies 소모 값을 나타낸다. 

우리가 필요로 하는 값은 두번째, 네번째, 다섯번째 필드의 값으로 이들 값을 이용해서 CPU의 사용율을 얻어올 수 있다.

/proc/stat의 출력내용을 보면 cpu와 cpu0과 같이 뒤에 숫자가 붙은 것과 그렇지 않은 것이 있는데, 
숫자가 붙지 않은 것은 (cpu가 여러 개 있을 때)전체 cpu에 대한 jiffies총합의 계산을 보여준다. 숫자가 붙은것들은 개별 cpu에 대한 jiffies값을 보여준다. 
위의 리눅스시스템의 경우 단지 하나의 cpu로 이루어져 있음을 짐작할 수 있을 것이다.

만약 CPU의 IDLE상태만을 알고 싶다면 다섯번째 필드값만을 조사하면 될것이다.
이렇게 해서 CPU의 jiffies를 가지고 오긴 했지만 이것만으로는 우리가 원하는 정보를 얻어올순 없다. 
실제 우리가 원하는 정보는 전체 CPU사용 jiffies중 idle jiffies를 얻어와야 하고 이것을 다시 백분율로 표시해야 하므로 약간의 가공을 해주어야 한다.
방법은 간단하다. 일정시간 소비된 idel jiffies를 총 소비된 jiffies로 나눠 주면 된다. (idle jiffies)*100 / (idle jiffies + use jiffies + system jiffies + low prio jiffies)
위의 방법을 통하면 cpu의 idle의 백분율 값을 얻어올 수 있다. 일정 시간은 초단위로 하면 되고, 소비된 jiffies를 구해야 함으로 이전의 jiffies값을 가지고 있어야 된다.

cpu 전체값 = (user모드+nice user 모드+system 모드+idle 상태)
user 모드 사용율 = (user 모드)*100 / cpu 전체값
user 모드 사용율 = (user모드 / cpu 전체값) * 100
system 모드 사용율 = (system 모드 / cpu 전체값) * 100

#endif

MMP_RESULT CMmpUtil::system_cpu_usage(struct mmp_system_cpu_usage* p_cpu_usage) {

    char* cpu_string;
    char *buffer;
    int bufsize=4096, readsz;
    char name[32];
    int i;
    int cpu_id;
    unsigned int jiffies_user, jiffies_nice_user, jiffies_system, jiffies_idle;
    unsigned int jiffies_total, jiffies_sum, jiffies_sum1;

    memset(p_cpu_usage, 0x00, sizeof(struct mmp_system_cpu_usage) );
    
    buffer = new char[bufsize];
    MMP_ASSERT(buffer);

#ifdef WIN32
    strcpy(buffer, "cpu  214870 2743 87520 1782728 3082 0 117 0 0 0\
cpu0 36854 518 15468 469083 754 0 96 0 0 0\
cpu1 63861 875 25179 431831 994 0 21 0 0 0\
cpu2 57385 700 23781 440252 644 0 0 0 0 0\
cpu3 56769 649 23091 441561 690 0 0 0 0 0\
intr 6041476 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 4182124 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 25776 638294 0 9207 742 0 0 0 0 0 0 10315 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 11552 0 196 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 26012 1 23288 1 23283 1 23271 1 23259 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\
ctxt 3324389\
btime 1445231895\
processes 5728\
procs_running 2\
procs_blocked 0\
softirq 8000330 0 4182122 0 41074 0 0 511759 2227924 0 1037451" );
    readsz = strlen(buffer);
#else
    int fd;

    fd = open("/proc/stat", O_RDONLY);
    MMP_ASSERT(fd >= 0);
    readsz = read(fd, buffer, bufsize);
    close(fd);
#endif
    
    buffer[readsz] = '\0';

    /* check cpu count */
    for(i = 0; i < 4; i++) {
        sprintf(name, "cpu%d", i);
        cpu_string = strstr(buffer, name);  
        if(cpu_string!=NULL) p_cpu_usage->cpu_count++;
        else break;
    }

    /* read cpu jiffies value */
    for(i = 0; i < p_cpu_usage->cpu_count; i++) {
        sprintf(name, "cpu%d", i);
        cpu_string = strstr(buffer, name);  
        sscanf(cpu_string, "cpu%d %lu %lu %lu %lu ", &cpu_id, &jiffies_user, &jiffies_nice_user, &jiffies_system, &jiffies_idle);
        
        p_cpu_usage->jiffies_user[cpu_id] = jiffies_user;
        p_cpu_usage->jiffies_nice_user[cpu_id] = jiffies_nice_user;
        p_cpu_usage->jiffies_system[cpu_id] = jiffies_system;
        p_cpu_usage->jiffies_idle[cpu_id] = jiffies_idle;
    }

    /* sum totol cpu jifies */
    jiffies_total = 0;
    for(i = 0; i < p_cpu_usage->cpu_count; i++) {
        jiffies_total += p_cpu_usage->jiffies_user[i];
        jiffies_total += p_cpu_usage->jiffies_nice_user[i];
        jiffies_total += p_cpu_usage->jiffies_system[i];
        jiffies_total += p_cpu_usage->jiffies_idle[i];
    }

    /* cal percent */
    for(i = 0; i < p_cpu_usage->cpu_count; i++) {
        
        jiffies_sum = 0;
        jiffies_sum1 = 0;

        jiffies_sum += p_cpu_usage->jiffies_user[i];
        jiffies_sum += p_cpu_usage->jiffies_nice_user[i];
        jiffies_sum += p_cpu_usage->jiffies_system[i];
        
        jiffies_sum1 = jiffies_sum +p_cpu_usage->jiffies_idle[i];

        p_cpu_usage->percent[i] = jiffies_sum*100/jiffies_sum1;
    }

#ifndef WIN32
    close(fd);
#endif

    delete [] buffer;
    
    return MMP_SUCCESS;
}


#if (MMP_OS == MMP_OS_LINUX) 

MMP_U32 CMmpUtil::GetSystemJiffies(void) {
    
    int fd;
    MMP_U32 jiffies = 0;
    struct driver_mme_io mmeio;
    int iret;

    fd = open(MME_DRIVER_NODE, O_RDWR|O_NONBLOCK);
    if(fd >= 0) {
        iret = ioctl(fd, MME_IOC_SYSTEM_GET_JIFFIES, &mmeio);
        if(iret == 0) {
            jiffies = mmeio.parm[0];
        }
        close(fd);
    }
    
    return jiffies;
}

#elif (MMP_OS == MMP_OS_WIN32) 

MMP_U32 CMmpUtil::GetSystemJiffies(void) {
    return GetTickCount();
}


#else
#error "Select OS for CMmpUtil::GetSystemJiffies"
#endif


MMP_BOOL CMmpUtil::IsVideoDecFrameValid(MMP_S32 width, MMP_S32 height) {

    MMP_BOOL bFlag = MMP_TRUE;
    MMP_S32 virtual_width, virtual_height;

    
    /*
        ���ΰ� �� �� ���찡 �����Ƿ�, �̿� ���� ������ �ʿ��ϴ�. 
    */
    if(width < height) {
        virtual_width = height;
        virtual_height = width;
    }
    else {
        virtual_width = width;
        virtual_height = height;
    }
        
    /* check pic width */
    if( (virtual_width >= MMP_VIDEO_DEC_MIN_WIDTH) && (virtual_width <= MMP_VIDEO_MAX_WIDTH) ) {
        /* Nothing to do */
    }
    else {
        bFlag = MMP_FALSE;
    }
    
    
    /* check pic height */
    if( (virtual_height >= MMP_VIDEO_DEC_MIN_HEIGHT) && (virtual_height <= MMP_VIDEO_MAX_HEIGHT) ) {
        /* Nothing to do */
    }
    else {
        bFlag = MMP_FALSE;
    }
        
    return bFlag;
}

MMP_BOOL CMmpUtil::IsVideoEncFrameValid(MMP_S32 width, MMP_S32 height) {

    MMP_BOOL bFlag = MMP_TRUE;
    MMP_S32 virtual_width, virtual_height;

    
    /*
        ���ΰ� �� �� ���찡 �����Ƿ�, �̿� ���� ������ �ʿ��ϴ�. 
    */
    if(width < height) {
        virtual_width = height;
        virtual_height = width;
    }
    else {
        virtual_width = width;
        virtual_height = height;
    }
        
    /* check pic width */
    if( (virtual_width >= MMP_VIDEO_ENC_MIN_WIDTH) && (virtual_width <= MMP_VIDEO_MAX_WIDTH) ) {
        /* Nothing to do */
    }
    else {
        bFlag = MMP_FALSE;
    }
    
    
    /* check pic height */
    if( (virtual_height >= MMP_VIDEO_ENC_MIN_HEIGHT) && (virtual_height <= MMP_VIDEO_MAX_HEIGHT) ) {
        /* Nothing to do */
    }
    else {
        bFlag = MMP_FALSE;
    }
        
    return bFlag;
}

MMP_BOOL CMmpUtil::IsDebugEnable(const char* name) {
    
    char value[PROPERTY_VALUE_MAX];
    MMP_BOOL bflag;

    value[0] ='A';
    property_get(name, value, NULL);
    if(value[0] == '1') {
        bflag = MMP_TRUE;
    }
    else {
        bflag = MMP_FALSE;
    }

    return bflag;
}

#define ANDROID_PROPNAME_VPUDEC "mme.debug.vpudec"
#define ANDROID_PROPNAME_VPUENC "mme.debug.vpuenc"
#define ANDROID_PROPNAME_MMEDEC "mme.debug.mmedec"
#define ANDROID_PROPNAME_MMEENC "mme.debug.mmeenc"
#define ANDROID_PROPNAME_OMX_BASE "mme.debug.omx.base"
#define ANDROID_PROPNAME_OMX_VIDEO_DEC "mme.debug.omx.vdec"
#define ANDROID_PROPNAME_OMX_VIDEO_ENC "mme.debug.omx.venc"
#define ANDROID_PROPNAME_OMX_AUDIO_DEC "mme.debug.omx.adec"
#define ANDROID_PROPNAME_OMX_AUDIO_ENC "mme.debug.omx.aenc"

/*
setprop mme.debug.vpudec 1
setprop mme.debug.vpuenc 1
setprop mme.debug.mmedec 1
setprop mme.debug.mmeenc 1
setprop mme.debug.omx.base 1
setprop mme.debug.omx.vdec 1
setprop mme.debug.omx.venc 1
setprop mme.debug.omx.adec 1
setprop mme.debug.omx.aenc 1
*/

MMP_BOOL CMmpUtil::IsDebugEnable_VPUDec(void) {
    return CMmpUtil::IsDebugEnable(ANDROID_PROPNAME_VPUDEC);
}

MMP_BOOL CMmpUtil::IsDebugEnable_VPUEnc(void) {
    return CMmpUtil::IsDebugEnable(ANDROID_PROPNAME_VPUENC);
}

MMP_BOOL CMmpUtil::IsDebugEnable_MMEDec(void) {
    return CMmpUtil::IsDebugEnable(ANDROID_PROPNAME_MMEDEC);
}

MMP_BOOL CMmpUtil::IsDebugEnable_MMEEnc(void) {
    return CMmpUtil::IsDebugEnable(ANDROID_PROPNAME_MMEENC);
}

MMP_BOOL CMmpUtil::IsDebugEnable_OMXBase(void) {
    return CMmpUtil::IsDebugEnable(ANDROID_PROPNAME_OMX_BASE);
}

MMP_BOOL CMmpUtil::IsDebugEnable_OMXVideoDec(void) {
    return CMmpUtil::IsDebugEnable(ANDROID_PROPNAME_OMX_VIDEO_DEC);
}

MMP_BOOL CMmpUtil::IsDebugEnable_OMXVideoEnc(void) {
    return CMmpUtil::IsDebugEnable(ANDROID_PROPNAME_OMX_VIDEO_ENC);
}

MMP_BOOL CMmpUtil::IsDebugEnable_OMXAudioDec(void) {
    return CMmpUtil::IsDebugEnable(ANDROID_PROPNAME_OMX_AUDIO_DEC);
}

MMP_BOOL CMmpUtil::IsDebugEnable_OMXAudioEnc(void) {
    return CMmpUtil::IsDebugEnable(ANDROID_PROPNAME_OMX_AUDIO_ENC);
}


/**********************************************************
CTS 
**********************************************************/

static MMP_BOOL isColorClose(int actual, int expected) {
    const int MAX_DELTA = 8;

    int delta = actual - expected;
    if(delta<0) delta*=-1;
    //if (delta > mLargestColorDelta) {
     //   mLargestColorDelta = delta;
   // }
    return (delta <= MAX_DELTA)?MMP_TRUE:MMP_FALSE;
}

MMP_BOOL CMmpUtil::CTS_CheckSurfaceFrame_RGB32(MMP_S32 frameIndex, 
                                               MMP_S32 mWidth, MMP_S32 mHeight, MMP_PTR data,
                                                MMP_S32 TEST_R0, MMP_S32 TEST_G0, MMP_S32 TEST_B0,
                                                MMP_S32 TEST_R1, MMP_S32 TEST_G1, MMP_S32 TEST_B1,
                                                MMP_BOOL TopDown
                                                ) {
   MMP_U32* rgbdata = (MMP_U32*)data;
   MMP_BOOL frameFailed = MMP_FALSE;
   
   for (int i = 0; i < 8; i++) {

       // Note the coordinates are inverted on the Y-axis in GL.
        int x, y;
        /* Top=>Bottom */
        if(TopDown) {
            if (i < 4) {
                x = i * (mWidth / 4) + (mWidth / 8);
                y = mHeight / 4;
            } else {
                x = (7 - i) * (mWidth / 4) + (mWidth / 8);
                y = (mHeight * 3) / 4;
            }
        }
        else {
            if (i < 4) {
                x = i * (mWidth / 4) + (mWidth / 8);
                y = (mHeight * 3) / 4;
            } else {
                x = (7 - i) * (mWidth / 4) + (mWidth / 8);
                y = mHeight / 4;
            }
        }

        //GLES20.glReadPixels(x, y, 1, 1, GL10.GL_RGBA, GL10.GL_UNSIGNED_BYTE, pixelBuf);
        MMP_U8* ptr;
        ptr = (MMP_U8*)&rgbdata[y*mWidth + x];
        MMP_S32 r = (int)ptr[0] & 0xff;
        MMP_S32 g = (int)ptr[1] & 0xff;
        MMP_S32 b = (int)ptr[2] & 0xff;
        //Log.d(TAG, "GOT(" + frameIndex + "/" + i + "): r=" + r + " g=" + g + " b=" + b);

        MMPDEBUGMSG(0, (TEXT("[CTS_CheckSurfaceFrame_RGB32] TopDn(%d) frnIdx(%d %d) RGB=(%d %d %d) TestRGB0(%d %d %d) TestRGB1(%d %d %d) "),
                    TopDown,
                    frameIndex, frameIndex % 8 ,
                    r, g, b,
                    TEST_R0, TEST_G0, TEST_B0,
                    TEST_R1, TEST_G1, TEST_B1 ));


        int expR, expG, expB;//, expR_bt709, expG_bt709, expB_bt709;
        if (i == (frameIndex % 8) ) {
            // colored rect
            expR = TEST_R1;
            expG = TEST_G1;
            expB = TEST_B1;
            //expR_bt709 = TEST_R1_BT709;
            //expG_bt709 = TEST_G1_BT709;
            //expB_bt709 = TEST_B1_BT709;
        } else {
            // zero background color
            expR = TEST_R0;
            expG = TEST_G0;
            expB = TEST_B0;
            //expR_bt709 = TEST_R0_BT709;
            //expG_bt709 = TEST_G0_BT709;
            //expB_bt709 = TEST_B0_BT709;
        }

        if(isColorClose(r, expR) &&
            isColorClose(g, expG) &&
            isColorClose(b, expB) ) {
        
        }
        else {
            frameFailed = MMP_TRUE;
        }

   }
    
    return frameFailed?MMP_FALSE:MMP_TRUE;
}
