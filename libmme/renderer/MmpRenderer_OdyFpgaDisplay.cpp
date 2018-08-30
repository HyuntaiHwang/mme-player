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

#include "MmpRenderer_OdyFpgaDisplay.hpp"

#if (MMP_OS == MMP_OS_LINUX)

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

#include <time.h>
#include <inttypes.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <linux/types.h>
#include <linux/stat.h>


#include <ion_api.h>

//#include "define.h"
#include "gdm_fb.h"
//#include "ipc_msg.h"

#include <linux/fb.h>


static int open_framebuffer(void)
{
	int fd = 0;

	fd = open(FRAMEBUFFER_NAME, O_RDWR);
	if (fd < 0) {
		printf("cannot open /dev/fb0, retrying with /dev/fb0\n");
		return -1;
	}

	return fd;
}

static int get_video_frame(int fd, unsigned char *pdata, int size, int *index)
{
	//int ret;
	int nread = 0;


	nread = read(fd, pdata, size);

	if(nread != size) {
		close(fd);
		return -1;
	}

	if(index) { (*index)++; }

	return 0;
}

static void dump_fb_info(struct fb_fix_screeninfo *fi, struct fb_var_screeninfo *vi)
{
	printf("dump_fb_info\n");
	fprintf(stderr,"vi.xres = %d\n", vi->xres);
	fprintf(stderr,"vi.yres = %d\n", vi->yres);
	fprintf(stderr,"vi.xresv = %d\n", vi->xres_virtual);
	fprintf(stderr,"vi.yresv = %d\n", vi->yres_virtual);
	fprintf(stderr,"vi.xoff = %d\n", vi->xoffset);
	fprintf(stderr,"vi.yoff = %d\n", vi->yoffset);
	fprintf(stderr, "vi.bits_per_pixel = %d\n", vi->bits_per_pixel);
	fprintf(stderr, "fi.line_length = %d\n", fi->line_length);
}

static int get_framebuffer(int fd,
		struct fb_var_screeninfo *vi,
		struct fb_fix_screeninfo *fi)
{
	int vsync_enable = 1;
	int ret = 0;
	if(ioctl(fd, FBIOGET_VSCREENINFO, vi) < 0) {
		printf("failed to get fb0 info");
		return -1;
	}

	if(ioctl(fd, FBIOGET_FSCREENINFO, fi) < 0) {
		perror("failed to get fb0 info");
		return -1;
	}

	dump_fb_info(fi, vi);

	ret = ioctl(fd, GDMFB_OVERLAY_VSYNC_CTRL, &vsync_enable);
	return ret;
}

static int alloc_video_memory(struct ody_videoframe *pframe)
{
	int ret = 0;
	int fd;

	fd = ion_open();

	ret = ion_alloc_fd(fd, pframe->size, 0, ION_HEAP_CARVEOUT_MASK,
		0, &pframe->shared_fd);
	if (ret) {
		printf("share failed %s\n", strerror(errno));
		goto cleanup;
	}

	pframe->address = (unsigned char*)mmap(NULL, pframe->size, (PROT_READ | PROT_WRITE), MAP_SHARED, pframe->shared_fd, 0);
	if (pframe->address == MAP_FAILED) {
		goto cleanup;
	}

cleanup:
	ion_close(fd);

	printf("alloc_video_memory: fd: %d, ret: %d\n", fd, ret);
	return ret;

}


static void dss_overlay_default_config(struct gdm_dss_overlay *req,
	struct ody_player *p_player)
{

	memset(req, 0x00, sizeof(*req));

	req->alpha = 255;
	req->blend_op = GDM_FB_BLENDING_NONE;
	req->src.width = p_player->video_info.width;
	req->src.height = p_player->video_info.height;
	req->src.format = p_player->video_info.format;
	//req->src.endian = 0;
	//req->src.swap = 0;
	req->pipe_type = GDM_DSS_PIPE_TYPE_VIDEO;

	req->src_rect.x = req->src_rect.y = 0;
	req->src_rect.w = req->src.width;
	req->src_rect.h = req->src.height;

	req->dst_rect.x = req->dst_rect.y = 0;
	req->dst_rect.w = p_player->fb_info.vi.xres;
	req->dst_rect.h = p_player->fb_info.vi.yres;

	req->transp_mask = 0;
	req->flags = GDM_DSS_FLAG_SCALING;
	req->id = GDMFB_NEW_REQUEST;

}


static int dss_overlay_set(int fd, struct gdm_dss_overlay *req)
{
	return ioctl(fd, GDMFB_OVERLAY_SET, req);
}

static int dss_overlay_play(int fd, struct gdm_dss_overlay_data *req_data,
	struct ody_videoframe *pframe)
{
    req_data->num_plane = 1;
	req_data->data[0].memory_id = pframe->shared_fd;
	req_data->data[0].offset = 0;

