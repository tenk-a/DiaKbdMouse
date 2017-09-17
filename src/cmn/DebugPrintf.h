/**
 *	@file	DebugPrintf.h
 *	@brief	win-api ��p�����ȈՂ�debug���O�o�͗pprintf
 */
#ifndef DEBUGPRINTF
#ifndef NDEBUG
#define DEBUGPRINTF(...)	do {			\
		char		___stRbUf[1024];		\
		wsprintf(___stRbUf, __VA_ARGS__);	\
		___stRbUf[1023] = 0;				\
		OutputDebugString(___stRbUf);		\
	} while (0)
#include <windows.h>
#else
#define DEBUGPRINTF(...)
#endif
#endif
