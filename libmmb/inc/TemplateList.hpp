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


#ifndef TEMPLATE_LIST_HPP__
#define TEMPLATE_LIST_HPP__

template<class Type> class TList;
template<class Type> class TQueue;
template<class Type> class TStack;
 
template<class Type> class TListNode
{
   friend class TList<Type>;
   friend class TQueue<Type>;
   friend class TStack<Type>;

   public :
	   Type GetData() { return m_data; }
	   
   private :
	     Type m_data;
		 TListNode* m_link;
		 TListNode(Type d) {  m_data=d; m_link=0; }
	     TListNode() { m_link=0; }
};


//////////////////////////////////////////////////////////////
// class List
// This is Single List 
template<class Type> class TList
{
   public :
	   TList()  { m_head=new TListNode<Type>; m_tail=m_head; m_CurNum=0;  }
	   ~TList();
       void Add( Type data); 
       void Add( TListNode<Type>* x); 

	   void Insert( int i, Type data  );
   
       Type Get( int i); //Get ith data
       void Del( int i); //Delete ith data
	   void Del_All();   //Delete all data
	   bool Del_Cur();
	   void Set( int i, Type data );
       int Search( Type v);// search the value being equal to "v", and return the Index    
       int GetSize() { return m_CurNum; }
	   void Delete( TListNode<Type>* delNode, TListNode<Type>* fNode); // delete delNode after node fNode

	   TListNode<Type>* First(); 
       TListNode<Type>* Next();

       bool SeekFirst();
	   bool SeekRandom( int index );
       
	   bool GetFirst(Type& data);
	   bool GetNext(Type& data);
	   
	   bool GetLast(Type& data);
	   bool GetBefore(Type& data);
	   
	   bool GetRandom(int index, Type& data);
	   
	   // Cross Set of two list " list1, list2 "
	   TList<Type>& Cross( TList<Type>& list1, TList<Type>& list2 );
	   
	   TListNode<Type>* GetFirstNode(); 
	   TListNode<Type>* GetLastNode(); 
	   TListNode<Type>* GetNextNode(TListNode<Type>* pCurNode); 
  	   TListNode<Type>* GetNode(int i);
	   
   private :
       TListNode<Type>* m_head;
	   TListNode<Type>* m_tail;
	   TListNode<Type>* m_current;
	   TListNode<Type>* m_before;
	   int m_CurNum;  //current node count
};

template<class Type> TList<Type>::~TList()
{
  TListNode<Type> *node, *next;
  node=m_head;
  while( node )
  {
     next=node->m_link;
	 delete node;
	 node=next;
  }
}

template<class Type> Type TList<Type>::Get(int i)
{
  Type t;
  TListNode<Type> *node;
  if( i<0 || i>=m_CurNum ) 
  {
	  return t;
  }
  
  node=m_head->m_link;  
  int j;
  for( j=0; j<i; j++)
  {
	  if(node==0) 
	        return t; 
      node=node->m_link;
  }
  if(node==0) 
        return t;
  return node->m_data;
}

template<class Type> void TList<Type>::Set(int i, Type data)
{
  //Type t;
  TListNode<Type> *node;
  if( i<0 || i>=m_CurNum ) return;
  
  node=m_head->m_link;  
  if(node==0) return;

  int j;
  for( j=0;j<i; j++)
  {
      node=node->m_link;
	  if(node==0) return;
  }
  node->m_data=data;
}

template<class Type> void TList<Type>::Del(int i)
{
  if( i<0 || i>=m_CurNum ) return;
  
  TListNode<Type>* cur=m_head->m_link;
  TListNode<Type>* front=m_head;  
  
  if(cur==0) return;

  int j;
  for( j=0; j<i; j++)
  {
     front=cur; 
	 cur=cur->m_link;
	 if(cur==0) return;
  }
  
  Delete( cur, front);
 
}

template<class Type> void TList<Type>::Del_All()
{
  TListNode<Type> *node, *next;
  node=m_head;
  while( node )
  {
     next=node->m_link;
	 delete node;
	 node=next;
  }
  
  m_head=new TListNode<Type>; 
  m_tail=m_head; 
  m_CurNum=0;
}

