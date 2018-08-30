

#ifndef _HEADER_MmpGL_HPP
#define _HEADER_MmpGL_HPP

#include "MmpDefine.h"
#include "MmpUtil.hpp"
#include "MmpGLIF.hpp"

#if (MMP_OS==MMP_OS_WIN32)
//#include<gl/gl.h>
//#include<gl/glu.h>
#include <GL/glew.h>
#elif (MMP_OS==MMP_OS_WINCE60)
#include <EGL/egl.h>
#include <GLES/gl.h>
#else
#error "ERROR: Select OS in MmpGL.hpp"
#endif

#include "GL3DVector.h"


const enum GLWND_DMENUM { GLWND_GRID, GLWND_SOLID, GLWND_POINT };

//각종 플래그 
const int GLWND_SMOOTH    = 0x00000001;
const int GLWND_CULLING   = 0x00000002;
const int GLWND_DEPTHTEST = 0x00000004;

//const int GLWND_LIGHT0    = 0x00000008;
//const int GLWND_LIGHT1    = 0x00000010;
//const int GLWND_LIGHT2    = 0x00000020;
//const int GLWND_LIGHT3    = 0x00000040;
//const int GLWND_LIGHT4    = 0x00000080;  
const int GLWND_LIGHTING    = 0x00000008;
const int GLWND_FLYTHROUGH  = 0x00000010;
const int GLWND_BLENDING    = 0x00000020; 
const int GLWND_AXIS        = 0x00000040; 

const int GLWND_MAXLIGHT=5;
const int GLWND_MAX_STRING=	1024;

///////////////////////////////////////////
//CGLRGB

class CGLRGB
{
private :
	GLfloat m_V[4]; // r, g, b, alpha

public :
	CGLRGB( GLfloat r=255.0f, GLfloat g=255.0f, GLfloat b=255.0f, GLfloat a=1.0f ) { Put(r,g,b, a); }
	 
	CGLRGB( CGLRGB& glrgb ) { GLfloat a=1.0f; *this=glrgb; }
	CGLRGB( COLORREF col )  { GLfloat a=1.0f; *this=col; }
	CGLRGB& operator=(CGLRGB& glrgb) { Put_R(glrgb.R()), Put_G(glrgb.G()), Put_B(glrgb.B()), Put_A(glrgb.A()); return *this;}
	CGLRGB& operator=(COLORREF col ) { Put_R(col), Put_G(col), Put_B(col); return*this;}

	GLfloat R() { return m_V[0]; }
	GLfloat G() { return m_V[1]; }
	GLfloat B() { return m_V[2]; }
	GLfloat A() { return m_V[3]; }
    GLfloat* RGBA_Array() { return (GLfloat*)m_V; }
    COLORREF RGBA_COLORREF() 
	{ 
	   COLORREF col;
	   col=RGB( (BYTE)(m_V[0]*255.0f), (BYTE)(m_V[1]*255.0f), (BYTE)(m_V[2]*255.0f) );
	   return col;
	}

	void Put( GLfloat r, GLfloat g, GLfloat b , GLfloat a=1.0f) 
	{ Put_R(r), Put_G(g), Put_B(b), Put_A(a);}
	void Put_R( GLfloat r ) { m_V[0]=r; }
	void Put_G( GLfloat g ) { m_V[1]=g; }
	void Put_B( GLfloat b ) { m_V[2]=b; }
    void Put_R( COLORREF col ) {  m_V[0]=(GLfloat)GetRValue(col)/255.0f; }
    void Put_G( COLORREF col ) {  m_V[1]=(GLfloat)GetGValue(col)/255.0f; }
    void Put_B( COLORREF col ) {  m_V[2]=(GLfloat)GetBValue(col)/255.0f; }
	void Put_A( GLfloat a ) { m_V[3]=a; }
	

	void SetColor() { glColor4f( R(), G(), B(), 0.25f ); }

