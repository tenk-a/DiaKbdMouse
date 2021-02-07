/**
 *  @file   DiaKbdMouseHook_ConvKey.h
 *  @brief  APPS+でのキー変換テーブル
 *  @auther Masashi KITAMURA
 *  @date   2006
 *  @note
 *      フリーソース
 */

#include "stdafx.h"
#include "DiaKbdMouseHook_Impl.h"

#if 1 // 定義ファイルから読み込むので、初期設定しないでおく.
/// キー変換テーブル.
CDiaKbdMouseHook_ConvKeyTbl CDiaKbdMouseHook_Impl::s_convKeys_;
#else

#define XX(a,b)         {{ a , b }}     // a:APPS+時  b:APPS+Q+時

#define CLS             CDiaKbdMouseHook_ConvKey
#define NOUSE           {CLS::MD_NONE     ,0}           // 使わない
#define K(k)            {CLS::MD_USE      ,k}           // kのみ
#define C(k)            {CLS::MD_CTRL     ,k}           // CTRL+k
#define S(k)            {CLS::MD_SHIFT    ,k}           // SHIFT+k
#define CS(k)           {CLS::MD_CTRLSHIFT,k}           // CTRL+SHIFT+k
#define TWOST           {CLS::MD_2ST_Q    ,1}           // 2ストロークキー(Q)用
#define EXSHIFT_SW      {CLS::MD_EX_SHIFT ,0}           // カーソル移動でシフト押し状態にするスイッチ
#define M(k)            {CLS::MD_MOUSE    ,k}           // マウス. 255 はマウスカーソルと通常カーソルの切替


