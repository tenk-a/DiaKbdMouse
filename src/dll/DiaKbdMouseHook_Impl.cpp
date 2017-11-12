/**
 *  @file   DiaKbdMouseHook_Impl.cpp
 *  @brief  �EWIN+��p�����_�C�A�����h�J�[�\�����삷�邽�߂̃L�[�̃t�b�N������
 *  @auther Masashi KITAMURA
 *  @date   2006
 *  @note
 *      �t���[�\�[�X
 */

#include "stdafx.h"
#include <windows.h>
#include <functional>
#include "DiaKbdMouseHook_Impl.h"
#include "DiaKbdMouseHook.h"
#include "../cmn/DebugPrintf.h"

#ifdef NDEBUG   // ���s���̎g�p�����������炷
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

bool        CDiaKbdMouseHook_Impl::s_bDiaMouse_         = 0;
bool        CDiaKbdMouseHook_Impl::s_bConvModeStat_     = 0;
bool        CDiaKbdMouseHook_Impl::s_bTwoStStatQ_       = 0;
bool        CDiaKbdMouseHook_Impl::s_bShiftStat_        = 0;
bool        CDiaKbdMouseHook_Impl::s_bCtrlStat_         = 0;

#ifdef DIAKBDMOUSEHOOK_USE_EX_SHIFT
bool        CDiaKbdMouseHook_Impl::s_bExShift_          = 0;
#endif
#ifdef USE_LWINKEY
bool        CDiaKbdMouseHook_Impl::s_bLWinStat_         = 0;
bool        CDiaKbdMouseHook_Impl::s_bLWinModeSw_       = 1;
#endif



/** �t�b�N��ݒ�
 */
bool CDiaKbdMouseHook_Impl::install(int keycode, CDiaKbdMouseHook_ConvKeyTbl const& tbl)
{
    if (s_hHook_LL_ == 0) {
        s_criticalSection_.create();
        setModeKeyTbl(keycode, tbl);
        s_hHook_LL_  = ::SetWindowsHookEx(WH_KEYBOARD_LL, CDiaKbdMouseHook_Impl::LowLevelKeyboardProc, s_hInst_, 0);
        if (s_hHook_LL_ == NULL) {
            return false;
        }
    }
    return true;
}



/** �t�b�N������
 */
bool CDiaKbdMouseHook_Impl::uninstall()
{
    if (s_hHook_LL_ && UnhookWindowsHookEx(s_hHook_LL_) == 0)
        return false;
    s_criticalSection_.release();
    s_hHook_LL_ = 0;
    return true;
}


/// ���[�h�ؑփL�[�̃R�[�h��ݒ�
void CDiaKbdMouseHook_Impl::setModeKeyTbl(unsigned vkMode, CDiaKbdMouseHook_ConvKeyTbl const& tbl) {
    assert(0 < vkMode && vkMode < 256);
    if (vkMode == 0)
        return;
    CCriticalSectionLock    lock(s_criticalSection_);
    s_uModeKey_ = vkMode;
 #if 1 // _MSC_VER == 1500 && defined(NDEBUG) && defined(_WIN64)
    // vc9 x64 Release �� memcpy �������N�ł��Ȃ��Ɛ����̂Ŗ��������
    unsigned* s = (unsigned*)&tbl;
    unsigned* d = (unsigned*)&s_convKeys_;
    unsigned* e = d + sizeof(s_convKeys_)/sizeof(unsigned);
    while (d < e)
        *d++ = *s++;
 #else
    s_convKeys_ = tbl;
 #endif
}


/// �}�E�X������L�[�����{�^�����������̂��擾
unsigned CDiaKbdMouseHook_Impl::mouseButton() {
    CCriticalSectionLock    lock(s_criticalSection_);
    // �g���L�[���쒆��Win+L �Ń��b�N��ʂɈڂ�߂�Ɠ�����ԕs���Ń}�E�X�ړ����\�����邱�Ƃ�����̂�
    // ��Q�y���̂��߃^�C�}�[��p�ӂ��ăN���A.
    if (s_iMouseButtonLife_) {
        if (--s_iMouseButtonLife_ <= 0) {
            s_iMouseButtonLife_ = 0;
            s_uMouseButton_     = 0;
        }
    }
    return s_uMouseButton_;
}


