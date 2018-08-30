/*
 *
 *  Copyright (C) 2010-2011 TokiPlayer Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MMPENCODERVIDEO_HPP__
#define MMPENCODERVIDEO_HPP__

#include "TemplateList.hpp"
#include "MmpEncoder.hpp"

struct mmp_enc_video_frame {
    MMP_U8* pData;
    MMP_U32 nDataSize;
    MMP_U32 nFlag;
};

class CMmpEncoderVideo : public CMmpEncoder
{
friend class CMmpEncoder;

public:
    enum ENC_TYPE {
        ENC_DEFAULT=0,
        ENC_SW=0x100,
        ENC_HW=0x101,
        ENC_DUMMY=0x102
    };

    struct create_config {
        
        enum ENC_TYPE enc_type;

        /* basic info */
        enum MMP_FOURCC fourcc_in;
        enum MMP_FOURCC fourcc_out;
                
        /* Inport info */
        MMP_S32 pic_width;
        MMP_S32 pic_height;
        MMP_S32 fr; /* nFrameRate; */
    
        /* Outport info */
        MMP_S32 ip; /*nIDRPeriod;*/
        MMP_S32 br; /*bitrate */
                
        union {
            MMP_VIDEO_PARAM_H264TYPE h264;
            MMP_VIDEO_PARAM_H263TYPE h263;
            MMP_VIDEO_PARAM_MPEG4TYPE mpeg4;
        }codec;
    };

    static MMP_RESULT gen_create_config(enum MMP_FOURCC fourcc_in,
                                        enum MMP_FOURCC fourcc_out,
                                        MMP_S32 pic_width, MMP_S32 pic_height,
                                        MMP_S32 fr /*framerate*/, MMP_S32 ip /*idr_period*/, MMP_S32 br /*bitrate*/,
                                        MMP_S32 sw_codec_use,
                                        MMP_OUT struct CMmpEncoderVideo::create_config *p_create_config
                                        ); 

    static CMmpEncoderVideo* CreateObject(enum ENC_TYPE enc_type,
                                          enum MMP_FOURCC fourcc_in,
                                          enum MMP_FOURCC fourcc_out,
                                          MMP_S32 pic_width, MMP_S32 pic_height,
                                          MMP_S32 fr /*framerate*/, MMP_S32 ip /*idr_period*/, MMP_S32 br /*bitrate*/); 

    static CMmpEncoderVideo* CreateObject(struct CMmpEncoderVideo::create_config *p_create_config);

private:
    struct CMmpEncoderVideo::create_config m_create_config;
    
    MMP_U32 m_mon_before_tick;
    MMP_U32 m_mon_fps_sum;
    MMP_U32 m_mon_bitrate_sum;
    MMP_U32 m_mon_pre_dur_sum;
    MMP_U32 m_mon_enc_dur_sum;

protected:
    enum MMP_FOURCC m_fourcc_last_input;
    MMP_BOOL m_DEBUG_MMEEnc;
    
protected:
    CMmpEncoderVideo(struct CMmpEncoderVideo::create_config *p_create_config, MMP_CHAR* class_name);
    virtual ~CMmpEncoderVideo();

    virtual MMP_RESULT Open();
	virtual MMP_RESULT Open(MMP_U8* pStream, MMP_U32 nStreamSize) {return MMP_FAILURE;}
    virtual MMP_RESULT Close();
            
public:
    virtual MMP_RESULT EncodeAu(class mmp_buffer_videoframe* p_buf_vf, class mmp_buffer_videostream* p_buf_vs) = 0;
    	
    inline MMP_S32 get_pic_width() { return m_create_config.pic_width; }
    inline MMP_S32 get_pic_height() { return m_create_config.pic_height; }
    virtual void set_pic_width(MMP_S32 w) { m_create_config.pic_width = w; }
    virtual void set_pic_height(MMP_S32 h) { m_create_config.pic_height = h; }
    
    inline MMP_S32 get_br() { return m_create_config.br; }
    inline MMP_S32 get_fr() { return m_create_config.fr; }
    inline MMP_S32 get_ip() { return m_create_config.ip; }
    inline void set_br(MMP_S32 br) { m_create_config.br = br; }
    
    void print_status(class mmp_buffer_videostream* p_buf_vs);

protected:
    MMP_RESULT EncodedFrameQueue_AddFrame(MMP_U8* pBuffer, MMP_U32 nBufSize, MMP_U32 nFlag);
    MMP_RESULT EncodedFrameQueue_AddFrameWithConfig_Mpeg4(MMP_U8* pBuffer, MMP_U32 nBufSize, MMP_U32 nFlag);
};

#endif

