/**
 *	@file	TrayIcon.h
 *	@brief	�g���C�풓�A�C�R�������p�N���X
 *	@author	Masashi KITAMURA
 *	@date	2006
 *	@note
 *		�t���[�\�[�X
 */
#ifndef TRAYICON_H
#define TRAYICON_H

#pragma once

#include <windows.h>


///	�g���C�풓�A�C�R�������p�N���X
class CTrayIcon {
public:
	CTrayIcon() { for (int i = sizeof *this; --i >= 0;) ((char*)this)[i] = '\0'; }	// ����@
	~CTrayIcon() { release(); }

	/// �g���C�A�C�R���̍쐬
	bool create(unsigned uWm, HINSTANCE hInstance, HWND hWnd, HICON hIcon, const TCHAR* pTip, UINT uMenu);

	/// �g���C�A�C�R���̊J��
	void release();

	/// WndProc �ɂăg���C��蒼���̃E�B���h�E���b�Z�[�W���������`�F�b�N���ė��Ă���č쐬
	void checkRecreate(UINT uMsg);

	/// �A�C�R���ʒu�Ń|�b�v�A�b�v���j���[�őI�����ꂽ��A����ID��Ԃ��B�����Ȃ�0��Ԃ��B
	UINT trackPopupMenu();

	/// ���݂̃g���C���j���[�̃n���h����Ԃ�
	const HMENU	getMenuHandle() const { return hMenuSub_; }

private:
	NOTIFYICONDATA 		notifyIconData_;			///< �g���C�ɂ̂�A�C�R���̂��߂̏��
	HINSTANCE			hInstance_;					///< �C���X�^���X��n���h��
	HWND				hWnd_;						///< �E�B���h�E�E�n���h��
	UINT 				uWmTaskBarCreated_;			///< �^�X�N�o�[�����ꂽ���ɂ���E�B���h�E�n���h��
	HMENU				hMenu_;						///< �g���C���j���[�̃n���h��
	HMENU				hMenuSub_;					///< �g���C���j���[�̃T�u���j���[�̃n���h��
	UINT				uMenuId_;					///< ���j���[��ID
};


#endif
