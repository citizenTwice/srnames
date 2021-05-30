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

 Multi-platorm directory lister class.

 */

#pragma once

#if !defined(MAXNAME)
#define MAXNAME 4096
#endif

#ifdef __DOS__
#include <sys/types.h>
#include <direct.h>

static const _TCHAR PATH_SEP[] = { _T('\\'), 0 };

struct LISTER {
  _TCHAR name[MAXNAME];
  _TCHAR sepstr[2];
  const _TCHAR* path;
  bool error;
  bool is_dir;
  bool is_file;
  DIR* pdir;
  struct dirent* pde;

  LISTER(const _TCHAR* _path) :
    path(_path), is_dir(false), is_file(false), error(false) {
    assert(path);
    if ((pdir = opendir(_path)) == NULL) {
      error = true;
    }
  }

  ~LISTER() {
    // if (pdir) { closedir(pdir); }
  }

  bool next() {
    if (pdir && !error) {
      while ((pde = readdir(pdir)) != NULL) {
        strcpy(name, pde->d_name);
        if (strcmp(name, _T("..")) == 0) { continue; }
        if (strcmp(name, _T(".")) == 0) { continue; }
        is_file = false;
        is_dir = false;
        is_file = !(pde->d_attr & _A_SUBDIR);
        is_dir = (pde->d_attr & _A_SUBDIR);
        return true;
      }
    }
    return false;
  }
};
#endif  

#if _WIN32
#include <windows.h>

static const _TCHAR PATH_SEP[] = { _T('\\'), 0 };

struct LISTER {
  DYNSTR path;
  DYNSTR work_path;
  DYNSTR ff_mask;
  _TCHAR name[MAX_PATH];
  bool error;
  bool is_dir;
  bool is_file;
  bool first;
  bool end;
  WIN32_FIND_DATA wfd;
  HANDLE          hfind;

   LISTER(const DYNSTR& other) = delete;
   LISTER& operator=(const LISTER& other) = delete;

private:
  void update_fields() {
    if (!error) {
      strcpy(name, wfd.cFileName);
      is_dir = wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
      is_file = (!is_dir &&
        !(FILE_ATTRIBUTE_DEVICE & wfd.dwFileAttributes) &&
        !(FILE_ATTRIBUTE_REPARSE_POINT & wfd.dwFileAttributes)
        );
    }
  }

  bool skip() {
    if (!error) {
      return (FILE_ATTRIBUTE_REPARSE_POINT & wfd.dwFileAttributes);
    }
    return false;
  }

public:
  LISTER(const _TCHAR* _path) :
    error(true), is_dir(false), is_file(false), first(true), end(false) {
    assert(_path);
    path.set(_path);
    ff_mask.set(_path);
    ff_mask.append(_T("\\*.*"));
    hfind = INVALID_HANDLE_VALUE;
    hfind = FindFirstFile(ff_mask.get(), &wfd);
    error = (INVALID_HANDLE_VALUE == hfind);
    if (!error) {
      while (skip() && !error && !end) {
        fprintf(stderr, _T("WARNING: SKIPPING %s\n"), wfd.cFileName);
        if (!FindNextFile(hfind, &wfd)) {
          end   = GetLastError() == ERROR_NO_MORE_FILES;
          error = GetLastError() != ERROR_NO_MORE_FILES;
          break;
        }
      }
      if (!error && !end) {
        update_fields();
        first = true;
      }
    }
  }

  ~LISTER() {
    if (INVALID_HANDLE_VALUE != hfind) {
      FindClose(hfind);
    }
  }

  bool next() {
    while (!error && !end) {
      if (!first) {
        if (!FindNextFile(hfind, &wfd)) {
          end = GetLastError() == ERROR_NO_MORE_FILES;
          error = GetLastError() != ERROR_NO_MORE_FILES;
          return false;
        }
        update_fields();
      } else {
        first = false;
      }
      if (strcmp(name, _T("..")) == 0) { continue; }
      if (strcmp(name, _T(".")) == 0) { continue; }
      if (skip()) { 
        fprintf(stderr, _T("WARNING: SKIPPING %s\n"), wfd.cFileName);
        continue; 
      }
      return true;
    }
    return false;
  }

};

#endif

#if __linux__ || __unix__ || __APPLE__
// POSIX
#include <sys/types.h>
#include <dirent.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>

static const _TCHAR PATH_SEP[] = { _T('/'), 0 };

struct LISTER {
  DYNSTR path;
  DYNSTR work_path;
  _TCHAR name[PATH_MAX*2];
  bool error;
  bool is_dir;
  bool is_file;
  DIR* pdir;
  struct dirent* pde;
  struct stat    info;

  LISTER(const DYNSTR& other) = delete;
  LISTER& operator=(const LISTER& other) = delete;

  LISTER(const _TCHAR* _path) :
    error(true), is_dir(false), is_file(false)  {
    assert(_path);
    path.set(_path);

    pdir = opendir(path.get());
    if (pdir == NULL) {
      error = true;
    } else {
      error = false;
    }
  }

  ~LISTER() {
    if (pdir) { closedir(pdir); }
  }

  bool next() {
    if (!error && (pdir != nullptr)) {
      while ((pde = readdir(pdir)) != NULL) {
        strcpy(name, pde->d_name);
        if (strcmp(name, _T("..")) == 0) { continue; }
        if (strcmp(name, _T(".")) == 0) { continue; }
        is_file = false;
        is_dir = false;
        work_path.set(path.get());
        work_path.append(PATH_SEP);
        work_path.append(name);
        if ((lstat(work_path.get(), &info) == 0)) {
          // link? Skip. Do not try renaming links as it can mess things up pretty badly
          if (S_ISLNK(info.st_mode)) { continue; }
          is_file = info.st_mode & S_IFREG;
          is_dir = info.st_mode & S_IFDIR;
        } else {
          continue;
        }
        return true;
      }
    }
    return false;
  }

};
#endif
