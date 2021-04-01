// ----------------------------------------------------------------------
// Module ASPIDLL.C
// Dynamic Link Library code for ASPI under Windows.
//
// Copyright (C) 1993, Brian Sawert.
// All rights reserved.
//
// Notes:
//	Compile with MEDIUM model for DLL.
//	Compiling as DLL defines __DLL__ and defines _FAR as far.
//
// ----------------------------------------------------------------------

#include <windows.h>

#include <fcntl.h>
#include <alloc.h>
#include <mem.h>
#include <dos.h>
#include <io.h>
#include <string.h>
#include <errno.h>

#include "aspi.h"						// ASPI definitions and constants
#include "scsi.h"						// SCSI definitions and constants


// -------------------- defines and macros -------------------

#define BUSY_WAIT	0x1000;				// repeat count for busy check
#define IOCTL_READ	2					// IOCTL read function
#define ASPI_NAME	"SCSIMGR$"			// SCSI manager driver name

#define asize(x)	sizeof((x)) / sizeof(*(x))


// -------------------- global variables -------------------

DWORD dwPtr[3];							// returns from GlobalDOSAlloc


// -------------------- external variables -------------------


// -------------------- local functions -------------------

void far *MaptoReal(void far *pptr);	// map to real mode address
void far *MaptoProt(void far *rptr);	// map to protected mode address
DWORD AllocRealBuff(DWORD bytes);		// allocate real mode buffer
DWORD FreeRealBuff(DWORD SelSeg);		// free real mode buffer


// -------------------- external functions -------------------


// -------------------- function definitions -------------------


#pragma argsused
#pragma option -w-aus

// ------------------------------------------------------------
// Windows DLL entry procedure.
// ------------------------------------------------------------

int FAR PASCAL _export LibMain(HANDLE hInstance, WORD wDataSeg, WORD wHeapSize,
	LPSTR lpszCmdLine)					// library initialization routine
	{
	if (wHeapSize != 0)					// just unlock data segment
		UnlockData(0);

	return 1;
	}


#pragma argsused
#pragma option -w-aus

// ------------------------------------------------------------
// Windows DLL exit procedure.
// ------------------------------------------------------------

int FAR PASCAL _export WEP(int bSystemExit)	// Windows exit procedure
	{
	return 1;							// just return success code
	}


// ----------------------------------------------------------------------
// Routine to map protected mode pointer to real mode.
//
// Usage:	void far *MaptoReal(void far *pptr);
//	
// Returns real mode pointer on success, NULL on error.
// ----------------------------------------------------------------------

void far *MaptoReal(void far *pptr)
	{
	WORD sel;							// protected mode selector
	void far *ptr = NULL;				// real mode pointer
	int count;

	sel = FP_SEG(pptr);					// get selector value

	for (count = 0; count < asize(dwPtr); count++)
		{								// loop through allocations
		if (sel == LOWORD(dwPtr[count]))
			{							// found matching selector
			ptr = MAKELP(HIWORD(dwPtr[count]), FP_OFF(pptr));
										// build real mode pointer
			break;
			}
		}

	return(ptr);
	}


// ----------------------------------------------------------------------
// Routine to map real mode pointer to protected  mode.
//
// Usage:	void far *MaptoProt(void far *rptr);
//	
// Returns protected mode pointer on success, NULL on error.
// ----------------------------------------------------------------------

void far *MaptoProt(void far *rptr)
	{
	WORD seg;							// real mode segment
	void far *ptr = NULL;				// protected mode pointer
	int count;

	seg = FP_SEG(rptr);					// get selector value

	for (count = 0; count < asize(dwPtr); count++)
		{								// loop through allocations
		if (seg == HIWORD(dwPtr[count]))
			{							// found matching segment
			ptr = MAKELP(LOWORD(dwPtr[count]), FP_OFF(rptr));
										// build protected mode pointer
			break;
			}
		}

	return(ptr);
	}


// ----------------------------------------------------------------------
// Routine to allocate real mode buffer.
//
// Usage:	DWORD AllocRealBuff(DWORD bytes);
//	
// Returns combined selector and segment on success, NULL on error.
// ----------------------------------------------------------------------

DWORD AllocRealBuff(DWORD bytes)
	{
	DWORD SelSeg;
	WORD sel;

	SelSeg = GlobalDosAlloc(bytes);		// allocate lower memory

	if (SelSeg != NULL)
		{								// allocated succeeded
		sel = LOWORD(SelSeg);			// extract selector
		if (GlobalPageLock(sel) == 0)
			{							// memory lock failed
			GlobalDosFree(sel);			// free memory
			SelSeg = NULL;				// clear return value
			}
		}

	return(SelSeg);						// return selector and segment
	}


// ----------------------------------------------------------------------
// Routine to free real mode buffer.
//
// Usage:	void FreeRealBuff(DWORD SelSeg);
//	
// Returns combined selector and segment.
// ----------------------------------------------------------------------

DWORD FreeRealBuff(DWORD SelSeg)
	{
	WORD sel;
	DWORD retval = SelSeg;

	sel = LOWORD(SelSeg);				// extract selector

	if (sel != NULL)
		{
		GlobalPageUnlock(sel);			// unlock memory
		GlobalDosFree(sel);				// free memory
		retval = 0L;
		}

	return(retval);
	}


