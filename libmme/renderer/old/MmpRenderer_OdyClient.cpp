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

#include "MmpRenderer_OdyClient.hpp"

#include "MmpUtil.hpp"
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h> // for POSIX threads
#include <ctype.h>

#include <fcntl.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
//#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <limits.h>

#include "hwcomposer.h"
#include "ion_api.h"
#include "sync.h"

#include "mmp_buffer_mgr.hpp"

#if (MMP_OS == MMP_OS_WIN32)
struct sockaddr_un {
    int sun_family;
    char sun_path[32];
};
#define bzero(mem, sz) memset(mem, 0x00, sz)
#define AF_UNIX 1

#else


#include <time.h>
#include <inttypes.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <linux/types.h>
#include <linux/stat.h>
#include <ion_api.h>
#include "gdm_fb.h"
#include <linux/fb.h>

#endif

#define UNIX_SOCKET_PATH       "/tmp/sock_msgio"
#define BUFFER_SIZE            (4096)
#define FD_COUNT               (16)

static int dss_get_response(int sockfd);
static void dss_get_fence_fd(int sockfd, int *release_fd, struct fb_var_screeninfo *vi);
static void dss_overlay_default_config(struct gdm_dss_overlay *req,	struct ody_player *gplayer);
static int dss_overlay_set(int sockfd, struct gdm_dss_overlay *req);
static int alloc_video_memory(struct ody_videoframe *pframe);
static int free_video_memory(struct ody_videoframe *pframe);
static int dss_overlay_queue(int sockfd, struct gdm_dss_overlay_data *req_data);
static int dss_overlay_unset(int sockfd);

/////////////////////////////////////////////////////////////
//CMmpRenderer_OdyClient Member Functions


CMmpRenderer_OdyClient::CMmpRenderer_OdyClient(CMmpRendererCreateProp* pRendererProp) :  CMmpRenderer(MMP_MEDIATYPE_VIDEO, pRendererProp)
,m_sock_fd(-1)
,m_buf_idx(0)
{
    memset(&m_gplayer, 0x00, sizeof(struct ody_player));
    m_gplayer.release_fd = -1;

}

CMmpRenderer_OdyClient::~CMmpRenderer_OdyClient()
{

}

MMP_RESULT CMmpRenderer_OdyClient::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;
    int iret;
    struct ody_player *gplayer = &m_gplayer;
    struct ody_framebuffer *pfb;
	struct ody_videofile *pvideo;


    mmpResult=CMmpRenderer::Open();

	MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_OdyClient::Open] +++ W:%d H:%d "), m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight));

    /* STEP0. Alloc FrameBuffer */
    if(mmpResult == MMP_SUCCESS) {

        pfb = &gplayer->fb_info;
		pvideo = &gplayer->video_info;
        pvideo->width = MMP_BYTE_ALIGN(m_pRendererProp->m_iPicWidth,16);
        pvideo->height = MMP_BYTE_ALIGN(m_pRendererProp->m_iPicHeight,16);
		pvideo->format = GDM_DSS_PF_YUV420P3;

        gplayer->frame[0].size[0] = pvideo->width * pvideo->height;
    	gplayer->frame[0].size[1] = gplayer->frame[0].size[2] = pvideo->width * pvideo->height / 4;

	    gplayer->frame[1].size[0] = gplayer->frame[0].size[0];
	    gplayer->frame[1].size[1] = gplayer->frame[0].size[1];
	    gplayer->frame[1].size[2] = gplayer->frame[0].size[2];

#if 1
        if(alloc_video_memory(&gplayer->frame[0]) != 0) {/* front buffer */
    		mmpResult = MMP_FAILURE;
			memset(&gplayer->frame[0], 0x00, sizeof(struct ody_videoframe));
    	}
        else {
	        if(alloc_video_memory(&gplayer->frame[1]) != 0) { /* back buffer */
		        mmpResult = MMP_FAILURE;
				memset(&gplayer->frame[1], 0x00, sizeof(struct ody_videoframe));
    	    }
        }
#endif
    }

    /* STEP1. connect to server */
	if(mmpResult == MMP_SUCCESS) {

		struct sockaddr_un server_addr;

		m_sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
		if(m_sock_fd >= 0) {

			bzero(&server_addr, sizeof(server_addr));
			server_addr.sun_family = AF_UNIX;
			strcpy(server_addr.sun_path, UNIX_SOCKET_PATH);

			iret = connect(m_sock_fd,(struct sockaddr *)&server_addr, sizeof(server_addr));
            if(iret != 0) {
                MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_OdyClient::Open] FAIL: connect ")));
                mmpResult = MMP_FAILURE;
            }
            else {
                dss_get_fence_fd(m_sock_fd, &gplayer->release_fd, &gplayer->vi);
            }
		}
		else {
			mmpResult = MMP_FAILURE;
		}
	}

    // step-02: request overlay to display
    if(mmpResult == MMP_SUCCESS) {

	    dss_overlay_default_config(&m_req, gplayer);
	    dss_overlay_set(m_sock_fd, &m_req);
		dss_get_response(m_sock_fd);
    }

	m_luma_size = m_pRendererProp->m_iPicWidth*m_pRendererProp->m_iPicHeight;
	m_chroma_size = m_luma_size/4;


    return mmpResult;
}


