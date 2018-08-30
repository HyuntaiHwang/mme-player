//------------------------------------------------------------------------------
// File: vdi.h
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------

#ifndef _VDI_H_
#define _VDI_H_

#ifdef CNM_HISI_PLATFORM
#include <linux/fb.h>
#endif
#include "../vpuapi/vpuconfig.h"
#include "../vpuapi/regdefine.h"
#include "mm.h"


#define SUPPORT_MULTI_CORE_IN_ONE_DRIVER
#define MAX_VPU_CORE_NUM MAX_NUM_VPU_CORE
#define MAX_VPU_BUFFER_POOL (64*VPU_MAX_NUM_INSTANCE)
#ifdef CNM_HISI_PLATFORM
/* Added by d00206042 2014-04-26 for SYSMMU start */
#define VPU_RESERVED_MEMORY_SIZE (49 * 1024 * 1024)
/* Added by d00206042 2014-04-26 for SYSMMU end */
#endif //CNM_HISI_PLATFORM


#define VpuWriteReg( CORE, ADDR, DATA )					vdi_write_register( CORE, ADDR, DATA )					// system register write
#define VpuReadReg( CORE, ADDR )			            vdi_read_register( CORE, ADDR )							// system register write
#define VpuWriteMem( CORE, ADDR, DATA, LEN, ENDIAN )	vdi_write_memory( CORE, ADDR, DATA, LEN, ENDIAN )		// system memory write
#define VpuReadMem( CORE, ADDR, DATA, LEN, ENDIAN )		vdi_read_memory( CORE, ADDR, DATA, LEN, ENDIAN )		// system memory write



typedef struct vpu_buffer_t {
	unsigned int size;
	unsigned int  phys_addr;	
#ifdef CNM_HISI_PLATFORM	
	unsigned long long base;
#else
	unsigned long base;
#endif
	unsigned long virt_addr;
    unsigned int firmware_code_reuse; // vpu_code_write flag		
} vpu_buffer_t;

#ifdef CNM_HISI_PLATFORM
/* add by y00251056 at 2014-5-9 for sram  start */
typedef struct SramVdiInfo {
   unsigned int sram_size;
   unsigned long phys_addr;
   unsigned int is_allocated;
} SramVdiInfo;
/* add by y00251056 at 2014-5-9 for sram  end */
#endif //CNM_HISI_PLATFORM

typedef enum {
	VDI_LITTLE_ENDIAN = 0,
	VDI_BIG_ENDIAN,
	VDI_32BIT_LITTLE_ENDIAN,
	VDI_32BIT_BIG_ENDIAN,
} EndianMode;



typedef enum {
	VDI_LINEAR_FRAME_MAP  = 0,
	VDI_TILED_FRAME_V_MAP = 1,
	VDI_TILED_FRAME_H_MAP = 2,
	VDI_TILED_FIELD_V_MAP = 3,
	VDI_TILED_MIXED_V_MAP = 4,
	VDI_TILED_FRAME_MB_RASTER_MAP = 5,
	VDI_TILED_FIELD_MB_RASTER_MAP = 6,
	VDI_TILED_FRAME_NO_BANK_MAP = 7,
	VDI_TILED_FIELD_NO_BANK_MAP = 8,
	VDI_LINEAR_FIELD_MAP  = 9,
	VDI_TILED_MAP_TYPE_MAX
} vdi_gdi_tiled_map;

typedef struct vpu_instance_pool_t {
	unsigned char codecInstPool[VPU_MAX_NUM_INSTANCE][VPU_MAX_INST_HANDLE_SIZE];	// Since VDI don't know the size of CodecInst structure, VDI should have the enough space not to overflow.
	int vpu_instance_num;
	void* vpu_mutex;
	void* vpu_omx_mutex;
	void* vpu_disp_mutex;
	void* vmem_mutex;
	int instance_pool_inited;
	void *pendingInst;
	int pendingInstIdxPlus1;
	vpu_buffer_t vpu_common_buffer;
	int vpu_clock_state;
	video_mm_t vmem;	    
} vpu_instance_pool_t;


