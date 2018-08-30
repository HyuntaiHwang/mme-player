#------------------------------------------------------------------------------
#
# Odyssesus Test Platform Makefiles
#
# AUTHOR	: hthwang@anapass.com
# DATE		: 2014-08-07
#
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# DO NOT EDIT UNDER THIS LINE
ifndef	BASEDIR
BASEDIR	:= $(shell pwd)/../..
endif
#------------------------------------------------------------------------------

TARGET			:= libmme.so

#MME_PLATFORM := __PLATFORM_ODY_FPGA
MME_PLATFORM := __PLATFORM_TIZEN

MME_VPU_DEFINE := __VPU_PLATFORM_MME
MME_JPU_DEFINE := __JPU_PLATFORM_MME

MME_TOP = .
MMB_TOP = $(MME_TOP)/../libmmb
FFMPEG_TOP = $(MME_TOP)/pre-built/ffmpeg/ffmpeg-2.1.3
JPEGLIB_TOP = $(MME_TOP)/pre-built/jpeglib
MME_VPULIB_TOP=$(MME_TOP)/decoder/cnm-coda960-ref_sw_pkg-v3.3.0

MME_SRC_API = \
   $(MME_TOP)/api/mme_c_api_util.cpp \
   $(MME_TOP)/api/mme_c_api_video_encoder.cpp \
   $(MME_TOP)/api/mme_c_api_muxer.cpp \
   $(MME_TOP)/api/mme_c_api_demuxer.cpp \
   $(MME_TOP)/api/mme_c_api_jpeg.cpp \
   $(MME_TOP)/api/mme_c_api_imagetool.cpp \

MME_SRC_UTILS = \
    $(MME_TOP)/utils/MmpUtil.cpp \
    $(MME_TOP)/utils/MmpUtil_Jpeg.cpp \
    $(MME_TOP)/utils/MmpImageTool.cpp \
    $(MME_TOP)/utils/linux_tool/sysinfo.c \
    $(MME_TOP)/utils/linux_tool/version.c \
    $(MME_TOP)/utils/MmpBitExtractor.cpp\
    $(MME_TOP)/utils/MmpMpeg4Tool.cpp\
    $(MME_TOP)/utils/MmpH264Tool.cpp\
         
MME_SRC_OAL = \
    $(MME_TOP)/oal/mmp_msg_proc.cpp \
    $(MME_TOP)/oal/mmp_lock.cpp \
    $(MME_TOP)/oal/mmp_oal_cond.cpp \
    $(MME_TOP)/oal/mmp_oal_cond_linux.cpp \
    $(MME_TOP)/oal/mmp_oal_mutex.cpp \
    $(MME_TOP)/oal/mmp_oal_mutex_linux.cpp \
    $(MME_TOP)/oal/mmp_simple_heap.cpp \
    $(MME_TOP)/oal/mmp_oal_task.cpp \
    $(MME_TOP)/oal/mmp_oal_task_linux.cpp \
    $(MME_TOP)/oal/mmp_oal_shm.cpp \
    $(MME_TOP)/oal/mmp_oal_shm_heap.cpp \
    
MME_SRC_BUFFER = \
    $(MME_TOP)/decoder/buffer/mmp_buffer.cpp \
    $(MME_TOP)/decoder/buffer/mmp_buffer_ion.cpp \
    $(MME_TOP)/decoder/buffer/mmp_buffer_ion_attach.cpp \
    $(MME_TOP)/decoder/buffer/mmp_buffer_heap.cpp \
    $(MME_TOP)/decoder/buffer/mmp_buffer_heap_attach.cpp \
    $(MME_TOP)/decoder/buffer/mmp_buffer_anw43_attach.cpp \
    $(MME_TOP)/decoder/buffer/mmp_buffer_mgr.cpp \
    $(MME_TOP)/decoder/buffer/mmp_buffer_mgr_ex1.cpp \
    $(MME_TOP)/decoder/buffer/mmp_buffer_media.cpp \
    $(MME_TOP)/decoder/buffer/mmp_buffer_videoframe.cpp \
    $(MME_TOP)/decoder/buffer/mmp_buffer_videostream.cpp \
    $(MME_TOP)/decoder/buffer/mmp_buffer_imageframe.cpp \
    $(MME_TOP)/decoder/buffer/mmp_buffer_imagestream.cpp \
	$(MME_TOP)/decoder/buffer/mmp_buffer_audioframe.cpp \
    $(MME_TOP)/decoder/buffer/mmp_buffer_audiostream.cpp \
	
