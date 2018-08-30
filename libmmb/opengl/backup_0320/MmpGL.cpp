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

/*******************************************************************************
 * INCLUDE FILES                                                           
 ******************************************************************************/
#include "../MmpGlobal/MmpDefine.h"
#include "MmpGL.hpp"
#include "MmpGL_Grid.hpp"
#include "MmpGL_Movie.hpp"

//#define DIRECT_RENDER_ENABLE

#if (MMP_OS==MMP_OS_WIN32)
#define mglOrtho glOrtho
#define mgluPerspective gluPerspective

#define mglMakeCurrent(flag) flag?wglMakeCurrent(m_hDC,m_hRC):wglMakeCurrent(NULL, NULL) 
#define mglSwapBuffers()  SwapBuffers(m_hDC)

#elif (MMP_OS==MMP_OS_WINCE60)
#define mglOrtho glOrthof
#define mgluPerspective gluPerspective

#define mglMakeCurrent(flag) flag?eglMakeCurrent( m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext ):eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)
#define mglSwapBuffers()  eglSwapBuffers(m_eglDisplay, m_eglSurface)

void gluPerspective(double fovy, double aspect,  double zNear, double zFar) {}
#endif




////////////////////////////////////////////////////////////////////////////
//CGLObject_LIGHT
CGLObject_LIGHT::CGLObject_LIGHT(int n)
{
  m_nNum=GL_LIGHT0+n;
  m_rgbAmbient=CGLRGB(0.3f, 0.3f, 0.3f);
  m_rgbDiffuse=CGLRGB(0.7f, 0.7f, 0.7f);
  m_rgbSpecular=CGLRGB(1.0f, 1.0f, 1.0f );  
  m_vecPos=CGL3DVector( 100.0f, 0.0f, 0.0f);
  m_bON=FALSE;
}

CGLObject_LIGHT::~CGLObject_LIGHT()
{
  this->Off();
}

CGLRGB CGLObject_LIGHT::GetAmbient() { return m_rgbAmbient;}
CGLRGB CGLObject_LIGHT::GetDiffuse(){return m_rgbDiffuse;}
CGLRGB CGLObject_LIGHT::GetSpecular(){return m_rgbSpecular;}
CGL3DVector CGLObject_LIGHT::GetPos(){return m_vecPos;}

void CGLObject_LIGHT::SetAmbient( GLfloat r, GLfloat g, GLfloat b ) { m_rgbAmbient=CGLRGB(r,g,b); }
void CGLObject_LIGHT::SetAmbient( CGLRGB ambient ){m_rgbAmbient=ambient; }
void CGLObject_LIGHT::SetDiffuse( GLfloat r, GLfloat g, GLfloat b ){m_rgbDiffuse=CGLRGB(r,g,b);}
void CGLObject_LIGHT::SetDiffuse( CGLRGB diffuse ){m_rgbDiffuse=diffuse;}
void CGLObject_LIGHT::SetSpecular( GLfloat r, GLfloat g, GLfloat b ){m_rgbSpecular=CGLRGB(r,g,b);}
void CGLObject_LIGHT::SetSpecular( CGLRGB specular ){m_rgbSpecular=specular;}
void CGLObject_LIGHT::SetPos( GLfloat x, GLfloat y, GLfloat z ){m_vecPos=CGL3DVector(x,y,z);}
void CGLObject_LIGHT::SetPos( CGL3DVector vecPos ){ m_vecPos=vecPos;}

void CGLObject_LIGHT::On()
{
	GLfloat lightPos[]={ 0.0f, 150.0f, 450.0f, 1.0f };
	
    //조명 모델 설정 
	//glEnable(GL_LIGHTING);
	glLightfv(m_nNum, GL_AMBIENT,  m_rgbAmbient.RGBA_Array());
	glLightfv(m_nNum, GL_DIFFUSE,  m_rgbDiffuse.RGBA_Array());
	glLightfv(m_nNum, GL_SPECULAR,  m_rgbSpecular.RGBA_Array());
	lightPos[0]=(float)m_vecPos.X();
    lightPos[1]=(float)m_vecPos.Y();
    lightPos[2]=(float)m_vecPos.Z();
    glLightfv(m_nNum, GL_POSITION, lightPos);
	glEnable(m_nNum);

	m_bON=TRUE;
	//glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	//glMaterialfv(GL_FRONT, GL_SPECULAR,m_LT_rgbSpecular.RGBA_Array());
	//glMateriali(GL_FRONT,GL_SHININESS,128);

	//칼라 재질 설정 
	//glEnable( GL_COLOR_MATERIAL );
	//glColorMaterial( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );
	//glColorMaterial( GL_FRONT, GL_SPECULAR );
}

