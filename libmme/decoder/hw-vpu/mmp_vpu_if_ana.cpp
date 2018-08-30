/*
 * Copyright (c) 2014 Anapass Co., Ltd.
 *              http://www.anapass.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, Licensed under the Apache License, Version 2.0 (the "License") 
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

#include <signal.h>		/* SIGIO */
#include <fcntl.h>		/* fcntl */
#include <pthread.h>
#include <sys/mman.h>		/* mmap */
#include <sys/ioctl.h>		/* fopen/fread */
#include <sys/errno.h>		/* fopen/fread */
#include <sys/types.h>
#include <sys/time.h>

#include "mmp_vpu_if_ana.hpp"
#include "MmpUtil.hpp"
#include "mmp_vpu_dev.hpp"
#include "mmp_singleton_mgr.hpp"

#include "mmp_vpu_def.h"


/**********************************************************
class members
**********************************************************/


mmp_vpu_if_ana::mmp_vpu_if_ana(struct mmp_vpu_if_create_config* p_create_config) : mmp_vpu_if(p_create_config)
{

    
}

mmp_vpu_if_ana::~mmp_vpu_if_ana() {

}


MMP_RESULT mmp_vpu_if_ana::open() {

	MMP_RESULT mmpResult = MMP_SUCCESS;

    mmpResult = mmp_singleton_mgr::get_result(mmp_singleton_mgr::ID_VPU_DEV);
   
	return mmpResult;
}

MMP_RESULT mmp_vpu_if_ana::close() {
	
	return MMP_SUCCESS;
}

MMP_RESULT mmp_vpu_if_ana::streaming_start(void *vpu_inst) {
    return mmp_vpu_dev::get_instance()->streaming_start(vpu_inst); 
}
	
MMP_RESULT mmp_vpu_if_ana::streaming_stop(void *vpu_inst) {
    return mmp_vpu_dev::get_instance()->streaming_stop(vpu_inst); 
}

MMP_S32 mmp_vpu_if_ana::streaming_ref_count() {
    return mmp_vpu_dev::get_instance()->streaming_ref_count(); 
}

MMP_BOOL mmp_vpu_if_ana::streaming_is_full() {
    return mmp_vpu_dev::get_instance()->streaming_is_full(); 
}

MMP_RESULT mmp_vpu_if_ana::clk_enable() {
    return mmp_vpu_dev::get_instance()->clk_enable();
}
	
MMP_RESULT mmp_vpu_if_ana::clk_disable() {
    return mmp_vpu_dev::get_instance()->clk_disable();
}

Uint32  mmp_vpu_if_ana::VPU_IsBusy(Uint32 coreIdx) { 
    return mmp_vpu_dev::get_instance()->VPU_IsBusy(coreIdx); 
}

Uint32  mmp_vpu_if_ana::VPU_WaitInterrupt(Uint32 coreIdx, 
                                          int timeout) { 
       return mmp_vpu_dev::get_instance()->VPU_WaitInterrupt(coreIdx, timeout); 
}

Uint32  mmp_vpu_if_ana::VPU_IsInit(Uint32 coreIdx) { 
    return mmp_vpu_dev::get_instance()->VPU_IsInit(coreIdx); 
}

RetCode  mmp_vpu_if_ana::VPU_InitWithBitcode(Uint32 coreIdx,	
                                             const Uint16 *bitcode,	
                                             Uint32 size) { 
    return mmp_vpu_dev::get_instance()->VPU_InitWithBitcode(coreIdx, bitcode, size); 
}

RetCode  mmp_vpu_if_ana::VPU_Init(Uint32 coreIdx) { 
    return mmp_vpu_dev::get_instance()->VPU_Init(coreIdx); 
}

void  mmp_vpu_if_ana::VPU_DeInit(Uint32 coreIdx) { 
    mmp_vpu_dev::get_instance()->VPU_DeInit(coreIdx);  
}

Uint32  mmp_vpu_if_ana::VPU_GetOpenInstanceNum(Uint32 coreIdx) { 
   return mmp_vpu_dev::get_instance()->VPU_GetOpenInstanceNum(coreIdx); 
}

