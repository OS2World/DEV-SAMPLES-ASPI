// ----------------------------------------------------------------------
// Module ASPIDLL.H
// Declarations for ASPI routines in ASPIDLL.C
//
// Copyright (C) 1993, Brian Sawert.
// All rights reserved.
//
// ----------------------------------------------------------------------


#ifndef	_ASPIDLL_H						// check for multiple inclusion
#define _ASPIDLL_H

#include "aspi.h"						// ASPI definitions and constants


// -------------------- external functions --------------------

void far *MaptoReal(void far *pptr);	// map to real mode address
void far *MaptoProt(void far *rptr);	// map to protected mode address
DWORD AllocRealBuff(DWORD bytes);		// allocate real mode buffer
DWORD FreeRealBuff(DWORD SelSeg);		// free real mode buffer
int AspiCall(void far *aspiproc, aspi_req_t far *ar);	// DPMI function


// -------------------- external variables --------------------

extern DWORD dwPtr[3];					// returns from GlobalDOSAlloc


#endif

