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

#include "MmpDecoderVideo_VpuS.hpp"
#include "MmpUtil.hpp"
#include <ion.h>

#define CSC_HW_V4L2 1
#define CSC_HW_MSCALER  2
#define CSC_SW 3

#ifdef WIN32
#define CSC_HW  CSC_SW
#else
//#define CSC_HW  CSC_HW_V4L2
//#define CSC_HW  CSC_HW_MSCALER
#define CSC_HW  CSC_SW
#endif

/////////////////////////////////////////////////////////////
//CMmpDecoderVideo_VpuS Member Functions

MMP_BOOL CMmpDecoderVideo_VpuS::CheckSupportCodec(enum MMP_FOURCC fourcc_in) {
    MMP_BOOL bflag;

    switch(fourcc_in) {
        case MMP_FOURCC_VIDEO_H263:
        case MMP_FOURCC_VIDEO_H264:
        case MMP_FOURCC_VIDEO_MPEG4:
        case MMP_FOURCC_VIDEO_MPEG2:
        
        case MMP_FOURCC_VIDEO_WMV3:
        
        case MMP_FOURCC_VIDEO_RV30: 
        case MMP_FOURCC_VIDEO_RV40: 
        
        case MMP_FOURCC_VIDEO_MSMPEG4V2:
        case MMP_FOURCC_VIDEO_MSMPEG4V3: /* Divx3 */
        
        case MMP_FOURCC_VIDEO_VP80:
		case MMP_FOURCC_VIDEO_THEORA:
		case MMP_FOURCC_VIDEO_FLV1:
        //case MMP_FOURCC_VIDEO_VP60:
        //case MMP_FOURCC_VIDEO_VP6F:
        
        /*
           VPU support THEORA, but It is difficult to understand NOW,,  
            case MMP_FOURCC_VIDEO_THEORA:
        */

        //case MMP_FOURCC_VIDEO_FLV1:
        
            bflag = MMP_TRUE;
            break;

        default:
            bflag = MMP_FALSE;
    }

    return bflag;
}


static const enum MMP_FOURCC s_support_fourcc_out[]={
     MMP_FOURCC_IMAGE_YUV420,
     MMP_FOURCC_IMAGE_YUV420M,
     MMP_FOURCC_IMAGE_YVU420,
     MMP_FOURCC_IMAGE_YVU420M
};

CMmpDecoderVideo_VpuS::CMmpDecoderVideo_VpuS(struct CMmpDecoderVideo::create_config *p_create_config) : 

    CMmpDecoderVideo(p_create_config, "VPU", s_support_fourcc_out, sizeof(s_support_fourcc_out)/sizeof(s_support_fourcc_out[0]))
    ,CLASS_DECODER_VPU(p_create_config->fourcc_in)

,m_bDecodeDSI(MMP_FALSE)
,m_hScaler(NULL)
,m_mscaler_test_fd(-1)

,m_p_buf_vs_in(NULL)
,m_p_buf_vf_vpu_last(NULL)
,m_dec_dur_last(0)
,m_dec_pts_last(0)

{
    
}

CMmpDecoderVideo_VpuS::~CMmpDecoderVideo_VpuS()
{

}

MMP_RESULT CMmpDecoderVideo_VpuS::Open()
{
    MMP_RESULT mmpResult;

#if (CSC_HW==CSC_HW_V4L2)
    m_hScaler = MScalerOpen("/dev/video8");
#elif (CSC_HW==CSC_HW_MSCALER)
    m_mscaler_test_fd = ::open("/dev/mscaler", O_RDWR);
#endif
    
    mmpResult=CMmpDecoderVideo::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }
    

    mmpResult=CLASS_DECODER_VPU::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    return MMP_SUCCESS;
}


MMP_RESULT CMmpDecoderVideo_VpuS::Close()
{
    MMP_RESULT mmpResult;


    mmpResult=CLASS_DECODER_VPU::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_VpuS::Close] CMmpDecoderFfmpeg::Close() \n\r")));
        return mmpResult;
    }

    mmpResult=CMmpDecoderVideo::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_VpuS::Close] CMmpDecoderVideo::Close() \n\r")));
        return mmpResult;
    }

    if(m_hScaler != NULL) {
        MScalerClose(m_hScaler);
    }
    
    if(m_mscaler_test_fd >= 0) {
        ::close(m_mscaler_test_fd);
    }
    
    return MMP_SUCCESS;
}


