/**
 *	@file	KbdMouseCtrl.h
 *	@brief	�L�[�{�[�h�Ń}�E�X���삷�鏈��(�ʃX���b�h)
 *	@auther	Masashi KITAMURA
 *	@date	2006
 *	@note
 *		�t���[�\�[�X
 */
#ifndef KBDMOUSECTRL_H
#define KBDMOUSECTRL_H

#pragma once

#include "stdafx.h"
#include "../cmn/DgtXY2AnlgXY.hpp"


/// �L�[�{�[�h�Ń}�E�X���삷�鏈��(�ʃX���b�h)
class CKbdMouseCtrl {
public:
	/// �쐬
	static void		create();

	/// ����
	static void		release();

private:
	CKbdMouseCtrl();
	~CKbdMouseCtrl();

	static void		run();

	static void		ctrl();
	static void		moveMouse(unsigned uNow, unsigned uOld, unsigned uTrig);
	static unsigned makeReptKey(unsigned uNow, unsigned uOld, unsigned uTrig);
	static void 	sendMouseButton(unsigned uTrig, unsigned uRel);
 #ifdef USE_LWINKEY
	static void 	checkModeKey(unsigned uNow, unsigned uTrig);
 #endif

private:
	enum {SLEEP_COUNT =  8 };
	enum { DLT		  =  4 };
	enum { HIS_NUM	  = 12 };
	static HANDLE 							s_hThread_;					///< �X���b�h�n���h��
	static unsigned							s_uOld_;					///< 1�t���[���O�̃{�^�����
	static DgtXY2AnlgXY<float,256,HIS_NUM>	s_dgtXY2AnlgXY_;			///< �f�W�^���{�^�������A�i���O�����邽�߂̃N���X
	static bool								s_bWin1st_;
	static unsigned							s_uReptCnt_;
	static const INPUT						s_input_mouseSendTbl_[];	///< �}�E�X���ɕϊ�����Ƃ��Ɏg��
};


#endif