template<class Type> void TList<Type>::Add( Type d )
{

  TListNode<Type>* node=new TListNode<Type>(d);
  if(m_head==0)  
  { 
     delete node; 
     return; 
  }
  else 
  {
	  m_tail->m_link=node;
	  m_tail=node;
	  m_CurNum++;
  }
  
}

template<class Type> void TList<Type>::Insert(  int i, Type d )
{
  TListNode<Type>* node=new TListNode<Type>(d);
  TListNode<Type>* cur=m_head->m_link;  
  TListNode<Type>* before=m_head;
  int j;
  for(j=0;j<i;j++)
  {
	  before=before->m_link;
      cur=cur->m_link;
  }
  
  node->m_link=cur;
  before->m_link=node;

  m_CurNum++;
}

template<class Type> void TList<Type>::Add( TListNode<Type>* node )
{
  if(!node) 
      return;
  
  if(!m_head)  
        return; 
  else 
  {
     m_tail->m_link=node;
	 m_tail=node;
	 m_CurNum++;
  }
}

template<class Type> 
void TList<Type>::Delete( TListNode<Type>* delNode, TListNode<Type>* fNode)
{
   if( delNode == m_tail ) 
       m_tail=fNode;
   
   fNode->m_link=delNode->m_link;
   delete delNode;
   m_CurNum--;
}

template<class Type>  TListNode<Type>* TList<Type>::First()
{
  m_current=m_head->m_link;
  return m_head->m_link;
}

template<class Type>  TListNode<Type>* TList<Type>::Next()
{
   m_current=m_current->m_link;
   return m_current;   
}

template<class Type>  TListNode<Type>* TList<Type>::GetFirstNode()
{
  return m_head->m_link;
}

template<class Type>  TListNode<Type>* TList<Type>::GetLastNode()
{
  return m_tail;
}

template<class Type>  
TListNode<Type>* TList<Type>::GetNextNode(TListNode<Type>* pCurNode)
{
  return pCurNode->m_link;
}

template<class Type>  TListNode<Type>* TList<Type>::GetNode(int i)
{
  TListNode<Type> *node;
  if( i<0 || i>=m_CurNum ) 
  {
	  return (TListNode<Type>*)0;
  }
  
  node=m_head->m_link;  
  int j;
  for( j=0; j<i; j++)
  {
	  if(node==0) 
	  {
        return (TListNode<Type>*)0;
      }
      node=node->m_link;
  }
  
  return node;
}


template<class Type> bool TList<Type>::SeekFirst()
{
  m_current=m_head->m_link;
  return (m_current)?true:false;
}

//n is 0 based index.
//The Head Node will not be counted.
template<class Type> bool TList<Type>::SeekRandom( int n )
{
  if( n >= this->GetSize() || n<0 ) 
        return false;
  m_current=m_head->m_link;
  
  for( int i=0; i<n; i++)   
    m_current=m_current->m_link;
    
  return true;
}

template<class Type> bool TList<Type>::Del_Cur()
{
  TListNode<Type>* cur;
  TListNode<Type>* front;

  if(m_current==0) 
       return false;

  front=m_before;
  cur=m_current;
  
  m_current=m_current->link;
 
  this->Delete( cur, front);

  return true;
}

template<class Type> bool TList<Type>::GetFirst(Type& data)
{
  m_current=m_head->m_link;  
  if( m_current == 0 ) 
       return false;
  data=m_current->m_data;
  m_before=m_head;
  return true;
}

template<class Type> bool TList<Type>::GetRandom( int n , Type& data)
{
  if( !SeekRandom(n) ) 
     return false;
  data=m_current->m_data;
  return true;
}

template<class Type>  bool TList<Type>::GetNext(Type& data)
{
  if(m_current==0) 
      return false;
      
  m_before=m_current;
  m_current=m_current->m_link;
  if(m_current==0) 
     return false;
  data=m_current->m_data;   
  return true;
}

template<class Type> bool TList<Type>::GetLast(Type& data)
{
  if(m_head==m_tail) 
       return false;  //empty node

  m_current=m_tail;  
  if( m_current == 0 ) 
      return false;
  data=m_current->m_data;

  return true;
}
	   