void CGLObject_LIGHT::Off()
{
    glDisable(m_nNum);
	m_bON=FALSE;
}

///////////////////////////////////////////
//CMmpGL Create/Destroy Object

CMmpGLIF* CMmpGLIF::CreateObject(HWND hWnd, HDC hDC)
{
    CMmpGLIF* pObj;
    
    //pObj=new CMmpGL(hWnd, hDC);
    pObj=new CMmpGL_Grid(hWnd, hDC);
   // pObj=new CMmpGL_Movie(hWnd, hDC);
    if(pObj==NULL)
    {
        return pObj;
    }

    if(pObj->Open()!=MMP_SUCCESS)
    {
        pObj->Close();
        delete pObj;
        pObj=NULL;
        return pObj;
    }
   
    return pObj;
}

MMP_RESULT CMmpGLIF::DestroyObject(CMmpGLIF* pObj)
{
    if(pObj==NULL)
    {
        return MMP_FAILURE;
    }
    
    pObj->Close();
    delete pObj;
    pObj=NULL;

    return MMP_SUCCESS;
}


/////////////////////////////////////////////////////////////////
//class CMmpGL

CMmpGL::CMmpGL(HWND hWnd, HDC hDC) :
m_hWnd(hWnd)
,m_hDC(hDC)
,m_rgbBack(1.0f,1.0f,1.0f)
#if (MMP_OS==MMP_OS_WIN32)
,m_hRC(NULL)
#elif (MMP_OS==MMP_OS_WINCE60)
,m_eglDisplay(NULL)
,m_eglConfig(NULL)
,m_eglContext(NULL)
,m_eglConfigArr(NULL)
,m_eglSurface(NULL)

#endif
{
    int i;

    m_szClient=CMmpSize(0,0);

    //View Volume
    for(i=0; i<6; i++) m_VV_fPar[i]=0.0f;
    VV_SetAuto(TRUE);	
    //VV_SetMode_P( 20.0f, 0.0f, 300.0f, 700.0f );
    //VV_SetMode_P( 35.0f, 0.0f, 1.0f, 2000.0f );
    VV_SetMode_O( -100.0f, 100.0f, -100.0f, 100.0f, -100.0f,100.0f );
	//VV_SetMode_O( -150.0f, 150.0f, -150.0f, 150.0f, -100.0f,100.0f );	
	
    //Rotate
	m_VecX=CGL3DVector( 1, 0, 0 );
	m_VecZ=CGL3DVector( 0, 0, 1 );

}

CMmpGL::~CMmpGL(void)
{
	
}

#if (MMP_OS==MMP_OS_WIN32)

MMP_RESULT CMmpGL::Open()
{
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

    return MMP_SUCCESS;
}

MMP_RESULT CMmpGL::Close()
{
    if(m_hRC)
    {
        wglDeleteContext(m_hRC);
        m_hRC;
    }

	return MMP_SUCCESS;
}

#elif (MMP_OS==MMP_OS_WINCE60)

MMP_RESULT CMmpGL::Open()
{
    int i;
    HDC hDC=NULL;
    HWND hWnd=NULL;

    EGLint      major, minor, num_config, max_num_config;

    EGLint      attrib_list[] = {
                            EGL_RED_SIZE, 5,
                            EGL_GREEN_SIZE, 6,
                            EGL_BLUE_SIZE, 5, 
                            EGL_ALPHA_SIZE, 0,
                            EGL_RENDERABLE_TYPE,  EGL_OPENGL_ES_BIT, 
                            EGL_SURFACE_TYPE,  EGL_WINDOW_BIT,  EGL_NONE };

#ifndef DIRECT_RENDER_ENABLE
    hDC=m_hDC;
    hWnd=m_hWnd;
#endif
    
    if(hDC==NULL)
        m_eglDisplay = eglGetDisplay(GetDC(hWnd));
    else
        m_eglDisplay = eglGetDisplay(hDC);
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

   	//m_eglSurface = eglCreatePbufferSurface( m_eglDisplay, m_eglConfig, NULL );
    m_eglSurface = eglCreateWindowSurface( m_eglDisplay, m_eglConfig, hWnd, NULL );
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

    return MMP_SUCCESS;
}

