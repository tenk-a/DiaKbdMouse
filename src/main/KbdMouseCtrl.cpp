/**
 *	@file	KbdMouseCtrl.cpp
 *	@brief	�L�[�{�[�h�Ń}�E�X���삷�鏈��(�ʃX���b�h)
 *	@auther	Masashi KITAMURA
 *	@date	2006
 *	@note
 *		�ʂɃX���b�h�킯��K�v�Ȃ���������...
 *		�����AtimeBeginPeriod()�Ƃ��������...
 *		���A�`�F�b�N���Ȃ����̖ʓ|�Ȃ�ł��̂܂�
 *
 *		�t���[�\�[�X
 */

#include "stdafx.h"
#include "../dll/DiaKbdMouseHook.h"
#include "../cmn/DebugPrintf.h"
#include "KbdMouseCtrl.h"

HANDLE 			CKbdMouseCtrl::s_hThread_		= 0;
unsigned		CKbdMouseCtrl::s_uOld_			= 0;
bool			CKbdMouseCtrl::s_bWin1st_ 		= 0;
unsigned		CKbdMouseCtrl::s_uReptCnt_		= 0;

DgtXY2AnlgXY<float,256,CKbdMouseCtrl::HIS_NUM> 	CKbdMouseCtrl::s_dgtXY2AnlgXY_;


/// �}�E�X�����邽�߂̏��
const INPUT CKbdMouseCtrl::s_input_mouseSendTbl_[] = {
	{ INPUT_MOUSE	, { 	   0, 0, 0		 , MOUSEEVENTF_LEFTDOWN  , 0, 0, }},		// 0
	{ INPUT_MOUSE	, { 	   0, 0, 0		 , MOUSEEVENTF_LEFTUP	 , 0, 0, }},		// 1
	{ INPUT_MOUSE	, { 	   0, 0, 0		 , MOUSEEVENTF_RIGHTDOWN , 0, 0, }},		// 2
	{ INPUT_MOUSE	, { 	   0, 0, 0		 , MOUSEEVENTF_RIGHTUP	 , 0, 0, }},		// 3
	{ INPUT_MOUSE	, { 	   0, 0, 0		 , MOUSEEVENTF_MIDDLEDOWN, 0, 0, }},		// 4
	{ INPUT_MOUSE	, { 	   0, 0, 0		 , MOUSEEVENTF_MIDDLEUP  , 0, 0, }},		// 5
	{ INPUT_MOUSE	, { 	   0, 0, XBUTTON1, MOUSEEVENTF_XDOWN	 , 0, 0, }},		// 6
	{ INPUT_MOUSE	, { 	   0, 0, XBUTTON1, MOUSEEVENTF_XUP		 , 0, 0, }},		// 7
	{ INPUT_MOUSE	, { 	   0, 0, XBUTTON2, MOUSEEVENTF_XDOWN	 , 0, 0, }},		// 8
	{ INPUT_MOUSE	, { 	   0, 0, XBUTTON2, MOUSEEVENTF_XUP		 , 0, 0, }},		// 9
	{ INPUT_KEYBOARD, { 	0xF0, 0, 0		 , KEYEVENTF_KEYUP		 , 0, 0, }},		// 10 Win+�J�[�\��or1,2,3���ɁA�ŏ��Ƀ_�~�[�ŉ����L�[�������ꂽ���Ƃɂ���
};


/** �쐬
 */
void CKbdMouseCtrl::create()
{
	::timeBeginPeriod(1);
	DWORD		threadId;
	s_hThread_ = ::CreateThread(NULL, 4*1024, (LPTHREAD_START_ROUTINE)CKbdMouseCtrl::run, (void*)NULL, 0, &threadId);
}


/** �J��
 */
void CKbdMouseCtrl::release()
{
	::CloseHandle(s_hThread_);
	::Sleep(3*SLEEP_COUNT);
	::timeEndPeriod(1);
}


/** �ʃX���b�h�ł̎��s
 */
void CKbdMouseCtrl::run()
{
	for (;;) {
		ctrl();
		::Sleep(SLEEP_COUNT);
	}
}


