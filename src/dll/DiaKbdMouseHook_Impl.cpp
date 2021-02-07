/**
 *  @file   DiaKbdMouseHook_Impl.cpp
 *  @brief  APPS+を用いたダイアモンドカーソル操作するためのキーのフック側処理
 *  @auther Masashi KITAMURA
 *  @date   2006
 *  @note
 *      フリーソース
 */

#include "stdafx.h"
#include <windows.h>
#include <functional>
#include "DiaKbdMouseHook_Impl.h"
#include "DiaKbdMouseHook.h"
#include "../cmn/DebugPrintf.h"

#if 0 //def NDEBUG   // 実行時の使用メモリを減らす
//#pragma comment(linker, "/opt:nowin98")
//#pragma comment(linker, "/ignore:4078")
#pragma comment(linker, "/entry:\"DllMain\"")
#pragma comment(linker, "/nodefaultlib:\"libc.lib\"")
#pragma comment(linker, "/merge:.rdata=.text")
#pragma comment(linker, "/base:\"0x18000000\"")
#endif


#pragma comment(linker, "/section:DIACURSO,rws")
#pragma data_seg("DIACURSO")
volatile HHOOK  CDiaKbdMouseHook_Impl::s_hHook_LL_  = 0;
#pragma data_seg()


CCriticalSection    CDiaKbdMouseHook_Impl::s_criticalSection_;

HINSTANCE   CDiaKbdMouseHook_Impl::s_hInst_             = 0;
unsigned    CDiaKbdMouseHook_Impl::s_uModeKey_          = 0;
unsigned    CDiaKbdMouseHook_Impl::s_uMouseButton_      = 0;
int         CDiaKbdMouseHook_Impl::s_iMouseButtonLife_  = 0;

bool        CDiaKbdMouseHook_Impl::s_bDiaMouse_         = false;
bool        CDiaKbdMouseHook_Impl::s_bConvModeStat_     = false;
bool        CDiaKbdMouseHook_Impl::s_bTwoStStatQ_       = false;
bool        CDiaKbdMouseHook_Impl::s_bShiftStat_        = false;
bool        CDiaKbdMouseHook_Impl::s_bCtrlStat_         = false;

#ifdef DIAKBDMOUSEHOOK_USE_EX_SHIFT
bool        CDiaKbdMouseHook_Impl::s_bExShift_          = false;
#endif


/** フックを設定.
 */
bool CDiaKbdMouseHook_Impl::install(int keycode, CDiaKbdMouseHook_ConvKeyTbl const& tbl)
{
    if (s_hHook_LL_ == 0) {
        s_criticalSection_.create();
        clearStat();
        s_bDiaMouse_ = false;
        setModeKeyTbl(keycode, tbl);
        s_hHook_LL_  = ::SetWindowsHookEx(WH_KEYBOARD_LL, CDiaKbdMouseHook_Impl::LowLevelKeyboardProc, s_hInst_, 0);
        if (s_hHook_LL_ == NULL) {
            return false;
        }
    }
    return true;
}


/** フックを解除.
 */
bool CDiaKbdMouseHook_Impl::uninstall()
{
    if (s_hHook_LL_ && UnhookWindowsHookEx(s_hHook_LL_) == 0)
        return false;
    s_criticalSection_.release();
    s_hHook_LL_ = 0;
    return true;
}


/// モード切替キーのコードを設定.
///
void CDiaKbdMouseHook_Impl::setModeKeyTbl(unsigned vkMode, CDiaKbdMouseHook_ConvKeyTbl const& tbl) {
    assert(0 < vkMode && vkMode < 256);
    if (vkMode == 0)
        return;
    CCriticalSectionLock    lock(s_criticalSection_);
    s_uModeKey_ = vkMode;
 #if 1 // _MSC_VER == 1500 && defined(NDEBUG) && defined(_WIN64)
    // vc9 x64 Release で memcpy がリンクできないと宣われるので無理やり回避.
    unsigned* s = (unsigned*)&tbl;
    unsigned* d = (unsigned*)&s_convKeys_;
    unsigned* e = d + sizeof(s_convKeys_)/sizeof(unsigned);
    while (d < e)
        *d++ = *s++;
 #else
    s_convKeys_ = tbl;
 #endif
}


/** マウス化するキー情報をボタン化したものを取得.
 */
unsigned CDiaKbdMouseHook_Impl::mouseButton() {
    CCriticalSectionLock    lock(s_criticalSection_);
    // 拡張キー操作中にWin+L でロック画面に移り戻ると内部状態不正でマウス移動が暴発することがあるので
    // 被害軽減のためタイマーを用意してクリア.
    if (s_iMouseButtonLife_) {
        if (--s_iMouseButtonLife_ <= 0) {
            s_iMouseButtonLife_ = 0;
            s_uMouseButton_     = 0;
        }
    }
    return s_uMouseButton_;
}


