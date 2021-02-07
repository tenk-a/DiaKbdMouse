/**
 *  @file   FileTextReader.hpp
 *  @brief  バイナリファイルをテキストファイルとして文字・行読み込みするためのクラス
 *  @note
 *  - \r\n を \n 化する.
 */
#ifndef FILETEXTREADER_HPP
#define FILETEXTREADER_HPP

#include "FileHdl.hpp"
#include "TextReaderBase.hpp"

class FileTextReader : public TextReaderBase {
    enum { DFLT_BUF_SZ = 1024 * 1024 };
    typedef TextReaderBase  base;
public:
    typedef FileHdl::OpenMode   OpenMode;

    FileTextReader() {}
    explicit FileTextReader(TCHAR const* nm, OpenMode md=FileHdl::R) { open(nm, md); }
    ~FileTextReader() { }

    bool        open(TCHAR const* fname, OpenMode md=FileHdl::R) {
                    bool rc = fh_.open(fname, md);
                    if (rc)
                        setbuf(NULL, DFLT_BUF_SZ);
                    return rc;
                }
    bool        openStd(int type) { return type ? false : fh_.openStd(type); }

    bool        is_open() const { return fh_.is_open(); }

    //void      close() { base::close(); }
    //size_t    read(void* readbuf, size_t sz) { return base::read(readbuf, sz); }
    //int       getc() { return base::getc(); }
    //char*     gets(char* buf, size_t sz) { return base::gets(buf, sz); }
    //int       setbuf(char* buf, size_t sz) { return base::setbuf(buf, sz); }

private:
    void        raw_close() { fh_.close(); }
    size_t      raw_read(void* buf, size_t sz) { return fh_.read(buf, sz); }
    void        raw_free(void* p) { return FileHdl::memFree(p); }
    void*       raw_malloc(size_t sz) { return FileHdl::memRealloc(NULL, sz); }

private:
    FileHdl     fh_;
};

#endif