	return ioctl(fd, GDMFB_OVERLAY_PLAY, req_data);
}

static int dss_overlay_commit(int fd)
{
	struct gdm_display_commit commit;

	memset(&commit, 0x00, sizeof(commit));

	//return ioctl(fd, GDMFB_OVERLAY_COMMIT, &commit);//
	return ioctl(fd, GDMFB_OVERLAY_COMMIT);
}

static int dss_overlay_unset(int fd, int ov_id)
{
	return ioctl(fd, GDMFB_OVERLAY_UNSET, &ov_id);

}

/////////////////////////////////////////////////////////////
//CMmpRenderer_OdyFpgaDisplay Member Functions


CMmpRenderer_OdyFpgaDisplay::CMmpRenderer_OdyFpgaDisplay(CMmpRendererCreateProp* pRendererProp) :  CMmpRenderer(MMP_MEDIATYPE_VIDEO, pRendererProp)
,m_fd_framebuffer(-1)
,m_buf_ndx(0)
,m_fp_dump(NULL)
{
    memset(&gplayer, 0x00, sizeof(struct ody_player));

}

CMmpRenderer_OdyFpgaDisplay::~CMmpRenderer_OdyFpgaDisplay()
{

}

MMP_RESULT CMmpRenderer_OdyFpgaDisplay::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;

    mmpResult=CMmpRenderer::Open();

	MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_OdyFpgaDisplay::Open] +++ W:%d H:%d "), m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight));

	this->m_fd_framebuffer = open_framebuffer();
	if(this->m_fd_framebuffer < 0) {
		MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_OdyFpgaDisplay::Open] FAIL: open_framebuffer ")));
		mmpResult = MMP_FAILURE;
	}
	else {

		struct ody_framebuffer *pfb;
		struct ody_videofile *pvideo;

		pfb = &gplayer.fb_info;
		pvideo = &gplayer.video_info;

		MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_OdyFpgaDisplay::Open] ln=%d "), __LINE__ ));

		pfb->fd = this->m_fd_framebuffer;
		get_framebuffer(pfb->fd, &pfb->vi, &pfb->fi);

		MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_OdyFpgaDisplay::Open] ln=%d "), __LINE__ ));

		pvideo->fd = -1;
		pvideo->width = m_pRendererProp->m_iPicWidth;
		pvideo->height = m_pRendererProp->m_iPicHeight;
		pvideo->format = VIDEO_FORMAT;

		MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_OdyFpgaDisplay::Open] ln=%d "), __LINE__ ));

		gplayer.frame[0].size = pvideo->width * pvideo->height * 3 / 2;
		gplayer.frame[1].size = gplayer.frame[0].size;

		MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_OdyFpgaDisplay::Open] ln=%d "), __LINE__ ));

		if(alloc_video_memory(&gplayer.frame[0]) != 0) {/* front buffer */
			MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_OdyFpgaDisplay::Open] FAIL: alloc_video_memory 0 ")));
			mmpResult = MMP_FAILURE;
		}

		MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_OdyFpgaDisplay::Open] ln=%d "), __LINE__ ));

		if(alloc_video_memory(&gplayer.frame[1]) != 0) { /* back buffer */
			MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_OdyFpgaDisplay::Open] FAIL: alloc_video_memory 1 ")));
			mmpResult = MMP_FAILURE;
		}

		MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_OdyFpgaDisplay::Open] ln=%d "), __LINE__ ));
	}

	if(mmpResult == MMP_SUCCESS) {

		int ret = 0;

		MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_OdyFpgaDisplay::Open] ln=%d "), __LINE__ ));

		dss_overlay_default_config(&m_request, &gplayer);

		MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_OdyFpgaDisplay::Open] ln=%d "), __LINE__ ));

		ret = dss_overlay_set(gplayer.fb_info.fd, &m_request);
		if(ret){
			MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_OdyFpgaDisplay::Open] FAIL: dss_overlay_set ")));
			mmpResult = MMP_FAILURE;
		}
		else {
			m_req_data.id = m_request.id;
			MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_OdyFpgaDisplay::Open] pipe id is %d "), m_request.id ));
		}

#if (MMPRENDERER_ODYFPGA_DUMP == 1)
        m_fp_dump = fopen("/mnt/dump.yuv", "wb");
#endif

	}

	MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_OdyFpgaDisplay::Open] ln=%d "), __LINE__ ));

	m_luma_size = m_pRendererProp->m_iPicWidth*m_pRendererProp->m_iPicHeight;
	m_chroma_size = m_luma_size/4;


    return mmpResult;
}


