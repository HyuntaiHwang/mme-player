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

#ifndef MME_C_API_H__
#define MME_C_API_H__

#include "v4l2_api_def.h"

#define MME_FOURCC_VIDEO_H263     V4L2_PIX_FMT_H263
#define MME_FOURCC_VIDEO_H264     V4L2_PIX_FMT_H264
#define MME_FOURCC_VIDEO_MPEG4    V4L2_PIX_FMT_MPEG4
#define MME_FOURCC_VIDEO_YUV420   V4L2_PIX_FMT_YUV420
#define MME_FOURCC_VIDEO_YUV420M  V4L2_PIX_FMT_YUV420M

#if 0
#define MMEMAKEFOURCC(ch0, ch1, ch2, ch3)                           \
                ((unsigned int)(unsigned char)(ch0) | ((unsigned int)(unsigned char)(ch1) << 8) |   \
                ((unsigned int)(unsigned char)(ch2) << 16) | ((unsigned int)(unsigned char)(ch3) << 24 ))
/* Video Codec */
#define MME_FOURCC_VIDEO_H263     MMEMAKEFOURCC('H','2','6','3')
#define MME_FOURCC_VIDEO_H264     MMEMAKEFOURCC('H','2','6','4')
#define MME_FOURCC_VIDEO_MPEG4    MMEMAKEFOURCC('M','P','4','V')

/* RGB */
#define MME_FOURCC_IMAGE_RGB888   MMEMAKEFOURCC('R','G','B','3')   /* 24bit RGB */
#define MME_FOURCC_IMAGE_RGBA8888 MMEMAKEFOURCC('R','G','B','4')   /* 32bit ARGB8888 */

/* YUV */
#define MME_FOURCC_IMAGE_YUV444_P1 MMEMAKEFOURCC('4','4','4','1')  /* 24bit Y/U/V 444 Plane 1 */
#define MME_FOURCC_IMAGE_YUV444_P3 MMEMAKEFOURCC('4','4','4','3')  /* 24bit Y/U/V 444 Plane 3 */
#define MME_FOURCC_IMAGE_YCbCr422_P2 MMEMAKEFOURCC('N','V','1','6')  /* 164bit Y/CbCr 4:2:2 Plane 2, V4L2_PIX_FMT_NV16 */
#define MME_FOURCC_IMAGE_YCrCb422_P2 MMEMAKEFOURCC('N','V','6','1')  /* 164bit Y/CrCb 4:2:2 Plane 2, V4L2_PIX_FMT_NV61 */


#define MME_FOURCC_IMAGE_YVU420      MMEMAKEFOURCC('Y','V','1','2')  /* V4L2_PIX_FMT_YVU420 12  YVU 4:2:0  3P contigous */
#define MME_FOURCC_IMAGE_YUV420      MMEMAKEFOURCC('Y','U','1','2')  /* V4L2_PIX_FMT_YUV420 12  YUV 4:2:0  3P contigous */
#define MME_FOURCC_IMAGE_YUV420M     MMEMAKEFOURCC('Y','M','1','2')  /* V4L2_PIX_FMT_YUV420M 12  YUV 4:2:0  3P non contiguous planes - Y, Cb, Cr */
#define MME_FOURCC_IMAGE_YVU420M     MMEMAKEFOURCC('Y','M','2','1')  /* V4L2_PIX_FMT_YVU420M 12  YUV 4:2:0  3P non contiguous planes - Y, Cb, Cr */
#define MME_FOURCC_IMAGE_NV12        V4L2_PIX_FMT_NV12   /* "NV12" 12  Y/CbCr 4:2:0  2Plane contigous -- one Y, one Cr + Cb interleaved */
#define MME_FOURCC_IMAGE_NV21        V4L2_PIX_FMT_NV21   /* "NV21" 12  Y/CrCb 4:2:0  2Plane contigous -- one Y, one Cr + Cb interleaved*/
#define MME_FOURCC_IMAGE_NV12M       V4L2_PIX_FMT_NV12M  /* "NM12" 12  Y/CbCr 4:2:0  2Plane non-contigous */
#define MME_FOURCC_IMAGE_NV21M       V4L2_PIX_FMT_NV21M  /* "NM21" 12  Y/CrCb 4:2:0  2Plane non-contigous */