	CGLRGB& operator+=( CGLRGB& glRGB ) { for( int i=0; i<3; i++) m_V[i]+=glRGB.m_V[i]; return *this; }
	CGLRGB& operator-=( CGLRGB& glRGB ) { for( int i=0; i<3; i++) m_V[i]-=glRGB.m_V[i]; return *this; }
    CGLRGB& operator*=( CGLRGB& glRGB ) { for( int i=0; i<3; i++) m_V[i]*=glRGB.m_V[i]; return *this; }
	CGLRGB& operator/=( CGLRGB& glRGB ) { for( int i=0; i<3; i++) if(glRGB.m_V[i]!=0.0f) m_V[i]/=glRGB.m_V[i]; return *this; }
    CGLRGB& operator+=( COLORREF col ) { CGLRGB glRGB(col); return *this+=glRGB; }
	CGLRGB& operator-=( COLORREF col ) { CGLRGB glRGB(col); return *this+=glRGB; }
    CGLRGB& operator*=( GLfloat v )   { for( int i=0; i<3; i++) m_V[i]*=v; return *this; }
    CGLRGB& operator/=( GLfloat v )   { if(v==0.0f) return *this; for( int i=0; i<4; i++) m_V[i]/=v; return *this; }

	friend CGLRGB operator+( CGLRGB glRGB1, CGLRGB glRGB2 ) { CGLRGB glRGB(glRGB1);  glRGB+=glRGB2; return glRGB; }
	friend CGLRGB operator-( CGLRGB glRGB1, CGLRGB glRGB2 ) { CGLRGB glRGB(glRGB1);  glRGB-=glRGB2; return glRGB; }
	friend CGLRGB operator*( CGLRGB glRGB1, CGLRGB glRGB2 ) { CGLRGB glRGB(glRGB1);  glRGB*=glRGB2; return glRGB; }
	friend CGLRGB operator/( CGLRGB glRGB1, CGLRGB glRGB2 ) { CGLRGB glRGB(glRGB1);  glRGB/=glRGB2; return glRGB; }
	friend CGLRGB operator*( CGLRGB glRGB1, GLfloat v ) { CGLRGB glRGB(glRGB1);  glRGB*=v; return glRGB; }
	friend CGLRGB operator/( CGLRGB glRGB1, GLfloat v ) { CGLRGB glRGB(glRGB1);  glRGB/=v; return glRGB; }
	friend CGLRGB operator*( GLfloat v, CGLRGB glRGB1 ) { CGLRGB glRGB(glRGB1);  glRGB*=v; return glRGB; }
	friend CGLRGB operator/( GLfloat v, CGLRGB glRGB1  ) { CGLRGB glRGB(glRGB1);  glRGB/=v; return glRGB; }
	
	
};

////////////////////////////////////////////////////////////
// CGLObject_LIGHT

class CGLObject_LIGHT
{
  private :
	  CGLRGB m_rgbAmbient;
	  CGLRGB m_rgbDiffuse;
	  CGLRGB m_rgbSpecular;
	  CGL3DVector m_vecPos;

	  int m_nNum; //조명 번호
	  BOOL m_bON;

  public :
      CGLObject_LIGHT(int n=0);
	  ~CGLObject_LIGHT();

	  CGLRGB GetAmbient();
	  CGLRGB GetDiffuse();
	  CGLRGB GetSpecular();
	  CGL3DVector GetPos();

	  void SetAmbient( GLfloat r, GLfloat g, GLfloat b );
	  void SetAmbient( CGLRGB ambient );
	  void SetDiffuse( GLfloat r, GLfloat g, GLfloat b );
	  void SetDiffuse( CGLRGB ambient );
	  void SetSpecular( GLfloat r, GLfloat g, GLfloat b );
	  void SetSpecular( CGLRGB ambient );
	  void SetPos( GLfloat r, GLfloat g, GLfloat b );
	  void SetPos( CGL3DVector ambient );

	  void On();
	  void Off();
	  BOOL IsON() { return m_bON; }
};