template<class Type>  bool TList<Type>::GetBefore(Type& data)
{
  TListNode<Type>* cur;
  TListNode<Type>* before;
  
  if( m_current==m_head->m_link ) 
       return false;
  if( m_head->m_link==0) 
       return false;

  cur=m_head->m_link->m_link;
  before=m_head->m_link;
   
  while(cur)
  {
    if(cur==m_current ) 
	{
	  data=before->m_data;
	  m_current=before;
	  return true;
	}
    
	before=cur;
	cur=cur->m_link;
  }
  
  return false;
}
	   

template<class Type> 
TList<Type>& TList<Type>::Cross( TList<Type>& list1, TList<Type>& list2)
{
  int i,j;
  Type v1, v2;
  
  // delete all node  
  for( i=0; i<m_CurNum; i++) this->Del(0);
  
  for( i=0; i<list1.GetSize(); i++)  
  {
      v1=list1.Get(i);
      
      for( j=0; j<list2.GetSize(); j++)
       {
        v2=list2.Get(j);
     	if( v1==v2 ) {this->Add(v1); break;}
       }
  }
  
  return *this;
}

template<class Type> int TList<Type>::Search( Type v )
{
  int i;
  for( i=0; i<this->GetSize(); i++)
  {
    if( v==this->Get(i) ) return i;
  }
  return -1;
}

// end of class List
/////////////////////////

//////////////////////////////////////////////////////////////////
//clas Stack

template<class Type> class TStack
{
public :
      TStack(int arraySize)  
      { 
         m_nArraySize=arraySize; 
         m_Array=new Type[m_nArraySize+1];
         m_nHeadIndex=-1;
         m_nDataSize=0;
      }
	   ~TStack()
      {
         delete [] m_Array;
         m_Array=0;
      }

	   void Add( const Type& ele)
      {
         m_nHeadIndex++;
         m_Array[m_nHeadIndex]=ele;
         m_nDataSize++;
      }

	   Type* Delete(Type& ele)
      {
         ele=m_Array[m_nHeadIndex];
         m_nHeadIndex--;
         m_nDataSize--;
         return &ele;
      }
	   
      bool IsEmpty() { return (m_nHeadIndex<0)?true:false; }
	   bool IsFull() { return (m_nHeadIndex==m_nArraySize-1)?true:false; }
	   int GetSize() { return m_nDataSize; }
      void Flush()
      {
         m_nHeadIndex=-1;
         m_nDataSize=0;
      }

private :
      Type* m_Array;
      int m_nArraySize; 
      int m_nDataSize;
      int m_nHeadIndex;

};


//////////////////////////////////////////////////////////////////
//clas Stack

template<class Type> class TDynamicStack
{
public :
	   TDynamicStack()   { front=0; size=0;  }
	   ~TDynamicStack();

	   void Add( const Type& ele);
	   Type* Delete(Type& );
	   
	   int IsEmpty() { if( !front ) return 1; else return 0; }
	   int getSize() { return size; }

private :
      TListNode<Type>* front;
	  int size; 

};

template<class Type> TDynamicStack<Type>::~TDynamicStack()
{
  TListNode<Type> *node, *temp;
  node=front;
  while( node )
  {
     temp=node->link;
	 delete node;
	 node=temp;
  }
}

template<class Type> void TDynamicStack<Type>::Add(const Type& ele)
{
	TListNode<Type>* temp=new TListNode<Type>(ele);
	temp->link=front;
	front=temp;
    size++;
}

template<class Type> Type* TDynamicStack<Type>::Delete(Type& ele)
{
	if( !front ) return 0; 
    else {
	       TListNode<Type>* temp=front;
		   ele=front->data; 
	       front=front->link;
		   delete temp;
		   size--;
		   return &ele;
	}
}

//////////////////////////////////////////////////////////////
// class Queue
// Single List를 이용한  Queue이댜.  front, rear을 가진다. 
template<class Type> class TQueue
{
   public :
	   TQueue()   { m_front=m_rear=0; m_size=0;  }
	   ~TQueue();
       
	   void Add( const Type& ele);
	   Type* Delete(Type& );
	   int IsEmpty() { if( !m_front ) return 1; else return 0; }
	   int getSize() { return m_size; }

	   void Flush() {  Type par; while( !IsEmpty() ) Delete(par); }

   private :
       TListNode<Type>* m_front;
	   TListNode<Type>* m_rear;
	  int m_size; 
	    
};

