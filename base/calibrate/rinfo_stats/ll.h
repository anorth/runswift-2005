/*

Copyright 2003 The University of New South Wales (UNSW) and National  
ICT Australia (NICTA).

This file is part of the 2003 team rUNSWift RoboCup entry.  You may  
redistribute it and/or modify it under the terms of the GNU General  
Public License as published by the Free Software Foundation; either  
version 2 of the License, or (at your option) any later version as  
modified below.  As the original licensors, we add the following  
conditions to that license:

In paragraph 2.b), the phrase "distribute or publish" should be  
interpreted to include entry into a competition, and hence the source  
of any derived work entered into a competition must be made available  
to all parties involved in that competition under the terms of this  
license.

In addition, if the authors of a derived work publish any conference  
proceedings, journal articles or other academic papers describing that  
derived work, then appropriate academic citations to the original work  
must be included in that publication.

This rUNSWift source is distributed in the hope that it will be useful,  
but WITHOUT ANY WARRANTY; without even the implied warranty of  
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
General Public License for more details.

You should have received a copy of the GNU General Public License along  
with this source code; if not, write to the Free Software Foundation,  
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/


#include <iostream.h>

#ifndef LL_H

#define LL_H
/*******************************************
 * ll.h
 * Name: Andres Olave
 * Date: 31/7/01
 * A Simple Linked List of objects
 *
 * Class Declarations
 *   LinkedList, LinkedList::ListElement, LinkedList::iterator
 ******************************************/


/*************************************************
 * Class LinkedList
 *
 * The managment of a linked list of ListElements
 ************************************************/


template<class T> 
class LinkedList 
{
    class ListElement;
    class iterator;

    ListElement *first;
    ListElement *last;
    unsigned int length;
    ListElement* goToPos(unsigned int pos);

/*******************************************
 * Class ListElement
 *
 * Holds a single element of the list
 ******************************************/
    class ListElement 
        {
            T* object;
            ListElement* next;
      
            ListElement(T*);
            ~ListElement() 
                {
                }
            friend class LinkedList;            
            friend class LinkedList::iterator;
        };

  public:
    LinkedList();
    ~LinkedList();
    void add(T* obj);
    void addPrefix(T* obj);
    bool remove(unsigned int pos);
    bool insert(T* obj, unsigned int pos);
    T* getObject(unsigned int pos);  
    unsigned int getLength();

// iterator begin, end methods
    iterator begin() const 
        {
            return iterator(*this);
        }
    iterator end() const 
        { 
            return iterator(); 
        }
        // iterator must be a friend of the linked list
    friend class iterator;

        /*********************************************
         * LinkedList::iterator
         *
         * Iterator for this linked list.
         * Contains default, copy and NULL constructor
         * and appropriate iterator operators: *, ++, ==, !=, bool
         *********************************************/
    class iterator
        {
            ListElement* p;
          public:
            iterator(const LinkedList& l) : p(l.first)
                {
                }
                // Copy-constructor:
            iterator(const iterator& il) : p(il.p)
                {
                }
                // The end sentinel iterator:
            iterator() : p(NULL) 
                {
                }
                // operator++ returns boolean indicating end:
            bool operator++() 
                {
                    if (p!=NULL)
                        p = p->next;
                    return bool(p);
                }
            bool operator++(int) 
                { 
                    return operator++(); 
                }
            T* current() const 
                {
                    if(p!=NULL) 
                        return p->object;
                    return NULL;
                }
            T* operator*() const 
                { 
                    return current(); 
                }
                // bool conversion for conditional test:
            operator bool() const 
                { 
                    return p!=NULL; 
                }
                // Comparison to test for end:
            bool operator==(const iterator&) const 
            {
                return p == NULL;
            }
            bool operator!=(const iterator&) const 
            {
                return p != NULL;
            }
        };
};

/******************************************
 * ListElement* goToPos(unsigned in pos)
 *
 * returns the elements at pos
 *****************************************/
template<class T> 
inline LinkedList<T>::ListElement* LinkedList<T>::goToPos(unsigned int pos)
{
       if (pos>=length)
        return NULL;

       ListElement* le;
       unsigned int i;
       for (le=first, i=0; i<pos; i++, le=le->next)
           ;
       return le;
}


/******************************************
 * Constructor
 * LinkedList:: ListElement :: ListElement(T* obj)) 
 *
 * Intialise ListElement with obj
 *****************************************/
template<class T> 
LinkedList<T>:: ListElement::ListElement(T* obj) : object(obj), next(NULL)
{
}

/*****************************************
 * Constructor
 * LinkedList :: LinkedList() 
 *
 * Intialises LinkedList to empty
 ****************************************/
template<class T> 
LinkedList<T>::LinkedList() 
{
  length=0;
  first=last=NULL;
}

    
/*****************************************
 * Constructor
 * LinkedList :: ~LinkedList() 
 *
 * Deletes all nodes of a LinkedList
 ****************************************/
template<class T> 
LinkedList<T>::~LinkedList() 
{
    ListElement* next;
    for(; first!=NULL; first=next)
    {
        next=first->next;
        delete first;
    }
}


/****************************************
 * void LinkedList::add(T* obj)
 *
 * Appends an object to a linked list
 ***************************************/
template<class T> 
void LinkedList<T>::add(T* obj) 
{
  if (length==0) 
  {
    first=last=new LinkedList::ListElement(obj);
  }
  else 
  {
    last->next=new LinkedList::ListElement(obj);
    last=last->next;
  }
  length++;
}

/****************************************
 * Method
 * void LinkedList::addPrefix(T* obj)
 *
 * Adds an object to the front of the linked list
 ***************************************/
template<class T> 
void LinkedList<T>::addPrefix(T* obj) 
{
  if (length==0) 
  {
    first=last=new LinkedList::ListElement(obj);
  }
  else 
  {
      LinkedList::ListElement* le=new LinkedList::ListElement(obj);
      le->next=first;
      first=le;
  }
  length++;
}

/****************************************
 * Method
 * void LinkedList::insert(T* obj, pos)
 *
 * Adds an object to the requested position in the linked list
 ***************************************/
template<class T> 
bool LinkedList<T>::insert(T* obj, unsigned int pos)
{
    LinkedList::ListElement* le;
    if ((le=goToPos(pos-1))==NULL)
        return false;
    LinkedList::ListElement* tmp=new LinkedList::ListElement(obj);
    tmp->next=le->next;
    le->next=tmp;
    return true;
}


/****************************************
 * Method
 * void LinkedList::remove(pos)
 *
 * Removes the elements at pos.
 *  Add: return the object at that element
 * Returns true if remove was succesful
 ***************************************/
template<class T> 
bool LinkedList<T>::remove(unsigned int pos)
{
    LinkedList::ListElement* le;
    if ((le=goToPos(pos-1))==NULL)
        return false;
    LinkedList:: ListElement* tmp=le->next->next;
    delete(le->next);
    le->next=tmp;
    return true;
}

/****************************************
 * T* getObject(unsigned int pos)
 *
 * Returns a pointer to the object at pos.  If it fails returns NULL
 ****************************************/
template<class T> 
T* LinkedList<T>::getObject(unsigned int pos)
{
    LinkedList::ListElement* le;
    if ((le=goToPos(pos))!=NULL)
        return (le->object);
    return NULL;
}
/****************************************
 * unsigned int getLength()
 *
 * Returns the length of the list
 ***************************************/
template<class T> 
unsigned int LinkedList<T>::getLength()
{
    return length;
}

#endif

