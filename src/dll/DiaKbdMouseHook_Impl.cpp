/**
 *  @file   DiaKbdMouseHook_Impl.cpp
 *  @brief  APPS+を用いたダイアモンドカーソル操作するためのキーのフック側処理
 *  @auther Masashi KITAMURA
 *  @date   2006
 *  @license Boost Software License Version 1.0
 */

#include "stdafx.h"
#include <windows.h>
#include "DiaKbdMouseHook_Impl.h"
#include "DiaKbdMouseHook.h"
#include "../cmn/DebugPrintf.h"
#include <stdarg.h>
#include <stdio.h>

// バグ調査用のログ出力.(毎打鍵でファイル出力).
#if !defined(DKM_DEBUG_LOG) || DKM_DEBUG_LOG == 0
#define dkmDbgPrintf(...)
#else
static void dkmDbgPrintf(char const* fmt, ...)
{
 #if DKM_DEBUG_LOG == 1
    static FILE* s_fp = NULL;
    if (!s_fp)
        s_fp = fopen("dbg.log", "w");
    if (s_fp) {
        va_list args;
        va_start(args, fmt);
        vfprintf(s_fp, fmt, args);
        va_end(args);
        fflush(s_fp);
    }
 #elif DKM_DEBUG_LOG == 2
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    wvsprintfA(buf, fmt, args);
    va_end(args);
    buf[sizeof(buf) - 1] = 0;
    OutputDebugStringA(buf);
 #else
    (void)fmt;
 #endif
}
#endif

static int dkmKeyDownAsync(int vk)
{
    return (::GetAsyncKeyState(vk) & 0x8000) ? 1 : 0;
}

#if 0 && defined(_MSC_VER)  // 実行時の使用メモリを減らす.
//#pragma comment(linker, "/opt:nowin98")
//#pragma comment(linker, "/ignore:4078")
#pragma comment(linker, "/entry:\"DllMain\"")
#pragma comment(linker, "/nodefaultlib:\"libc.lib\"")
#pragma comment(linker, "/merge:.rdata=.text")
#pragma comment(linker, "/base:\"0x18000000\"")
#endif


#ifdef _MSC_VER
#pragma comment(linker, "/section:DIACURSO,rws")
#pragma data_seg("DIACURSO")
#endif
volatile HHOOK  CDiaKbdMouseHook_Impl::s_hHook_LL_  = 0;
#ifdef _MSC_VER
#pragma data_seg()
#endif


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
bool        CDiaKbdMouseHook_Impl::s_bSentKeyDown_[CDiaKbdMouseHook_Impl::VK_NUM];

bool        CDiaKbdMouseHook_Impl::s_bPhysModDown_[CDiaKbdMouseHook_Impl::WATCHDOG_MOD_NUM];
DWORD       CDiaKbdMouseHook_Impl::s_physModDownTick_[CDiaKbdMouseHook_Impl::WATCHDOG_MOD_NUM];
DWORD       CDiaKbdMouseHook_Impl::s_orphanSinceTick_[CDiaKbdMouseHook_Impl::WATCHDOG_MOD_NUM];
DWORD       CDiaKbdMouseHook_Impl::s_watchdogLastTick_ = 0;

// 修飾キー固着監視のパラメータ. (コンパイルオプションで上書き可能)
#ifndef DKM_WATCHDOG_POLL_MSEC
#define DKM_WATCHDOG_POLL_MSEC      250     ///< 監視の実行間隔.
#endif
#ifndef DKM_WATCHDOG_ORPHAN_MSEC
#define DKM_WATCHDOG_ORPHAN_MSEC    2000    ///< 論理押下だけが残った状態をこの時間見たら解除.
#endif
#ifndef DKM_WATCHDOG_LONGHOLD_MSEC
#define DKM_WATCHDOG_LONGHOLD_MSEC  90000   ///< 物理押下がこの時間続いたら切替器のラッチを疑い解除. 0で無効.
#endif