RetCode  mmp_vpu_if_ana::VPU_GetVersionInfo(Uint32 coreIdx, 
                                            Uint32 *versionInfo,	
                                            Uint32 *revision, 
                                            Uint32 *productId) { 
  return mmp_vpu_dev::get_instance()->VPU_GetVersionInfo(coreIdx, versionInfo, revision, productId); 
}

void  mmp_vpu_if_ana::VPU_ClearInterrupt(Uint32 coreIdx) { 
    mmp_vpu_dev::get_instance()->VPU_ClearInterrupt(coreIdx); 
}

RetCode  mmp_vpu_if_ana::VPU_SWReset(Uint32 coreIdx,	
                                     int resetMode, 
                                     void *pendingInst) { 
   return mmp_vpu_dev::get_instance()->VPU_SWReset(coreIdx, resetMode, pendingInst); 
}

RetCode  mmp_vpu_if_ana::VPU_HWReset(Uint32 coreIdx) { 
    return mmp_vpu_dev::get_instance()->VPU_HWReset(coreIdx); 
}

RetCode  mmp_vpu_if_ana::VPU_SleepWake(Uint32 coreIdx, int iSleepWake) { 
    return mmp_vpu_dev::get_instance()->VPU_SleepWake(coreIdx, iSleepWake); 
}	

int  mmp_vpu_if_ana::VPU_GetMvColBufSize(CodStd codStd, int width, int height, int num) { 
    return mmp_vpu_dev::get_instance()->VPU_GetMvColBufSize(codStd, width, height, num); 
}

int  mmp_vpu_if_ana::VPU_GetFrameBufSize(int width, int height, int mapType,	int format,	DRAMConfig *pDramCfg) { 
    return mmp_vpu_dev::get_instance()->VPU_GetFrameBufSize(width, height, mapType,	format,	pDramCfg); 
}

// function for decode
RetCode  mmp_vpu_if_ana::VPU_DecOpen(DecHandle *pHandle,	DecOpenParam *pop) { 
    return mmp_vpu_dev::get_instance()->VPU_DecOpen(pHandle, pop); 
}

RetCode  mmp_vpu_if_ana::VPU_DecClose(DecHandle handle) { 
    return mmp_vpu_dev::get_instance()->VPU_DecClose(handle); 
}

RetCode  mmp_vpu_if_ana::VPU_DecSetEscSeqInit(DecHandle handle, int escape) { 
    return mmp_vpu_dev::get_instance()->VPU_DecSetEscSeqInit(handle, escape); 
}

RetCode  mmp_vpu_if_ana::VPU_DecGetInitialInfo(DecHandle handle,	DecInitialInfo *info) { 
    return mmp_vpu_dev::get_instance()->VPU_DecGetInitialInfo(handle, info); 
}

RetCode  mmp_vpu_if_ana::VPU_DecIssueSeqInit(DecHandle handle) { 
    return mmp_vpu_dev::get_instance()->VPU_DecIssueSeqInit(handle); 
}

RetCode  mmp_vpu_if_ana::VPU_DecCompleteSeqInit(DecHandle handle, DecInitialInfo *info) { 
    return mmp_vpu_dev::get_instance()->VPU_DecCompleteSeqInit(handle, info); 
}

RetCode  mmp_vpu_if_ana::VPU_DecRegisterFrameBuffer(DecHandle handle, FrameBuffer *bufArray,	int num, int stride, int height, int mapType) { 
    return mmp_vpu_dev::get_instance()->VPU_DecRegisterFrameBuffer(handle, bufArray, num, stride, height, mapType); 
}

RetCode  mmp_vpu_if_ana::VPU_DecGetFrameBuffer(DecHandle handle, int frameIdx, FrameBuffer *frameBuf) { 
    return mmp_vpu_dev::get_instance()->VPU_DecGetFrameBuffer(handle, frameIdx, frameBuf); 
}

