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


#include "mmp_util.h"
#include <fcntl.h>
#include <unistd.h>
#include "MmpUtil.hpp"

struct eynos4412_reg_apll_st {
    unsigned int SDIV:3;
    unsigned int res1:5;
    unsigned int PDIV:6;
    unsigned int res2:2;
    unsigned int MDIV:10;
    unsigned int ress:1;
    unsigned int FSEL:1;
    unsigned int res3:1;
    unsigned int LOCKED:1;
    unsigned int res4:1;
    unsigned int ENABLE:1;
    
};

union eynos4412_reg_apll{
  unsigned int v;
  struct eynos4412_reg_apll_st s;  
};


struct eynos4412_reg_div_dmc0_st {
    unsigned int ACP_RATIO:3;
    unsigned int res1:1;
    unsigned int ACP_PCLK_RATION:3;
    unsigned int res2:1;
    unsigned int DPHY_RATIO:3;
    unsigned int res3:1;
    unsigned int DMC_RATIO:3;
    unsigned int res4:1;
    unsigned int DMCD_RATIO:3;
    unsigned int res5:1;
    unsigned int DMCP_RATIO:3;
    unsigned int res6:1;
    unsigned int res7:8;
};

union eynos4412_reg_div_dmc0{
  unsigned int v;
  struct eynos4412_reg_div_dmc0_st s;  
};

#define PLL35XX_MDIV_MASK       (0x3FF)
#define PLL35XX_PDIV_MASK       (0x3F)
#define PLL35XX_SDIV_MASK       (0x7)
#define PLL35XX_MDIV_SHIFT      (16)
#define PLL35XX_PDIV_SHIFT      (8)
#define PLL35XX_SDIV_SHIFT      (0)

MMP_U32  exynos4412_clock_apll_cal(MMP_U32 APLL_CON0) {

    union eynos4412_reg_apll reg;
    MMP_U32  mdiv, pdiv, sdiv, pll_con;
    MMP_U64 fvco = 24000000;

    reg.v = APLL_CON0;

    pll_con = APLL_CON0;
    mdiv = (pll_con >> PLL35XX_MDIV_SHIFT) & PLL35XX_MDIV_MASK;
	pdiv = (pll_con >> PLL35XX_PDIV_SHIFT) & PLL35XX_PDIV_MASK;
	sdiv = (pll_con >> PLL35XX_SDIV_SHIFT) & PLL35XX_SDIV_MASK;

    mdiv = reg.s.MDIV;
    pdiv = reg.s.PDIV;
    sdiv = reg.s.SDIV;

    fvco *= mdiv;
#if 0
	do_div(fvco, (pdiv << sdiv)); //linux kernel function
#else
    fvco /= (pdiv << sdiv);
#endif

    MMPDEBUGMSG(1, (TEXT(" v=0x%08x  s.SDIV=0x%08x  fvco=%d \n"), reg.v, reg.s.SDIV, (unsigned int)fvco ));

    reg.s.MDIV = 175;
    reg.s.PDIV = 3;
    reg.s.SDIV = 0;
    MMPDEBUGMSG(1, (TEXT("1.4GHz reg=0x%08x \n"),  reg.v));

    reg.s.MDIV = 200;
    reg.s.PDIV = 4;
    reg.s.SDIV = 0;
    MMPDEBUGMSG(1, (TEXT("1.2GHz reg=0x%08x \n"),  reg.v));

    reg.s.MDIV = 275;
    reg.s.PDIV = 6;
    reg.s.SDIV = 0;
    MMPDEBUGMSG(1, (TEXT("1.1GHz reg=0x%08x \n"),  reg.v));

    union eynos4412_reg_div_dmc0 reg_div_dmc0;
    reg_div_dmc0.v = 0x00111113;
    MMPDEBUGMSG(1, (TEXT("reg_div_dmc0.s.DMC_RATIO = %d \n"),  reg_div_dmc0.s.DMC_RATIO ));   //Memory
    MMPDEBUGMSG(1, (TEXT("reg_div_dmc0.s.DMCD_RATIO = %d \n"),  reg_div_dmc0.s.DMCD_RATIO ));  //Data Bus
    MMPDEBUGMSG(1, (TEXT("reg_div_dmc0.s.DMCP_RATIO = %d \n"),  reg_div_dmc0.s.DMCP_RATIO ));  //Peri Bus

    reg_div_dmc0.s.DMC_RATIO = 2;
    MMPDEBUGMSG(1, (TEXT("reg_div_dmc0.v = 0x%08x \n"),  reg_div_dmc0.v ));   //Memory

    return 0;
}


