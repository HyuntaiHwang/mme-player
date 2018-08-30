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

#include "MmpEncoderVideo_Vpu.hpp"
#include "MmpUtil.hpp"
#include "MmpImageTool.hpp"
#include "mmp_lock.hpp"

/////////////////////////////////////////////////////////////
//CMmpEncoderVideo_Vpu Member Functions

#define CTS_FRAME_IMAGE_TEST_Y  120                  // YUV values for colored rect
#define CTS_FRAME_IMAGE_TEST_U  160
#define CTS_FRAME_IMAGE_TEST_V  200
#define CTS_FRAME_IMAGE_TEST_R0  0                   // RGB equivalent of {0,0,0} (BT.601)
#define CTS_FRAME_IMAGE_TEST_G0  136
#define CTS_FRAME_IMAGE_TEST_B0  0
#define CTS_FRAME_IMAGE_TEST_R1  236                 // RGB equivalent of {120,160,200} (BT.601)
#define CTS_FRAME_IMAGE_TEST_G1  50
#define CTS_FRAME_IMAGE_TEST_B1  186
#define CTS_FRAME_IMAGE_TEST_R0_BT709  0             // RGB equivalent of {0,0,0} (BT.709)
#define CTS_FRAME_IMAGE_TEST_G0_BT709  77
#define CTS_FRAME_IMAGE_TEST_B0_BT709  0
#define CTS_FRAME_IMAGE_TEST_R1_BT709  250           // RGB equivalent of {120,160,200} (BT.709)
#define CTS_FRAME_IMAGE_TEST_G1_BT709  76
#define CTS_FRAME_IMAGE_TEST_B1_BT709  189


class mmp_vpu_enc_lock {

private:
	class mmp_vpu_if *m_p_vpu_if;
	
public:
	mmp_vpu_enc_lock(class mmp_vpu_if *p_vpu_if) : m_p_vpu_if(p_vpu_if)
    {
        p_vpu_if->get_external_mutex()->lock();
        p_vpu_if->clk_enable();
    }

	~mmp_vpu_enc_lock() {
        m_p_vpu_if->clk_disable();
        m_p_vpu_if->get_external_mutex()->unlock();
    }
};


CMmpEncoderVideo_Vpu::CMmpEncoderVideo_Vpu(struct CMmpEncoderVideo::create_config *p_create_config) : 

CMmpEncoderVideo(p_create_config, "VPU")
,CLASS_ENCODER_VPU(p_create_config->fourcc_in, p_create_config->fourcc_out,  p_create_config->pic_width, p_create_config->pic_height)
,m_p_vpu_if(NULL)
,m_nInputPictureCount(0)
,m_nEncodedStreamCount(0)
,m_bEncodeOpen(MMP_FALSE)
,m_bEncodeDSI(MMP_FALSE)
,m_p_buf_vf_csc(NULL)
#ifdef CMmpEncoderVideo_Vpu_CSC_SAVE_BMP
,m_bitmap_index(0)
#endif
{
    
}

CMmpEncoderVideo_Vpu::~CMmpEncoderVideo_Vpu()
{

#ifdef CMmpEncoderVideo_Vpu_CSC_SAVE_BMP
    int ibmp;

    char bmpname[128];
    
    for(ibmp = 0; ibmp < m_bitmap_index; ibmp++) {

        if(m_bmp_data[ibmp]==NULL) continue;

        sprintf(bmpname, "/data/bmp/video_enc%04d.bmp", ibmp);
        if( CMmpImageTool::Bmp_SaveFile(bmpname, 
                                        m_bmp_width, m_bmp_height, m_bmp_stride, 
                                        (MMP_U8*)m_bmp_data[ibmp], 
                                        m_bmp_fourcc ) == MMP_SUCCESS ) {

            MMPDEBUGMSG(1, (TEXT("[CMmpEncoderVideo_Vpu::vf_csc] BMP SAVE %s  m_bitmap_index=%d "),bmpname, m_bitmap_index));
        }
        else {
            MMPDEBUGMSG(1, (TEXT("[CMmpEncoderVideo_Vpu::vf_csc] BMP SAVE %s "),bmpname));
        }
    }

    MMPDEBUGMSG(1, (TEXT("[CMmpEncoderVideo_Vpu::~~~]  ")));

#endif

}

