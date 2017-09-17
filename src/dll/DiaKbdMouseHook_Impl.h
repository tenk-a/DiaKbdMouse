/**
 *	@file	DiaKbdMouseHook_Impl.h
 *	@brief	�L�[�{�[�h�Ń}�E�X���삷�邽�߂̃L�[�̃t�b�N�pDLL
 *	@auther	Masashi KITAMURA
 *	@date	2006
 *	@note
 *		�t���[�\�[�X
 */
#ifndef DIAKBDMOUSEHOOK_IMPL_H
#define DIAKBDMOUSEHOOK_IMPL_H
#pragma once

#include <cassert>
#include <windows.h>
#include "DiaKbdMouseHook.h"
#include "../cmn/CriticalSection.hpp"

//#define DIAKBDMOUSEHOOK_USE_EX_SHIFT


/// �L�[���͂��擾(�̂��Ƃ�)���߂̃t�b�N
class CDiaKbdMouseHook_Impl {
public:
	enum { VK_NUM = 256 };

	/// ������
	static void init(HINSTANCE hInst) { s_hInst_ = hInst; }

	/// �t�b�N����
	static bool install(int keycode, CDiaKbdMouseHook_ConvKeyTbl const& tbl);

	/// �t�b�N������
	static bool uninstall();

	/// �}�E�X������L�[�����{�^�����������̂��擾
	static unsigned mouseButton();

  #ifdef USE_LWINKEY
	/// ��Win�g����on/off
	static bool setLWinMode(int sw);
  #endif

	/// �ϊ��e�[�u�����w��t�@�C������ǂݍ���
	static void setConvTable(const CDiaKbdMouseHook_ConvKey tbl[VK_NUM]);

private:
	static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wparam, LPARAM lparam);

	/// ���[�h�ؑփL�[�̃R�[�h��ݒ�
	static void setModeKeyTbl(unsigned vkMode, CDiaKbdMouseHook_ConvKeyTbl const& tbl);

	static bool keyDownUp(bool sw, unsigned vkCode);
	static void sendConvKey(bool sw, unsigned uKey );
	static void sendKey(int mode, unsigned uFlags, unsigned uVk );
	static void setInputParam(INPUT& rImput, unsigned uFlags, unsigned uVk );

	static bool makeMouseButton( bool sw, unsigned uVk );
	static bool setMouseButton(unsigned btn, bool sw);

	static void clearStat();

 #ifdef DIAKBDMOUSEHOOK_USE_EX_SHIFT
	static void clearExShift();
 #endif

private:
	enum { DiaKbdMouseHook_EXTRAINFO = 0xD1AC };

	static CCriticalSection		s_criticalSection_;
	static HINSTANCE   			s_hInst_;			///< Window�C���X�^���X
	static volatile HHOOK		s_hHook_LL_;		///< �t�b�N
	static unsigned				s_uModeKey_;		///< ���[�h�ؑփL�[�̃L�[�R�[�h
	static unsigned				s_uMouseButton_;	///< �}�E�X����p�̃{�^�����
	static int					s_iMouseButtonLife_; ///< (Win+L�ł�)�\�����̔�Q�y���p
	static bool					s_bConvModeStat_;	///< �L�[����ɂ�郂�[�hon/off
	static bool					s_bTwoStStatQ_;		///< 2�X�g���[�N�L�[���[�h��
	static bool					s_bShiftStat_;		///< SHIFT��������Ă�Ƃ�
	static bool					s_bCtrlStat_;		///< CTRL��������Ă�Ƃ�
	static bool					s_bDiaMouse_;		///< �_�C�A�����h�J�[�\���Ń}�E�X�𓮂���
  #ifdef DIAKBDMOUSEHOOK_USE_EX_SHIFT
	static bool					s_bExShift_;		///< �J�[�\���ړ��ł̎����V�t�g����
  #endif
  #ifdef USE_LWINKEY
	static bool					s_bLWinStat_;		///< ��Win�L�[����ɂ��on/off
	static bool					s_bLWinModeSw_;		///< ��Win�L�[�ɂ��L�[�{�[�h�}�E�X���p��on/off
  #endif

	static CDiaKbdMouseHook_ConvKeyTbl	s_convKeys_;
};

#endif