#ifdef USE_LWINKEY
/** ��Win�L�[�ɂ��}�E�X��������邩�ǂ�����ݒ�.
 *  -1���Ɖ������Ȃ�(��Ԏ擾�p)�B
 *  @return ���O�̏�Ԃ�Ԃ�.
 */
bool CDiaKbdMouseHook_Impl::setLWinMode(int sw) {
    bool oldMode = s_bLWinModeSw_;
    if (sw >= 0) {
        clearStat();
        s_bLWinModeSw_ = (sw != 0);
    }
    return oldMode;
}
#endif



/// �}�E�X�����{�^����ݒ�
///
inline bool CDiaKbdMouseHook_Impl::setMouseButton(unsigned btn, bool sw)
{
    CCriticalSectionLock    lock(s_criticalSection_);
    if (sw)
        s_uMouseButton_ |= btn;
    else
        s_uMouseButton_ &= ~btn;
    return true;
}



/// WH_KEYBOARD_LL �ŌĂ΂��֐�
///
LRESULT CALLBACK CDiaKbdMouseHook_Impl::LowLevelKeyboardProc(int nCode, WPARAM wparam, LPARAM lparam)
{
    KBDLLHOOKSTRUCT*    pInfo = (KBDLLHOOKSTRUCT*)lparam;

    // DEBUGPRINTF("* %02x %04x {%02x %02x %02x %d %08x}\n", nCode, wparam, pInfo->vkCode, pInfo->scanCode, pInfo->flags, pInfo->time, pInfo->dwExtraInfo );

    {
        CCriticalSectionLock    lock(s_criticalSection_);
        s_iMouseButtonLife_ = 1000;
        if (s_bConvModeStat_ == 0) {    // �g���V�t�g�́AAPPS��������Ă���Ԃ̂ݗL��.
            clearStat();
        }

        if (nCode >= 0 && pInfo->dwExtraInfo != DiaKbdMouseHook_EXTRAINFO) {    // �L���ȏ�Ԃ�
            bool sw = false;
            switch (wparam) {
            case WM_KEYDOWN:        // �L�[�������ꂽ��
            case WM_SYSKEYDOWN:
                sw = true;          // on��Ԃ�
                // ����
            case WM_KEYUP:          // �L�[�������ꂽ��
            case WM_SYSKEYUP:       // off��Ԃ�
                // ����̃L�[�Ȃ牡��肵�āA�����T���A���̓���ɔ������Ȃ��悤�ɂ��ĕԂ�
                {
                    if (keyDownUp( sw, pInfo->vkCode ))
                        return true;
                }
                break;

            default:
                ;
            }
        }
    }
    return ::CallNextHookEx(s_hHook_LL_, nCode, wparam, lparam);
}