MMP_RESULT CMmpEncoderVideo_Vpu::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;
    
    /* Enc initially check */
    if(mmpResult == MMP_SUCCESS) {
        mmpResult=CMmpEncoderVideo::Open();
        if(mmpResult!=MMP_SUCCESS)   {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVideo_Vpu::Open] FAIL : CMmpEncoderVideo::Open() ")));
        }
    }

    /* vpu_if 생성하고, streaming_ref_count를 체크한다.  ref_count가 Full이면 Error 리턴*/
    if(mmpResult == MMP_SUCCESS) {
        m_p_vpu_if = mmp_vpu_if::create_object();
        if(m_p_vpu_if == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVideo_Vpu::Open] FAIL : mmp_vpu_if::create_object ")));
        }
        else {
            enum MMP_FOURCC fourcc_in = this->get_fourcc_in(); 
            enum MMP_FOURCC fourcc_out = this->get_fourcc_out(); 
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[VPU Enc] Streaming Reference Count : %d    %c%c%c%c %c%c%c%c "), 
                         m_p_vpu_if->streaming_ref_count() ,
                         MMPGETFOURCCARG(fourcc_in), MMPGETFOURCCARG(fourcc_out)
                         ));
            if(m_p_vpu_if->streaming_is_full() == MMP_TRUE) {
                mmpResult = MMP_FAILURE;
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("ERROR: [VPU Enc] Streaming Reference Count is Full !! ")));
            }
        }
    }

    if(m_p_vpu_if!=NULL)   m_p_vpu_if->get_external_mutex()->lock();

    /* Streaming Start / PM-Domain On  */
    if(mmpResult == MMP_SUCCESS) {
        mmpResult = m_p_vpu_if->streaming_start((void*)this);
        if(mmpResult != MMP_SUCCESS) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVideo_Vpu::Open] FAIL: VPU Streaming Start !! ")));
        }
    }
       

    if(m_p_vpu_if!=NULL)   m_p_vpu_if->get_external_mutex()->unlock();

    return mmpResult;
}


MMP_RESULT CMmpEncoderVideo_Vpu::Close()
{
    MMP_RESULT mmpResult;
    
    if(m_p_vpu_if != NULL) {
        
        m_p_vpu_if->get_external_mutex()->lock();

        m_p_vpu_if->clk_enable();
        CLASS_ENCODER_VPU::Close(m_p_vpu_if);
        m_p_vpu_if->clk_disable();
        
        m_p_vpu_if->streaming_stop((void*)this);
        
        m_p_vpu_if->get_external_mutex()->unlock();

        mmp_vpu_if::destroy_object(m_p_vpu_if);
        m_p_vpu_if = NULL;
    }

    mmpResult=CMmpEncoderVideo::Close();
    if(mmpResult!=MMP_SUCCESS)   {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVideo_Vpu::Close] CMmpEncoderVideo::Close() \n\r")));
    }
    
    if(m_p_buf_vf_csc != NULL) {
        mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_vf_csc);
        m_p_buf_vf_csc = NULL;
    }

    return mmpResult;
}

void CMmpEncoderVideo_Vpu::set_pic_width(MMP_S32 w) {

    CMmpEncoderVideo::set_pic_width(w);
    CLASS_ENCODER_VPU::set_pic_width(w);
}
    
void CMmpEncoderVideo_Vpu::set_pic_height(MMP_S32 h) {
    
    CMmpEncoderVideo::set_pic_height(h);
    CLASS_ENCODER_VPU::set_pic_height(h);
}

class mmp_buffer_videoframe* CMmpEncoderVideo_Vpu::vf_csc(class mmp_buffer_videoframe* p_buf_vf) {

    class mmp_buffer_videoframe* p_buf_vf_src = NULL;
    enum MMP_FOURCC fourcc_input = p_buf_vf->get_fourcc();

