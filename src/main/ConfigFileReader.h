/**
 *  @file   ConfigFileReader.h
 *  @brief  定義ファイルからキー定義を取得する.
 *  @author Masashi Kitamura
 *  @date   2013-09
 *  @license Boost Software License Version 1.0
 */
#ifndef CONFIGFILE_READER_H
#define CONFIGFILE_READER_H

#include <string>
#include "../dll/DiaKbdMouseHook.h"

/// 定義ファイル読み込み.
class CConfigFileReader {
public:
    CConfigFileReader(char const fname[], CDiaKbdMouseHook_ConvKeyTbl& rTbl)
        : fileName_(fname ? fname : ""), fline_(0), rTbl_(rTbl) {}

    unsigned getData();

private:
    void operator=(CConfigFileReader const&);   // 未使用.

    char const* skip_spc(char const* s);
    void        set1Data(bool qmode, const char* str);
    unsigned    getKey1(const char*& rStr, unsigned mode);
    void        get_name(char* name, std::size_t sz, const char*& rStr);

    void logPuts(const char* str);

private:
    struct KeyNameVal {
        bool operator<(const KeyNameVal& r) const {
            return strcmp(this->name, r.name) < 0;
        }
    public:
        const char* name;
        unsigned    val;
    };

    enum {
        F_CTRL  = 1,
        F_SHIFT = 2,
        F_ALT   = 4,
        F_WIN   = 8,
        F_DIRECT= 0x10,
    };
private:
    std::string                     fileName_;
    unsigned                        fline_;
    CDiaKbdMouseHook_ConvKeyTbl&    rTbl_;
    static unsigned const           s_keyNameValTblSize_;
    static KeyNameVal const         s_keyNameValTbl_[];
};

#endif