#define MME_BYTE_ALIGN V4L2_BYTE_ALIGN
#endif

#ifdef WIN32
#define MME_DRIVER_OPEN  kernel_driver_open
#define MME_DRIVER_CLOSE kernel_driver_close
#define MME_DRIVER_WRITE kernel_driver_write
#define MME_DRIVER_IOCTL kernel_driver_ioctl
#define MME_DRIVER_MMAP  kernel_driver_mmap
#define MME_DRIVER_MUNMAP  kernel_driver_munmap

#ifdef __cplusplus
#define MME_FUNC_EXTERN extern "C" 
#else
#define MME_FUNC_EXTERN extern
#endif
MME_FUNC_EXTERN int kernel_driver_open(const char* drvname, int flag);
MME_FUNC_EXTERN int kernel_driver_close(int fd);
MME_FUNC_EXTERN int kernel_driver_write(int fd, char* buf, int bufsize);
MME_FUNC_EXTERN int kernel_driver_ioctl(int d, unsigned long request, void* arg);
MME_FUNC_EXTERN void* kernel_driver_mmap(void *addr, size_t length, int prot, int flags, int fd, unsigned int offset);
MME_FUNC_EXTERN int kernel_driver_munmap(void* start, size_t length);

#else
#define MME_DRIVER_OPEN    open
#define MME_DRIVER_CLOSE   close
#define MME_DRIVER_WRITE   write
#define MME_DRIVER_MMAP    mmap
#define MME_DRIVER_MUNMAP  munmap
#define MME_DRIVER_IOCTL   ioctl

#endif

#define MME_VIDEO_ENCODER_TEST_DUMP_KEY 0x00AA
struct mme_video_encoder_config {
    int width;
    int height;
    unsigned int in_format;
    unsigned int format;
    int bitrate;
    int fps;
    int idr_period; 
    int sw_codec_use; /* if 1, use FFMpeg Encoder */

    unsigned int test_dump_key; /* to dump, set MME_VIDEO_ENCODER_TEST_DUMP_KEY */
};

struct mme_muxer_config {
    char filename[256];
    struct mme_video_encoder_config video_encoder_config;
};

#define MME_MAX_PLANE_COUNT 3

struct mme_heap_plane {
    int plane_count;
    unsigned char* p_buf[MME_MAX_PLANE_COUNT];
    int buf_size[MME_MAX_PLANE_COUNT];
    int stride[MME_MAX_PLANE_COUNT];
};

struct mme_ion_buffer {
    int shared_fd;
    int buf_size;
    unsigned int vir_addr;
    int mem_offset;
    int stride;
};

struct mme_ion_plane {
    int plane_count;
    struct mme_ion_buffer plane[MME_MAX_PLANE_COUNT];
};

#define MME_IMG_BUF_USERPTR    0
#define MME_IMG_BUF_HEAP_PLANE 1
#define MME_IMG_BUF_ION_PLANE  2
struct mme_image_buffer {
    int type;
    int width;
    int height;
    unsigned int fourcc;
    int stride_userptr;
    union {
        unsigned char* data;
        struct mme_heap_plane* heap_plane;
        struct mme_ion_plane* ion_plane;
    }m;
};


struct mme_audio_format
{
    unsigned int  fourcc;         /* format type */
    int  sampleing_freq;     /* sample rate */
    int  channel;          /* number of channels (i.e. mono, stereo...) */
    int  bits_per_sample;     /* number of bits per sample of mono data */
    int bitrate;
};

