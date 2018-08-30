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

#ifndef MMP_BUFFER_DEF_H__
#define MMP_BUFFER_DEF_H__

#define CLASS_BUFFER_MGR mmp_buffer_mgr_ex1

/*
    Video Frame Buffer 
*/
#define MMP_BUFFER_VIDEOFRAME_MAX_PLANE 3

/*
  FD Start Base
*/
#define MMP_BUFFER_DMA_FD_BASE 0
#define MMP_BUFFER_HEAP_FD_BASE 0x70000000
#define MMP_BUFFER_EXT_FD_BASE  0x71000000

#endif