MMP_RESULT CMmpGL::Close()
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

#endif

//////////////////////////////////////////////////
// View Volumn
void  CMmpGL::VV_SetMode_O(GLfloat par1,GLfloat par2,GLfloat par3,GLfloat par4,GLfloat par5,GLfloat par6 , BOOL bRedraw)
{
  m_VV_bMode=TRUE;
  m_VV_fPar[0]=par1;
  m_VV_fPar[1]=par2;
  m_VV_fPar[2]=par3;
  m_VV_fPar[3]=par4;
  m_VV_fPar[4]=par5;
  m_VV_fPar[5]=par6;
  if(bRedraw) VV_Resize(bRedraw);

  TR_Set( 0.0f, 0.0f, 0.0f );
}

void  CMmpGL::VV_SetMode_P(GLfloat par1,GLfloat par2,GLfloat par3,GLfloat par4, BOOL bRedraw) 
{
  m_VV_bMode=FALSE;

  m_VV_fPar[0]=par1;
  m_VV_fPar[1]=par2;
  m_VV_fPar[2]=par3;
  m_VV_fPar[3]=par4;
  
  if(bRedraw) VV_Resize(bRedraw);

  TR_Set( 0.0f, 0.0f, -( par3+par4 )/2.0f );
}

void  CMmpGL::VV_Resize( BOOL bRedraw)
{
    //if(m_szClient.cy!=240)
    //    return;

     mglMakeCurrent(TRUE);
	
     //m_szClient.cx=800;
     //m_szClient.cy=480;
     glViewport(0, 0, m_szClient.m_cx, m_szClient.m_cy);

     glMatrixMode(GL_PROJECTION);
     glLoadIdentity();

     if( VV_IsAuto() )
     {
         GLfloat w, h;
	     w=(GLfloat)m_szClient.m_cx; h=(GLfloat)m_szClient.m_cy;

         if( VV_IsMode_O() ) 
	     {
	        if( w<=h)
		    {
              if( w==0.0f ) w=1.0f;
		      mglOrtho( m_VV_fPar[0], m_VV_fPar[1], m_VV_fPar[2]*h/w, m_VV_fPar[3]*h/w, m_VV_fPar[4], m_VV_fPar[5] );
		    }
		    else
		    {
              if( h==0.0f ) h=1.0f;
		      mglOrtho( m_VV_fPar[0]*w/h, m_VV_fPar[1]*w/h, m_VV_fPar[2], m_VV_fPar[3], m_VV_fPar[4], m_VV_fPar[5] );
		    }
		    
	     }
	     else if( VV_IsMode_P() )
	     {
	       if( h==0.0f ) h=1.0f;
	       gluPerspective( m_VV_fPar[0], w/h, m_VV_fPar[2], m_VV_fPar[3] ); 
	     }
     }
     else 
     {
        if( VV_IsMode_O() ) 
	        mglOrtho( m_VV_fPar[0], m_VV_fPar[1], m_VV_fPar[2], m_VV_fPar[3], m_VV_fPar[4], m_VV_fPar[5] ); 
        else if( VV_IsMode_P() )
	        gluPerspective( m_VV_fPar[0], m_VV_fPar[1], m_VV_fPar[2], m_VV_fPar[3] ); 
    }
   
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    

    mglMakeCurrent(FALSE);

   //if(bRedraw) Invalidate();
}

////////////////////////////////////////////////////////////////////////
//Rotate

void CMmpGL::Rotate_Do()
{
    float yr, xr, angle;
    float i,j;

    CGL3DVector VecX(1, 0, 0 );
    CGL3DVector VecZ;
    VecX.RotateY( m_VecZ.V() );  //OpenGL의 현재 x축 위치를 구한다.
                                //실제로 변환된 x축 위치와 차이가 있다. 
         
    //실제 x축과 현재 opengl상의 x축 간의 사이각 
    angle=CGL3DVector::GetAngle( m_VecX, VecX); 
   
    //외적을 이용하여 사이각의 방향성을 결정한다. 
    VecZ=m_VecX&VecX;
    i=VecZ.U();
    j=m_VecZ.U();
    if( i==0.0 || j==0.0 ) angle*=1;
    else if(  fabs( i/fabs(i) - j/fabs(j) ) > PI/180.0 ) angle*=-1;

    yr=m_VecZ.V()*180/PI;
    xr=m_VecZ.U()*180/PI;

    //z축 회전 
    glRotatef( yr, 0.0f, 1.0f, 0.0f );
    glRotatef( xr, 1.0f, 0.0f, 0.0f );
   
    //x축 보정 
    glRotatef( (float)(angle*180/PI), 0.0f, 0.0f, 1.0f );
}

