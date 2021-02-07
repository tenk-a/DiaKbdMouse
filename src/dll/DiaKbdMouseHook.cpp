/**
 *  @file   DiaKbdMouseHook.h
 *  @brief  APPS+を用いたダイアモンドカーソル操作するためのキーのフック用DLL
 *  @auther Masashi KITAMURA
 *  @date   2006
 *  @note
 *      フリーソース
 */

#include "stdafx.h"
#include "DiaKbdMouseHook.h"
#include "DiaKbdMouseHook_Impl.h"


/// DLL エントリ.
BOOL APIENTRY DllMain(
        HANDLE hModule,
        DWORD  ul_reason_for_call,
        LPVOID /*lpReserved*/
){
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        CDiaKbdMouseHook_Impl::init( HINSTANCE(hModule) );
        ::DisableThreadLibraryCalls(HMODULE(hModule));
        break;
    case DLL_THREAD_ATTACH:     break;
    case DLL_THREAD_DETACH:     break;
    case DLL_PROCESS_DETACH:    break;
    default:                    break;
    }
    return TRUE;
}



/** フックを設定.
 */
HOOKDLL_API int DiaKbdMouseHook_install(int keycode, CDiaKbdMouseHook_ConvKeyTbl const& tbl)
{
    return CDiaKbdMouseHook_Impl::install(keycode, tbl);
}



/**
 *  フックを解除.
 */
HOOKDLL_API int DiaKbdMouseHook_uninstall()
{
    return CDiaKbdMouseHook_Impl::uninstall();
}


/** パッド的にしたマウスボタン情報の取得.
 */
HOOKDLL_API unsigned DiaKbdMouseHook_mouseButton()
{
    return CDiaKbdMouseHook_Impl::mouseButton();
}