MMP_RESULT CMmpDecoderVideo_VpuS::DecodeAu(class mmp_buffer_videostream* p_buf_vs, class mmp_buffer_videoframe** pp_buf_vf, MMP_BOOL *is_reconfig) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U32 dec_start_tick, dec_end_tick;
    class mmp_buffer_videoframe* p_buf_vf_vpu = NULL;
    class mmp_buffer_videoframe* p_buf_vf_decoded = NULL;

     /* Init Parmeter */
    if(pp_buf_vf != NULL) {
        *pp_buf_vf = NULL;
    }

    if(is_reconfig!=NULL) *is_reconfig = MMP_FALSE;
       
    if(m_bDecodeDSI == MMP_FALSE) {
        mmpResult = CLASS_DECODER_VPU::DecodeDSI(p_buf_vs);
        if(mmpResult == MMP_SUCCESS) {
            m_bDecodeDSI = MMP_TRUE;
            if( (this->get_pic_width() != CLASS_DECODER_VPU::GetVpuPicWidth())  
                ||  (this->get_pic_height() != CLASS_DECODER_VPU::GetVpuPicHeight()) ) {
		            if(is_reconfig!=NULL) *is_reconfig = MMP_TRUE;
                    this->cfg_buf_vf(CLASS_DECODER_VPU::GetVpuPicWidth(), CLASS_DECODER_VPU::GetVpuPicHeight());
                    return MMP_SUCCESS;
            }
        }
        else {
            return mmpResult;
        }
    }

    if((p_buf_vs->get_flag()&mmp_buffer_media::FLAG_CONFIGDATA) != 0) {
        if(is_reconfig!=NULL) *is_reconfig = MMP_TRUE;
        return MMP_SUCCESS;
    }

    dec_start_tick = CMmpUtil::GetTickCount();
    mmpResult = this->DecodeAu_PinEnd(p_buf_vs, &p_buf_vf_vpu);
    dec_end_tick = CMmpUtil::GetTickCount();

    /* check reconfig */
    if((this->get_pic_width() != CLASS_DECODER_VPU::GetVpuPicWidth())  ||  (this->get_pic_height() != CLASS_DECODER_VPU::GetVpuPicHeight())  ) {

        if(is_reconfig!=NULL) *is_reconfig = MMP_TRUE;
        this->cfg_buf_vf(CLASS_DECODER_VPU::GetVpuPicWidth(), CLASS_DECODER_VPU::GetVpuPicHeight());

    }
    /* copy image to decoded buffer */
    else if(p_buf_vf_vpu != NULL)  {

        p_buf_vf_decoded = this->get_next_vf();
        if(p_buf_vf_decoded != NULL) {
            
#if (CSC_HW==CSC_HW_V4L2)
            this->do_csc_hw(p_buf_vf_vpu, p_buf_vf_decoded);
#elif (CSC_HW==CSC_HW_MSCALER)
            this->do_csc_mscaler_test(p_buf_vf_vpu, p_buf_vf_decoded);
#else
            this->do_csc_sw(p_buf_vf_vpu, p_buf_vf_decoded);
#endif

            //this->do_csc(p_buf_vf_vpu, p_buf_vf_decoded);
            //this->do_csc_hw(p_buf_vf_vpu, p_buf_vf_decoded);
            //this->do_csc_mscaler_test(p_buf_vf_vpu, p_buf_vf_decoded);
            
            p_buf_vf_decoded->set_coding_dur(dec_end_tick - dec_start_tick);
            p_buf_vf_decoded->set_pts(p_buf_vs->get_pts());
            p_buf_vf_decoded->clear_own();

        }
    }

    if(pp_buf_vf != NULL) {
        *pp_buf_vf = p_buf_vf_decoded;
    }

    p_buf_vs->inc_stream_offset(p_buf_vs->get_stream_real_size());

	return mmpResult; 
}

MMP_RESULT CMmpDecoderVideo_VpuS::Play_Function_Tool(MMP_PLAY_FORMAT playformat, MMP_S64 curpos, MMP_S64 totalpos)
{
	MMP_RESULT mmpResult = MMP_SUCCESS; 

    MMPDEBUGMSG(0, (TEXT("[CMmpDecoderVideo_VpuS] Play_Function_Tool!!!")));
	mmpResult = CLASS_DECODER_VPU::Play_Function_Tool(playformat,curpos,totalpos);
	
	return mmpResult;
}


