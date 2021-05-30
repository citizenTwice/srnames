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

 Bulk string allocator class.

 */

#pragma once

#ifndef MAX_STRBLK_SEGMENTS
#define MAX_STRBLK_SEGMENTS 4096
#endif

#ifndef DEFAULT_STRBLK_SIZE
#define DEFAULT_STRBLK_SIZE 65536
#endif


struct STRBLK {
private: 
#if !defined(__DOS__)
  STRBLK(const STRBLK& other) = delete;
  STRBLK& operator=(const STRBLK& other) = delete;
#endif
private:
  struct SEGMENT  {
    _TCHAR* m_baseptr;
    uint32_t  m_size_in_chars;
    _TCHAR* m_freespace_ptr;
  };

  uint32_t m_current;
  SEGMENT* m_segments;

public:  
  STRBLK() {
    m_current = 0;
    m_segments = (SEGMENT*)(calloc(sizeof(SEGMENT),MAX_STRBLK_SEGMENTS) );
    memset(m_segments, 0, sizeof(SEGMENT)*MAX_STRBLK_SEGMENTS);
  }

  _TCHAR* alloc(uint32_t len_in_chars) {
    uint32_t tlen = len_in_chars + 1;
    if (m_current >= MAX_STRBLK_SEGMENTS ) {
      return 0;
    }
    if (m_segments[m_current].m_baseptr == 0) {
      uint32_t required_size_in_chars = DEFAULT_STRBLK_SIZE > (tlen) ? DEFAULT_STRBLK_SIZE : (tlen);  
      m_segments[m_current].m_size_in_chars = required_size_in_chars;
      m_segments[m_current].m_baseptr = 
        m_segments[m_current].m_freespace_ptr = 
        (_TCHAR*)(calloc(1, required_size_in_chars * sizeof(_TCHAR)));
    }
    if ((m_segments[m_current].m_freespace_ptr + tlen) >= 
      (m_segments[m_current].m_baseptr + m_segments[m_current].m_size_in_chars)
    ) {
      m_current++;
      uint32_t required_size_in_chars = DEFAULT_STRBLK_SIZE > (tlen) ? DEFAULT_STRBLK_SIZE : (tlen);  
      m_segments[m_current].m_size_in_chars = required_size_in_chars;
      m_segments[m_current].m_baseptr = 
        m_segments[m_current].m_freespace_ptr = 
        (_TCHAR*)(calloc(1, required_size_in_chars*sizeof(_TCHAR)));    
    }  
    _TCHAR* rv = m_segments[m_current].m_freespace_ptr;
    m_segments[m_current].m_freespace_ptr += tlen;
    return rv;
  }
  
  _TCHAR* append(const _TCHAR *s) {
    assert(s);
    _TCHAR* p = this->alloc(strlen(s));
    assert(p);
    strcpy(p, s); 
    return p;
  }

  ~STRBLK() {
    if (m_segments) {
      for (int i = 0; i < MAX_STRBLK_SEGMENTS; i++) {
        if (m_segments[i].m_baseptr) {
          free(m_segments[i].m_baseptr);
        }
      }
      free(m_segments);
    }
  }
};


#if defined(UNIT_TEST)
static void STRBLK_unit_test0() {
  static const _TCHAR* tab[] = {
    _T("a"),
    _T("========== Build: 1 succeeded, 0 failed, 0 up-to-date, 0 skipped =========="),
    _T("Isolatie: Dakisolatie en muurisolatie"),
    _T("0123"),
    _T("%^%^#@%^!@%^#$%^!@r$%^!@r#$!@#$$%%^#%^%^^%$^%@#%"),
    _T("")
  };
  _TCHAR** ptrtab = new _TCHAR*[32768];
  STRBLK blk;  
  for (int i = 0; i < 32768; i++) {
    ptrtab[i] = blk.append(tab[i % 6]);
  }
  for (int i = 0; i < 32768; i++) {
    assert(strcmp(ptrtab[i],tab[i % 6]) == 0);
  }
}
static void STRBLK_unit_test() {
  printf(_T("STRBLK TEST...\n"));
  STRBLK_unit_test0();
}
#endif


// static void* strblk_keep_str(const _TCHAR *s) {
//   assert(s);
//   assert(strlen(s));
//   void* p = strblk_alloc_str(strlen(s));
//   strcpy(p ,s);
//   return strdup(s);
// }