/// watchdog が監視する修飾キー(L/R別). ビット順は DIAKBDMOUSE_WMOD_* と一致させること.
static unsigned const s_watchdogModVkTbl_[CDiaKbdMouseHook_Impl::WATCHDOG_MOD_NUM] = {
    VK_LSHIFT, VK_RSHIFT, VK_LCONTROL, VK_RCONTROL,
    VK_LMENU,  VK_RMENU,  VK_LWIN,     VK_RWIN,
};

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
        initPhysModifierState();
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

    {
        CCriticalSectionLock    lock(s_criticalSection_);
        clearStat();
    }
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
    // 拡張キー操作中にWin+L でロック画面に移り戻ると内部状態不正でマウス移動が暴発することがあるので,
    // 被害軽減のためタイマーを用意してクリア.
    if (s_iMouseButtonLife_) {
        if (--s_iMouseButtonLife_ <= 0) {
            clearStat();
        }
    }
    return s_uMouseButton_;
}


/** 修飾キーの押下状態を解放.
 */
void CDiaKbdMouseHook_Impl::releaseModifierKeys()
{
    // フック設定中のみ内部状態をクリア. DLL_PROCESS_DETACH は uninstall() 後
    // (s_criticalSection_ 解放済み)に呼ばれるため、そこでロックするとUBになる.
    if (s_hHook_LL_) {
        CCriticalSectionLock    lock(s_criticalSection_);
        clearStat();
    }
    // Windows 側に残った論理修飾キー状態を解除する.
    sendModifierKeyUpByVk();
    sendModifierKeyUpByScanCode();
}


/** 修飾キーの KEYUP を仮想キー指定で送る. 汎用 VK_CONTROL 等も含める.
 *  USB切替器やKVM後に左右キーだけでは論理状態が戻らない環境があるため.
 */
void CDiaKbdMouseHook_Impl::sendModifierKeyUpByVk()
{
    static unsigned const modifierKeys[] = {
        VK_SHIFT,    VK_LSHIFT,   VK_RSHIFT,
        VK_CONTROL,  VK_LCONTROL, VK_RCONTROL,
        VK_MENU,     VK_LMENU,    VK_RMENU,
        VK_LWIN,     VK_RWIN,
    };
    enum { modifierKeys_size = sizeof(modifierKeys)/sizeof(modifierKeys[0]) };

    for (unsigned i = 0; i < modifierKeys_size; ++i) {
        INPUT input;
        setInputParam(input, KEYEVENTF_KEYUP, modifierKeys[i]);
        sendInputOne(input);
    }
}


/** 修飾キーの KEYUP をスキャンコード指定で送る.
 *  一部のKVM/USB切替器では HID の再接続後に VK 指定の KEYUP だけでは
 *  押下状態が解けないことがあるため, 物理キー位置に近い形式でも解除.
 */
void CDiaKbdMouseHook_Impl::sendModifierKeyUpByScanCode()
{
    struct SKey { WORD scan; DWORD flags; };
    static SKey const keys[] = {
        { 0x2a, 0 },                         // Left Shift
        { 0x36, 0 },                         // Right Shift
        { 0x1d, 0 },                         // Left Ctrl
        { 0x1d, KEYEVENTF_EXTENDEDKEY },     // Right Ctrl
        { 0x38, 0 },                         // Left Alt
        { 0x38, KEYEVENTF_EXTENDEDKEY },     // Right Alt
        { 0x5b, KEYEVENTF_EXTENDEDKEY },     // Left Win
        { 0x5c, KEYEVENTF_EXTENDEDKEY },     // Right Win
    };
    enum { keys_size = sizeof(keys)/sizeof(keys[0]) };

    for (unsigned i = 0; i < keys_size; ++i) {
        INPUT input;
        ZeroMemory(&input, sizeof(input));
        input.type           = INPUT_KEYBOARD;
        input.ki.wVk         = 0;
        input.ki.wScan       = keys[i].scan;
        input.ki.dwFlags     = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE | keys[i].flags;
        input.ki.time        = 0;
        input.ki.dwExtraInfo = DWORD(DiaKbdMouseHook_EXTRAINFO);
        sendInputOne(input);
    }
}


/// 監視対象の修飾キーなら 0～WATCHDOG_MOD_NUM-1 のインデックス、対象外なら -1.
///
int CDiaKbdMouseHook_Impl::modifierIndexOfVk(unsigned uVk)
{
    for (int i = 0; i < WATCHDOG_MOD_NUM; ++i) {
        if (s_watchdogModVkTbl_[i] == uVk)
            return i;
    }
    return -1;
}


