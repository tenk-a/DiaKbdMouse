/**
 *  @file   DiaKbdMouse.cpp
 *  @brief  キーボードでマウスを制御.
 *  @author Masashi KITAMURA
 *  @date   2006
 *  @license Boost Software License Version 1.0
 */

#include "stdafx.h"
#include "DiaKbdMouse.h"
#include <dbt.h>
#include <cstdlib>
#include <cstring>
#include <string>
#include <utility>
#include "../cmn/misc.h"
#include "../dll/DiaKbdMouseHook.h"

namespace {
    enum { TIMER_RELEASE_MODIFIER_KEYS    = 0xD1AC };
    enum { MODIFIER_RELEASE_QUIET_MSEC    = 250 };
    enum { MODIFIER_RELEASE_MAX_WAIT_MSEC = 2000 };
}
#include "KbdMouseCtrl.h"
#include "TrayIcon.h"
#include "ConfigFileReader.h"


/// キーボードでマウスを制御するアプリ・クラス.
class CDiaKbdMouseApp {
public:
    CDiaKbdMouseApp();
    // ~CDiaKbdMouseApp() {;}

    int winMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);

private:
    ATOM            registerClass(HINSTANCE hInstance);
    bool            initInstance(HINSTANCE hInstance);

    static LRESULT CALLBACK wndProc(HWND, UINT, WPARAM, LPARAM);
    static LRESULT CALLBACK dlgProcAbout(HWND, UINT, WPARAM, LPARAM);

    DWORD           getConfigData(CDiaKbdMouseHook_ConvKeyTbl& tbl);
    void            showMessageDialogEJ(wchar_t const* en_msg, wchar_t const* jp_msg);
    LRESULT         wmCreate (HWND hWnd, WPARAM wParam, LPARAM lParam);
    LRESULT         wmCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
    LRESULT         wmDeviceChange(WPARAM wParam);
    LRESULT         wmUserTrayIcon(HWND hWnd, LPARAM lParam);
    void            scheduleModifierRelease();
    void            onModifierReleaseTimer();
    void            checkMenuItem(int id, bool checkSw /*, int dispSw*/);

    //static void     setMinmumWorkingSetSize();

private:
    enum { MAX_LOADSTRING   = 100         };
    enum { WM_USER_TRAYICON = WM_USER + 1 };

    HWND            hWnd_;
    HINSTANCE       hInstance_;                     ///< 現在のインターフェイス.
    HICON           hIconSm_;
    CTrayIcon       trayIcon_;
    TCHAR           szTitle_[MAX_LOADSTRING];       ///< タイトル バーのテキスト.
    TCHAR           szWindowClass_[MAX_LOADSTRING]; ///< メイン ウィンドウ クラス名.
    wchar_t const*  startupErrorEn_;                ///< 起動失敗時に表示する英語メッセージ.
    wchar_t const*  startupErrorJp_;                ///< 起動失敗時に表示する日本語メッセージ.
    bool            modifierReleaseTimerActive_;
    bool            modifierReleasedInBurst_;
    DWORD           modifierReleaseFirstTick_;
    DWORD           modifierReleaseLastTick_;
    //TCHAR         szIniName_[0x4000];             ///< モジュール名.
    static CDiaKbdMouseApp* s_pSelf_;               ///< 自分自身の変数(インスタンス)へのポインタ.
};

/// 自分自身へのポインタ.
CDiaKbdMouseApp*    CDiaKbdMouseApp::s_pSelf_ = 0;


/** コンストラクタ
 */
CDiaKbdMouseApp::CDiaKbdMouseApp()
    : hWnd_(0)
    , hInstance_(0)
    , hIconSm_(0)
    , trayIcon_()
    , startupErrorEn_(0)
    , startupErrorJp_(0)
    , modifierReleaseTimerActive_(false)
    , modifierReleasedInBurst_(false)
    , modifierReleaseFirstTick_(0)
    , modifierReleaseLastTick_(0)
{
    std::memset(szTitle_      , 0, sizeof szTitle_);
    std::memset(szWindowClass_, 0, sizeof szWindowClass_);
    assert( s_pSelf_ == NULL );
    s_pSelf_ = this;
}