MMP_RESULT CMmpDecoderVideo_VpuS::do_csc(class mmp_buffer_videoframe* p_buf_vf_vpu, class mmp_buffer_videoframe* p_buf_vf_decoded) {
    
    int pic_width, pic_height;
    MMP_RESULT mmpResult;

    MMP_BOOL b_hw = MMP_TRUE;
    
    pic_width = CLASS_DECODER_VPU::GetVpuPicWidth();
    pic_height = CLASS_DECODER_VPU::GetVpuPicHeight();
    
    if( (pic_width <= 480) && (pic_height <= 272) )  b_hw = MMP_FALSE;
    if(p_buf_vf_decoded->get_buf_shared_fd() == -1)  b_hw = MMP_FALSE;

#if (CSC_HW==CSC_HW_V4L2)
    if(m_hScaler == NULL)     b_hw = MMP_FALSE;
#elif (CSC_HW==CSC_HW_MSCALER)
    if(m_mscaler_test_fd<0)  b_hw = MMP_FALSE;
#endif

#if (CSC_HW==CSC_HW_V4L2)
    if(b_hw) mmpResult = this->do_csc_hw(p_buf_vf_vpu, p_buf_vf_decoded);
#elif (CSC_HW==CSC_HW_MSCALER)
    if(b_hw) mmpResult = this->do_csc_mscaler_test(p_buf_vf_vpu, p_buf_vf_decoded);
#else
    if(b_hw) mmpResult = this->do_csc_sw(p_buf_vf_vpu, p_buf_vf_decoded);
#endif
    else mmpResult = this->do_csc_sw(p_buf_vf_vpu, p_buf_vf_decoded);

    return mmpResult;
}
    
#if (CSC_HW==CSC_HW_V4L2)
static struct mscaler_scaler_coeff scaler_coeff = {
    .apply = 0x00,
    .yh = {
        0,    0,    0,    0,    0, 1024,    0,    0,    0,    0,    0,    0,
        0,    0,    0,    6,  -24, 1020,   28,   -6,    0,    0,    0,    0,
        0,    0,    0,   12,  -47, 1015,   57,  -13,    0,    0,    0,    0,
        0,    0,    0,   17,  -68, 1007,   89,  -22,    1,    0,    0,    0,
        0,    0,    0,   21,  -86,  994,  123,  -30,    2,    0,    0,    0,
        0,    0,    0,   24, -102,  979,  159,  -39,    3,    0,    0,    0,
        0,    0,    0,   27, -115,  960,  197,  -49,    4,    0,    0,    0,
        0,    0,    0,   29, -126,  938,  236,  -59,    6,    0,    0,    0,
        0,    0,    0,   31, -135,  911,  277,  -68,    8,    0,    0,    0,
        0,    0,    0,   32, -142,  884,  319,  -78,    9,    0,    0,    0,
        0,    0,    0,   32, -147,  853,  362,  -88,   12,    0,    0,    0,
        0,    0,    0,   32, -149,  820,  405,  -98,   14,    0,    0,    0,
        0,    0,    0,   31, -150,  785,  449, -107,   16,    0,    0,    0,
        0,    0,    0,   30, -149,  747,  494, -116,   18,    0,    0,    0,
        0,    0,    0,   29, -147,  707,  538, -124,   21,    0,    0,    0,
        0,    0,    0,   27, -143,  667,  581, -131,   23,    0,    0,    0,
        0,    0,    0,   25, -138,  625,  625, -138,   25,    0,    0,    0
    },
    .yv = {
         -111,  623,  623, -111,    0,
         -104,  577,  665, -116,    2,
          -97,  532,  707, -121,    3,
          -89,  485,  747, -124,    5,
          -81,  440,  784, -126,    7,
          -73,  395,  821, -127,    8,
          -64,  350,  853, -125,   10,
          -56,  307,  884, -122,   11,
          -48,  265,  913, -117,   11,
          -40,  225,  937, -110,   12,
          -32,  186,  959, -101,   12,
          -25,  150,  978,  -90,   11,
          -19,  115,  994,  -76,   10,
          -13,   83, 1007,  -61,    8,
           -8,   53, 1016,  -43,    6,
           -3,   25, 1021,  -22,    3,
            0,    0, 1024,    0,    0
    },
    .ch = {
        0,    0,    0, 1024,    0,    0,    0,    0,
        0,    6,  -24, 1020,   28,   -6,    0,    0,
        0,   12,  -47, 1015,   57,  -13,    0,    0,
        0,   17,  -68, 1007,   89,  -22,    1,    0,
        0,   21,  -86,  994,  123,  -30,    2,    0,
        0,   24, -102,  979,  159,  -39,    3,    0,
        0,   27, -115,  960,  197,  -49,    4,    0,
        0,   29, -126,  938,  236,  -59,    6,    0,
        0,   31, -135,  911,  277,  -68,    8,    0,
        0,   32, -142,  884,  319,  -78,    9,    0,
        0,   32, -147,  853,  362,  -88,   12,    0,
        0,   32, -149,  820,  405,  -98,   14,    0,
        0,   31, -150,  785,  449, -107,   16,    0,
        0,   30, -149,  747,  494, -116,   18,    0,
        0,   29, -147,  707,  538, -124,   21,    0,
        0,   27, -143,  667,  581, -131,   23,    0,
        0,   25, -138,  625,  625, -138,   25,    0
    },

