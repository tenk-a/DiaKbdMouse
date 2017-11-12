/**
 *	@file	ConfigFileReader.h
 *	@brief	定義ファイルからキー定義を取得する.
 *	@author	Masashi Kitamura
 *	@date	2013-09
 */

#include "stdafx.h"
#include <cstring>
#include "DiaKbdMouse.h"
#include "ConfigFileReader.h"
#include "../cmn/FileTextReader.hpp"
#include "../cmn/binary_tbl_n.hpp"
#include "../cmn/DebugPrintf.h"


char const* CConfigFileReader::skip_spc(char const* s) {
	while (isspace(*s))
		++s;
	return s;
}


unsigned CConfigFileReader::getData()
{
	unsigned keyCode = 0;
	if (pFName_ == NULL || pFName_[0] == 0)
		return 0;
	FileTextReader	ftr;
	if (ftr.open(pFName_) == false) {
		errPrintf("%s をオープンできなかった\n", pFName_);
		return 0;
	}
	fline_ = 0;
	char		lbuf[4096];
	while (ftr.gets(lbuf, 4096)) {
		++fline_;
		char const* s = skip_spc(lbuf);
		if (*s == '#' || *s == '\0' || *s == '\n')
			continue;
		if (*s == '=') {
			s = skip_spc(s+1);
			if (keyCode == 0) {
				keyCode = getKey1(s, 0);
			} else {
				errPuts("拡張キーの指定が複数ある.\n");
			}
		} else if (*s == '+') {
			set1Data(0, s+1);
		} else if (*s == '*') {
			set1Data(1, s+1);
		} else {
			errPuts("余分な文字がある\n");
		}
	}
	return keyCode;
}

void 	CConfigFileReader::set1Data(bool qmode, const char* s) {
	unsigned keyCode = getKey1(s, 1);
	if (keyCode == 0) {
		//errPuts("設定するキー名が指定されていない.\n");
		return;
	}
	unsigned flags = 0;
	s = skip_spc(s);
	while (*s) {
		if      (*s == ':') flags |= F_DIRECT;
		else if (*s == '^') flags |= F_CTRL;
		else if (*s == '!') flags |= F_SHIFT;
	 #if 0 //あとで
		else if (*s == '@') flags |= F_ALT;
		else if (*s == '%') flags |= F_WIN;
	 #endif
		else break;
		++s;
	}
	unsigned tgtKey = getKey1(s, 2);
	if (tgtKey == 0) {
		//errPuts("発生させるキーが指定されていない.\n");
		return;
	}
	unsigned tgtMode = tgtKey >> 8;
	if (tgtMode == CDiaKbdMouseHook_ConvKey::MD_MOUSE && flags) {
		errPuts("マウス化キーに修飾キーを設定することはできない\n");
		return;
	}
	if      (flags == F_DIRECT)			tgtMode = CDiaKbdMouseHook_ConvKey::MD_DIRECT;
	else if (flags == F_CTRL )			tgtMode = CDiaKbdMouseHook_ConvKey::MD_CTRL;
	else if (flags == F_SHIFT)			tgtMode = CDiaKbdMouseHook_ConvKey::MD_SHIFT;
	else if (flags == (F_SHIFT|F_CTRL))	tgtMode = CDiaKbdMouseHook_ConvKey::MD_CTRLSHIFT;
	else if (tgtMode == 0)				tgtMode = CDiaKbdMouseHook_ConvKey::MD_USE;
	CDiaKbdMouseHook_ConvKey& rConvKey = rTbl_[ keyCode & 0xff ];
	rConvKey.oneKey_[qmode].u8VkCode_  = tgtKey;
	rConvKey.oneKey_[qmode].u8Mode_    = tgtMode;

	if (tgtMode == CDiaKbdMouseHook_ConvKey::MD_2ST_Q && qmode == 0) {
		rConvKey.oneKey_[1].u8VkCode_  = tgtKey;
		rConvKey.oneKey_[1].u8Mode_    = tgtMode;
	}

	s = skip_spc(s);
	if (*s != '\0' && *s != '#')
		errPuts("行末に余計な文字がある.\n");
}


