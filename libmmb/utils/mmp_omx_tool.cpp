#include "MmpDefine.h"
#include "mmp_omx_tool.h"
#include <system/graphics.h>

/* OMX Component Supprot */
struct omx_component_desc
{
	enum MMP_FOURCC fourcc;
	MMP_CHAR cComponentName[32];
	MMP_CHAR mime[32];
	MMP_CHAR role[32];
	MMP_BOOL  bEncoder;
    enum MMP_MEDIATYPE mt;

    MMP_U32 omx_codingtype;
};

static struct omx_component_desc s_omx_comp_desc[]= {

    /* Video Decoder in VPU */
	{ MMP_FOURCC_VIDEO_H264,  OMX_COMPNAME_VIDEO_H264_DECODER,  OMX_MIMETYPE_VIDEO_H264, OMX_ROLENAME_VIDEO_DECODER_H264, MMP_FALSE , MMP_MEDIATYPE_VIDEO, (MMP_U32)OMX_VIDEO_CodingAVC },
    
    { MMP_FOURCC_VIDEO_MPEG4, OMX_COMPNAME_VIDEO_MPEG4_DECODER, OMX_MIMETYPE_VIDEO_MPEG4, OMX_ROLENAME_VIDEO_DECODER_MPEG4, MMP_FALSE, MMP_MEDIATYPE_VIDEO, (MMP_U32)OMX_VIDEO_CodingMPEG4 },
    { MMP_FOURCC_VIDEO_MSMPEG4V3, OMX_COMPNAME_VIDEO_MSMPEG4V3_DECODER, OMX_MIMETYPE_VIDEO_MSMPEG4V3, OMX_ROLENAME_VIDEO_DECODER_MSMPEG4V3, MMP_FALSE, MMP_MEDIATYPE_VIDEO, (MMP_U32)OMX_VIDEO_CodingMSMPEG4V3 },
    { MMP_FOURCC_VIDEO_MPEG2, OMX_COMPNAME_VIDEO_MPEG2_DECODER, OMX_MIMETYPE_VIDEO_MPEG2, OMX_ROLENAME_VIDEO_DECODER_MPEG2, MMP_FALSE, MMP_MEDIATYPE_VIDEO, (MMP_U32)OMX_VIDEO_CodingMPEG2 },
    
    { MMP_FOURCC_VIDEO_WMV3,   OMX_COMPNAME_VIDEO_WMV3_DECODER,   OMX_MIMETYPE_VIDEO_WMV3, OMX_ROLENAME_VIDEO_DECODER_WMV3, MMP_FALSE , MMP_MEDIATYPE_VIDEO, (MMP_U32)OMX_VIDEO_CodingWMV },
    { MMP_FOURCC_VIDEO_WVC1,   OMX_COMPNAME_VIDEO_WVC1_DECODER,   OMX_MIMETYPE_VIDEO_WMV3, OMX_ROLENAME_VIDEO_DECODER_WMV3, MMP_FALSE , MMP_MEDIATYPE_VIDEO, (MMP_U32)OMX_VIDEO_CodingWMV },
    
    { MMP_FOURCC_VIDEO_RV,    OMX_COMPNAME_VIDEO_RV_DECODER,  OMX_MIMETYPE_VIDEO_RV, OMX_ROLENAME_VIDEO_DECODER_RV9, MMP_FALSE , MMP_MEDIATYPE_VIDEO, (MMP_U32)OMX_VIDEO_CodingRV},
    
    { MMP_FOURCC_VIDEO_VP80,  OMX_COMPNAME_VIDEO_VP8_DECODER,  OMX_MIMETYPE_VIDEO_VP8, OMX_ROLENAME_VIDEO_DECODER_VP8, MMP_FALSE, MMP_MEDIATYPE_VIDEO, (MMP_U32)OMX_VIDEO_CodingVP8},
    
    { MMP_FOURCC_VIDEO_H263,  OMX_COMPNAME_VIDEO_H263_DECODER,  OMX_MIMETYPE_VIDEO_H263, OMX_ROLENAME_VIDEO_DECODER_H263, MMP_FALSE, MMP_MEDIATYPE_VIDEO, (MMP_U32)OMX_VIDEO_CodingH263},
    { MMP_FOURCC_VIDEO_FLV1,  OMX_COMPNAME_VIDEO_FLV1_DECODER,  OMX_MIMETYPE_VIDEO_FLV1, OMX_ROLENAME_VIDEO_DECODER_FLV1, MMP_FALSE , MMP_MEDIATYPE_VIDEO, (MMP_U32)OMX_VIDEO_CodingFLV1},
    { MMP_FOURCC_VIDEO_THEORA,  OMX_COMPNAME_VIDEO_THEORA_DECODER,  OMX_MIMETYPE_VIDEO_THEORA, OMX_ROLENAME_VIDEO_DECODER_THEORA, MMP_FALSE , MMP_MEDIATYPE_VIDEO, (MMP_U32)OMX_VIDEO_CodingTHEORA},
    