///////////////////////////////////////////////////////////////////////
//Light

/////////////////////////////////////////////////////
//Light

//조명개수 리턴
int CMmpGL::Light_GetNum() {  return GLWND_MAXLIGHT; }

void CMmpGL::Light_On( int n ) //조명을 켠다.
{
   CGLObject_LIGHT* pLTObj;
   if(n<0 || n>=Light_GetNum()) return;
   pLTObj=&m_Light_Array[n];

   if(m_hDC)
   {
      mglMakeCurrent(TRUE);
      pLTObj->On();
      mglMakeCurrent(FALSE);
   }
}

void CMmpGL::Light_Off( int n) //조명을 끈다. 
{
   CGLObject_LIGHT* pLTObj;
   if(n<0 || n>=Light_GetNum()) return;
   pLTObj=&m_Light_Array[n];

   if(m_hDC)
   {
     mglMakeCurrent(TRUE);
     pLTObj->Off();
     mglMakeCurrent(FALSE);
   }
}
	 
BOOL CMmpGL::Light_IsON( int n ) //조명
{
   CGLObject_LIGHT* pLTObj;
   if(n<0 || n>=Light_GetNum()) return FALSE;
   pLTObj=&m_Light_Array[n];
   return pLTObj->IsON();
}


void CMmpGL::Light_SetAmbient( int n, GLfloat r, GLfloat g, GLfloat b )
{
   CGLObject_LIGHT* pLTObj;
   if(n<0 || n>=Light_GetNum()) return;
   pLTObj=&m_Light_Array[n];
   pLTObj->SetAmbient(r,g,b);
}

void CMmpGL::Light_SetDiffuse( int n, GLfloat r, GLfloat g, GLfloat b )
{
   CGLObject_LIGHT* pLTObj;
   if(n<0 || n>=Light_GetNum()) return;
   pLTObj=&m_Light_Array[n];
   pLTObj->SetDiffuse(r,g,b);
}

void CMmpGL::Light_SetSpecular( int n, GLfloat r, GLfloat g, GLfloat b )
{
   CGLObject_LIGHT* pLTObj;
   if(n<0 || n>=Light_GetNum()) return;
   pLTObj=&m_Light_Array[n];
   pLTObj->SetSpecular(r,g,b);
}

void CMmpGL::Light_SetPos( int n, GLfloat x, GLfloat y, GLfloat z )
{
   CGLObject_LIGHT* pLTObj;
   if(n<0 || n>=Light_GetNum()) return;
   pLTObj=&m_Light_Array[n];
   pLTObj->SetPos(x,y,z);
}


CGLRGB CMmpGL::Light_GetAmbient( int n )
{
   CGLObject_LIGHT* pLTObj;
   CGLRGB rgb;
   if(n<0 || n>=Light_GetNum()) return rgb;
   pLTObj=&m_Light_Array[n];
   return pLTObj->GetAmbient();
}

CGLRGB CMmpGL::Light_GetDiffuse( int n )
{
   CGLObject_LIGHT* pLTObj;
   CGLRGB rgb;
   if(n<0 || n>=Light_GetNum()) return rgb;
   pLTObj=&m_Light_Array[n];
   return pLTObj->GetDiffuse();
}

CGLRGB CMmpGL::Light_GetSpecular( int n )
{
   CGLObject_LIGHT* pLTObj;
   CGLRGB rgb;
   if(n<0 || n>=Light_GetNum()) return rgb;
   pLTObj=&m_Light_Array[n];
   return pLTObj->GetSpecular();
}

CGL3DVector CMmpGL::Light_GetPos( int n )
{
   CGLObject_LIGHT* pLTObj;
   CGL3DVector vec;
   if(n<0 || n>=Light_GetNum()) return vec;
   pLTObj=&m_Light_Array[n];
   return pLTObj->GetPos();
}



////////////////////////////////////////////////////////////////////////
// Back Color

COLORREF CMmpGL::GetBackColor() { return m_rgbBack.RGBA_COLORREF(); }

void CMmpGL::SetBackColor(COLORREF col) { m_rgbBack=col; }

void CMmpGL::SetBackColor(BYTE r, BYTE g, BYTE b) { m_rgbBack=RGB(r,g,b); }

