/**
 *  @file   DiaKbdMouseHook_Impl.h
 *  @brief  キーボードでマウス操作するためのキーのフック用DLL
 *  @auther Masashi KITAMURA
 *  @date   2006
 *  @license Boost Software License Version 1.0
 */
#ifndef DIAKBDMOUSEHOOK_IMPL_H
#define DIAKBDMOUSEHOOK_IMPL_H
#pragma once

#include <cassert>
#include <windows.h>
#include "DiaKbdMouseHook.h"
#include "../cmn/CriticalSection.hpp"

//#define DIAKBDMOUSEHOOK_USE_EX_SHIFT


/// キー入力を取得(のっとる)ためのフック.
class CDiaKbdMouseHook_Impl {
public:
    enum { VK_NUM = 256 };
    enum { WATCHDOG_MOD_NUM = 8 };      ///< 監視する修飾キー数(L/R別 SHIFT,CTRL,ALT,WIN)

    /// 初期化.
    static void init(HINSTANCE hInst) { s_hInst_ = hInst; }

    /// フックする.
    static bool install(int keycode, CDiaKbdMouseHook_ConvKeyTbl const& tbl);

    /// フックを解除.
    static bool uninstall();

    /// マウス化するキー情報をボタン化したものを取得.
    static unsigned mouseButton();

    /// 修飾キーの押下状態を解放.
    static void releaseModifierKeys();

    /// 修飾キー固着の監視・自動解除. 戻り値は解除したキーのビットマスク(DIAKBDMOUSE_WMOD_*).
    static unsigned watchdog();

    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wparam, LPARAM lparam);

private:
    /// モード切替キーのコードを設定.
    static void setModeKeyTbl(unsigned vkMode, CDiaKbdMouseHook_ConvKeyTbl const& tbl);

    static bool keyDownUp(bool sw, unsigned vkCode);
    static void sendConvKey(bool sw, unsigned uKey );
    static bool sendKey(int mode, unsigned uFlags, unsigned uVk );
    static void setInputParam(INPUT& rImput, unsigned uFlags, unsigned uVk );
    static bool sendInputOne(INPUT& rInput);
    static bool isExtendedKey(unsigned uVk);
    static void sendModifierKeyUpByVk();
    static void sendModifierKeyUpByScanCode();
    static void setSentKeyDown(unsigned uVk, bool sw);
    static void releaseSentKeys();
    static int  modifierIndexOfVk(unsigned uVk);
    static void recordPhysModifier(unsigned uVk, bool sw);
    static void initPhysModifierState();

    static bool makeMouseButton( bool sw, unsigned uVk );
    static bool setMouseButton(unsigned btn, bool sw);

    static void clearStat();

 #ifdef DIAKBDMOUSEHOOK_USE_EX_SHIFT
    static void clearExShift();
 #endif

private:
    enum { DiaKbdMouseHook_EXTRAINFO = 0xD1AC };

    static CCriticalSection     s_criticalSection_;
    static HINSTANCE            s_hInst_;           	///< Windowインスタンス.
    static volatile HHOOK       s_hHook_LL_;        	///< フック.
    static unsigned             s_uModeKey_;        	///< モード切替キーのキーコード.
    static unsigned             s_uMouseButton_;    	///< マウス操作用のボタン情報.
    static int                  s_iMouseButtonLife_;	///< (Win+Lでの)暴発時の被害軽減用.
    static bool                 s_bConvModeStat_;   	///< キー操作によるモードon/off
    static bool                 s_bTwoStStatQ_;     	///< 2ストロークキーモードか.
    static bool                 s_bShiftStat_;      	///< SHIFTが押されてるとき.
    static bool                 s_bCtrlStat_;       	///< CTRLが押されてるとき.
    static bool                 s_bDiaMouse_;       	///< ダイアモンドカーソルでマウスを動かす.
    static bool                 s_bSentKeyDown_[VK_NUM];///< SendInputで押したままのキー.
    static bool                 s_bPhysModDown_[WATCHDOG_MOD_NUM];      ///< フックが確認した修飾キーの物理押下状態.
    static DWORD                s_physModDownTick_[WATCHDOG_MOD_NUM];   ///< 物理押下を確認した時刻(GetTickCount).
    static DWORD                s_orphanSinceTick_[WATCHDOG_MOD_NUM];   ///< 「論理押下だけ残っている」状態を検出した時刻.
    static DWORD                s_watchdogLastTick_;                    ///< watchdog前回実行時刻.
  #ifdef DIAKBDMOUSEHOOK_USE_EX_SHIFT
    static bool                 s_bExShift_;        	///< カーソル移動での自動シフト押し.
  #endif

    static CDiaKbdMouseHook_ConvKeyTbl  s_convKeys_;
};

#endif