    /* Video Decoder in Ffmpeg */
    { MMP_FOURCC_VIDEO_FFMPEG,  OMX_COMPNAME_VIDEO_FFMPEG_DECODER,  OMX_MIMETYPE_VIDEO_FFMPEG, OMX_ROLENAME_VIDEO_DECODER_FFMPEG, MMP_FALSE , MMP_MEDIATYPE_VIDEO, (MMP_U32)OMX_VIDEO_CodingFFMpeg},

    /* Video Encoder */
    { MMP_FOURCC_VIDEO_H264, OMX_COMPNAME_VIDEO_H264_ENCODER, OMX_MIMETYPE_VIDEO_H264, OMX_ROLENAME_VIDEO_ENCODER_H264, MMP_TRUE, MMP_MEDIATYPE_VIDEO, (MMP_U32)OMX_VIDEO_CodingAVC},
    { MMP_FOURCC_VIDEO_MPEG4, OMX_COMPNAME_VIDEO_MPEG4_ENCODER, OMX_MIMETYPE_VIDEO_MPEG4, OMX_ROLENAME_VIDEO_ENCODER_MPEG4, MMP_TRUE , MMP_MEDIATYPE_VIDEO, (MMP_U32)OMX_VIDEO_CodingMPEG4},
    { MMP_FOURCC_VIDEO_H263, OMX_COMPNAME_VIDEO_H263_ENCODER, OMX_MIMETYPE_VIDEO_H263, OMX_ROLENAME_VIDEO_ENCODER_H263, MMP_TRUE, MMP_MEDIATYPE_VIDEO, (MMP_U32)OMX_VIDEO_CodingH263},
    
    /* Audio Decoder */
    { MMP_FOURCC_AUDIO_FFMPEG, OMX_COMPNAME_AUDIO_FFMPEG_DECODER, OMX_MIMETYPE_AUDIO_FFMPEG, OMX_ROLENAME_AUDIO_DECODER_FFMPEG, MMP_FALSE, MMP_MEDIATYPE_AUDIO, (MMP_U32)OMX_AUDIO_CodingFFMpeg},
    
    { MMP_FOURCC_AUDIO_MP3, OMX_COMPNAME_AUDIO_MP3_DECODER, OMX_MIMETYPE_AUDIO_MP3, OMX_ROLENAME_AUDIO_DECODER_MP3, MMP_FALSE, MMP_MEDIATYPE_AUDIO, (MMP_U32)OMX_AUDIO_CodingMP3},
    { MMP_FOURCC_AUDIO_AAC,  OMX_COMPNAME_AUDIO_AAC_DECODER, OMX_MIMETYPE_AUDIO_AAC, OMX_ROLENAME_AUDIO_DECODER_AAC, MMP_FALSE , MMP_MEDIATYPE_AUDIO, (MMP_U32)OMX_AUDIO_CodingAAC},
    { MMP_FOURCC_AUDIO_WMA2,  OMX_COMPNAME_AUDIO_WMAV2_DECODER, OMX_MIMETYPE_AUDIO_WMAV2, OMX_ROLENAME_AUDIO_DECODER_WMA2, MMP_FALSE , MMP_MEDIATYPE_AUDIO,  (MMP_U32)OMX_AUDIO_CodingWMA},

    { MMP_FOURCC_AUDIO_FLAC,  OMX_COMPNAME_AUDIO_FLAC_DECODER, OMX_MIMETYPE_AUDIO_FLAC, OMX_ROLENAME_AUDIO_DECODER_FLAC, MMP_FALSE , MMP_MEDIATYPE_AUDIO,  (MMP_U32)OMX_AUDIO_CodingFLAC},
    { MMP_FOURCC_AUDIO_FLAC,  OMX_COMPNAME_AUDIO_FLAC_MME_DECODER, OMX_MIMETYPE_AUDIO_FLAC_MME, OMX_ROLENAME_AUDIO_DECODER_FLAC, MMP_FALSE , MMP_MEDIATYPE_AUDIO,  (MMP_U32)OMX_AUDIO_CodingFLAC},
    
};

MMP_S32 mmp_get_omx_component_count(void) {
    
    MMP_S32 cnt;
    cnt = sizeof(s_omx_comp_desc)/sizeof(s_omx_comp_desc[0]);
    return cnt;
}

