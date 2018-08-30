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


#include "MmpRenderer_ALSA.hpp"
#include "MmpUtil.hpp"

#if (MMP_OS == MMP_OS_LINUX_TIZEN)

/////////////////////////////////////////////////////////////
//CMmpRenderer_ALSA Member Functions

static void alsa_error_handler(const char *file, int line, const char *function, int err, const char *format, ...)  {

  char tmp[0xc00];
  va_list va;

  va_start(va, format);
  vsnprintf(tmp, sizeof tmp, format, va);
  va_end(va);
  tmp[sizeof tmp - 1] = '\0';

  if (err)  {
        MMPDEBUGMSG(1, (TEXT("[AO_ALSA] alsa-lib: %s:%i:(%s) %s: %s\n\r"),   file, line, function, tmp, snd_strerror(err) ));
  }
  else  {
        MMPDEBUGMSG(1, (TEXT("[AO_ALSA] alsa-lib: %s:%i:(%s) %s\n\r"),  file, line, function, tmp ));
  }
}


CMmpRenderer_ALSA::CMmpRenderer_ALSA(CMmpRendererCreateProp* pRendererProp) :  CMmpRenderer(MMP_MEDIATYPE_AUDIO, pRendererProp)
    ,m_alsa_hdl(NULL)
    ,m_alsa_hwparams(NULL)
    ,m_alsa_swparams(NULL)
{
    
}

CMmpRenderer_ALSA::~CMmpRenderer_ALSA()
{

}

static int quiet = 0;
static int debugflag = 0;
static int no_check = 0;
static int smixer_level = 0;
static int ignore_error = 0;
static char card[64] = "default";
static struct snd_mixer_selem_regopt smixer_options;
static int ALSA_Mixer_Info(void)
{
	int err;
	snd_ctl_t *handle;
	snd_mixer_t *mhandle;
	snd_ctl_card_info_t *info;
	snd_ctl_elem_list_t *clist;
	snd_ctl_card_info_alloca(&info);
	snd_ctl_elem_list_alloca(&clist);
	
	if ((err = snd_ctl_open(&handle, card, 0)) < 0) {
		printf("Control device %s open error: %s", card, snd_strerror(err));
		return err;
	}
	
	if ((err = snd_ctl_card_info(handle, info)) < 0) {
		printf("Control device %s hw info error: %s", card, snd_strerror(err));
		return err;
	}
	printf("Card %s '%s'/'%s'\n", card, snd_ctl_card_info_get_id(info),
	       snd_ctl_card_info_get_longname(info));
	printf("  Mixer name	: '%s'\n", snd_ctl_card_info_get_mixername(info));
	printf("  Components	: '%s'\n", snd_ctl_card_info_get_components(info));
	if ((err = snd_ctl_elem_list(handle, clist)) < 0) {
		printf("snd_ctl_elem_list failure: %s", snd_strerror(err));
	} else {
		printf("  Controls      : %i\n", snd_ctl_elem_list_get_count(clist));
	}
	snd_ctl_close(handle);
	if ((err = snd_mixer_open(&mhandle, 0)) < 0) {
		printf("Mixer open error: %s", snd_strerror(err));
		return err;
	}
	if (smixer_level == 0 && (err = snd_mixer_attach(mhandle, card)) < 0) {
		printf("Mixer attach %s error: %s", card, snd_strerror(err));
		snd_mixer_close(mhandle);
		return err;
	}
	if ((err = snd_mixer_selem_register(mhandle, smixer_level > 0 ? &smixer_options : NULL, NULL)) < 0) {
		printf("Mixer register error: %s", snd_strerror(err));
		snd_mixer_close(mhandle);
		return err;
	}
	err = snd_mixer_load(mhandle);
	if (err < 0) {
		printf("Mixer load %s error: %s", card, snd_strerror(err));
		snd_mixer_close(mhandle);
		return err;
	}
	printf("  Simple ctrls  : %i\n", snd_mixer_get_count(mhandle));
	//snd_mixer_close(mhandle);
	return 0;
}


