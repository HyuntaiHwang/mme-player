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

#ifndef MMP_VPU_IF_HPP__
#define MMP_VPU_IF_HPP__

#include "mmp_vpu_def.h"

struct mmp_vpu_if_create_config {
	MMP_S32 dummy;
};

class mmp_vpu_if {

public:
    static class mmp_vpu_if* create_object();
	static class mmp_vpu_if* create_object(struct mmp_vpu_if_create_config* p_create_config);
	static MMP_RESULT destroy_object(class mmp_vpu_if* p_obj);

protected:

    struct mmp_vpu_if_create_config m_create_config;

protected:
    mmp_vpu_if(struct mmp_vpu_if_create_config* p_create_config);
    virtual ~mmp_vpu_if();

    virtual MMP_RESULT open()=0;
    virtual MMP_RESULT close()=0;

public:
    virtual MMP_RESULT  streaming_start(void *vpu_inst)=0;
	virtual MMP_RESULT  streaming_stop(void *vpu_inst)=0;
	virtual MMP_S32  streaming_ref_count()=0;
	virtual MMP_BOOL  streaming_is_full()=0;
	
    virtual MMP_RESULT  clk_enable()=0;
	virtual MMP_RESULT  clk_disable()=0;
	
    virtual Uint32  VPU_IsBusy(Uint32 coreIdx) = 0;
	virtual Uint32  VPU_WaitInterrupt(Uint32 coreIdx, int timeout) = 0;
	virtual Uint32  VPU_IsInit(Uint32 coreIdx) = 0;
	virtual RetCode VPU_Init(Uint32 coreIdx) = 0;
	virtual RetCode VPU_InitWithBitcode(Uint32 coreIdx,	const Uint16 *bitcode,	Uint32 size) = 0;
	virtual void    VPU_DeInit(Uint32 coreIdx) = 0;
	virtual Uint32  VPU_GetOpenInstanceNum(Uint32 coreIdx) = 0;
	virtual RetCode VPU_GetVersionInfo(Uint32 coreIdx, Uint32 *versionInfo,	Uint32 *revision, Uint32 *productId) = 0;
	virtual void    VPU_ClearInterrupt(Uint32 coreIdx) = 0;
	virtual RetCode VPU_SWReset(Uint32 coreIdx,	int resetMode, void *pendingInst) = 0;
	virtual RetCode VPU_HWReset(Uint32 coreIdx) = 0;
	virtual RetCode VPU_SleepWake(Uint32 coreIdx, int iSleepWake) = 0;	
	virtual int     VPU_GetMvColBufSize(CodStd codStd, int width, int height, int num) = 0;
	virtual int     VPU_GetFrameBufSize(int width, int height, int mapType,	int format,	DRAMConfig *pDramCfg) = 0;
	
    // function for decode
	virtual RetCode VPU_DecOpen(DecHandle *pHandle,	DecOpenParam *pop) = 0;
	virtual RetCode VPU_DecClose(DecHandle handle) = 0;
	virtual RetCode VPU_DecSetEscSeqInit(DecHandle handle, int escape) = 0;
	virtual RetCode VPU_DecGetInitialInfo(DecHandle handle,	DecInitialInfo *info) = 0;
	virtual RetCode VPU_DecIssueSeqInit(DecHandle handle) = 0;
	virtual RetCode VPU_DecCompleteSeqInit(DecHandle handle, DecInitialInfo *info) = 0;
	virtual RetCode VPU_DecRegisterFrameBuffer(DecHandle handle, FrameBuffer *bufArray,	int num, int stride, int height, int mapType) = 0;
	virtual RetCode VPU_DecGetFrameBuffer(DecHandle handle,	int frameIdx, FrameBuffer *frameBuf) = 0;
	virtual RetCode VPU_DecGetBitstreamBuffer(DecHandle handle,	PhysicalAddress *prdPrt, PhysicalAddress *pwrPtr, int *size) = 0;
	virtual RetCode VPU_DecUpdateBitstreamBuffer(DecHandle handle, int size) = 0;
	virtual RetCode VPU_DecStartOneFrame(DecHandle handle, DecParam *param) = 0;
	virtual RetCode VPU_DecStartOneFrameAndWaitInterrupt(DecHandle handle, DecParam *param, int timeout) = 0;
	virtual RetCode VPU_DecGetOutputInfo(DecHandle handle, DecOutputInfo * info) = 0;
	virtual RetCode VPU_DecFrameBufferFlush(DecHandle handle) = 0;
	virtual RetCode VPU_DecSetRdPtr(DecHandle handle, PhysicalAddress addr,	int updateWrPtr) = 0;
	virtual RetCode VPU_DecClrDispFlag(DecHandle handle, int index) = 0;
	virtual RetCode VPU_DecGiveCommand(DecHandle handle, CodecCommand cmd, void * param) = 0;
	virtual RetCode VPU_DecAllocateFrameBuffer(DecHandle handle, FrameBufferAllocInfo info,	FrameBuffer *frameBuffer) = 0;
	
    // function for encode
	virtual RetCode VPU_EncOpen(EncHandle *pHandle,	EncOpenParam *pop) = 0;
	virtual RetCode VPU_EncClose(EncHandle handle) = 0;
	virtual RetCode VPU_EncGetInitialInfo(EncHandle handle,	EncInitialInfo * info) = 0;
	virtual RetCode VPU_EncRegisterFrameBuffer(EncHandle handle, FrameBuffer * bufArray, int num, int stride, int height, int mapType) = 0;
	virtual RetCode VPU_EncGetFrameBuffer(EncHandle handle,	int frameIdx, FrameBuffer *frameBuf) = 0;
	virtual RetCode VPU_EncGetBitstreamBuffer(EncHandle handle,	PhysicalAddress *prdPrt, PhysicalAddress *pwrPtr, int * size) = 0;
	virtual RetCode VPU_EncUpdateBitstreamBuffer(EncHandle handle, int size) = 0;
	virtual RetCode VPU_EncStartOneFrame(EncHandle handle, EncParam * param) = 0;
	virtual RetCode VPU_EncGetOutputInfo(EncHandle handle, EncOutputInfo * info) = 0;
	virtual RetCode VPU_EncGiveCommand(EncHandle handle, CodecCommand cmd, void * param) = 0;
	virtual RetCode VPU_EncAllocateFrameBuffer(EncHandle handle, FrameBufferAllocInfo info,	FrameBuffer *frameBuffer) = 0;

    /* etc */
    virtual int vdi_allocate_dma_memory(unsigned long core_idx, vpu_buffer_t *vb) = 0;
    virtual void vdi_free_dma_memory(unsigned long core_idx, vpu_buffer_t *vb) = 0;

    virtual int vdi_register_dma_memory(unsigned long core_idx, vpu_buffer_t *vb) = 0;
    virtual void vdi_unregister_dma_memory(unsigned long core_idx, vpu_buffer_t *vb) = 0;

    virtual int WriteBsBufFromBufHelper(Uint32 core_idx,  DecHandle handle,  vpu_buffer_t *pVbStream, BYTE *pChunk, int chunkSize, int endian) = 0;

    virtual void enter_critical_section() = 0;
    virtual void leave_critical_section() = 0;
    virtual class mmp_oal_mutex* get_external_mutex() = 0;

    virtual MMP_S32 VPU_GetCodecInstanceIndex(void* CodecHdl) = 0;
    virtual MMP_S32 VPU_GetCodecInstanceUse(void* CodecHdl) = 0;

};

#endif
