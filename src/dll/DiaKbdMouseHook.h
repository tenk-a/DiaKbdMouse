/**
 *	@file	DiaKbdMouseHook.h
 *	@brief	�L�[�{�[�h�Ń}�E�X���삷�邽�߂̃L�[�̃t�b�N�pDLL
 *	@auther	Masashi KITAMURA
 *	@date	2006
 *  @note
 *		�t���[�\�[�X
 */
#ifndef DIAKBDMOUSEHOOK_H
#define DIAKBDMOUSEHOOK_H
#pragma once

#include <cassert>
#include <cstring>

#ifdef HOOKDLL_EXPORTS
#define HOOKDLL_API extern "C" __declspec(dllexport)
#else
#define HOOKDLL_API extern "C" __declspec(dllimport)
#endif

class CDiaKbdMouseHook_ConvKeyTbl;

/// �t�b�N����
HOOKDLL_API int 	 DiaKbdMouseHook_install(int keyCode, CDiaKbdMouseHook_ConvKeyTbl const& tbl);

/// �t�b�N����߂�
HOOKDLL_API int 	 DiaKbdMouseHook_uninstall();

/// ���[�h��؂�ւ���L�[��ݒ�
HOOKDLL_API void	DiaKbdMouseHook_setModeKey(unsigned vkMode);

#ifdef USE_LWINKEY
/// ��Win�L�[�ɂ��}�E�X�����on/off
HOOKDLL_API	bool	DiaKbdMouseHook_setLWinMode(int sw);
#endif

/// �p�b�h�I�ɂ����}�E�X�{�^�����̎擾
HOOKDLL_API unsigned DiaKbdMouseHook_mouseButton();

/// �}�E�X������L�[�̏��
enum EDiaKbdMouse_Mouse {
	DIAKBDMOUSE_MOUSE_LEFT		= 0x0001,
	DIAKBDMOUSE_MOUSE_UP		= 0x0002,
	DIAKBDMOUSE_MOUSE_RIGHT		= 0x0004,
	DIAKBDMOUSE_MOUSE_DOWN		= 0x0008,
	DIAKBDMOUSE_MOUSE_LBUTTON	= 0x0010,
	DIAKBDMOUSE_MOUSE_RBUTTON	= 0x0020,
	DIAKBDMOUSE_MOUSE_MBUTTON	= 0x0040,
	DIAKBDMOUSE_MOUSE_XBUTTON1	= 0x0080,
	DIAKBDMOUSE_MOUSE_XBUTTON2	= 0x0100,
	DIAKBDMOUSE_MOUSE_WHEEL1	= 0x0200,
	DIAKBDMOUSE_MOUSE_WHEEL2	= 0x0400,
  #ifdef USE_LWINKEY
	DIAKBDMOUSE_MOUSE_MODEKEY	= 0x0800,		///< ��Win���g�����Ƃ��̑΍�
  #else
	DIAKBDMOUSE_MOUSE_SPEEDUP	= 0x0800,
  #endif
	DIAKBDMOUSE_MOUSE_SPEEDCHG	= 0x1000,
};



/// �ϊ��e�[�u���p�̌^
class CDiaKbdMouseHook_ConvKey {
public:
	///< ���[�h
	enum EMode {
		MD_NONE		= 0,			///< �g��Ȃ�
		MD_USE		= 1,			///< �g��
		MD_CTRL		= 2,			///< CTRL���p
		MD_SHIFT	= 3,			///< SHIFT���p
		MD_CTRLSHIFT= 4,			///< CTRL+SHIFT���p
		MD_2ST_Q	= 5,			///< 2�X�g���[�N�L�[�̃g���K�[(Q)
		MD_EX_SHIFT = 6,			//x �p�~(�J�[�\���ړ��ŃV�t�g���p���[�h(�͈͑I����z��))
		MD_MOUSE	= 7,			///< �}�E�X
	};
	class COne {
	public:
		unsigned char u8Mode_;		///< ���[�h(EMode)
		unsigned char u8VkCode_;	///< �ϊ�����L�[�R�[�h
	};
	COne	oneKey_[2];				///< [0]=�g���L�[�݂̂̂Ƃ��̃L�[  [1]=2�X�g���[�N�L�[(Q)���������Ƃ��p
};

class CDiaKbdMouseHook_ConvKeyTbl {
public:
	enum { MAX_SIZE = 256 };
	//CDiaKbdMouseHook_ConvKeyTbl() { std::memset(tbl_, 0, sizeof tbl_); }
	CDiaKbdMouseHook_ConvKey& operator[](unsigned n) { assert(n < MAX_SIZE); return tbl_[n]; }
	CDiaKbdMouseHook_ConvKey const& operator[](unsigned n) const { assert(n < MAX_SIZE); return tbl_[n]; }
	void	clear() { std::memset(tbl_, 0, sizeof tbl_); }
public:
	CDiaKbdMouseHook_ConvKey	tbl_[MAX_SIZE];
};

#endif