void kernel_io_system_print_clock() {

    int fd;
    struct driver_mme_io mmeio;
    
    fd = MMP_DRIVER_OPEN(MME_DRIVER_NODE, O_RDWR|O_NONBLOCK);
    if(fd >= 0) {
        MMP_DRIVER_IOCTL(fd, MME_IOC_SYSTEM_PRINT_CLOCK, &mmeio);
        MMP_DRIVER_CLOSE(fd);
    }
    
}

/**************************************************************************************
  PCM Dump Tool
**************************************************************************************/

class pcm_dump {

private:
    enum {
        KEY_VALUE = 0x98299829
    };
protected:
    MMP_U32 m_key;
    FILE* m_fp;
    MMP_S32 m_sr;
    MMP_S32 m_ch;
    MMP_S32 m_bps;
    MMP_BOOL m_is_write;
    MMP_S32 m_filesize;

    pcm_dump(MMP_CHAR* filename, MMP_S32 sr, MMP_S32 ch, MMP_S32 bps, MMP_BOOL is_write) :
      m_key(KEY_VALUE)
      ,m_fp(NULL) 
      ,m_sr(sr) 
      ,m_ch(ch) 
      ,m_bps(bps)
      ,m_is_write(is_write) {
    
          if(is_write) {
              m_fp = fopen(filename, "wb");
              m_filesize = 1;
          }
          else {
              m_fp = fopen(filename, "rb");
              if(m_fp!=NULL) {
                  fseek(m_fp, -4*(int)sizeof(MMP_S32), SEEK_END);
                  m_filesize = (MMP_S32)ftell(m_fp);
                  fread(&m_key, 1, sizeof(MMP_U32), m_fp);
                  fread(&m_sr, 1, sizeof(MMP_S32), m_fp);
                  fread(&m_ch, 1, sizeof(MMP_S32), m_fp);
                  fread(&m_bps, 1, sizeof(MMP_S32), m_fp);
                  fseek(m_fp, 0, SEEK_SET);

                  if(m_key != KEY_VALUE) {
                       fclose(m_fp);
                       m_fp = NULL;
                  }
              }
          }
      }
    
public:
    virtual ~pcm_dump() {

         if(m_fp) {
             if(m_is_write) {
                 fseek(m_fp, 0, SEEK_END);
                 fwrite(&m_key, 1, sizeof(MMP_U32), m_fp);
                 fwrite(&m_sr, 1, sizeof(MMP_S32), m_fp);
                 fwrite(&m_ch, 1, sizeof(MMP_S32), m_fp);
                 fwrite(&m_bps, 1, sizeof(MMP_S32), m_fp);
             }
             fclose(m_fp);
         }
      }

public:
    inline MMP_BOOL is_init() { return (m_fp!=NULL)?MMP_TRUE:MMP_FALSE; }
    inline MMP_S32 get_sr() { return m_sr; }
    inline MMP_S32 get_ch() { return m_ch; }
    inline MMP_S32 get_bps() { return m_bps; } /* bits per sample */
    inline MMP_S32 get_abps() { return m_sr*m_ch*m_bps/8; } /* AvgByte Per Sec */
    MMP_S64 get_play_dur() { 
        long pos = 0;
        MMP_S64 dur = 0;

        dur = ((MMP_S64)m_filesize*1000000L) / (MMP_S64)this->get_abps();

        if(m_fp) {
            pos = ftell(m_fp);
        }
        return dur;
    }
};

class pcm_dump_write : public pcm_dump {

public:
    pcm_dump_write(MMP_CHAR* filename, MMP_S32 sr, MMP_S32 ch, MMP_S32 bps) : pcm_dump(filename, sr, ch, bps, MMP_TRUE)
    {
        
    }
    
