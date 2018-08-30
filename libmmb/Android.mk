LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

-include $(LOCAL_PATH)/../libmmb/common.mk

MMB_TOP := .

MMB_SRC_BASE = \
   $(MMB_TOP)/base/mmp_object.cpp \

MMB_SRC_UTILS = \
    $(MMB_TOP)/utils/MmpUtil.cpp \
    $(MMB_TOP)/utils/mmp_util.cpp \
    $(MMB_TOP)/utils/MmpUtil_Jpeg.cpp \
    $(MMB_TOP)/utils/MmpImageTool.cpp \
    $(MMB_TOP)/utils/MmpImageTool_neon.cpp \
    $(MMB_TOP)/utils/linux_tool/sysinfo.c \
    $(MMB_TOP)/utils/linux_tool/version.c \
    $(MMB_TOP)/utils/MmpBitExtractor.cpp\
    $(MMB_TOP)/utils/MmpMpeg4Tool.cpp\
    $(MMB_TOP)/utils/MmpH264Tool.cpp\
	$(MMB_TOP)/utils/MmpWMVTool.cpp\
    $(MMB_TOP)/utils/MmpAACTool.cpp\
    $(MMB_TOP)/utils/mmp_omx_tool.cpp\
	$(MMB_TOP)/utils/mmp_audio_tool.cpp\
	$(MMB_TOP)/utils/mmp_video_tool.cpp\
         
MMB_SRC_OAL = \
    $(MMB_TOP)/oal/mmp_msg_proc.cpp \
    $(MMB_TOP)/oal/mmp_lock.cpp \
    $(MMB_TOP)/oal/mmp_oal_cond.cpp \
    $(MMB_TOP)/oal/mmp_oal_cond_linux.cpp \
    $(MMB_TOP)/oal/mmp_oal_mutex.cpp \
    $(MMB_TOP)/oal/mmp_oal_mutex_linux.cpp \
    $(MMB_TOP)/oal/mmp_oal_task.cpp \
    $(MMB_TOP)/oal/mmp_oal_task_linux.cpp \
    $(MMB_TOP)/oal/mmp_oal_shm.cpp \
    $(MMB_TOP)/oal/mmp_oal_shm_heap.cpp \
	$(MMB_TOP)/oal/mmp_oal_shm_ashmem.cpp \
   
    

LOCAL_SRC_FILES := $(MMB_SRC_BASE) $(MMB_SRC_UTILS) $(MMB_SRC_OAL) 

LOCAL_C_INCLUDES:= \
      $(MMB_INC_TOP)/inc\
	  $(MMB_INC_TOP)/inc/openmax\
	  $(MMB_INC_TOP)/oal\
      $(MMB_INC_TOP)/utils\
      $(MMB_INC_TOP)/base\
	  $(MMB_INC_TOP)/../libv4l2_api/inc\
	  $(TOP)/frameworks/native/include/media/openmax \
	  $(TOP)/external/jpeg\

LOCAL_SHARED_LIBRARIES :=  libutils libcutils libjpeg 
        
LOCAL_MODULE := libmmb

LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)


