
 /*********************************************************************
 * FILE: vector.H                                                    *
 *                                                                   *
 * AUTHOR: Matthew Miller (mlm)                                      *
 *                                                                   *
 * HISTORY:                                                          *
 *    1 JUN 93 -- (mlm) created                                      *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Template for resizeable, one dimensional array.                 *
 *                                                                   *
 *   The template is used in one of the following ways:              *
 *                                                                   *
 *     CVectorOf< type > vec;                                        *
 *     CVectorOf< type > vec( highIndex );                           *
 *     CVectorOf< type > vec( lowIndex, highIndex );                 *
 *                                                                   *
 *   Where                                                           *
 *                                                                   *
 *     type = the type of object to be contained in the vector       *
 *     vec = the vector object                                       *
 *     highIndex = highest allowable index into vec                  *
 *     lowIndex = lowest allowable index into vec                    *
 *                                                                   *
 *   When lowIndex is not given, it defaults to 0.  So vec( 5 )      *
 *   constructs a 6 element vector, indexed from vec[ 0 ] to         *
 *   vec[ 5 ].                                                       *
 *                                                                   *
 *   When neither lowIndex nor highIndex is given, no memory is      *
 *   allocated, and the vector cannot be used until it is resized    *
 *   (see below).                                                    *
 *                                                                   *
 *   The vector has the following member functions:                  *
 *                                                                   *
 *     operator[]( index ) --                                        *
 *       Access a given element of the vector.                       *
 *                                                                   *
 *     resize( highIndex ), or                                       *
 *     resize( lowIndex, highIndex ) --                              *
 *       Change the limits on the index.  New memory is only         *
 *       allocated if the old buffer isn't big enough.  Resize       *
 *       never causes the vector to free any memory.  That can only  *
 *       be done by destroying the vector and constructing a new     *
 *       one.                                                        *
 *                                                                   *
 *     clear() --                                                    *
 *       Set all the bytes in the vector to 0.  This is dangerous,   *
 *       and is meant only for use with vectors of simple data       *
 *       types.                                                      *
 *                                                                   *
 * ----------------------------------------------------------------- *
 *                                                                   *
 *             Copyright (c) 1993, NEC Research Institute            *
 *                       All Rights Reserved.                        *
 *                                                                   *
 *   Permission to use, copy, and modify this software and its       *
 *   documentation is hereby granted only under the following terms  *
 *   and conditions.  Both the above copyright notice and this       *
 *   permission notice must appear in all copies of the software,    *
 *   derivative works or modified versions, and any portions         *
 *   thereof, and both notices must appear in supporting             *
 *   documentation.                                                  *
 *                                                                   *
 *   Correspondence should be directed to NEC at:                    *
 *                                                                   *
 *                     Ingemar J. Cox                                *
 *                                                                   *
 *                     NEC Research Institute                        *
 *                     4 Independence Way                            *
 *                     Princeton                                     *
 *                     NJ 08540                                      *
 *                                                                   *
 *                     phone:  609 951 2722                          *
 *                     fax:  609 951 2482                            *
 *                     email:  ingemar@research.nj.nec.com (Inet)    *
 *                                                                   *
 *********************************************************************/

#ifndef CVECTOR_H_
#define CVECTOR_H_

#include "CMatrix.hh" // necessary only for matrix exceptions
template < class CType >
class CVectorOf
{
  private:

    int m_size;
    int m_lowIndex;
    int m_highIndex;
    CType *m_buf;
    CType *m_data;

  public:

    CVectorOf(void) :
    m_size(0),
    m_lowIndex(0),
    m_highIndex(0),
    m_buf(0),
    m_data(0)
    {
    }

    CVectorOf(int highIndex) :
    m_size(highIndex + 1),
    m_lowIndex(0),
    m_highIndex(highIndex),
    m_buf(new CType[m_size]),
    m_data(m_buf)
    {
    }

    CVectorOf(int lowIndex, int highIndex) :
    m_size(highIndex - lowIndex + 1),
    m_lowIndex(lowIndex),
    m_highIndex(highIndex),
    m_buf(new CType[m_size]),
    m_data(m_buf - lowIndex)
    {
    }

    ~CVectorOf(void)
    {
      delete[] m_buf;
    }

    CType &operator[](int index)
    {
      MATRIX_ASSERT((m_buf == 0),
                    "CVectorOf::operator[]: " << "Trying to index into unallocated vector");

      MATRIX_ASSERT(((m_lowIndex > index) || (index > m_highIndex)),
                    "CVectorOf::operator[]: " << "Index " << index << " out of bounds in vector");

      return m_data[index];
    }

    void resize(int highIndex)
    {
      int newSize = highIndex + 1;

      if (newSize > m_size) {
        delete[] m_buf;
        m_buf = new CType[newSize];
        m_size = newSize;
      }

      m_lowIndex = 0;
      m_highIndex = highIndex;
      m_data = m_buf;
    }

    void resize(int lowIndex, int highIndex)
    {
      int newSize = highIndex - lowIndex + 1;

      if (newSize > m_size) {
        delete[] m_buf;
        m_buf = new CType[newSize];
        m_size = newSize;
      }

      m_lowIndex = lowIndex;
      m_highIndex = highIndex;
      m_data = m_buf - lowIndex;
    }

    void clear(void)
    {
      if (m_buf != 0) {
        memset(m_buf, 0, (m_highIndex - m_lowIndex + 1) * sizeof(CType));
      }
    }
  int size(void)
  {
    return(m_highIndex - m_lowIndex + 1);
  }
};

#endif // CVECTOR_H_
