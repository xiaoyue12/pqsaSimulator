/*
 * File: linked-list.h
 * Author: Suman Banerjee <suman@cs.umd.edu>
 * Date: July 31, 2001
 * Terms: GPL
 *
 * myns simulator
 */

/* $Id: linked-list.h,v 1.1 2001/08/08 12:42:30 suman Exp suman $ */

#ifndef _LINKED_LIST_H_
#define _LINKED_LIST_H_

#include <stdio.h>

// Template for data stored in a linked list. The data is sorted
// by a key. ListData should have = operator and copy constructors
// defined. ListKey should have the same as well as comparison operators
// defined.
// The data elements are stored in increasing order sorted by the
// key parameter.
template <class ListData, class ListKey>
struct ListElement 
{
  ListData m_data;
  ListKey m_key;
  ListElement *m_prev;
  ListElement *m_next;

  ListElement ()
   {
    m_prev = NULL;
    m_next = NULL;
   }

  ListElement (ListData data, ListKey key)
   {
    m_prev = NULL;
    m_next = NULL;
    m_data = data;
    m_key = key;
   }
};

template <class ListData, class ListKey>
class LinkedList {

 private :
  int m_nSize;
  ListElement<ListData,ListKey> *m_pHead;
  ListElement<ListData,ListKey> *m_pTail;

 public :
  LinkedList (void) {m_nSize = 0; m_pHead = NULL; m_pTail = NULL; };
  ~LinkedList (void);

  int GetSize (void) {return m_nSize;};
  bool IsEmpty (void) { return (m_nSize == 0); };

  // Adds the class instance data at the appropriate position in
  // the list, and returns the position where added.
  void *Add (ListData data, ListKey key);

  // Returns the member variable, m_pHead.
  void *GetHeadPosition(void);

  // Returns the data in List Element position, pos. Assumes that
  // pos is valid.
  ListData GetAt(void*& pos) 
    { return ((ListElement<ListData,ListKey>*)pos)->m_data; }

  // Returns the data in List Element position, pos. Advances pos to
  // the next position. Assumes pos is valid.
  ListData GetNext(void*& pos);

  // Removes the specified data item given by position, from the list.
  // Assumes pos is valid.
  ListData RemoveAt(void* pos);

  // Adds all the elements in the cList to this list.
  void MergeList (LinkedList<ListData,ListKey>& cList);

  // Locate the position where data corresponding to this key is stored.
  // Returns NULL if not found.
  void *Locate (ListKey key);

  // Clears the linked list.
  void RemoveAll (void);

  // Print the contents of an <int,int> list.
  void DisplayList(void);

 private :
 // Insert a new ListElement 'before' before the element 'after'.
 // Assumes that both before and after are not NULL.
 void InsertBefore (ListElement<ListData,ListKey> *after,
				 ListElement<ListData,ListKey> *before);
};

//LinkedList::~LinkedList (void)
template<class ListData, class ListKey>
LinkedList<ListData, ListKey>::~LinkedList (void)
{
 RemoveAll();
}

template<class ListData, class ListKey>
void LinkedList<ListData, ListKey>::RemoveAll (void)
{
 ListElement<ListData,ListKey> *iter = m_pHead;

 while (iter != NULL)
   {
    ListElement<ListData,ListKey> *iter_next = iter->m_next;
    delete iter;
    iter = iter_next;
   }
 m_nSize = 0;
 m_pHead = NULL;
 m_pTail = NULL;
 return;
}

template <class ListData, class ListKey>
void * LinkedList<ListData, ListKey>::Add (ListData data, ListKey key)
{
 ListElement<ListData,ListKey> *pNewLe = new ListElement<ListData,ListKey>(data,key);
 m_nSize ++;

 if (m_nSize == 1)
   {
    m_pHead = pNewLe;
    m_pTail = pNewLe;
    return (void*)pNewLe;
   }

 ListElement<ListData,ListKey> *iter = m_pHead;
 while (iter != NULL)
   {
    if (key < iter->m_key)
      {
       InsertBefore (iter, pNewLe);
       if (iter == m_pHead)
          m_pHead = pNewLe;
       return (void*)pNewLe;
      }
    iter = iter->m_next;
   }

 // Inserting in the tail
 m_pTail->m_next = pNewLe;
 pNewLe->m_prev = m_pTail;
 m_pTail = pNewLe;
 return (void*)pNewLe;
}

template <class ListData, class ListKey>
void * LinkedList<ListData, ListKey>::GetHeadPosition (void)
{
 return (void*)m_pHead;
}

template <class ListData, class ListKey>
ListData LinkedList<ListData, ListKey>::GetNext(void*& pos)
{
 ListElement<ListData,ListKey> *pLe = (ListElement<ListData,ListKey>*)pos;
 ListData ret_val = pLe->m_data;

 pos = (void*)(pLe->m_next);
 return ret_val;
}
 
template <class ListData, class ListKey>
ListData LinkedList<ListData, ListKey>::RemoveAt(void* pos)
{
 ListElement<ListData,ListKey> *pLe = (ListElement<ListData,ListKey>*)pos;
 ListData ret_val = pLe->m_data;

 m_nSize --;

 if (pLe->m_prev != NULL)
   pLe->m_prev->m_next = pLe->m_next;
 if (pLe->m_next != NULL)
   pLe->m_next->m_prev = pLe->m_prev;

 if (m_pHead == pLe)
   m_pHead = pLe->m_next;
 if (m_pTail == pLe)
   m_pTail = pLe->m_prev;

 delete pLe;
 return ret_val;
}

template <class ListData, class ListKey>
void LinkedList<ListData, ListKey>::MergeList (LinkedList<ListData,ListKey>& cList)
{
  for (void *pos = cList.GetHeadPosition();
       pos != NULL;
       cList.GetNext(pos) )
    {
      ListElement<ListData,ListKey> *pLe = (ListElement<ListData,ListKey>*)pos;
      Add(pLe->m_data,pLe->m_key);
    }
  return;
}

template <class ListData, class ListKey>
void * LinkedList<ListData, ListKey>::Locate (ListKey key)
{
 ListElement<ListData,ListKey> *pLe = m_pHead;
 while (pLe != NULL)
   {
    if (pLe->m_key == key)
      return (void*)pLe;
    if (pLe->m_key > key)
      return NULL;
    pLe = pLe->m_next;
   }

 return NULL;
}
      
template <class ListData, class ListKey>
void LinkedList<ListData, ListKey>::DisplayList(void)
{
 printf ("Display of list : Size : %d\n", m_nSize);
 ListElement<ListData,ListKey> *iter = m_pHead;

 while (iter != NULL)
   {
    printf ("Item : %d\n",(*(int*)&(iter->m_data)));
    iter = iter->m_next;
   }
}

// New element before is inserted before the element after.
template <class ListData, class ListKey>
void LinkedList<ListData, ListKey>::InsertBefore (ListElement<ListData,ListKey> *after, ListElement<ListData,ListKey> *before)
{
 if (after->m_prev != NULL)
    after->m_prev->m_next = before;
 before->m_prev = after->m_prev;
 after->m_prev = before;
 before->m_next = after;
 return;
}


#endif
