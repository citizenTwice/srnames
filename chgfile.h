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

 Changelog class.

 */
#pragma once

//#if defined(_MSC_VER) 
//// #include <tchar.h>
//#if defined(_UNICODE)
//#define tmpnam _wtmpnam
//#define fopen _wfopen
//#define _strdup wcsdup
//#define fprintf fwprintf
//#endif
//#endif

#if defined(_WIN32) 
#define fun_strdup _strdup
#else
#define fun_strdup strdup
#endif
struct CHANGES_FILE {
private:
  DYNSTR      m_tmpold, m_tmpnew, m_tmpwork;
public:
  const _TCHAR* m_path;
  FILE* m_fp;
  bool  m_error;
  uint32_t m_ord;

  CHANGES_FILE(const _TCHAR* _path = NULL)
    : m_path(_path), m_fp(NULL), m_error(true), m_ord(1) {
    if (_path == NULL) {
      const _TCHAR* t = ::tmpnam(NULL);
      if (t) {
        // shd point to a static buffer, but just in case..
        m_path = fun_strdup(t);
      } else {
        m_error = true;
        return;
      }
    } else {
      m_path = fun_strdup(_path);
    }
    FILE* tf = fopen(m_path, _T("w"));
    if (tf != NULL) {
      m_fp = tf;
      m_error = false;
      fprintf(m_fp, _T("# list of changed names. Format is: <number><TAB><oldname><TAB><newname>\n"));
      fprintf(m_fp, _T("# Undo of renames is best done in reverse order (of <number>).\n"));
    }
  }

  ~CHANGES_FILE() {
    if (m_path) {
      free((void*)(m_path));
    }
    if (m_fp && !m_error) {
      fflush(m_fp);
      fclose(m_fp);
    }
  }

  void add(const _TCHAR* _old, const _TCHAR* _new) {
    if (m_fp && !m_error) {
#if defined(_WIN32)
      m_tmpold.set(_old);
      m_tmpold.replace_all(_T("^"), _T("^^"));
      m_tmpnew.set(_new);
      m_tmpnew.replace_all(_T("^"), _T("^^"));
      fprintf(m_fp, _T("%d\t\"%s\"\t\"%s\"\n"), m_ord, m_tmpold.get(), m_tmpnew.get());
#elif defined(__DOS__)
      fprintf(m_fp, _T("%d\t%s\t%s\n"), m_ord, _old, _new);
#else
      fprintf(m_fp, _T("%d\t'%s'\t'%s'\n"), m_ord, _old, _new);
#endif
      m_ord++;
    }
  }
};