    MMP_RESULT write_data(MMP_U8* data, MMP_S32 datasz) {
        MMP_S32 sz;
        MMP_RESULT mmpResult = MMP_FAILURE;
        if(m_fp) {
            sz = fwrite(data, 1, datasz, m_fp);
            if(sz == datasz) {
                mmpResult = MMP_SUCCESS;
            }
        }
        return mmpResult;
    }
};

class pcm_dump_read : public pcm_dump {

public:
    pcm_dump_read(MMP_CHAR* filename) : pcm_dump(filename, 0, 0, 0, MMP_FALSE)
    {
        
    }
    
    MMP_RESULT read_data(MMP_U8* data, MMP_S32 datasz, MMP_S32 *readsz) {
        MMP_S32 sz;
        MMP_RESULT mmpResult = MMP_FAILURE;

        if(readsz) *readsz = 0;

        if(m_fp) {
            sz = fread(data, 1, datasz, m_fp);
            if(readsz) *readsz = sz;
        }
        return mmpResult;
    }
};

void* pcm_dump_write_create(MMP_CHAR* pcm_filename, MMP_S32 sr, MMP_S32 ch, MMP_S32 bps) {

    class pcm_dump_write *p_pcm_dump = new pcm_dump_write(pcm_filename, sr, ch, bps);
    if(p_pcm_dump != NULL) {
        if(p_pcm_dump->is_init() != MMP_TRUE) {
            delete p_pcm_dump;
            p_pcm_dump = NULL;
        }
    }
    return (void*)p_pcm_dump;
}

MMP_RESULT pcm_dump_write_data(void* hdl, MMP_U8* data, MMP_S32 datasz) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    class pcm_dump_write *p_pcm_dump = (class pcm_dump_write *)hdl;
    if(p_pcm_dump!=NULL) {
        mmpResult = p_pcm_dump->write_data(data, datasz);
    }
    return mmpResult;
}

void* pcm_dump_read_create(MMP_CHAR* pcm_filename) {

    class pcm_dump_read *p_pcm_dump = new pcm_dump_read(pcm_filename);
    if(p_pcm_dump != NULL) {
        if(p_pcm_dump->is_init() != MMP_TRUE) {
            delete p_pcm_dump;
            p_pcm_dump = NULL;
        }
    }
    return (void*)p_pcm_dump;
}

MMP_RESULT pcm_dump_read_data(void* hdl, MMP_U8* data, MMP_S32 datasz, MMP_S32 *readsz) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    class pcm_dump_read *p_pcm_dump = (class pcm_dump_read *)hdl;
    if(p_pcm_dump!=NULL) {
        mmpResult = p_pcm_dump->read_data(data, datasz, readsz);
    }
    return mmpResult;
}

MMP_RESULT pcm_dump_destroy(void* hdl) {

    class pcm_dump *p_pcm_dump = (class pcm_dump *)hdl;
    if(p_pcm_dump!=NULL) {
        delete p_pcm_dump;
    }
    return MMP_SUCCESS;
}

/* sample rate*/
MMP_S32 pcm_dump_get_sr(void* hdl)  {

    MMP_S32 sr=0;
    class pcm_dump *p_pcm_dump = (class pcm_dump *)hdl;
    if(p_pcm_dump!=NULL) {
        sr = p_pcm_dump->get_sr();
    }
    return sr;
}

/* channel */
MMP_S32 pcm_dump_get_ch(void* hdl)  {

    MMP_S32 ch = 0;
    class pcm_dump *p_pcm_dump = (class pcm_dump *)hdl;
    if(p_pcm_dump!=NULL) {
        ch =p_pcm_dump->get_ch();
    }
    return ch;
}

/* bits per sample */
MMP_S32 pcm_dump_get_bps(void* hdl)  {
    MMP_S32 bps;
    class pcm_dump *p_pcm_dump = (class pcm_dump *)hdl;
    if(p_pcm_dump!=NULL) {
        bps = p_pcm_dump->get_bps();
    }
    return bps;
}