/// マウス向けボタンを設定
///
inline bool CDiaKbdMouseHook_Impl::setMouseButton(unsigned btn, bool sw)
{
    //CCriticalSectionLock    lock(s_criticalSection_);
    if (sw)
        s_uMouseButton_ |= btn;
    else
        s_uMouseButton_ &= ~btn;
    return true;
}


/** WH_KEYBOARD_LL で呼ばれる関数.
 */
LRESULT CALLBACK CDiaKbdMouseHook_Impl::LowLevelKeyboardProc(int nCode, WPARAM wparam, LPARAM lparam)
{
    KBDLLHOOKSTRUCT*    pInfo = (KBDLLHOOKSTRUCT*)lparam;

    // DEBUGPRINTF("* %02x %04x {%02x %02x %02x %d %08x}\n", nCode, wparam, pInfo->vkCode, pInfo->scanCode, pInfo->flags, pInfo->time, pInfo->dwExtraInfo );

    if (nCode >= 0 && pInfo->dwExtraInfo != DiaKbdMouseHook_EXTRAINFO) {    // 自身が生成したキーでないなら有効として.
        CCriticalSectionLock    lock(s_criticalSection_);
        s_iMouseButtonLife_ = 1000;
        if (s_bConvModeStat_ == 0) {    // 拡張シフトは、APPSが押されている間のみ有効.
            clearStat();
        }
        bool sw = false;
        switch (wparam) {
        case WM_KEYDOWN:        // キーが押されたら.
        case WM_SYSKEYDOWN:
            sw = true;          // on状態に.
            //[[fallthrough]];
        case WM_KEYUP:          // キーが放されたら.
        case WM_SYSKEYUP:       // off状態に.
            // 所定のキーなら横取りして、情報を控え、他の動作に反応しないようにして返る.
            if (keyDownUp( sw, pInfo->vkCode ))
                return true;
            break;

        default:
            ;
        }
    }
    return ::CallNextHookEx(s_hHook_LL_, nCode, wparam, lparam);
}


/// 所定のキーなら横取りして、情報を控え、他の動作に反応しないようにして返る.
///
bool CDiaKbdMouseHook_Impl::keyDownUp(bool sw, unsigned vkCode)
{
    // DEBUGPRINTF("* %02x %d\n", vkCode, sw );

 #ifdef DIAKBDMOUSEHOOK_USE_EX_SHIFT
    if (s_bExShift_) {  // 拡張シフト中は、カーソル移動のみ有効.
        const CDiaKbdMouseHook_ConvKey::COne&   rOne = s_convKeys_[vkCode].oneKey_[ s_bTwoStStatQ_ ];
        unsigned vk = rOne.u8VkCode_;
        switch (vk) {
        case VK_LEFT:   case VK_RIGHT:
        case VK_UP:     case VK_DOWN:
        case VK_NEXT:   case VK_PRIOR:
        case VK_HOME:   case VK_END:
        case VK_RSHIFT: case VK_LSHIFT:
        case VK_RCONTROL:case VK_LCONTROL:
        case 0xF0:
            break;

        default:    // カーソル以外のキーが押されたら.
            if (vkCode != VK_LSHIFT && vkCode != VK_RSHIFT && vkCode != VK_RCONTROL && vkCode != VK_LCONTROL) {
                // DEBUGPRINTF("ExShift %#x %#x\n", vkCode, vk);
                clearExShift();
            }
            break;
        }
    }
 #endif

    // モード切替キーだったら、全くWinのデフォルト動作をさせない
    if (vkCode == s_uModeKey_ && s_uModeKey_ != 0) {
      #if 0 // SHIFT+APPS をCapsLockにしている場合... 微妙な判定でいやな状態ありそうなのでやめ(右WIN+TABに変更)
        if (s_bShiftStat_) {            // シフトが押されてたら、CapsLock扱い
            sendKey(1, sw?0:KEYEVENTF_KEYUP, VK_CAPITAL);

        } else
      #endif
        {
            s_bConvModeStat_ = sw;

            if (sw) {   // CapsLock もどきのフリをする
             #if 0
                //INPUT input   = { INPUT_KEYBOARD, { 0xF0, 0, 0, 0, 0, 0, }};
                INPUT input = { INPUT_KEYBOARD, { 0xF0, 0, 0, 0, DiaKbdMouseHook_EXTRAINFO, }};
                ::SendInput(1, &input, sizeof(INPUT));
             #endif
            } else {
                clearStat();
            }
        }
        return true;
    } else
    if (/*s_bConvModeStat_ == 0 &&*/ (vkCode == VK_SHIFT || vkCode == VK_RSHIFT || vkCode == VK_LSHIFT)) {
        // Shiftキーの状態設定
        s_bShiftStat_ = sw;
        //return true;
    } else
    if (/*s_bConvModeStat_ == 0 &&*/ (vkCode == VK_CONTROL || vkCode == VK_RCONTROL || vkCode == VK_LCONTROL)) {
        // Ctrlキーの状態設定
        s_bCtrlStat_  = sw;
        //return true;
    } else
    if (vkCode == 0xF0 || vkCode == 0xF2) {
        // IME 対策でスルーしとく...
    } else {
        if (s_bConvModeStat_) {
            // この場でキーを変換してしまう
            sendConvKey(sw, vkCode);
            // モード切替キーが押されている間は、他のキーもWinのデフォルト動作をさせちゃ駄目
            return true;
        }
    }
    return false;
}