    .cv = {
         -111,  623,  623, -111,    0,
         -104,  577,  665, -116,    2,
          -97,  532,  707, -121,    3,
          -89,  485,  747, -124,    5,
          -81,  440,  784, -126,    7,
          -73,  395,  821, -127,    8,
          -64,  350,  853, -125,   10,
          -56,  307,  884, -122,   11,
          -48,  265,  913, -117,   11,
          -40,  225,  937, -110,   12,
          -32,  186,  959, -101,   12,
          -25,  150,  978,  -90,   11,
          -19,  115,  994,  -76,   10,
          -13,   83, 1007,  -61,    8,
           -8,   53, 1016,  -43,    6,
           -3,   25, 1021,  -22,    3,
            0,    0, 1024,    0,    0
    },
};

static struct mscaler_rgb_coeff rgb_coeff = {
    .use_user_coeff = 1,
    .pre_offset1 = 16,
    .pre_offset2 = 128,
    .post_offset = 16,
    .coeff0_y = 76284,
    .coeff0_v = 104595,
    .coeff1_u = 25625,
    .coeff1_v = 53281,
    .coeff2_u = 132252,
};


struct IonHelper {
    int fd;
    unsigned int flags;
    unsigned int heapMask;
    ion_user_handle_t handle[3];

    int mapFd;
    unsigned char* base;
    int pic_width;
    int pic_height;
    int length;

};

static struct IonHelper* allocContigMemory(int pic_width, int pic_height)
{
    int i;
    int mapFd;
    int ret = 0;
    struct IonHelper *ion;

    ion = (struct IonHelper *)calloc(1, sizeof(*ion));
    
    ion->flags = 0;
    //ion->heapMask = (1 << ION_DISPLAY_CARVEOUT);
    ion->heapMask = (1 << ION_HEAP_TYPE_CARVEOUT);
    ion->fd = ion_open();
    ion->pic_width = pic_width;
    ion->pic_height = pic_height;
    ion->length = pic_width*pic_height*3/2;
    
    ion_alloc(ion->fd, ion->length,  0, ion->heapMask, ion->flags, &ion->handle[0]);
    
        // 여기서 맵핑된 메모리 공간에 소스 이미지를 로딩한다.
    ion_map(ion->fd, ion->handle[0], ion->length, PROT_READ | PROT_WRITE, MAP_SHARED, 0,   (unsigned char **)&ion->base, &ion->mapFd);
    
    return ion;
}

static int freeContigMemory(struct IonHelper* ion)
{
    int i;
    
    munmap(ion->base, ion->length);
    close(ion->mapFd);
    ion_free(ion->fd, (ion_user_handle_t)ion->handle[0]);
    ion_close(ion->fd);
    
    free(ion);
    
    return 0;
}
#endif