MME_SRC_HW_VPU = \
    $(MME_TOP)/decoder/hw-vpu/mmp_vpu_if.cpp \
    $(MME_TOP)/decoder/hw-vpu/mmp_vpu_if_ana.cpp \
    $(MME_TOP)/decoder/hw-vpu/mmp_vpu_if_cnm.cpp \
    $(MME_TOP)/decoder/hw-vpu/mmp_vpu_dev.cpp \
    $(MME_TOP)/decoder/hw-vpu/mmp_vpu_dev_shm.cpp \

MME_SRC_SINGLETON = \
   $(MME_TOP)/decoder/singleton/mmp_singleton_mgr.cpp\
   $(MME_TOP)/decoder/singleton/mmp_env_mgr.cpp\

MME_SRC_CSC = \
   $(MME_TOP)/decoder/csc/color_space_convertor.c \
   $(MME_TOP)/decoder/csc/csc_interleave_memcpy_neon.s \
   $(MME_TOP)/decoder/csc/csc_linear_to_tiled_crop_neon.S \
   $(MME_TOP)/decoder/csc/csc_linear_to_tiled_interleave_crop_neon.S\
   $(MME_TOP)/decoder/csc/csc_tiled_to_linear_crop_neon.S\
   $(MME_TOP)/decoder/csc/csc_tiled_to_linear_deinterleave_crop_neon.S\

MME_SRC_DECODER = \
   $(MME_TOP)/decoder/MmpDecoder.cpp \
   $(MME_TOP)/decoder/MmpDecoderAudio.cpp \
   $(MME_TOP)/decoder/MmpDecoderAudio_Ffmpeg.cpp \
   $(MME_TOP)/decoder/MmpDecoderVideo.cpp \
   $(MME_TOP)/decoder/MmpDecoderFfmpeg.cpp \
   $(MME_TOP)/decoder/MmpDecoderVideo_Ffmpeg.cpp \
   $(MME_TOP)/decoder/MmpDecoderVideo_V4L2_MFC.cpp \
   $(MME_TOP)/decoder/MmpDecoderImage.cpp \
   $(MME_TOP)/decoder/MmpDecoderImage_JpegLib.cpp \
   $(MME_TOP)/../libffmpeg/ffmpeg_c_api.c \
  
  
MME_SRC_ENCODER = \
   $(MME_TOP)/encoder/MmpEncoder.cpp \
   $(MME_TOP)/encoder/MmpEncoderFfmpeg.cpp \
   $(MME_TOP)/encoder/MmpEncoderVideo.cpp \
   $(MME_TOP)/encoder/MmpEncoderVideo_Ffmpeg.cpp \
   
MME_SRC_VPULIB = \
   $(MME_VPULIB_TOP)/vpuapi/vpuapi.c \
   $(MME_VPULIB_TOP)/vpuapi/vpuapi_mme.c \
   $(MME_VPULIB_TOP)/vpuapi/vpuapifunc.c\
   $(MME_VPULIB_TOP)/vdi/mm.c\
   $(MME_VPULIB_TOP)/vdi/linux/vdi.c\
   $(MME_VPULIB_TOP)/vdi/linux/vdi_osal.c\
   $(MME_VPULIB_TOP)/src/vpuhelper.c\
   $(MME_VPULIB_TOP)/src/vpuio.c\

MME_SRC_DEMUXER = \
   $(MME_TOP)/demuxer/MmpDemuxerBuffer.cpp \
   $(MME_TOP)/demuxer/MmpDemuxer.cpp \
   $(MME_TOP)/demuxer/MmpDemuxer_Ffmpeg.cpp \
   $(MME_TOP)/demuxer/MmpDemuxer_ammf.cpp \
  
MME_SRC_MUXER = \
   $(MME_TOP)/muxer/MmpMuxer.cpp \
   $(MME_TOP)/muxer/MmpMuxer_ammf.cpp \
   $(MME_TOP)/muxer/MmpMuxer_Ffmpeg.cpp \
   $(MME_TOP)/muxer/MmpMuxer_rawstream.cpp \

