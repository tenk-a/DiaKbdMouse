/**
 *	@file	DiaKbdMouseHook_Impl.h
 *	@brief	キーボードでマウス操作するためのキーのフック用DLL
 *	@auther	Masashi KITAMURA
 *	@date	2006
 *	@note
 *		フリーソース
 */
#ifndef DIAKBDMOUSEHOOK_IMPL_H
#define DIAKBDMOUSEHOOK_IMPL_H
#pragma once

#include <cassert>
#include <windows.h>
#include "DiaKbdMouseHook.h"
#include "../cmn/CriticalSection.hpp"

//#define DIAKBDMOUSEHOOK_USE_EX_SHIFT


/// キー入力を取得(のっとる)ためのフック
class CDiaKbdMouseHook_Impl {
public:
	enum { VK_NUM = 256 };

	/// 初期化
	static void init(HINSTANCE hInst) { s_hInst_ = hInst; }

	/// フックする
	static bool install(int keycode, CDiaKbdMouseHook_ConvKeyTbl const& tbl);

	/// フックを解除
	static bool uninstall();

	/// マウス化するキー情報をボタン化したものを取得
	static unsigned mouseButton();

  #ifdef USE_LWINKEY
	/// 左Win拡張のon/off
	static bool setLWinMode(int sw);
  #endif

	/// 変換テーブルを指定ファイルから読み込む
	static void setConvTable(const CDiaKbdMouseHook_ConvKey tbl[VK_NUM]);

private:
	static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wparam, LPARAM lparam);

	/// モード切替キーのコードを設定
	static void setModeKeyTbl(unsigned vkMode, CDiaKbdMouseHook_ConvKeyTbl const& tbl);

	static bool keyDownUp(bool sw, unsigned vkCode);
	static void sendConvKey(bool sw, unsigned uKey );
	static void sendKey(int mode, unsigned uFlags, unsigned uVk );
	static void setInputParam(INPUT& rImput, unsigned uFlags, unsigned uVk );

	static bool makeMouseButton( bool sw, unsigned uVk );
	static bool setMouseButton(unsigned btn, bool sw);

	static void clearStat();

 #ifdef DIAKBDMOUSEHOOK_USE_EX_SHIFT
	static void clearExShift();
 #endif

private:
	enum { DiaKbdMouseHook_EXTRAINFO = 0xD1AC };

	static CCriticalSection		s_criticalSection_;
	static HINSTANCE   			s_hInst_;			///< Windowインスタンス
	static volatile HHOOK		s_hHook_LL_;		///< フック
	static unsigned				s_uModeKey_;		///< モード切替キーのキーコード
	static unsigned				s_uMouseButton_;	///< マウス操作用のボタン情報
	static int					s_iMouseButtonLife_; ///< (Win+Lでの)暴発時の被害軽減用
	static bool					s_bConvModeStat_;	///< キー操作によるモードon/off
	static bool					s_bTwoStStatQ_;		///< 2ストロークキーモードか
	static bool					s_bShiftStat_;		///< SHIFTが押されてるとき
	static bool					s_bCtrlStat_;		///< CTRLが押されてるとき
	static bool					s_bDiaMouse_;		///< ダイアモンドカーソルでマウスを動かす
  #ifdef DIAKBDMOUSEHOOK_USE_EX_SHIFT
	static bool					s_bExShift_;		///< カーソル移動での自動シフト押し
  #endif
  #ifdef USE_LWINKEY
	static bool					s_bLWinStat_;		///< 左Winキー操作によるon/off
	static bool					s_bLWinModeSw_;		///< 左Winキーによるキーボードマウス利用のon/off
  #endif

	static CDiaKbdMouseHook_ConvKeyTbl	s_convKeys_;
};

#endif