/* avg byte per sec */
MMP_S32 pcm_dump_get_abps(void* hdl) {
    MMP_S32 abps;
    class pcm_dump *p_pcm_dump = (class pcm_dump *)hdl;
    if(p_pcm_dump!=NULL) {
        abps = p_pcm_dump->get_abps();
    }
    return abps;
}
/* play duration */
MMP_S64 pcm_dump_get_play_dur(void* hdl)  {

    MMP_S64 dur;
    class pcm_dump *p_pcm_dump = (class pcm_dump *)hdl;
    if(p_pcm_dump!=NULL) {
        dur = p_pcm_dump->get_play_dur();
    }
    return dur;
}


/**************************************************************************************
  YUV Dump Tool
**************************************************************************************/
#if (MMP_PLATFORM == MMP_PLATFORM_WIN32)
#define YUV_DUMP_PATH "d:\\work\\"
#elif (MMP_PLATFORM == MMP_PLATFORM_ANDROID)
#define YUV_DUMP_PATH "/data/"
#else
#error "ERROR : Select YUV_DUMP_PATH"
#endif

class yuv_dump {

public:
    enum MODE {
        READ=0,
        WRITE
    };
private:
    MMP_S32 m_pic_width;
    MMP_S32 m_pic_height;
    enum MMP_FOURCC m_fourcc;
    FILE* m_fp;
    enum MODE m_mode;

public:
    yuv_dump(MMP_CHAR* name, MMP_S32 width, MMP_S32 height, enum MMP_FOURCC fourcc, enum MODE mode) : 
       m_pic_width(width), m_pic_height(height), m_fourcc(fourcc), m_fp(NULL), m_mode(mode) {
           
           MMP_CHAR filename[256];
           MMP_CHAR extname[16];

           if(m_mode == WRITE) {

               switch(m_fourcc) {
                    
                    case MMP_FOURCC_IMAGE_RGB888:  strcpy(extname, "rgb8888"); break;
                    case MMP_FOURCC_IMAGE_BGR888:  strcpy(extname, "bgr8888"); break;

                    case MMP_FOURCC_IMAGE_ARGB8888:  strcpy(extname, "argb8888"); break;
                    case MMP_FOURCC_IMAGE_ABGR8888:  strcpy(extname, "abgr8888"); break;

                    case MMP_FOURCC_IMAGE_YUV420:
                    case MMP_FOURCC_IMAGE_YUV420M:  strcpy(extname, "yuv"); break;

                    default: 
                        strcpy(extname, "unknown"); break;
               }

              sprintf(filename, YUV_DUMP_PATH"%s-%d.%s", name, CMmpUtil::GetTickCount(), extname );

              m_fp = fopen(filename, "wb");
              if(m_fp == NULL) {
                    MMPDEBUGMSG(1, (TEXT("[yuv_dump init] FAIL: file open(write mode) - %s  %dx%d "), filename, width, height ));
              }
           }
           else {
              m_fp = fopen(name, "rb");  
           }
       }

    ~yuv_dump() { 

        if(m_fp != NULL) {

            if(m_mode == WRITE) {
                MMPBITMAPINFOHEADER bih;

                bih.biSize = sizeof(MMPBITMAPINFOHEADER);
                if( (m_fourcc == MMP_FOURCC_IMAGE_YUV420) 
                    || (m_fourcc==MMP_FOURCC_IMAGE_YUV420M) 
                    ) {
                    bih.biCompression = MMPMAKEFOURCC('Y','U','V',' ');
                }
                else {
                    bih.biCompression = m_fourcc;
                }
                bih.biWidth = m_pic_width;
                bih.biHeight = m_pic_height;

                fwrite((void*)&bih, 1, bih.biSize, m_fp);
            }

            fclose(m_fp);
            m_fp = NULL;
        }
    }

    MMP_BOOL is_init() { return (m_fp!=NULL)?MMP_TRUE:MMP_FALSE; }
    
    MMP_RESULT write_data(MMP_U8* data, MMP_S32 datasz) {
        MMP_RESULT mmpResult = MMP_FAILURE;
        if( (m_mode == WRITE) && (m_fp!=NULL) ) {
            fwrite((void*)data, 1, datasz, m_fp);
            mmpResult = MMP_SUCCESS;
        }
        return mmpResult;
    }


