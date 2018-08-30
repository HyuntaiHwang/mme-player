


LIBS_TOP=hardware/anapass/media
ANAPASS_MEDIA_LIBS_INC_TOP=hardware/anapass/media

MMB_INC_TOP := $(ANAPASS_MEDIA_LIBS_INC_TOP)/libmmb
MME_INC_TOP := $(ANAPASS_MEDIA_LIBS_INC_TOP)/libmme
OMX_INC_TOP := $(ANAPASS_MEDIA_LIBS_INC_TOP)/libomx
LIB_INC_TOP := $(ANAPASS_MEDIA_LIBS_INC_TOP)/include

LIBFFMPEG_INC_TOP := $(ANAPASS_MEDIA_LIBS_INC_TOP)/libffmpeg
V4L2_API_INC_TOP := $(ANAPASS_MEDIA_LIBS_INC_TOP)/libv4l2_api
MSCALER_INC_TOP := $(ANAPASS_MEDIA_LIBS_INC_TOP)/libmscaler

MMB_INC_PATH = $(MMB_INC_TOP)/inc $(MMB_INC_TOP)/oal $(MMB_INC_TOP)/utils
MME_INC_PATH = $(MME_INC_TOP)/api \
               $(MME_INC_TOP)/player \
		   $(MME_INC_TOP)/demuxer \
		   $(MME_INC_TOP)/muxer \
		   $(MME_INC_TOP)/decoder \
		   $(MME_INC_TOP)/decoder/buffer \
               $(MME_INC_TOP)/decoder/csc \
               $(MME_INC_TOP)/encoder \
		   $(MME_INC_TOP)/renderer\
		   $(MME_INC_TOP)/source $(MME_INC_TOP)/camera

OMX_INC_PATH = $(OMX_INC_TOP)/inc $(OMX_INC_TOP)/oal $(OMX_INC_TOP)/utils $(OMX_INC_TOP)/component/base $(OMX_INC_TOP)/component/video $(OMX_INC_TOP)/component/audio
V4L2_API_INC_PATH = $(V4L2_API_INC_TOP)/inc

ANDROID_C_INCLUDES:= \
      $(TOP)/frameworks/native/include/media/openmax \
      $(TOP)/frameworks/native/include/media/hardware \
      $(TOP)/frameworks/native/include/ \
      $(TOP)/frameworks/av/include $(TOP)/frameworks \
      $(TOP)/external/tinyalsa/include \
	$(TOP)/system/core/libion/include/ion \
      $(TOP)/external/libyuv/files/include \
      $(TOP)/external/openssl/include \
	$(TOP)/system/core/libion/include/ion\
	  
      
MMP_TARGET_SOC=7243        
MMP_TARGET_BOARD=7243      
MMP_TARGET_OS=1
MMP_TARGET_PLATFORM=1
MMP_TARGET_PLATFORM_VERSION=51
MMP_TARGET_FFMPEG=1

LOCAL_CFLAGS += -DMMP_TARGET_BOARD=$(MMP_TARGET_BOARD)
LOCAL_CFLAGS += -DMMP_TARGET_SOC=$(MMP_TARGET_SOC)
LOCAL_CFLAGS += -DMMP_TARGET_OS=$(MMP_TARGET_OS)
LOCAL_CFLAGS += -DMMP_TARGET_PLATFORM=$(MMP_TARGET_PLATFORM) 
LOCAL_CFLAGS += -DMMP_TARGET_PLATFORM_VERSION=$(MMP_TARGET_PLATFORM_VERSION)
LOCAL_CFLAGS += -DMMP_TARGET_FFMPEG=$(MMP_TARGET_FFMPEG)

#LOCAL_CFLAGS += -Wformat -Wno-switch -Werror=implicit-function-declaration -Werror=missing-prototypes -Werror=return-type -Werror=vla -Wno-format -mfpu=neon 
#LOCAL_CFLAGS +=  -Wno-unused-parameter  -Wno-write-strings
#LOCAL_CFLAGS += -D__STDINT_MACROS -DJPU_PLATFORM_V4L2_ENABLE=1
#LOCAL_CFLAGS += -D__ARM_NEON_ENABLE 

ifeq ($(MMP_TARGET_FFMPEG), 1)
FFMPEG_VERSION :=n3.1.1
FFMPEG_TOP := $(TOP)/vendor/anapass/gdm7243v/opensource/tools/ffmpeg/ffmpeg-$(FFMPEG_VERSION)
endif