/// �L�[���͂Ń}�E�X�𐧌�
///
void CKbdMouseCtrl::ctrl()
{
	unsigned	uNow = DiaKbdMouseHook_mouseButton();
	// if (uNow) DEBUGPRINTF("%mouse now=%x\n", uNow);

	// �g���K�[�쐬
	unsigned	uOld  = s_uOld_;
	unsigned 	uTrig = uNow & (~uOld);

	// �������u��(�����[�X)���쐬
	unsigned 	uRel  = (~uNow) & uOld;

	// ����p�ɍT����
	s_uOld_ = uNow;

 #ifdef USE_LWINKEY
	// ���[�h�L�[�̒���.
	checkModeKey(uNow, uTrig);
 #endif

	// �}�E�X�ړ�
	moveMouse(uNow, uOld, uTrig);

	// �}�E�X�{�^���̏���.
	sendMouseButton(uTrig, uRel);
}


#ifdef USE_LWINKEY
/// ���[�h�E�L�[�̂��܍��킹.
///
void CKbdMouseCtrl::checkModeKey(unsigned uNow, unsigned uTrig)
{
	// ��Win�L�[�����@�\�L���Ŏ��s����ƁAWin�L�[����������ɃX�^�[�g���j���[���J���̂ŁA
	// ��u�ʂ̃L�[�����������Ƃɂ��āA���܂���
	if (uTrig & DIAKBDMOUSE_MOUSE_MODEKEY)						// �����ꂽ�����on
		s_bWin1st_ = true;
	else if ((uNow & DIAKBDMOUSE_MOUSE_MODEKEY) == 0)			// �����ꂽ�� off
		s_bWin1st_ = 0;
	if (s_bWin1st_ && (uNow & ~DIAKBDMOUSE_MOUSE_MODEKEY)) {	// Win�ȊO�̑��̃L�[�������ꂽ��A���܂����ȃL�[�𔭌�
		s_bWin1st_ = 0;
		::SendInput(1, const_cast<LPINPUT>(&s_input_mouseSendTbl_[10]), sizeof(INPUT));
			// 0xF0��IME�N�������Ƃ悭�Ȃ����������A�����������̂悤�Ȏ��ɉ������Ƃ͂Ȃ����낤�A��
	}
}
#endif


/// �}�E�X�ړ�
///
void CKbdMouseCtrl::moveMouse(unsigned uNow, unsigned uOld, unsigned uTrig)
{
	// �J�[�\���ړ��̂��߂̓���
	int tx  = 0;
	int ty  = 0;
	int mul = (uNow & DIAKBDMOUSE_MOUSE_SPEEDUP) ? 2 : 1;
	if (uNow & DIAKBDMOUSE_MOUSE_SPEEDCHG) {	// ������肿����ƂÂړ�.
		unsigned uRept = makeReptKey(uNow, uOld, uTrig);
		tx = -((uRept & DIAKBDMOUSE_MOUSE_LEFT) != 0) + ((uRept & DIAKBDMOUSE_MOUSE_RIGHT) != 0);
		ty = -((uRept & DIAKBDMOUSE_MOUSE_UP  ) != 0) + ((uRept & DIAKBDMOUSE_MOUSE_DOWN ) != 0);
		tx *= 4*mul;
		ty *= 4*mul;
	} else {								// ���������Ȃ߂炩�Ɉړ�.
		int 	dx = -((uNow  & DIAKBDMOUSE_MOUSE_LEFT) != 0) + ((uNow  & DIAKBDMOUSE_MOUSE_RIGHT) != 0);
		int 	dy = -((uNow  & DIAKBDMOUSE_MOUSE_UP  ) != 0) + ((uNow  & DIAKBDMOUSE_MOUSE_DOWN ) != 0);
		s_dgtXY2AnlgXY_.set(dx, dy);
		// �A�i���O�l�Ƃ��� -256�`256�̒l��Ԃ�
		float	ax = s_dgtXY2AnlgXY_.analogX();
		float	ay = s_dgtXY2AnlgXY_.analogY();
		tx	= LONG(ax*DLT*mul) >> 8;
		ty	= LONG(ay*DLT*mul) >> 8;
	}

	// ���͒l����������A�J�[�\�����ړ�
	if (tx | ty) {
		INPUT	input;
		input.type			 = INPUT_MOUSE;
		input.mi.mouseData	 = 65535;
		input.mi.dwFlags	 = MOUSEEVENTF_MOVE;
		input.mi.time		 = 0;
		input.mi.dwExtraInfo = 0;
		input.mi.dx			 = tx;
		input.mi.dy			 = ty;
		::SendInput(1, &input, sizeof(INPUT));
	}
}