/// APPS+で入力されたキーを変換してSendInputする
///
void CDiaKbdMouseHook_Impl::sendConvKey(bool sw, unsigned uKey )
{
    typedef CDiaKbdMouseHook_ConvKey    CConvKey;
    if (uKey >= CDiaKbdMouseHook_Impl::VK_NUM) {
        assert(uKey < CDiaKbdMouseHook_Impl::VK_NUM);
        return;
    }
    const CConvKey::COne&   rOne = s_convKeys_[uKey].oneKey_[ s_bTwoStStatQ_ ];

    switch (rOne.u8Mode_) {
    case CConvKey::MD_NONE:     // 変換無しのとき
        if (uKey != s_uModeKey_) {
            s_bTwoStStatQ_ = 0;
        }
        break;

    case CConvKey::MD_USE:      // 変換を行うキーの場合
    case CConvKey::MD_CTRL:
    case CConvKey::MD_SHIFT:
    case CConvKey::MD_CTRLSHIFT:
        if (s_bDiaMouse_) {     // 強制的にダイアモンドカーソルでマウスを動かす
            makeMouseButton(sw, rOne.u8VkCode_);
            break;
        }
        //[[fallthourgh]]
    case CConvKey::MD_DIRECT:
        if (sw) {               // キーDOWN
            sendKey(rOne.u8Mode_, 0, rOne.u8VkCode_);
        } else {                // キーUP
            sendKey(rOne.u8Mode_, KEYEVENTF_KEYUP, rOne.u8VkCode_);
            s_bTwoStStatQ_ = 0;
        }
        break;

    case CConvKey::MD_2ST_Q:    // 2ストロークキーのトリガーキーだったら
        if (sw)                 // 押したときのみ
            s_bTwoStStatQ_ = 1;
        break;

 #ifdef DIAKBDMOUSEHOOK_USE_EX_SHIFT
    case CConvKey::MD_EX_SHIFT:
        if (sw == 0) {
            if (s_bExShift_) {
                clearExShift();
            } else {
                s_bExShift_ = true;
                sendKey(1, 0, VK_LSHIFT);
                //x OutputDebugString(s_bExShift_?"ex in\n":"ex out\n");
            }
        }
        break;
 #endif

    case CConvKey::MD_MOUSE:
        if (sw == 0 && rOne.u8VkCode_ == 0xff) {    // 手抜きでリリース時でチェック
            s_bDiaMouse_ = !s_bDiaMouse_;           // ダイアモンドカーソルでマウス移動するかどうかを切替
        } else {
            unsigned vk = rOne.u8VkCode_;
            makeMouseButton(sw, vk);
        }
        break;

    default:
        assert(1);
    }
}


/// ボタン状態を全てクリア(キーテーブル変更等の時のため)
///
void CDiaKbdMouseHook_Impl::clearStat()
{
    s_bConvModeStat_    = false;
    s_bTwoStStatQ_      = false;
    s_bShiftStat_       = false;
    s_bCtrlStat_        = false;
    //s_bDiaMouse_      = false;
    s_uMouseButton_     = 0;        // マウス情報クリア
    s_iMouseButtonLife_ = 0;
 #ifdef DIAKBDMOUSEHOOK_USE_EX_SHIFT
    clearExShift();
 #endif
}


#ifdef DIAKBDMOUSEHOOK_USE_EX_SHIFT
/// 拡張シフト状態をクリア
///
void CDiaKbdMouseHook_Impl::clearExShift()
{
    if (s_bExShift_) {
        //X s_uMouseButton_ = 0;        // マウス情報クリア
        s_bExShift_     = false;
        sendKey(1, KEYEVENTF_KEYUP, VK_LSHIFT);
      #if 0
        // 拡張シフトの終わりの合図として0xF0(CapsLock)が放されたことにする
        INPUT input = { INPUT_KEYBOARD, { 0xF0, 0, KEYEVENTF_KEYUP, 0, DiaKbdMouseHook_EXTRAINFO, }};
        ::SendInput(1, &input, sizeof(INPUT));
      #endif
    }
}
#endif