MMP_RESULT CMmpRenderer_OdyClient::Close()
{
    MMP_RESULT mmpResult;

    mmpResult=CMmpRenderer::Close();

    if(m_sock_fd >= 0) {

        // unset
	    dss_overlay_unset(m_sock_fd);

        if(m_gplayer.release_fd != -1) {
            ::MMP_DRIVER_CLOSE(m_gplayer.release_fd);
            m_gplayer.release_fd = -1;
        }

        ::MMP_DRIVER_CLOSE(m_sock_fd);
        m_sock_fd = -1;
    }

	free_video_memory(&m_gplayer.frame[0]);
	free_video_memory(&m_gplayer.frame[1]);

    return mmpResult;
}

MMP_RESULT CMmpRenderer_OdyClient::Render(CMmpMediaSampleDecodeResult* pDecResult)
{

    return this->RenderYUV420Planar((MMP_U8*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_Y],
                                    (MMP_U8*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_U],
                                    (MMP_U8*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_V],
								    m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight
								 );
}

MMP_RESULT CMmpRenderer_OdyClient::RenderYUV420Planar(MMP_U8* Y, MMP_U8* U, MMP_U8* V, MMP_U32 buffer_width, MMP_U32 buffer_height) {

    MMP_RESULT mmpResult;
    unsigned int key=0xAAAA9829;
    unsigned int key1, key2;

	key1 = *((unsigned int*)U);
    key2 = *((unsigned int*)V);

	if( (key1 == key) && (key2 == key) ) {
	    mmpResult = this->RenderYUV420Planar_Ion(Y, U, V, buffer_width, buffer_height);
    }
    else {
	    mmpResult = this->RenderYUV420Planar_Memory(Y, U, V, buffer_width, buffer_height);
    }

	return mmpResult;
}

#include "vpuapi.h"

typedef void (*vdi_memcpy_func)(void* param, void* dest_vaddr, void* src_paddr, int size);

