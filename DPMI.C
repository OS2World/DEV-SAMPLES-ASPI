// ----------------------------------------------------------------------
// Module DPMI.C
// DLL routine for DPMI access to ASPI driver.
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
#include <mem.h>
#include "aspi.h"						// ASPI definitions and constants


// -------------------- defines and macros --------------------

#define DPMI_INT	0x31				// DPMI interrupt number
#define REAL_CALL	0x301				// real mode call function

typedef struct RealCall
	{									// struct for real mode call
	DWORD edi, esi, ebp, reserved, ebx, edx, ecx, eax;
	WORD flags, es, ds, fs, gs, ip, cs, sp, ss;
	} RealCall_t ;


// -------------------- global variables -------------------

// -------------------- external variables -------------------


// ----------------------------------------------------------------------
// Call ASPI manager through DPMI server.
//
// Usage:	int AspiCall(void far *aspiproc, aspi_req_t far *ar);
//
// Returns 1 on success, 0 otherwise.
// ----------------------------------------------------------------------

int AspiCall(void far *aspiproc, aspi_req_t far *ar)
	{
	RealCall_t rcs;						// real mode call struct

	int retval = 0;
	int npush;
	void far *sptr;

	memset(&rcs, 0, sizeof(RealCall_t));	// clear call structure

	rcs.cs = HIWORD(aspiproc);			// point to real mode proc
	rcs.ip = LOWORD(aspiproc);

	npush = sizeof(aspi_req_t far *) / sizeof(WORD);
										// words to pass on stack
	sptr = (void far *) &rcs;			// far pointer to call structure

	asm {
		push di							// save registers
		push es

		sub bx, bx						// don't reset A20 line
		mov cx, npush;					// number of words to copy to stack
		les di, sptr					// point ES:DI to call structure

		mov ax, REAL_CALL				// load function number

		push WORD PTR [ar + 2]			// push SRB address
		push WORD PTR [ar]

		int DPMI_INT					// call DPMI server

		pop ax							// restore stack count
		pop ax

		pop es							// restore registers
		pop di

		jc asm_exit						// DPMI error

		mov retval, 1					// return 1 for success
		}

	asm_exit:


	return(retval);
	}