const MMP_CHAR* mmp_get_omx_component_name(MMP_S32 idx) {

    MMP_CHAR *pstr = NULL;
    MMP_S32 cnt;
    cnt = sizeof(s_omx_comp_desc)/sizeof(s_omx_comp_desc[0]);
    
    if(idx < cnt) {
        pstr = s_omx_comp_desc[idx].cComponentName;
    }

    return (const MMP_CHAR*)pstr;
}

const MMP_CHAR* mmp_get_omx_component_mime(MMP_S32 idx) {

    MMP_CHAR *pstr = NULL;
    MMP_S32 cnt;
    cnt = sizeof(s_omx_comp_desc)/sizeof(s_omx_comp_desc[0]);
    
    if(idx < cnt) {
        pstr = s_omx_comp_desc[idx].mime;
    }

    return (const MMP_CHAR*)pstr;
}

enum MMP_MEDIATYPE mmp_get_omx_component_get_media_type(MMP_S32 idx) {

    enum MMP_MEDIATYPE mt = MMP_MEDIATYPE_UNKNOWN;
    MMP_S32 cnt;
    cnt = sizeof(s_omx_comp_desc)/sizeof(s_omx_comp_desc[0]);
    
    if(idx < cnt) {
        mt = s_omx_comp_desc[idx].mt;
    }

    return mt;
}

MMP_BOOL mmp_get_omx_component_is_encoder(MMP_S32 idx) {

    MMP_BOOL bflag = MMP_FALSE;
    MMP_S32 cnt;
    cnt = sizeof(s_omx_comp_desc)/sizeof(s_omx_comp_desc[0]);
    
    if(idx < cnt) {
        bflag = s_omx_comp_desc[idx].bEncoder;
    }

    return bflag;
}

enum MMP_FOURCC mmp_get_omx_component_get_fourcc(MMP_S32 idx) {

    enum MMP_FOURCC fourcc = MMP_FOURCC_VIDEO_UNKNOWN;
    MMP_S32 cnt;
    cnt = sizeof(s_omx_comp_desc)/sizeof(s_omx_comp_desc[0]);
    
    if(idx < cnt) {
        fourcc = s_omx_comp_desc[idx].fourcc;
    }

    return fourcc;
}

MMP_U32 mmp_get_omx_component_omx_coding_type(MMP_S32 idx) {

    MMP_U32 codingtype = 0;
    MMP_S32 cnt;
    cnt = sizeof(s_omx_comp_desc)/sizeof(s_omx_comp_desc[0]);
    
    if(idx < cnt) {
        codingtype = s_omx_comp_desc[idx].omx_codingtype;
    }

    return codingtype;

}

/* OMX Format Supprot */
struct omx_format_desc
{
	enum MMP_FOURCC fourcc;
    OMX_COLOR_FORMATTYPE omxfmt;
};

//#ifdef WIN32
//#define HAL_PIXEL_FORMAT_YCbCr_420_888 0x23
//#endif

static struct omx_format_desc s_omx_format_desc[]= {
    /* 2017,2,10  I will not support FLEXIBLE COLOR becuase it make more difficult */
    //{MMP_FOURCC_IMAGE_YVU420_FLEXIBLE, (OMX_COLOR_FORMATTYPE)HAL_PIXEL_FORMAT_YCbCr_420_888}, 
    {MMP_FOURCC_IMAGE_YUV420, OMX_COLOR_FormatYUV420Planar},
    {MMP_FOURCC_IMAGE_NV12, OMX_COLOR_FormatYUV420SemiPlanar},
    {MMP_FOURCC_IMAGE_ANDROID_OPAQUE, OMX_COLOR_FormatAndroidOpaque},
};

OMX_COLOR_FORMATTYPE mmp_get_omxformat_from_fourcc(enum MMP_FOURCC fourcc) {

    MMP_S32 i, cnt;
    OMX_COLOR_FORMATTYPE omxfmt = OMX_COLOR_FormatUnused;
    cnt = sizeof(s_omx_format_desc)/sizeof(s_omx_format_desc[0]);
    
    for(i = 0; i < cnt; i++) {

        if(s_omx_format_desc[i].fourcc == fourcc) {
            omxfmt = s_omx_format_desc[i].omxfmt;
            break;
        }
    }

    return omxfmt;
}

enum MMP_FOURCC mmp_get_fourcc_from_omxformat(OMX_COLOR_FORMATTYPE omxfmt) {

    MMP_S32 i, cnt;
    enum MMP_FOURCC fourcc = MMP_FOURCC_UNKNOWN;
    cnt = sizeof(s_omx_format_desc)/sizeof(s_omx_format_desc[0]);
    
