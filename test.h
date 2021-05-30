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

 Some unit tests.

 */

#pragma once

#if defined(UNIT_TEST)
static void unit_test0() {
  {
    DYNSTR s1(_T("FILE#cnt.TXT"));
    replace_and_update_keywords(&s1);
    assert(s1.equals(_T("FILE1.TXT")));
  }
  {
    DYNSTR s1(_T("##FILE##cnt.TXT"));
    replace_and_update_keywords(&s1);
    assert(s1.equals(_T("#FILE#cnt.TXT")));
  }
  {
    DYNSTR s1(_T("#FILE##cnt.TXT"));
    replace_and_update_keywords(&s1);
    assert(s1.equals(_T("#FILE#cnt.TXT")));
  }
  {
    DYNSTR s2(_T("FILE#2cnt.TXT"));
    replace_and_update_keywords(&s2);
    assert(s2.equals(_T("FILE04.TXT")));
  }
  {
    DYNSTR s2(_T("FILE#3cnt.TXT"));
    replace_and_update_keywords(&s2);
    assert(s2.equals(_T("FILE005.TXT")));
  }
  {
    DYNSTR s2(_T("FILE#4cnt.TXT"));
    replace_and_update_keywords(&s2);
    assert(s2.equals(_T("FILE0006.TXT")));
  }
  {
    DYNSTR s2(_T("FILE#8cnt.TXT"));
    replace_and_update_keywords(&s2);
    assert(s2.equals(_T("FILE00000007.TXT")));
  }
#if __unix__ || __linux__ || __APPLE__
  {
    const _TCHAR* p = _T("/path/to/dirName");
    _TCHAR pbuf[TARGET_MAXP];
    get_parent_basename(pbuf, p);
    assert(strcmp(pbuf, _T("dirName")) == 0);
  }
  {
    const _TCHAR* p = _T("dir");
    _TCHAR pbuf[TARGET_MAXP];
    get_parent_basename(pbuf, p);
    assert(strcmp(pbuf, _T("dir")) == 0);
  }
  {
    const _TCHAR* p = _T("../..");
    _TCHAR pbuf[TARGET_MAXP];
    get_parent_basename(pbuf, p);
    assert(strcmp(pbuf, _T("PARENTDIR")) == 0);
  }
  {
    const _TCHAR* p = _T("..");
    _TCHAR pbuf[TARGET_MAXP];
    get_parent_basename(pbuf, p);
    assert(strcmp(pbuf, _T("PARENTDIR")) == 0);
  }
  {
    const _TCHAR* p = _T(".");
    _TCHAR pbuf[TARGET_MAXP];
    get_parent_basename(pbuf, p);
    assert(strcmp(pbuf, _T("PARENTDIR")) == 0);
  }
  {
    const _TCHAR* p = _T("/");
    _TCHAR pbuf[TARGET_MAXP];
    get_parent_basename(pbuf, p);
    assert(strcmp(pbuf, _T("PARENTDIR")) == 0);
  }
#endif
#if defined(_WIN32)
  {
    const _TCHAR* p = _T("C:\\PATH\\TO\\DIR");
    _TCHAR pbuf[TARGET_MAXP];
    get_parent_basename(pbuf, p);
    assert(strcmp(pbuf, _T("DIR")) == 0);
  }
  {
    const _TCHAR* p = _T("C:\\");
    _TCHAR pbuf[TARGET_MAXP];
    get_parent_basename(pbuf, p);
    assert(strcmp(pbuf, _T("PARENTDIR")) == 0);
  }
  {
    const _TCHAR* p = _T("\\SERVERNAME\\");
    _TCHAR pbuf[TARGET_MAXP];
    get_parent_basename(pbuf, p);
    assert(strcmp(pbuf, _T("SERVERNAME")) == 0);
  }
  {
    const _TCHAR* p = _T("C:");
    _TCHAR pbuf[TARGET_MAXP];
    get_parent_basename(pbuf, p);
    assert(strcmp(pbuf, _T("PARENTDIR")) == 0);
  }
  {
    const _TCHAR* p = _T("..\\..");
    _TCHAR pbuf[TARGET_MAXP];
    get_parent_basename(pbuf, p);
    assert(strcmp(pbuf, _T("PARENTDIR")) == 0);
  }
  {
    const _TCHAR* p = _T("..");
    _TCHAR pbuf[TARGET_MAXP];
    get_parent_basename(pbuf, p);
    assert(strcmp(pbuf, _T("PARENTDIR")) == 0);
  }
  {
    const _TCHAR* p = _T("");
    _TCHAR pbuf[TARGET_MAXP];
    get_parent_basename(pbuf, p);
    assert(strcmp(pbuf, _T("PARENTDIR")) == 0);
  }
  {
    const _TCHAR* p = _T("temp");
    _TCHAR pbuf[TARGET_MAXP];
    get_parent_basename(pbuf, p);
    assert(strcmp(pbuf, _T("temp")) == 0);
  }
  {
    const _TCHAR* p = _T("\\temp");
    _TCHAR pbuf[TARGET_MAXP];
    get_parent_basename(pbuf, p);
    assert(strcmp(pbuf, _T("temp")) == 0);
  }
  {
    const _TCHAR* p = _T("\\temp\\");
    _TCHAR pbuf[TARGET_MAXP];
    get_parent_basename(pbuf, p);
    assert(strcmp(pbuf, _T("temp")) == 0);
  }
  {
    const _TCHAR* p = _T("temp\\");
    _TCHAR pbuf[TARGET_MAXP];
    get_parent_basename(pbuf, p);
    assert(strcmp(pbuf, _T("temp")) == 0);
  }
#endif
#if defined(VAR_ALL_FEATURES)
  // dtm test
  update_dtm_var();
  {
    _TCHAR expected1[256];
    _TCHAR expected2[256];
    _TCHAR expected3[256];
    expected1[0] = 0;
    expected2[0] = 0;
    expected3[0] = 0;
    snprintf(expected1, 256, _T("FILE_%s_123"), g_globals.var_dtm);
    snprintf(expected2, 256, _T("%s_FILE.BIN"), g_globals.var_dtm);
    snprintf(expected3, 256, _T("DOC01.TXT.%s"), g_globals.var_dtm);
    DYNSTR s1(_T("FILE_#dtm_123"));
    DYNSTR s2(_T("#dtm_FILE.BIN"));
    DYNSTR s3(_T("DOC01.TXT.#dtm"));
    replace_and_update_keywords(&s1);
    replace_and_update_keywords(&s2);
    replace_and_update_keywords(&s3);
    assert(s1.equals(expected1));
    assert(s2.equals(expected2));
    assert(s3.equals(expected3));
  }
  {
    g_globals.var_parent_dir = _T("TESTDIR");
    DYNSTR s1(_T("##FILE_#dir_123#"));
    DYNSTR s2(_T("FILE_##dir_123##"));
    replace_and_update_keywords(&s1);
    replace_and_update_keywords(&s2);
    assert(s1.equals(_T("#FILE_TESTDIR_123#")));
    assert(s2.equals(_T("FILE_#dir_123#")));
  }
  // dir test
#endif
}

static void unit_test() {
  printf(_T("\nRUNNING UNIT TESTS\n"));
  printf(_T("==================\n"));
  DYNSTR_unit_test();
  DYNARRAY_unit_test();
  STRBLK_unit_test();
  printf(_T("SRNAMES TEST...\n"));
  unit_test0();
  printf(_T("==================\n"));
  printf(_T("TESTS PASSED\n"));
}
#include <time.h>
static void do_test() {
}

#endif
