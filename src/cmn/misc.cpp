/**
 *  @file   misc.cpp
 *  @brief  misc.
 *  @author Masashi Kitamura (tenka@6809.net)
 *  @license Boost Software Lisence Version 1.0
 */

#include "misc.h"
#include "CriticalSection.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <string>
#include <windows.h>

#if defined(_WIN32) || defined(_DOS)
 #include <io.h>
 #include <direct.h>
 #if defined(_WIN32)
  #include <windows.h>
  #if defined(_MBCS)
   #define IS_LEADBYTE(c) IsDBCSLeadByte((unsigned char)(c))
  #endif
 #else
  #define IS_LEADBYTE(c) ( ((unsigned char)(c) >= 0x81U && (unsigned char)(c) <= 0x9FU) \
                         ||((unsigned char)(c) >= 0xE0U && (unsigned char)(c) <= 0xFCU) )
 #endif
#else
 #include <unistd.h>
#endif

using namespace std;

#if 0
void str_replace(char str[], char old_c, char new_c)
{
    while (*str) {
        if (*str == old_c)
            *str = new_c;
        ++str;
    }
}


char* strdupAddCapa(char const* str, size_t add_n)
{
    size_t l = strlen(str);
    char*  m = (char*)calloc(1, l + 1 + add_n);
    if (m) {
        memcpy(m, str, l + 1);
    }
    return m;
}
#endif


//  -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

char*   fname_baseName(char const* p)
{
    char const *adr = p;
    while (*p) {
     #if defined(_WIN32) || defined(MSDOS)
        unsigned c = *(unsigned char const*)p;
        ++p;
      #if defined(IS_LEADBYTE)
        if (IS_LEADBYTE(c) && *p) {
            ++p;
            continue;
        }
      #endif
        if (c == ':' || c == '/' || c == '\\')
            adr = p;
     #else
        char c = *p++;
        if (c == ':' || c == '/')
            adr = p;
     #endif
    }
    return (char*)adr;
}

#if 0

char const* fname_ext(char const* fname)
{
    char const* p = fname_baseName(fname);
    char const* e = strrchr(p, '.');
    return e ? e : "";
}


int fname_startsWith(char const* a, char const* prefix)
{
 #if defined(_WIN32) || defined(MSDOS)
    return _strnicmp(a, prefix, strlen(prefix)) == 0;
 #else
    return strncmp(a, prefix, strlen(prefix)) == 0;
 #endif
}


/** win/dos なら filePath中の \ を / に置換.
 */
char *fname_backslashToSlash(char filePath[])
{
  #if defined(_WIN32) || defined(_DOS)
    char *p = filePath;
    while (*p != '\0') {
      #if defined(IS_LEADBYTE)
        if (IS_LEADBYTE(*p) && p[1]) {
            p += 2;
        } else
      #endif
        if (*p == '\\') {
            *p = '/';
            ++p;
        } else {
            ++p;
        }
    }
 #endif
    return filePath;
}


int fname_isAbsolutePath(char const* s)
{
 #if defined(_WIN32) || defined(MSDOS)
    if (fname_isDirSep(*s))
        return 1;
    if (*s && s[1] == ':' && fname_isDirSep(s[2])
        && (*s >= 'A' && *s <= 'Z' || *s >= 'a' && *s <= 'z')
    ) {
        return 1;
    }
    return 0;
  #else
    return fname_isDirSep(*s);
  #endif
}


char*   fname_removeDirSep(char path[]) {
    char* p = fname_baseName(path);
    if (p > path && *p == 0 && FILE_IS_DIR_SEP(p[-1]))
        *--p = 0;
    return path;
}


char*   fname_addDirSep(char* path, size_t capa) {
    char* p = path;
    fname_removeDirSep(p);
    p += strlen(p);
    if (p < path + capa - 1) {
        *p++ = FILE_DIR_SEP;
        *p = 0;
    } else {
        path = NULL;
    }
    return path;
}

char*   fname_appendDup(char const* dir, char const* fname)
{
    if (dir && *dir && !fname_isAbsolutePath(fname)) {
        size_t dirlen = strlen(dir);
        size_t fnmlen = strlen(fname);
        size_t bufcap = dirlen + fnmlen + 4;
        char*  buf    = strdupAddCapa(dir, fnmlen + 4);
        if (buf) {
            fname_addDirSep(buf, bufcap);
            strcat(buf, fname);
        }
        return buf;
    } else {
        return strdupAddCapa(fname, 4);
    }
}
#endif