/// ����̃L�[�Ȃ牡��肵�āA�����T���A���̓���ɔ������Ȃ��悤�ɂ��ĕԂ�.
///
bool CDiaKbdMouseHook_Impl::keyDownUp(bool sw, unsigned vkCode)
{
    // DEBUGPRINTF("* %02x %d\n", vkCode, sw );

 #ifdef DIAKBDMOUSEHOOK_USE_EX_SHIFT
    if (s_bExShift_) {  // �g���V�t�g���́A�J�[�\���ړ��̂ݗL��
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

        default:    // �J�[�\���ȊO�̃L�[�������ꂽ��
            if (vkCode != VK_LSHIFT && vkCode != VK_RSHIFT && vkCode != VK_RCONTROL && vkCode != VK_LCONTROL) {
                // DEBUGPRINTF("ExShift %#x %#x\n", vkCode, vk);
                clearExShift();
            }
            break;
        }
    }
 #endif

    // ���[�h�ؑփL�[��������A�S��Win�̃f�t�H���g����������Ȃ�
    if (vkCode == s_uModeKey_ && s_uModeKey_ != 0) {
      #ifdef USE_LWINKEY
        s_bLWinStat_ = false;               // ��WIN�������Ȃ炻��̓L�����Z��
      #endif
      #if 0 // SHIFT+�EWIN ��CapsLock�ɂ��Ă���ꍇ... �����Ȕ���ł���ȏ�Ԃ��肻���Ȃ̂ł��(�EWIN+TAB�ɕύX)
        if (s_bShiftStat_) {            // �V�t�g��������Ă���ACapsLock����
            sendKey(1, sw?0:KEYEVENTF_KEYUP, VK_CAPITAL);

        } else
      #endif
        {
            s_bConvModeStat_ = sw;

            if (sw) {   // CapsLock ���ǂ��̃t��������
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
    } //else
    if (/*s_bConvModeStat_ == 0 &&*/ (vkCode == VK_SHIFT || vkCode == VK_RSHIFT || vkCode == VK_LSHIFT)) {
        // Shift�L�[�̏�Ԑݒ�
        s_bShiftStat_ = sw;
    } //else
    if (/*s_bConvModeStat_ == 0 &&*/ (vkCode == VK_CONTROL || vkCode == VK_RCONTROL || vkCode == VK_LCONTROL)) {
        // Ctrl�L�[�̏�Ԑݒ�
        s_bCtrlStat_  = sw;
    }
  #ifdef USE_LWINKEY
    else if (s_bConvModeStat_ == 0 && vkCode == VK_LWIN && s_bLWinModeSw_) {
        // ��Win�L�[�̏�Ԑݒ�
        setMouseButton(DIAKBDMOUSE_MOUSE_MODEKEY, sw);
        s_bLWinStat_     = sw;
        if (sw == 0)
            s_uMouseButton_ = 0;
    }
  #endif
    else if (vkCode == 0xF0 || vkCode == 0xF2) {
        // IME �΍�ŃX���[���Ƃ�...

    } else {
        if (s_bConvModeStat_) {
            // ���̏�ŃL�[��ϊ����Ă��܂�
            sendConvKey(sw, vkCode);
            // ���[�h�ؑփL�[��������Ă���Ԃ́A���̃L�[��Win�̃f�t�H���g�������������ʖ�
            return true;
        }
      #ifdef USE_LWINKEY
        if (s_bLWinStat_ && s_bLWinModeSw_) {
            switch (vkCode) {
            case '1': vkCode = VK_LBUTTON ; break;
            case '2': vkCode = VK_MBUTTON ; break;
            case '3': vkCode = VK_RBUTTON ; break;
            case '4': vkCode = VK_XBUTTON1; break;
            case '5': vkCode = VK_XBUTTON2; break;
            default: break;
            }
            makeMouseButton(sw, vkCode);
            return true;
        }
      #endif
    }
    return false;
}



/// �EWIN+�œ��͂��ꂽ�L�[��ϊ�����SendInput����
///
void CDiaKbdMouseHook_Impl::sendConvKey(bool sw, unsigned uKey )
{
    if (uKey >= CDiaKbdMouseHook_Impl::VK_NUM) {
        assert(uKey < CDiaKbdMouseHook_Impl::VK_NUM);
        return;
    }
    typedef CDiaKbdMouseHook_ConvKey    CConvKey;
    const CConvKey::COne&   rOne = s_convKeys_[uKey].oneKey_[ s_bTwoStStatQ_ ];

    switch (rOne.u8Mode_) {
    case CConvKey::MD_NONE:     // �ϊ������̂Ƃ�
        if (uKey != s_uModeKey_) {
            s_bTwoStStatQ_ = 0;
        }
        break;

    case CConvKey::MD_USE:      // �ϊ����s���L�[�̏ꍇ
    case CConvKey::MD_CTRL:
    case CConvKey::MD_SHIFT:
    case CConvKey::MD_CTRLSHIFT:
        if (s_bDiaMouse_) {     // �����I�Ƀ_�C�A�����h�J�[�\���Ń}�E�X�𓮂���
            makeMouseButton(sw, rOne.u8VkCode_);
            break;
        }
        //[[fallthourgh]]
    case CConvKey::MD_DIRECT:
        if (sw) {                       // �L�[DOWN
            sendKey(rOne.u8Mode_, 0, rOne.u8VkCode_);
        } else {                        // �L�[UP
            sendKey(rOne.u8Mode_, KEYEVENTF_KEYUP  , rOne.u8VkCode_);
            s_bTwoStStatQ_ = 0;
        }
        break;

    case CConvKey::MD_2ST_Q:    // 2�X�g���[�N�L�[�̃g���K�[�L�[��������
        if (sw)                 // �������Ƃ��̂�
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
        if (sw == 0 && rOne.u8VkCode_ == 0xff) {    // �蔲���Ń����[�X���Ń`�F�b�N
            s_bDiaMouse_ ^= 1;                      // �_�C�A�����h�J�[�\���Ń}�E�X�ړ����邩�ǂ�����ؑ�
        } else {
            unsigned vk = rOne.u8VkCode_;
            makeMouseButton(sw, vk);
        }
        break;

    default:
        assert(1);
    }
}


/** �{�^����Ԃ�S�ăN���A(�L�[�e�[�u���ύX���̎��̂���)
 */
void CDiaKbdMouseHook_Impl::clearStat()
{
    s_bConvModeStat_    = 0;
    s_bTwoStStatQ_      = 0;
    s_bShiftStat_       = 0;
    s_bCtrlStat_        = 0;
    //s_bDiaMouse_      = 0;
    s_uMouseButton_     = 0;        // �}�E�X���N���A
    s_iMouseButtonLife_ = 0;
 #ifdef USE_LWINKEY
    s_bLWinStat_        = 0;
 #endif
 #ifdef DIAKBDMOUSEHOOK_USE_EX_SHIFT
    clearExShift();
 #endif
}


#ifdef DIAKBDMOUSEHOOK_USE_EX_SHIFT
/// �g���V�t�g��Ԃ��N���A
///
void CDiaKbdMouseHook_Impl::clearExShift()
{
    if (s_bExShift_) {
        //X s_uMouseButton_ = 0;        // �}�E�X���N���A
        s_bExShift_     = false;
        sendKey(1, KEYEVENTF_KEYUP, VK_LSHIFT);
      #if 0
        // �g���V�t�g�̏I���̍��}�Ƃ���0xF0(CapsLock)�������ꂽ���Ƃɂ���
        INPUT input = { INPUT_KEYBOARD, { 0xF0, 0, KEYEVENTF_KEYUP, 0, DiaKbdMouseHook_EXTRAINFO, }};
        ::SendInput(1, &input, sizeof(INPUT));
      #endif
    }
}
#endif


/// SendInput����
///
void CDiaKbdMouseHook_Impl::sendKey(int mode, unsigned uFlags, unsigned uVk )
{
    typedef CDiaKbdMouseHook_ConvKey    CConvKey;
    INPUT       input[8];
    unsigned    n = 0;
    bool        bCtrl  = (s_bCtrlStat_ == 0) && (mode == CConvKey::MD_CTRL  || mode == CConvKey::MD_CTRLSHIFT);
 #ifdef DIAKBDMOUSEHOOK_USE_EX_SHIFT
    bool        bShift = ((s_bShiftStat_|s_bExShift_) == 0) && (mode == CConvKey::MD_SHIFT || mode == CConvKey::MD_CTRLSHIFT);
 #else
    bool        bShift = (s_bShiftStat_ == 0) && (mode == CConvKey::MD_SHIFT || mode == CConvKey::MD_CTRLSHIFT);
 #endif

    if (bCtrl)  // CTRL�������ꂽ���Ƃɂ���
        setInputParam( input[n++], 0, VK_LCONTROL);
    if (bShift) // Shift�������ꂽ���Ƃɂ���
        setInputParam( input[n++], 0, VK_LSHIFT  );

    setInputParam( input[n++], uFlags, uVk );

    if (bCtrl)  // CTRL�������ꂽ���Ƃɂ���
        setInputParam( input[n++], KEYEVENTF_KEYUP, VK_LCONTROL);
    if (bShift) // Shift�������ꂽ���Ƃɂ���
        setInputParam( input[n++], KEYEVENTF_KEYUP, VK_LSHIFT  );

    ::SendInput(n, &input[0], sizeof(INPUT));

    //x DEBUGPRINTF("sendKey %#x %#x\n", uFlags, uVk);
}


/// SendInput�̃p�����[�^���L�[�{�[�h�����ɐݒ�
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



/// �}�E�X����p�̃{�^���𐶐�
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
