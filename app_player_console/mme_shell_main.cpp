#include "mme_shell.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>


#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <powermanager/IPowerManager.h>
#include <powermanager/PowerManager.h>
#include <cutils/android_reboot.h>

static int mme_parse_args(char *cmdline, char **argv);
static int command_help(int argc, char **argv);
static int command_test1(int argc, char **argv);
static int command_test2(int argc, char **argv);

struct mme_command {
    char *cmdstr;                     	// command string
    int (*func)(int argc, char **argv);	// execute function
    char *desc;    			            // descriptor
};

static struct mme_command mme_cmds[] = { 
        { (char*)"help" , command_help,  (char*)"display command description"},  
        { (char*)"?"    , command_help,  (char*)"display command description"},
        { (char*)"test1"    , command_test1,  (char*)"test1 "},
        { (char*)"test2"    , command_test2,  (char*)"test2 "},

        /* system */
        { (char*)"syst_reset"   , mme_command_system_reset,  (char*)"system test reset"},
        { (char*)"syst_malloc"  , mme_command_system_memalloc,  (char*)"system test malloc"},
        { (char*)"syst_hardwork"  , mme_command_system_hardwork,  (char*)"system test hardwork"},
        { (char*)"syst_cpuspeed"  , mme_command_system_cpu_speed,  (char*)"system test cpu speed"},
        { (char*)"syst_cpuusage"  , mme_command_system_cpu_usage,  (char*)"system test cpu usage"},
        { (char*)"syst_diskusage"  , mme_command_system_disk_usage,  (char*)"system disk usage"},
        { (char*)"syst_modemlog"  , mme_command_system_modemlog_list,  (char*)"system disk usage"},
        { (char*)"syst_time"  , mme_command_system_time,  (char*)"system disk usage"},

#if (MMP_OS == MMP_OS_LINUX)
        //{ (char*)"syst_clisock"  , mme_command_socket_connect_to_server,  (char*)"connect to server via TCP/IP socket comm"},
#endif
        { (char*)"syst_meminfo"  , mme_command_system_meminfo,  (char*)"/proc/meminfo"},
        { (char*)"syst_tick"  , mme_command_system_checktick,  (char*)"check systick"},
        { (char*)"syst_sleep"  , mme_command_system_check_sleep,  (char*)"check sleep"},
        { (char*)"syst_delay"  , mme_command_system_check_delay,  (char*)"check delay"},
        { (char*)"syst_align"  , mme_command_system_struct_align,  (char*)"check struct align"},
        { (char*)"syst_clk_pr"  , mme_command_system_clock_print,  (char*)"kernel clock print"},
        { (char*)"syst_cpu_stall"  , mme_command_system_force_cpu_stall,  (char*)"force cpu stall"},
        { (char*)"syst_print_clk"  , mme_command_system_print_clock,  (char*)"print clock reg"},
        { (char*)"syst_power"  , mme_command_system_power_ctrl,  (char*)"power on/off"},
        { (char*)"syst_clk"  , mme_command_system_clock_ctrl,  (char*)"clk on/off [clknum]"},
        { (char*)"syst_excep"  , mme_command_system_test_exception_handler,  (char*)"test execp"},
        { (char*)"syst_clk_print"  , mme_command_system_test_clk_print,  (char*)"clk print"},
        
        { (char*)"pm_sleep"  , mme_command_system_pm_sleep,  (char*)"clk print"},
        { (char*)"task_stack"  , mme_command_system_thread_stack,  (char*)"chk task stack"},
        
