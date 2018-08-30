LOCAL_PATH := $(call my-dir)


##########################################################################################################
# libmme_cnm_release.a
##########################################################################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES:=  cnm-coda960-ref_sw_pkg-v3.3.0.a
LOCAL_MODULE := libmme_cnm_release
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_SUFFIX :=.a
include $(BUILD_PREBUILT)

##########################################################################################################
# libmme.so
##########################################################################################################
include $(CLEAR_VARS)

-include $(LOCAL_PATH)/../libmmb/common.mk

MME_TOP := .
MME_VPULIB_TOP := $(MME_TOP)/decoder/cnm-coda960-ref_sw_pkg-v3.3.0
MME_INC_VPULIB_TOP := $(MME_INC_TOP)/decoder/cnm-coda960-ref_sw_pkg-v3.3.0

MME_SRC_API = \
   $(MME_TOP)/api/mme_c_api_util.cpp \
   $(MME_TOP)/api/mme_c_api_video_encoder.cpp \
   $(MME_TOP)/api/mme_c_api_muxer.cpp \
   $(MME_TOP)/api/mme_c_api_demuxer.cpp \
   $(MME_TOP)/api/mme_c_api_jpeg.cpp \
   $(MME_TOP)/api/mme_c_api_imagetool.cpp \
   $(MME_TOP)/api/mme_c_api_movie_mux.cpp \
         
    
MME_SRC_BUFFER = \
    $(MME_TOP)/decoder/buffer/mmp_buffer.cpp \
    $(MME_TOP)/decoder/buffer/mmp_buffer_ion.cpp \
    $(MME_TOP)/decoder/buffer/mmp_buffer_ion_attach.cpp \
    $(MME_TOP)/decoder/buffer/mmp_buffer_heap.cpp \
    $(MME_TOP)/decoder/buffer/mmp_buffer_heap_attach.cpp \
    $(MME_TOP)/decoder/buffer/mmp_buffer_anw43_attach.cpp \
    $(MME_TOP)/decoder/buffer/mmp_buffer_meta.cpp \
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
    $(MME_TOP)/decoder/hw-vpu/mmp_vpu_dev.cpp \
	$(MME_TOP)/decoder/hw-vpu/mmp_vpu_dev_shm.cpp \
	$(MME_TOP)/decoder/hw-vpu/mmp_vpu_dev_ex4.cpp \
	$(MME_TOP)/decoder/hw-vpu/mmp_vpu_hw.cpp \
	$(MME_TOP)/decoder/hw-vpu/mmp_vpu_hw_linux.cpp \

MME_SRC_HW_JPU = \
    $(MME_TOP)/decoder/hw-jpu/mmp_jpu_if.cpp \
    $(MME_TOP)/decoder/hw-jpu/mmp_jpu_if_ana.cpp \
    $(MME_TOP)/decoder/hw-jpu/mmp_jpu_dev.cpp \
    $(MME_TOP)/decoder/hw-jpu/mmp_jpu_dev_shm.cpp \

MME_SRC_SINGLETON = \
   $(MME_TOP)/decoder/singleton/mmp_singleton_mgr.cpp\
   $(MME_TOP)/decoder/singleton/mmp_env_mgr.cpp\

MME_SRC_CSC = \
   $(MME_TOP)/decoder/csc/mmp_csc.cpp \
   $(MME_TOP)/decoder/csc/mmp_csc_sw.cpp \
   

MME_SRC_DECODER = \
   $(MME_TOP)/decoder/MmpDecoder.cpp \
   $(MME_TOP)/decoder/MmpDecoderAudio.cpp \
   $(MME_TOP)/decoder/MmpDecoderAudio_Ffmpeg.cpp \
   $(MME_TOP)/decoder/MmpDecoderAudio_FfmpegEx2.cpp \
   $(MME_TOP)/decoder/MmpDecoderAudio_FfmpegEx3.cpp \
   $(MME_TOP)/decoder/MmpDecoderVideo.cpp \
   $(MME_TOP)/decoder/MmpDecoderVpuEx1.cpp \
   $(MME_TOP)/decoder/MmpDecoderVideo_VpuEx1.cpp \
   $(MME_TOP)/decoder/MmpDecoderImage.cpp \
   $(MME_TOP)/decoder/MmpDecoderImage_JpegLib.cpp \
ifeq ($(MMP_TARGET_FFMPEG), 1)
MME_SRC_DECODER += \
   $(MME_TOP)/decoder/MmpDecoderFfmpegEx1.cpp \
   $(MME_TOP)/decoder/MmpDecoderVideo_FfmpegEx1.cpp \
endif
  

MME_SRC_ENCODER = \
   $(MME_TOP)/encoder/MmpEncoder.cpp \
   $(MME_TOP)/encoder/MmpEncoderVpuIF.cpp \
   $(MME_TOP)/encoder/MmpEncoderVideo.cpp \
   $(MME_TOP)/encoder/MmpEncoderVideo_Vpu.cpp \
   $(MME_TOP)/encoder/MmpEncoderVideo_Dummy.cpp \

ifeq ($(MMP_TARGET_FFMPEG), 1)
MME_SRC_ENCODER += \
   $(MME_TOP)/encoder/MmpEncoderFfmpegEx1.cpp \
   $(MME_TOP)/encoder/MmpEncoderVideo_FfmpegEx1.cpp 

endif

   
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
   $(MME_TOP)/demuxer/MmpDemuxer_ammf.cpp
ifeq ($(MMP_TARGET_FFMPEG), 1)
MME_SRC_DEMUXER += \
   $(MME_TOP)/demuxer/MmpDemuxer_Ffmpeg.cpp \
   $(MME_TOP)/demuxer/MmpDemuxer_FfmpegEx2.cpp 
