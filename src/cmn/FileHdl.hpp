/**
 *  @file   FileHdl.h
 *  @brief  win-api をc標準ライブラリぽくしたラッパークラス.
 */
#ifndef FILEHDL_H
#define FILEHDL_H

#include <windows.h>
#include <cstdarg>
#include <cstdio>


class FileHdl {

public:
    typedef std::size_t         size_t;
    typedef __int64             int64_t;
    typedef unsigned __int64    uint64_t;
    typedef HANDLE              FH;         // windows版では win32 HANDLE に同じ、unix系はint
    typedef int64_t             fh_ofs_t;
    typedef uint64_t            fh_size_t;
    typedef uint64_t            fh_time_t;
    struct fh_stat_t {
        fh_ofs_t    st_size;
        fh_time_t   st_atime;
        fh_time_t   st_mtime;
        fh_time_t   st_ctime;
    };
    enum OpenMode { R,W,RP,WP };    // fh_open_mode_t と同じ.

public:
    FileHdl() : fh_(FH(-1)) {}
    explicit FileHdl(TCHAR const* nm, OpenMode md=R) : fh_(FH(-1)) { open(nm, md); }
    ~FileHdl() { close(); }

    FH          get_handle() const { return fh_;}

    static FH   stdIn()  { return (FH) ::GetStdHandle(0xfffffff6/*STD_INPUT_HANDLE*/); }
    static FH   stdOut() { return (FH) ::GetStdHandle(0xfffffff5/*STD_OUTPUT_HANDLE*/);}
    static FH   stdErr() { return (FH) ::GetStdHandle(0xfffffff4/*STD_ERROR_HANDLE*/); }

    bool        is_open() const { return fh_ != FH(-1); }

    bool        open(TCHAR const* fname, OpenMode md=R) {
                    assert(fh_ == FH(-1));
                    static const unsigned long s_gen[] = {
                        0x80000000/*GENERIC_READ*/,
                        0x40000000/*GENERIC_WRITE*/,
                        0xC0000000/*GENERIC_READ|GENERIC_WRITE*/,
                        0xC0000000/*GENERIC_READ|GENERIC_WRITE*/,
                    };
                    static const unsigned long  s_opt[] = {
                        3/*OPEN_EXISTING*/, 2/*CREATE_ALWAYS*/, 3/*OPEN_EXISTING*/, 2/*CREATE_ALWAYS*/,
                    };
                    assert(fname);
                    fh_ = (FH)::CreateFile(fname, s_gen[md&3], 1/*FILE_SHARE_READ*/, 0, s_opt[md&3], 0x80/*FILE_ATTRIBUTE_NORMAL*/, 0);
                    return fh_ != FH(-1);
                }

    bool        openStd(int type) {
                    assert(fh_ == FH(-1));
                    if      (type == 0) fh_ = stdIn();
                    else if (type == 1) fh_ = stdOut();
                    else if (type == 2) fh_ = stdErr();
                    return unsigned(type) <= 2;
                }

    void        close() {
                    if (fh_ != FH(-1)) {
                        if (fh_ != stdIn() && fh_ != stdOut() && fh_ != stdErr())
                            ::CloseHandle(fh_);
                        fh_ = FH(-1);
                    }
                }

    size_t      read(void* b, size_t sz) {
                    unsigned long r = 0;
                    assert(fh_!=FH(-1) && b!=0 && sz > 0);
                    if (! ::ReadFile(fh_,b,sz,&r,0)) r=0;
                    return r;
                }

    size_t      write(const void* b, size_t sz) {
                    unsigned long r=0;
                    assert(fh_!=FH(-1) && b!=0 && sz > 0);
                    if (! ::WriteFile(fh_,b,sz,&r,0))
                        r = 0;
                    return r;
                }

    int         flush() {
                    return ::FlushFileBuffers(fh_) ? 0 : -1;
                }

    fh_size_t   tell() {
                    return seek(0, 1/*FILE_CURRENT*/);
                }

    fh_size_t   seek(fh_size_t ofs, int mode) {
                    return ::SetFilePointerEx(fh_, *(LARGE_INTEGER*)&ofs, (LARGE_INTEGER*)&ofs, mode) ? ofs : 0;
                }