        /* player */
        { (char*)"play"    , mme_command_player_start,  (char*)"player start"},
        { (char*)"stop"    , mme_command_player_stop,  (char*)"player stop"},
        { (char*)"stop_all"    , mme_command_player_stop_all,  (char*)"all player stop"},
        { (char*)"pause"    , mme_command_player_pause,  (char*)"player pause"},
        { (char*)"resume"    , mme_command_player_resume,  (char*)"player resume"},
        { (char*)"seek"    , mme_command_player_seek,  (char*)"player seek ex)seek [hour] [min] [sec] "},
        { (char*)"stat"    , mme_command_player_status,  (char*)"display play status infomation  "},
        { (char*)"play_loop"    , mme_command_player_loop,  (char*)"player loop "},
        { (char*)"disp"    , mme_command_player_set_first_renderer,  (char*)"set first renderer to display  "},
        { (char*)"rotate"    , mme_command_player_set_rotate,  (char*)"set rotate degree for display "},
        { (char*)"meminfo"    , mme_command_player_meminfo,  (char*)"display player memory info"},
        { (char*)"player_test1"    , mme_command_player_test1,  (char*)"plaer test1"},
        { (char*)"media_info"    , mme_command_player_get_prop,  (char*)"player start"},
        { (char*)"mmh_t1"    , mme_command_player_minmaxheap_test,  (char*)"minmax heap test"},

        { (char*)"mux_d_video"    , mme_command_movie_mux_direct_copy_video,  (char*)"mux video direct"},
        
#if 0
        /* jpeg */
        { (char*)"jplay"    , mme_command_jpegviewer_start,  (char*)"player start"},
        { (char*)"jstop"    , mme_command_jpegviewer_stop,  (char*)"player stop"},
        { (char*)"jstop_all"    , mme_command_jpegviewer_stop_all,  (char*)"all player stop"},
#endif
        
        /* mscaler */
        //{ (char*)"mscaler1"    , mme_command_mscaler_test1,  (char*)"mscaler test1"},
        //{ (char*)"mscaler2"    , mme_command_mscaler_test2,  (char*)"mscaler test2"},
        
        /* csc */
        { (char*)"csc_t1"    , mme_command_csc_test1,  (char*)"csc test1"},
        { (char*)"csc_encdec", mme_command_csc_encdec,  (char*)"csc enc&dec"},
        
        /* i2c codec */
#if (MMESHELL_ALSA_TEST==1)
        { (char*)"i2c2_print"    , mme_command_i2c_codec_print_reg,  (char*)"i2c codec print"},
        { (char*)"i2c2_war"    , mme_command_i2c_codec_write_all_regs,  (char*)"i2c codec write all regs"},
        { (char*)"i2c2_var"    , mme_command_i2c_codec_verify_all_regs,  (char*)"i2c codec verify all regs"},
        { (char*)"i2c2_test"    , mme_command_i2c_codec_write_test,  (char*)"i2c codec print"},
        { (char*)"i2c2_wr"    , mme_command_i2c_codec_write_reg,  (char*)"i2c codec wr reg"},
        { (char*)"i2c2_rd"    , mme_command_i2c_codec_read_reg,  (char*)"i2c codec read reg"},

        { (char*)"astest"    , mme_command_alsa_speaker_test,  (char*)"alsa speaker test"},
#endif
        
#if (MMESHELL_VIDEO_MFC_TEST == 1)
        { (char*)"mfcdec_op"    , mme_command_v4l2_mfc_dec_open,  (char*)"v4l2 videl mfc open"},
        { (char*)"mfcdec_cl"    , mme_command_v4l2_mfc_dec_close,  (char*)"v4l2 videl mfc close"},
#endif
                
        /* ion */
        { (char*)"ion_t1"  , mme_command_ion_test1,  (char*)"ion test1"},
        { (char*)"ion_cache"  , mme_command_ion_cache_oper,  (char*)"ion cache oper"},
        