/** エントリ.
 */
int CDiaKbdMouseApp::winMain(HINSTANCE /*hInstance0*/, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
    // Windowインスタンスの取得.
    HINSTANCE hInstance = ::GetModuleHandle(NULL);

    // リソースより名前文字列を取得.
    if (::LoadString(hInstance, IDS_APP_TITLE, szTitle_, MAX_LOADSTRING) == 0
        || ::LoadString(hInstance, IDS_APP_NAME, szWindowClass_, MAX_LOADSTRING) == 0
    ) {
        showMessageDialogEJ(
            L"Failed to load application resources.",
            // アプリケーション リソースの読み込みに失敗しました.
            L"\x30A2\x30D7\x30EA\x30B1\x30FC\x30B7\x30E7\x30F3 "
            L"\x30EA\x30BD\x30FC\x30B9\x306E\x8AAD\x307F\x8FBC\x307F"
            L"\x306B\x5931\x6557\x3057\x307E\x3057\x305F\x3002"
        );
        return -1;
    }

    // 多重起動防止.
    HANDLE hMutex = ::CreateMutex(NULL, 1, szWindowClass_);
    if (hMutex == NULL) {
        showMessageDialogEJ(
            L"Failed to create the application mutex.",
            // アプリケーション ミューテックスの作成に失敗しました.
            L"\x30A2\x30D7\x30EA\x30B1\x30FC\x30B7\x30E7\x30F3 "
            L"\x30DF\x30E5\x30FC\x30C6\x30C3\x30AF\x30B9\x306E\x4F5C\x6210"
            L"\x306B\x5931\x6557\x3057\x307E\x3057\x305F\x3002"
        );
        return -1;
    }
    if (::GetLastError() == ERROR_ALREADY_EXISTS) {
        showMessageDialogEJ(
            L"DiaKbdMouse is already running.",
            // DiaKbdMouse は既に起動しています.
            L"DiaKbdMouse \x306F\x65E2\x306B\x8D77\x52D5\x3057\x3066"
            L"\x3044\x307E\x3059\x3002"
        );
        return -2;
    }

    // ウィンドウ クラスを登録.
    if (registerClass(hInstance) == 0) {
        showMessageDialogEJ(
            L"Failed to register the application window class.",
            // アプリケーションのウィンドウ クラス登録に失敗しました.
            L"\x30A2\x30D7\x30EA\x30B1\x30FC\x30B7\x30E7\x30F3\x306E"
            L"\x30A6\x30A3\x30F3\x30C9\x30A6 \x30AF\x30E9\x30B9\x767B\x9332"
            L"\x306B\x5931\x6557\x3057\x307E\x3057\x305F\x3002"
        );
        return -1;
    }

    // アプリケーションの初期化.
    if (initInstance(hInstance/*, nCmdShow */) == 0) {
        showMessageDialogEJ(
            startupErrorEn_ ? startupErrorEn_ : L"Failed to initialize DiaKbdMouse.",
            // DiaKbdMouse の初期化に失敗しました.
            startupErrorJp_ ? startupErrorJp_
                : L"DiaKbdMouse \x306E\x521D\x671F\x5316\x306B\x5931\x6557"
                  L"\x3057\x307E\x3057\x305F\x3002"
        );
        return -1;
    }

    // メイン メッセージ ループ.
    HACCEL      hAccelTable;
    hAccelTable = ::LoadAccelerators(hInstance, (LPCTSTR)IDC_ACCELTABLE);
    MSG         msg;
    while (::GetMessage(&msg, NULL, 0, 0)) {
        if (::TranslateAccelerator(msg.hwnd, hAccelTable, &msg) == 0) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
        //Sleep(33);
        // Sleep 禁止. GetMessage 側ブロックで空ループ無なので不要.
        // WH_KEYBOARD_LL の callback は、このスレッドのメッセージ処理経由なので,
        // 1メッセージ毎に Sleep すると、連打時処理滞留,
        // LowLevelHooksTimeout(既定数百ms)超過で,
        // イベントがフック素通り(モードキーUPの取り逃し→モード固着)したり,
        // OSに黙ってフックを外されたりする.
    }

    // DLL終了.
    DiaKbdMouseHook_uninstall();

    return (int) msg.wParam;
}