    switch(fourcc_input) {

        case MMP_FOURCC_IMAGE_ABGR8888:
        case MMP_FOURCC_IMAGE_ARGB8888:
            {
                unsigned char *Y, *U, *V;
                int luma_stride, chroma_stride;
                int pic_width, pic_height;
                int rgb_buf_stride;

                pic_width = p_buf_vf->get_pic_width();
                pic_height = p_buf_vf->get_pic_height();
                rgb_buf_stride = p_buf_vf->get_buf_stride();

                if(m_p_buf_vf_csc != NULL) {
                    if(  (m_p_buf_vf_csc->get_pic_width() != p_buf_vf->get_pic_width())
                        || (m_p_buf_vf_csc->get_pic_height() != p_buf_vf->get_pic_height())
                        ) {
                        
                        mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_vf_csc);
                        m_p_buf_vf_csc = NULL;       
                    }
                }

                if(m_p_buf_vf_csc == NULL) {
                    m_p_buf_vf_csc = mmp_buffer_mgr::get_instance()->alloc_media_videoframe((MMP_MEDIA_ID)this, 
                                                                                            pic_width, pic_height,
                                                                                            MMP_FOURCC_IMAGE_YUV420M, 
                                                                                            mmp_buffer::ION,
                                                                                            mmp_buffer::FLAG_CACHED ); 
                }

#ifdef CMmpEncoderVideo_Vpu_CSC_SAVE_BMP
                {
#if 0
                    char bmpname[128];
                    unsigned int t1, t2;
                    p_buf_vf->sync_buf(0);
                    t1 = CMmpUtil::GetTickCount();
                    sprintf(bmpname, "/data/bmp/video_enc%04d.bmp", m_bitmap_index);
                    if( CMmpImageTool::Bmp_SaveFile(bmpname, 
                                                    pic_width, pic_height, rgb_buf_stride, 
                                                    (MMP_U8*)p_buf_vf->get_buf_vir_addr(), 
                                                    p_buf_vf->get_fourcc() ) == MMP_SUCCESS ) {

                        t2 = CMmpUtil::GetTickCount();
                        MMPDEBUGMSG(1, (TEXT("[CMmpEncoderVideo_Vpu::vf_csc] BMP SAVE %s dur=%d "),bmpname, t2-t1));
                        m_bitmap_index++;
                    }
                    else {
                        MMPDEBUGMSG(1, (TEXT("[CMmpEncoderVideo_Vpu::vf_csc] BMP SAVE %s "),bmpname));
                    }
#else
                    m_bmp_data[m_bitmap_index] = new unsigned char[ pic_width * pic_height * 5];
                    memcpy(m_bmp_data[m_bitmap_index], (const void*)p_buf_vf->get_buf_vir_addr(), rgb_buf_stride * pic_height );

                    m_bmp_width = pic_width;
                    m_bmp_height = pic_height;
                    m_bmp_stride = rgb_buf_stride;
                    m_bmp_fourcc = p_buf_vf->get_fourcc();
                    m_bitmap_index++;
#endif
                }
#endif

                Y = (unsigned char *)m_p_buf_vf_csc->get_buf_vir_addr_y();
                U = (unsigned char *)m_p_buf_vf_csc->get_buf_vir_addr_u();
                V = (unsigned char *)m_p_buf_vf_csc->get_buf_vir_addr_v();
            
                luma_stride = m_p_buf_vf_csc->get_stride_luma();
                chroma_stride = m_p_buf_vf_csc->get_stride_chroma();

                MMPDEBUGMSG(m_DEBUG_MMEEnc, (TEXT("[CMmpEncoderVideo_Vpu::vf_csc] RGB=>YUV420 pic_width=%d pic_height=%d stride=%d fourcc=%c%c%c%c luma_stride=%d chroma_stride=%d "), 
                                pic_width, pic_height, rgb_buf_stride, 
                                MMPGETFOURCCARG(fourcc_input), 
                                luma_stride, chroma_stride 
                                ));
                
#if 0
                MMP_BOOL bChkImg = CMmpUtil::CTS_CheckSurfaceFrame_RGB32(m_nInputPictureCount-1, //MMP_S32 frameIndex, 
                                            pic_width, pic_height, (MMP_PTR)p_buf_vf->get_buf_vir_addr(),
                                            CTS_FRAME_IMAGE_TEST_R0, CTS_FRAME_IMAGE_TEST_G0, CTS_FRAME_IMAGE_TEST_B0,
                                            CTS_FRAME_IMAGE_TEST_R1, CTS_FRAME_IMAGE_TEST_G1, CTS_FRAME_IMAGE_TEST_B1,
                                            MMP_TRUE /*MMP_BOOL TopDown*/
                                            );

                if(bChkImg != MMP_TRUE) {
                    MMPDEBUGMSG(1, (TEXT("[CMmpEncoderVideo_Vpu::vf_csc]  ")));
                    MMPDEBUGMSG(1, (TEXT("[CMmpEncoderVideo_Vpu::vf_csc] ImgIdx=%d  ChkImg ERRRRRRRRRRRRRRRRRR !!!!!!!! "), m_nInputPictureCount-1 ));
                    MMPDEBUGMSG(1, (TEXT("[CMmpEncoderVideo_Vpu::vf_csc]  ")));
                    return NULL;
                }
                else {
                    MMPDEBUGMSG(1, (TEXT("[CMmpEncoderVideo_Vpu::vf_csc]  ")));
                    MMPDEBUGMSG(1, (TEXT("[CMmpEncoderVideo_Vpu::vf_csc] ImgIdx=%d  ChkImg OK ^^^^^^^^^^^^"), m_nInputPictureCount-1 ));
                    MMPDEBUGMSG(1, (TEXT("[CMmpEncoderVideo_Vpu::vf_csc]  ")));
                }
#endif

#if 0
                /*  CTS-media 
                       android.media.cts.EncodeDecodeTest:: testEncodeDecodeVideoFromPersistentSurfaceToSurface720p	관련 module, 
                */
                unsigned char *Y1 = new unsigned char[luma_stride*pic_height];
                unsigned char *U1 = new unsigned char[luma_stride*pic_height];
                unsigned char *V1 = new unsigned char[luma_stride*pic_height];
                unsigned char *RGB = new unsigned char[rgb_buf_stride*pic_height];
                
                p_buf_vf->sync_buf(0);
                memcpy(RGB, (const void*)p_buf_vf->get_buf_vir_addr(), rgb_buf_stride*pic_height);

                CMmpImageTool::ConvertRGBtoYUV420M_neon_ex1((MMP_U8*)RGB, 
                                                  pic_width, pic_height, rgb_buf_stride, 
                                                  p_buf_vf->get_fourcc(),
                                                  Y1, U1, V1,
                                                  luma_stride, chroma_stride, chroma_stride);

                memcpy(Y, Y1, luma_stride*pic_height);
                memcpy(U, U1, chroma_stride*pic_height/2);
                memcpy(V, V1, chroma_stride*pic_height/2);

                m_p_buf_vf_csc->sync_buf(0);
                m_p_buf_vf_csc->sync_buf(1);
                m_p_buf_vf_csc->sync_buf(2);

                delete [] Y1;
                delete [] U1;
                delete [] V1;
                delete [] RGB;

#else
                //p_buf_vf->sync_buf(0);

#if 0
                CMmpImageTool::ConvertRGBtoYUV420M((MMP_U8*)p_buf_vf->get_buf_vir_addr(), 
                                                  p_buf_vf->get_pic_width(), p_buf_vf->get_pic_height(), p_buf_vf->get_fourcc(),
                                                  Y, U, V,
                                                  luma_stride, chroma_stride, chroma_stride);

#else
                CMmpImageTool::ConvertRGBtoYUV420M_neon_ex1((MMP_U8*)p_buf_vf->get_buf_vir_addr(), 
                                                  p_buf_vf->get_pic_width(), p_buf_vf->get_pic_height(), p_buf_vf->get_buf_stride(), 
                                                  p_buf_vf->get_fourcc(),
                                                  Y, U, V,
                                                  luma_stride, chroma_stride, chroma_stride);

#endif

                m_p_buf_vf_csc->sync_buf(0);
                m_p_buf_vf_csc->sync_buf(1);
                m_p_buf_vf_csc->sync_buf(2);

                
#endif
                                
                p_buf_vf_src = m_p_buf_vf_csc;
                                
            }
            break;

