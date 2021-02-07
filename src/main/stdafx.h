// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once


#define _WIN32_WINNT 0x0500     // WIN2000以降.
#define _WIN32_IE    0x0500
#define WIN32_LEAN_AND_MEAN     // Windows ヘッダーから使用されていない部分を除外します.

// Windows ヘッダー ファイル :
#include <windows.h>
#include <MMSystem.h>           // timeBeginPeriodのため.
#pragma comment( lib, "WinMM" )
#include <ShellAPI.h>

// C ランタイム ヘッダー ファイル.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string.h>
#include <ctype.h>