///  ウィンドウ クラスを登録.
///
ATOM CDiaKbdMouseApp::registerClass(HINSTANCE hInstance)
{
    WNDCLASSEX      wcex;
    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = (WNDPROC) wndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = ::LoadIcon(hInstance, (LPCTSTR) IDI_ICON);
    wcex.hCursor        = ::LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH) (COLOR_WINDOW + 1);
    wcex.lpszMenuName   = (LPCTSTR) IDC_MENU;
    wcex.lpszClassName  = szWindowClass_;
    wcex.hIconSm        = hIconSm_ = ::LoadIcon(wcex.hInstance, (LPCTSTR) IDI_SMALL);
    return ::RegisterClassEx(&wcex);
}

/// メイン プログラム ウィンドウを作成.
///
bool CDiaKbdMouseApp::initInstance(HINSTANCE hInstance /*, int nCmdShow*/)
{
    hInstance_  = hInstance;
    hWnd_       = ::CreateWindowEx(
                        0/*dwExStyle*/,
                        szWindowClass_,
                        szTitle_,
                        WS_CAPTION | /*WS_SYSMENU |*/ WS_POPUP,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        0,
                        0,
                        NULL,
                        NULL,
                        hInstance,
                        NULL);

    if (hWnd_ == 0)
        return false;

    ::ShowWindow(hWnd_, SW_HIDE/*nCmdShow*/);
    ::UpdateWindow(hWnd_);

    return true;
}

/// メイン ウィンドウのメッセージを処理.
///
LRESULT CALLBACK CDiaKbdMouseApp::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CDiaKbdMouseApp*    pSelf = s_pSelf_;
    assert( pSelf != 0 );
    switch (uMsg) {
    case WM_CREATE:
        return pSelf->wmCreate(hWnd, wParam, lParam);

    case WM_COMMAND:        // メニュー項目の処理.
        if (pSelf->wmCommand(hWnd, wParam, lParam) == 0)
            return 0;
        break;

    case WM_USER_TRAYICON:  // トレイアイコンでクリックされたときの処理.
        return pSelf->wmUserTrayIcon(hWnd, lParam);

    case WM_DEVICECHANGE:   // キーボード等の抜き差しで修飾キー状態が残らないようにする.
        return pSelf->wmDeviceChange(wParam);

    case WM_TIMER:
        if (wParam == TIMER_RELEASE_MODIFIER_KEYS) {
            pSelf->onModifierReleaseTimer();
            return 0;
        }
        break;

    case WM_POWERBROADCAST:
        switch (wParam) {
        case PBT_APMRESUMECRITICAL:
        case PBT_APMRESUMESUSPEND:
        case PBT_APMRESUMEAUTOMATIC:
            // サスペンドや休止からの復帰では、停止中にKEYUPを取り逃ス場合がある模様.
            pSelf->scheduleModifierRelease();
            break;
        default:
            ;
        }
        return TRUE;

    case WM_DESTROY:
        ::KillTimer(hWnd, TIMER_RELEASE_MODIFIER_KEYS);
        pSelf->modifierReleaseTimerActive_ = false;
        DiaKbdMouseHook_releaseModifierKeys();
        // キーボードでマウス操作する処理のスレッドを終了.
        CKbdMouseCtrl::release();
        // トレイアイコンの終了.
        pSelf->trayIcon_.release();
        // メッセージループを終了させる.
        if (hWnd == pSelf->hWnd_)   // wmCreate 成功時のみ Quit.
            ::PostQuitMessage(0);
        return 0;

    default:
        pSelf->trayIcon_.checkRecreate(uMsg);   // タスクバーが再作成されたらアイコンを再登録.
    }

    return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/// 定義ファイル取得.