    for(i = 0; i < cnt; i++) {

        if(s_omx_format_desc[i].omxfmt == omxfmt) {
            fourcc = s_omx_format_desc[i].fourcc;
            break;
        }
    }

    return fourcc;
}



/* H264 Supprot Profile  VPU */

struct omx_h264_decoder_profile_desc
{
	OMX_VIDEO_AVCPROFILETYPE profile;
    OMX_VIDEO_AVCLEVELTYPE level;
};

static struct omx_h264_decoder_profile_desc s_omx_h264_dec_profile_desc[]=
{
    { OMX_VIDEO_AVCProfileBaseline,  OMX_VIDEO_AVCLevel42},
    { OMX_VIDEO_AVCProfileMain,  OMX_VIDEO_AVCLevel42},
    { OMX_VIDEO_AVCProfileHigh,  OMX_VIDEO_AVCLevel42},
};


struct omx_mpeg4_decoder_profile_desc
{
	OMX_VIDEO_MPEG4PROFILETYPE profile;
    OMX_VIDEO_MPEG4LEVELTYPE level;
};

static struct omx_mpeg4_decoder_profile_desc s_omx_mpeg4_dec_profile_desc[]=
{
    { OMX_VIDEO_MPEG4ProfileSimple,  OMX_VIDEO_MPEG4Level3},
    { OMX_VIDEO_MPEG4ProfileSimple,  OMX_VIDEO_MPEG4Level5},
    { OMX_VIDEO_MPEG4ProfileAdvancedSimple,  OMX_VIDEO_MPEG4Level3},
    { OMX_VIDEO_MPEG4ProfileAdvancedSimple,  OMX_VIDEO_MPEG4Level5},
};


struct omx_h263_decoder_profile_desc
{
	OMX_VIDEO_H263PROFILETYPE profile;
    OMX_VIDEO_H263LEVELTYPE level;
};

static struct omx_h263_decoder_profile_desc s_omx_h263_dec_profile_desc[]=
{
    
    { OMX_VIDEO_H263ProfileBaseline,  OMX_VIDEO_H263Level30},
    { OMX_VIDEO_H263ProfileBaseline,  OMX_VIDEO_H263Level45},
    { OMX_VIDEO_H263ProfileISWV2,  OMX_VIDEO_H263Level30},
    { OMX_VIDEO_H263ProfileISWV2,  OMX_VIDEO_H263Level45},
    { OMX_VIDEO_H263ProfileISWV3,  OMX_VIDEO_H263Level70},
};

OMX_ERRORTYPE mmp_get_omx_vidoe_decoder_prifle_level(enum MMP_FOURCC fourcc, OMX_VIDEO_PARAM_PROFILELEVELTYPE *profileLevel) {

    OMX_ERRORTYPE omxResult = OMX_ErrorUndefined;
    MMP_S32 idx, cnt;

    switch(fourcc) {
        
        case MMP_FOURCC_VIDEO_H264:

            idx = (MMP_S32)profileLevel->nProfileIndex; 
            cnt = sizeof(s_omx_h264_dec_profile_desc)/sizeof(s_omx_h264_dec_profile_desc[0]);
            if(idx < cnt) {
                
                profileLevel->eProfile = s_omx_h264_dec_profile_desc[idx].profile;
                profileLevel->eLevel = s_omx_h264_dec_profile_desc[idx].level;
                omxResult = OMX_ErrorNone;
            }

            break;

        case MMP_FOURCC_VIDEO_MPEG4:

            idx = (MMP_S32)profileLevel->nProfileIndex; 
            cnt = sizeof(s_omx_mpeg4_dec_profile_desc)/sizeof(s_omx_mpeg4_dec_profile_desc[0]);
            if(idx < cnt) {
                
                profileLevel->eProfile = s_omx_mpeg4_dec_profile_desc[idx].profile;
                profileLevel->eLevel = s_omx_mpeg4_dec_profile_desc[idx].level;
                omxResult = OMX_ErrorNone;
            }

            break;

        case MMP_FOURCC_VIDEO_H263:
            idx = (MMP_S32)profileLevel->nProfileIndex; 
            cnt = sizeof(s_omx_h263_dec_profile_desc)/sizeof(s_omx_h263_dec_profile_desc[0]);
            if(idx < cnt) {
                
                profileLevel->eProfile = s_omx_h263_dec_profile_desc[idx].profile;
                profileLevel->eLevel = s_omx_h263_dec_profile_desc[idx].level;
                omxResult = OMX_ErrorNone;
            }

            break;
    }

    return omxResult;
}