#if 1
MMP_RESULT CMmpDecoderVideo_VpuS::do_csc_hw(class mmp_buffer_videoframe* p_buf_vf_vpu, class mmp_buffer_videoframe* p_buf_vf_decoded) {

    MScalerHandle hScaler = this->m_hScaler;
    struct MScalerImageFormat srcFormat={0}, dstFormat={0} ; 
    struct MScalerImageData srcData, dstData;
        
    srcFormat.width = p_buf_vf_vpu->get_buf_stride();
    srcFormat.height = p_buf_vf_vpu->get_buf_height();
    srcFormat.pixelformat = V4L2_PIX_FMT_YUV420;//(unsigned int)p_buf_vf_vpu->get_fourcc();
    srcFormat.crop.left=0;
    srcFormat.crop.top=0;
    srcFormat.crop.width = srcFormat.width;
    srcFormat.crop.height = srcFormat.height;


    srcData.planes = 1;
    srcData.plane[0].fd = p_buf_vf_vpu->get_buf_shared_fd();
    srcData.plane[0].length = p_buf_vf_vpu->get_buf_size();
    srcData.plane[0].base = NULL;//(void*)p_buf_vf_vpu->get_buf_vir_addr();
    srcData.acquireFenceFd = -1;
    srcData.releaseFenceFd = -1;
    srcData.priv = NULL;
    
    dstFormat.width = p_buf_vf_decoded->get_buf_stride();
    dstFormat.height = p_buf_vf_decoded->get_buf_height();
    dstFormat.pixelformat = V4L2_PIX_FMT_YUV420;//srcFormat.pixelformat;//(unsigned int)p_buf_vf_decoded->get_fourcc();
    dstFormat.crop.left=0;
    dstFormat.crop.top=0;
    dstFormat.crop.width = dstFormat.width;
    dstFormat.crop.height = dstFormat.height;

    dstData.planes = 1;
    dstData.plane[0].fd = p_buf_vf_decoded->get_buf_shared_fd();
    dstData.plane[0].length = p_buf_vf_decoded->get_buf_size();
    dstData.plane[0].base = NULL;//(void*)p_buf_vf_decoded->get_buf_vir_addr();
    dstData.acquireFenceFd = -1;
    dstData.releaseFenceFd = -1;
    dstData.priv = NULL;
    
    //MMPDEBUGMSG(1, (TEXT("SRC_VF: %d %d %d "), p_buf_vf_vpu->get_buf_stride(), p_buf_vf_vpu->get_buf_height(),  p_buf_vf_vpu->get_buf_size() ));
    //MMPDEBUGMSG(1, (TEXT("DST_VF: %d %d %d "), p_buf_vf_decoded->get_buf_stride(), p_buf_vf_decoded->get_buf_height(),  p_buf_vf_decoded->get_buf_size() ));
#if 1
    MScalerLock(hScaler, -1);
    MScalerSetImageFormat(hScaler, &srcFormat, &dstFormat);
    MScalerSetImageData(hScaler, &srcData, &dstData);
    MScalerRun(hScaler);
    MScalerWaitDone(hScaler, -1);
    MScalerStop(hScaler);
#endif

    MScalerUnlock(hScaler);

    return MMP_SUCCESS;
}

