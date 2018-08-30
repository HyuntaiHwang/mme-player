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

#include"MmpDefine.h"
#include"mmp_object.hpp"

mmp_object::mmp_object()
{
   
}
   
mmp_object::~mmp_object()
{

}


#ifdef MMPOBJECT_OVERLOAD_NEW
void* mmp_object::operator new(size_t num_bytes)
{
   void* rp;
   
   if( CMmpHeap::GetInstance() )   
     rp = CMmpHeap::GetInstance()->Allocate(num_bytes);
   else
     rp = OAL_MemoryAlloc(num_bytes);
          
   return rp;
}

void* mmp_object::operator new[](size_t num_bytes)
{
   void* rp;
   
   if( CMmpHeap::GetInstance() )   
     rp = CMmpHeap::GetInstance()->Allocate(num_bytes);
   else
     rp = OAL_MemoryAlloc(num_bytes);
          
   return rp;
}
   
void mmp_object::operator delete(void* p)
{
   if( CMmpHeap::GetInstance() )   
     CMmpHeap::GetInstance()->Free(p);
   else
     OAL_MemoryFree(p);
}

void mmp_object::operator delete[](void* p)
{
   if( CMmpHeap::GetInstance() )   
     CMmpHeap::GetInstance()->Free(p);
   else
     OAL_MemoryFree(p);
}
#endif

/*
Mmp_RESULT mmp_object::SendEventToHost( int msg )
{
   MmpEvent mtvServiceEvent;
   mtvServiceEvent.nMsg=msg;
   return this->SendEventToHost(&mtvServiceEvent);
}

Mmp_RESULT mmp_object::SendEventToHost( int msg, int par1 )
{
   MmpEvent mtvServiceEvent;
   mtvServiceEvent.nMsg=msg;
   mtvServiceEvent.nPar1=par1;
   return this->SendEventToHost(&mtvServiceEvent);
}

Mmp_RESULT mmp_object::SendEventToHost( const MmpEvent* pMmpServiceEvent )
{
   return Mmp_SUCCESS;
}*/