////////////////////////////////////////////////////////////////////////
// Drawing

void CMmpGL::Draw()
{
    this->Draw_Begin();
    this->Draw_Objects();
    this->Draw_Axis();
    this->Draw_End();
}

void CMmpGL::Draw_Begin()
{
    mglMakeCurrent(TRUE);

    glClearColor( m_rgbBack.R(), m_rgbBack.G(), m_rgbBack.B(), 1.0f);            //배경색 지정
	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT); //지우기 
	
    glPushMatrix(); 

    Rotate_Do();
}

void CMmpGL::Draw_End()
{
    glPopMatrix(); 

    //SwapBuffers(m_hDC);
    mglSwapBuffers();
    mglMakeCurrent(FALSE);
}

#if 0
void CMmpGL::Draw_Objects()
{
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    //glLoadIdentity();
    //glTranslatef(0.0, 0.0, -6.0f);
    //glRotatef(55, 0.3f, 1.f, 1.3f);
    //glColor3f( 1.0f, 0.0f, 0.0f );
	
  const GLushort fanIx [] = {
                0, 1, 2, 3, 6, 5, 4, 1,   /* One tri.fan  */
                7, 6, 3, 2, 1, 4, 5, 6};  /*  and another */

    const GLfloat coord [] = {
                 50.f,  50.f,  50.f,      /* 0 */
                -50.f,  50.f,  50.f,      /* 1 */
                -50.f, -50.f,  50.f,      /* 2 */
                 50.f, -50.f,  50.f,      /* 3 */
                -50.f,  50.f, -50.f,      /* 4 */
                 50.f,  50.f, -50.f,      /* 5 */
                 50.f, -50.f, -50.f,      /* 6 */
                -50.f, -50.f, -50.f };    /* 7 */

  const GLubyte color [] = {
                255, 255, 255, 255, 
                255,   0, 255, 255, 
                0,     0, 255, 255, 
                0,   255, 255, 255,
                255,   0,   0, 255,
                255, 255,   0, 255,
                0,   255,   0, 255,
                0,     0,   0, 255};

    glVertexPointer(3, GL_FLOAT, 0, coord);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, color);

    glDrawElements(GL_TRIANGLE_FAN, 8, GL_UNSIGNED_SHORT, fanIx);
    glDrawElements(GL_TRIANGLE_FAN, 8, GL_UNSIGNED_SHORT, &(fanIx[8]));
}
#endif

void CMmpGL::Draw_Axis()
{
#if 0//(MMP_OS==MMP_OS_WIN32)
    glBegin( GL_LINES );

     //x축 그리기 
     glColor3f( 1.0f, 0.0f, 0.0f );
	 glVertex3f( 0.0f,0.0f, 0.0f );
	 glVertex3f( 100.0f, 0.0f, 0.0f );
     
	//y축 그리기 
     glColor3f( 0.0f, 1.0f, 0.0f );
	 glVertex3f( 0.0f,0.0f, 0.0f );
	 glVertex3f( 0.0f, 100.0f, 0.0f );
     
     //z축 그리기 
     glColor3f( 0.0f, 0.0f, 1.0f );
	 glVertex3f( 0.0f,0.0f, 0.0f );
	 glVertex3f( 0.0f, 0.0f, 100.0f );
    
   glEnd();
#else

    const GLfloat vertex[] = {
                 0.f,  0.f,  0.f,      /* 0 */
                100.f,  0.f,  0.f,      /* 1 */
                0.f,  100.f,  0.f,      /* 2 */
                0.f,  0.f,  100.f,      /* 3 */
                 };  

   const GLubyte color [] = {
                0, 0, 0, 255, 
                255, 0, 0, 255, 
                0, 255, 0, 255, 
                0, 0, 255, 255, 
                };

    const GLushort Line0[] = {0,1};
    const GLushort Line1[] = {0,2};
    const GLushort Line2[] = {0,3};

    glVertexPointer(3, GL_FLOAT, 0, vertex);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, color);

    glDrawElements(GL_LINES, 2, GL_UNSIGNED_SHORT, Line0);
    glDrawElements(GL_LINES, 2, GL_UNSIGNED_SHORT, Line1);
    glDrawElements(GL_LINES, 2, GL_UNSIGNED_SHORT, Line2);

#endif
}

void CMmpGL::Resize(int cx, int cy)
{
#if (MMP_OS==MMP_OS_WINCE60)

#ifndef DIRECT_RENDER_ENABLE
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
#endif
#endif

   m_szClient=CMmpSize(cx,cy);
   this->VV_Resize();

   Light_SetPos(0, 100.0f, 100.0f, 100.0f);
   Light_On(0);
}