MMP_RESULT CMmpRenderer_OdyClient::RenderYUV420Planar_Ion(MMP_U8* Y, MMP_U8* U, MMP_U8* V, MMP_U32 buffer_width, MMP_U32 buffer_height) {

    FrameBuffer* pVPU_FrameBuffer;
	int iret;
	unsigned int t1, t2;

    pVPU_FrameBuffer = (FrameBuffer*)Y;

    MMPDEBUGMSG(0, (TEXT("[CMmpRenderer_OdyClient::RenderYUV420Planar] +++ ION  fd=%d buf(0x%08x 0x%08x 0x%08x , 0x%08x) "),
                          pVPU_FrameBuffer->ion_shared_fd,
                          pVPU_FrameBuffer->bufY, pVPU_FrameBuffer->bufCb, pVPU_FrameBuffer->bufCr,
                          pVPU_FrameBuffer->ion_base_phyaddr));


	memset(&m_req_data, 0x00, sizeof(struct gdm_dss_overlay_data));
	m_req_data.id = 0;
	m_req_data.num_plane = 1;
	m_req_data.data[0].memory_id = pVPU_FrameBuffer->ion_shared_fd;
	m_req_data.data[0].offset = pVPU_FrameBuffer->bufY - pVPU_FrameBuffer->ion_base_phyaddr;

	dss_overlay_queue(m_sock_fd, &m_req_data);
    if(m_gplayer.release_fd == -1) {
        dss_get_fence_fd(m_sock_fd, &m_gplayer.release_fd, NULL);
    }

    if(m_gplayer.release_fd != -1) {
		//printf("wait frame done signal\n");

		t1 = CMmpUtil::GetTickCount();
		iret = sync_wait(m_gplayer.release_fd, 1000);
		t2 = CMmpUtil::GetTickCount();

        ::MMP_DRIVER_CLOSE(m_gplayer.release_fd);
        //MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_OdyClient::RenderYUV420Planar] sync_wait %d"), t2-t1));

        m_gplayer.release_fd = -1;
#if 0
		if( (t2-t1) < 100) {
			CMmpUtil::Sleep( 100 - (t2-t1) );
		}
#endif
	}
    else {
        CMmpUtil::Sleep(100);
    }

	if( (m_pVideoEncoder != NULL) && (m_pMuxer != NULL) && (m_p_enc_stream!=NULL) ) {

		void* param;
		vdi_memcpy_func vdi_memcpy;
		unsigned int addr, value;
		unsigned char *dest_y, *dest_u, *dest_v;

		dest_y = m_gplayer.frame[m_buf_idx].address[0];
		dest_u = m_gplayer.frame[m_buf_idx].address[1];
		dest_v = m_gplayer.frame[m_buf_idx].address[2];


		addr = (unsigned int)V;

		addr+=sizeof(unsigned int);
		memcpy(&value, (void*)addr, sizeof(unsigned int));
		param = (void*)value;

		addr+=sizeof(unsigned int);
		memcpy(&value, (void*)addr, sizeof(unsigned int));
		vdi_memcpy = (vdi_memcpy_func)value;

#ifdef __VPU_PLATFORM_MME
        class mmp_buffer* p_mmp_buf;
        class mmp_buffer_addr buf_addr;
        MMP_U32 src_y, src_u, src_v;
        p_mmp_buf = mmp_buffer_mgr::get_instance()->get_buffer(pVPU_FrameBuffer->ion_shared_fd);
        if(p_mmp_buf != NULL) {
            buf_addr = p_mmp_buf->get_buf_addr();

            src_y = buf_addr.m_vir_addr;
            src_u = src_y + MMP_BYTE_ALIGN(m_pRendererProp->m_iPicWidth,16)*MMP_BYTE_ALIGN(m_pRendererProp->m_iPicHeight,16);
            src_v = src_u + MMP_BYTE_ALIGN(m_pRendererProp->m_iPicWidth,16)*MMP_BYTE_ALIGN(m_pRendererProp->m_iPicHeight,16)/4;

            memcpy(dest_y, (void*)src_y, m_luma_size);
            memcpy(dest_u, (void*)src_u, m_chroma_size);
            memcpy(dest_v, (void*)src_v, m_chroma_size);

            CMmpRenderer::EncodeAndMux(dest_y, dest_u, dest_v, buffer_width, buffer_height);
        }
#else
		(*vdi_memcpy)(param, dest_y, (void*)pVPU_FrameBuffer->bufY, m_luma_size);
		(*vdi_memcpy)(param, dest_u, (void*)pVPU_FrameBuffer->bufCb, m_chroma_size);
		(*vdi_memcpy)(param, dest_v, (void*)pVPU_FrameBuffer->bufCr, m_chroma_size);

        CMmpRenderer::EncodeAndMux(dest_y, dest_u, dest_v, buffer_width, buffer_height);
#endif


	}

	m_buf_idx ^= 1;

	return MMP_SUCCESS;
}