/// フックが見た実イベントから修飾キーの物理押下状態を記録.
/// ※ s_criticalSection_ ロック中に呼ぶこと.
///
void CDiaKbdMouseHook_Impl::recordPhysModifier(unsigned uVk, bool sw)
{
    int i = modifierIndexOfVk(uVk);
    if (i < 0)
        return;
    if (s_bPhysModDown_[i] != sw) {
        s_bPhysModDown_[i] = sw;
        if (sw)
            s_physModDownTick_[i] = ::GetTickCount();
    }
    s_orphanSinceTick_[i] = 0;  // 実イベントを確認できたので固着疑いをリセット.
}


/// 修飾キー監視状態の初期化. フック開始時点の論理状態を物理状態の初期値とみなす.
/// (Shiftを押しながら起動した場合などに誤射しないため)
///
void CDiaKbdMouseHook_Impl::initPhysModifierState()
{
    DWORD now = ::GetTickCount();
    for (int i = 0; i < WATCHDOG_MOD_NUM; ++i) {
        s_bPhysModDown_[i]     = (::GetAsyncKeyState((int)s_watchdogModVkTbl_[i]) & 0x8000) != 0;
        s_physModDownTick_[i]  = now;
        s_orphanSinceTick_[i]  = 0;
    }
    s_watchdogLastTick_ = now;
}


/** 修飾キー固着の監視・自動解除. 別スレッドから定期的に呼ばれる.
 *
 *  検出する固着は2種類:
 *  1. 孤児状態: GetAsyncKeyState は押下なのに、フックは物理押下を確認して
 *     いない. (フックのタイムアウト素通りや他プロセスの注入残り等で
 *     論理状態だけが残ったケース) → DKM_WATCHDOG_ORPHAN_MSEC 継続で解除.
 *  2. 超長押し: 物理押下のまま異常に長時間経過. USB切替器/KVMの
 *     キーボードエミュレーションが修飾ビットをラッチしたまま KEYUP を
 *     送ってこないケース(この場合ユーザーがShiftを押し直しても
 *     ビット変化が起きずイベント自体が来ない)を想定.
 *     → DKM_WATCHDOG_LONGHOLD_MSEC 継続で解除. 本当に押し続けていた場合は
 *       次の実イベント(押し直しやリピート)で状態が復元されるので実害は軽微.
 *
 *  戻り値: 解除したキーの DIAKBDMOUSE_WMOD_* ビットマスク.
 */
unsigned CDiaKbdMouseHook_Impl::watchdog()
{
    if (s_hHook_LL_ == 0)   // フック無効中(解放後のCriticalSection使用を避ける)は何もしない.
        return 0;

    CCriticalSectionLock    lock(s_criticalSection_);

    DWORD now = ::GetTickCount();
    if (now - s_watchdogLastTick_ < DKM_WATCHDOG_POLL_MSEC)
        return 0;
    s_watchdogLastTick_ = now;

    unsigned releasedMask = 0;
    for (int i = 0; i < WATCHDOG_MOD_NUM; ++i) {
        unsigned vk = s_watchdogModVkTbl_[i];
        bool asyncDown = (::GetAsyncKeyState((int)vk) & 0x8000) != 0;
        if (!asyncDown) {
            s_orphanSinceTick_[i] = 0;
            continue;
        }
        if (vk < VK_NUM && s_bSentKeyDown_[vk]) {
            // 変換で自前注入して押下維持中のキーは固着ではない(解放はreleaseSentKeys側の担当).
            s_orphanSinceTick_[i] = 0;
            continue;
        }
     #ifdef DIAKBDMOUSEHOOK_USE_EX_SHIFT
        if (s_bExShift_ && vk == VK_LSHIFT) {   // 拡張シフトで意図的に押下維持中.
            s_orphanSinceTick_[i] = 0;
            continue;
        }
     #endif
        if (s_bPhysModDown_[i]) {
            s_orphanSinceTick_[i] = 0;
         #if DKM_WATCHDOG_LONGHOLD_MSEC > 0
            if (now - s_physModDownTick_[i] >= DKM_WATCHDOG_LONGHOLD_MSEC) {
                dkmDbgPrintf("DKM watchdog longhold release vk=%02x\n", vk);
                INPUT input;
                setInputParam(input, KEYEVENTF_KEYUP, vk);
                if (sendInputOne(input)) {
                    releasedMask |= 1u << i;
                    s_physModDownTick_[i] = now;    // 実押下継続なら次の実イベントで復元される.
                }
            }
         #endif
        } else {
            if (s_orphanSinceTick_[i] == 0) {
                s_orphanSinceTick_[i] = now ? now : 1;
            } else if (now - s_orphanSinceTick_[i] >= DKM_WATCHDOG_ORPHAN_MSEC) {
                dkmDbgPrintf("DKM watchdog orphan release vk=%02x\n", vk);
                INPUT input;
                setInputParam(input, KEYEVENTF_KEYUP, vk);
                if (sendInputOne(input))
                    releasedMask |= 1u << i;
                s_orphanSinceTick_[i] = 0;
            }
        }
    }
    return releasedMask;
}