///
DWORD   CDiaKbdMouseApp::getConfigData(CDiaKbdMouseHook_ConvKeyTbl& tbl)
{
    DWORD   dwKeyCode = 0;
    wchar_t moduleName[0x1000];
    DWORD length = ::GetModuleFileNameW(NULL, moduleName, 0x1000);
    std::wstring configName(moduleName, length);
    std::size_t l = configName.size();
    if (l > 4) {
        configName.replace(l - 3, 3, L"cfg");
        std::string configNameUtf8 = wcsToUtf8(configName);
        //x DEBUGPRINTF("%s\n", path);
        CConfigFileReader   configData(configNameUtf8.c_str(), tbl);
        dwKeyCode   = configData.getData();
    }
    return dwKeyCode;
}

/// 英語メッセージをログに出力し、UI言語に応じたメッセージを表示.
///
void CDiaKbdMouseApp::showMessageDialogEJ(wchar_t const* en_msg, wchar_t const* jp_msg)
{
    LogPuts(en_msg);
    LogPuts(L"\n");

    LANGID language = ::GetUserDefaultLangID();
    bool isJapanese = PRIMARYLANGID(language) == LANG_JAPANESE;
    showMessageDialog(isJapanese && jp_msg ? jp_msg : en_msg);
}

/// WM_CREATE で行う処理.
///
LRESULT CDiaKbdMouseApp::wmCreate(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    // トレイにアイコンを登録.
    int rc = trayIcon_.create(
            WM_USER_TRAYICON,
            hInstance_,
            hWnd,
            hIconSm_,
            _T("DiaKbdMouse"),
            IDC_MENU
        );
    if (rc == 0) {      // トレイアイコンの作成に失敗したら終了.
        startupErrorEn_ = L"Failed to create the notification area icon.";
        // 通知領域アイコンの作成に失敗しました。
        startupErrorJp_ =
            L"\x901A\x77E5\x9818\x57DF\x30A2\x30A4\x30B3\x30F3\x306E\x4F5C\x6210"
            L"\x306B\x5931\x6557\x3057\x307E\x3057\x305F\x3002";
        return -1;
    }

    // 定義ファイル読み込み.
    CDiaKbdMouseHook_ConvKeyTbl tbl;
    tbl.clear();
    DWORD   dwKeyCode = getConfigData(tbl); // VK_RWIN; // VK_RMENU;
    if (dwKeyCode == 0) {   // 定義ファイルでエラーがあった場合.
        startupErrorEn_ = L"Failed to load the configuration file.";
        // 設定ファイルの読み込みに失敗しました。
        startupErrorJp_ =
            L"\x8A2D\x5B9A\x30D5\x30A1\x30A4\x30EB\x306E\x8AAD\x307F\x8FBC\x307F"
            L"\x306B\x5931\x6557\x3057\x307E\x3057\x305F\x3002";
        return -1;
    }
    if (DiaKbdMouseHook_install(dwKeyCode, tbl) == 0) {
        startupErrorEn_ = L"Failed to install the keyboard hook.";
        // キーボード フックの設定に失敗しました。
        startupErrorJp_ =
            L"\x30AD\x30FC\x30DC\x30FC\x30C9 \x30D5\x30C3\x30AF\x306E\x8A2D\x5B9A"
            L"\x306B\x5931\x6557\x3057\x307E\x3057\x305F\x3002";
        return -1;
    }

    // キーボードでマウス操作する処理のスレッドを起動.
    if (!CKbdMouseCtrl::create()) {
        DiaKbdMouseHook_uninstall();
        startupErrorEn_ = L"Failed to start the keyboard control thread.";
        // キーボード制御スレッドの起動に失敗しました。
        startupErrorJp_ =
            L"\x30AD\x30FC\x30DC\x30FC\x30C9\x5236\x5FA1\x30B9\x30EC\x30C3\x30C9"
            L"\x306E\x8D77\x52D5\x306B\x5931\x6557\x3057\x307E\x3057\x305F\x3002";
        return -1;
    }

    // ヒープメモリの調整.
    //setMinmumWorkingSetSize();
    return 0;
}