class CMmpGL : public CMmpGLIF
{
public:
//    static CMmpGL* CreateObject(HWND hWnd, HDC hDC);
 //   static MMP_RESULT DestroyObject(CMmpGL*);

protected:
    HWND m_hWnd;
    HDC m_hDC;
    CGLRGB m_rgbBack; //Background Color
    CMmpSize m_szClient; 

#if (MMP_OS==MMP_OS_WIN32)
    HGLRC m_hRC;
#elif (MMP_OS==MMP_OS_WINCE60)
    EGLDisplay  m_eglDisplay;
    EGLContext  m_eglContext;
    EGLConfig   m_eglConfig;
    EGLConfig  *m_eglConfigArr;
    EGLSurface  m_eglSurface;
#endif
    
protected:  //View Volumn
    BOOL m_VV_bMode; //TRUE->Orthograhic , FALSE->Perspective
    GLfloat m_VV_fPar[6];
	BOOL m_VV_bAuto;  // TRUE->Auto , Default is TRUE
   
    BOOL  VV_IsMode_O() { return m_VV_bMode; }  //Orthogaphic Mode이면 TRUE
    BOOL  VV_IsMode_P() { return m_VV_bMode?FALSE:TRUE; }  //perspective Mode 이면 TRUE
    BOOL  VV_IsAuto() { return m_VV_bAuto; }
    void VV_SetAuto(BOOL flag) { m_VV_bAuto=flag;}
	GLfloat VV_GetPar( int i ) { if( i>=0 && i <6 ) return m_VV_fPar[i]; else return 0.0f; }
    void  VV_Resize( BOOL bRedraw=FALSE );
    void  VV_SetMode_O(GLfloat par1,GLfloat par2,GLfloat par3,GLfloat par4,GLfloat par5,GLfloat par6 , BOOL bRedraw=FALSE);
    void  VV_SetMode_P(GLfloat par1,GLfloat par2,GLfloat par3,GLfloat par4, BOOL bRedraw=FALSE); 
	
//////////////////////////
 // Translate
 private :
   GLfloat m_TR_fPar[4];   // x, y, z,  증가 및 감소 값

 public :

    GLfloat TR_GetX() { return m_TR_fPar[0]; }
    GLfloat TR_GetY() { return m_TR_fPar[1]; }
    GLfloat TR_GetZ() { return m_TR_fPar[2]; }
    GLfloat TR_GetDiff() { return m_TR_fPar[3]; }
    void    TR_SetX(GLfloat v)   { m_TR_fPar[0]=v; }
    void    TR_SetY(GLfloat v)   { m_TR_fPar[1]=v; }
    void    TR_SetZ(GLfloat v)   { m_TR_fPar[2]=v; }
    void    TR_SetDiff( GLfloat v){ m_TR_fPar[3]=v; }
    void    TR_Set( GLfloat x, GLfloat y, GLfloat z) { m_TR_fPar[0]=x, m_TR_fPar[1]=y,m_TR_fPar[2]=z; }
    void    TR_Set (GLfloat x, GLfloat y, GLfloat z, GLfloat diff)  { m_TR_fPar[0]=x, m_TR_fPar[1]=y,m_TR_fPar[2]=z, m_TR_fPar[3]=diff; }
    void    TR_IncX() { m_TR_fPar[0]+=m_TR_fPar[3]; }
    void    TR_IncY() { m_TR_fPar[1]+=m_TR_fPar[3]; }
    void    TR_IncZ() { m_TR_fPar[2]+=m_TR_fPar[3]; }
    void    TR_DecX() { m_TR_fPar[0]-=m_TR_fPar[3]; }
    void    TR_DecY() { m_TR_fPar[1]-=m_TR_fPar[3]; }
    void    TR_DecZ() { m_TR_fPar[2]-=m_TR_fPar[3]; }
    void    TR_IncX(GLfloat fg) { m_TR_fPar[0]+=fg; }
    void    TR_IncY(GLfloat fg) { m_TR_fPar[1]+=fg; }
    void    TR_IncZ(GLfloat fg) { m_TR_fPar[2]+=fg; }
    void    TR_DecX(GLfloat fg) { m_TR_fPar[0]-=fg; }
    void    TR_DecY(GLfloat fg) { m_TR_fPar[1]-=fg; }
    void    TR_DecZ(GLfloat fg) { m_TR_fPar[2]-=fg; }
    void    TR_Do() { glTranslatef( m_TR_fPar[0], m_TR_fPar[1], m_TR_fPar[2] ); } 

///////////////////////////////
// Rotate : 절대축을 기준으로 회전할 것이다. 
protected : 
    CGL3DVector m_VecX;  //상대축 x의 절대계 좌표 
    CGL3DVector m_VecZ;  //상대축 z의 절대계 좌표
    CMmpPoint m_posLBClick; //왼쪽 버튼이 눌러진 위치 , 마우스 이동시 사용
    CGL3DVector m_VecX1, m_VecZ1;  //마우스 이동시 사용할 벡업본 

