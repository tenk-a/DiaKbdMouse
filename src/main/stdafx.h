#pragma once


#define _WIN32_WINNT 0x0500     // WIN2000以降.
#define _WIN32_IE    0x0500
#define WIN32_LEAN_AND_MEAN     // Windows ヘッダーから使用されていない部分を除外.

#include <windows.h>
#include <MMSystem.h>           // timeBeginPeriodのため.
#ifdef _MSC_VER
#pragma comment( lib, "WinMM" )
#endif
#include <ShellAPI.h>

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string.h>
#include <ctype.h>
