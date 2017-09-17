/**
 *	@file	DiaKbdMouseHook.h
 *	@brief	キーボードでマウス操作するためのキーのフック用DLL
 *	@auther	Masashi KITAMURA
 *	@date	2006
 *  @note
 *		フリーソース
 */
#ifndef DIAKBDMOUSEHOOK_H
#define DIAKBDMOUSEHOOK_H
#pragma once

#include <cassert>
#include <cstring>

#ifdef HOOKDLL_EXPORTS
#define HOOKDLL_API extern "C" __declspec(dllexport)
#else
#define HOOKDLL_API extern "C" __declspec(dllimport)
#endif

class CDiaKbdMouseHook_ConvKeyTbl;

/// フックする
HOOKDLL_API int 	 DiaKbdMouseHook_install(int keyCode, CDiaKbdMouseHook_ConvKeyTbl const& tbl);

/// フックをやめる
HOOKDLL_API int 	 DiaKbdMouseHook_uninstall();

/// モードを切り替えるキーを設定
HOOKDLL_API void	DiaKbdMouseHook_setModeKey(unsigned vkMode);

#ifdef USE_LWINKEY
/// 左Winキーによるマウス操作のon/off
HOOKDLL_API	bool	DiaKbdMouseHook_setLWinMode(int sw);
#endif

/// パッド的にしたマウスボタン情報の取得
HOOKDLL_API unsigned DiaKbdMouseHook_mouseButton();

/// マウス化するキーの情報
enum EDiaKbdMouse_Mouse {
	DIAKBDMOUSE_MOUSE_LEFT		= 0x0001,
	DIAKBDMOUSE_MOUSE_UP		= 0x0002,
	DIAKBDMOUSE_MOUSE_RIGHT		= 0x0004,
	DIAKBDMOUSE_MOUSE_DOWN		= 0x0008,
	DIAKBDMOUSE_MOUSE_LBUTTON	= 0x0010,
	DIAKBDMOUSE_MOUSE_RBUTTON	= 0x0020,
	DIAKBDMOUSE_MOUSE_MBUTTON	= 0x0040,
	DIAKBDMOUSE_MOUSE_XBUTTON1	= 0x0080,
	DIAKBDMOUSE_MOUSE_XBUTTON2	= 0x0100,
	DIAKBDMOUSE_MOUSE_WHEEL1	= 0x0200,
	DIAKBDMOUSE_MOUSE_WHEEL2	= 0x0400,
  #ifdef USE_LWINKEY
	DIAKBDMOUSE_MOUSE_MODEKEY	= 0x0800,		///< 左Winを使ったときの対策
  #else
	DIAKBDMOUSE_MOUSE_SPEEDUP	= 0x0800,
  #endif
	DIAKBDMOUSE_MOUSE_SPEEDCHG	= 0x1000,
};



/// 変換テーブル用の型
class CDiaKbdMouseHook_ConvKey {
public:
	///< モード
	enum EMode {
		MD_NONE		= 0,			///< 使わない
		MD_USE		= 1,			///< 使う
		MD_CTRL		= 2,			///< CTRL併用
		MD_SHIFT	= 3,			///< SHIFT併用
		MD_CTRLSHIFT= 4,			///< CTRL+SHIFT併用
		MD_2ST_Q	= 5,			///< 2ストロークキーのトリガー(Q)
		MD_EX_SHIFT = 6,			//x 廃止(カーソル移動でシフト併用モード(範囲選択を想定))
		MD_MOUSE	= 7,			///< マウス
	};
	class COne {
	public:
		unsigned char u8Mode_;		///< モード(EMode)
		unsigned char u8VkCode_;	///< 変換するキーコード
	};
	COne	oneKey_[2];				///< [0]=拡張キーのみのときのキー  [1]=2ストロークキー(Q)を押したとき用
};

class CDiaKbdMouseHook_ConvKeyTbl {
public:
	enum { MAX_SIZE = 256 };
	//CDiaKbdMouseHook_ConvKeyTbl() { std::memset(tbl_, 0, sizeof tbl_); }
	CDiaKbdMouseHook_ConvKey& operator[](unsigned n) { assert(n < MAX_SIZE); return tbl_[n]; }
	CDiaKbdMouseHook_ConvKey const& operator[](unsigned n) const { assert(n < MAX_SIZE); return tbl_[n]; }
	void	clear() { std::memset(tbl_, 0, sizeof tbl_); }
public:
	CDiaKbdMouseHook_ConvKey	tbl_[MAX_SIZE];
};

#endif