MMP_RESULT CMmpRenderer_ALSA::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;
    //MMP_CHAR alsa_name[3][48] = { "default", "plughw:0,0",  "plughw:1,0"};
    MMP_CHAR alsa_name[3][48] = { "plughw:0,0",  "plughw:1,0", "default", };
    MMP_S32 i;
    MMP_S32 iret_alsa = 0;
    int dir, ch, periods;
    unsigned int val;
    
    snd_pcm_uframes_t sample_count_per_frame;
    snd_pcm_uframes_t periodsize;
    snd_pcm_uframes_t exact_buffersize;

    //ALSA_Mixer_Info();

    val = 44100;
    dir = 0;
    ch = 2;
    sample_count_per_frame = 1024; 
    periods = 4;
    
    snd_lib_error_set_handler(alsa_error_handler);

    if(mmpResult == MMP_SUCCESS) {
        for(i = 0; i < 3; i++)  {

            i=0;
            //strcpy(alsa_name[i], "plug:'dmix:RATE=44100'");
            //strcpy(alsa_name[i], "hw:0,0");
            //strcpy(alsa_name[i], "default");
            iret_alsa = snd_pcm_open(&this->m_alsa_hdl, alsa_name[i], SND_PCM_STREAM_PLAYBACK, 0);
            //iret_alsa = snd_pcm_open(&this->m_alsa_hdl, "dmix", SND_PCM_STREAM_PLAYBACK, 0);
            if( iret_alsa < 0 )  {

                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_ALSA::Open] FAIL: snd_pcm_open (err=%d)  alsa_name(%s) ^^ \n\r"), iret_alsa,  alsa_name[i]));
                mmpResult = MMP_FAILURE;
            }
            else
            {
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_ALSA::Open] SUCCESS: snd_pcm_open (err=%d)  alsa_name(%s) OK \n\r"), iret_alsa,  alsa_name[i]));
                break;
            }
        }
        if( i == 3) {
            mmpResult = MMP_FAILURE;
        }
    }

    if(iret_alsa == 0) {
    
        /* 스택에 snd_pcm_hw_param_t 자료구조를 할당 */
        if ((iret_alsa = snd_pcm_hw_params_malloc(&this->m_alsa_hwparams)) < 0)      {
            printf("cannot allocate hardware parameter structure \n");
        }
        else {
            printf("SUCCESS: allocate hardware parameter structure \n");
        }

        /* hwparam를 초기화 */
        if ((iret_alsa = snd_pcm_hw_params_any(this->m_alsa_hdl, this->m_alsa_hwparams)) < 0)  {
            printf("cannot initialize hardware parameter structure\n");
        }
        else {
            printf("SUCCESS: initialize hardware parameter structure\n");
        }
 
        /* 액세스 타입을 설정 */
        if ((iret_alsa = snd_pcm_hw_params_set_access(this->m_alsa_hdl, this->m_alsa_hwparams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)   {
            printf("cannot set access type \n");
        }
        else {
            printf("SUCCESS:  set access type \n");
        }


        /* 지금 출력할 파일이 어떤 형식인지 알아야된다. 8비트인지, 16비트인지, 44100인지 24000인지..
         파일의 오른쪽 버튼으로 속성을 확인한후 값을 넣어주면 된다.
         44100 bits/second sampling rate (CD quality) */

        /* Signed 16-bit little-endian format */
        if ((iret_alsa = snd_pcm_hw_params_set_format(this->m_alsa_hdl, this->m_alsa_hwparams, SND_PCM_FORMAT_S16_LE)) < 0)   {
            printf("cannot set sample format1 \n");
        }
        else {
            printf("SUCCESS:  set sample format1 \n");
        }

        
        if ((iret_alsa = snd_pcm_hw_params_set_rate_near(this->m_alsa_hdl, this->m_alsa_hwparams, &val, &dir)) < 0)  {
            printf("cannot set sample rate1 \n");
        }
        else  {
            printf("[ALSA] ResamplingFreq : %d \n", val);
        }

        if ((iret_alsa = snd_pcm_hw_params_set_channels(this->m_alsa_hdl, this->m_alsa_hwparams, ch)) < 0)   {
            printf("cannot set channel count 1\n");
        }
        else {
            printf("SUCCESS:  set channel count 1\n");
        }

        // Set period size to 32 frames.
        if ((iret_alsa = snd_pcm_hw_params_set_period_size_near(this->m_alsa_hdl, this->m_alsa_hwparams, &sample_count_per_frame, &dir)) < 0)   {
            printf("cannot snd_pcm_hw_params_set_period_size_near \n");
        }
        else {
            printf("SUCCESS: snd_pcm_hw_params_set_period_size_near \n");
        }


//타이젠은 이 함수를 사용하면 안되는 것 같다.  tizen_3.0_work\platform\upstream\alsa-utils\aplay.c 참고함 
#if 0 
        if ((iret_alsa = snd_pcm_hw_params_set_periods(this->m_alsa_hdl, this->m_alsa_hwparams, periods, 0)) < 0)     {
            printf("cannot  snd_pcm_hw_params_set_periods \n");
        }
        else {
            printf("SUCCESS:  snd_pcm_hw_params_set_periods \n");
        }
#endif
 
        exact_buffersize = sample_count_per_frame*periods;
        if ((iret_alsa = snd_pcm_hw_params_set_buffer_size_near(this->m_alsa_hdl, this->m_alsa_hwparams, &exact_buffersize)) < 0)    {
            printf("cannot snd_pcm_hw_params_set_buffer_size_near \n");
        }
        else {
            printf("SUCCESS: snd_pcm_hw_params_set_buffer_size_near \n");
        }
        periodsize = (exact_buffersize << 2) / periods;


        //  Write the parameters to the driver  하드웨어 정보를 pcm장치에 적용한다.
        iret_alsa = snd_pcm_hw_params(this->m_alsa_hdl, this->m_alsa_hwparams);
        if (iret_alsa < 0)      {
            MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_ALSA::Open] FAIL: snd_pcm_hw_params(this->m_alsa_hdl, this->m_alsa_hwparams) \n\r")));
            return MMP_FAILURE;
        }

        printf("snd_pcm_prepare +++  \n");
        snd_pcm_prepare(this->m_alsa_hdl);
        printf("snd_pcm_prepare ---  \n");
    }


    
    return mmpResult;
}

