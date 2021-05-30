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

 Bare-bones dynamic string class.

 */

#pragma once

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <stdio.h>
#include <wchar.h>

#if defined(_WITH_REGEX)
#include <regex>
#endif

// #ifdef _WITH_WCHAR
// #include <wchar.h>
// #define _TCHAR wchar_t
// #else
// #define _TCHAR _TCHAR
// #define strdup  strdup
// #define strstr  strstr
// #define strncpy strncpy
// #define strlen  strlen
// #define strcat  strcat
// #endif

#ifndef UINT32
#define UINT32 uint32_t
#endif

typedef  _TCHAR*(*strfunptr)(const _TCHAR*, const _TCHAR*) ;
typedef  volatile strfunptr *strfunptrptr;

// to disambiguate overload on getting function ptr
static _TCHAR* strstrfunction(const _TCHAR*s1, const _TCHAR*s2) {
  return (_TCHAR*)::strstr(s1,s2);
}


#if __DOS__  || __APPLE__ || __linux__ || __unix__
  static _TCHAR* dos_strcasestr(const _TCHAR* s1, const _TCHAR* s2) {
    assert(s1); assert(s2);
    for (int i = 0; i < strlen(s1); i++) {
      if (strncasecmp(s1 + i, s2, strlen(s2)) == 0) {
        return (_TCHAR*)(s1 + i);
      }
    }
    return NULL;
  }
  #define the_strcasestr dos_strcasestr
#elif _WIN32
  #include <shlwapi.h>
  #pragma comment(lib, "shlwapi.lib")
  static _TCHAR* win_strcasestr(const _TCHAR* s1, const _TCHAR *s2) {
    return (_TCHAR*)(StrStrI(s1,s2));
  }
  #define the_strcasestr win_strcasestr  
#endif

struct DYNSTR {

private:
#if !defined(__DOS__)
  DYNSTR(const DYNSTR& other) = delete;
  DYNSTR& operator=(const DYNSTR& other) = delete;
#endif
public:
  _TCHAR  *buf;
  UINT32 bufsize_in_chars;
  static const UINT32 ALLOC_PADDING = 128;
  
  _TCHAR* alloc_chars(UINT32 sz) {
   _TCHAR* rv = (_TCHAR*)calloc(sz, sizeof(_TCHAR));
   assert(rv);
   //fflush(stdout);
   return rv;
  }

  void free_chars(void *b) {
//    fflush(stdout);
    ::free(b);
  }
  
  DYNSTR() {
    buf = 0;
    bufsize_in_chars = 0;
  }

  ~DYNSTR() {
    if (buf) {
      this->freebuf();
    }
  }

  void freebuf() {
    if (buf) {
      this->free_chars(buf);
      buf = NULL;
    }
  }

  void set(const _TCHAR* s) {
    assert(s);
    assert(buf != s);
    if ( buf && ((strlen(s) + 1) <= bufsize_in_chars)) {
      strcpy(buf, s);
    } else {
      if (buf) {
        this->freebuf();
      }
      assert(s);
      bufsize_in_chars = strlen(s) + 1 + ALLOC_PADDING;
      buf = this->alloc_chars(bufsize_in_chars);
      strcpy(buf, s);
     }
  }

  void swapbuf(_TCHAR* newbuf, UINT32 newsize) {
    if (buf) { this->freebuf();  }
    assert(newbuf);
    assert(newsize);
    bufsize_in_chars = newsize;
    buf = newbuf;
  }

  DYNSTR(const _TCHAR* s) {
    buf = 0;
    bufsize_in_chars = 0;
    this->set(s);
  }

  DYNSTR(UINT32 capacity) {
    buf = this->alloc_chars(capacity);
    bufsize_in_chars = capacity;
  }  
  
  _TCHAR* get() {
    return buf;
  }
  
  _TCHAR* append(const _TCHAR* s) {
    if (!buf) {
      set(s);
    } else {
      if ((strlen(buf) + strlen(s) +1) < bufsize_in_chars) {
        strcat(buf, s);
      } else {
        UINT32 newsize = (strlen(buf) + strlen(s) +1) + ALLOC_PADDING;
        _TCHAR* newbuf = this->alloc_chars(newsize);
        strcpy(newbuf, buf);
        strcat(newbuf, s);
        swapbuf(newbuf, newsize);       
      }
    }
    return buf;
  }
  