/// WM_COMMAND で行う処理.
///
LRESULT CDiaKbdMouseApp::wmCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
    DWORD wmId    = LOWORD(wParam);
    //x DWORD wmEvent = HIWORD(wParam);
    // 選択されたメニューの解析.
    switch (wmId) {
    case IDM_HELP:
        {
            TCHAR path[MAX_PATH+16]; // 実行ファイル名を格納するバッファ.
            ::GetModuleFileName( NULL, path, MAX_PATH );
            std::size_t l = ::_tcslen(path);
            if (l > 4) {
                std::memcpy(&path[l-3], _T("htm"), 4*sizeof(TCHAR));    // 拡張子をhtmにしてそれを開くことにする.
                //x DEBUGPRINTF("%s\n", path);
                ::ShellExecute (hWnd, _T("open"), path, NULL, NULL, SW_SHOW);
                //setMinmumWorkingSetSize();
            }
        }
        return 0;

    case IDM_ABOUT:
        ::DialogBox(hInstance_, (LPCTSTR) IDD_ABOUTBOX, hWnd, (DLGPROC) dlgProcAbout);
        return 0;

    case IDM_RELEASE_MODIFIERS: // 修飾キー固着時の手動レスキュー.
        LogPrintf("# Manual ReleaseModifierKeys from tray menu\n");
        DiaKbdMouseHook_releaseModifierKeys();
        return 0;

    case IDM_EXIT:
        ::DestroyWindow(hWnd);
        return 0;

    default:
        ;
    }
    return 1;
}

/// デバイス構成変更時に行う処理.
///
LRESULT CDiaKbdMouseApp::wmDeviceChange(WPARAM wParam)
{
    switch (wParam) {
    case DBT_DEVICEARRIVAL:
    case DBT_DEVICEREMOVECOMPLETE:
    case DBT_DEVICEREMOVEPENDING:
    case DBT_DEVNODES_CHANGED:
    case DBT_CONFIGCHANGED:
        LogPrintf("# DeviceChange 0x%x(%d)\n", (unsigned)wParam, (int)wParam);
        scheduleModifierRelease();
        break;
    default:
        break;
    }
    return TRUE;
}


/// デバイス変更等の後に修飾キーを解除するタイマーを開始.
///
void CDiaKbdMouseApp::scheduleModifierRelease()
{
    DWORD now = ::GetTickCount();
    modifierReleaseLastTick_ = now;
    if (!hWnd_ || modifierReleaseTimerActive_)
        return;

    modifierReleaseFirstTick_ = now;
    modifierReleasedInBurst_ = false;
    if (::SetTimer(hWnd_, TIMER_RELEASE_MODIFIER_KEYS, MODIFIER_RELEASE_QUIET_MSEC, NULL))
        modifierReleaseTimerActive_ = true;
}


