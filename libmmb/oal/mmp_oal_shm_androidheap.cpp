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

#include "mmp_oal_shm_androidheap.hpp"

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <binder/IMemory.h>
#include <utils/Timers.h>

#include <utils/Log.h>
#include <stdint.h>
#include <sys/types.h>
#include <binder/MemoryHeapBase.h>
#include <binder/MemoryHeapBase.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>

namespace android {

    enum {
        GET_BUFFER = IBinder::FIRST_CALL_TRANSACTION
    };

    class IEneaBuffer: public IInterface {

        public:
            DECLARE_META_INTERFACE(EneaBuffer);
            virtual sp<IMemoryHeap>           getBuffer() = 0;
    };

    /* --- Server side --- */
    class BnEneaBuffer: public BnInterface<IEneaBuffer>    {
        public:
            virtual status_t onTransact( uint32_t code,
                                  const Parcel& data,
                                  Parcel* reply,
                                  uint32_t flags = 0) {
            
                  switch (code)   {
                    case GET_BUFFER:
                    {
                      CHECK_INTERFACE(IEneaBuffer, data, reply);
                      sp<IMemoryHeap> Data = getBuffer();
                      if (Data != NULL)
                      {
                        reply->writeStrongBinder(Data->asBinder());
                      }
                      return NO_ERROR;
                      break;
                    }
                    default:
                      return BBinder::onTransact(code, data, reply, flags);
                  }
            
            }
    };

    /* --- Client side --- */
    class BpEneaBuffer: public BpInterface<IEneaBuffer>   {
    
        public:
        
            BpEneaBuffer(const sp<IBinder>& impl) : BpInterface<IEneaBuffer>(impl)  {
        
            }

            sp<IMemoryHeap> getBuffer()  {

                Parcel data, reply;
                sp<IMemoryHeap> memHeap = NULL;
                data.writeInterfaceToken(IEneaBuffer::getInterfaceDescriptor());
                // This will result in a call to the onTransact()
                // method on the server in it's context (from it's binder threads)
                remote()->transact(GET_BUFFER, data, &reply);
                memHeap = interface_cast<IMemoryHeap> (reply.readStrongBinder());
                return memHeap;
          }
    };
    

    IMPLEMENT_META_INTERFACE(EneaBuffer, "android.vendor.IEneaBuffer");


    class EneaBufferService : public BnEneaBuffer {
        public:
            static void instantiate();
            EneaBufferService();
            virtual ~EneaBufferService();
            virtual sp<IMemoryHeap> getBuffer();
        private:
            sp<MemoryHeapBase> mMemHeap;
    };

    sp<IMemoryHeap> EneaBufferService::getBuffer()  {
        return mMemHeap;
    }

    void EneaBufferService::instantiate()   {
        status_t status;
        status = defaultServiceManager()->addService(String16("vendor.enea.Buffer"), new EneaBufferService());
    }

    EneaBufferService::EneaBufferService()   {
        //The memory is allocated using a MemoryHeapBase, and thereby is using ashmem
        mMemHeap = new MemoryHeapBase(1024*1024);//MEMORY_SIZE);
        unsigned int *base = (unsigned int *) mMemHeap->getBase();
        *base=0xdeadcafe; //Initiate first value in buffer
    }

    EneaBufferService::~EneaBufferService()
    {
      mMemHeap = 0;
    }

};


/*****************************************************************************
VPU Command 
*****************************************************************************/
#define VPU_DRIVER_NODE "/dev/vpu"
#define VDI_IOCTL_MAGIC  'V'

#define VDI_IOCTL_ALLOC_SHARED_MEMORY _IO(VDI_IOCTL_MAGIC, 100)
#define VDI_IOCTL_FREE_SHARED_MEMORY _IO(VDI_IOCTL_MAGIC, 101)

/**********************************************************
class members
**********************************************************/

mmp_oal_shm_androidheap::mmp_oal_shm_androidheap(struct mmp_oal_shm_create_config* p_create_config) : mmp_oal_shm(p_create_config)
,m_fd(-1)
,m_shm_buf(NULL)
{
    android::sp<android::IServiceManager> sm = android::defaultServiceManager();
    android::sp<android::IBinder> binder;
    int itry;
    
    for(itry = 0; itry < 3; itry++) {
        binder = sm->getService(android::String16("vendor.enea.Buffer"));
        if(binder == NULL) {
             android::EneaBufferService::instantiate();   
        }
        else {
            break;
        }
    }
}

mmp_oal_shm_androidheap::~mmp_oal_shm_androidheap() {

}


MMP_RESULT mmp_oal_shm_androidheap::open() {

	MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 shm_size;
    struct driver_mme_io mme_io;
    int iret;
    
    shm_size = this->get_shm_size();
        
    /* open driver */
    if(mmpResult == MMP_SUCCESS) {
        m_fd = MMP_DRIVER_OPEN("/dev/vpu", O_RDWR);
        if(m_fd < 0) {
            mmpResult = MMP_FAILURE;
        }
    }

    /* alloc virtual buffer */
    if(mmpResult == MMP_SUCCESS) {
        m_shm_buf = new MMP_U8[shm_size+64];
        if(m_shm_buf == NULL) {
            mmpResult = MMP_FAILURE;
        }
    }

    /* alloc driver shared buffer */
    if(mmpResult == MMP_SUCCESS) {
        mme_io.parm[0] = (unsigned int)this;
        mme_io.parm[1] = shm_size;
        mme_io.parm[2] = (unsigned int)m_shm_buf;
        iret = MMP_DRIVER_IOCTL(m_fd, VDI_IOCTL_ALLOC_SHARED_MEMORY, &mme_io);
        if(iret != 0) {
            mmpResult = MMP_FAILURE;
        }
    }
        
    return mmpResult;
}

MMP_RESULT mmp_oal_shm_androidheap::close(MMP_BOOL is_remove_from_system) {
	
    MMP_RESULT mmpResult = MMP_SUCCESS;
    struct driver_mme_io mme_io;
    MMP_S32 shm_size;
    int iret;

    shm_size = this->get_shm_size();

    /* free driver shared buffer */
    if(m_fd>=0) {
        mme_io.parm[0] = (unsigned int)this;
        mme_io.parm[1] = shm_size;
        mme_io.parm[2] = (unsigned int)m_shm_buf;
        iret = MMP_DRIVER_IOCTL(m_fd, VDI_IOCTL_FREE_SHARED_MEMORY, &mme_io);
        if(iret != 0) {
            mmpResult = MMP_FAILURE;
        }
    }
    
    /* free buffer */
    if(m_shm_buf != NULL) {
        delete [] m_shm_buf;
        m_shm_buf = NULL;
    }

    
	return mmpResult;
}

MMP_S32 mmp_oal_shm_androidheap::get_attach_process_count() {
    
    MMP_S32 proc_count = 0;
    return proc_count;
}

