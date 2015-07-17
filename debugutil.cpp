/****************************************************************************
*
*  (C) COPYRIGHT 2003-2004
*
*  FILE:        DebugUtil.cpp
*
*  VERSION:     1.0.2
*
*  DATE:        03/11/2004
*
*  DESCRIPTION:
*    Implementation for debug helper classes and functions.
*
*****************************************************************************/

#if defined(__KERNEL_MODE__)
#include "SnCam.h"
#else
#include "DebugUtil.h"
#endif

#if defined(_DBGU_ALWAYS) || defined(_DEBUG) || (DBG)


DWORD g_dwDbgUtilFlags = DBGUFLAG_ERRORS | DBGUFLAG_WARNINGS | DBGUFLAG_INFOS | DBGUFLAG_TRACES;
//DWORD g_dwDbgUtilFlags = DBGUFLAG_DONT_LOG;

void __stdcall DbgUtilPrintf(LPCSTR lpszFmt, ...)
{
	va_list arglist;
	CHAR szBuf[MAX_DBGOUTBUF_SIZEBYTES];
	int nBuf;

	va_start(arglist, lpszFmt);

	nBuf = _vsnprintf(szBuf, sizeof(szBuf), lpszFmt, arglist);
	
	if (nBuf > 0) DBGU_PRINT(szBuf);

	va_end(arglist);
}

void __stdcall DbgUtilPrintfEx(DWORD dwFlags, LPCSTR lpszFmt, ...)
{
	va_list arglist;
	BOOL fLog;
	CHAR szBuf[MAX_DBGOUTBUF_SIZEBYTES];
	int nBuf;

    //
    // See if log messages are enabled and the flag is enabled
    //
	fLog = !(g_dwDbgUtilFlags & DBGUFLAG_DONT_LOG) && (g_dwDbgUtilFlags & dwFlags);
    
	if (fLog) {
		va_start(arglist, lpszFmt);

		nBuf = _vsnprintf(szBuf, sizeof(szBuf), lpszFmt, arglist);
		
		if (nBuf > 0) DBGU_PRINT(szBuf);

		va_end(arglist);
	}
}

void __stdcall DbgUtilError(LPCSTR lpszFmt, ...)
{
	va_list arglist;
	BOOL fLog;
	CHAR szBuf[MAX_DBGOUTBUF_SIZEBYTES] = PREFIX_ERROR;
	CHAR *pszMsg = &szBuf[sizeof(PREFIX_ERROR)-1];
	int nBuf;

    //
    // See if log messages are enabled and the flag is enabled
    //
	fLog = !(g_dwDbgUtilFlags & DBGUFLAG_DONT_LOG) && (g_dwDbgUtilFlags & DBGUFLAG_ERRORS);
    
	if (fLog) {
		va_start(arglist, lpszFmt);

		nBuf = _vsnprintf(pszMsg, sizeof(szBuf)-sizeof(PREFIX_ERROR)+1, lpszFmt, arglist);
		
		if (nBuf > 0) DBGU_PRINT(szBuf);

		va_end(arglist);
	}

    if (g_dwDbgUtilFlags & DBGUFLAG_BREAK_ON_ERRORS) {
		DBGU_BREAK();
    }
}

void __stdcall DbgUtilWarning(LPCSTR lpszFmt, ...)
{
	va_list arglist;
	BOOL fLog;
	CHAR szBuf[MAX_DBGOUTBUF_SIZEBYTES] = PREFIX_WARNING;
	CHAR *pszMsg = &szBuf[sizeof(PREFIX_WARNING)-1];
	int nBuf;

    //
    // See if log messages are enabled and the flag is enabled
    //
	fLog = !(g_dwDbgUtilFlags & DBGUFLAG_DONT_LOG) && (g_dwDbgUtilFlags & DBGUFLAG_WARNINGS);
    
	if (fLog) {
		va_start(arglist, lpszFmt);

		nBuf = _vsnprintf(pszMsg, sizeof(szBuf)-sizeof(PREFIX_WARNING)+1, lpszFmt, arglist);
		
		if (nBuf > 0) DBGU_PRINT(szBuf);

		va_end(arglist);
	}
}

void __stdcall DbgUtilInfo(LPCSTR lpszFmt, ...)
{
	va_list arglist;
	BOOL fLog;
	CHAR szBuf[MAX_DBGOUTBUF_SIZEBYTES] = PREFIX_INFO;
	CHAR *pszMsg = &szBuf[sizeof(PREFIX_INFO)-1];
	int nBuf;

    //
    // See if log messages are enabled and the flag is enabled
    //
	fLog = !(g_dwDbgUtilFlags & DBGUFLAG_DONT_LOG) && (g_dwDbgUtilFlags & DBGUFLAG_INFOS);
    
	if (fLog) {
		va_start(arglist, lpszFmt);

		nBuf = _vsnprintf(pszMsg, sizeof(szBuf)-sizeof(PREFIX_INFO)+1, lpszFmt, arglist);
		
		if (nBuf > 0) DBGU_PRINT(szBuf);

		va_end(arglist);
	}
}

void __stdcall DbgUtilTrace(LPCSTR lpszFmt, ...)
{
	va_list arglist;
	BOOL fLog;
	CHAR szBuf[MAX_DBGOUTBUF_SIZEBYTES] = PREFIX_TRACE;
	CHAR *pszMsg = &szBuf[sizeof(PREFIX_TRACE)-1];
	int nBuf;

    //
    // See if log messages are enabled and the flag is enabled
    //
	fLog = !(g_dwDbgUtilFlags & DBGUFLAG_DONT_LOG) && (g_dwDbgUtilFlags & DBGUFLAG_TRACES);
    
	if (fLog) {
		va_start(arglist, lpszFmt);

		nBuf = _vsnprintf(pszMsg, sizeof(szBuf)-sizeof(PREFIX_TRACE)+1, lpszFmt, arglist);
		
		if (nBuf > 0) DBGU_PRINT(szBuf);

		va_end(arglist);
	}
}

#endif //defined(_DBGU_ALWAYS) || defined(_DEBUG) || (DBG)