  _TCHAR char_at(UINT32 offs) {
    if (!buf) {
      return 0;
    }
    if ((((signed int)offs) >= 0) && (offs < strlen(buf))) {
      return buf[offs];
    } else {
      return 0;
    }
  }

  void print() {
    if (buf) {
     #ifdef _WITH_WCHAR
      wprintf(L"%s", buf);
     #else 
      printf(_T("%s"), buf);
     #endif
    }
  }

  void println() {
    if (buf) {
     #ifdef _WITH_WCHAR
      wprintf(L"%s\n", buf);
     #else 
      printf(_T("%s\n"), buf);
     #endif
    }
  }

  bool contains(const _TCHAR* s, bool nocase = false) {
    assert(buf);
    if (buf) {
      if (nocase) {
        return (the_strcasestr(buf, s) != NULL);
      } else {
        return strstr(buf, s) != NULL;
      }
    }
    return false;
  }

  bool equals(const _TCHAR* s) {
    assert(buf);
    if (buf) {
      return strcmp(buf, s) == 0;
    }
    return false;
  }

#if defined(_WITH_REGEX)
  bool replace_all_regex(const _TCHAR* search_s, const _TCHAR* repl_s, bool nocase = false) {
    assert(search_s);
    assert(strlen(search_s));
    assert(repl_s);
    assert(buf);
    std::regex re{ search_s };
//    wprintf(L"BEFORE %s\n", buf);
    std::string rs = std::regex_replace(std::string{ buf }, re, std::string{ repl_s });
//    wprintf(L"AFTER  %s\n", rs.c_str());
    bool rv = strcmp(rs.c_str(), buf) != 0;
    if (rv) {
      set(rs.c_str());
    }
//    wprintf(L"out    %s\n", buf);
    return rv;
  }
#endif

  bool replace_all_kw(const _TCHAR* search_s, const _TCHAR* repl_s, _TCHAR escape, bool nocase = false) {   
    assert(search_s);
    assert(strlen(search_s));
    assert(repl_s);
    assert(buf);
    strfunptr funfun = NULL;
    if (nocase) {
      funfun = the_strcasestr;
    } else {
      funfun = strstrfunction;
    }
    _TCHAR* tmpbuf = this->alloc_chars(strlen(buf) * (strlen(repl_s)+1) + 1);               
    UINT32 offs = 0;   
    const _TCHAR* foundp = NULL;
    bool changes = false;
    while ( offs < strlen(buf) && ((foundp = (funfun)(buf + offs, search_s)) != NULL)  ) {
      strncpy(tmpbuf + strlen(tmpbuf), buf + offs, foundp - (buf + offs));
      if (escape && char_at((foundp - buf) - 1) == escape) {
        strncpy(tmpbuf + strlen(tmpbuf), foundp, 1);
        offs = (foundp - buf) + 1;
        continue;
      }
      strcat(tmpbuf, repl_s);
      changes = true;
      offs = (foundp - buf) + strlen(search_s);
    }
    strcat(tmpbuf, buf + offs);
    this->set(tmpbuf);
    this->free_chars(tmpbuf);
    return changes;
  }
  
  bool replace_all(const _TCHAR* search_s, const _TCHAR* repl_s, bool nocase = false) {   
    return replace_all_kw(search_s, repl_s, 0, nocase);
  }

};