/// マウス向けボタンを設定.
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

    if (nCode >= 0 && pInfo) {
        dkmDbgPrintf(
            "DKM LLK enter nc=%d wp=%04x vk=%02x scan=%02x flags=%02x extra=%p mode=%d ctrl=%d shift=%d asyncC=%d asyncLC=%d asyncRC=%d keyC=%04x keyLC=%04x keyRC=%04x\n",
            nCode,
            (unsigned)wparam,
            (unsigned)pInfo->vkCode,
            (unsigned)pInfo->scanCode,
            (unsigned)pInfo->flags,
            (void*)pInfo->dwExtraInfo,
            s_bConvModeStat_ ? 1 : 0,
            s_bCtrlStat_ ? 1 : 0,
            s_bShiftStat_ ? 1 : 0,
            dkmKeyDownAsync(VK_CONTROL),
            dkmKeyDownAsync(VK_LCONTROL),
            dkmKeyDownAsync(VK_RCONTROL),
            (unsigned)::GetKeyState(VK_CONTROL),
            (unsigned)::GetKeyState(VK_LCONTROL),
            (unsigned)::GetKeyState(VK_RCONTROL));
    }

    if (nCode >= 0 && pInfo->dwExtraInfo != DiaKbdMouseHook_EXTRAINFO) {    // 自身が生成したキーでないなら有効として.
        CCriticalSectionLock    lock(s_criticalSection_);
        s_iMouseButtonLife_ = 1000;
        if (s_bConvModeStat_ == 0) {    // 拡張シフトは、APPSが押されている間のみ有効.
            clearStat();
        }
        // watchdog用: フックまで届いた修飾キーイベントを「物理押下の真値」として記録.
        // (食べる/食べないに関わらず記録する. 自身の注入分はdwExtraInfoで除外済み)
        if (wparam == WM_KEYDOWN || wparam == WM_SYSKEYDOWN)
            recordPhysModifier(pInfo->vkCode, true);
        else if (wparam == WM_KEYUP || wparam == WM_SYSKEYUP)
            recordPhysModifier(pInfo->vkCode, false);
        bool sw = false;
        switch (wparam) {
        case WM_KEYDOWN:        // キーが押されたら.
        case WM_SYSKEYDOWN:
            sw = true;          // on状態に.
            //[[fallthrough]];
        case WM_KEYUP:          // キーが放されたら.
        case WM_SYSKEYUP:       // off状態に.
            {
                // 所定のキーなら横取りして、情報を控え、他の動作に反応しないようにして返る.
                bool eaten = keyDownUp( sw, pInfo->vkCode );
                dkmDbgPrintf(
                    "DKM LLK leave vk=%02x sw=%d eaten=%d mode=%d ctrl=%d shift=%d mouse=%08x\n",
                    (unsigned)pInfo->vkCode,
                    sw ? 1 : 0,
                    eaten ? 1 : 0,
                    s_bConvModeStat_ ? 1 : 0,
                    s_bCtrlStat_ ? 1 : 0,
                    s_bShiftStat_ ? 1 : 0,
                    s_uMouseButton_);
                if (eaten)
                    return true;
            }
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
    dkmDbgPrintf(
        "DKM keyDownUp enter vk=%02x sw=%d mode=%d ctrl=%d shift=%d two=%d sentVk=%d asyncC=%d asyncLC=%d asyncRC=%d\n",
        vkCode,
        sw ? 1 : 0,
        s_bConvModeStat_ ? 1 : 0,
        s_bCtrlStat_ ? 1 : 0,
        s_bShiftStat_ ? 1 : 0,
        s_bTwoStStatQ_ ? 1 : 0,
        (vkCode < VK_NUM && s_bSentKeyDown_[vkCode]) ? 1 : 0,
        dkmKeyDownAsync(VK_CONTROL),
        dkmKeyDownAsync(VK_LCONTROL),
        dkmKeyDownAsync(VK_RCONTROL));

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

    // モード切替キーだったら、全くWinのデフォルト動作をさせない.
    if (vkCode == s_uModeKey_ && s_uModeKey_ != 0) {
      #if 0 // SHIFT+APPS をCapsLockにしている場合... 微妙な判定でいやな状態ありそうなのでやめ(右WIN+TABに変更)
        if (s_bShiftStat_) {            // シフトが押されてたら、CapsLock扱い.
            sendKey(1, sw?0:KEYEVENTF_KEYUP, VK_CAPITAL);

        } else
      #endif
        {
            s_bConvModeStat_ = sw;
            dkmDbgPrintf("DKM modekey vk=%02x sw=%d -> mode=%d\n", vkCode, sw ? 1 : 0, s_bConvModeStat_ ? 1 : 0);

            if (sw) {   // CapsLock もどきのフリをする.
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
        // Shiftキーの状態設定.
        s_bShiftStat_ = sw;
        dkmDbgPrintf("DKM shiftstat vk=%02x sw=%d\n", vkCode, sw ? 1 : 0);
        //return true;
    } else
    if (/*s_bConvModeStat_ == 0 &&*/ (vkCode == VK_CONTROL || vkCode == VK_RCONTROL || vkCode == VK_LCONTROL)) {
        // Ctrlキーの状態設定.
        s_bCtrlStat_  = sw;
        dkmDbgPrintf("DKM ctrlstat vk=%02x sw=%d\n", vkCode, sw ? 1 : 0);
        //return true;
    } else
    if (vkCode == 0xF0 || vkCode == 0xF2) {
        // IME 対策でスルーしとく...
    } else {
        if (s_bConvModeStat_) {
            // この場でキーを変換してしまう.
            dkmDbgPrintf("DKM conv vk=%02x sw=%d mode=%d ctrl=%d shift=%d\n", vkCode, sw ? 1 : 0, s_bConvModeStat_ ? 1 : 0, s_bCtrlStat_ ? 1 : 0, s_bShiftStat_ ? 1 : 0);
            sendConvKey(sw, vkCode);
            // モード切替キーが押されている間は、他のキーもWinのデフォルト動作をさせちゃ駄目.
            return true;
        }
    }
    return false;
}


/// APPS+で入力されたキーを変換してSendInput
///
void CDiaKbdMouseHook_Impl::sendConvKey(bool sw, unsigned uKey )
{
    typedef CDiaKbdMouseHook_ConvKey    CConvKey;
    dkmDbgPrintf("DKM sendConvKey enter key=%02x sw=%d two=%d diaMouse=%d\n", uKey, sw ? 1 : 0, s_bTwoStStatQ_ ? 1 : 0, s_bDiaMouse_ ? 1 : 0);
    if (uKey >= CDiaKbdMouseHook_Impl::VK_NUM) {
        assert(uKey < CDiaKbdMouseHook_Impl::VK_NUM);
        return;
    }
    const CConvKey::COne&   rOne = s_convKeys_[uKey].oneKey_[ s_bTwoStStatQ_ ];

    switch (rOne.u8Mode_) {
    case CConvKey::MD_NONE:     // 変換無しのとき.
        if (uKey != s_uModeKey_) {
            s_bTwoStStatQ_ = 0;
        }
        break;

    case CConvKey::MD_USE:      // 変換を行うキーの場合.
    case CConvKey::MD_CTRL:
    case CConvKey::MD_SHIFT:
    case CConvKey::MD_CTRLSHIFT:
        if (s_bDiaMouse_) {     // 強制的にキーでマウスを動かす.
            makeMouseButton(sw, rOne.u8VkCode_);
            break;
        }
        //[[fallthourgh]]
    case CConvKey::MD_DIRECT:
        if (sw) {               // キーDOWN
            dkmDbgPrintf("DKM sendConvKey map key=%02x -> vk=%02x mode=%02x DOWN\n", uKey, rOne.u8VkCode_, rOne.u8Mode_);
            if (sendKey(rOne.u8Mode_, 0, rOne.u8VkCode_))
                setSentKeyDown(rOne.u8VkCode_, true);
        } else {                // キーUP
            dkmDbgPrintf("DKM sendConvKey map key=%02x -> vk=%02x mode=%02x UP\n", uKey, rOne.u8VkCode_, rOne.u8Mode_);
            if (sendKey(rOne.u8Mode_, KEYEVENTF_KEYUP, rOne.u8VkCode_))
                setSentKeyDown(rOne.u8VkCode_, false);
            s_bTwoStStatQ_ = 0;
        }
        break;

    case CConvKey::MD_2ST_Q:    // 2ストロークキーのトリガーキーだったら,
        if (sw)                 // 押したときのみ.
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
        if (sw == 0 && rOne.u8VkCode_ == 0xff) {    // 手抜きでリリース時でチェック.
            s_bDiaMouse_ = !s_bDiaMouse_;           // キーでマウス移動するかどうかを切替.
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
    dkmDbgPrintf("DKM clearStat before mode=%d ctrl=%d shift=%d two=%d mouse=%08x\n", s_bConvModeStat_ ? 1 : 0, s_bCtrlStat_ ? 1 : 0, s_bShiftStat_ ? 1 : 0, s_bTwoStStatQ_ ? 1 : 0, s_uMouseButton_);
    releaseSentKeys();
    s_bConvModeStat_    = false;
    s_bTwoStStatQ_      = false;
    s_bShiftStat_       = false;
    s_bCtrlStat_        = false;
    //s_bDiaMouse_      = false;
    s_uMouseButton_     = 0;        // マウス情報クリア.
    s_iMouseButtonLife_ = 0;
 #ifdef DIAKBDMOUSEHOOK_USE_EX_SHIFT
    clearExShift();
 #endif
}


#ifdef DIAKBDMOUSEHOOK_USE_EX_SHIFT
/// 拡張シフト状態をクリア.
///
void CDiaKbdMouseHook_Impl::clearExShift()
{
    if (s_bExShift_) {
        //X s_uMouseButton_ = 0;        // マウス情報クリア.
        s_bExShift_     = false;
        sendKey(1, KEYEVENTF_KEYUP, VK_LSHIFT);
      #if 0
        // 拡張シフトの終わりの合図として0xF0(CapsLock)が放されたことにする.
        INPUT input = { INPUT_KEYBOARD, { 0xF0, 0, KEYEVENTF_KEYUP, 0, DiaKbdMouseHook_EXTRAINFO, }};
        ::SendInput(1, &input, sizeof(INPUT));
      #endif
    }
}
#endif


/// SendInput
///
bool CDiaKbdMouseHook_Impl::sendKey(int mode, unsigned uFlags, unsigned uVk )
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
    if (bCtrl)  // CTRLが押されたことにする.
        setInputParam( input[n++], 0, VK_LCONTROL);
    if (bShift) // Shiftが押されたことにする.
        setInputParam( input[n++], 0, VK_LSHIFT  );

    unsigned const keyIndex = n;
    setInputParam( input[n++], uFlags, uVk );

    if (bCtrl)  // CTRLが放されたことにする.
        setInputParam( input[n++], KEYEVENTF_KEYUP, VK_LCONTROL);
    if (bShift) // Shiftが放されたことにする.
        setInputParam( input[n++], KEYEVENTF_KEYUP, VK_LSHIFT  );

    dkmDbgPrintf("DKM sendKey mode=%d flags=%04x vk=%02x bCtrl=%d bShift=%d n=%u\n", mode, uFlags, uVk, bCtrl ? 1 : 0, bShift ? 1 : 0, n);
    UINT sent = ::SendInput(n, &input[0], sizeof(INPUT));
    if (sent != n) {
        DWORD error = ::GetLastError();
        dkmDbgPrintf("DKM SendInput partial sent=%u/%u error=%lu\n", sent, n, (unsigned long)error);

        // SendInput は部分成功を返し得る。修飾キーDOWNまでしか投入されなかった
        // 場合でも押下状態を残さないよう、UPを単独の呼び出しで必ず再送する.
        if (bShift) {
            INPUT inputUp;
            setInputParam(inputUp, KEYEVENTF_KEYUP, VK_LSHIFT);
            sendInputOne(inputUp);
        }
        if (bCtrl) {
            INPUT inputUp;
            setInputParam(inputUp, KEYEVENTF_KEYUP, VK_LCONTROL);
            sendInputOne(inputUp);
        }
    }

    //x DEBUGPRINTF("sendKey %#x %#x\n", uFlags, uVk);
    return sent > keyIndex;
}


/// SendInputのパラメータをキーボード向けに設定.
///
void CDiaKbdMouseHook_Impl::setInputParam(INPUT& rInput, unsigned uFlags, unsigned uVk )
{
    ZeroMemory(&rInput, sizeof(rInput));
    rInput.type             = INPUT_KEYBOARD;
    rInput.ki.wVk           = WORD(uVk);
    rInput.ki.wScan         = WORD( ::MapVirtualKey(uVk, 0) );
    rInput.ki.time          = 0;
    rInput.ki.dwExtraInfo   = DWORD(DiaKbdMouseHook_EXTRAINFO);
    rInput.ki.dwFlags       = uFlags | (isExtendedKey(uVk) ? KEYEVENTF_EXTENDEDKEY : 0);
}


/// INPUTを1件だけ送信. 一括送信の部分成功で後続のKEYUPが欠落することを避ける.
///
bool CDiaKbdMouseHook_Impl::sendInputOne(INPUT& rInput)
{
    UINT sent = ::SendInput(1, &rInput, sizeof(INPUT));
    if (sent != 1) {
        DWORD error = ::GetLastError();
        dkmDbgPrintf("DKM SendInput one failed error=%lu\n", (unsigned long)error);
        return false;
    }
    return true;
}


/// KEYEVENTF_EXTENDEDKEY が必要なキーかを判定.
///
bool CDiaKbdMouseHook_Impl::isExtendedKey(unsigned uVk)
{
    switch (uVk) {
    case VK_RCONTROL:
    case VK_RMENU:
    case VK_INSERT:
    case VK_DELETE:
    case VK_HOME:
    case VK_END:
    case VK_PRIOR:
    case VK_NEXT:
    case VK_LEFT:
    case VK_UP:
    case VK_RIGHT:
    case VK_DOWN:
    case VK_NUMLOCK:
    case VK_CANCEL:
    case VK_SNAPSHOT:
    case VK_DIVIDE:
    case VK_LWIN:
    case VK_RWIN:
    case VK_APPS:
    case VK_BROWSER_BACK:
    case VK_BROWSER_FORWARD:
    case VK_BROWSER_REFRESH:
    case VK_BROWSER_STOP:
    case VK_BROWSER_SEARCH:
    case VK_BROWSER_FAVORITES:
    case VK_BROWSER_HOME:
    case VK_VOLUME_MUTE:
    case VK_VOLUME_DOWN:
    case VK_VOLUME_UP:
    case VK_MEDIA_NEXT_TRACK:
    case VK_MEDIA_PREV_TRACK:
    case VK_MEDIA_STOP:
    case VK_MEDIA_PLAY_PAUSE:
    case VK_LAUNCH_MAIL:
    case VK_LAUNCH_MEDIA_SELECT:
    case VK_LAUNCH_APP1:
    case VK_LAUNCH_APP2:
        return true;
    default:
        return false;
    }
}


/// SendInputで押したままのキーを記録.
///
void CDiaKbdMouseHook_Impl::setSentKeyDown(unsigned uVk, bool sw)
{
    if (uVk < VK_NUM)
        s_bSentKeyDown_[uVk] = sw;
}


/// SendInputで押したままのキーをすべて解放.
///
void CDiaKbdMouseHook_Impl::releaseSentKeys()
{
    dkmDbgPrintf("DKM releaseSentKeys enter\n");
    for (unsigned i = 0; i < VK_NUM; ++i) {
        if (s_bSentKeyDown_[i]) {
            INPUT input;
            setInputParam(input, KEYEVENTF_KEYUP, i);
            // 送信に失敗したキーは追跡状態を残し、次のclearStatで再試行する.
            if (sendInputOne(input))
                s_bSentKeyDown_[i] = false;
        }
    }
}


/// マウス操作用のボタンを生成.
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
    //case VK_LCONTROL: return setMouseButton(DIAKBDMOUSE_MOUSE_SPEEDCHG    , sw);
    //case VK_RCONTROL: return setMouseButton(DIAKBDMOUSE_MOUSE_SPEEDCHG    , sw);
    default:
        ;
    }
    return false;
}
