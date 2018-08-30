/*
 * vpu.c
 *
 * linux device driver for VPU.
 *
 * Copyright (C) 2006 - 2013  CHIPS&MEDIA INC.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */

#ifndef __VPU_DRV_H__
#define __VPU_DRV_H__

#include <linux/fs.h>
#include <linux/types.h>

#if defined VPU_CONFIG_ION_RESERVED_MEMORY
// ION Feature
#include <linux/dma-buf.h>
#include <linux/memblock.h>
#include "../../../../../../../linux/drivers/staging/android/ion/ion.h"
#include "../../../../../../../linux/drivers/staging/android/sw_sync.h"

#endif

#define VDI_IOCTL_MAGIC  'V'
#define VDI_IOCTL_ALLOCATE_PHYSICAL_MEMORY	_IO(VDI_IOCTL_MAGIC, 0)
#define VDI_IOCTL_FREE_PHYSICALMEMORY		_IO(VDI_IOCTL_MAGIC, 1)
#define VDI_IOCTL_WAIT_INTERRUPT			_IO(VDI_IOCTL_MAGIC, 2)
#define VDI_IOCTL_SET_CLOCK_GATE			_IO(VDI_IOCTL_MAGIC, 3)
#define VDI_IOCTL_RESET                     _IO(VDI_IOCTL_MAGIC, 4)
#define VDI_IOCTL_GET_INSTANCE_POOL			_IO(VDI_IOCTL_MAGIC, 5)
#define VDI_IOCTL_GET_COMMON_MEMORY			_IO(VDI_IOCTL_MAGIC, 6)
#define VDI_IOCTL_GET_RESERVED_VIDEO_MEMORY_INFO _IO(VDI_IOCTL_MAGIC, 8)
#define VDI_IOCTL_OPEN_INSTANCE				_IO(VDI_IOCTL_MAGIC, 9)
#define VDI_IOCTL_CLOSE_INSTANCE			_IO(VDI_IOCTL_MAGIC, 10)
#define VDI_IOCTL_GET_INSTANCE_NUM			_IO(VDI_IOCTL_MAGIC, 11)

#define VDI_IOCTL_GET_ION_PHY_ADDR_WITH_SHARED_FD	_IO(VDI_IOCTL_MAGIC, 30)
#define VDI_IOCTL_GET_ION_PHY_ADDR_WITH_HANDLE		_IO(VDI_IOCTL_MAGIC, 31)
#define VDI_IOCTL_GET_ION_PHY_ADDR_FOR_COMMON_BUFFER _IO(VDI_IOCTL_MAGIC, 32)
#define VDI_IOCTL_GET_DRIVER_OPEN_COUNT              _IO(VDI_IOCTL_MAGIC, 33)

#define VDI_IOCTL_PM_DOMAIN_ON    _IO(VDI_IOCTL_MAGIC, 40)
#define VDI_IOCTL_PM_DOMAIN_OFF   _IO(VDI_IOCTL_MAGIC, 41)
#define VDI_IOCTL_CLK_ON          _IO(VDI_IOCTL_MAGIC, 42)
#define VDI_IOCTL_CLK_OFF         _IO(VDI_IOCTL_MAGIC, 43)

#if defined VPU_CONFIG_ION_RESERVED_MEMORY
// ION Feature
struct vb_dma_buf_data {
	struct ion_handle *ion_handle;
	struct dma_buf *dma_buf;
	struct dma_buf_attachment *attachment;
	struct sg_table *sg_table;
	dma_addr_t dma_addr;
	struct sync_fence *fence;
};

struct gdm_codec_drv_data {
    // ION Feature
    struct ion_client *iclient; /* ION iclient */	 
    struct vb_dma_buf_data dma_buf_data[16];/* ION dma_buf_data */	
    struct platform_device *pdev;
};
#endif

typedef struct vpudrv_buffer_t {
	unsigned int size;
	unsigned int phys_addr;
	unsigned long base;	     /*kernel logical address in use kernel*/
	unsigned long virt_addr; /* virtual user space address */

	unsigned int firmware_code_reuse; // vpu_code_write flag		
	unsigned int ion_shared_fd;    // ion fd buffer index value
} vpudrv_buffer_t;

 
typedef struct vpu_bit_firmware_info_t {
	unsigned int size;		/* size of this structure */
	unsigned int core_idx;
	unsigned int reg_base_offset;
	unsigned short bit_code[512];
} vpu_bit_firmware_info_t;

typedef struct vpudrv_inst_info_t {
	unsigned int core_idx;
	unsigned int inst_idx;
	int inst_open_count;   /* for output only */
} vpudrv_inst_info_t;


#define VPU_DEBUG 1


#define vpu_loge(fmt, ...) printk(KERN_ERR     "[ERROR][" LOG_TAG "]" "[F:%s-L:%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define vpu_logw(fmt, ...) printk(KERN_WARNING "[WARN ][" LOG_TAG "]" "[F:%s-L:%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define vpu_logi(fmt, ...) printk(KERN_WARNING "[INFO ][" LOG_TAG "]" "[F:%s-L:%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#if VPU_DEBUG
#define vpu_logd(fmt, ...) printk(KERN_WARNING "[DEBUG][" LOG_TAG "]" "[F:%s-L:%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define vpu_logd(fmt, ...)
#endif


#endif