        case MMP_FOURCC_IMAGE_YUV420:
        
            if(p_buf_vf->get_buf_type() == MMP_BUFTYPE_SW ){
                            
                if(m_p_buf_vf_csc != NULL) {
                    if(  (m_p_buf_vf_csc->get_pic_width() != p_buf_vf->get_pic_width())
                        || (m_p_buf_vf_csc->get_pic_height() != p_buf_vf->get_pic_height())
                        ) {
                        mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_vf_csc);
                        m_p_buf_vf_csc = NULL;       
                    }
                }

                if(m_p_buf_vf_csc == NULL) {
                    m_p_buf_vf_csc = mmp_buffer_mgr::get_instance()->alloc_media_videoframe((MMP_MEDIA_ID)this, 
                                                                                            p_buf_vf->get_pic_width(), p_buf_vf->get_pic_height(),
                                                                                            MMP_FOURCC_IMAGE_YUV420M, 
                                                                                            mmp_buffer::ION,
                                                                                            mmp_buffer::FLAG_NONE /*mmp_buffer::FLAG_CACHED*/ ); 
                }

                mmp_buffer_videoframe::csc_YUV420toYUV420(p_buf_vf, m_p_buf_vf_csc);
            
                MMPDEBUGMSG(m_DEBUG_MMEEnc, (TEXT("[CMmpEncoderVideo_Vpu::vf_csc] YUV420=>YUV420 pic_width=%d pic_height=%d stride=%d fourcc=%c%c%c%c "), 
                                       p_buf_vf->get_pic_width(), p_buf_vf->get_pic_height(), p_buf_vf->get_buf_stride(), 
                                       MMPGETFOURCCARG(fourcc_input)
                                       ));