MME_SRC_PLAYER = \
   $(MME_TOP)/player/MmpPlayerService.cpp \
   $(MME_TOP)/player/MmpPlayer.cpp \
   $(MME_TOP)/player/MmpPlayerVideoEx1.cpp \
   $(MME_TOP)/player/MmpPlayerAudio.cpp \
   $(MME_TOP)/player/MmpPlayerAVEx4.cpp \
   $(MME_TOP)/player/MmpPlayerTONE.cpp \
   $(MME_TOP)/player/MmpPlayerPCM.cpp \
   $(MME_TOP)/player/MmpPlayerYUV.cpp \
   $(MME_TOP)/player/MmpPlayerJPEG.cpp \
   $(MME_TOP)/player/MmpPlayerStagefright.cpp \

MME_SRC_RENDERER = \
   $(MME_TOP)/renderer/MmpRenderer.cpp \
   $(MME_TOP)/renderer/MmpRenderer_YUVWriter.cpp \
   $(MME_TOP)/renderer/MmpRenderer_Dummy.cpp \
   $(MME_TOP)/renderer/MmpRenderer_OdyClientEx2.cpp \
   $(MME_TOP)/renderer/MmpRenderer_ALSA.cpp \
   

SRCS = $(MME_SRC_API) $(MME_SRC_BUFFER) $(MME_SRC_SINGLETON) $(MME_SRC_CSC)\
       $(MME_SRC_DECODER) $(MME_SRC_ENCODER) \
	   $(MME_SRC_DEMUXER) $(MME_SRC_MUXER) $(MME_SRC_RENDERER)\
	   $(MME_SRC_PLAYER) \

OBJS   += $(addsuffix .o, $(basename $(SRCS)) )

LOCAL_CFLAGS := -O2 -DLINUX -D_GNU_SOURCE -Wall -shared -fPIC -D$(MME_PLATFORM) -D$(MME_VPU_DEFINE) -D$(MME_JPU_DEFINE) -DJPU_PLATFORM_V4L2_ENABLE=1 \
            -I$(BASEDIR)/include\
            -I$(MME_TOP)/../libion \
            -I$(MME_TOP)/../libmsgio\
            -I$(MME_TOP)/../libhwc\
            -I$(MME_TOP)/../libv4l2_api/inc\
            -I$(FFMPEG_TOP) -I$(JPEGLIB_TOP)\
            -I$(MME_VPULIB_TOP)/theoraparser/include\
            -I$(MME_VPULIB_TOP)/vpuapi -I$(MME_VPULIB_TOP)/include -I$(MME_VPULIB_TOP)/vdi/linux/driver  -I$(MME_VPULIB_TOP)/vdi\
            -I$(MMB_TOP)/inc -I$(MMB_TOP)/inc/openmax\
            -I$(MMB_TOP)/oal \
            -I$(MMB_TOP)/utils \
            -I$(MME_TOP)/demuxer -I$(MME_TOP)/muxer \
            -I$(MME_TOP)/decoder -I$(MME_TOP)/encoder \
            -I$(MME_TOP)/decoder/buffer\
            -I$(MME_TOP)/decoder/singleton\
            -I$(MME_TOP)/decoder/hw-vpu\
            -I$(MME_TOP)/renderer \
            -I$(MME_TOP)/player \
            -I$(MME_TOP)/../libffmpeg \
            
LOCAL_CXXFLAGS := $(LOCAL_CFLAGS) -D__STDC_CONSTANT_MACROS
LOCAL_ASFLAGS :=  -mfloat-abi=softfp -mfpu=neon -march=armv7-a

LOCAL_LFLAGS := -shared -lpthread -ldl -lrt -lm -lstdc++ -L$(MME_TOP)/pre-built/linux_arm_ody -lffmpeg_2.1.3_ody_vfp -lx264 -ljpeg_ody -L$(MME_VPULIB_TOP)/theoraparser/theoraparser
LOCAL_LFLAGS	+= -L$(BASEDIR)/libs/libhwc -lhwc -lpthread
LOCAL_LFLAGS	+= -L$(BASEDIR)/libs/libion -lion
LOCAL_LFLAGS 	+= -L$(BASEDIR)/libs/libsync -lsync
LOCAL_LFLAGS 	+= -L$(BASEDIR)/libs/libmsgio -lmsgio
LOCAL_LFLAGS 	+= -L$(BASEDIR)/libs/libmmb -lmmb
LOCAL_LFLAGS 	+= -L$(BASEDIR)/libs/libv4l2_api -lv4l2_api
LOCAL_LFLAGS 	+= -lasound

#------------------------------------------------------------------------------
# DO NOT EDIT UNDER THIS LINE
include $(BASEDIR)/Rules.mk
#------------------------------------------------------------------------------