#ifdef __cplusplus
extern "C" {
#endif

/* util */
void mme_util_sleep(int milesec);
unsigned int mme_util_get_tick_count();
long long mme_util_get_tick_count_us();

void mme_util_split_file_name(char* file_path_name, char* filename); /* e.g  IN:/mnt/abcde.dat OUT: abcde */
void mme_util_split_file_ext(char* file_path_name, char* file_ext); /* e.g  IN:/mnt/abcde.dat OUT: dat */
void mme_util_string_make_lower(char* sz);
void mme_util_string_make_upper(char* sz);

/* demuxer */
int mme_demuxer_get_audio_prop(char* mediafilename, struct mme_audio_format* p_audio_format);

/* Muxer */
void* mme_muxer_create_object(struct mme_muxer_config* p_muxer_config);
int mme_muxer_destroy_object(void* hdl);
int mme_muxer_add_video_config(void* hdl, unsigned char* p_config_data, int config_data_size);
int mme_muxer_add_video_data(void* hdl, unsigned char* p_video_stream, int video_stream_size, unsigned int flag);

/* video encoder */
void* mme_video_encoder_create_object(struct mme_video_encoder_config* p_video_encoder_config);
int mme_video_encoder_destroy_object(void* hdl);
int mme_video_encoder_run(void* hdl, 
                          int* shared_ion_fd, int* mem_offset, 
                          unsigned char* p_enc_stream, int stream_max_size, int* enc_stream_size, unsigned int* flag, unsigned int *enc_dur,
                          unsigned char* p_config_data, int config_max_size, int *config_data_size
                          );

int mme_video_encoder_run_YUV420(void* hdl, 
                          int* shared_ion_fd, int* mem_offset, 
                          unsigned char* p_enc_stream, int stream_max_size, int* enc_stream_size, unsigned int* flag, unsigned int *enc_dur,
                          unsigned char* p_config_data, int config_max_size, int *config_data_size
                          );

int mme_video_encoder_run_heap(void* hdl, 
                          long Y, long U, long V, 
                          unsigned char* p_enc_stream, int stream_max_size, int* enc_stream_size, unsigned int* flag,
                          unsigned char* p_config_data, int config_max_size, int *config_data_size
                          );

/* jpeg */
int mme_jpeg_decode_libjpeg(unsigned char* jpegdata, int jpegsize, struct mme_image_buffer* p_dst_buf);
int mme_jpeg_encode_libjpeg(struct mme_image_buffer* p_src_buf, int quality /* rage 0~100 */, unsigned char* jpegdata, int jpegdata_max_size, int* jpegsize);
int mme_jpeg_encode_jpu(struct mme_image_buffer* p_src_buf, int quality /* rage 0~100 */, unsigned char* jpegdata, int jpegdata_max_size, int* jpegsize);
int mme_jpeg_encode_v4l2(struct mme_image_buffer* p_src_buf, int quality /* rage 0~100 */, unsigned char* jpegdata, int jpegdata_max_size, int* jpegsize);

/* image tool */
int mme_imagetool_bmp_get_info(char* bmp_filename, int* pic_width, int *pic_height, unsigned int *fourcc, int* stride);
int mme_imagetool_bmp_load_file(char* bmp_filename, unsigned char* p_data_rgb, int max_data_size);
int mme_imagetool_bmp_load_file_for_v4l2(char* bmp_filename, struct v4l2_ion_frame* p_v4l2_ion_frame);
int mme_imagetool_yuv_load_file_for_v4l2(char* yuv_filename, struct v4l2_ion_frame* p_v4l2_ion_frame);
int mme_imagetool_convert_color(struct mme_image_buffer *p_src_buf, struct mme_image_buffer* p_dst_buf);
int mme_imagetool_jpeg_get_width_height_fourcc(char* jpegfilename, int* pic_width, int* pic_height, unsigned int* v4l2_fourcc);

/* movie muxing */
int mme_movie_mux_direct_copy_video(const char* srcfilename, const char* dstfilename, unsigned long start_pts_ms, unsigned long dur_ms, int fps);

#ifdef __cplusplus
}
#endif

#endif