template<class Type> TQueue<Type>::~TQueue()
{
  TListNode<Type> *node, *temp;
  node=m_front;
  while( node )
  {
     temp=node->m_link;
	 delete node;
	 node=temp;
  }
}

template<class Type> void TQueue<Type>::Add(const Type& ele)
{
	TListNode<Type>* temp=new TListNode<Type>(ele);
	if(temp==0) 
		return;
	
	if( m_rear==0 ) 
	{ 
		m_rear=temp;
	    m_front=m_rear; 
	}
    else 
	{
	    m_rear->m_link=temp;
	    m_rear=temp;
	}
    m_size++;
}

template<class Type> Type* TQueue<Type>::Delete(Type& ele)
{
	if( m_front==0 ) 
        return 0; 
    else 
    {
	       TListNode<Type>* temp=m_front;
		   ele=m_front->m_data; 
	       m_front=m_front->m_link;
		   if(!m_front) m_rear=m_front;
		   delete temp;
		   m_size--;
		   return &ele;
	}
}

//////////////////////////////////////////////////////////////
// class Circular_Queue
template<class Type> class TCircular_Queue
{
   public :
	   TCircular_Queue(int array_size=16)   
	   { 
		   m_count=0;
		   m_front=m_rear=0; m_size=array_size+1;  
		   m_Array=new Type[m_size]; 
	   }
	   ~TCircular_Queue()
	   {
	       if(m_Array) delete [] m_Array;
	   }

       
      bool Add( const Type& ele)
      {
         m_Array[m_rear]=ele;
         m_rear++;
         m_rear%=m_size;
         m_count++;
         return true;
      }
	   
	   Type* Delete(Type& ele)
	   {
         ele=m_Array[m_front];
         m_front++;
         m_front%=m_size;
         m_count--;
         return &ele;  
	   }

      Type* GetFirstItem(Type& ele)
      {
	      ele=m_Array[m_front];
         return &ele;  
      }
	  
      Type* SetFirstItem(Type& ele)
      {
	      m_Array[m_front] = ele;
          return &ele;  
      }
	  
      Type* GetItem(int index, Type& ele)
	   {
          int i;
          i=m_front+index;
          i%=m_size;
          ele=m_Array[i];
          return &ele;  
	   }
	   
	   int IsFull() 
	   {
         int v;
         v=m_rear+1;
         v%=m_size;
         return (m_front==v) ? 1 : 0;
	   }
	   
	   int IsEmpty() { return m_front==m_rear; }
	   int GetTotalSize() { return m_size; }
	   int GetSize() {  return m_count; }
	   int GetEmptySize() { return m_size-1-m_count; }
      bool SetSize( int array_size )
	   { 
         if(m_Array) delete [] m_Array;
		   m_front=m_rear=0; m_size=array_size;  
		   m_Array=new Type[array_size]; 
		   if(m_Array) return true;
		   else return false;
	   }
	   void Flush()
	   {
	      m_front=m_rear=0;
	      m_count=0;
	   }
	      
   private :
      int m_count;
      int m_front;
	   int m_rear;
	   int m_size; 
	   Type* m_Array; 
};


//////////////////////////////////////////////////////////////
// class HashTable
// Single List를 이용한  HashTable이다.  
template<class Type> class THashTable
{
   public :

	   THashTable();
	   ~THashTable();

	   bool del_all();


   private :

	   TList< TList<Type>* > m_TableList;
};

template<class Type> THashTable<Type>::THashTable()
{
  
}

template<class Type> THashTable<Type>::~THashTable()
{
	del_all();
}


template<class Type> bool THashTable<Type>::del_all()
{
  TList<Type>* pTable;

  if(!m_TableList.SeekFirst()) return false; 

  while(1)
  {
    if( !m_TableList.GetNext( pTable ) ) break;
	if(pTable) delete pTable;
  }

  return true;
}