MMP_RESULT CMmpRenderer_ALSA::Close()
{
    if(this->m_alsa_hdl)  {
        snd_pcm_close(this->m_alsa_hdl);
        this->m_alsa_hdl = (snd_pcm_t*)NULL;
    }
    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_ALSA::Render(class mmp_buffer_audioframe* p_buf_af)
{
    //MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[%s::%s] 1. sz = %d"), MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__));
    int  try_frame_count, written_frame_count;
    MMP_U8* pcmdata;

    pcmdata = (MMP_U8*)p_buf_af->get_buf_vir_addr();
    try_frame_count = p_buf_af->get_data_size() >> 2; 

     while(try_frame_count>0)   {

        written_frame_count = snd_pcm_writei(this->m_alsa_hdl, pcmdata, try_frame_count);
        if (written_frame_count >= 0)   {

            //USXDEBUGMSG(1, (TEXT("R1(%d %d)"), try_frame_count ,written_frame_count));
            try_frame_count -= written_frame_count;
            pcmdata += written_frame_count<<2;
        }
        else if (written_frame_count < 0)
        {
            //USXDEBUGMSG(1, (TEXT("R2")));
            snd_pcm_prepare(this->m_alsa_hdl);
            //break;
        }
        //USXDEBUGMSG(1, (TEXT("R3")));

    }

    return MMP_SUCCESS;
}

#endif