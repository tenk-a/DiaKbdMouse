/**
 *	@file	TrayIcon.cpp
 *	@brief	�g���C�풓�A�C�R�������p�N���X
 *	@author	Masashi KITAMURA
 *	@date	2006
 *	@note
 *		�t���[�\�[�X
 */

#include "stdafx.h"
#include "TrayIcon.h"



/// ������src ��dst�� size-1 �͈̔͂ŃR�s�[�Bdst�ɂ͕K���Ō��\0������B
static inline const TCHAR* tcsCpyNum(TCHAR *dst, const TCHAR *src, unsigned size) {
	TCHAR*		d = dst;
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


/** �g���C�A�C�R���̍쐬
 */
bool CTrayIcon::create(unsigned uWm, HINSTANCE hInstance, HWND hWnd, HICON hIcon, const TCHAR* pTip, UINT uMenu)
{
	hInstance_							= hInstance;
	hWnd_								= hWnd;

	// �ʒm�̈�̃A�C�R�����쐬
	notifyIconData_.cbSize 				= sizeof(NOTIFYICONDATA);
	notifyIconData_.hWnd 				= hWnd;
	notifyIconData_.uID 				= 0;
	notifyIconData_.uFlags 				= NIF_ICON | NIF_MESSAGE | NIF_TIP;
	notifyIconData_.uCallbackMessage 	= uWm;
	notifyIconData_.hIcon 				= hIcon;
	
	tcsCpyNum(notifyIconData_.szTip, pTip, sizeof(notifyIconData_.szTip));

	// �A�C�R����o�^�ł��邩�G���[�ɂȂ�܂ŌJ��Ԃ�
	while (::Shell_NotifyIcon(NIM_ADD, &notifyIconData_) == 0) {
		if (::GetLastError() != ERROR_TIMEOUT) {	// �^�C���A�E�g�ȊO�Ȃ�
			notifyIconData_.cbSize = 0;
			return false;	// �A�C�R���o�^�G���[
		}
		// �o�^�ł��Ă��Ȃ����Ƃ��m�F����
		if (::Shell_NotifyIcon(NIM_MODIFY, &notifyIconData_)) {
			// �o�^�ł��Ă���
			break;
		} else {
			// �o�^�ł��Ă��Ȃ�����
			::Sleep(100);
		}
	}

	uWmTaskBarCreated_ = ::RegisterWindowMessage( _T("TaskbarCreated") );

	// �����Ń��j���[���p�ӂ��Ă��܂�
	uMenuId_  = uMenu;
	hMenu_    = ::LoadMenu( hInstance_, MAKEINTRESOURCE( uMenu ) );
	hMenuSub_ = ::GetSubMenu( hMenu_, 0 );
	return true;
}



/** �g���C�A�C�R���̊J��
 */
void CTrayIcon::release()
{
	// �ʒm�̈�̃A�C�R���̍폜
	if (notifyIconData_.cbSize) {
		notifyIconData_.cbSize 	= sizeof(NOTIFYICONDATA);
		notifyIconData_.hWnd 	= hWnd_;
		notifyIconData_.uID 	= 0;
		notifyIconData_.uFlags 	= 0;
		::Shell_NotifyIcon(NIM_DELETE, &notifyIconData_);
		notifyIconData_.cbSize 	= 0;
	}
	::DestroyMenu(hMenu_);  //���j���[�j��
}



/** WndProc �ɂăg���C��蒼���̃E�B���h�E���b�Z�[�W���������`�F�b�N���ė��Ă���č쐬
 */
void CTrayIcon::checkRecreate(UINT msg)
{
	if (msg == uWmTaskBarCreated_) {
		create(notifyIconData_.uCallbackMessage, hInstance_, hWnd_, notifyIconData_.hIcon, notifyIconData_.szTip, uMenuId_);
	}
}



/** �A�C�R���ʒu�Ń|�b�v�A�b�v���j���[�őI�����ꂽ��A����ID��Ԃ��B�����Ȃ�0��Ԃ��B
 */
UINT CTrayIcon::trackPopupMenu()
{
	// �|�b�v�A�b�v���j���[���J���O�Ƀ��C���E�B���h�E���A�N�e�B�u�ɂ���.
	::SetForegroundWindow(hWnd_);

	// �|�b�v�A�b�v���j���[���J��.
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

