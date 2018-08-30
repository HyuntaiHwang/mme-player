// Matrix.h: interface for the CMatrix class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MATRIX_H__24F333E8_F490_4B71_BF5B_7734F2DCDC3F__INCLUDED_)
#define AFX_MATRIX_H__24F333E8_F490_4B71_BF5B_7734F2DCDC3F__INCLUDED_

#include "MmpDefine.h"

//  m_MtxSize.x=m_MtxSize.y=0 <=> 행, 열 이 없음 , 즉,  matrix  없음 
//  col, m_MtxSize.x => 행,   row, m_MtxSize.y => 열
//
//  caution;  HashTable 은  hPoint.x =>  열, hPoint.y => 행 
//            즉,  hMatrix와 순서쌍이 바뀌어져 있다.       
//            
//		 그리고  아래와 같이 명명한다.
//
//               0열 1열 2열 
//           -----------------
//		0행	|	 1   2   3
//		1행	|	 5	 6	 4
//      2행 |    6   7   5
//

class CMatrix  
{
public:
	CMatrix(int col=2, int row=2);
	CMatrix( CMatrix& mtx);
	virtual ~CMatrix();

///////////////////////////
//행렬 크기 

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
//행렬 배열 

private :
    float **m_Array;
  
	BOOL CreateArray();  //배열 생성 
	void DeleteArray();  //배열 소멸 

//////////////////////////////////////
// 입출력

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
//연산
	CMatrix& operator=(CMatrix& mtx);

	CMatrix& operator+=(CMatrix& mtx);
 	CMatrix& operator-=(CMatrix& mtx);
	CMatrix& operator*=(CMatrix& mtx);
	CMatrix& operator*=( float v );
	
    CMatrix& Add(CMatrix& mtx1, CMatrix& mtx2);
    CMatrix& Sub(CMatrix& mtx1, CMatrix& mtx2);
    CMatrix& Multi(CMatrix& mtx1, CMatrix& mtx2);
 
    float GetDet(); //Determinent 구하기 
	float CalDet( CMatrix& mtx); //
};

#endif // !defined(AFX_MATRIX_H__24F333E8_F490_4B71_BF5B_7734F2DCDC3F__INCLUDED_)
