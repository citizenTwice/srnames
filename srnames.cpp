/*

SRNAMES - coded by Luigi Galli LG@citizenTwice.nl

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


//#define UNIT_TEST
//#define _WITH_REGEX
//#define _UNICODE
//#define UNICODE

#if defined(_MSC_VER)

  #if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
  #define _CRT_SECURE_NO_WARNINGS
  #endif

#endif


#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>

#if defined(_WITH_REGEX) 
#include <regex>
#endif
#if defined(UNIT_TEST) 
#include <string>
#endif

#if defined(_MSC_VER) && defined(_DEBUG)
#include <conio.h>
#endif

#if defined(_WIN32)
#include <windows.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#endif

#if defined(_UNICODE) && defined(_MSC_VER)
#include <tchar.h>
//#define main _tmain
//#define char _TCHAR
// #define _WITH_WCHAR
#define strdup  wcsdup
#define _strdup  _wcsdup
#define strncpy wcsncpy
#define strcpy  wcscpy
#define strstr  wcsstr
#define strlen  wcslen
#define strcat  wcscat
#define strcmp  wcscmp
#define rename  _wrename
#define printf  wprintf
#define getc  getwc
#define fprintf fwprintf
#define snprintf _snwprintf
#define regex  wregex
#define string wstring
#define tmpnam _wtmpnam
#define fopen _wfopen
#define strftime wcsftime
//#define _strdup wcsdup
//#define fprintf fwprintf
#endif
#if (!defined(_MSC_VER)) || (!defined(UNICODE) && defined(_MSC_VER))
#define _T(x) (x)
#define _TCHAR char
#define _tmain main
#endif


#if defined(__DOS__)
#define MAX_STRBLK_SEGMENTS 4096
#define DEFAULT_STRBLK_SIZE 65536
#else
#define MAX_STRBLK_SEGMENTS 4096
#define DEFAULT_STRBLK_SIZE 1048576
#endif

#if !defined(TARGET_MAXP)
#if defined(__DOS__)
#define TARGET_MAXP 256
#elif defined(__linux__) || (__unix__) || (__APPLE__)
#define TARGET_MAXP PATH_MAX*2
#elif defined(_WIN32)
#define TARGET_MAXP MAX_PATH*2
#elif
#include <DONT_KNOW_HOW_TO_HANDLE_THIS_PLATFORM>
#endif
#endif

#if defined(__DOS__)
#define VAR_BASIC_FEATURES
#else
#define VAR_ALL_FEATURES
#endif

#include "dynstr.h"
#include "dynarray.h"
#include "strblk.h"
#include "chgfile.h"
#include "lister.h"

struct _g__globals {
  STRBLK all_names;
  bool recurse;
  bool dirs_too;
  bool whatif;
  bool nocase;
  bool quiet;
  bool no_confirm;
  bool use_regex;
  // vars for replacement expressions
  uint32_t    var_rencnt;
  const _TCHAR* var_parent_dir;
  const _TCHAR* var_dtm;
  const _TCHAR* search;
  const _TCHAR* repl;
  const _TCHAR* chglog_path;  
  CHANGES_FILE* changes_file;
  bool do_not_log_changes;
  _g__globals() {
    recurse = false;
    dirs_too = false;
    whatif = false;
    nocase = false;
    quiet = false;
    no_confirm = false;
    use_regex = false;
    search = NULL;
    repl = NULL;
    chglog_path = NULL;
    changes_file = NULL;
#if LOG_CHANGES_BY_DEFAULT
    do_not_log_changes = false;
#else
    do_not_log_changes = true;
#endif
    var_rencnt = 1;
    var_parent_dir = _T("UNDEFINED");
    var_dtm = NULL;
  }
  _TCHAR* add_name(const _TCHAR* s) {
    return all_names.append(s);
  }
} g_globals;

static const _TCHAR* get_opt_val(const _TCHAR* arg, const _TCHAR* opt) {
  const _TCHAR* p = NULL;
  if ((p = strstr(arg, opt))) {
    p = arg + strlen(opt);
    if (*p >= 0x20) {
      return p;
    } else {
      return NULL;
    }
  } else {
    return NULL;
  }
}

static const bool is_opt(const _TCHAR* arg, const _TCHAR* opt) {
  return strcmp(arg,opt) == 0;
}

static bool rename_file_or_dir(const _TCHAR* from, const _TCHAR* to) {
  return rename(from, to) == 0;
}

static void log_change(const _TCHAR* _old, const _TCHAR* _new) {
  if (!g_globals.do_not_log_changes && g_globals.changes_file) {
    g_globals.changes_file->add(_old, _new);
  }
}
static void update_dtm_var() {
  if (g_globals.var_dtm == NULL) {
    time_t _tmptime;
    time(&_tmptime);
    struct tm* _lt = localtime(&_tmptime);
    _TCHAR* buf = new _TCHAR[256];
    strftime(buf, 256, _T("%Y%m%d%H%M%S"), _lt);
    g_globals.var_dtm = buf;
  }
}

static void replace_and_update_keywords(DYNSTR* s) {
  if (
    s->contains(_T("#cnt" )) ||
    s->contains(_T("#2cnt")) ||
    s->contains(_T("#3cnt")) ||
    s->contains(_T("#4cnt")) ||
    s->contains(_T("#8cnt"))
   ) {
    static const int TMPSZ = 256;
    _TCHAR tmp[TMPSZ];
    snprintf(tmp, TMPSZ, _T("%d"), g_globals.var_rencnt);
    s->replace_all_kw(_T("#cnt"), tmp, _T('#'));
    snprintf(tmp, TMPSZ, _T("%02d"), g_globals.var_rencnt);
    s->replace_all_kw(_T("#2cnt"), tmp, _T('#'));
    snprintf(tmp, TMPSZ, _T("%03d"), g_globals.var_rencnt);
    s->replace_all_kw(_T("#3cnt"), tmp, _T('#'));
    snprintf(tmp, TMPSZ, _T("%04d"), g_globals.var_rencnt);
    s->replace_all_kw(_T("#4cnt"), tmp, _T('#'));
    snprintf(tmp, TMPSZ, _T("%08d"), g_globals.var_rencnt);
    s->replace_all_kw(_T("#8cnt"), tmp, _T('#'));
    g_globals.var_rencnt++;
  }
#if defined(VAR_ALL_FEATURES)
  if (g_globals.var_parent_dir != NULL) {
    s->replace_all_kw(_T("#dir"), g_globals.var_parent_dir, _T('#'));
  }
  if (s->contains(_T("#dtm"))) {
    update_dtm_var();
    s->replace_all_kw(_T("#dtm"), g_globals.var_dtm, _T('#'));
  }
#endif
  s->replace_all(_T("##"), _T("#"));
}

static void get_parent_basename(_TCHAR*buf, const _TCHAR* path) {
#if defined(_WIN32)
  if (!PathCanonicalize(buf, path)) {
    strcpy(buf, _T("PARENTDIR"));
  }
  PathStripPath(buf);
  if (buf[strlen(buf) - 1] == _T('\\')) {
    buf[strlen(buf) - 1] = _TCHAR(0);
  }
  if ( (strcmp(buf, _T(".")) == 0) ||
    (strcmp(buf, _T("..")) == 0) ||
    (strlen(buf) == 0) ||
    (PathGetDriveNumber(buf) > 0) || (PathIsUNC(buf))) {
    strcpy(buf, _T("PARENTDIR"));
  }
#elif !defined(__DOS__)
  _TCHAR tmp[TARGET_MAXP];
  strcpy(tmp, path);
  _TCHAR* b = basename(tmp);
  strcpy(buf, b);
  if ((strcmp(buf, _T(".")) == 0) ||
    (strcmp(buf, _T("..")) == 0) ||
    (buf[0] == _T('/')) ||
    (strlen(buf) == 0)
   ) {
    strcpy(buf, _T("PARENTDIR"));
  }
#endif
}

static bool do_dir(
  const _TCHAR* path
) {
  DYNARRAY<const _TCHAR*> all_dirs;
  DYNARRAY<const _TCHAR*> all_files;
  DYNSTR entry_path;
  DYNSTR temp_name;
  DYNSTR new_path;
  _TCHAR parent[TARGET_MAXP];
  // gather all names at specified path
  {
    LISTER lst(path);
    if (lst.error) {
      fprintf(stderr, _T("ERROR LISTING DIRECTORY %s\n"), path);
      return false;
    }
    while (lst.next())  {
      _TCHAR* tname = g_globals.add_name(lst.name);
      if (lst.is_dir) {
        all_dirs.append(tname);
      } else if (lst.is_file) {
        all_files.append(tname);
      }    
    }    
  }
#if defined(VAR_ALL_FEATURES)
  {
    // update extra vars for supported platforms
    get_parent_basename(parent, path);
    g_globals.var_parent_dir = parent;
  }
#endif
  for (uint32_t  i = 0; i < all_files.size(); i++) {
    entry_path.set(path);
    entry_path.append(PATH_SEP);
    entry_path.append(all_files[i]);
    temp_name.set(all_files[i]);
    bool chg = false;
    if (g_globals.use_regex) {
#if defined(_WITH_REGEX)
      chg = temp_name.replace_all_regex(g_globals.search, g_globals.repl, g_globals.nocase);
#else
      chg = temp_name.replace_all(g_globals.search, g_globals.repl, g_globals.nocase);
#endif
    } else {
      chg = temp_name.replace_all(g_globals.search, g_globals.repl, g_globals.nocase);
    }
    if (chg) {
      replace_and_update_keywords(&temp_name);
      new_path.set(path);
      new_path.append(PATH_SEP);
      new_path.append(temp_name.get());
      if (!g_globals.quiet) {
        if (!g_globals.whatif) {
          printf(_T("RENAMING   "));
        } else {
          printf(_T("WOULD REN. "));
        }
        printf(_T("FILE : %s\n"), entry_path.get());
        printf(_T("           TO   : %s\n"), new_path.get());
      }
      if (!g_globals.whatif) {
        if (!rename_file_or_dir(entry_path.get(), new_path.get())) {
          fprintf(stderr, _T("ERROR: renaming %s failed\n"), entry_path.get());
        } else {
          log_change(entry_path.get(), new_path.get());
        }
      }
    } 
  }
  for (uint32_t  i = 0; i < all_dirs.size(); i++) {
    entry_path.set(path);
    entry_path.append(PATH_SEP);
    entry_path.append(all_dirs[i]);
    if (g_globals.recurse) {
      do_dir(entry_path.get());
      #if defined(VAR_ALL_FEATURES)
      {
        // reset parent
        g_globals.var_parent_dir = parent;
      }
      #endif
      }
    temp_name.set(all_dirs[i]);
    if (g_globals.dirs_too) {
      bool chg = false;
      if (g_globals.use_regex) {
#if defined(_WITH_REGEX)
        chg = temp_name.replace_all_regex(g_globals.search, g_globals.repl, g_globals.nocase);
#else
        chg = temp_name.replace_all(g_globals.search, g_globals.repl, g_globals.nocase);
#endif
      } else {
        chg = temp_name.replace_all(g_globals.search, g_globals.repl, g_globals.nocase);
      }
      if (chg) {
        replace_and_update_keywords(&temp_name);
        new_path.set(path);
        new_path.append(PATH_SEP);
        new_path.append(temp_name.get());
        if (!g_globals.quiet) {
          if (!g_globals.whatif) {
            printf(_T("RENAMING  "));
          } else {
            printf(_T("WOULD REN."));
          }
          printf(_T(" DIR  : %s\n"), entry_path.get());
          printf(_T("           TO   : %s\n"), new_path.get());
        }
        if (!g_globals.whatif) {
          if (!rename_file_or_dir(entry_path.get(), new_path.get())) {
            fprintf(stderr, _T("ERROR: renaming %s failed\n"), entry_path.get());
          } else {
            log_change(entry_path.get(), new_path.get());
          }
        }
      }
    } 
  }
  return true;
}

static void show_help() {
  printf(_T("srnames - search and replace file/dir names.\n"));
  printf(_T("  Required:\n"));
  printf(_T("          --search=str1              String to search to for.\n"));
  printf(_T("          --repl=str2                Replacement string.\n"));
  printf(_T("  Options:\n"));
  printf(_T("      -i, --ignore-case              Ignore case in searches.\n"));
  printf(_T("      -d, --dirs-too                 Also rename directories (default is files-only.)\n"));
  printf(_T("      -a, --start-at=path            Start searching at given path.\n"));
  printf(_T("                                     Default is: current directory.\n"));
  printf(_T("      -r, --recurse                  Recursively include all directories.\n"));
#if defined(_WITH_REGEX)
  printf(_T("          --regex                    Interpret search string as regex.\n"));
  printf(_T("                                     Makes capture groups available in replacement string.\n"));
#endif
  printf(_T("          --what-if                  Preview results without making any changes.\n"));
  printf(_T("          --dry-run                  Same as --what-if.\n"));
  printf(_T("      -q, --quiet                    Do not print changes to stdout.\n"));
  printf(_T("      -y, --yes                      Skip initial confirmation prompt.\n"));
  printf(_T("          --chglog=filepath          Log changes (enables undo).\n"));
  printf(_T("                                     On by default (temp file) for some platforms.\n"));
  printf(_T("      -n, --no-chglog                Disable changes log.\n"));
  printf(_T("  Exit codes:                        0 = Success.\n"));
  printf(_T("                                     1 = Error.\n"));
#if defined(_WITH_REGEX)
  printf(_T("  Regex replacement variables:       $1..n = Capture group n. E.g. \'file_(\\d+)\'\n"));
  printf(_T("                                     $&    = The entire regex match\n"));
#endif
  printf(_T("  Repl. string variables:            (applied after regex, when that's available)\n"));
#if defined(VAR_ALL_FEATURES)
  printf(_T("                                     #dir  = Parent dir of current item.\n"));
  printf(_T("                                     #dtm  = YYYYMMDDhhmmss.\n"));
#endif
  printf(_T("                                     #cnt  = Rename counter.\n"));
  printf(_T("                                     #2cnt = ditto, zero-filled NN.\n"));
  printf(_T("                                     #3cnt = ditto, zero-filled NNN.\n"));
  printf(_T("                                     #4cnt = ditto, zero-filled NNNN.\n"));
  printf(_T("                                     #8cnt = ditto, zero-filled NNNNNNNN.\n"));
  printf(_T("                                     ##    = literal #.\n"));
  printf(_T("\n"));
}

#if defined(UNIT_TEST)
#include "test.h"
#endif

int _tmain(int ac, _TCHAR** av) {
//  tmp_test();
  #if defined(UNIT_TEST)
    do_test();
    unit_test();
    return 0;
  #endif
  int exit_code = 0;
  const _TCHAR* start_dir = _T(".");
  for (int i = 1; i < ac; i++) {
    const _TCHAR* argval = NULL;
    if (is_opt(av[i],_T("-r")) ||
      is_opt(av[i],_T("--recurse"))
    ) {
      g_globals.recurse = true;
    } else if (is_opt(av[i],_T("--dirs-too")) || 
      is_opt(av[i],_T("-d"))
    ) {
      g_globals.dirs_too = true;
    } else if (is_opt(av[i], _T("--quiet")) ||
      is_opt(av[i], _T("-q"))
      ) {
      g_globals.quiet = true;
  } else if (is_opt(av[i], _T("--yes")) ||
    is_opt(av[i], _T("-y"))
  ) {
    g_globals.no_confirm = true;
  } else if (is_opt(av[i], _T("--regex"))) {
      #if defined(_WITH_REGEX)
            g_globals.use_regex = true;
      #else
            fprintf(stderr, _T("WARNING: NO REGEX SUPPORT\n"));
      #endif
    } else if (is_opt(av[i], _T("--ignore-case")) ||
      is_opt(av[i], _T("-i"))
      ) {
      g_globals.nocase = true;
    } else if (is_opt(av[i], _T("--no-chglog")) ||
      is_opt(av[i], _T("-n"))
      ) {
      g_globals.do_not_log_changes = true;
    } else if (is_opt(av[i], _T("--quiet")) ||
      is_opt(av[i], _T("-q"))
      ) {
      g_globals.quiet = true;
    } else if (is_opt(av[i], _T("--what-if"))) {
      g_globals.whatif = true;
    } else if (is_opt(av[i], _T("--dry-run"))) {
      g_globals.whatif = true;
    } else if ((argval = get_opt_val(av[i], _T("--start-at=")))) {
      start_dir = argval;
    } else if ((argval = get_opt_val(av[i], _T("-a")))) {
      start_dir = argval;
    } else if ((argval = get_opt_val(av[i], _T("--search=")))) {
      g_globals.search = argval;
    } else if ((argval = get_opt_val(av[i], _T("--repl=")))) {
      g_globals.repl = argval;
    } else if ((argval = get_opt_val(av[i], _T("--replace=")))) {
      g_globals.repl = argval;
    } else if ((argval = get_opt_val(av[i], _T("--chglog=")))) {
      g_globals.do_not_log_changes = false;
      g_globals.chglog_path = argval;
    } else {
      fprintf(stderr, _T("WARNING: IGNORING ARG: %s\n"), av[i]);
    }
  }
  if (!g_globals.search || !g_globals.repl) {
    show_help();
    exit_code = 1;
    goto _done;
  }
  if (strlen(start_dir) >= TARGET_MAXP) {
    fprintf(stderr, _T("Be reasonable...\n"));
    exit_code = 1;
    goto _done;
  }
  if (!g_globals.do_not_log_changes) {
    g_globals.changes_file = new CHANGES_FILE(g_globals.chglog_path);
    if (g_globals.changes_file->m_error) {
      fprintf(stderr, _T("ERROR CREATING CHANGES LIST FILE %s\n"), g_globals.changes_file->m_path);
      exit_code = 1;
      goto _done;
    } else {
      printf(_T("WRITING CHANGES LOG TO %s\n"), g_globals.changes_file->m_path);
    }
  }
  if (!g_globals.no_confirm && !g_globals.whatif) {
    printf(_T("Settings:\n -> look for files "));
    if (g_globals.dirs_too) {
      printf(_T("and directories\n"));
    } else {
      printf(_T("\n"));
    }
    printf(_T(" -> with name containing <%s>\n"), g_globals.search);
    printf(_T(" -> replacing it with    <%s>\n"), g_globals.repl);
    printf(_T(" -> Starting at path     <%s>\n"), start_dir);
    if (g_globals.changes_file) {
      printf(_T(" -> Logging changes to   <%s>\n"), g_globals.changes_file->m_path);
    }
    printf(_T(" -> Flags: quiet[%d] | dir-recurse[%d] | ignore-case[%d]"), g_globals.quiet, g_globals.recurse, g_globals.nocase);
#if defined(_WITH_REGEX)
    printf(_T(" | regex[%d]\n"), g_globals.use_regex);
#else
    printf(_T("\n"));
#endif
    while (true) {
      printf(_T("\ny + [ENTER] to confirm | [CTRL-C] to abort | p + [ENTER] to preview changes\n"));
      _TCHAR c = getc(stdin);
      while (c != EOF && (c <= _T(' '))) {
        c = getc(stdin);
      }
      if (c == _T('p') || c == _T('P')) {
        g_globals.whatif = true;
        g_globals.quiet = false;
        break;
      } 
      if (c == _T('y') || c == _T('Y')) {
        break;
      }
    }
  }
  do_dir(start_dir);
_done:
#if defined(_MSC_VER) && defined(_DEBUG)
  fprintf(stderr, _T("PRESS ANY KEY TO CONTINUE\n"));
  fflush(stderr);
  _getch();
#endif
  return exit_code;
}