#else
MMP_RESULT CMmpDecoderVideo_VpuS::do_csc_hw(class mmp_buffer_videoframe* p_buf_vf_vpu, class mmp_buffer_videoframe* p_buf_vf_decoded) {

    MScalerHandle hScaler = this->m_hScaler;
    struct MScalerImageFormat srcFormat={0}, dstFormat={0} ; 
    struct MScalerImageData srcData, dstData;

    struct IonHelper *ion_src = allocContigMemory(1920,1080);
    struct IonHelper *ion_dst = allocContigMemory(1920,1080);
    
    p_buf_vf_decoded = m_p_buf_vf_arr[1];
        
    srcFormat.width = ion_src->pic_width;
    srcFormat.height = ion_src->pic_height;
    srcFormat.pixelformat = V4L2_PIX_FMT_NV12;//(unsigned int)p_buf_vf_vpu->get_fourcc();
    srcFormat.crop.left=0;
    srcFormat.crop.top=0;
    srcFormat.crop.width = srcFormat.width;
    srcFormat.crop.height = srcFormat.height;


    srcData.planes = 1;
    srcData.plane[0].fd = ion_src->mapFd;//p_buf_vf_vpu->get_buf_shared_fd();
    srcData.plane[0].length = ion_src->length;//p_buf_vf_vpu->get_buf_size();
    srcData.plane[0].base = ion_src->base;//NULL;//(void*)p_buf_vf_vpu->get_buf_vir_addr();
    srcData.acquireFenceFd = -1;
    srcData.releaseFenceFd = -1;
    srcData.priv = NULL;
    MMPDEBUGMSG(1, (TEXT("SRC_ION: fd=%d map_fd=%d  len=%d "), ion_src->fd, ion_src->mapFd, ion_src->length ));
    
    dstFormat.width = ion_dst->pic_width;
    dstFormat.height = ion_dst->pic_height;
    dstFormat.pixelformat = V4L2_PIX_FMT_NV12;//srcFormat.pixelformat;//(unsigned int)p_buf_vf_decoded->get_fourcc();
    dstFormat.crop.left=0;
    dstFormat.crop.top=0;
    dstFormat.crop.width = 0;//dstFormat.width;
    dstFormat.crop.height = 0;//dstFormat.height;

    dstData.planes = 1;
    dstData.plane[0].fd = ion_dst->mapFd;//p_buf_vf_decoded->get_buf_shared_fd();
    dstData.plane[0].length = ion_dst->length; //p_buf_vf_decoded->get_buf_size();
    dstData.plane[0].base = ion_dst->base;//NULL;//(void*)p_buf_vf_decoded->get_buf_vir_addr();
    dstData.acquireFenceFd = -1;
    dstData.releaseFenceFd = -1;
    dstData.priv = NULL;

    MMPDEBUGMSG(1, (TEXT("DST_ION: fd=%d map_fd=%d  len=%d "), ion_dst->fd, ion_dst->mapFd, ion_dst->length ));


#if 1
    MScalerLock(hScaler, -1);
    MScalerSetImageFormat(hScaler, &srcFormat, &dstFormat);
    MScalerSetImageData(hScaler, &srcData, &dstData);
    MScalerRun(hScaler);
    MScalerWaitDone(hScaler, -1);
    MScalerStop(hScaler);
#else
        
    MMP_U32 t1, t2, t3;
    

    int fr=0;
    int i;
    unsigned long sum = 0;

    int ret1, ret2, ret3;
    struct MScalerBufferInfo srcBufInfo, dstBufInfo;

    //MScalerSetScalerCoeff(hScaler, &scaler_coeff);
    //MScalerSetRGBCoeff(hScaler, &rgb_coeff);
    
    
    MScalerLock(hScaler, -1);

    //while(1) {
    //for(i = 0; i< 100; i++) {

        MScalerSetImageFormat(hScaler, &srcFormat, &dstFormat);    

        MScalerGetBufferInfo(hScaler, &srcBufInfo, 0);
        MScalerGetBufferInfo(hScaler, &dstBufInfo, 1);

        MMPDEBUGMSG(1, (TEXT("SRC: %d %d  "), srcBufInfo.planes,  srcBufInfo.planeSizes[0] ));
        MMPDEBUGMSG(1, (TEXT("DST: %d %d  "), dstBufInfo.planes,  dstBufInfo.planeSizes[0] ));
    
        
        MScalerSetImageData(hScaler, &srcData, &dstData);

        t1 = CMmpUtil::GetTickCount();
        ret1 = MScalerRun(hScaler);
        //ret2 = MScalerWaitDone(hScaler, -1);
        t2 = CMmpUtil::GetTickCount();
        ret2 = MScalerWaitDone(hScaler, 20);
        t3 = CMmpUtil::GetTickCount();

        ret3 = MScalerStop(hScaler);

        fr++;
        sum += t2-t1;
        //if(fr%10 == 0)
         //MMPDEBUGMSG(1, (TEXT(" t2-t1 = %d "), t2-t1 ));
        //MMPDEBUGMSG(1, (TEXT("a")));
        // CMmpUtil::Sleep(30);
    //}

    MScalerUnlock(hScaler);

    MMPDEBUGMSG(1, (TEXT(" t2-t1 = %d   t3-t2=%d   t3-t1=%d   %d %d %d "), 
        t2-t1, 
        t3-t2,
        t3-t1,
        ret1, ret2, ret3 ));
    

    MScalerClose(hScaler);
#endif

    freeContigMemory(ion_src);
    freeContigMemory(ion_dst);

    return MMP_SUCCESS;
}
#endif
    