    MMP_RESULT write_YUV420M(MMP_U8* Y, MMP_U8* U, MMP_U8* V, 
                        MMP_S32 y_stride, MMP_S32 u_stride, MMP_S32 v_stride
                        ) {
    
        if( (m_mode == WRITE) && (m_fp!=NULL) && (m_fourcc==MMP_FOURCC_IMAGE_YUV420M) ) {
        
            MMP_U8 *line;
            MMP_S32 y;

            line = Y;
            for(y = 0; y < m_pic_height; y++) {
                fwrite((void*)line, 1, m_pic_width, m_fp);
                line += y_stride;
            }
            
            line = U;
            for(y = 0; y < (m_pic_height/2); y++) {
                fwrite((void*)line, 1, m_pic_width/2, m_fp);
                line += u_stride;
            }
            
            line = V;
            for(y = 0; y < (m_pic_height/2); y++) {
                fwrite((void*)line, 1, m_pic_width/2, m_fp);
                line += v_stride;
            }
        
        }

        return MMP_SUCCESS;
    }


private:
    MMP_RESULT write_data_yuv420(MMP_U8* data, MMP_S32 datasz, MMP_S32 luma_stride, MMP_S32 chroma_stride, MMP_S32 luma_alignheight, MMP_S32 chroma_alignheight) {

        MMP_U8 *Y, *U, *V, *line;
        MMP_S32 y;

        Y = data;
        U = Y + luma_stride*luma_alignheight;
        V = U + chroma_stride*chroma_alignheight;
    
        line = Y;
        for(y = 0; y < m_pic_height; y++) {
            fwrite((void*)line, 1, m_pic_width, m_fp);
            line += luma_stride;
        }
            
        line = U;
        for(y = 0; y < (m_pic_height/2); y++) {
            fwrite((void*)line, 1, m_pic_width/2, m_fp);
            line += chroma_stride;
        }
            
        line = V;
        for(y = 0; y < (m_pic_height/2); y++) {
            fwrite((void*)line, 1, m_pic_width/2, m_fp);
            line += chroma_stride;
        }
            
        return MMP_SUCCESS;
    }

};

void* yuv_dump_write_create(MMP_CHAR* name, MMP_S32 width, MMP_S32 height, enum MMP_FOURCC fourcc) {

    class yuv_dump *p_obj;

    p_obj = new yuv_dump(name, width, height, fourcc, yuv_dump::WRITE);
    if(p_obj != NULL) {
        if(p_obj->is_init() != MMP_TRUE) {
            delete p_obj;
            p_obj = NULL;
        }
    }

    return (void*)p_obj;
}

MMP_RESULT yuv_dump_destroy(void* hdl) {
    
    class yuv_dump *p_obj = (class yuv_dump *)hdl;
    if(p_obj != NULL) {
        delete p_obj;
    }
    return MMP_SUCCESS;
}

MMP_RESULT yuv_dump_write_data(void* hdl, MMP_U8* data, MMP_S32 datasz) {
    
    class yuv_dump *p_obj = (class yuv_dump *)hdl;
    MMP_RESULT mmpResult = MMP_FAILURE;
    
    if(p_obj != NULL) {
        mmpResult = p_obj->write_data(data, datasz);
    }

    return mmpResult;
}

MMP_RESULT yuv_dump_write_YUV420M(void* hdl, 
                                  MMP_U8* Y, MMP_U8* U, MMP_U8* V, 
                                  MMP_S32 y_stride, MMP_S32 u_stride, MMP_S32 v_stride
                                  ) {
    
    class yuv_dump *p_obj = (class yuv_dump *)hdl;
    MMP_RESULT mmpResult = MMP_FAILURE;
    
    if(p_obj != NULL) {
        mmpResult = p_obj->write_YUV420M(Y, U, V, y_stride, u_stride, v_stride);
    }

    return mmpResult;
}


/**************************************************************************************
  end of PCM Dump Tool
**************************************************************************************/

MMP_S64 mmp_audio_cal_dur(MMP_S32 pcm_byte, MMP_S32 sr, MMP_S32 ch, MMP_S32 bps);