/// �L�[���s�[�g����
///
unsigned CKbdMouseCtrl::makeReptKey(unsigned uNow, unsigned uOld, unsigned uTrig)
{
	enum { REPT_KEY = DIAKBDMOUSE_MOUSE_LEFT | DIAKBDMOUSE_MOUSE_RIGHT | DIAKBDMOUSE_MOUSE_UP | DIAKBDMOUSE_MOUSE_DOWN };
	enum { TIME1ST	= 32 };		// 8*30=256msec ���炢.
	enum { TIME2ND	=  8 };		// 8*8 = 64msec ���炢.
	unsigned uRept = uTrig;
	if (uNow == uOld) {
		++s_uReptCnt_;
		if (s_uReptCnt_ == TIME1ST) {
			uRept = uNow & REPT_KEY;
		} else if (s_uReptCnt_ == TIME1ST + TIME2ND) {
			uRept 		= uNow & REPT_KEY;
			s_uReptCnt_	= TIME1ST;
		}
	} else {
		s_uReptCnt_ = 0;
	}
	return uRept;
}


/// �}�E�X�{�^���̑�p.
///
void CKbdMouseCtrl::sendMouseButton(unsigned uTrig, unsigned uRel)
{
	if (uTrig & DIAKBDMOUSE_MOUSE_LBUTTON)  ::SendInput(1, const_cast<LPINPUT>(&s_input_mouseSendTbl_[ 0]), sizeof(INPUT));
	if (uRel  & DIAKBDMOUSE_MOUSE_LBUTTON)  ::SendInput(1, const_cast<LPINPUT>(&s_input_mouseSendTbl_[ 1]), sizeof(INPUT));
	if (uTrig & DIAKBDMOUSE_MOUSE_RBUTTON)  ::SendInput(1, const_cast<LPINPUT>(&s_input_mouseSendTbl_[ 2]), sizeof(INPUT));
	if (uRel  & DIAKBDMOUSE_MOUSE_RBUTTON)  ::SendInput(1, const_cast<LPINPUT>(&s_input_mouseSendTbl_[ 3]), sizeof(INPUT));
	if (uTrig & DIAKBDMOUSE_MOUSE_MBUTTON)  ::SendInput(1, const_cast<LPINPUT>(&s_input_mouseSendTbl_[ 4]), sizeof(INPUT));
	if (uRel  & DIAKBDMOUSE_MOUSE_MBUTTON)  ::SendInput(1, const_cast<LPINPUT>(&s_input_mouseSendTbl_[ 5]), sizeof(INPUT));
	if (uTrig & DIAKBDMOUSE_MOUSE_XBUTTON1) ::SendInput(1, const_cast<LPINPUT>(&s_input_mouseSendTbl_[ 6]), sizeof(INPUT));
	if (uRel  & DIAKBDMOUSE_MOUSE_XBUTTON1) ::SendInput(1, const_cast<LPINPUT>(&s_input_mouseSendTbl_[ 7]), sizeof(INPUT));
	if (uTrig & DIAKBDMOUSE_MOUSE_XBUTTON2) ::SendInput(1, const_cast<LPINPUT>(&s_input_mouseSendTbl_[ 8]), sizeof(INPUT));
	if (uRel  & DIAKBDMOUSE_MOUSE_XBUTTON2) ::SendInput(1, const_cast<LPINPUT>(&s_input_mouseSendTbl_[ 9]), sizeof(INPUT));
}

