#ifndef MMESHELL_H__
#define MMESHELL_H__

#include "MmpDefine.h"
#include "MmpDemuxer.hpp"
#include "MmpMuxer.hpp"
#include "MmpDecoderVideo.hpp"
#include "MmpEncoderVideo.hpp"
#include "MmpRenderer.hpp"
#include "MmpUtil.hpp"
#include "MmpPlayer.hpp"


#define MMESHELL_RIL     0
#define MMESHELL_SYSTEM  1
#define MMESHELL_PLAYER  1

#if (MMP_PLATFORM == MMP_PLATFORM_ANDROID)
#define MMESHELL_ANDORID  1
#elif (MMP_PLATFORM == MMP_PLATFORM_WIN32)
#define MMESHELL_ANDORID  0
#else
#error "ERROR : Select Platform"
#endif

#if (MMP_PLATFORM == MMP_PLATFORM_WIN32)
#define MMESHELL_SECURITY  0
#elif (MMP_PLATFORM == MMP_PLATFORM_ANDROID)
#define MMESHELL_SECURITY  1
#else
#error "ERROR : Select Platform"
#endif

#if (MMP_PLATFORM == MMP_PLATFORM_TIZEN)
#define MMESHELL_ALSA_TEST  1
#elif (MMP_PLATFORM == MMP_PLATFORM_ANDROID)
#define MMESHELL_ALSA_TEST  1
#elif (MMP_PLATFORM == MMP_PLATFORM_WIN32)
#define MMESHELL_ALSA_TEST  0
#else
#error "ERROR : Select Platform"
#endif

#if ((MMP_PLATFORM == MMP_PLATFORM_LINUX_TIZEN) || (MMP_PLATFORM == MMP_PLATFORM_WIN32) )
#define MMESHELL_VIDEO_MFC_TEST  1
#elif (MMP_PLATFORM == MMP_PLATFORM_ANDROID )
#define MMESHELL_VIDEO_MFC_TEST  0
#else
#error "ERROR : Select Platform"
#endif

#define MMESHELL_INFO  1
#define MMESHELL_ERROR 1
#if (MMP_PLATFORM == MMP_PLATFORM_WIN32)
//#define MMESHELL_PRINT(cond,printf_exp) do { if(cond) CMmpUtil::Printf printf_exp; }while(0);
#define MMESHELL_PRINT(cond,printf_exp) do { if(cond) printf printf_exp; }while(0);
#else
#define MMESHELL_PRINT(cond,printf_exp) do { if(cond) printf printf_exp; }while(0);
#endif

