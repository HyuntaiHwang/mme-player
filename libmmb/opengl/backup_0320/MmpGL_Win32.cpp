/*
 *******************************************************************************
 *  (C) Copyright MtekVision Ltd. 2007 All right reserved.
 *    Confidential Information
 *  
 *  All parts of the MtekVision Program Source are protected by copyright law 
 *  and all rights are reserved. 
 *  This documentation may not, in whole or in part, be copied, photocopied, 
 *  reproduced, translated, or reduced to any electronic medium or machine 
 *  readable form without prior consent, in writing, from MtekVision. 
 *******************************************************************************
 *  Project     : OpenOS1
 *  Description :
 *      
 *******************************************************************************
 *  Version control information
 *  $Id$
 *
 *******************************************************************************
 *  Modification History
 *  $Log$
 *
 *******************************************************************************
 */

#include "../MmpGlobal/MmpDefine.h"
#include "MmpGL_Win32.hpp"

#if (MMP_OS==MMP_OS_WIN32)

CMmpGL_Win32::CMmpGL_Win32(HWND hWnd, HDC hDC) : CMmpGL(hWnd, hDC)
,m_hRC(NULL)
{
}

CMmpGL_Win32::~CMmpGL_Win32(void)
{
	
}

MMP_RESULT CMmpGL_Win32::Open()
{
    MMP_RESULT mmpResult;
    int nPixelFormat;					// Pixel format index

   	static PIXELFORMATDESCRIPTOR pfd = 
    {
		sizeof(PIXELFORMATDESCRIPTOR),	// Size of this structure
		1,								// Version of this structure	
		PFD_DRAW_TO_WINDOW |			// Draw to Window (not to bitmap)
		PFD_SUPPORT_OPENGL |			// Support OpenGL calls in window
		PFD_DOUBLEBUFFER,				// Double buffered mode
		PFD_TYPE_RGBA,					// RGBA Color mode
		24,								// Want 24bit color 
		0,0,0,0,0,0,					// Not used to select mode
		0,0,							// Not used to select mode
		0,0,0,0,0,						// Not used to select mode
		32,								// Size of depth buffer
		0,								// Not used to select mode
		0,								// Not used to select mode
		PFD_MAIN_PLANE,					// Draw in main plane
		0,								// Not used to select mode
		0,0,0 };						// Not used to select mode


    if( (nPixelFormat = ChoosePixelFormat(m_hDC, &pfd) ) == 0 )
    {
        return MMP_FAILURE;
    } 

    // Set the pixel format for the device context
    if( SetPixelFormat(m_hDC, nPixelFormat, &pfd) == FALSE )
	{
	    nPixelFormat=1;
	    if( DescribePixelFormat(m_hDC, nPixelFormat, sizeof(pfd), &pfd ) == 0 )
        {
	        return FALSE;
	    }
	}

    int n;
	PIXELFORMATDESCRIPTOR pfd1;
	n=GetPixelFormat(m_hDC);
	DescribePixelFormat(m_hDC, n, sizeof(pfd1), &pfd1 );

    // Create the rendering context
	m_hRC = wglCreateContext(m_hDC);

	// Make the rendering context current, perform initialization, then
	// deselect it
	wglMakeCurrent(m_hDC,m_hRC);

    wglMakeCurrent(NULL,NULL);

    mmpResult=CMmpGL::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

	return MMP_SUCCESS;
}

MMP_RESULT CMmpGL_Win32::Close()
{
    if(m_hRC)
    {
        wglDeleteContext(m_hRC);
        m_hRC;
    }

	return MMP_SUCCESS;
}

void CMmpGL_Win32::mglMakeCurrent(BOOL bFlag)
{
    if(bFlag)
    {
        wglMakeCurrent(m_hDC,m_hRC);                       //DC, RC ÁöÁ¤   
    }
    else
    {
        wglMakeCurrent(NULL, NULL); 
    }
}

void CMmpGL_Win32::mglSwapBuffers()
{
    SwapBuffers(m_hDC);
}

#endif