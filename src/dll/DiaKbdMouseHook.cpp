/**
 *	@file	DiaKbdMouseHook.h
 *	@brief	APPS+を用いたダイアモンドカーソル操作するためのキーのフック用DLL
 *	@auther	Masashi KITAMURA
 *	@date	2006
 *  @note
 *		フリーソース
 */

#include "stdafx.h"
#include "DiaKbdMouseHook.h"
#include "DiaKbdMouseHook_Impl.h"


/// DLL エントリ
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
	case DLL_THREAD_ATTACH:		break;
	case DLL_THREAD_DETACH:		break;
	case DLL_PROCESS_DETACH:	break;
	default:					break;
	}
	return TRUE;
}



/** フックを設定
 */
HOOKDLL_API int DiaKbdMouseHook_install(int keycodde, CDiaKbdMouseHook_ConvKeyTbl const& tbl)
{
	return CDiaKbdMouseHook_Impl::install(keycodde, tbl);
}



/**
 *	フックを解除
 */
HOOKDLL_API int DiaKbdMouseHook_uninstall()
{
	return CDiaKbdMouseHook_Impl::uninstall();
}


/** パッド的にしたマウスボタン情報の取得
 */
HOOKDLL_API unsigned DiaKbdMouseHook_mouseButton()
{
	return CDiaKbdMouseHook_Impl::mouseButton();
}


#if 0
/** モードを切り替えるキーを設定
 */
HOOKDLL_API void DiaKbdMouseHook_setModeKey(unsigned vkMode) {
	CDiaKbdMouseHook_Impl::setModeKey(vkMode);
}
#endif


#ifdef USE_LWINKEY
/** 左Winによるマウス操作の利用のon/off. -1だと何もしない.
 *	@return 返値は直前の状態
 */
HOOKDLL_API bool DiaKbdMouseHook_setLWinMode(int sw)
{
	return CDiaKbdMouseHook_Impl::setLWinMode(sw);
}
#endif
