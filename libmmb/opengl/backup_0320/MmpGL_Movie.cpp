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

#include "MmpGL_Movie.hpp"

CMmpGL_Movie::CMmpGL_Movie(HWND hWnd, HDC hDC) : CMmpGL(hWnd, hDC)
,m_mtxGrid(3,3)//GRID_VERTEX_COUNT,GRID_VERTEX_COUNT)
,m_vertexGrid(NULL)
,m_colorGrid(NULL)
{
}

CMmpGL_Movie::~CMmpGL_Movie(void)
{
	
}

MMP_RESULT CMmpGL_Movie::Open()
{
    float sinvalue[GRID_VERTEX_COUNT];
    int i, iCol, iRow, iv, ic, iidx;
    float sita,xg, fv;
    BYTE r, g, b;

    xg=2.0f*PI/(float)GRID_VERTEX_COUNT;
    sita=0;
    for(i=0;i<GRID_VERTEX_COUNT;i++)
    {
        sinvalue[i]=(float)sin(sita)*50.0f;
        sita+=xg;
    }

    m_vertexGrid=new GLfloat[ (GRID_VERTEX_COUNT*3)*(GRID_VERTEX_COUNT*3) ];
    m_colorGrid=new GLubyte[ (GRID_VERTEX_COUNT*4)*(GRID_VERTEX_COUNT*4) ];
    m_idxGrid=new GLushort[ GRID_VERTEX_COUNT*GRID_VERTEX_COUNT ];

    iv=0;
    ic=0;
    iidx=0;
    for(iCol=0; iCol<GRID_VERTEX_COUNT; iCol++)
    {
        for(iRow=0; iRow<GRID_VERTEX_COUNT; iRow++)
        {
            fv=sinvalue[iRow];
            fv+=sinvalue[GRID_VERTEX_COUNT-1-iCol];
            //m_mtxGrid.Put(iCol, iRow, fv);

            m_vertexGrid[iv++]=(iRow-(GRID_VERTEX_COUNT/2))*1.0f;
            m_vertexGrid[iv++]=(iCol-(GRID_VERTEX_COUNT/2))*1.0f;
            m_vertexGrid[iv++]=fv;

            if(fv>=0)
            {
               r=255*fv/50;
               g=0;
               b=0;
            }
            else
            {
               r=0;
               g=0;
               b=255*(-fv)/50;
            }

            m_colorGrid[ic++]=r;
            m_colorGrid[ic++]=g;
            m_colorGrid[ic++]=b;
            m_colorGrid[ic++]=255;

            m_idxGrid[iidx]=iidx;
            iidx++;
        }
    }

    if( CMmpGL::Open()!=MMP_SUCCESS )
    {
        return MMP_FAILURE;
    }

    unsigned char* image=new unsigned char[100*100*4];
    memset(image, 0x00, 100*100*4);
    //TEX_LoadRGB32(100, 100, image, 0);
    delete [] image;
    

	return MMP_SUCCESS;
}

MMP_RESULT CMmpGL_Movie::Close()
{
    if( CMmpGL::Close()!=MMP_SUCCESS )
    {
        return MMP_FAILURE;
    }

    if(m_vertexGrid)
    {
        delete [] m_vertexGrid;
        m_vertexGrid=NULL;
    }

    if(m_colorGrid)
    {
        delete [] m_colorGrid;
        m_colorGrid=NULL;
    }

    if(m_idxGrid)
    {
        delete [] m_idxGrid;
        m_idxGrid=NULL;
    }

	return MMP_SUCCESS;
}

#define GL_3D_FONT_LIST 1000
void CMmpGL_Movie::Draw_Objects()
{
#if 1
    int w,h;
    int x,y;

    glEnable( GL_TEXTURE_2D );
    
    TEX_CallList(1); 
    w=m_TEX_ImageWidth[1];
    h=m_TEX_ImageHeight[1];

    
#if 0
    glBegin(GL_LINES);

    for(y=0;y<h;y++)
    {
        for(x=0;x<w;x++)
        {
            glTexCoord2i(x, y);	
            //glColor3i(x%255,y%255,0);
            //glColor3f(0.0f, 0.0f, 0.0f);
            glVertex2i(x, y);
        }
        
    }
    glEnd();

#else

    glBegin(GL_QUADS);
#if 0
	glTexCoord2i(0, 0);	glVertex2i(0, 0);
	glTexCoord2i(w/2, 0);	glVertex2i(w,0);
	glTexCoord2i(w/2, h/2);	glVertex2i(w,h);
	glTexCoord2i(0, h/2);	glVertex2i(0,h);
#else
    w=100;
    h=100;
    glTexCoord2f(0.0f, 0.0f);	glVertex2i(-w,-h);
	glTexCoord2f(1.0f, 0.0f);	glVertex2i(w,-h);
	glTexCoord2f(1.0f, 1.0f);	glVertex2i(w,h);
	glTexCoord2f(0.0f, 1.0f);	glVertex2i(-w,h);
#endif

    glEnd();
    glDisable( GL_TEXTURE_2D );

#endif

#else
    glVertexPointer(3, GL_FLOAT, 0, m_vertexGrid);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, m_colorGrid);
    glDrawElements(GL_LINES, GRID_VERTEX_COUNT*GRID_VERTEX_COUNT, GL_UNSIGNED_SHORT, m_idxGrid);

#endif
   
}


void CMmpGL_Movie::Draw()
{
    this->Draw_Begin();
    this->Draw_Objects();
    //this->Draw_Axis();
    this->Draw_End();
}

void CMmpGL_Movie::Key_Proc(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    this->Draw();
}