MMP_RESULT CMmpDecoderVideo_VpuS::do_csc_sw(class mmp_buffer_videoframe* p_buf_vf_vpu, class mmp_buffer_videoframe* p_buf_vf_decoded) {

#if 1
    MMP_U8 *y_src, *u_src, *v_src;
    MMP_U8 *y_dst, *u_dst, *v_dst;
    MMP_S32 luma_size, chroma_size;

    y_src = (MMP_U8*)p_buf_vf_vpu->get_buf_vir_addr_y();
    u_src = (MMP_U8*)p_buf_vf_vpu->get_buf_vir_addr_u();
    v_src = (MMP_U8*)p_buf_vf_vpu->get_buf_vir_addr_v();

    if(this->is_android_buffer() == MMP_TRUE) {
        y_dst = (MMP_U8*)p_buf_vf_decoded->get_buf_vir_addr_y();
        u_dst = (MMP_U8*)p_buf_vf_decoded->get_buf_vir_addr_v();
        v_dst = (MMP_U8*)p_buf_vf_decoded->get_buf_vir_addr_u();
    }
    else {
        y_dst = (MMP_U8*)p_buf_vf_decoded->get_buf_vir_addr_y();
        u_dst = (MMP_U8*)p_buf_vf_decoded->get_buf_vir_addr_u();
        v_dst = (MMP_U8*)p_buf_vf_decoded->get_buf_vir_addr_v();
    }

    luma_size = p_buf_vf_decoded->get_buf_size_luma();
    chroma_size = p_buf_vf_decoded->get_buf_size_chroma();
         
/*
    printf("dst(0x%08x 0x%08x 0x%08x ) src((0x%08x 0x%08x 0x%08x ) sz(%d %d)  off(0x%08x 0x%08x ) dur=%d \n\r", 
            y_dst, u_dst, v_dst,
            y_src, u_src, v_src,
            luma_size, chroma_size,
            u_dst-y_dst, u_src-y_src,
            dec_end_tick - dec_start_tick
            );
    */

    memcpy(y_dst, y_src, luma_size);
    memcpy(u_dst, u_src, chroma_size);
    memcpy(v_dst, v_src, chroma_size);

#endif

    return MMP_SUCCESS;
}
    

/*
[drivers/media/platform/gdm-mscaler/mscaler-asic.c mscaler_hw_set_src_addr] ln=324 stride=1920 value=0x30780 
[drivers/media/platform/gdm-mscaler/mscaler-asic.c mscaler_hw_set_dst_addr] ln=365 stride=1920 value=0x30780 
[drivers/media/platform/gdm-mscaler/mscaler-asic.c mscaler_hw_set_src_format] ln=171 frame->scaler_fmt->hw_fmt_idx=3  frame->scaler_fmt->hw_swp_idx=0 value=0x30780 
[drivers/media/platform/gdm-mscaler/mscaler-asic.c mscaler_hw_set_src_size] ln=213 crop w:1920 h:1080 
[drivers/media/platform/gdm-mscaler/mscaler-asic.c mscaler_hw_set_dst_format] ln=193 frame->scaler_fmt->hw_fmt_idx=3  frame->scaler_fmt->hw_swp_idx=0 value=0x30780 
[drivers/media/platform/gdm-mscaler/mscaler-asic.c mscaler_hw_set_dst_size] ln=236 w:1920 h:1080 
[drivers/media/platform/gdm-mscaler/mscaler-asic.c mscaler_hw_set_ratio] ln=390 w:1920 h:1080 

[drivers/media/platform/gdm-mscaler/mscaler-asic.c mscaler_hw_set_enable] ln=136 enable=1 
[drivers/media/platform/gdm-mscaler/mscaler-asic.c mscaler_hw_get_state] ln=401 
[drivers/media/platform/gdm-mscaler/mscaler-asic.c mscaler_hw_clear_state] ln=415 
[drivers/media/platform/gdm-mscaler/mscaler-asic.c mscaler_hw_set_enable] ln=136 enable=0 
*/
struct mscaler_frame {
    int width;
    int height;
    unsigned int fourcc;
    unsigned int phyaddr;
};

#define MSCALER_REQUEST_BASE      0x12340000
#define MSCALER_REQUEST_SET_SRC   (MSCALER_REQUEST_BASE+0)
#define MSCALER_REQUEST_SET_DST   (MSCALER_REQUEST_BASE+1)
#define MSCALER_REQUEST_ENABLE    (MSCALER_REQUEST_BASE+2)
#define MSCALER_REQUEST_DISABLE   (MSCALER_REQUEST_BASE+3)
#define MSCALER_REQUEST_WAIT_INTR (MSCALER_REQUEST_BASE+4)

MMP_RESULT CMmpDecoderVideo_VpuS::do_csc_mscaler_test(class mmp_buffer_videoframe* p_buf_vf_vpu, class mmp_buffer_videoframe* p_buf_vf_decoded) {

    int fd;
    struct mscaler_frame frame_src, frame_dst;

    fd = m_mscaler_test_fd;//open("/dev/mscaler", O_RDWR);

    if(fd >= 0) {
        frame_src.width = p_buf_vf_vpu->get_buf_stride();
        frame_src.height = p_buf_vf_vpu->get_buf_height();
        frame_src.fourcc = V4L2_PIX_FMT_YUV420;//(unsigned int)p_buf_vf_vpu->get_fourcc();
        frame_src.phyaddr = p_buf_vf_vpu->get_buf_phy_addr();

        frame_dst.width = p_buf_vf_decoded->get_buf_stride();
        frame_dst.height = p_buf_vf_decoded->get_buf_height();
        frame_dst.fourcc = V4L2_PIX_FMT_YUV420;//(unsigned int)p_buf_vf_vpu->get_fourcc();
        frame_dst.phyaddr = p_buf_vf_decoded->get_buf_phy_addr();
                
        ioctl(fd, MSCALER_REQUEST_SET_SRC, &frame_src);
        ioctl(fd, MSCALER_REQUEST_SET_DST, &frame_dst);
        ioctl(fd, MSCALER_REQUEST_ENABLE, NULL);
        ioctl(fd, MSCALER_REQUEST_WAIT_INTR, NULL);
        ioctl(fd, MSCALER_REQUEST_DISABLE, NULL);
        
    }

    //close(fd);

    return MMP_SUCCESS;
}