/// SendInputする
///
void CDiaKbdMouseHook_Impl::sendKey(int mode, unsigned uFlags, unsigned uVk )
{
    typedef CDiaKbdMouseHook_ConvKey    CConvKey;
    bool bCtrl  = !s_bCtrlStat_ && (mode == CConvKey::MD_CTRL  || mode == CConvKey::MD_CTRLSHIFT);
 #ifdef DIAKBDMOUSEHOOK_USE_EX_SHIFT
    bool bShift = !(s_bShiftStat_|s_bExShift_) && (mode == CConvKey::MD_SHIFT || mode == CConvKey::MD_CTRLSHIFT);
 #else
    bool bShift = !s_bShiftStat_ && (mode == CConvKey::MD_SHIFT || mode == CConvKey::MD_CTRLSHIFT);
 #endif

    INPUT       input[8];
    unsigned    n = 0;
    if (bCtrl)  // CTRLが押されたことにする
        setInputParam( input[n++], 0, VK_LCONTROL);
    if (bShift) // Shiftが押されたことにする
        setInputParam( input[n++], 0, VK_LSHIFT  );

    setInputParam( input[n++], uFlags, uVk );

    if (bCtrl)  // CTRLが放されたことにする
        setInputParam( input[n++], KEYEVENTF_KEYUP, VK_LCONTROL);
    if (bShift) // Shiftが放されたことにする
        setInputParam( input[n++], KEYEVENTF_KEYUP, VK_LSHIFT  );

    ::SendInput(n, &input[0], sizeof(INPUT));

    //x DEBUGPRINTF("sendKey %#x %#x\n", uFlags, uVk);
}


/// SendInputのパラメータをキーボード向けに設定
///
void CDiaKbdMouseHook_Impl::setInputParam(INPUT& rInput, unsigned uFlags, unsigned uVk )
{
    rInput.type             = INPUT_KEYBOARD;
    rInput.ki.wVk           = WORD(uVk);
    rInput.ki.wScan         = WORD( ::MapVirtualKey(uVk, 0) );
    rInput.ki.time          = 0;
    rInput.ki.dwExtraInfo   = DWORD(DiaKbdMouseHook_EXTRAINFO);
    rInput.ki.dwFlags       = uFlags | KEYEVENTF_EXTENDEDKEY;
}


/// マウス操作用のボタンを生成
///
bool CDiaKbdMouseHook_Impl::makeMouseButton( bool sw, unsigned uVk )
{
    setMouseButton(DIAKBDMOUSE_MOUSE_SPEEDUP    , s_bShiftStat_);
    setMouseButton(DIAKBDMOUSE_MOUSE_SPEEDCHG   , s_bCtrlStat_);
    switch (uVk) {
    case VK_LEFT:       return setMouseButton(DIAKBDMOUSE_MOUSE_LEFT        , sw);
    case VK_UP:         return setMouseButton(DIAKBDMOUSE_MOUSE_UP          , sw);
    case VK_RIGHT:      return setMouseButton(DIAKBDMOUSE_MOUSE_RIGHT       , sw);
    case VK_DOWN:       return setMouseButton(DIAKBDMOUSE_MOUSE_DOWN        , sw);
    case VK_LBUTTON:    return setMouseButton(DIAKBDMOUSE_MOUSE_LBUTTON     , sw);
    case VK_RBUTTON:    return setMouseButton(DIAKBDMOUSE_MOUSE_RBUTTON     , sw);
    case VK_MBUTTON:    return setMouseButton(DIAKBDMOUSE_MOUSE_MBUTTON     , sw);
    case VK_XBUTTON1:   return setMouseButton(DIAKBDMOUSE_MOUSE_XBUTTON1    , sw);
    case VK_XBUTTON2:   return setMouseButton(DIAKBDMOUSE_MOUSE_XBUTTON2    , sw);
    case VK_PRIOR:      return setMouseButton(DIAKBDMOUSE_MOUSE_WHEEL1      , sw);
    case VK_NEXT:       return setMouseButton(DIAKBDMOUSE_MOUSE_WHEEL2      , sw);
//  case VK_LCONTROL:   return setMouseButton(DIAKBDMOUSE_MOUSE_SPEEDCHG    , sw);
//  case VK_RCONTROL:   return setMouseButton(DIAKBDMOUSE_MOUSE_SPEEDCHG    , sw);
    default:
        ;
    }
    return false;
}