/// デバイス変更の連打をデバウンスしつつ、解除が永久に先送りされるのを防ぐ.
///
void CDiaKbdMouseApp::onModifierReleaseTimer()
{
    DWORD now       = ::GetTickCount();
    DWORD quietMsec = now - modifierReleaseLastTick_;
    DWORD waitMsec  = now - modifierReleaseFirstTick_;

    // 通常は変更通知が静まってから解除.
    // 通知が止まらない環境でも2秒で一度だけ強制解除し,
    // 同じ通知バースト中に繰り返しShiftを切らない.
    if (!modifierReleasedInBurst_
        && (quietMsec >= MODIFIER_RELEASE_QUIET_MSEC
            || waitMsec >= MODIFIER_RELEASE_MAX_WAIT_MSEC)
    ) {
        LogPrintf("# ReleaseModifierKeys quiet=%lu wait=%lu%s\n",
            (unsigned long)quietMsec,
            (unsigned long)waitMsec,
            quietMsec < MODIFIER_RELEASE_QUIET_MSEC ? " forced" : "");
        DiaKbdMouseHook_releaseModifierKeys();
        modifierReleasedInBurst_ = true;
    }

    if (quietMsec >= MODIFIER_RELEASE_QUIET_MSEC) {
        ::KillTimer(hWnd_, TIMER_RELEASE_MODIFIER_KEYS);
        modifierReleaseTimerActive_ = false;
    }
}

/// メニュー項目にチェックマークをつけはずし.
///
void CDiaKbdMouseApp::checkMenuItem(int id, bool checkSw /*, int dispSw*/)
{
    int     flags = checkSw ? MFS_CHECKED : MFS_UNCHECKED;
    // HMENU    hMenu = ::GetSubMenu(::GetMenu(hWnd_), 0);
    HMENU   hMenu = trayIcon_.getMenuHandle();
    //x int checked =
    ::CheckMenuItem(hMenu, id, MF_BYCOMMAND|flags);

    //x int dispFlags   = (dispMode < 0) ? MF_GLAY : (dispMode == 0) ? MF_DISABLE : MF_ENABLE;
    //x EnableMenuItem(hMenu, id, MF_BYCOMMAND|dispFlags);
}

/// 通知領域のアイコンに対して操作が行われた場合.
///
LRESULT CDiaKbdMouseApp::wmUserTrayIcon(HWND hWnd, LPARAM lParam)
{
    switch (lParam) {
    case WM_RBUTTONDOWN:    // 右クリックのとき、ポップアップメニューで選択.
        {
            unsigned no = trayIcon_.trackPopupMenu();
            wmCommand(hWnd, no, lParam);
        }
        break;
    default:
        ;
    }
    return 0;
}

/// バージョン情報ボックスのメッセージ ハンドラ.
///
LRESULT CALLBACK CDiaKbdMouseApp::dlgProcAbout(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    (void)lParam;
    switch (uMsg) {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            ::EndDialog(hDlg, LOWORD(wParam));
            //setMinmumWorkingSetSize();
            return TRUE;
        }
        break;

    default:
        ;
    }
    return FALSE;
}

#if 0
/// ヒープメモリの調整.
///
void CDiaKbdMouseApp::setMinmumWorkingSetSize()
{
    HINSTANCE hDll = ::LoadLibrary( _T( "kernel32.dll" ) );
    if ( hDll ) {
        typedef BOOL ( WINAPI *fn_t_SetProcessWorkingSetSize )( HANDLE, SIZE_T, SIZE_T );
        fn_t_SetProcessWorkingSetSize fn_SetProcessWorkingSetSize
            = reinterpret_cast< fn_t_SetProcessWorkingSetSize >( ::GetProcAddress( hDll, "SetProcessWorkingSetSize" ) );
        if ( fn_SetProcessWorkingSetSize ) {
            HANDLE hCurrentProcess = ::GetCurrentProcess();
            fn_SetProcessWorkingSetSize(hCurrentProcess, SIZE_T(-1), SIZE_T(-1));
        }
        ::FreeLibrary( hDll );
    }
}
#endif


// ===========================================================================
/** 起動エントリ.
 */
// ===========================================================================
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    LogPrintfInit(fpath_getLocalAppDataA() + "\\tenk-a\\DiaKbdMouse\\Log\\DiaKbdMouse.Log");
    CDiaKbdMouseApp     diaCursorApp;
    return diaCursorApp.winMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}