    void Rotate_Do();

/////////////////////////////////////
//Light 조명
protected :
     CGLObject_LIGHT m_Light_Array[GLWND_MAXLIGHT];

	 int Light_GetNum(); //조명개수 리턴
     void Light_On( int n ); //조명을 켠다.
	 void Light_Off( int n); //조명을 끈다. 
	 BOOL Light_IsON( int n ); //{ return m_Light_Array[n].IsON(); }
	 
	 void Light_SetAmbient( int n, GLfloat r, GLfloat g, GLfloat b );
	 void Light_SetDiffuse( int n, GLfloat r, GLfloat g, GLfloat b );
	 void Light_SetSpecular( int n, GLfloat r, GLfloat g, GLfloat b );
     void Light_SetPos( int n, GLfloat x, GLfloat y, GLfloat z );

	 CGLRGB Light_GetAmbient( int n );
	 CGLRGB Light_GetDiffuse( int n );
	 CGLRGB Light_GetSpecular( int n );
	 CGL3DVector Light_GetPos( int n );

////////////////////////////////////
//Texture Mapping
protected:
    int m_GLLIST_List[25]; //25개의 이미지 리스트를 사용할 것이다 
                           //0부터 9까지는 텍스쳐 리스트이다.(주의!!!)
 
    int m_TEX_ImageWidth[10];
    int m_TEX_ImageHeight[10];

    int m_imgWidth, m_imgHeight;
    unsigned char* m_imgData;

public:
    int GLLIST_Begin(int n);   //이미지 리스트 시작 , 리스트 번호 리턴 
    void GLLIST_End();    //이미지 리스트 끝  
    void GLLIST_Delete( int n );
    void GLLIST_Call( int n );

    int TEX_LoadRGB32(int imageWidth, int imageHeight, unsigned char* imageData, int n );
    int TEX_UpdateRGB32(int imageWidth, int imageHeight, unsigned char* imageData, int n );
    void TEX_CallList( int n );                 //리스트 호출 
    void TEX_Mapping( GLfloat x, GLfloat y );
    void TEX_DeleteAllList();
	void TEX_DeleteList(int n);
    

////////////////////////////////////
//class Interface
friend class CMmpGLIF;
protected:
	CMmpGL(HWND hWnd, HDC hDC);
	virtual ~CMmpGL(void);


	virtual MMP_RESULT Open();
	virtual MMP_RESULT Close();

    //virtual void mglMakeCurrent(BOOL bFlag)=0;
    //virtual void mglSwapBuffers()=0;

protected:
    virtual void Draw_Begin();
    virtual void Draw_End();
    virtual void Draw_Objects()=0;

    void Draw_Axis();

public:
    virtual void Draw();

 public :
	 COLORREF GetBackColor();
	 void SetBackColor(COLORREF col);
	 void SetBackColor(BYTE r, BYTE g, BYTE b);

     virtual void Resize(int cx, int cy);
     virtual void Key_Proc(UINT nChar, UINT nRepCnt, UINT nFlags);
    
};

#endif