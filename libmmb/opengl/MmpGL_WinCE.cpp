
#include "MmpDefine.h"
#include "MmpGL_WinCE.hpp"

#if (MMP_OS==MMP_OS_WINCE60)

CMmpGL_WinCE::CMmpGL_WinCE(HWND hWnd, HDC hDC) : CMmpGL(hWnd, hDC)
,m_eglDisplay(NULL)
,m_eglConfig(NULL)
,m_eglContext(NULL)
,m_eglConfigArr(NULL)
,m_eglSurface(NULL)
{
}

CMmpGL_WinCE::~CMmpGL_WinCE(void)
{
	
}

MMP_RESULT CMmpGL_WinCE::Open()
{
    MMP_RESULT mmpResult;
    int i;
    EGLint      major, minor, num_config, max_num_config;

    EGLint      attrib_list[] = {
                            EGL_RED_SIZE, 5,
                            EGL_GREEN_SIZE, 6,
                            EGL_BLUE_SIZE, 5, 
                            EGL_ALPHA_SIZE, 0,
                            EGL_RENDERABLE_TYPE,  EGL_OPENGL_ES_BIT, 
                            EGL_SURFACE_TYPE,  EGL_WINDOW_BIT,  EGL_NONE };


    m_eglDisplay = eglGetDisplay(m_hDC);
    if(EGL_NO_DISPLAY==m_eglDisplay)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("CMmpGL_WinCE::Open] FAIL: eglGetDisplay() (error 0x%x)\n\r"), eglGetError() ));
        return MMP_FAILURE;
    }

    if( EGL_FALSE == eglInitialize( m_eglDisplay, &major, &minor ) )
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("CMmpGL_WinCE::Open] FAIL: eglInitialize() (error 0x%x)\n\r"), eglGetError() ));
        return MMP_FAILURE;
    }

    if ( EGL_FALSE == eglGetConfigs(m_eglDisplay, NULL, 0, &max_num_config) )
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("CMmpGL_WinCE::Open] FAIL: eglGetConfigs() (error 0x%x)\n\r"), eglGetError() ));
        return MMP_FAILURE;
    }

    if(max_num_config <= 0)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("CMmpGL_WinCE::Open] FAIL: No EGLConfig found \n\r")));
        return MMP_FAILURE;
    }

    m_eglConfigArr = (EGLConfig *)malloc( sizeof( EGLConfig) * max_num_config );
    if ( NULL == m_eglConfigArr )
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("CMmpGL_WinCE::Open] FAIL: malloc config arr \n\r")));
        return MMP_FAILURE;
    }

    if ( EGL_FALSE == eglChooseConfig( m_eglDisplay, attrib_list, m_eglConfigArr, max_num_config, &num_config ) )
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("CMmpGL_WinCE::Open] FAIL: eglChooseConfig() (error 0x%x)\n\r"), eglGetError() ));
        return MMP_FAILURE;
    }

    if ( 0 == num_config )
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("CMmpGL_WinCE::Open] FAIL: eglChooseConfig() was unable to find a suitable config \n\r")));
        return MMP_FAILURE;
    }

    for ( i = 0; i < num_config; i++ )
    {
        EGLint value;

        /*Use this to explicitly check that the EGL config has the expected color depths */
        eglGetConfigAttrib( m_eglDisplay, m_eglConfigArr[i], EGL_RED_SIZE, &value );
        if ( 5 != value ) continue;

		eglGetConfigAttrib( m_eglDisplay, m_eglConfigArr[i], EGL_GREEN_SIZE, &value );
        if ( 6 != value ) continue;

		eglGetConfigAttrib( m_eglDisplay, m_eglConfigArr[i], EGL_BLUE_SIZE, &value );
        if ( 5 != value ) continue;

		eglGetConfigAttrib( m_eglDisplay, m_eglConfigArr[i], EGL_ALPHA_SIZE, &value );
        if ( 0 != value ) continue;

		eglGetConfigAttrib( m_eglDisplay, m_eglConfigArr[i], EGL_SAMPLES, &value );
        if ( 4 != value ) continue;

        m_eglConfig = m_eglConfigArr[i];
        break;
    }

   	m_eglSurface = eglCreateWindowSurface( m_eglDisplay, m_eglConfig, m_hWnd, NULL );
    if ( EGL_NO_SURFACE == m_eglSurface )
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("CMmpGL_WinCE::Open] FAIL: eglCreateWindowSurface() (error 0x%x)\n\r"), eglGetError() ));
        return MMP_FAILURE;
    }

    m_eglContext = eglCreateContext( m_eglDisplay, m_eglConfig, EGL_NO_CONTEXT, NULL );
    if ( EGL_NO_CONTEXT == m_eglContext )
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("CMmpGL_WinCE::Open] FAIL: eglCreateContext() (error 0x%x)\n\r"), eglGetError() ));
        return MMP_FAILURE;
    }

    if ( EGL_FALSE == eglMakeCurrent( m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext ) )
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("CMmpGL_WinCE::Open] FAIL: eglMakeCurrent() (error 0x%x)\n\r"), eglGetError() ));
        return MMP_FAILURE;
    }

    eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    mmpResult=CMmpGL::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

	return MMP_SUCCESS;
}

MMP_RESULT CMmpGL_WinCE::Close()
{

    if(m_eglDisplay)
    {
        eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(m_eglDisplay, m_eglContext);
        eglDestroySurface(m_eglDisplay, m_eglSurface);
        eglTerminate(m_eglDisplay);
        m_eglDisplay=NULL;
    }

    if(m_eglConfigArr)
    {
        free((void*)m_eglConfigArr);
        m_eglConfigArr=NULL;
    }

	return MMP_SUCCESS;
}

void CMmpGL_WinCE::mglMakeCurrent(BOOL bFlag)
{
    if(bFlag)
    {
        eglMakeCurrent( m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext );
    }
    else
    {
        eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }
}

void CMmpGL_WinCE::mglSwapBuffers()
{
    eglSwapBuffers(m_eglDisplay, m_eglSurface);
}

void CMmpGL_WinCE::Resize(int cx, int cy)
{
    if(m_eglSurface)
    {
        eglDestroySurface(m_eglDisplay, m_eglSurface);

        m_eglSurface = eglCreateWindowSurface( m_eglDisplay, m_eglConfig, m_hWnd, NULL );
        if ( EGL_NO_SURFACE == m_eglSurface )
        {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("CMmpGL_WinCE::Resize] FAIL: eglCreateWindowSurface() (error 0x%x)\n\r"), eglGetError() ));
            return;
        }

    }

    CMmpGL::Resize(cx,cy);
}

#endif//#if (MMP_OS==MMP_OS_WINCE60)