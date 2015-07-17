/****************************************************************************
*
*  (C) COPYRIGHT 2003-2004
*
*  FILE:        DebugUtil.h
*
*  VERSION:     1.0.2
*
*  DATE:        03/11/2004
*
*  DESCRIPTION:
*    Definitions for debug helper classes and functions.
*
*****************************************************************************/

#if !defined(_DEBUGUTIL_H__7E259C71_6727_4d8e_939D_57754834674D__INCLUDED_)
#define _DEBUGUTIL_H__7E259C71_6727_4d8e_939D_57754834674D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

inline NTSTATUS KeDelay(IN int Tms)
{
	LARGE_INTEGER dueTime;

	dueTime.QuadPart = (-10000 * Tms);
	return KeDelayExecutionThread(KernelMode, FALSE, &dueTime);
};

//#define _DBGU_ALWAYS

#if !defined(_DBGU_ALWAYS) && !defined(_DEBUG) && !(DBG)

#define DBGU_ONLY(f)
#define DBGU_PRINTF
#define DBGU_PRINTFEX
#define DBGU_ERROR
#define DBGU_WARNING
#define DBGU_INFO
#define DBGU_TRACE
#define DBGU_FN
#else

#define MAX_DBGOUTBUF_SIZEBYTES	256

#define PREFIX_ERROR	"[ERR ]"
#define PREFIX_WARNING	"[WARN]"
#define PREFIX_INFO		"[INFO]"
#define PREFIX_TRACE	""
#define PREFIX_FN		"[FUNC]"
#define POSTFIX_FN		"------"
#define PREFIX_FN2		"------"
#define POSTFIX_FN2		"------"

#if defined(__KERNEL_MODE__)
#define DBGU_PRINT	DbgPrint
#define DBGU_BREAK	DbgBreakPoint
#else
#define	DBGU_PRINT	OutputDebugString
#define DBGU_BREAK	DebugBreak
#endif

//
// Macros for mapping the debug functions
//
#define DBGU_ONLY(f)		f
#define DBGU_PRINTF			DbgUtilPrintf
#define DBGU_PRINTFEX		DbgUtilPrintfEx
#define DBGU_ERROR			DbgUtilError
#define DBGU_WARNING		DbgUtilWarning
#define DBGU_INFO			DbgUtilInfo
#define DBGU_TRACE			DbgUtilTrace
#define DBGU_FN(lpszFnName)	CDbgUtilFn __CDbgUtilFnObject(lpszFnName, __FILE__, __LINE__);

//
// Predefined debug flags
//
const DWORD DBGUFLAG_ERRORS					= 0x00000001;
const DWORD DBGUFLAG_WARNINGS				= 0x00000002;
const DWORD DBGUFLAG_INFOS					= 0x00000004;
const DWORD DBGUFLAG_TRACES					= 0x00000008;
const DWORD DBGUFLAG_FNS					= 0x00000010; // Function entry and exit
const DWORD DBGUFLAG_DUMP					= 0x00000100; // Dump data, not implement
const DWORD DBGUFLAG_PRINT_TIME				= 0x08000000; // Prints time for each message, not implement
const DWORD DBGUFLAG_PRINT_INFO				= 0x10000000; // Turns on thread, file, line info
const DWORD DBGUFLAG_DONT_LOG_TO_DEBUGGER	= 0x20000000; // not implement
const DWORD DBGUFLAG_DONT_LOG_TO_FILE		= 0x40000000; // not implement
const DWORD DBGUFLAG_BREAK_ON_ERRORS		= 0x80000000; // Do DebugBreak on errors

const DWORD DBGUFLAG_DONT_LOG = DBGUFLAG_DONT_LOG_TO_DEBUGGER | DBGUFLAG_DONT_LOG_TO_FILE;

//
// Declarations for global variables;
//
extern DWORD	g_dwDbgUtilFlags;