//  -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

//
size_t file_size(const char* fname)
{
 #if defined(_MSC_VER)
    struct _stat st;
    int   rc = _stat(fname, &st);
 #else
    struct stat st;
    int   rc = stat(fname, &st);
 #endif
    return (rc == 0) ? (size_t)st.st_size : (size_t)-1;
}

int file_exist(char const* fname)
{
 #if defined(_WIN32) || defined(_DOS)
    return access(fname, 0) == 0;
 #else
    struct stat st;
    return stat(fname, &st) == 0;
 #endif
}

#if 0
size_t file_load(char const* fname, void* dst, size_t bytes, size_t max_bytes)
{
    size_t rbytes;
    if (dst == NULL || bytes == 0)
        return 0;
    if (max_bytes && bytes > max_bytes)
        bytes = max_bytes;

    if (fname) {
     #if defined(_WIN32) || defined(_DOS)
        int fd = _open(fname, _O_RDONLY|_O_BINARY);
        if (fd == -1)
            return 0;
        rbytes = _read(fd, dst, bytes);
        _close(fd);
     #else
        int fd = open(fname, O_RDONLY);
        if (fd == -1)
            return 0;
        rbytes = read(fd, dst, bytes);
        close(fd);
     #endif
    } else {
        rbytes = read(0, dst, bytes);

    }
    return rbytes;
}


void* file_loadMalloc(char const* fname, size_t* pReadSize, size_t max_size)
{
    char*  m;
    size_t bytes, rbytes;
    if (pReadSize)
        *pReadSize = 0;
    bytes = file_size(fname);
    if (bytes == (size_t)(-1))
        return NULL;
    if (max_size && bytes >= max_size)
        bytes = max_size;
    m = (char*)malloc(bytes+1);
    if (m == NULL)
        return NULL;
    m[bytes] = 0;
    rbytes = file_load(fname, m, bytes, max_size);
    if (pReadSize)
        *pReadSize = rbytes;
    return m;
}
#endif


//  -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

#if defined(_WIN32) || defined(_DOS)
 #define FILE_MKDIR(fnm, pmode) _mkdir(fnm)
#else
 #define FILE_MKDIR(fnm, pmode) mkdir((fnm), (pmode))
#endif

static int recursive_mkdir_sub2(char* dir, int pmode)
{
    char const* e  = dir + strlen(dir);
    char*   s;
    (void)pmode;
    do {
        s = fname_baseName(dir);
        if (s <= dir)
            return -1;
        --s;
        *s = 0;
    } while (FILE_MKDIR(dir, pmode) != 0);
    do {
        *s  = FILE_DIR_SEP;
        s  += strlen(s);
    } while (FILE_MKDIR(dir, pmode) == 0 && s < e);
    return (s >= e) ? 0 : -1;
}

///Extension of mkdir, making directories on the way
int file_recursive_mkdir(char const* dir, int pmode)
{
    char*  buf;
 #if defined(_MSC_VER)
    struct _stat st;
    int    rc  = _stat(dir, &st);
    int    mod = (rc == 0) ? st.st_mode : -1;
    if (mod != -1)
        return (mod & _S_IFDIR) ? 0 : -1;
 #else
    struct stat st;
    int    rc  = stat(dir, &st);
    int    mod = (rc == 0) ? st.st_mode : -1;
    if (mod != -1)
        return (mod & S_IFDIR) ? 0 : -1;
 #endif
    if (FILE_MKDIR(dir, pmode) == 0)
        return 0;   // ok.

    buf = strdup(dir);
    if (!buf)
        return -1;
    rc = recursive_mkdir_sub2(buf, pmode);
    free(buf);
    return rc;
}


//  -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

#if 0

std::string fpath_addDirSep(char const* dir)
{
    if (dir && *dir && !fname_isDirSep(dir[strlen(dir)-1]))
        return std::string(dir) + FILE_DIR_SEP;
    return dir;
}

std::string fpath_delLastDirSep(char const* dir)
{
    if (dir && *dir) {
        std::size_t len = strlen(dir);
        if (fname_isDirSep(dir[len-1]))
            return std::string(dir, len-1);
    }
    return dir;
}

#endif


//  -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -


