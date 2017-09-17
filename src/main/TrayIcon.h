/**
 *	@file	TrayIcon.h
 *	@brief	トレイ常駐アイコン処理用クラス
 *	@author	Masashi KITAMURA
 *	@date	2006
 *	@note
 *		フリーソース
 */
#ifndef TRAYICON_H
#define TRAYICON_H

#pragma once

#include <windows.h>


///	トレイ常駐アイコン処理用クラス
class CTrayIcon {
public:
	CTrayIcon() { for (int i = sizeof *this; --i >= 0;) ((char*)this)[i] = '\0'; }	// 無作法
	~CTrayIcon() { release(); }

	/// トレイアイコンの作成
	bool create(unsigned uWm, HINSTANCE hInstance, HWND hWnd, HICON hIcon, const TCHAR* pTip, UINT uMenu);

	/// トレイアイコンの開放
	void release();

	/// WndProc にてトレイ作り直しのウィンドウメッセージが来たかチェックして来てたら再作成
	void checkRecreate(UINT uMsg);

	/// アイコン位置でポップアップメニューで選択されたら、そのIDを返す。無効なら0を返す。
	UINT trackPopupMenu();

	/// 現在のトレイメニューのハンドルを返す
	const HMENU	getMenuHandle() const { return hMenuSub_; }

private:
	NOTIFYICONDATA 		notifyIconData_;			///< トレイにのるアイコンのための情報
	HINSTANCE			hInstance_;					///< インスタンス･ハンドル
	HWND				hWnd_;						///< ウィンドウ・ハンドル
	UINT 				uWmTaskBarCreated_;			///< タスクバーが作られた時にくるウィンドウハンドル
	HMENU				hMenu_;						///< トレイメニューのハンドル
	HMENU				hMenuSub_;					///< トレイメニューのサブメニューのハンドル
	UINT				uMenuId_;					///< メニューのID
};


#endif