#ifdef __cplusplus
extern "C" {
#endif

void mme_shell_main(int argc_app, char* argv_app[]);
void mme_shell_deinit(void);

int  mme_console_get_number(void);

/* player */
int mme_command_player_start(int argc, char* argv[]);
int mme_command_player_direct_start(int argc, char* argv[]);
int mme_command_player_stop(int argc, char* argv[]);
int mme_command_player_pause(int argc, char* argv[]);
int mme_command_player_resume(int argc, char* argv[]);
int mme_command_player_stop_all(int argc, char* argv[]);
int mme_command_player_seek(int argc, char* argv[]);
int mme_command_player_status(int argc, char* argv[]);
int mme_command_player_loop(int argc, char* argv[]);
int mme_command_player_set_first_renderer(int argc, char* argv[]);
int mme_command_player_set_rotate(int argc, char* argv[]);
int mme_command_player_meminfo(int argc, char* argv[]);
int mme_command_player_test1(int argc, char* argv[]);
int mme_command_player_get_prop(int argc, char* argv[]);
int mme_command_player_minmaxheap_test(int argc, char* argv[]);

/* encoder */
int mme_command_encoder_test01(int argc, char* argv[]);
int mme_command_player_enc_start(int argc, char* argv[]);
int mme_command_player_enc_stop(int argc, char* argv[]);

/* muxing */
int mme_command_movie_mux_direct_copy_video(int argc, char* argv[]);

/* jpeg */
int mme_command_jpegviewer_start(int argc, char* argv[]);
int mme_command_jpegviewer_stop(int argc, char* argv[]);
int mme_command_jpegviewer_stop_all(int argc, char* argv[]);

/* mscaler */
int mme_command_mscaler_test1(int argc, char* argv[]);
int mme_command_mscaler_test2(int argc, char* argv[]);

/* i2c codec */
#if (MMESHELL_ALSA_TEST == 1)
int mme_command_i2c_codec_print_reg(int argc, char* argv[]);
int mme_command_i2c_codec_write_all_regs(int argc, char* argv[]); 
int mme_command_i2c_codec_verify_all_regs(int argc, char* argv[]);
int mme_command_i2c_codec_write_test(int argc, char* argv[]);
int mme_command_i2c_codec_write_reg(int argc, char* argv[]); 
int mme_command_i2c_codec_read_reg(int argc, char* argv[]); 

int mme_command_alsa_speaker_test(int argc, char* argv[]);

#endif

/* vpu */
int mme_command_vpu_load(int argc, char* argv[]);
int mme_command_vpu_unload(int argc, char* argv[]);
int mme_command_vpu_test1(int argc, char* argv[]);
int mme_command_vpu_test2(int argc, char* argv[]);
int mme_command_vpu_test3(int argc, char* argv[]);

/* system */
int mme_command_system_init(int argc, char* argv[]);
int mme_command_system_deinit(int argc, char* argv[]);
int mme_command_system_reset(int argc, char* argv[]);
int mme_command_system_memalloc(int argc, char* argv[]);
int mme_command_system_hardwork(int argc, char* argv[]);
int mme_command_system_meminfo(int argc, char* argv[]);
int mme_command_system_checktick(int argc, char* argv[]);
int mme_command_system_check_sleep(int argc, char* argv[]);
int mme_command_system_check_delay(int argc, char* argv[]);
int mme_command_system_struct_align(int argc, char* argv[]);
int mme_command_system_clock_print(int argc, char* argv[]);
int mme_command_system_cpu_speed(int argc, char* argv[]);
int mme_command_system_cpu_usage(int argc, char* argv[]);
int mme_command_system_disk_usage(int argc, char* argv[]);
int mme_command_system_time(int argc, char* argv[]);
int mme_command_system_modemlog_list(int argc, char* argv[]);
int mme_command_system_print_clock(int argc, char* argv[]);
int mme_command_system_power_ctrl(int argc, char* argv[]);
int mme_command_system_clock_ctrl(int argc, char* argv[]);
int mme_command_system_test_exception_handler(int argc, char* argv[]);
int mme_command_system_test_clk_print(int argc, char* argv[]);

int mme_command_socket_connect_to_server(int argc, char* argv[]);

int mme_command_system_force_cpu_stall(int argc, char* argv[]);
int mme_command_system_pm_sleep(int argc, char* argv[]);
int mme_command_system_memcpy_speed(int argc, char* argv[]);
int mme_command_system_thread_stack(int argc, char* argv[]);

/* ion */
int mme_command_ion_test1(int argc, char* argv[]);
int mme_command_ion_alloc_fd(int argc, char* argv[]);
int mme_command_ion_free_fd(int argc, char* argv[]);
int mme_command_ion_import(int argc, char* argv[]);
int mme_command_ion_phy_to_vir(int argc, char* argv[]);
int mme_command_ion_cache_oper(int argc, char* argv[]);

/* android shared memory */
int mme_command_ashmem_create(int argc, char* argv[]);
int mme_command_ashmem_destroy(int argc, char* argv[]);
int mme_command_ashmem_print(int argc, char* argv[]);

/* csc */
int mme_command_csc_test1(int argc, char* argv[]);
int mme_command_csc_encdec(int argc, char* argv[]);

/* android */
namespace android {
int mme_command_surfaceflinger_color_output(int argc, char* argv[]);
int mme_command_audioflinger_pcm_output(int argc, char* argv[]);
int mme_command_MediaMetadataRetriever_test1(int argc, char* argv[]);
}

int mme_command_tinyalsa_test1(int argc, char* argv[]);
int mme_command_snd_test1(int argc, char* argv[]);
int mme_command_android_system_property_get(int argc, char* argv[]);

/* Video MFC */
int mme_command_v4l2_mfc_dec_open(int argc, char* argv[]);
int mme_command_v4l2_mfc_dec_close(int argc, char* argv[]);

/* ril */
int mme_command_ril_init(int argc, char* argv[]);
int mme_command_ril_deinit(int argc, char* argv[]);
int mme_command_ril_set_radio_power(int argc, char* argv[]); /* airplane mode on/off */
int mme_command_ril_modem_init(int argc, char* argv[]); 
int mme_command_ril_signal_strength(int argc, char* argv[]);
int mme_command_ril_mute(int argc, char* argv[]);
int mme_command_ril_call(int argc, char* argv[]);
int mme_command_ril_test_request_mem(int argc, char* argv[]);


/* security */
int mme_command_security_cryptoapi_test1(int argc, char* argv[]);
int mme_command_device_mapper_test1(int argc, char* argv[]);
int mme_command_dm_crypto_create(int argc, char* argv[]);
int mme_command_dm_crypto_destroy(int argc, char* argv[]);
int mme_command_dm_crypto_pwrite64_blk(int argc, char* argv[]);
int mme_command_dm_crypto_pread64_blk(int argc, char* argv[]);
int mme_command_dm_crypto_read_blk(int argc, char* argv[]);
int mme_command_dm_crypto_read_test(int argc, char* argv[]);

int mme_command_dm_crypto_multitask_test(int argc, char* argv[]);

#ifdef __cplusplus
}
#endif


#endif