unsigned CConfigFileReader::getKey1(const char*& rStr, unsigned mode) {
	static const char* s_modeStr[] = { "拡張キー", "元キー", "発生キー" };
	const char*        modeStr     = s_modeStr[mode];
	if (*rStr == 'x' && isxdigit(rStr[1])) {	// １６進数指定.
		unsigned n = strtol(rStr+1, (char**)&rStr, 16);
		if (n > 0 && n <= 255)
			return n;
		errPrintf("%s (%d): %s> キーコード 0x%02X(%d) は範囲外.\n", pFName_, fline_, modeStr, n, n);
	} else if (*rStr) {
		char name[1024];
		get_name(name, 1024, rStr);
		KeyNameVal knv;
		knv.name = name, knv.val = 0;
		unsigned n = binary_find_tbl_n( &s_keyNameValTbl_[0], s_keyNameValTblSize_, knv );
		if (n >= 0 && n < s_keyNameValTblSize_) {
			n =  s_keyNameValTbl_[n].val;
			if (mode < 2 && n > 0xff) {
				errPrintf("%s (%d): %s は %s として使えないキー名.\n", pFName_, fline_, name, modeStr);
				n = 0;
			}
			return n;
		}
		if (name[0] == 0) {
			errPrintf("%s (%d): %s> キーが指定されていない.\n", pFName_, fline_, modeStr);
		} else {
			errPrintf("%s (%d): %s> %s は知らないキー名.\n", pFName_, fline_, modeStr, name);
		}
	}
	return 0;
}

void CConfigFileReader::get_name(char* name, std::size_t sz, const char*& rStr) {
	const char* s  = rStr;
	char*		np = name;
	char*		ne = name + sz;
	while (*s && (isalnum(*s) || *s == '_') && np < ne) {
		*np++ = *s++;
	}
	*np  = 0;
	rStr = s;
}

void CConfigFileReader::errPuts(const char* str) {
	errPrintf("%s (%d) : %s", pFName_, fline_, str);
}


void CConfigFileReader::errPrintf(const char* fmt, ...) {
	if (!errFh_.is_open() && !errOpen_) {
		errOpen();
	}
	char str[8192];
	va_list a;
	va_start(a, fmt);
	vsprintf(str, fmt, a);
	va_end(a);
	if (errFh_.is_open()) {
		errFh_.puts(str);
	}
	OutputDebugString(str);
}


void CConfigFileReader::errOpen() {
	errOpen_ = true;
	char buf[FNAME_SZ+1];
	std::size_t l = strlen(pFName_);
	if (l < FNAME_SZ) {
		std::strncpy(&buf[0], pFName_, l); buf[l] = 0;
		std::strncpy(&buf[l-4], ".err", 5);
		errFh_.open(buf, FileHdl::WP);
		if (errFh_.is_open() == false)
			DEBUGPRINTF("%sをオープンできなかった\n", buf);
	}
}

