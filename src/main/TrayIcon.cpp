/**
 *  @file   TrayIcon.cpp
 *  @brief  トレイ常駐アイコン処理用クラス
 *  @author Masashi KITAMURA
 *  @date   2006
 *  @note
 *      フリーソース
 */

#include "stdafx.h"
#include "TrayIcon.h"


/// 文字列src をdstに size-1 の範囲でコピー。dstには必ず最後に\0をつける.
static inline const TCHAR* tcsCpyNum(TCHAR *dst, const TCHAR *src, unsigned size) {
    TCHAR*      d = dst;
    if (src && *src) {
        const TCHAR *e = dst+size;
        if (size)
            --e;
        const TCHAR * s = src;
        do {
            *d++ = *s++;
        } while ((*s != 0) & (d < e));
    }
    *d = _T('\0');
    return dst;
}

/** トレイアイコンの作成.
 */
bool CTrayIcon::create(unsigned uWm, HINSTANCE hInstance, HWND hWnd, HICON hIcon, const TCHAR* pTip, UINT uMenu)
{
    hInstance_                          = hInstance;
    hWnd_                               = hWnd;

    // 通知領域のアイコンを作成.
    notifyIconData_.cbSize              = sizeof(NOTIFYICONDATA);
    notifyIconData_.hWnd                = hWnd;
    notifyIconData_.uID                 = 0;
    notifyIconData_.uFlags              = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    notifyIconData_.uCallbackMessage    = uWm;
    notifyIconData_.hIcon               = hIcon;

    tcsCpyNum(notifyIconData_.szTip, pTip, sizeof(notifyIconData_.szTip));

    // アイコンを登録できるかエラーになるまで繰り返す.
    while (::Shell_NotifyIcon(NIM_ADD, &notifyIconData_) == 0) {
        if (::GetLastError() != ERROR_TIMEOUT) {    // タイムアウト以外なら.
            notifyIconData_.cbSize = 0;
            return false;   // アイコン登録エラー.
        }
        // 登録できていないことを確認する.
        if (::Shell_NotifyIcon(NIM_MODIFY, &notifyIconData_)) {
            // 登録できていた.
            break;
        } else {
            // 登録できていなかった.
            ::Sleep(100);
        }
    }

    uWmTaskBarCreated_ = ::RegisterWindowMessage( _T("TaskbarCreated") );

    // ここでメニューも用意してしまう.
    uMenuId_  = uMenu;
    hMenu_    = ::LoadMenu( hInstance_, MAKEINTRESOURCE( uMenu ) );
    hMenuSub_ = ::GetSubMenu( hMenu_, 0 );
    return true;
}

/** トレイアイコンの開放.
 */
void CTrayIcon::release()
{
    // 通知領域のアイコンの削除.
    if (notifyIconData_.cbSize) {
        notifyIconData_.cbSize  = sizeof(NOTIFYICONDATA);
        notifyIconData_.hWnd    = hWnd_;
        notifyIconData_.uID     = 0;
        notifyIconData_.uFlags  = 0;
        ::Shell_NotifyIcon(NIM_DELETE, &notifyIconData_);
        notifyIconData_.cbSize  = 0;
    }
    ::DestroyMenu(hMenu_);  //メニュー破棄.
}

/** WndProc にてトレイ作り直しのウィンドウメッセージが来たかチェックして来てたら再作成.
 */
void CTrayIcon::checkRecreate(UINT msg)
{
    if (msg == uWmTaskBarCreated_) {
        create(notifyIconData_.uCallbackMessage, hInstance_, hWnd_, notifyIconData_.hIcon, notifyIconData_.szTip, uMenuId_);
    }
}

/** アイコン位置でポップアップメニューで選択されたら、そのIDを返す。無効なら0を返す.
 */
UINT CTrayIcon::trackPopupMenu()
{
    // ポップアップメニューを開く前にメインウィンドウをアクティブにする.
    ::SetForegroundWindow(hWnd_);

    // ポップアップメニューを開く.
    POINT point;
    ::GetCursorPos(&point);
    if (hMenuSub_ == NULL)
        return 0;

    UINT n = ::TrackPopupMenu(
                    hMenuSub_,
                    TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
                    point.x,
                    point.y,
                    0,
                    hWnd_,
                    NULL
                );
    return n;
}
