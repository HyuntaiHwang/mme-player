#ifndef MME_VPU_COMPILEHELPER_H__
#define MME_VPU_COMPILEHELPER_H__

#define MAX_NUM_INSTANCE   VPU_MAX_NUM_INSTANCE
#define MAX_NUM_VPU_CORE   VPU_MAX_NUM_VPU_CORE

#define MAX_DEC_PIC_WIDTH  VPU_MAX_DEC_PIC_WIDTH
#define MAX_DEC_PIC_HEIGHT VPU_MAX_DEC_PIC_HEIGHT

#define MAX_ENC_PIC_WIDTH  VPU_MAX_ENC_PIC_WIDTH
#define MAX_ENC_PIC_HEIGHT VPU_MAX_ENC_PIC_HEIGHT
#define MIN_ENC_PIC_WIDTH  VPU_MIN_ENC_PIC_WIDTH
#define MIN_ENC_PIC_HEIGHT VPU_MIN_ENC_PIC_HEIGHT

#ifdef __cplusplus
extern "C" {
#endif

unsigned long mmp_env_mgr_get_vpu_ocmmem_phy_addr(void);
int mmp_env_mgr_get_vpu_ocmmem_size(void);

#ifdef __cplusplus
}
#endif

#endif