        /* android shmem */
        { (char*)"ashmem_c"  , mme_command_ashmem_create,  (char*)"ashmem create"},
        { (char*)"ashmem_d"  , mme_command_ashmem_destroy,  (char*)"ashmem destroy"},
        { (char*)"ashmem_p"  , mme_command_ashmem_print,  (char*)"ashmem print"},
        
#if (MMESHELL_ANDORID==1)
        { (char*)"android_disp", android::mme_command_surfaceflinger_color_output,  (char*)"android surfaceflinger color output"},
        { (char*)"android_aout", android::mme_command_audioflinger_pcm_output,  (char*)"android audioflinger pcm out"},
        { (char*)"android_meta", android::mme_command_MediaMetadataRetriever_test1,  (char*)"android IMediaMetaDataRetriever Test"},
        { (char*)"alsa_t1", mme_command_tinyalsa_test1,  (char*)"alsa test 1"},
#ifndef WIN32
        { (char*)"and_snd_t1", mme_command_snd_test1,  (char*)"snd test1"},
#endif
        { (char*)"and_getprop", mme_command_android_system_property_get,  (char*)"android getprop"},
#endif
        
#if (MMESHELL_RIL==1)
        /*ril*/
        { (char*)"ril_init"   , mme_command_ril_init,  (char*)"ril init"},
        { (char*)"ril_deinit"   , mme_command_ril_deinit,  (char*)"ril deinit"},
        { (char*)"ril_radiopower"   , mme_command_ril_set_radio_power,  (char*)"ril radio power on/off (airplane mode)  ex)ril_radiopower on/off"},
        { (char*)"ril_modeminit"   , mme_command_ril_modem_init,  (char*)"ril modem init"},
        { (char*)"ril_sig_stren"   , mme_command_ril_signal_strength,  (char*)"display ril signal strength"},
        { (char*)"ril_mute"   , mme_command_ril_mute,  (char*)"ri_mute  0/1"},
        { (char*)"ril_call"   , mme_command_ril_call,  (char*)"ri_call 01077369829"},
        { (char*)"ril_testmem"   , mme_command_ril_test_request_mem,  (char*)"test req mem"},
#endif

#if (MMESHELL_SECURITY == 1)
        { (char*)"crypto_t1"   , mme_command_security_cryptoapi_test1,  (char*)"cryptoapi_test1"},
        { (char*)"dm_t1"   , mme_command_device_mapper_test1,  (char*)"/dev/device-mapper test1"},
        { (char*)"dm_cc", mme_command_dm_crypto_create,  (char*)"dm-crypto block create"},
        { (char*)"dm_cd", mme_command_dm_crypto_destroy,  (char*)"dm-crypto block destroy"},
        { (char*)"dm_wr", mme_command_dm_crypto_pwrite64_blk,  (char*)"dm-crypto block write"},
        { (char*)"dm_rd", mme_command_dm_crypto_pread64_blk,  (char*)"dm-crypto block write"},
        { (char*)"dm_rt", mme_command_dm_crypto_read_test,  (char*)"dm-crypto block read test"},
        { (char*)"dm_t2", mme_command_dm_crypto_multitask_test,  (char*)"multi read test"},
#endif
};

int command_help(int argc, char **argv) {
  
   int i,j,k,sz;

  sz=sizeof(mme_cmds)/sizeof(mme_cmds[0]);
  for(i=0;i<sz;i++)
  {
        fputc('\r', stdout);
        fputc('\t', stdout);
	 
        k=(int)strlen(mme_cmds[i].cmdstr);
        for(j=0;j<k;j++)
            fputc(mme_cmds[i].cmdstr[j], stdout);
        for(;j<15;j++)
            fputc(' ', stdout); //MTVDEBUG_putchar(' ');
	 
        fputs(mme_cmds[i].desc, stdout);
        fputc('\n', stdout);
        fputc('\r', stdout);
    }
     
    return 0;
}

extern "C" int mme_singleton_mgr_create(void);
extern "C" int mme_singleton_mgr_destroy(void);

static void mme_shell_init() {

    mme_command_system_init(0, NULL);

    mme_singleton_mgr_create();
}

void mme_shell_deinit() {

    mme_command_player_stop_all(0, NULL);
    
    //mme_command_jpegviewer_stop_all(0, NULL);

    mme_command_system_deinit(0, NULL);

#if (MMESHELL_RIL==1)
    mme_command_ril_deinit(0, NULL);
#endif

    mme_singleton_mgr_destroy();
}

