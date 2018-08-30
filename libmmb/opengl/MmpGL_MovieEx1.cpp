
#include "MmpGL_MovieEx1.hpp"

CMmpGL_MovieEx1::CMmpGL_MovieEx1(HWND hWnd, HDC hDC, int picWidth, int picHeight) : CMmpGL(hWnd, hDC)

,m_iViewWidth(picWidth)
,m_iViewHeight(picHeight)

,m_iPicWidth(picWidth)
,m_iPicHeight(picHeight)
,m_pImageBuffer(NULL)
{
    VV_SetAuto(FALSE);
    VV_SetMode_O(0.0f, (GLfloat)m_iViewWidth, 0.0f, (GLfloat)m_iViewHeight, -100.0f,100.0f );
}

CMmpGL_MovieEx1::~CMmpGL_MovieEx1(void)
{
	
}

MMP_RESULT CMmpGL_MovieEx1::Open()
{   
    int buffer_width = MMP_BYTE_ALIGN(m_iPicWidth, 16);
    int buffer_height = MMP_BYTE_ALIGN(m_iPicHeight, 16);

    m_pImageBuffer=new unsigned char[buffer_width*buffer_height*3];
    if(!m_pImageBuffer)
    {
        return MMP_FAILURE;
    }

    if( CMmpGL::Open()!=MMP_SUCCESS )
    {
        return MMP_FAILURE;
    }

    
	return MMP_SUCCESS;
}

MMP_RESULT CMmpGL_MovieEx1::Close()
{
    if( CMmpGL::Close()!=MMP_SUCCESS )
    {
        return MMP_FAILURE;
    }

    if(m_pImageBuffer)
    {
        delete [] m_pImageBuffer;
        m_pImageBuffer=NULL;
    }

	return MMP_SUCCESS;
}

void CMmpGL_MovieEx1::Draw_Objects()
{
    // Clear the window with current clearing color
   // glClear(GL_COLOR_BUFFER_BIT);
    
    // Targa's are 1 byte aligned
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    //int w, h;
#if 0
    glBegin(GL_QUADS);
    w=100;
    h=100;
    glVertex2i(-w,-h);
	glVertex2i(w,-h);
	glVertex2i(w,h);
	glVertex2i(-w,h);
    glEnd();
#endif

    //GLint iViewport[4];         // Viewport

    // Use Window coordinates to set raster position
	//glRasterPos2i(-100, -100);
    glRasterPos2i(0,0);//-100, -100);

    //glGetIntegerv(GL_VIEWPORT, iViewport);
    //glPixelZoom((GLfloat)iViewport[2] / (GLfloat)m_iPicWidth/2, (GLfloat) iViewport[3] / (GLfloat)m_iPicHeight); 

	
    // Draw the pixmap
    glDrawPixels(m_iPicWidth, m_iPicHeight, GL_RGB, GL_UNSIGNED_BYTE, m_pImageBuffer);
}

void CMmpGL_MovieEx1::Draw()
{
    wglMakeCurrent(m_hDC, m_hRC);

    this->Draw_Begin();
    this->Draw_Objects();
 //   this->Draw_Axis();
    this->Draw_End();

    wglMakeCurrent(NULL, NULL);
}

void CMmpGL_MovieEx1::Key_Proc(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    this->Draw();
}