//////////////////////////////////////////////////////////////
// class hArray
// 
template<class Type> class TArray
{
   public :
	   TArray()   { m_size=0; m_data=0; m_index=0; }
	   TArray(int size)   { m_size=size; m_data=new Type[size];  m_index=0; }
	   TArray(const TArray& array)   { m_size=0; m_data=0; *this=array;  m_index=0; }
	   ~TArray() { if(m_data) delete [] m_data; }
       
      TArray& operator=( const TArray& array )
      {
         Flush();
         SetSize(array.m_size);
         for( int i=0; i<m_size; i++)
            m_data[i]=array.m_data[i];
         return *this;
      }
	   
      TArray& operator=( Type v )
      {
         for( int i=0; i<m_size; i++) m_data[i]=v;
         return *this;
      }

      TArray& operator+=( TArray& array )
      {
         for( int i=0; i<m_size; i++)  m_data[i]+=array.m_data[i];
         return *this;
      }

      TArray& operator-=( TArray& array )
      {
         for( int i=0; i<m_size; i++)  m_data[i]-=array.m_data[i];
         return *this;
      }

      void Set(int i, const Type& v) { m_data[i]=v; }
      Type Get(int i) { return m_data[i]; }
      Type* Get() { return m_data; }

      int GetSize() { return m_size; }
      void SetSize( int size ) 
      { 
		   Flush(); 
         m_size=size; m_data=new Type[size]; 
      }
      void Flush() {  if(m_data) delete [] m_data; m_data=0; m_size=0; }

      void Attach( Type* data, int size )
      {
         Flush();
         m_data=data;
         m_size=size;
      }
    
      //buffer로 사용한다.
      void AddToBuffer( Type data )
      {
         m_data[m_index]=data;
         m_index++;
         m_index%=m_size;  
      }  

      void GetFromBuffer( Type* buffer )
      {
         int i, j;
         for( j=0, i=m_index; i<m_size; i++, j++)
            buffer[j]=m_data[i];
         for( i=0; i<m_index; i++, j++)
            buffer[j]=m_data[i];
      }

private :
      Type* m_data;
      int m_size; 
	   int m_index;   
};

//////////////////////////////////////////////////////////////
// class hArray_Multi
//              col0   col1  col2 ...
//       row0                         <==  array0
//       row1                         <==  array1
//       row2                         <==  array2
//       row3                         <==  array3 
//
template<class Type> class TArray_Multi
{
   public :
	   TArray_Multi()   { m_row=m_col=0; m_data=0; }
	   TArray_Multi(int row, int col)   { m_row=row, m_col=col; m_data=new Type[row*col]; }
	   ~TArray_Multi() { if(m_data) delete [] m_data; }

       int GetRowSize() { return m_row;}
	   int GetColSize() { return m_col; }

	   Type* Get( int row ) {  return m_data+m_col*row; }
	   Type Get( int row, int col ) {  return m_data[ m_col*row+col ]; }
	   void Set( int row, int col, Type v ) { m_data[ m_col*row+col ]=v; }
	   void Set( int row, Type* v ) {  for( int icol=0; icol<m_col; icol++) this->Set( row, icol, v[icol] ); }

	   void SetSize( int row, int col )
	   {
	      if(m_data) delete [] m_data;
		  m_data=new Type[row*col];
		  m_row=row, m_col=col;
	   }

   private :
      Type* m_data;
	  int m_row, m_col; 
	  
};


/////////////////////////////////////////////////////////////
//Min Heap
template<class Type> class TMinHeap
{
public:
	TMinHeap( int heapsize=64 ) { m_heap=new Type[heapsize]; m_heapsize=heapsize; m_tail=0;}
	~TMinHeap() { delete [] m_heap; }

    void Add( const Type& ele);
	Type* Delete(Type& );
	bool IsEmpty() { return m_tail>0?false:true; }
	bool IsFull() { return (m_tail<(m_heapsize-1))?false:true; }
	int getHeapMaxSize() { return m_heapsize; }
	int getHeapSize() { return m_tail; }

private:
	Type* m_heap;
	int m_heapsize;
	int m_tail;
};

template<class Type> void TMinHeap<Type>::Add(const Type& ele)
{
	int parent;
	int child;
	Type t;

	m_heap[m_tail]=ele;
	
	child=m_tail;
	parent=(child-1)/2;
    while( parent!=child )
	{
	  if( m_heap[parent] > m_heap[child] )
	  {
	     t=m_heap[parent];
		 m_heap[parent]=m_heap[child];
		 m_heap[child]=t;
	  }
	  else break;

	  child=parent;
	  parent=(child-1)/2;
	}
	m_tail++;
}