    fh_size_t   size() {
                    uint64_t l = 0;
                    return GetFileSizeEx(fh_, (union _LARGE_INTEGER*)&l) ? l : 0;
                }

                // 時間の取得. 成功したら 0, 失態したら負を返す.
    int         get_time(fh_time_t* pCreat, fh_time_t* pLastAcs, fh_time_t* pLastWrt) {
                    typedef struct _FILETIME* FT;
                    return ::GetFileTime(fh_, (FT)pCreat, (FT)pLastAcs, (FT)pLastWrt) ? 0 : -1;
                }

                // 時間の設定. 成功したら 0, 失態したら負を返す.
    int         set_time(fh_time_t creatTim, fh_time_t lastAcs, fh_time_t lastWrt) {
                    typedef const struct _FILETIME* CFT;
                    return ::SetFileTime(fh_, (CFT)&creatTim, (CFT)&lastAcs, (CFT)&lastWrt) ? 0 : -1;
                }

    int         stat(fh_stat_t* st) {
                    st->st_size = fh_ofs_t(size());
                    int      rc = get_time(&st->st_ctime, &st->st_atime, &st->st_mtime);
                    return (st->st_size >= 0 && rc >= 0) - 1;
                }

    template<class STRING>
    size_t      puts(const STRING& str) { return str.empty() ? 0 : write_text(&str[0], str.size()); }

    size_t      puts(const char* str) { return write_text(str, strlen(str)); }

    size_t      printf(const char* fmt, ...) {
                    va_list  a;
                    va_start(a,fmt);
                    size_t n = vprintf(fmt, a);
                    va_end(a);
                    return n;
                }

                /// fh_への vprintf 出力.
    size_t      vprintf(const char* fmt, va_list arg) {
                    typedef char    Char;
                    enum { BUF_SZ = 0x4000 };
                    Char    buf[ BUF_SZ + 1 ];
                    Char    *dp , *bp = buf;
                    int     n   , dsz = BUF_SZ;
                    assert(fh_ != FH(-1) && fmt);
                    do {
                        n = _vsnprintf((dp = bp), dsz, fmt, arg);
                    } while ((n < 0 || n >= dsz) && (bp = (Char*)memRealloc( dp!=buf?dp:0, (dsz=dsz*2>0x8000?dsz*2:0x8000)*sizeof(Char) ))!=0);
                    if (n >= dsz) n = dsz-1, dp[n] = 0;
                    if (n > 0   ) n = (int) write_text(dp, n);
                    if (dp != buf)
                        memFree(dp);
                    return n;
                }

                // 文字列出力. ※ winでは \n を\r\nにして出力. ※簡易版としてなのでencodeはos依存.
    size_t      write_text(const char* str, size_t strLen) {
                    typedef char    Char;
                    Char        c;
                    Char        dst[4096 + 1];
                    Char        *d  = dst, *de = d + sizeof(dst)-1;
                    const Char  *s  = str, *se = s + strLen;
                    size_t      sz  = 0;
                    while (s < se) {
                        *d++ = c = *s++;
                        if (c == '\r' && s < se && *s == '\n') {
                            *d++ = *s++;
                        } else if (c == '\n') {
                            d[-1] = '\r';
                            *d++  = c;
                        }
                        if (d >= de || s >= se) {
                            size_t l = d - dst;
                            size_t r = write(dst, l*sizeof(Char));
                            if (r != l * sizeof(Char))
                                return 0;
                            sz += l;
                            d = dst;
                        }
                    }
                    assert(d <= dst);
                    return sz;
                }

public:
    static void* memRealloc(void* p, size_t newBytes) {
                    if (p)  p = ::LocalReAlloc(p, newBytes, 0);
                    if (!p && newBytes) p = LocalAlloc(0/*LMEM_FIXED*/, newBytes);
                    return p;
                }
    static void memFree(void* p) {
                    LocalFree(p);
                }

private:
    FH          fh_;
};

#endif  // ARA_FILEHDL_HPP_INCLUDED