void mme_shell_main(int argc_app, char* argv_app[]) {

    int cmdlp, cmdsz;
    int maxbufsize = 4096;
    char* readbuffer = (char*)malloc(maxbufsize);
    int  argc;
    char *argv[32];
	FILE* fp;
	int i;
	int diriect_file_play_mode = 0;

    mme_shell_init();

    cmdsz=sizeof(mme_cmds)/sizeof(mme_cmds[0]);

    printf("------------------------------------ \n");
	printf("  App   : mmeplayer  \n");
    printf("  Build : %s , hthwang@anapass.com \n", __DATE__);
    if(argc_app >= 2) {
        printf("  \targc:%d  argv[0]:%s  argv[1]:%s \n", argc_app, argv_app[0], argv_app[1]);
    }
	printf("------------------------------------ \n");
	
	if(argc_app == 2) {
		fp = fopen(argv_app[1], "rb");
		if(fp!=NULL) {
			fseek(fp, 0, SEEK_END);
			i = ftell(fp);
			if(i > 1024*100) {
				diriect_file_play_mode = 1;
			}
			fclose(fp);
		}
	}

	if(diriect_file_play_mode == 1) {
	
		argv[0] = argv_app[1];
        mme_command_player_direct_start(0xFFFF, argv);
		while(1) {
			CMmpUtil::Sleep(1000*10);	
		}

	}
	else {

		while(1)
		{
			printf("mmeshell>> ");
			fgets(readbuffer, maxbufsize, stdin); 
			argc = mme_parse_args(readbuffer, argv);

			if(argc >= 1)
			{
				cmdlp = 0;
				while( cmdlp<cmdsz )
				{
					if( strcmp( argv[0], mme_cmds[cmdlp].cmdstr ) == 0 )
					{
						if(mme_cmds[cmdlp].func)
						{
							mme_cmds[cmdlp].func( argc, argv );
							printf("\n");
							break;
						}
					}
					cmdlp++;
				}
	         
				if(cmdlp==cmdsz) 
				{
					if( strcmp(argv[0], "exit") == 0 )
					{
						break;
					}
					else
					{
						printf("\tunknown command: %s\n\r", argv[0] );
					}
				}
			}
		}

	}


    free(readbuffer);

    mme_shell_deinit();

    printf("\n\rmme_shell_main bye bye! \n\n\r");
}


static char * ___mme_strtok;
static char * mme_strpbrk( char * cs,char * ct)
{
	const char *sc1,*sc2;

	for( sc1 = cs; *sc1 != '\0'; ++sc1) {
		for( sc2 = ct; *sc2 != '\0'; ++sc2) {
			if (*sc1 == *sc2)
				return (char *) sc1;
		}
	}
	return 0;
}

static char * mme_strtok(char * s,const char * ct)
{
	char *sbegin, *send;

	sbegin  = s ? s : ___mme_strtok;
	if (!sbegin) {
		return 0;
	}
	sbegin += strspn(sbegin,ct);
	if (*sbegin == '\0') {
		___mme_strtok = 0;
		return( 0 );
	}
	send = mme_strpbrk( sbegin, (char*)ct);
	if (send && *send != '\0')
		*send++ = '\0';
	___mme_strtok = send;
	return (sbegin);
}

static const char *mme_delim = " \f\n\r\t\v";
static int mme_parse_args(char *cmdline, char **argv)
{
	char *tok;
	int argc = 0;

	argv[argc] = 0;
                   
	for (tok = mme_strtok(cmdline, mme_delim); tok; tok = mme_strtok(0, mme_delim)) 
    {
		argv[argc++] = tok;
	}

	return argc;
}

int  mme_console_get_number(void)
{
    char buffer[16];
    fgets(buffer, 16, stdin);
    return atoi(buffer);
}