endif

  
MME_SRC_MUXER = \
   $(MME_TOP)/muxer/MmpMuxer.cpp \
   $(MME_TOP)/muxer/MmpMuxer_ammf.cpp \
   $(MME_TOP)/muxer/MmpMuxer_rawstream.cpp 

ifeq ($(MMP_TARGET_FFMPEG), 1)
MME_SRC_MUXER += \
   $(MME_TOP)/muxer/MmpMuxer_Ffmpeg.cpp \
   $(MME_TOP)/muxer/MmpMuxer_FfmpegEx1.cpp \
   $(MME_TOP)/muxer/MmpMuxer_FfmpegEx2.cpp

endif

MME_SRC_PLAYER = \
   $(MME_TOP)/player/MmpPlayerService.cpp \
   $(MME_TOP)/player/MmpPlayer.cpp \
   $(MME_TOP)/player/MmpPlayerVideoEx3.cpp \
   $(MME_TOP)/player/MmpPlayerAudio.cpp \
   $(MME_TOP)/player/MmpPlayerAVEx5.cpp \
   $(MME_TOP)/player/MmpPlayerTONE.cpp \
   $(MME_TOP)/player/MmpPlayerPCM.cpp \
   $(MME_TOP)/player/MmpPlayerYUV.cpp \
   $(MME_TOP)/player/MmpPlayerRGB.cpp \
   $(MME_TOP)/player/MmpPlayerJPEG.cpp \
   $(MME_TOP)/player/MmpPlayerStagefright.cpp \


MME_SRC_RENDERER = \
   $(MME_TOP)/renderer/MmpRenderer.cpp \
   $(MME_TOP)/renderer/MmpRendererAudio.cpp \
   $(MME_TOP)/renderer/MmpRendererVideo.cpp \
   $(MME_TOP)/renderer/MmpRenderer_YUVWriter.cpp \
   $(MME_TOP)/renderer/MmpRenderer_DummyVideo.cpp \
   $(MME_TOP)/renderer/MmpRenderer_AndroidSurfaceEx3.cpp \
   $(MME_TOP)/renderer/MmpRenderer_AndroidAudioTrack.cpp \

LOCAL_SRC_FILES := $(MME_SRC_API) $(MME_SRC_BUFFER) $(MME_SRC_SINGLETON) $(MME_SRC_CSC)\
                   $(MME_SRC_DECODER) $(MME_SRC_ENCODER) \
     		       $(MME_SRC_HW_VPU)  \
				   $(MME_SRC_PLAYER) $(MME_SRC_RENDERER) $(MME_SRC_DEMUXER) $(MME_SRC_MUXER)

LOCAL_C_INCLUDES:= \
      $(MMB_INC_TOP)/inc\
	  $(MMB_INC_TOP)/inc/openmax\
	  $(MMB_INC_TOP)/base\
      $(MMB_INC_TOP)/oal\
      $(MMB_INC_TOP)/utils\
      $(MME_INC_TOP)/api\
      $(MME_INC_TOP)/renderer\
      $(MME_INC_TOP)/encoder\
      $(MME_INC_TOP)/decoder\
      $(MME_INC_TOP)/decoder/buffer\
      $(MME_INC_TOP)/decoder/hw-jpu\
      $(MME_INC_TOP)/decoder/hw-vpu\
	  $(MME_INC_TOP)/decoder/singleton\
	  $(MME_INC_TOP)/demuxer\
	  $(MME_INC_TOP)/muxer\
	  $(MME_INC_VPULIB_TOP)/vpuapi $(MME_INC_VPULIB_TOP)/include $(MME_INC_VPULIB_TOP)/vdi/linux/driver $(MME_INC_VPULIB_TOP)/vdi $(MME_INC_VPULIB_TOP)/include\
      $(MME_INC_TOP)/../libv4l2_api/inc\
	  $(MME_INC_TOP)/../libion\
	  $(TOP)/system/media/audio_utils/include\
	  $(TOP)/frameworks/native/include/media/hardware\
	  $(TOP)/frameworks/native/include/media/openmax \
	  $(TOP)/external/tinyalsa/include\
	  $(TOP)/external/jpeg\
	  $(TOP)/system/core/libion/include/ion\
      $(TOP)/frameworks/av/include\
	  $(TOP)/external/tinycompress/include\

ifeq ($(MMP_TARGET_FFMPEG), 1)
LOCAL_C_INCLUDES += $(LIBFFMPEG_INC_TOP) $(FFMPEG_TOP)
endif
            
LOCAL_CFLAGS += -D__STDINT_MACROS -DJPU_PLATFORM_V4L2_ENABLE=1 -D__VPU_PLATFORM_MME
LOCAL_CFLAGS += -Wno-error=missing-prototypes
#LOCAL_CFLAGS += -Wno-deprecated-declarations

LOCAL_SHARED_LIBRARIES :=    \
        libutils             \
        libcutils            \
        libbinder            \
        libui                \
        libdl                \
        libstagefright_foundation \
        libstagefright \
        libjpeg\
        libtinyalsa\
	    libgui \
        libEGL \
	    libGLESv1_CM \
	    libGLESv2 \
	    libsync \
	    libion\
        libmedia libmediaplayerservice	    \
	    libmmb libv4l2_api 

ifeq ($(MMP_TARGET_FFMPEG), 1)
LOCAL_SHARED_LIBRARIES += libffmpeg
endif

LOCAL_STATIC_LIBRARIES := libmme_cnm_release

LOCAL_MODULE := libmme
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)