                p_buf_vf_src = m_p_buf_vf_csc;
            }
            else {
                p_buf_vf_src = p_buf_vf;
            }

            break;

        default:
            p_buf_vf_src = p_buf_vf;
            break;
    }

    return p_buf_vf_src;
}

MMP_RESULT CMmpEncoderVideo_Vpu::EncodeAu(class mmp_buffer_videoframe* p_buf_vf, class mmp_buffer_videostream* p_buf_vs) {

    MMP_RESULT mmpResult = MMP_SUCCESS; 
    MMP_U32 preproc_start_tick, enc_start_tick, enc_end_tick;
    MMP_U8* pTemp;
    class mmp_buffer_videoframe* p_buf_vf_src;
    class mmp_vpu_enc_lock autolock(m_p_vpu_if);
    
    preproc_start_tick = CMmpUtil::GetTickCount();
    m_fourcc_last_input = p_buf_vf->get_fourcc();

    p_buf_vs->set_stream_size(0);
    p_buf_vs->set_stream_offset(0);
    p_buf_vs->set_flag(mmp_buffer_media::FLAG_NULL);
    p_buf_vs->set_pts(p_buf_vf->get_pts());
    p_buf_vs->set_dsi_size(0);

    m_nInputPictureCount++;
    
    p_buf_vf_src = this->vf_csc(p_buf_vf);
    if(p_buf_vf_src == NULL) {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVideo_Vpu::EncodeAu] ln=%d FAIL: csc resule is NULL "), __LINE__ ));
        return MMP_FAILURE;
    }
           
    /* VPU Open 
        Enc에 필요한 config값이  CMmpEncoderVideo_Vpu::Open 이 호출될때는 아직 설정되지 않았으므로  여기에 VPU_Open을 수행해야 한다.
    */   
    if(m_bEncodeOpen == MMP_FALSE) {
        
        mmpResult=CLASS_ENCODER_VPU::Open(m_p_vpu_if, p_buf_vf_src);
        if(mmpResult!=MMP_SUCCESS)  {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVideo_Vpu::EncodeAu] FAIL: CLASS_ENCODER_VPU::Open() ")));
            return mmpResult;
        }
        else {
            m_bEncodeOpen = MMP_TRUE;
        }

    }
        
    if(m_bEncodeDSI == MMP_FALSE) {
        
        mmpResult = CLASS_ENCODER_VPU::EncodeDSI(m_p_vpu_if, p_buf_vf_src);
        if(mmpResult != MMP_SUCCESS) {
            return mmpResult;
        }
        m_bEncodeDSI = MMP_TRUE;

#if (MMP_OS == MMP_OS_WIN32)
        this->m_DSISize = 32;
#endif
        if(this->m_DSISize > 0) {
            p_buf_vs->alloc_dsi_buffer(this->m_DSISize);
            memcpy(p_buf_vs->get_dsi_buffer(), this->m_DSI, this->m_DSISize);
            p_buf_vs->set_dsi_size(this->m_DSISize);
        }
        
        pTemp = (MMP_U8*)this->m_DSI;
        MMPDEBUGMSG(0, (TEXT("[CMmpEncoderVideo_Vpu::EncodeAu] ln=%d Sz=(%d) (%02x %02x %02x %02x %02x %02x %02x %02x )"), __LINE__, 
                this->m_DSISize,
               (unsigned int)pTemp[0],(unsigned int)pTemp[1],(unsigned int)pTemp[2],(unsigned int)pTemp[3],
               (unsigned int)pTemp[4],(unsigned int)pTemp[5],(unsigned int)pTemp[6],(unsigned int)pTemp[7]
             ));
   }


    enc_start_tick = CMmpUtil::GetTickCount();
    mmpResult = CLASS_ENCODER_VPU::EncodeAu(m_p_vpu_if, p_buf_vf_src, p_buf_vs);
    if(mmpResult == MMP_SUCCESS) {

#if (MMP_OS == MMP_OS_WIN32)
        p_buf_vs->set_stream_size(4096);
#endif
        if(p_buf_vs->get_stream_real_size() > 0) {
            m_nEncodedStreamCount++;
        }
    }
    enc_end_tick = CMmpUtil::GetTickCount();
    
    p_buf_vs->set_preproc_dur(enc_start_tick - preproc_start_tick);
    p_buf_vs->set_coding_dur(enc_end_tick - enc_start_tick);

    //MMPDEBUGMSG(1, (TEXT("[CMmpEncoderVideo_Vpu::EncodeAu] ln=%d pre_dur=%d enc_dur=%d "), __LINE__, enc_start_tick - preproc_start_tick, enc_end_tick - enc_start_tick ));
        
    return mmpResult;
}