/// キー変換テーブル
CDiaKbdMouseHook_ConvKeyTbl CDiaKbdMouseHook_Impl::s_convKeys_ = {{
    XX( NOUSE           , NOUSE            ),       // 0x00
    XX( NOUSE           , NOUSE            ),       // 0x01 LBUTTON
    XX( NOUSE           , NOUSE            ),       // 0x02 RBUTTON
    XX( NOUSE           , NOUSE            ),       // 0x03 CANCEL
    XX( NOUSE           , NOUSE            ),       // 0x04 MBUTTON
    XX( NOUSE           , NOUSE            ),       // 0x05 XBUTTON1
    XX( NOUSE           , NOUSE            ),       // 0x06 XBUTTON2
    XX( NOUSE           , NOUSE            ),       // 0x07
    XX( NOUSE           , NOUSE            ),       // 0x08 BACK
    XX( NOUSE           , NOUSE            ),       // 0x09 TAB
    XX( NOUSE           , NOUSE            ),       // 0x0A
    XX( NOUSE           , NOUSE            ),       // 0x0B
    XX( NOUSE           , NOUSE            ),       // 0x0C CLEAR
    XX( K(VK_KANJI)     , NOUSE            ),       // 0x0D RETURN
    XX( NOUSE           , NOUSE            ),       // 0x0E
    XX( NOUSE           , NOUSE            ),       // 0x0F
    XX( K(VK_SHIFT)     , NOUSE            ),       // 0x10 SHIFT
    XX( K(VK_CONTROL)   , NOUSE            ),       // 0x11 CONTROL
    XX( K(VK_MENU)      , NOUSE            ),       // 0x12 MENU
    XX( NOUSE           , NOUSE            ),       // 0x13 PAUSE
    XX( NOUSE           , NOUSE            ),       // 0x14 CAPITAL
    XX( NOUSE           , NOUSE            ),       // 0x15 KANA  HANGUL
    XX( NOUSE           , NOUSE            ),       // 0x16
    XX( NOUSE           , NOUSE            ),       // 0x17 JUNJA
    XX( NOUSE           , NOUSE            ),       // 0x18 FINAL
    XX( NOUSE           , NOUSE            ),       // 0x19 KANJI  HANJA
    XX( NOUSE           , NOUSE            ),       // 0x1A
    XX( K(VK_CAPITAL)   , NOUSE            ),       // 0x1B ESCAPE
    XX( NOUSE           , NOUSE            ),       // 0x1C CONVERT
    XX( NOUSE           , NOUSE            ),       // 0x1D NONCONVERT
    XX( NOUSE           , NOUSE            ),       // 0x1E ACCEPT
    XX( NOUSE           , NOUSE            ),       // 0x1F MODECHANGE
    XX( NOUSE           , NOUSE            ),       // 0x20 SPACE
    XX( M(VK_PRIOR)     , NOUSE            ),       // 0x21 PRIOR
    XX( M(VK_NEXT)      , NOUSE            ),       // 0x22 NEXT
    XX( NOUSE           , NOUSE            ),       // 0x23 END
    XX( NOUSE           , NOUSE            ),       // 0x24 HOME
    XX( M(VK_LEFT)      , NOUSE            ),       // 0x25 LEFT
    XX( M(VK_UP)        , NOUSE            ),       // 0x26 UP
    XX( M(VK_RIGHT)     , NOUSE            ),       // 0x27 RIGHT
    XX( M(VK_DOWN)      , NOUSE            ),       // 0x28 DOWN
    XX( NOUSE           , NOUSE            ),       // 0x29 SELECT
    XX( NOUSE           , NOUSE            ),       // 0x2A PRINT
    XX( NOUSE           , NOUSE            ),       // 0x2B EXECUTE
    XX( NOUSE           , NOUSE            ),       // 0x2C SNAPSHOT
    XX( NOUSE           , NOUSE            ),       // 0x2D INSERT
    XX( NOUSE           , NOUSE            ),       // 0x2E DELETE
    XX( NOUSE           , NOUSE            ),       // 0x2F HELP
    XX( NOUSE           , NOUSE            ),       // 0x30 0
    XX( M(VK_LBUTTON)   , NOUSE            ),       // 0x31 1
    XX( M(VK_MBUTTON)   , NOUSE            ),       // 0x32 2
    XX( M(VK_RBUTTON)   , NOUSE            ),       // 0x33 3
    XX( M(VK_XBUTTON1)  , NOUSE            ),       // 0x34 4
    XX( M(VK_XBUTTON2)  , NOUSE            ),       // 0x35 5
    XX( M(255)          , NOUSE            ),       // 0x36 6
    XX( NOUSE           , NOUSE            ),       // 0x37 7
    XX( NOUSE           , NOUSE            ),       // 0x38 8
    XX( NOUSE           , NOUSE            ),       // 0x39 9
    XX( NOUSE           , NOUSE            ),       // 0x3A
    XX( NOUSE           , NOUSE            ),       // 0x3B
    XX( NOUSE           , NOUSE            ),       // 0x3C
    XX( NOUSE           , NOUSE            ),       // 0x3D
    XX( NOUSE           , NOUSE            ),       // 0x3E
    XX( NOUSE           , NOUSE            ),       // 0x3F
    XX( NOUSE           , NOUSE            ),       // 0x40 @
    XX( C(VK_LEFT)      , C('H')           ),       // 0x41 A
    XX( EXSHIFT_SW      , NOUSE            ),       // 0x42 B
    XX( K(VK_NEXT)      , C(VK_END)        ),       // 0x43 C
    XX( K(VK_RIGHT)     , K(VK_END)        ),       // 0x44 D
    XX( K(VK_UP)        , NOUSE            ),       // 0x45 E
    XX( C(VK_RIGHT)     , C('F')           ),       // 0x46 F
    XX( K(VK_DELETE)    , NOUSE            ),       // 0x47 G
    XX( K(VK_BACK)      , NOUSE            ),       // 0x48 H
    XX( M(VK_UP)        , NOUSE            ),       // 0x49 I       K(VK_TAB)
    XX( M(VK_LEFT)      , NOUSE            ),       // 0x4A J
    XX( M(VK_RIGHT)     , NOUSE            ),       // 0x4B K
    XX( NOUSE           , NOUSE            ),       // 0x4C L
    XX( M(VK_DOWN)      , NOUSE            ),       // 0x4D M       K(VK_RETURN)
    XX( NOUSE           , NOUSE            ),       // 0x4E N
    XX( NOUSE           , NOUSE            ),       // 0x4F O
    XX( NOUSE           , NOUSE            ),       // 0x50 P
    XX( TWOST           , TWOST            ),       // 0x51 Q (手抜きのためTWOSTの2回目は必ずNOUSE以外でないと不味い)
    XX( K(VK_PRIOR)     , C(VK_HOME)       ),       // 0x52 R
    XX( K(VK_LEFT)      , K(VK_HOME)       ),       // 0x53 S
    XX( NOUSE           , NOUSE            ),       // 0x54 T
    XX( NOUSE           , NOUSE            ),       // 0x55 U
    XX( K(VK_INSERT)    , NOUSE            ),       // 0x56 V
    XX( C(VK_UP)        , NOUSE            ),       // 0x57 W
    XX( K(VK_DOWN)      , NOUSE            ),       // 0x58 X
    XX( NOUSE           , NOUSE            ),       // 0x59 Y
    XX( C(VK_DOWN)      , NOUSE            ),       // 0x5A Z
    XX( K(VK_LWIN)      , NOUSE            ),       // 0x5B LWIN
    XX( K(VK_RWIN)      , NOUSE            ),       // 0x5C RWIN
    XX( NOUSE           , NOUSE            ),       // 0x5D APPS
    XX( NOUSE           , NOUSE            ),       // 0x5E
    XX( NOUSE           , NOUSE            ),       // 0x5F SLEEP
    XX( NOUSE           , NOUSE            ),       // 0x60 NUMPAD0
    XX( NOUSE           , NOUSE            ),       // 0x61 NUMPAD1
    XX( NOUSE           , NOUSE            ),       // 0x62 NUMPAD2
    XX( NOUSE           , NOUSE            ),       // 0x63 NUMPAD3
    XX( NOUSE           , NOUSE            ),       // 0x64 NUMPAD4
    XX( NOUSE           , NOUSE            ),       // 0x65 NUMPAD5
    XX( NOUSE           , NOUSE            ),       // 0x66 NUMPAD6
    XX( NOUSE           , NOUSE            ),       // 0x67 NUMPAD7
    XX( NOUSE           , NOUSE            ),       // 0x68 NUMPAD8
    XX( NOUSE           , NOUSE            ),       // 0x69 NUMPAD9
    XX( NOUSE           , NOUSE            ),       // 0x6A MULTIPLY
    XX( NOUSE           , NOUSE            ),       // 0x6B ADD
    XX( NOUSE           , NOUSE            ),       // 0x6C SEPARATOR
    XX( NOUSE           , NOUSE            ),       // 0x6D SUBTRACT
    XX( NOUSE           , NOUSE            ),       // 0x6E DECIMAL
    XX( NOUSE           , NOUSE            ),       // 0x6F DIVIDE
    XX( NOUSE           , NOUSE            ),       // 0x70 F1
    XX( NOUSE           , NOUSE            ),       // 0x71 F2
    XX( NOUSE           , NOUSE            ),       // 0x72 F3
    XX( NOUSE           , NOUSE            ),       // 0x73 F4
    XX( NOUSE           , NOUSE            ),       // 0x74 F5
    XX( NOUSE           , NOUSE            ),       // 0x75 F6
    XX( NOUSE           , NOUSE            ),       // 0x76 F7
    XX( NOUSE           , NOUSE            ),       // 0x77 F8
    XX( NOUSE           , NOUSE            ),       // 0x78 F9
    XX( NOUSE           , NOUSE            ),       // 0x79 F10
    XX( NOUSE           , NOUSE            ),       // 0x7A F11
    XX( NOUSE           , NOUSE            ),       // 0x7B F12
    XX( NOUSE           , NOUSE            ),       // 0x7C F13
    XX( NOUSE           , NOUSE            ),       // 0x7D F14
    XX( NOUSE           , NOUSE            ),       // 0x7E F15
    XX( NOUSE           , NOUSE            ),       // 0x7F F16
    XX( NOUSE           , NOUSE            ),       // 0x80 F17
    XX( NOUSE           , NOUSE            ),       // 0x81 F18
    XX( NOUSE           , NOUSE            ),       // 0x82 F19
    XX( NOUSE           , NOUSE            ),       // 0x83 F20
    XX( NOUSE           , NOUSE            ),       // 0x84 F21
    XX( NOUSE           , NOUSE            ),       // 0x85 F22
    XX( NOUSE           , NOUSE            ),       // 0x86 F23
    XX( NOUSE           , NOUSE            ),       // 0x87 F24
    XX( NOUSE           , NOUSE            ),       // 0x88
    XX( NOUSE           , NOUSE            ),       // 0x89
    XX( NOUSE           , NOUSE            ),       // 0x8A
    XX( NOUSE           , NOUSE            ),       // 0x8B
    XX( NOUSE           , NOUSE            ),       // 0x8C
    XX( NOUSE           , NOUSE            ),       // 0x8D
    XX( NOUSE           , NOUSE            ),       // 0x8E
    XX( NOUSE           , NOUSE            ),       // 0x8F
    XX( NOUSE           , NOUSE            ),       // 0x90 NUMLOCK
    XX( NOUSE           , NOUSE            ),       // 0x91 SCROLL
    XX( NOUSE           , NOUSE            ),       // 0x92 OEM_NEC_EQUAL (PC98)   OEM_FJ_JISHO (Fujitsu OASYS)
    XX( NOUSE           , NOUSE            ),       // 0x93 OEM_FJ_MASSHOU (Fujitsu OASYS)
    XX( NOUSE           , NOUSE            ),       // 0x94 OEM_FJ_TOUROKU (Fujitsu OASYS)
    XX( NOUSE           , NOUSE            ),       // 0x95 OEM_FJ_LOYA    (Fujitsu OASYS)
    XX( NOUSE           , NOUSE            ),       // 0x96 OEM_FJ_ROYA    (Fujitsu OASYS)
    XX( NOUSE           , NOUSE            ),       // 0x97
    XX( NOUSE           , NOUSE            ),       // 0x98
    XX( NOUSE           , NOUSE            ),       // 0x99
    XX( NOUSE           , NOUSE            ),       // 0x9A
    XX( NOUSE           , NOUSE            ),       // 0x9B
    XX( NOUSE           , NOUSE            ),       // 0x9C
    XX( NOUSE           , NOUSE            ),       // 0x9D
    XX( NOUSE           , NOUSE            ),       // 0x9E
    XX( NOUSE           , NOUSE            ),       // 0x9F
    XX( K(VK_LSHIFT)    , NOUSE            ),       // 0xA0 LSHIFT
    XX( K(VK_RSHIFT)    , NOUSE            ),       // 0xA1 RSHIFT
    XX( K(VK_LCONTROL)  , NOUSE            ),       // 0xA2 LCONTROL
    XX( K(VK_RCONTROL)  , NOUSE            ),       // 0xA3 RCONTROL
    XX( K(VK_LMENU)     , NOUSE            ),       // 0xA4 LMENU (左ALT)
    XX( K(VK_RMENU)     , NOUSE            ),       // 0xA5 RMENU (右ALT)
    XX( NOUSE           , NOUSE            ),       // 0xA6 BROWSER_BACK
    XX( NOUSE           , NOUSE            ),       // 0xA7 BROWSER_FORWARD
    XX( NOUSE           , NOUSE            ),       // 0xA8 BROWSER_REFRESH
    XX( NOUSE           , NOUSE            ),       // 0xA9 BROWSER_STOP
    XX( NOUSE           , NOUSE            ),       // 0xAA BROWSER_SEARCH
    XX( NOUSE           , NOUSE            ),       // 0xAB BROWSER_FAVORITES
    XX( NOUSE           , NOUSE            ),       // 0xAC BROWSER_HOME
    XX( NOUSE           , NOUSE            ),       // 0xAD VOLUME_MUTE
    XX( NOUSE           , NOUSE            ),       // 0xAE VOLUME_DOWN
    XX( NOUSE           , NOUSE            ),       // 0xAF VOLUME_UP
    XX( NOUSE           , NOUSE            ),       // 0xB0 MEDIA_NEXT_TRACK
    XX( NOUSE           , NOUSE            ),       // 0xB1 MEDIA_PREV_TRACK
    XX( NOUSE           , NOUSE            ),       // 0xB2 MEDIA_STOP
    XX( NOUSE           , NOUSE            ),       // 0xB3 MEDIA_PLAY_PAUSE
    XX( NOUSE           , NOUSE            ),       // 0xB4 LAUNCH_MAIL
    XX( NOUSE           , NOUSE            ),       // 0xB5 LAUNCH_MEDIA_SELECT
    XX( NOUSE           , NOUSE            ),       // 0xB6 LAUNCH_APP1
    XX( NOUSE           , NOUSE            ),       // 0xB7 LAUNCH_APP2
    XX( NOUSE           , NOUSE            ),       // 0xB8
    XX( NOUSE           , NOUSE            ),       // 0xB9
    XX( NOUSE           , NOUSE            ),       // 0xBA OEM_1   (US ;:)
    XX( NOUSE           , NOUSE            ),       // 0xBB OEM_PLUS
    XX( NOUSE           , NOUSE            ),       // 0xBC OEM_COMMA
    XX( NOUSE           , NOUSE            ),       // 0xBD OEM_MINUS
    XX( NOUSE           , NOUSE            ),       // 0xBE OEM_PERIOD
    XX( NOUSE           , NOUSE            ),       // 0xBF OEM_2   (US /?)
    XX( NOUSE           , NOUSE            ),       // 0xC0 OEM_3   (US `~)
    XX( NOUSE           , NOUSE            ),       // 0xC1
    XX( NOUSE           , NOUSE            ),       // 0xC2
    XX( NOUSE           , NOUSE            ),       // 0xC3
    XX( NOUSE           , NOUSE            ),       // 0xC4
    XX( NOUSE           , NOUSE            ),       // 0xC5
    XX( NOUSE           , NOUSE            ),       // 0xC6
    XX( NOUSE           , NOUSE            ),       // 0xC7
    XX( NOUSE           , NOUSE            ),       // 0xC8
    XX( NOUSE           , NOUSE            ),       // 0xC9
    XX( NOUSE           , NOUSE            ),       // 0xCA
    XX( NOUSE           , NOUSE            ),       // 0xCB
    XX( NOUSE           , NOUSE            ),       // 0xCC
    XX( NOUSE           , NOUSE            ),       // 0xCD
    XX( NOUSE           , NOUSE            ),       // 0xCE
    XX( NOUSE           , NOUSE            ),       // 0xCF
    XX( NOUSE           , NOUSE            ),       // 0xD0
    XX( NOUSE           , NOUSE            ),       // 0xD1
    XX( NOUSE           , NOUSE            ),       // 0xD2
    XX( NOUSE           , NOUSE            ),       // 0xD3
    XX( NOUSE           , NOUSE            ),       // 0xD4
    XX( NOUSE           , NOUSE            ),       // 0xD5
    XX( NOUSE           , NOUSE            ),       // 0xD6
    XX( NOUSE           , NOUSE            ),       // 0xD7
    XX( NOUSE           , NOUSE            ),       // 0xD8
    XX( NOUSE           , NOUSE            ),       // 0xD9
    XX( NOUSE           , NOUSE            ),       // 0xXX
    XX( NOUSE           , NOUSE            ),       // 0xDB OEM_4   (US [{)
    XX( NOUSE           , NOUSE            ),       // 0xDC OEM_5   (US \|)
    XX( NOUSE           , NOUSE            ),       // 0xDD OEM_6   (US ]})
    XX( NOUSE           , NOUSE            ),       // 0xDE OEM_7   (US '")
    XX( NOUSE           , NOUSE            ),       // 0xDF OEM_8
    XX( NOUSE           , NOUSE            ),       // 0xE0
    XX( NOUSE           , NOUSE            ),       // 0xE1 OEM_AX
    XX( NOUSE           , NOUSE            ),       // 0xE2 OEM_102 (RT102key <> or \| )
    XX( NOUSE           , NOUSE            ),       // 0xE3 ICO_HELP
    XX( NOUSE           , NOUSE            ),       // 0xE4 ICO_00
    XX( NOUSE           , NOUSE            ),       // 0xE5 PROCESSKEY
    XX( NOUSE           , NOUSE            ),       // 0xE6 ICO_CLEAR
    XX( NOUSE           , NOUSE            ),       // 0xE7 PACKET
    XX( NOUSE           , NOUSE            ),       // 0xE8
    XX( NOUSE           , NOUSE            ),       // 0xE9 OEM_RESET   (Nokia/Ericsson)
    XX( NOUSE           , NOUSE            ),       // 0xEA OEM_JUMP    (Nokia/Ericsson)
    XX( NOUSE           , NOUSE            ),       // 0xEB OEM_PA1     (Nokia/Ericsson)
    XX( NOUSE           , NOUSE            ),       // 0xEC OEM_PA2     (Nokia/Ericsson)
    XX( NOUSE           , NOUSE            ),       // 0xED OEM_PA3     (Nokia/Ericsson)
    XX( NOUSE           , NOUSE            ),       // 0xEE OEM_WSCTRL  (Nokia/Ericsson)
    XX( NOUSE           , NOUSE            ),       // 0xEF OEM_CUSEL   (Nokia/Ericsson)
    XX( NOUSE           , NOUSE            ),       // 0xF0 OEM_ATTN    (Nokia/Ericsson)
    XX( NOUSE           , NOUSE            ),       // 0xF1 OEM_FINISH  (Nokia/Ericsson)
    XX( NOUSE           , NOUSE            ),       // 0xF2 OEM_COPY    (Nokia/Ericsson)
    XX( NOUSE           , NOUSE            ),       // 0xF3 OEM_AUTO    (Nokia/Ericsson)
    XX( NOUSE           , NOUSE            ),       // 0xF4 OEM_ENLW    (Nokia/Ericsson)
    XX( NOUSE           , NOUSE            ),       // 0xF5 OEM_BACKTAB (Nokia/Ericsson)
    XX( NOUSE           , NOUSE            ),       // 0xF6 ATTN
    XX( NOUSE           , NOUSE            ),       // 0xF7 CRSEL
    XX( NOUSE           , NOUSE            ),       // 0xF8 EXSEL
    XX( NOUSE           , NOUSE            ),       // 0xF9 EREOF
    XX( NOUSE           , NOUSE            ),       // 0xFA PLAY
    XX( NOUSE           , NOUSE            ),       // 0xFB ZOOM
    XX( NOUSE           , NOUSE            ),       // 0xFC NONAME
    XX( NOUSE           , NOUSE            ),       // 0xFD PA1
    XX( NOUSE           , NOUSE            ),       // 0xFE OEM_CLEAR
    XX( NOUSE           , NOUSE            ),       // 0xFF
}};

#endif
