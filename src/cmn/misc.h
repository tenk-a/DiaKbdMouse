/**
 *  @file   misc.h
 *  @brief  misc.
 *  @author Masashi Kitamura (tenka@6809.net)
 *  @license Boost Software Lisence Version 1.0
 */

#ifndef MISC_H_INCLUDE__
#define MISC_H_INCLUDE__

#include <stddef.h>
#include <string.h>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) || defined(_DOS) || defined(_MSC_VER)
#define FILE_DIR_SEP        '/'     // '\\'
#define FILE_IS_DIR_SEP(c)  ((c) == '/' || (c) == '\\')
#else
#define FILE_DIR_SEP        '/'
#define FILE_IS_DIR_SEP(c)  ((c) == '/')
#endif

static inline int fname_isDirSep(char c) { return FILE_IS_DIR_SEP(c); }

//void      str_replace(char str[], char old_c, char new_c);
//char*     strdupAddCapa(char const* str, size_t add_n);
//char*     fname_appendDup(char const* dir, char const* fname);

char*       fname_baseName(char const* p);

#if 0
char const* fname_ext( char const* p);
bool        fname_removeDirSep(char path[]);
char*       fname_addDirSep(char path[], size_t size);
char*       fname_backslashToSlash(char filePath[]);

int         fname_startsWith(char const* a, char const* prefix);
int         fname_isAbsolutePath(char const* fname);

size_t      file_load( char const* fname, void* dst, size_t bytes, size_t max_bytes);
void*       file_loadMalloc(char const* fname, size_t* pSize, size_t max_size);
#endif

size_t      file_size( char const* fpath);
int         file_exist(char const* fpath);
int         file_recursive_mkdir(char const* fpath, int pmode);

#ifdef __cplusplus
}
#endif

//  -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

#ifdef __cplusplus

#if 0
static inline std::string fpath_getDir(char const* s) { return std::string(s, fname_baseName(s) - s); }
std::string fpath_addDirSep(char const* dir);
std::string fpath_delLastDirSep(char const* dir);
static inline std::string fpath_getDirAddSep(char const* s) { return fpath_addDirSep(fpath_getDir(s)); }
static inline std::string fpath_getDirDelSep(char const* s) { return fpath_delLastDirSep(fpath_getDir(s)); }
#endif
static inline std::string fpath_getDir(std::string const& str) {
    char const* s = str.c_str();
    return std::string(s, fname_baseName(s) - s);
}
static inline std::string fpath_addDirSep(std::string dir) {
 #ifdef __WATCOMC__
    if (!dir.empty() && !fname_isDirSep(dir[dir.size()-1])) return dir + FILE_DIR_SEP;
 #else
    if (!dir.empty() && !fname_isDirSep(dir.back())) return dir + FILE_DIR_SEP;
 #endif
    return dir;
}
static inline std::string fpath_delLastDirSep(std::string dir) {
 #ifdef __WATCOMC__
    if (!dir.empty() && fname_isDirSep(dir[dir.size()-1])) dir.resize(dir.size()-1);
 #else
    if (!dir.empty() && fname_isDirSep(dir.back())) dir.pop_back();
 #endif
    return dir;
}
static inline std::string fpath_getDirAddSep(std::string s) { return fpath_addDirSep(fpath_getDir(s)); }
static inline std::string fpath_getDirDelSep(std::string s) { return fpath_delLastDirSep(fpath_getDir(s)); }

//  -   -   -   -   -

std::wstring utf8ToWcs(char const* s);
static inline std::wstring utf8ToWcs(std::string const& s) { return utf8ToWcs(s.c_str()); }
std::string  wcsToUtf8(wchar_t const* s);
static inline std::string wcsToUtf8(std::wstring const& s) { return wcsToUtf8(s.c_str()); }
std::string  wcsToSys(wchar_t const* src);
static inline std::string wcsToSys(std::wstring const& s) { return wcsToSys(s.c_str()); }

void LogPrintfInit(std::string logpath);
void LogPrintf(char const* fmt, ...);
void LogPrintf(wchar_t const* fmt, ...);
void LogPuts(char const* s);
void LogPuts(wchar_t const* ws);


std::wstring fpath_getLocalAppDataW();
std::string  fpath_getLocalAppDataA();

void showMessageDialog(wchar_t const* message);

#endif

//  -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

#endif  // MISC_H