/////////////////////////////////////////////////////////////////////////////
// Texture Mapping

int CMmpGL::GLLIST_Begin(int n)   //이미지 리스트 시작 , 리스트 번호 리턴 
{
    if( n<10 || n>=25 ) return -1;

    wglMakeCurrent(m_hDC, m_hRC);
    glNewList( m_GLLIST_List[n], GL_COMPILE );

    return n;
}

void CMmpGL::GLLIST_End()    //이미지 리스트 끝  
{
    glEndList();
    wglMakeCurrent(NULL, NULL);
}

void CMmpGL::GLLIST_Delete( int n )
{
    if( n<10 || n>=25 ) 
        return;
  
    wglMakeCurrent(m_hDC, m_hRC);
    glDeleteLists(m_GLLIST_List[n],1);
    wglMakeCurrent(NULL, NULL);
}

void CMmpGL::GLLIST_Call( int n )
{
    if( n<10 || n>=25 ) return;
    
    if( glIsList( m_GLLIST_List[n] )==GL_TRUE )  
        glCallList(m_GLLIST_List[n]);
}


void CMmpGL::TEX_CallList( int n )
{
    if( n<0 || n>=10 ) 
        return;
  
    if( glIsList(m_GLLIST_List[n])==GL_TRUE )
        glCallList(m_GLLIST_List[n]);

}

void CMmpGL::TEX_DeleteAllList()
{
    int i;
  
    for(i=0; i<10; i++ )
    {
	    if( glIsList( m_GLLIST_List[i] )==GL_TRUE )
        {
	        glDeleteLists(m_GLLIST_List[i], 1);
        }
    }
}

void CMmpGL::TEX_DeleteList(int n)
{
    if( n<0 || n>9 ) 
        return;

    wglMakeCurrent(m_hDC, m_hRC);
    if( glIsList( m_GLLIST_List[n] )==GL_TRUE )  
    {
        glDeleteLists(m_GLLIST_List[n], 1);
    }
    wglMakeCurrent(NULL, NULL);
}

void CMmpGL::TEX_Mapping( GLfloat x, GLfloat y )
{
   glTexCoord2f( x, y);
}

int CMmpGL::TEX_LoadRGB32(int imageWidth, int imageHeight, unsigned char* imageData, int n )
{
	if( n<0 || n>=10 ) 
        return -1;

	//기존 리스트를 지운다 
	TEX_DeleteList(n);

	wglMakeCurrent(m_hDC, m_hRC);
    glNewList( m_GLLIST_List[n], GL_COMPILE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

#if 0
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);	// Force 4-byte alignment 
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
#endif

	glTexImage2D(GL_TEXTURE_2D, 0, 3, imageWidth, imageHeight, 0,
				 GL_RGB, GL_UNSIGNED_BYTE, imageData );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glEndList();
	wglMakeCurrent(NULL, NULL);

    m_TEX_ImageWidth[n]=imageWidth;
    m_TEX_ImageHeight[n]=imageHeight;

	return n; 

}

//////////////////////////////////////////////////////////////////////////////
// Key Proc

void CMmpGL::Key_Proc(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    float fInc=PI/18.0f;

    switch(nChar)
    {
        case VK_UP:
            m_VecX.RotateX(fInc*(float)nRepCnt); 
            m_VecZ.RotateX(fInc*(float)nRepCnt);
            //::InvalidateRect(m_hWnd, NULL, FALSE);
            this->Draw();
            break;

        case VK_DOWN:
            m_VecX.RotateX(-fInc*(float)nRepCnt);  
            m_VecZ.RotateX(-fInc*(float)nRepCnt);
            //::InvalidateRect(m_hWnd, NULL, FALSE);
            this->Draw();
            break;

        case VK_LEFT:
            m_VecX.RotateZ(-fInc*(float)nRepCnt); 
            m_VecZ.RotateZ(-fInc*(float)nRepCnt);
            //::InvalidateRect(m_hWnd, NULL, FALSE);
            this->Draw();
            break;

        case VK_RIGHT:
            m_VecX.RotateZ(fInc*(float)nRepCnt); 
            m_VecZ.RotateZ(fInc*(float)nRepCnt);
            //::InvalidateRect(m_hWnd, NULL, FALSE);
            this->Draw();
            break;
    }
}
