#ifndef VPUAPI_MME_H__
#define VPUAPI_MME_H__

#include "vpuapi.h"

struct vpu_codec_info {
    int index;
    int use;
    int is_decoder;
    unsigned int format;
};

#ifdef __cplusplus
extern "C" {
#endif

RetCode VPU_Init_Shm(Uint32 coreIdx);

int VPU_GetCodecInstanceIndex(void* CodecHdl);
int VPU_GetCodecInstanceUse(void* CodecHdl);

void VPU_GetCodecInfo(int idx, struct mmp_video_hw_codec_instance_info *p_info);

#ifdef __cplusplus
}
#endif

#endif