//
// Declarations for debug helper classes and functions
//
void __stdcall DbgUtilPrintf(LPCSTR lpszFmt, ...);
void __stdcall DbgUtilPrintfEx(DWORD dwFlags, LPCSTR lpszFmt, ...);
void __stdcall DbgUtilError(LPCSTR lpszFmt, ...);
void __stdcall DbgUtilWarning(LPCSTR lpszFmt, ...);
void __stdcall DbgUtilInfo(LPCSTR lpszFmt, ...);
void __stdcall DbgUtilTrace(LPCSTR lpszFmt, ...);

class CDbgUtilFn {
public:
	CDbgUtilFn(LPCSTR lpszFnName, LPCSTR lpszFileName, DWORD dwLineNumber)
	{
		m_lpszFnName = lpszFnName;
#if defined(__KERNEL_MODE__)
		m_dwIrql = (DWORD)KeGetCurrentIrql();
		m_dwProcessId = (m_dwIrql == PASSIVE_LEVEL) ? (DWORD)IoGetCurrentProcess() : 0;
		m_dwThreadId = (m_dwIrql <= DISPATCH_LEVEL) ? (DWORD)KeGetCurrentThread() : 0;

        if (g_dwDbgUtilFlags & DBGUFLAG_PRINT_INFO) {
			DbgUtilPrintfEx(DBGUFLAG_FNS,
				"%s%s Entering ==>> IRQL:%d, Process:0x%08x, Thread:0x%08x (%s, Ln %d)%s\n",
				PREFIX_FN, m_lpszFnName, m_dwIrql, m_dwProcessId, m_dwThreadId, lpszFileName, dwLineNumber, POSTFIX_FN);
		}
		else {
			DbgUtilPrintfEx(DBGUFLAG_FNS,
				"%s%s Entering ==>> IRQL:%d, Process:0x%08x, Thread:0x%08x%s\n",
				PREFIX_FN, m_lpszFnName, m_dwIrql, m_dwProcessId, m_dwThreadId, POSTFIX_FN);
		}
#else
		m_dwIrql = 0;
		m_dwProcessId = GetCurrentProcessId();
		m_dwThreadId = GetCurrentThreadId();

        if (g_dwDbgUtilFlags & DBGUFLAG_PRINT_INFO) {
			DbgUtilPrintfEx(DBGUFLAG_FNS,
				"%s%s Entering ==>> Process:0x%08x, Thread:0x%08x (%s, Ln %d)%s\n",
				PREFIX_FN, m_lpszFnName, m_dwProcessId, m_dwThreadId, lpszFileName, dwLineNumber, POSTFIX_FN);
		}
		else {
			DbgUtilPrintfEx(DBGUFLAG_FNS,
				"%s%s Entering ==>> Process:0x%08x, Thread:0x%08x%s\n",
				PREFIX_FN, m_lpszFnName, m_dwProcessId, m_dwThreadId, POSTFIX_FN);
		}
#endif
	}

	~CDbgUtilFn()
	{
#if defined(__KERNEL_MODE__)
		DbgUtilPrintfEx(DBGUFLAG_FNS,
			"%s%s Exiting ==>> IRQL:%d, Process:0x%08x, Thread:0x%08x%s\n",
			PREFIX_FN2, m_lpszFnName, m_dwIrql, m_dwProcessId, m_dwThreadId, POSTFIX_FN2);
#else
		DbgUtilPrintfEx(DBGUFLAG_FNS,
			"%s%s Exiting ==>> Process:0x%08x, Thread:0x%08x%s\n",
			PREFIX_FN2, m_lpszFnName, m_dwProcessId, m_dwThreadId, POSTFIX_FN2);
#endif
	}

private:
	LPCSTR	m_lpszFnName;
	DWORD	m_dwIrql;
	DWORD	m_dwProcessId;
	DWORD	m_dwThreadId;
};

#endif //!defined(_DBGU_ALWAYS) && !defined(_DEBUG) && !(DBG)

#endif // !defined(_DEBUGUTIL_H__7E259C71_6727_4d8e_939D_57754834674D__INCLUDED_)