RetCode  mmp_vpu_if_ana::VPU_DecGetBitstreamBuffer(DecHandle handle,	PhysicalAddress *prdPrt, PhysicalAddress *pwrPtr, int *size) { 
    return mmp_vpu_dev::get_instance()->VPU_DecGetBitstreamBuffer(handle, prdPrt, pwrPtr, size); 
}

RetCode  mmp_vpu_if_ana::VPU_DecUpdateBitstreamBuffer(DecHandle handle, int size) { 
    return mmp_vpu_dev::get_instance()->VPU_DecUpdateBitstreamBuffer(handle, size); 
}

RetCode  mmp_vpu_if_ana::VPU_DecStartOneFrame(DecHandle handle, DecParam *param) { 
    return mmp_vpu_dev::get_instance()->VPU_DecStartOneFrame(handle, param); 
}

RetCode  mmp_vpu_if_ana::VPU_DecStartOneFrameAndWaitInterrupt(DecHandle handle, DecParam *param, int timeout) { 
    return mmp_vpu_dev::get_instance()->VPU_DecStartOneFrameAndWaitInterrupt(handle, param, timeout); 
}

RetCode  mmp_vpu_if_ana::VPU_DecGetOutputInfo(DecHandle handle, DecOutputInfo * info) { 
    return mmp_vpu_dev::get_instance()->VPU_DecGetOutputInfo(handle, info); 
}

RetCode  mmp_vpu_if_ana::VPU_DecFrameBufferFlush(DecHandle handle) { 
    return mmp_vpu_dev::get_instance()->VPU_DecFrameBufferFlush( handle);
}

RetCode  mmp_vpu_if_ana::VPU_DecSetRdPtr(DecHandle handle, PhysicalAddress addr,	int updateWrPtr) { 
    return mmp_vpu_dev::get_instance()->VPU_DecSetRdPtr(handle, addr, updateWrPtr); 
}

RetCode  mmp_vpu_if_ana::VPU_DecClrDispFlag(DecHandle handle, int index) { 
    return mmp_vpu_dev::get_instance()->VPU_DecClrDispFlag(handle, index); 
}

RetCode  mmp_vpu_if_ana::VPU_DecGiveCommand(DecHandle handle, CodecCommand cmd, void * param) { 
    return mmp_vpu_dev::get_instance()->VPU_DecGiveCommand(handle, cmd, param);
}

RetCode  mmp_vpu_if_ana::VPU_DecAllocateFrameBuffer(DecHandle handle, FrameBufferAllocInfo info, FrameBuffer *frameBuffer) { 
    return mmp_vpu_dev::get_instance()->VPU_DecAllocateFrameBuffer(handle, info, frameBuffer);
}

// function for encode
RetCode  mmp_vpu_if_ana::VPU_EncOpen(EncHandle *pHandle,	EncOpenParam *pop) { 
    return mmp_vpu_dev::get_instance()->VPU_EncOpen(pHandle, pop);
}

RetCode  mmp_vpu_if_ana::VPU_EncClose(EncHandle handle) { 
    return mmp_vpu_dev::get_instance()->VPU_EncClose(handle);
}

RetCode  mmp_vpu_if_ana::VPU_EncGetInitialInfo(EncHandle handle, EncInitialInfo * info) { 
    return mmp_vpu_dev::get_instance()->VPU_EncGetInitialInfo(handle, info);
}

RetCode  mmp_vpu_if_ana::VPU_EncRegisterFrameBuffer(EncHandle handle, FrameBuffer * bufArray, int num, int stride, int height, int mapType) { 
    return mmp_vpu_dev::get_instance()->VPU_EncRegisterFrameBuffer(handle, bufArray, num, stride, height, mapType);
}

RetCode  mmp_vpu_if_ana::VPU_EncGetFrameBuffer(EncHandle handle, int frameIdx, FrameBuffer *frameBuf) { 
    return mmp_vpu_dev::get_instance()->VPU_EncGetFrameBuffer(handle, frameIdx, frameBuf);
}