MMP_RESULT CMmpRenderer_OdyClient::RenderYUV420Planar_Memory(MMP_U8* Y, MMP_U8* U, MMP_U8* V, MMP_U32 buffer_width, MMP_U32 buffer_height) {

	unsigned char *dest_y, *dest_u, *dest_v;
    int iret;

    MMPDEBUGMSG(0, (TEXT("[CMmpRenderer_OdyClient::RenderYUV420Planar_mem] +++ ")));
#if 1
	dest_y = m_gplayer.frame[m_buf_idx].address[0];
	dest_u = m_gplayer.frame[m_buf_idx].address[1];
	dest_v = m_gplayer.frame[m_buf_idx].address[2];

#if 0
    if(m_fp_dump != NULL) {
        fwrite(Y, 1, m_luma_size, m_fp_dump);
        fwrite(U, 1, m_chroma_size, m_fp_dump);
        fwrite(V, 1, m_chroma_size, m_fp_dump);
    }
#endif

#if 1
	memcpy(dest_y, Y, m_luma_size);
	memcpy(dest_u, U, m_chroma_size);
	memcpy(dest_v, V, m_chroma_size);

    //memset(dest_u, 128, m_chroma_size);
	//memset(dest_v, 128, m_chroma_size);
#else

    memset(dest_y, 0xFF, m_luma_size);
	memset(dest_u, 128, m_chroma_size);
	memset(dest_v, 128, m_chroma_size);
#endif


    memset(&m_req_data, 0x00, sizeof(struct gdm_dss_overlay_data));
	m_req_data.id = 0;
	m_req_data.num_plane = 3;
	m_req_data.data[0].memory_id = m_gplayer.frame[m_buf_idx].shared_fd[0];
	m_req_data.data[0].offset = 0;

	m_req_data.data[1].memory_id = m_gplayer.frame[m_buf_idx].shared_fd[1];
	m_req_data.data[1].offset = 0;

	m_req_data.data[2].memory_id = m_gplayer.frame[m_buf_idx].shared_fd[2];
	m_req_data.data[2].offset = 0;
#else
    memset(&m_req_data, 0x00, sizeof(struct gdm_dss_overlay_data));
	m_req_data.id = 0;
	m_req_data.num_plane = 1;
	m_req_data.data[0].memory_id = m_gplayer.frame[m_buf_idx].shared_fd[0];
	m_req_data.data[0].offset = 0;
#endif

    unsigned int t1, t2;

    t1 = CMmpUtil::GetTickCount();
	dss_overlay_queue(m_sock_fd, &m_req_data);
    if(m_gplayer.release_fd == -1) {
        dss_get_fence_fd(m_sock_fd, &m_gplayer.release_fd, NULL);
    }

    if(m_gplayer.release_fd != -1) {
        //printf("wait frame done signal\n");
		iret = sync_wait(m_gplayer.release_fd, 1000);
        t2 = CMmpUtil::GetTickCount();

        ::MMP_DRIVER_CLOSE(m_gplayer.release_fd);
        m_gplayer.release_fd = -1;
#if 0
        if( (t2-t1) < 100) {
			CMmpUtil::Sleep( 100 - (t2-t1) );
		}
#endif
        //MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_OdyClient::RenderYUV420Planar_mem] +++ %d "), t2-t1 ));
	}
    else {
        CMmpUtil::Sleep(100);
    }


	m_buf_idx ^= 1;

	CMmpRenderer::EncodeAndMux(Y, U, V, buffer_width, buffer_height);

	return MMP_SUCCESS;
}


static int dss_get_response(int sockfd)
{
	int ret = 0;
	struct gdm_msghdr *msg = NULL;

	msg = gdm_recvmsg(sockfd);

	if(msg != NULL){
		if(*(int *)msg->buf == -1)
			ret = -1;
		gdm_free_msghdr(msg);
	}

	return ret;
}


static void dss_get_fence_fd(int sockfd, int *release_fd, struct fb_var_screeninfo *vi)
{
	struct gdm_msghdr *msg = NULL;

//	printf("dss_get_fence_fd - start\n");
	msg = gdm_recvmsg(sockfd);

//	printf("received msg: %0x\n", (unsigned int)msg);
	if(msg != NULL){
		if(vi) {
			memcpy(vi, msg->buf, sizeof(struct fb_var_screeninfo));
		}
//		printf("msg->fds[0]: %d\n", msg->fds[0]);
		*release_fd = msg->fds[0];
		gdm_free_msghdr(msg);
	}
    else {
        *release_fd = -1;
    }

}

static void dss_overlay_default_config(struct gdm_dss_overlay *req,	struct ody_player *gplayer)
{

	memset(req, 0x00, sizeof(*req));

	req->alpha = 255;
	req->blend_op = GDM_FB_BLENDING_NONE;
	req->src.width = gplayer->video_info.width;
	req->src.height = gplayer->video_info.height;
	req->src.format = gplayer->video_info.format;
	//req->src.endian = 0;
	//req->src.swap = 0;
	req->pipe_type = GDM_DSS_PIPE_TYPE_VIDEO;

	req->src_rect.x = req->src_rect.y = 0;
	req->src_rect.w = req->src.width;
	req->src_rect.h = req->src.height;

	req->dst_rect.x = req->dst_rect.y = 0;
	req->dst_rect.w = gplayer->vi.xres;
	req->dst_rect.h = gplayer->vi.yres;

	req->transp_mask = 0;
	req->flags = GDM_DSS_FLAG_SCALING;
	req->id = GDMFB_NEW_REQUEST;


}

