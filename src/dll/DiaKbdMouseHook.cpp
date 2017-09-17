/**
 *	@file	DiaKbdMouseHook.h
 *	@brief	APPS+��p�����_�C�A�����h�J�[�\�����삷�邽�߂̃L�[�̃t�b�N�pDLL
 *	@auther	Masashi KITAMURA
 *	@date	2006
 *  @note
 *		�t���[�\�[�X
 */

#include "stdafx.h"
#include "DiaKbdMouseHook.h"
#include "DiaKbdMouseHook_Impl.h"


/// DLL �G���g��
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



/** �t�b�N��ݒ�
 */
HOOKDLL_API int DiaKbdMouseHook_install(int keycodde, CDiaKbdMouseHook_ConvKeyTbl const& tbl)
{
	return CDiaKbdMouseHook_Impl::install(keycodde, tbl);
}



/**
 *	�t�b�N������
 */
HOOKDLL_API int DiaKbdMouseHook_uninstall()
{
	return CDiaKbdMouseHook_Impl::uninstall();
}


/** �p�b�h�I�ɂ����}�E�X�{�^�����̎擾
 */
HOOKDLL_API unsigned DiaKbdMouseHook_mouseButton()
{
	return CDiaKbdMouseHook_Impl::mouseButton();
}


#if 0
/** ���[�h��؂�ւ���L�[��ݒ�
 */
HOOKDLL_API void DiaKbdMouseHook_setModeKey(unsigned vkMode) {
	CDiaKbdMouseHook_Impl::setModeKey(vkMode);
}
#endif


#ifdef USE_LWINKEY
/** ��Win�ɂ��}�E�X����̗��p��on/off. -1���Ɖ������Ȃ�.
 *	@return �Ԓl�͒��O�̏��
 */
HOOKDLL_API bool DiaKbdMouseHook_setLWinMode(int sw)
{
	return CDiaKbdMouseHook_Impl::setLWinMode(sw);
}
#endif