MMP_RESULT CMmpRenderer_OdyFpgaDisplay::Close()
{
    MMP_RESULT mmpResult;

    mmpResult=CMmpRenderer::Close();

	if(this->m_fd_framebuffer >= 0) {

		dss_overlay_unset(gplayer.fb_info.fd, m_request.id);

		close(this->m_fd_framebuffer);
		this->m_fd_framebuffer = -1;
	}

    if(m_fp_dump != NULL) {
        fclose(m_fp_dump);
        m_fp_dump = NULL;
    }

    return mmpResult;
}

MMP_RESULT CMmpRenderer_OdyFpgaDisplay::Render(CMmpMediaSampleDecodeResult* pDecResult)
{

    return this->RenderYUV420Planar((MMP_U8*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_Y],
                                    (MMP_U8*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_U],
                                    (MMP_U8*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_V],
								    m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight
								 );
}

MMP_RESULT CMmpRenderer_OdyFpgaDisplay::RenderYUV420Planar(MMP_U8* Y, MMP_U8* U, MMP_U8* V, MMP_U32 buffer_width, MMP_U32 buffer_height) {

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

MMP_RESULT CMmpRenderer_OdyFpgaDisplay::RenderYUV420Planar_Memory(MMP_U8* Y, MMP_U8* U, MMP_U8* V, MMP_U32 buffer_width, MMP_U32 buffer_height) {

    MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_OdyFpgaDisplay::RenderYUV420Planar_mem] +++ ")));

	unsigned char *dest_y, *dest_u, *dest_v;

	dest_y = gplayer.frame[m_buf_ndx].address;
	dest_u = dest_y + m_luma_size;
	dest_v = dest_u + m_chroma_size;

    if(m_fp_dump != NULL) {
        fwrite(Y, 1, m_luma_size, m_fp_dump);
        fwrite(U, 1, m_chroma_size, m_fp_dump);
        fwrite(V, 1, m_chroma_size, m_fp_dump);
    }

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

	dss_overlay_play(gplayer.fb_info.fd, &m_req_data, &gplayer.frame[m_buf_ndx]);
	dss_overlay_commit(gplayer.fb_info.fd);

	m_buf_ndx ^= 1;

	CMmpRenderer::EncodeAndMux(Y, U, V, buffer_width, buffer_height);

	return MMP_SUCCESS;
}


/*
static int dss_overlay_play(int fd, struct gdm_dss_overlay_data *req_data,
	struct ody_videoframe *pframe)
{
	req_data->data.memory_id = pframe->shared_fd;
	req_data->data.offset = 0;

	return ioctl(fd, GDMFB_OVERLAY_PLAY, req_data);
}
*/
#include "vpuapi.h"
MMP_RESULT CMmpRenderer_OdyFpgaDisplay::RenderYUV420Planar_Ion(MMP_U8* Y, MMP_U8* U, MMP_U8* V, MMP_U32 buffer_width, MMP_U32 buffer_height) {


    FrameBuffer* pVPU_FrameBuffer;
	unsigned char *dest_y, *dest_u, *dest_v;

    pVPU_FrameBuffer = (FrameBuffer*)Y;

    MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_OdyFpgaDisplay::RenderYUV420Planar] +++ ION  fd=%d buf(0x%08x 0x%08x 0x%08x , 0x%08x) "),
                          pVPU_FrameBuffer->ion_shared_fd,
                          pVPU_FrameBuffer->bufY, pVPU_FrameBuffer->bufCb, pVPU_FrameBuffer->bufCr,
                          pVPU_FrameBuffer->ion_base_phyaddr));

	//dss_overlay_play(gplayer.fb_info.fd, &m_req_data, &gplayer.frame[m_buf_ndx]);
    m_req_data.num_plane = 1;
    m_req_data.data[0].memory_id = pVPU_FrameBuffer->ion_shared_fd;
	m_req_data.data[0].offset = pVPU_FrameBuffer->bufY - pVPU_FrameBuffer->ion_base_phyaddr;
	ioctl(gplayer.fb_info.fd, GDMFB_OVERLAY_PLAY, &m_req_data);


    dss_overlay_commit(gplayer.fb_info.fd);

#if 0
	if( (m_pVideoEncoder != NULL) && (m_pMuxer != NULL) && (m_p_enc_stream!=NULL) ) {

		dest_y = gplayer.frame[m_buf_ndx].address;
		dest_u = dest_y + m_luma_size;
		dest_v = dest_u + m_chroma_size;

		vdi_read_memory(m_codec_idx, frameBuf.bufY, (unsigned char*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_Y],  luma_size, m_decOP.frameEndian);

		CMmpRenderer::EncodeAndMux(pVPU_FrameBuffer->bufY, pVPU_FrameBuffer->bufCb, pVPU_FrameBuffer->bufCr, buffer_width, buffer_height);

	}
#endif

	m_buf_ndx ^= 1;

	return MMP_SUCCESS;
}


#endif