RetCode  mmp_vpu_if_ana::VPU_EncGetBitstreamBuffer(EncHandle handle, PhysicalAddress *prdPrt, PhysicalAddress *pwrPtr, int * size) { 
    return mmp_vpu_dev::get_instance()->VPU_EncGetBitstreamBuffer(handle, prdPrt, pwrPtr, size);
}

RetCode  mmp_vpu_if_ana::VPU_EncUpdateBitstreamBuffer(EncHandle handle, int size) { 
    return mmp_vpu_dev::get_instance()->VPU_EncUpdateBitstreamBuffer(handle, size);
}

RetCode  mmp_vpu_if_ana::VPU_EncStartOneFrame(EncHandle handle, EncParam * param) { 
    return mmp_vpu_dev::get_instance()->VPU_EncStartOneFrame(handle, param);
}

RetCode  mmp_vpu_if_ana::VPU_EncGetOutputInfo(EncHandle handle, EncOutputInfo * info) { 
    return mmp_vpu_dev::get_instance()->VPU_EncGetOutputInfo(handle, info);
}

RetCode  mmp_vpu_if_ana::VPU_EncGiveCommand(EncHandle handle, CodecCommand cmd, void * param) { 
    return mmp_vpu_dev::get_instance()->VPU_EncGiveCommand(handle, cmd, param);
}

RetCode  mmp_vpu_if_ana::VPU_EncAllocateFrameBuffer(EncHandle handle, FrameBufferAllocInfo info, FrameBuffer *frameBuffer) { 
    return mmp_vpu_dev::get_instance()->VPU_EncAllocateFrameBuffer(handle, info, frameBuffer);
}

/* etc */
int mmp_vpu_if_ana::vdi_allocate_dma_memory(unsigned long core_idx, vpu_buffer_t *vb) { 
    return mmp_vpu_dev::get_instance()->vdi_allocate_dma_memory(core_idx, vb); 
}

void mmp_vpu_if_ana::vdi_free_dma_memory(unsigned long core_idx, vpu_buffer_t *vb) { 
    mmp_vpu_dev::get_instance()->vdi_free_dma_memory(core_idx, vb);
} 

int mmp_vpu_if_ana::vdi_register_dma_memory(unsigned long core_idx, vpu_buffer_t *vb) { 
    return mmp_vpu_dev::get_instance()->vdi_register_dma_memory(core_idx, vb);
}

void mmp_vpu_if_ana::vdi_unregister_dma_memory(unsigned long core_idx, vpu_buffer_t *vb) { 
    mmp_vpu_dev::get_instance()->vdi_unregister_dma_memory(core_idx, vb);
}

int mmp_vpu_if_ana::WriteBsBufFromBufHelper(Uint32 core_idx,  DecHandle handle,  vpu_buffer_t *pVbStream, BYTE *pChunk, int chunkSize, int endian) { 

    //MMPDEBUGMSG(1, (TEXT("[mmp_vpu_if_ana::WriteBsBufFromBufHelper] ln=%d "), __LINE__));
    return mmp_vpu_dev::get_instance()->WriteBsBufFromBufHelper(core_idx,  handle,  pVbStream, pChunk, chunkSize, endian); 
}

void mmp_vpu_if_ana::enter_critical_section() {

    mmp_vpu_dev::get_instance()->enter_critical_section();
}

void mmp_vpu_if_ana::leave_critical_section() {

    mmp_vpu_dev::get_instance()->leave_critical_section();
}

class mmp_oal_mutex* mmp_vpu_if_ana::get_external_mutex() {
    return mmp_vpu_dev::get_instance()->get_external_mutex();
}

MMP_S32 mmp_vpu_if_ana::VPU_GetCodecInstanceIndex(void* CodecHdl) {

    return mmp_vpu_dev::get_instance()->VPU_GetCodecInstanceIndex(CodecHdl);
}

MMP_S32 mmp_vpu_if_ana::VPU_GetCodecInstanceUse(void* CodecHdl) {

    return mmp_vpu_dev::get_instance()->VPU_GetCodecInstanceUse(CodecHdl);
}