#if defined (__cplusplus)
extern "C" {
#endif
	int vdi_probe(unsigned long core_idx);
	int vdi_init(unsigned long core_idx);
#ifdef __VPU_PLATFORM_MME
    int vdi_init_shm(unsigned long core_idx);
#endif
	int vdi_release(unsigned long core_idx);	//this function may be called only at system off.

	vpu_instance_pool_t * vdi_get_instance_pool(unsigned long core_idx);
#ifdef __VPU_PLATFORM_MME
    vpu_instance_pool_t * vdi_get_instance_pool_shm(unsigned long core_idx);
#endif
	int vdi_get_common_memory(unsigned long core_idx, vpu_buffer_t *vb);
	int vdi_allocate_dma_memory(unsigned long core_idx, vpu_buffer_t *vb);
	int vdi_attach_dma_memory(unsigned long core_idx, vpu_buffer_t *vb);
	void vdi_free_dma_memory(unsigned long core_idx, vpu_buffer_t *vb);
	int vdi_get_sram_memory(unsigned long core_idx, vpu_buffer_t *vb);
	int vdi_dettach_dma_memory(unsigned long core_idx, vpu_buffer_t *vb);

#ifdef __VPU_PLATFORM_MME
    //add by hthwnag
    int vdi_register_dma_memory(unsigned long coreIdx, vpu_buffer_t *vb);
    void vdi_unregister_dma_memory(unsigned long coreIdx, vpu_buffer_t *vb);
#endif

	int vdi_wait_interrupt(unsigned long core_idx, int timeout, unsigned int addr_bit_int_reason);
	int vdi_wait_vpu_busy(unsigned long core_idx, int timeout, unsigned int addr_bit_busy_flag);
	int vdi_wait_bus_busy(unsigned long coreIdx, int timeout, unsigned int gdi_busy_flag);
	int vdi_hw_reset(unsigned long core_idx);

#ifdef __VPU_PLATFORM_MME
	int vdi_set_clock_gate(unsigned long core_idx, int enable);
	int vdi_get_clock_gate(unsigned long core_idx);
#endif

	int  vdi_get_instance_num(unsigned long core_idx);
	int vdi_write_register(unsigned long core_idx, unsigned int addr, unsigned int data);
	unsigned int vdi_read_register(unsigned long core_idx, unsigned int addr);

	int vdi_write_memory(unsigned long core_idx, unsigned int addr, unsigned char *data, int len, int endian);
	int vdi_read_memory(unsigned long core_idx, unsigned int addr, unsigned char *data, int len, int endian);

	int vdi_lock(unsigned long core_idx);
	void vdi_unlock(unsigned long core_idx);
	int vdi_disp_lock(unsigned long core_idx);
	void vdi_disp_unlock(unsigned long core_idx);
#ifdef CNM_HISI_PLATFORM
	int vdi_get_screen_info(struct fb_var_screeninfo* var);
#endif
#if defined(linux) || defined(__linux) || defined(ANDROID)
#else	
	int vmem_lock(unsigned long coreIdx);
	void vmem_unlock(unsigned long coreIdx);
#endif
    void vdi_set_sdram(unsigned long core_idx, unsigned int addr, int len, unsigned char data, int endian);
	void vdi_log(unsigned long coreIdx, int cmd, int step);
	int vdi_open_instance(unsigned long coreIdx, unsigned long instIdx);
	int vdi_close_instance(unsigned long coreIdx, unsigned long instIdx);
	int vdi_set_bit_firmware_to_pm(unsigned long coreIdx, const unsigned short *code);

#ifdef CNM_HISI_PLATFORM
    /* Added by d00206042 2014-04-26 for SYSMMU start */
    int vdi_flush_ion_cache(unsigned long coreIdx);
    /* Added by d00206042 2014-04-26 for SYSMMU end */
    /* add by y00251056 at 2014-5-9 for sram  start */
    int vdi_allocate_sram_memory(unsigned int coreIdx, unsigned int memSize);
    int vdi_free_sram_memory(unsigned int coreIdx);
    /* add by y00251056 at 2014-5-9 for sram  end */
	/* add by y00251056 at 2014-7-31  start */
	int vdi_get_memory_virtual_addr(unsigned long coreIdx, unsigned int addr, vpu_buffer_t *outPutVdb);
	/* add by y00251056 at 2014-7-31  end */
#endif //CNM_HISI_PLATFORM

#ifdef CNM_FPGA_PLATFORM
#define HPI_SET_TIMING_MAX 1000
	void vdi_init_fpga(int core_idx);
	int vdi_set_timing_opt(unsigned long core_idx);
	int vdi_set_clock_freg(unsigned long core_idx, int Device, int OutFreqMHz, int InFreqMHz);
	int vdi_load_prom_via_usb(unsigned long core_idx, int need_init, const char* path);
#endif

#if defined (__cplusplus)
}
#endif

#endif //#ifndef _VDI_H_