MMP_RESULT CMmpDecoderVideo_VpuS::DecodeAu(class mmp_buffer_videostream* p_buf_vs, class mmp_buffer_videoframe* p_buf_vf) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    class mmp_buffer_videoframe* p_buf_vf_vpu = NULL;
    MMP_TICKS last_pts = 0;
    MMP_U32 last_dur = 0;
    MMP_U32 dec_start_tick, dec_end_tick;
         
    if(p_buf_vf!=NULL) p_buf_vf->set_coding_result(mmp_buffer_media::FAIL);
           
    if(m_bDecodeDSI == MMP_FALSE) {
        mmpResult = CLASS_DECODER_VPU::DecodeDSI(p_buf_vs);
        p_buf_vs->inc_stream_offset(p_buf_vs->get_stream_real_size());
        if(mmpResult == MMP_SUCCESS) {
            m_bDecodeDSI = MMP_TRUE;
            if( (this->get_pic_width() != CLASS_DECODER_VPU::GetVpuPicWidth())  
                ||  (this->get_pic_height() != CLASS_DECODER_VPU::GetVpuPicHeight()) ) {
		            if(p_buf_vf!=NULL) p_buf_vf->set_coding_result(mmp_buffer_media::RECONFIG);
                    this->cfg_buf_vf(CLASS_DECODER_VPU::GetVpuPicWidth(), CLASS_DECODER_VPU::GetVpuPicHeight());
                    return MMP_SUCCESS;
            }
        }
        
        return MMP_SUCCESS;
    }

    
    if((p_buf_vs->get_flag()&mmp_buffer_media::FLAG_CONFIGDATA) != 0) {
        if(p_buf_vf!=NULL) p_buf_vf->set_coding_result(mmp_buffer_media::RECONFIG);
        return MMP_SUCCESS;
    }

    /* Init CSC Info */
    p_buf_vf_vpu = m_p_buf_vf_vpu_last;
    last_pts = m_dec_pts_last;
    last_dur = m_dec_dur_last;
    
    m_p_buf_vf_vpu_last = NULL;
    m_dec_pts_last = 0;
    m_dec_dur_last = 0;
    
    dec_start_tick = CMmpUtil::GetTickCount();
    this->DecodeAu_PinEnd_Run(p_buf_vs);
    
    /* CSC */
    if( (p_buf_vf_vpu != NULL) && (p_buf_vf != NULL) )  {
    
            this->do_csc(p_buf_vf_vpu, p_buf_vf);
            
            p_buf_vf->set_coding_dur(last_dur);
            p_buf_vf->set_pts(last_pts);
            p_buf_vf->clear_own();
            p_buf_vf->set_coding_result(mmp_buffer_media::SUCCESS);
    }
    
    
    this->DecodeAu_PinEnd_WaitDone(&m_p_buf_vf_vpu_last);
    dec_end_tick = CMmpUtil::GetTickCount();

    p_buf_vs->inc_stream_offset(p_buf_vs->get_stream_real_size());

    m_dec_dur_last = dec_end_tick - dec_start_tick;
    m_dec_pts_last = p_buf_vs->get_pts();

    
    /* check reconfig */
    if((this->get_pic_width() != CLASS_DECODER_VPU::GetVpuPicWidth())  ||  (this->get_pic_height() != CLASS_DECODER_VPU::GetVpuPicHeight())  ) {
        if(p_buf_vf!=NULL) p_buf_vf->set_coding_result(mmp_buffer_media::RECONFIG);
        this->cfg_buf_vf(CLASS_DECODER_VPU::GetVpuPicWidth(), CLASS_DECODER_VPU::GetVpuPicHeight());
    }
    
	return MMP_SUCCESS;
}