/// 定義で指定するキーの名前. 名前順にソート済みであること.
CConfigFileReader::KeyNameVal const	CConfigFileReader::s_keyNameValTbl_[] = {
	{"0"		, 0x30 },
	{"1"		, 0x31 },
	{"2"		, 0x32 },
	{"3"		, 0x33 },
	{"4"		, 0x34 },
	{"5"		, 0x35 },
	{"6"		, 0x36 },
	{"7"		, 0x37 },
	{"8"		, 0x38 },
	{"9"		, 0x39 },
	{"A"		, 0x41 },
	{"ACCEPT"	, 0x1E },
	{"ADD"		, 0x6B },
	{"ALT"  	, 0x12 },
	{"APPS"		, 0x5D },
	{"ATMARK"	, 0x40 },
	{"B"		, 0x42 },
	{"BACK" 	, 0x08 },
	{"BACKSPACE", 0x08 },
	{"BROWSER_BACK"		, 0xA6 },
	{"BROWSER_FAVORITES", 0xAB },
	{"BROWSER_FORWARD"	, 0xA7 },
	{"BROWSER_HOME"		, 0xAC },
	{"BROWSER_REFRESH"	, 0xA8 },
	{"BROWSER_SEARCH"	, 0xAA },
	{"BROWSER_STOP"		, 0xA9 },
	{"C"		, 0x43 },
	{"CANCEL"	, 0x03 },
	{"CAPITAL"	, 0x14 },
	{"CAPSLOCK"	, 0x14 },
	{"CLEAR"	, 0x0C },
	{"CONTROL"	, 0x11 },
	{"CONVERT"	, 0x1C },
	{"CTRL" 	, 0x11 },
	{"D"		, 0x44 },
	{"DECIMAL"	, 0x6E },
	{"DELETE"	, 0x2E },
	{"DIVIDE"	, 0x6F },
	{"DOWN" 	, 0x28 },
	{"E"		, 0x45 },
	{"END"  	, 0x23 },
	{"ENTER"	, 0x0D },
	{"ESC"		, 0x1B },
	{"ESCAPE"	, 0x1B },
	{"EXECUTE"	, 0x2B },
	{"F"		, 0x46 },
	{"F1"		, 0x70 },
	{"F10"		, 0x79 },
	{"F11"		, 0x7A },
	{"F12"		, 0x7B },
	{"F13"		, 0x7C },
	{"F14"		, 0x7D },
	{"F15"		, 0x7E },
	{"F16"		, 0x7F },
	{"F17"		, 0x80 },
	{"F18"		, 0x81 },
	{"F19"		, 0x82 },
	{"F2"		, 0x71 },
	{"F20"		, 0x83 },
	{"F21"		, 0x84 },
	{"F22"		, 0x85 },
	{"F23"		, 0x86 },
	{"F24"		, 0x87 },
	{"F3"		, 0x72 },
	{"F4"		, 0x73 },
	{"F5"		, 0x74 },
	{"F6"		, 0x75 },
	{"F7"		, 0x76 },
	{"F8"		, 0x77 },
	{"F9"		, 0x78 },
	{"FINAL"	, 0x18 },
	{"G"		, 0x47 },
	{"H"		, 0x48 },
	{"HELP" 	, 0x2F },
	{"HOME" 	, 0x24 },
	{"I"		, 0x49 },
	{"INSERT"	, 0x2D },
	{"J"		, 0x4A },
	{"JUNJA"	, 0x17 },
	{"K"		, 0x4B },
	{"KANA" 	, 0x15 },
	{"KANJI"	, 0x19 },
	{"L"		, 0x4C },
	{"LALT"		, 0xA4 },
	{"LAUNCH_APP1"		, 0xB6 },
	{"LAUNCH_APP2"		, 0xB7 },
	{"LAUNCH_MAIL"		, 0xB4 },
	{"LAUNCH_MEDIA_SELECT",0xB5 },
	{"LBUTTON"	, 0x01 },
	{"LCONTROL"	, 0xA2 },
	{"LCTRL"	, 0xA2 },
	{"LEFT" 	, 0x25 },
	{"LMENU"	, 0xA4 },
	{"LSHIFT"	, 0xA0 },
	{"LWIN"		, 0x5B },
	{"M"		, 0x4D },
	{"MBUTTON"	, 0x04 },
	{"MEDIA_NEXT_TRACK"	, 0xB0 },
	{"MEDIA_PLAY_PAUSE"	, 0xB3 },
	{"MEDIA_PREV_TRACK"	, 0xB1 },
	{"MEDIA_STOP"		, 0xB2 },
	{"MENU" 	, 0x12 },
	{"MODECHANGE",0x1F },
	{"MULTIPLY"	, 0x6A },
	{"M_CUR2M"	, 0x07FF },
	{"M_DOWN"		, 0x0728 },
	{"M_LBUTTON"	, 0x0701 },
	{"M_LEFT"		, 0x0725 },
	{"M_MBUTTON"	, 0x0704 },
	{"M_NEXT"		, 0x0722 },
	{"M_PRIOR"		, 0x0721 },
	{"M_RBUTTON"	, 0x0702 },
	{"M_RIGHT"		, 0x0727 },
	{"M_UP"		, 0x0726 },
	{"M_XBUTTON1"	, 0x0705 },
	{"M_XBUTTON2"	, 0x0706 },
	{"N"		, 0x4E },
	{"NEXT" 	, 0x22 },
	{"NONCONVERT",0x1D },
	{"NUMLOCK"	, 0x90 },
	{"NUMPAD0"	, 0x60 },
	{"NUMPAD1"	, 0x61 },
	{"NUMPAD2"	, 0x62 },
	{"NUMPAD3"	, 0x63 },
	{"NUMPAD4"	, 0x64 },
	{"NUMPAD5"	, 0x65 },
	{"NUMPAD6"	, 0x66 },
	{"NUMPAD7"	, 0x67 },
	{"NUMPAD8"	, 0x68 },
	{"NUMPAD9"	, 0x69 },
	{"O"		, 0x4F },
	{"OEM_1"	, 0xBA },	//	;:)
	{"OEM_2"	, 0xBF },	//	(US /?)
	{"OEM_3"	, 0xC0 },	//	(US `~)
	{"OEM_4"	, 0xDB },	//	(US [{)
	{"OEM_5"	, 0xDC },	//	(US \|)
	{"OEM_6"	, 0xDD },	//	(US ]})
	{"OEM_7"	, 0xDE },	//	(US '")
	{"OEM_8"	, 0xDF },
	{"OEM_COMMA", 0xBC },
	{"OEM_MINUS", 0xBD },
	{"OEM_PERIOD",0xBE },
	{"OEM_PLUS"	, 0xBB },
	{"P"		, 0x50 },
	{"PAGEDOWN"	, 0x22 },
	{"PAGEUP"	, 0x21 },
	{"PAUSE"	, 0x13 },
	{"PRINT"	, 0x2A },
	{"PRIOR"	, 0x21 },
	{"Q"		, 0x51 },
	{"R"		, 0x52 },
	{"RALT"		, 0xA5 },
	{"RBUTTON"	, 0x02 },
	{"RCONTROL"	, 0xA3 },
	{"RCTRL"	, 0xA3 },
	{"RETURN"	, 0x0D },
	{"RIGHT"	, 0x27 },
	{"RMENU"	, 0xA5 },
	{"RSHIFT"	, 0xA1 },
	{"RWIN"		, 0x5C },
	{"S"		, 0x53 },
	{"SCROLL"	, 0x91 },
	{"SELECT"	, 0x29 },
	{"SEPARATOR", 0x6C },
	{"SHIFT"	, 0x10 },
	{"SLEEP"	, 0x5F },
	{"SNAPSHOT"	, 0x2C },
	{"SPACE"	, 0x20 },
	{"SUBTRACT"	, 0x6D },
	{"T"		, 0x54 },
	{"TAB"  	, 0x09 },
	{"U"		, 0x55 },
	{"UP"   	, 0x26 },
	{"V"		, 0x56 },
	{"VOLUME_DOWN"		, 0xAE },
	{"VOLUME_MUTE"		, 0xAD },
	{"VOLUME_UP"		, 0xAF },
	{"W"		, 0x57 },
	{"X"		, 0x58 },
	{"XBUTTON1"	, 0x05 },
	{"XBUTTON2"	, 0x06 },
	{"Y"		, 0x59 },
	{"Z"		, 0x5A },
	{"exkey"	, 0x05FF },		// 2ストロークキー指定. *Qは指定しないこと(APPS+Q+Q は不可)
};

unsigned const	CConfigFileReader::s_keyNameValTblSize_ = sizeof(CConfigFileReader::s_keyNameValTbl_) / sizeof(CConfigFileReader::s_keyNameValTbl_[0]);