#if defined(UNIT_TEST)
static void DYNSTR_unit_test3() {
#ifdef _WITH_REGEX
  {
    DYNSTR bla(_T("--some=/thing/else"));
    bla.replace_all_regex(_T("--some=(.*)"), _T("$path://$1; [WAS: $&]"));
    assert(bla.equals(_T("$path:///thing/else; [WAS: --some=/thing/else]")));
  }
  {
    DYNSTR bla1(_T("FILE_NUM22014_TEST.TXT"));
    DYNSTR bla2(_T("invoice_NUM00947_backup.doc"));
    DYNSTR bla3(_T("ORDRSP_NUM55632.sent"));
    bla1.replace_all_regex(_T("NUM(\\d\\d\\d\\d\\d)"), _T("NR.0$1"));
    bla2.replace_all_regex(_T("NUM(\\d\\d\\d\\d\\d)"), _T("NR.0$1"));
    bla3.replace_all_regex(_T("NUM(\\d\\d\\d\\d\\d)"), _T("NR.0$1"));
    assert(bla1.equals(_T("FILE_NR.022014_TEST.TXT")));
    assert(bla2.equals(_T("invoice_NR.000947_backup.doc")));
    assert(bla3.equals(_T("ORDRSP_NR.055632.sent")));  }
#endif
}
static void DYNSTR_unit_test1() {
  DYNSTR bla(_T("A STRING WITH STUFF IN IT"));
  assert(bla.contains(_T("STUFF"), false));
  assert(!bla.contains(_T("stuff"), false));
  assert(bla.contains(_T("STUFF"), true));
  assert(bla.contains(_T("stuff"), true));
}
static void DYNSTR_unit_test2() {
  DYNSTR bla(_T("B"));
  assert(!bla.replace_all(_T("z"), _T("Z"), false));
  assert(!bla.replace_all(_T("z"), _T("Z"), true));
  assert(bla.replace_all(_T("b"), _T("Z"), true));
  assert(bla.equals(_T("Z")));
  assert(strlen(bla.get()) == 1);
  assert(!bla.replace_all(_T("z"), _T("aaa"), false));
  assert(bla.replace_all(_T("Z"), _T("aaa"), true));
  assert(bla.equals(_T("aaa")));
  assert(strlen(bla.get()) == 3);
  assert(!bla.replace_all(_T("A"), _T(""), false));
  assert(strlen(bla.get()) == 3);
  assert(bla.replace_all(_T("a"), _T(""), false));
  assert(strlen(bla.get()) == 0);
  bla.append(_T("1"));
  assert(strlen(bla.get()) == 1);
}
static void DYNSTR_unit_test2b() {
  for (int i = 0; i < 1024; i++) {
    DYNSTR* bla = new DYNSTR(_T("._pktgen_bench_xm_v_vow(o)_#dirw(_vow(i)_#dir)_pktgent_m_v_vow(o)_#dirw(_vow(o)_#dir)_pktgende_net_v_vow(o)_#dirw(_vow(i)_#dir)_pktgenf_rece_v_vow(o)_#dirw(_vow(i)_#dir)_pktgenve.sh"));
    bla->replace_all_kw(_T("#dir"), _T("pktgen"), _T('#'), false);
    assert(bla->equals(_T("._pktgen_bench_xm_v_vow(o)_pktgenw(_vow(i)_pktgen)_pktgent_m_v_vow(o)_pktgenw(_vow(o)_pktgen)_pktgende_net_v_vow(o)_pktgenw(_vow(i)_pktgen)_pktgenf_rece_v_vow(o)_pktgenw(_vow(i)_pktgen)_pktgenve.sh")));
  }

}

static void DYNSTR_unit_test0() {
  {
    DYNSTR bla(_T(""));
    assert(bla.equals(_T("")));
    assert(strlen(bla.get()) == 0);
    assert(!bla.equals(_T(" ")));
    bla.append(_T(" "));
    assert(strlen(bla.get()) == 1);
    assert(bla.equals(_T(" ")));
    bla.append(_T("0"));
    assert(bla.equals(_T(" 0")));
    assert(strlen(bla.get()) == 2);
    bla.set(_T("1"));
    assert(strlen(bla.get()) == 1);
    assert(bla.equals(_T("1")));
  }
  {
    DYNSTR a;
    _TCHAR* arr = new _TCHAR[65537];
    for (int i = 0; i < 65537; i++) {
      arr[i] = _TCHAR(i | 1);
    }
    arr[65536] = 0;
    a.set(arr);
    assert(strlen(a.get()) == 65536);
    assert((a.equals(arr)));
    a.append(arr);
    assert(strlen(a.get()) == 131072);
    a.append(_T(" "));
    assert(strlen(a.get()) == 131073);
  }
}
static void DYNSTR_unit_test() {
  printf(_T("DYNSTR TEST...\n"));
  DYNSTR_unit_test0();
  DYNSTR_unit_test1();
  DYNSTR_unit_test2();
  DYNSTR_unit_test2b();
  DYNSTR_unit_test3();
}
#endif