static int command_test1(int argc, char **argv) {

#if 0
    FILE* fp;
    int i, j;
    const int bufsz=0x100000;
    char* buf=(char*)malloc(bufsz);

    fp = fopen("/mnt/abcde.bin", "wb");
    for(i = 0; i < 100; i++) {
        j = fwrite(buf, 1, bufsz, fp);
        printf("%d. sz=%d \n", i, j);
    }
    fclose(fp);
#endif



#if 0
    unsigned int fourcc = 0x32315659;
    printf("fourcc=0x%x %c%c%c%c \n", fourcc, MMPGETFOURCCARG(fourcc) );
    printf("sizeof(time_t)   = %d \n", sizeof(time_t) );
#endif

#if 0
    struct TestA {
        char a;
        int ia;
    };
    printf("size of  = %d \n", sizeof(struct TestA) );
#endif

#if 0

    printf("tmpnam(NULL) = %s \n", tmpnam(NULL) );
    //printf("mkstemp(NULL) = %s \n", mkstemp(NULL) );
    //printf("tmpfile(NULL) = %s \n", tmpfile(NULL) );
    
#endif

#if 0
    pthread_mutex_t initCS;
    int ret;

    printf("initCS=0x%08x \n", initCS );

    ret = pthread_mutex_init(&initCS, NULL);

    printf("initCS=0x%08x ret=%d \n", initCS, ret );

#endif

#if 0
    android::sp<android::IBinder> binder = android::defaultServiceManager()->checkService(android::String16("power"));
    android::sp<android::IPowerManager> mPowerManager;
    mPowerManager = android::interface_cast<android::IPowerManager>(binder);
    android::String16 reason("");
    android::status_t st = mPowerManager->reboot(true, reason, false);
    printf("st = %d \n", st);

#endif

#if 0
    android_reboot(ANDROID_RB_RESTART, 0, 0);

#endif

#if 0
    printf("-ENOMEM=%d \n", -ENOMEM);
    printf("-ENOMEM=0x%x \n", (unsigned int)(-ENOMEM));
#endif

#if 1

    {
        int fd = open("/dev/vpu", O_RDONLY);
        printf("fd=%d \n", fd);
        close(fd);
    }

#endif

    return 0;
}

int Android_LogcatEx1(const char* msg, ... ) {    

    va_list argptr;
    char buffer[10];

    va_start(argptr, msg);
    vsnprintf(buffer, 10, msg,  argptr); /* '-1' mean NULL character */
    va_end(argptr);
        
    printf("%s \n", buffer);
    return 0;
}

static int command_test2(int argc, char **argv) {

#if 0
    char * argvv[10];
    char argv0[32];
    char argv1[32];
    char argv2[32];
    char argv3[32];
    char argv4[32];
    char argv5[32];

    argvv[0] = argv0;
    argvv[1] = argv1;
    argvv[2] = argv2;
    argvv[3] = argv3;
    argvv[4] = argv4;
    argvv[5] = argv5;
    
    strcpy(argv0, "seek");
    strcpy(argv1, "0");
    strcpy(argv2, "0");
    strcpy(argv3, "0");
    strcpy(argv4, "20");
    strcpy(argv5, "0");
    mme_command_player_seek(6, argvv);

    CMmpUtil::Sleep(1000);

    strcpy(argv4, "10");
    mme_command_player_seek(6, argvv);

    CMmpUtil::Sleep(1000);
#endif


#if 0
    Android_LogcatEx1("%d%d%d%d%d", 1, 2, 3, 4, 5);
    Android_LogcatEx1("%s", "123451234512345" );
#endif

#if 1
    for(int iloop = 0; ; iloop++) {
        
        system("settings put global airplane_mode_on 1");
        system("am broadcast -a android.intent.action.AIRPLANE_MODE");
        printf("Airplane Mode On %d\n", iloop);
        sleep(3);

        system("settings put global airplane_mode_on 0");
        system("am broadcast -a android.intent.action.AIRPLANE_MODE");
        printf("Airplane Mode Off %d\n", iloop);
        sleep(10);
    }


#endif

    return 0;
}






