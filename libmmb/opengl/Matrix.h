// Matrix.h: interface for the CMatrix class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MATRIX_H__24F333E8_F490_4B71_BF5B_7734F2DCDC3F__INCLUDED_)
#define AFX_MATRIX_H__24F333E8_F490_4B71_BF5B_7734F2DCDC3F__INCLUDED_

#include "MmpDefine.h"

//  m_MtxSize.x=m_MtxSize.y=0 <=> ��, �� �� ���� , ��,  matrix  ���� 
//  col, m_MtxSize.x => ��,   row, m_MtxSize.y => ��
//
//  caution;  HashTable ��  hPoint.x =>  ��, hPoint.y => �� 
//            ��,  hMatrix�� �������� �ٲ���� �ִ�.       
//            
//		 �׸���  �Ʒ��� ���� ����Ѵ�.
//
//               0�� 1�� 2�� 
//           -----------------
//		0��	|	 1   2   3
//		1��	|	 5	 6	 4
//      2�� |    6   7   5
//

class CMatrix  
{
public:
	CMatrix(int col=2, int row=2);
	CMatrix( CMatrix& mtx);
	virtual ~CMatrix();

///////////////////////////
//��� ũ�� 

private :
	int m_nCol;  
    int m_nRow;

public :
	int  GetCol() { return m_nCol; }
	int  GetRow() { return m_nRow; }
	void SetCol( int col ) { m_nCol=col; }
	void SetRow( int row ) { m_nRow=row; }
	MMP_BOOL SetSize( int col, int row );

////////////////////////////////////
//��� �迭 

private :
    float **m_Array;
  
	BOOL CreateArray();  //�迭 ���� 
	void DeleteArray();  //�迭 �Ҹ� 

//////////////////////////////////////
// �����

public :
    float Get( int col, int row );// { return m_Array[col][row]; }
	void  Put( int col, int row, float v ) { if( col >= 0 && col < m_nCol && row >=0 && row<m_nRow ) m_Array[col][row]=v; }
	void  Put( float** varr ) 
	{
		int col, row;
		for( col=0; col<m_nCol; col++) 
			for( row=0; row<m_nRow; row++) m_Array[col][row]=varr[col][row];
	}

///////////////////////////////////////
//����
	CMatrix& operator=(CMatrix& mtx);

	CMatrix& operator+=(CMatrix& mtx);
 	CMatrix& operator-=(CMatrix& mtx);
	CMatrix& operator*=(CMatrix& mtx);
	CMatrix& operator*=( float v );
	
    CMatrix& Add(CMatrix& mtx1, CMatrix& mtx2);
    CMatrix& Sub(CMatrix& mtx1, CMatrix& mtx2);
    CMatrix& Multi(CMatrix& mtx1, CMatrix& mtx2);
 
    float GetDet(); //Determinent ���ϱ� 
	float CalDet( CMatrix& mtx); //
};

#endif // !defined(AFX_MATRIX_H__24F333E8_F490_4B71_BF5B_7734F2DCDC3F__INCLUDED_)