std::wstring utf8ToWcs(char const* src)
{
    if (src) {
        int len  = int(strlen(src));
        int size = MultiByteToWideChar(CP_UTF8, 0, src, len, 0, 0);
        if (size > 0) {
            std::wstring result(std::size_t(size), L'\0');
            if (MultiByteToWideChar(CP_UTF8, 0, src, len, &result[0], size) > 0)
                return result;
        }
    }
    return std::wstring();
}

std::string wcsToUtf8(wchar_t const* src)
{
    if (src) {
        int len  = int(wcslen(src));
        int size = WideCharToMultiByte(CP_UTF8, 0, src, len, 0, 0, 0, 0);
        if (size > 0) {
            std::string result(std::size_t(size), '\0');
            if (WideCharToMultiByte(CP_UTF8, 0, src, len, &result[0], size, 0, 0) > 0)
                return result;
        }
    }
    return std::string();
}

std::string wcsToSys(wchar_t const* src)
{
    if (src) {
        int len  = int(wcslen(src));
        int size = WideCharToMultiByte(0, 0, src, len, 0, 0, 0, 0);
        if (size > 0) {
            std::string result(std::size_t(size), '\0');
            if (WideCharToMultiByte(0, 0, src, len, &result[0], size, 0, 0) > 0)
                return result;
        }
    }
    return std::string();
}

void showMessageDialog(wchar_t const* message)
{
    ::MessageBoxW(
        NULL,
        message ? message : L"",
        L"DiaKbdMouse",
        MB_OK | MB_ICONERROR | MB_TASKMODAL
    );
}

std::wstring fpath_getLocalAppDataW()
{
    DWORD size = ::GetEnvironmentVariableW(L"LOCALAPPDATA", 0, 0);
    if (0 < size && size < MAX_PATH) {
        if (size < MAX_PATH) {
            wchar_t     buf[MAX_PATH+1];
            DWORD len = ::GetEnvironmentVariableW(L"LOCALAPPDATA", &buf[0], size);
            if (0 < len && len < size)
                return std::wstring(buf, size);
        }
    }
    return std::wstring();
}


std::string fpath_getLocalAppDataA()
{
    return wcsToUtf8(fpath_getLocalAppDataW());
}



//  -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -
static CCriticalSection s_criticalSection;
static FILE*            s_logFp = NULL;

void LogPrintfInit(std::string logpath)
{
    if (!logpath.empty()) {
        if (!file_exist(logpath.c_str())) {
            std::string logdir = fpath_getDirDelSep(logpath);
            file_recursive_mkdir(logdir.c_str(), 0744);
        }
        s_logFp = fopen(logpath.c_str(), "ab");
    }
}

void LogPrintf(char const* fmt, ...)
{
    using namespace std;
    if (!fmt)
        return;

    enum { BUF_SZ = 8192 };
    char buf[BUF_SZ + 1];
    va_list args;
    va_start(args, fmt);
 #if defined(_MSC_VER)
    int length = _vsnprintf(buf, BUF_SZ, fmt, args);
 #else
    int length = vsnprintf(buf, BUF_SZ, fmt, args);
 #endif
    va_end(args);

    LogPuts(buf);
}

void LogPuts(char const* s)
{
    if (!s)
        return;
    CCriticalSectionLock lock(s_criticalSection);
    if (s_logFp) {
        using namespace std;
        fprintf(s_logFp, "%s", s);
        fflush(s_logFp);
    }
    OutputDebugStringA( wcsToSys(utf8ToWcs(s)).c_str() );
}

void LogPrintf(wchar_t const* fmt, ...)
{
    using namespace std;
    if (!fmt)
        return;

    enum { BUF_SZ = 4096 };
    wchar_t buf[BUF_SZ + 1];
    va_list args;
    va_start(args, fmt);
 #if defined(_MSC_VER)
    int length = _vsnwprintf(buf, BUF_SZ, fmt, args);
 #else
    int length = vsnwprintf(buf, BUF_SZ, fmt, args);
 #endif
    va_end(args);

    if (length < 0 || length >= BUF_SZ)
        length = BUF_SZ;
    buf[length] = 0;

    LogPuts(buf);
}

void LogPuts(wchar_t const* ws)
{
    if (!ws)
        return;
    CCriticalSectionLock lock(s_criticalSection);
    if (s_logFp) {
        using namespace std;
        fprintf(s_logFp, "%s", wcsToUtf8(ws).c_str() );
        fflush(s_logFp);
    }
    OutputDebugStringA( wcsToSys(ws).c_str() );
}