static int dss_overlay_set(int sockfd, struct gdm_dss_overlay *req)
{
	struct gdm_hwc_msg msg_data;
	struct gdm_msghdr *msg = NULL;

	memset(&msg_data, 0x00, sizeof(struct gdm_hwc_msg));

	msg_data.app_id = APP_ID_MULTI_SAMPLE_PLAYER;
	msg_data.hwc_cmd = GDMFB_OVERLAY_SET;

	msg = gdm_alloc_msghdr(sizeof(struct gdm_hwc_msg), 0);

	memcpy(msg_data.data, req, sizeof(struct gdm_dss_overlay));
	memcpy(msg->buf, &msg_data, sizeof(struct gdm_hwc_msg));

	gdm_sendmsg(sockfd, msg);

	return 0;
}

static int alloc_video_memory(struct ody_videoframe *pframe)
{
	int i = 0;
	int ret = 0;
	int fd;

	fd = ion_open();
	for(i = 0; i< 3; i++) {

		ret = ion_alloc_fd(fd, pframe->size[i], 0, ION_HEAP_CARVEOUT_MASK,	0, &pframe->shared_fd[i]);
		if (ret) {
			printf("share failed %s\n", strerror(errno));
			goto cleanup;
		}

		pframe->address[i] = (unsigned char*)mmap(NULL, pframe->size[i], (PROT_READ | PROT_WRITE), MAP_SHARED, pframe->shared_fd[i], 0);
		if( pframe->address[i] == MAP_FAILED) {
			goto cleanup;
		}
	}

cleanup:
	ion_close(fd);

	printf("alloc_video_memory: fd: %d, ret: %d\n", fd, ret);
	return ret;

}

static int free_video_memory(struct ody_videoframe *pframe) {

	int i;
	int ret = 0;

	for(i = 0; i< 3; i++) {

		if(pframe->address[i] != NULL) {
            ::MMP_DRIVER_MUNMAP(pframe->address[i], pframe->size[i]);
		}
        if(pframe->shared_fd[i] >= 0) {
            ::MMP_DRIVER_CLOSE(pframe->shared_fd[i]);
        }
	}

	printf("free_video_memory \n");
	return ret;
}


static int dss_overlay_queue(int sockfd, struct gdm_dss_overlay_data *req_data)
{
	struct gdm_hwc_msg msg_data;
	struct gdm_msghdr *msg = NULL;
	MMP_S32 i = 0;
	memset(&msg_data, 0x00, sizeof(struct gdm_hwc_msg));

	msg = gdm_alloc_msghdr(sizeof(struct gdm_hwc_msg), req_data->num_plane);

	msg_data.app_id = APP_ID_MULTI_SAMPLE_PLAYER;
	msg_data.hwc_cmd = GDMFB_OVERLAY_PLAY;
	memcpy(msg_data.data, req_data, sizeof(struct gdm_dss_overlay_data));
	memcpy(msg->buf, &msg_data, sizeof(struct gdm_hwc_msg));

	for(i = 0 ; i < (MMP_S32)req_data->num_plane ; i++)
		msg->fds[i] = req_data->data[i].memory_id;
	gdm_sendmsg(sockfd, msg);

	return 0;
}

static int dss_overlay_unset(int sockfd)
{
	struct gdm_hwc_msg msg_data;
	struct gdm_msghdr *msg = NULL;

	memset(&msg_data, 0x00, sizeof(struct gdm_hwc_msg));
	msg_data.app_id = APP_ID_MULTI_SAMPLE_PLAYER;
	msg_data.hwc_cmd = GDMFB_OVERLAY_UNSET;

	msg = gdm_alloc_msghdr(sizeof(struct gdm_hwc_msg), 0);
	memcpy(msg->buf, &msg_data, sizeof(struct gdm_hwc_msg));
	gdm_sendmsg(sockfd, msg);

	return 0;

}

