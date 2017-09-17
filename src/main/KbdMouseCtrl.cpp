/**
 *	@file	KbdMouseCtrl.cpp
 *	@brief	キーボードでマウス操作する処理(別スレッド)
 *	@auther	Masashi KITAMURA
 *	@date	2006
 *	@note
 *		別にスレッドわける必要なかったかも...
 *		だし、timeBeginPeriod()とかもいらんか...
 *		が、チェックしなおすの面倒なんでそのまま
 *
 *		フリーソース
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


/// マウス化するための情報
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
	{ INPUT_KEYBOARD, { 	0xF0, 0, 0		 , KEYEVENTF_KEYUP		 , 0, 0, }},		// 10 Win+カーソルor1,2,3時に、最初にダミーで何かキーが押されたことにする
};


/** 作成
 */
void CKbdMouseCtrl::create()
{
	::timeBeginPeriod(1);
	DWORD		threadId;
	s_hThread_ = ::CreateThread(NULL, 4*1024, (LPTHREAD_START_ROUTINE)CKbdMouseCtrl::run, (void*)NULL, 0, &threadId);
}


/** 開放
 */
void CKbdMouseCtrl::release()
{
	::CloseHandle(s_hThread_);
	::Sleep(3*SLEEP_COUNT);
	::timeEndPeriod(1);
}


/** 別スレッドでの実行
 */
void CKbdMouseCtrl::run()
{
	for (;;) {
		ctrl();
		::Sleep(SLEEP_COUNT);
	}
}


/// キー入力でマウスを制御
///
void CKbdMouseCtrl::ctrl()
{
	unsigned	uNow = DiaKbdMouseHook_mouseButton();
	// if (uNow) DEBUGPRINTF("%mouse now=%x\n", uNow);

	// トリガー作成
	unsigned	uOld  = s_uOld_;
	unsigned 	uTrig = uNow & (~uOld);

	// 放した瞬間(リリース)を作成
	unsigned 	uRel  = (~uNow) & uOld;

	// 次回用に控える
	s_uOld_ = uNow;

 #ifdef USE_LWINKEY
	// モードキーの調整.
	checkModeKey(uNow, uTrig);
 #endif

	// マウス移動
	moveMouse(uNow, uOld, uTrig);

	// マウスボタンの処理.
	sendMouseButton(uTrig, uRel);
}


#ifdef USE_LWINKEY
/// モード・キーのつじつま合わせ.
///
void CKbdMouseCtrl::checkModeKey(unsigned uNow, unsigned uTrig)
{
	// 左Winキーを元機能有効で実行すると、Winキーを放した時にスタートメニューが開くので、
	// 一瞬別のキーを押したことにして、ごまかす
	if (uTrig & DIAKBDMOUSE_MOUSE_MODEKEY)						// 押された初回にon
		s_bWin1st_ = true;
	else if ((uNow & DIAKBDMOUSE_MOUSE_MODEKEY) == 0)			// 放されたら off
		s_bWin1st_ = 0;
	if (s_bWin1st_ && (uNow & ~DIAKBDMOUSE_MOUSE_MODEKEY)) {	// Win以外の他のキーも押されたら、ごまかしなキーを発効
		s_bWin1st_ = 0;
		::SendInput(1, const_cast<LPINPUT>(&s_input_mouseSendTbl_[10]), sizeof(INPUT));
			// 0xF0はIME起動中だとよくないかもだが、そもそもそのような時に押すことはないだろう、で
	}
}
#endif


/// マウス移動
///
void CKbdMouseCtrl::moveMouse(unsigned uNow, unsigned uOld, unsigned uTrig)
{
	// カーソル移動のための入力
	int tx  = 0;
	int ty  = 0;
	int mul = (uNow & DIAKBDMOUSE_MOUSE_SPEEDUP) ? 2 : 1;
	if (uNow & DIAKBDMOUSE_MOUSE_SPEEDCHG) {	// ゆっくりちょっとづつ移動.
		unsigned uRept = makeReptKey(uNow, uOld, uTrig);
		tx = -((uRept & DIAKBDMOUSE_MOUSE_LEFT) != 0) + ((uRept & DIAKBDMOUSE_MOUSE_RIGHT) != 0);
		ty = -((uRept & DIAKBDMOUSE_MOUSE_UP  ) != 0) + ((uRept & DIAKBDMOUSE_MOUSE_DOWN ) != 0);
		tx *= 4*mul;
		ty *= 4*mul;
	} else {								// そこそこなめらかに移動.
		int 	dx = -((uNow  & DIAKBDMOUSE_MOUSE_LEFT) != 0) + ((uNow  & DIAKBDMOUSE_MOUSE_RIGHT) != 0);
		int 	dy = -((uNow  & DIAKBDMOUSE_MOUSE_UP  ) != 0) + ((uNow  & DIAKBDMOUSE_MOUSE_DOWN ) != 0);
		s_dgtXY2AnlgXY_.set(dx, dy);
		// アナログ値として -256～256の値を返す
		float	ax = s_dgtXY2AnlgXY_.analogX();
		float	ay = s_dgtXY2AnlgXY_.analogY();
		tx	= LONG(ax*DLT*mul) >> 8;
		ty	= LONG(ay*DLT*mul) >> 8;
	}

	// 入力値があったら、カーソルを移動
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


/// キーリピート生成
///
unsigned CKbdMouseCtrl::makeReptKey(unsigned uNow, unsigned uOld, unsigned uTrig)
{
	enum { REPT_KEY = DIAKBDMOUSE_MOUSE_LEFT | DIAKBDMOUSE_MOUSE_RIGHT | DIAKBDMOUSE_MOUSE_UP | DIAKBDMOUSE_MOUSE_DOWN };
	enum { TIME1ST	= 32 };		// 8*30=256msec くらい.
	enum { TIME2ND	=  8 };		// 8*8 = 64msec くらい.
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


/// マウスボタンの代用.
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

