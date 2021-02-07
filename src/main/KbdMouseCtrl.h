/**
 *  @file   KbdMouseCtrl.h
 *  @brief  キーボードでマウス操作する処理(別スレッド)
 *  @auther Masashi KITAMURA
 *  @date   2006
 *  @note
 *      フリーソース
 */
#ifndef KBDMOUSECTRL_H
#define KBDMOUSECTRL_H

#pragma once

#include "stdafx.h"
#include "../cmn/DgtXY2AnlgXY.hpp"


/// キーボードでマウス操作する処理(別スレッド)
class CKbdMouseCtrl {
public:
    /// 作成.
    static void     create();

    /// 解除.
    static void     release();

private:
    CKbdMouseCtrl();
    ~CKbdMouseCtrl();

    static void     run();

    static void     ctrl();
    static void     moveMouse(unsigned uNow, unsigned uOld, unsigned uTrig);
    static unsigned makeReptKey(unsigned uNow, unsigned uOld, unsigned uTrig);
    static void     sendMouseButton(unsigned uTrig, unsigned uRel);

private:
    enum {SLEEP_COUNT =  8 };
    enum { DLT        =  4 };
    enum { HIS_NUM    = 12 };
    static HANDLE                           s_hThread_;                 ///< スレッドハンドル.
    static unsigned                         s_uOld_;                    ///< 1フレーム前のボタン情報.
    static DgtXY2AnlgXY<float,256,HIS_NUM>  s_dgtXY2AnlgXY_;            ///< デジタルボタン情報をアナログ化.
    static bool                             s_bWin1st_;
    static unsigned                         s_uReptCnt_;
    static const INPUT                      s_input_mouseSendTbl_[];    ///< マウス情報に変換するときに使う.
};


#endif