template<class Type> Type* TMinHeap<Type>::Delete(Type& ele)
{
   
   int parent;
   int child1, child2;
   int child;
   Type t, t1, t2, p;
	
   if( m_tail==0 ) return (Type*)0;

   ele=m_heap[0];
   m_heap[0]=m_heap[m_tail-1];
   m_tail--;
   
   parent=0;
   child1=parent*2+1;
   child2=(parent+1)*2;
   while( child2<m_tail || child1<m_tail)
   {
      if( child2<m_tail )
	  {
	       t1=m_heap[child1];
		   t2=m_heap[child2];
		   p=m_heap[parent];

		   if( (t1>=t2) && (p>t2) )
		   {
		       m_heap[child2]=p;
			   m_heap[parent]=t2;
			   child=child2;
		   }
		   else if( (t2>=t1) && (p>t1) )
		   {
		       m_heap[child1]=p;
			   m_heap[parent]=t1;
			   child=child1;
		   }
		   else {
               break;
           }

	  }
	  else if(child1<m_tail)
	  {
       
	       t1=m_heap[child1];
		   p=m_heap[parent];

		   if( p>t1 )
		   {
		       m_heap[child1]=p;
			   m_heap[parent]=t1;
			   child=child1;
		   }
		   else break;
	  }
	  parent=child;
      child1=parent*2+1;
      child2=(parent+1)*2;
   }
   
   return &ele;	
}


/////////////////////////////////////////////////////////////
//Max Heap
template<class Type> class TMaxHeap
{
public:
	TMaxHeap( int heapsize=64 ) { m_heap=new Type[heapsize]; m_heapsize=heapsize; m_tail=0;}
	~TMaxHeap() { delete [] m_heap; }

    void Add( const Type& ele);
	Type* Delete(Type& );
	bool IsEmpty() { return m_tail>0?false:true; }
	int getHeapMaxSize() { return m_heapsize; }
	int getHeapSize() { return m_tail; }

private:
	Type* m_heap;
	int m_heapsize;
	int m_tail;
};

template<class Type> void TMaxHeap<Type>::Add(const Type& ele)
{
	int parent;
	int child;
	Type t;

	m_heap[m_tail]=ele;
	
	child=m_tail;
	parent=(child-1)/2;
    while( parent!=child )
	{
	  if( m_heap[parent] < m_heap[child] )
	  {
	     t=m_heap[parent];
		 m_heap[parent]=m_heap[child];
		 m_heap[child]=t;
	  }
	  else break;

	  child=parent;
	  parent=(child-1)/2;
	}
	m_tail++;
}

template<class Type> Type* TMaxHeap<Type>::Delete(Type& ele)
{
   
   int parent;
   int child1, child2;
   int child;
   Type t, t1, t2, p;
	
   if( m_tail==0 ) return (Type*)0;

   ele=m_heap[0];
   m_heap[0]=m_heap[m_tail-1];
   m_tail--;
   
   parent=0;
   child1=parent*2+1;
   child2=(parent+1)*2;
   while( child2<m_tail || child1<m_tail)
   {
      if( child2<m_tail )
	  {
	       t1=m_heap[child1];
		   t2=m_heap[child2];
		   p=m_heap[parent];

		   if( t1<t2 && p<t2 )
		   {
		       m_heap[child2]=p;
			   m_heap[parent]=t2;
			   child=child2;
		   }
		   else if( t2<t1 && p<t1 )
		   {
		       m_heap[child1]=p;
			   m_heap[parent]=t1;
			   child=child1;
		   }
		   else break;

	  }
	  else if( child1<m_tail)
	  {
       
	       t1=m_heap[child1];
		   p=m_heap[parent];

		   if( p<t1 )
		   {
		       m_heap[child1]=p;
			   m_heap[parent]=t1;
			   child=child1;
		   }
		   else break;
	  }
	  parent=child;
      child1=parent*2+1;
      child2=(parent+1)*2;
   }
   
   return &ele;	
}



#endif

