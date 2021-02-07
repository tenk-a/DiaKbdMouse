/**
 *  @file   TextReaderBase.h
 *  @brief  バイト列をテキストとして 文字・行読み込みするための基底クラス
 *  @note
 *  - \r\n を \n 化する.
 *  - クラス継承して raw_close(), raw_read() (場合によっては raw_malloc, raw_free)
 *    を オーバーライドして実装のこと.
 */
#ifndef TEXTREADERBASE_H
#define TEXTREADERBASE_H

#include <cstddef>
#include <cassert>

// バイト列をテキストとして 文字・行読み込みするための基底クラス
class TextReaderBase {
    enum { INNER_BUF_SZ = 3 };  // 現状パディング利用のみ.
public:
    typedef std::size_t     size_t;
    TextReaderBase() { init(); }
    TextReaderBase(char* buf, unsigned sz) { init(); setbuf(buf,sz); }
    ~TextReaderBase() { close(); }

    void        close() {
                    raw_close();
                    if (alcFlag_)
                        raw_free(buf_);
                }

    size_t      read(void* readbuf, size_t size) {
                    return readEx(readbuf, size, false);
                }

    int         getc() {
                    char    buf[2];
                    return read(buf, 1) > 0 ? buf[0] : -1;
                }

    char*       gets(char* buf, size_t bufSize) {
                 #if 1
                    assert(buf && bufSize > 1);
                    std::size_t rdsz = readEx(buf, bufSize-1, true);
                    if (std::ptrdiff_t(rdsz) > 0) {
                        buf[rdsz] = 0;
                        return buf;
                    }
                    return NULL;
                 #else
                    char*   d  = buf;
                    assert(buf);
                    if (bufSize > 0) {
                        char*   de = d + bufSize - 1;
                        if (d < de) {
                            char    buf[2];
                            int c;
                            do {
                                if (read(buf, 1) <= 0)
                                    break;
                                c = buf[0];
                                *d++ = c;
                            } while (d < de && c != '\n' /*&& c != '\r'*/);
                        }
                        *d = 0;
                    }
                    return buf;
                 #endif
                }

    int         setbuf(char* buf, size_t size) {
                    int     rc = 0;
                    if (buf_) {
                        // flush();
                        if (alcFlag_)
                            raw_free(buf_);
                        buf_     = 0;
                        alcFlag_ = 0;
                    }
                    if (size >= 0x80000000) // バッファサイズはintの整数範囲.
                        goto ERR;
                    if (buf) {  // 呼び元が用意したバッファがあるとき
                        if (size < 2)   // 2未満なら内蔵のを使う.
                            goto ERR;
                    } else if (size <= INNER_BUF_SZ) {
                        buf         = innerBuf_;
                        size        = INNER_BUF_SZ;
                    } else {
                        size        = size;
                        buf         = (char*) raw_malloc( size );
                        if (buf) {
                            alcFlag_ = 1;
                        } else {    // allocateに失敗したら内蔵バッファを使う.
                  ERR:
                            buf     = innerBuf_;
                            size    = INNER_BUF_SZ;
                            rc      = -1;
                        }
                    }
                    buf_        = buf;
                    bufCapa_    = size;
                    bufCur_     = 0;
                    bufSize_    = 0;
                    return rc;
                }

private:
    size_t      readEx(void* readbuf, size_t size, bool crFlag) {
                    assert(readbuf != NULL);
                    assert(buf_ != NULL);
                    assert(bufCapa_ > 0);
                    assert(bufSize_ <= bufCapa_);
                    assert(bufCur_  <= bufSize_);

                    error_  = 0;

                    size_t  total       = 0;
                    char*   dst         = (char*)readbuf;
                    char*   buf         = buf_;
                    size_t  bufCapa     = bufCapa_;
                    while (size > 0) {
                        std::ptrdiff_t  r = bufSize_ - bufCur_;
                        assert(r >= 0);
                        if (r > 0) {    // バッファにデータがあればまずはそこから取得.
                            size_t l;
                            l = size;
                            if (l > size_t(r))
                                l = r;
                            if (crFlag == 0) {
                                std::memcpy(dst, buf+bufCur_, l);
                                //std::copy(dst, dst+l, buf+bufCur_);
                                dst         += l;
                            } else {
                                char* s  = buf+bufCur_;
                                char* se = s + l;
                                while (s < se) {
                                    int c  = *s++;
                                    *dst++ = c;
                                    if (c == '\n') {
                                        l    = s - (buf+bufCur_);
                                        size = l;
                                        break;
                                    }
                                }
                            }
                            bufCur_     += l;
                            total       += l;
                            size        -= l;
                            if (size == 0)
                                break;
                        }
                        bufCur_         = 0;
                        bufSize_        = 0;
                        if (eof_) { // 最後まで読み終わっていた場合は終了.
                            break;
                        }

                        // ファイルからバッファに読み込む.
                        if (restCR_) {  // 前回の最後がCRだった場合の調整.
                            restCR_ = 0;
                            buf[0]  = '\r';
                            r = raw_read(buf+1, bufCapa-1);
                            if (r >= 0)
                                ++r;
                        } else {
                            r = raw_read(buf, bufCapa);
                        }
                        if (r <= 0) {       // 読み込めなかった.
                            if (r < 0) {    // 負ならエラー発生.
                                error_  = 1;
                                total   = (size_t)-1;
                            }
                            break;
                        }
                        if (r < std::ptrdiff_t(bufCapa)) {
                            eof_    = 1;
                        }
                        bufSize_    = r;
                        if (r > 0) {    // データを読み込めていたら 改行コードの変換を行う.
                            const char* s  = buf;
                            const char* se = buf + r;
                            char*       d  = buf;
                            do {
                                int c = *s++;
                                if (c == '\r') {         // \rがあったら 直後の\nと一体化する.
                                    if (s < se) {
                                        if (*s == '\n') {   // \r\nなら \n化.
                                            c = '\n';
                                            ++s;
                                        }
                                    } else {             // バッファの最後1バイトが\rのとき.
                                        if (eof_ == 0) { // EOFがまだなら
                                            restCR_ = 1; // \rを次回に持ち越す.
                                            break;
                                        }                // EOF時はそのまま処理.
                                    }
                                }
                                *d++ = c;
                            } while (s < se);
                            bufSize_ = d - buf;
                        }
                    }
                    return total;
                }

    void        init() {
                    buf_     = NULL;
                    bufSize_ = 0;
                    bufCur_  = 0;
                    bufCapa_ = 0;
                    alcFlag_ = 0;
                    error_   = 0;
                    eof_     = 0;
                    restCR_  = 0;
                }

protected:
    virtual void    raw_close() {}  // = 0;
    virtual size_t  raw_read(void* buf, size_t size) = 0;
    virtual void*   raw_malloc(size_t) { return NULL; }
    virtual void    raw_free(void*) { }

private:
    char*           buf_;                       // バッファ先頭
    unsigned        bufSize_;                   // 読み込んでいるデータのサイズ
    unsigned        bufCur_;                    // 現在の処理位置
    unsigned        bufCapa_;                   // バッファの領域サイズ
    unsigned char   alcFlag_: 1;
    unsigned char   error_  : 1;
    unsigned char   eof_    : 1;
    unsigned char   restCR_ : 1;
    char            innerBuf_[INNER_BUF_SZ];    // 内蔵バッファ. 現状エラー時の最悪回避用.
};

#endif
