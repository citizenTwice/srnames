/*

MIT License

Copyright (c) 2021 Luigi Galli

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

/*
 
 Dynamic array class
 stripped down, pre-C++11 version, no move-semantics etc.

*/

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>

// static constexpr uint32_t DEFAULT_DYNARRAY_EXPAND_BY = 16U;
#define DEFAULT_DYNARRAY_EXPAND_BY 16U
template <class T, int EXPAND_BY=DEFAULT_DYNARRAY_EXPAND_BY> struct DYNARRAY {
private: 
#if !defined(__DOS__)
  DYNARRAY(const DYNARRAY& other) = delete;
  DYNARRAY& operator=(const DYNARRAY& other) = delete;
#endif
public:
  T*       m_buf;
  uint32_t m_capacity;
  uint32_t m_size;
  ~DYNARRAY() {
    if (m_buf) {
      free_bytes(m_buf);
    }
  }
  DYNARRAY() {
    m_buf = NULL;
    m_capacity = 0;
    m_size = 0;
  }
  static void free_bytes(T* p) {
    assert(p);
    ::free(p);
  }
  static T* alloc_bytes(uint32_t sz) {
    assert(sz);
    T* retval = reinterpret_cast<T*>(calloc(sz, 1));
    assert(retval);
    return retval;
  }
  DYNARRAY(T* ptr, T* end) {
    assert(ptr < end);
    uint32_t sz = EXPAND_BY + (end - ptr);
    m_capacity = sz;
    m_buf = alloc_bytes(uint32_t(sz*sizeof(T)));
    assert(m_buf != NULL);
    m_size = end - ptr;
    memcpy(m_buf, ptr, sz*sizeof(T));
  }
  bool append(T elm) {
    if (m_buf == NULL) {
      m_capacity = EXPAND_BY;
      m_size = 0;
      if ((m_buf = alloc_bytes(uint32_t(m_capacity*sizeof(T)))) == NULL) {
        return false;
      }
    } else if (m_buf != NULL && m_capacity == m_size ){
      T* oldbuf = m_buf;
      m_capacity = m_size + EXPAND_BY;
      if ((m_buf = alloc_bytes(uint32_t(m_capacity*sizeof(T)))) == NULL) {
        return false;
      }
      memcpy(m_buf, oldbuf, m_size*sizeof(T));
      free_bytes(oldbuf);
    }    
    assert(m_capacity > m_size);
    m_capacity--;
    m_size++;
    m_buf[m_size-1] = elm;
    return true;
  }
  bool prepend(T elm) {
    if (m_buf == NULL) {
      m_capacity = EXPAND_BY;
      m_size = 0;
      if ((m_buf = alloc_bytes(uint32_t(m_capacity*sizeof(T)))) == NULL) {
        return false;
      }
    } else if (m_buf != NULL && m_capacity == m_size ){
      T* oldbuf = m_buf;
      m_capacity = m_size + EXPAND_BY;
      if ((m_buf = alloc_bytes(uint32_t(m_capacity*sizeof(T)))) == NULL) {
        return false;
      }
      memcpy(m_buf, oldbuf, m_size*sizeof(T));
      free_bytes(oldbuf);
    }
    assert(m_capacity > m_size);
    ::memmove(&(m_buf[1]), &(m_buf[0]), m_size*sizeof(T));
    m_buf[0] = elm;
    m_capacity--;
    m_size++;
    return true;
  }
  bool push(T elm) {
    return append(elm);
  }
  T pop() {
    assert(m_buf != NULL && m_size != 0);
    return m_buf[--m_size];
  }
  T get(uint32_t index) {
    assert(m_buf != NULL && m_size != 0 && index < m_size);
    return m_buf[index];
  }
  T& operator[](uint32_t pos) {
    assert(m_buf);
    return m_buf[pos];
  }
  uint32_t size() {
    return m_size;
  }
  T& last() {
    assert(m_buf);
    return m_buf[m_size - 1];
  }
  T& first() {
    assert(m_buf);
    return m_buf[0];
  }
  void clear() {
    m_size = 0;
  }
  bool is_empty() {
    return m_size == 0;
  }
//   typedef T* iterator;
//   typedef const T* const_iterator;
}; 

#if defined(UNIT_TEST)

static void DYNARRAY_unit_test0() {  
  {
    DYNARRAY<uint32_t> arr;
    assert(arr.size() == 0);
    assert(arr.is_empty());
    arr.append(0);
    assert(arr.size() == 1);
    assert(!arr.is_empty());
    arr.append(1);
    arr.append(2);
    arr.append(3);
    assert(arr.size() == 4);
    assert(arr.first() == 0);
    assert(arr.last() == 3);
    assert(arr[0] == 0);
    assert(arr[1] == 1);
    assert(arr[2] == 2);
    assert(arr[3] == 3);
    assert(!arr.is_empty());
    arr.clear();
    assert(arr.is_empty());
  }
  {
    DYNARRAY<const _TCHAR*> arr;
    assert(arr.size() == 0);
    assert(arr.is_empty());
    arr.append(_T("0000"));
    assert(arr.size() == 1);
    assert(!arr.is_empty());
    arr.append(_T("1111"));
    arr.append(_T("1112"));
    arr.append(_T("1113"));
    assert(arr.size() == 4);
    assert(arr.first() == std::string(_T("0000")));
    assert(arr.last() ==  std::string(_T("1113")));
    assert(arr[0] == std::string(_T("0000")));
    assert(arr[1] == std::string(_T("1111")));
    assert(arr[2] == std::string(_T("1112")));
    assert(arr[3] == std::string(_T("1113")));
    arr.prepend(_T("A"));
    assert(arr.first() == std::string(_T("A")));
    assert(arr.last() ==  std::string(_T("1113")));
    assert(arr[0] == std::string(_T("A"   )));
    assert(arr[1] == std::string(_T("0000")));
    assert(arr[2] == std::string(_T("1111")));
    assert(arr[3] == std::string(_T("1112")));
    assert(arr[4] == std::string(_T("1113")));
    arr.push(_T("Z"));
    assert(arr.size() == 6);
    assert(arr.last() == std::string(_T("Z")));
    assert(arr.pop() == std::string(_T("Z")));
    assert(arr.size() == 5);
    assert(!arr.is_empty());
    arr.clear();
    assert(arr.is_empty());
  }
}

static void DYNARRAY_unit_test() {
  printf(_T("DYNARRAY TEST...\n"));
  DYNARRAY_unit_test0();
}

#endif
