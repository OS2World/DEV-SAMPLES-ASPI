// ----------------------------------------------------------------------
// Module ASPI.C
// ASPI manager interface routines.
//
// Copyright (C) 1993, Brian Sawert.
// All rights reserved.
//
// Notes:
//	Compile with MEDIUM or SMALL model for DOS, MEDIUM for DLL.
//	Compiling as DLL defines __DLL__ and defines _FAR as far.
//
// ----------------------------------------------------------------------

#if defined(__DLL__)					// DLL options
#include <windows.h>
#else
#include <stdlib.h>
#endif

#include <fcntl.h>
#include <mem.h>
#include <dos.h>
#include <io.h>
#include <string.h>

#if defined(__DLL__)					// DLL options
#include "aspidll.h"					// ASPI DLL routines and variables
#endif
#include "aspi.h"						// ASPI definitions and constants
#include "scsi.h"						// SCSI definitions and constants


// -------------------- defines and macros -------------------

#define BUSY_WAIT	0x1000;				// repeat count for busy check
#define IOCTL_READ	2					// IOCTL read function
#define ASPI_NAME	"SCSIMGR$"			// SCSI manager driver name


// -------------------- global variables -------------------

void (far *ASPIRoutine) (aspi_req_t far *);	// far address of ASPI routine
BYTE f_installed;						// flag for ASPI existence
BYTE aspi_stat;							// ASPI status byte
BYTE host_stat;							// host status byte
BYTE targ_stat;							// target status byte
BYTE host_count;						// number of host adapters
BYTE host_num;							// host adapter number (0 default)
BYTE host_id = -1;						// host SCSI ID

aspi_req_t _FAR *srb;					// SCSI Request Block pointers
abort_req_t _FAR *abortsrb;				// SCSI abort request structure
sense_block_t _FAR *srb_sense;			// pointer to SRB sense data


// -------------------- external variables -------------------

// -------------------- local functions -------------------

int aspi_func(aspi_req_t _FAR *ar);		// ASPI entry point function


// -------------------- external functions -------------------

// -------------------- function definitions -------------------


// ----------------------------------------------------------------------
// Routine to check for and initialize ASPI driver.
//
// Usage:	int FUNC aspi_open(void);
//
// Returns nonzero on success, 0 if ASPI driver not found or init failed.
// ----------------------------------------------------------------------

int FUNC aspi_open(void)
	{
	int dh;								// ASPI driver handle

	if (!f_installed)
		{								// not initialized
		if ((dh = open(ASPI_NAME, O_RDONLY | O_BINARY)) != -1)
			{							// open device driver
			if (ioctl(dh, IOCTL_READ, (void _FAR *) &ASPIRoutine,
				sizeof(ASPIRoutine)) == sizeof(ASPIRoutine))
				{						// got ASPI entry point
#if defined(__DLL__)					// DLL options
				dwPtr[0] = AllocRealBuff((DWORD) sizeof(aspi_req_t));
				dwPtr[1] = AllocRealBuff(sizeof(abort_req_t));

				if (dwPtr[0] != NULL && dwPtr[1] != NULL)
					{					// allocated SRB buffers
					srb = (aspi_req_t far *) MAKELP(LOWORD(dwPtr[0]), 0);
					abortsrb = (abort_req_t far *) MAKELP(LOWORD(dwPtr[1]), 0);
					f_installed++;		// set installed flag
					}
#else
				srb = (aspi_req_t *) malloc(sizeof(aspi_req_t));
				abortsrb = (abort_req_t *) malloc(sizeof(abort_req_t));
				if (srb != NULL && abortsrb != NULL)
					{					// allocated SRB buffers
					f_installed++;		// set installed flag
					}
#endif
				}
			close(dh);					// close device driver
			}
		}

	return(f_installed);
	}


// ----------------------------------------------------------------------
// Routine to close and clean up.
//
// Usage:	void FUNC aspi_close(void);
//
// Called with nothing.
// Returns nothing.
// ----------------------------------------------------------------------

void FUNC aspi_close(void)
	{

	if (f_installed)
		{								// already initialized
#if defined(__DLL__)					// DLL options
		dwPtr[0] = FreeRealBuff(dwPtr[0]);	// deallocate buffers
		dwPtr[1] = FreeRealBuff(dwPtr[1]);
#else
		free(srb);						// deallocate buffers
		free(abortsrb);
#endif
		srb = abortsrb = NULL;			// clear pointers
		f_installed = 0;				// clear installed flag
		}

	return;
	}


// ----------------------------------------------------------------------
// Routine to call ASPI driver.
//
// Usage:	int aspi_func(aspi_req_t _FAR *ar);
//	
// Called with pointer to SCSI Request Block (SRB).
// Returns nonzero on success, 0 on error.
// ----------------------------------------------------------------------

int aspi_func(aspi_req_t _FAR *ar)
	{
	int retval = 0;

	if (f_installed)
		{								// ASPI manager initialized
#if defined(__DLL__)					// DLL options
		aspi_req_t far *arptr;			// real mode pointer

		if ((arptr = (aspi_req_t far *) MaptoReal(ar)) != NULL)
			{							// got a valid real mode pointer
			retval = AspiCall(ASPIRoutine, arptr); // call ASPI through DPMI
			}
#else
		ASPIRoutine((aspi_req_t far *) ar);	// call ASPI manager
		retval++;
#endif
		}

	return(retval);
	}


// ----------------------------------------------------------------------
// Inquire the status of the host adapter.
//
// Usage:	int FUNC aspi_host_inq(char _FAR *idstr, BYTE _FAR *hprm);
//
// Called with pointers to buffer for manager ID string and host adapter
//	parameters.
// Returns number of host adapters on success, -1 on error.
//
// Note:
//	Host parameter support depends on vendor and hardware.
// ----------------------------------------------------------------------

int FUNC aspi_host_inq(char _FAR *idstr, BYTE _FAR *hprm)
	{
	int retval = -1;

	memset(srb, 0, sizeof(aspi_req_t));	// clear SRB
	srb->command = HOST_INQ;			// set command byte

	if (aspi_func(srb))
		{								// ASPI call succeeded
		if ((aspi_stat = srb->status) == REQ_NOERR)
			{							// request completed without error
			host_id = srb->su.s0.targid;	// save host SCSI ID
			if (idstr != NULL)
				{						// copy manager ID string
				strncpy(idstr, srb->su.s0.manageid, MAX_IDSTR);
				}
			if (hprm != NULL)
				{						// copy host adapter parameters
				memcpy(hprm, srb->su.s0.hostparams, MAX_IDSTR);
				}

			retval = srb->su.s0.numadapt;	// return host adapter count
			}
		}

	return(retval);
	}


// ----------------------------------------------------------------------
// Inquire device type for specified SCSI ID.
//
// Usage:	int FUNC aspi_devtype(BYTE id);
//
// Called with target ID.
// Returns device type on success, -1 on error.
// ----------------------------------------------------------------------

int FUNC aspi_devtype(BYTE id)
	{
	int retval = -1;

	memset(srb, 0, sizeof(aspi_req_t));	// clear SRB
	srb->command = GET_DEV;				// set command byte

	srb->hostnum = host_num;			// set host adapter number
	srb->su.s1.targid = id;				// set target SCSI ID

	if (aspi_func(srb))
		{								// ASPI call succeeded
		if ((aspi_stat = srb->status) == REQ_NOERR)
			{							// request completed without error
			retval = srb->su.s1.devtype;	// return device type code
			}
		}

	return(retval);
	}


// ----------------------------------------------------------------------
// Execute SCSI I/O through ASPI interface.
//
// Usage:	int FUNC aspi_io(BYTE _FAR *cdb, BYTE far *dbuff,
//			WORD dbytes, BYTE flags, BYTE id, WORD _FAR *stat);
//
// Called with pointer to data buffer, data buffer size, pointer to CDB,
//	request flags, and target ID.
// Returns ASPI status on success, -1 on error.  Fills stat variable
//	with host status in high byte, target status in low byte.
// ----------------------------------------------------------------------

int FUNC aspi_io(BYTE _FAR *cdb, BYTE far *dbuff, WORD dbytes,
	BYTE flags, BYTE id, WORD _FAR *stat)
	{
#if defined(__DLL__)					// DLL options
	void far *pptr, far *rptr;			// real and protected mode pointers
#endif
	int cdbsize;
	int timeout;						// timeout counter for polling
	int retval = -1;

	memset(srb, 0, sizeof(aspi_req_t));	// clear SRB
	srb->command = SCSI_IO;				// set command byte

	srb->hostnum = host_num;			// set host adapter number
	srb->su.s2.targid = id;				// set target SCSI ID
	srb->reqflags = flags;				// set request flags
	srb->su.s2.databufptr = dbuff;		// set pointer to data buffer
	srb->su.s2.datalength = dbytes;		// set data buffer length
	srb->su.s2.senselength = sizeof(sense_block_t);
										// set sense data buffer length
	cdbsize = sizeof(group_0_t);		// assume 6 byte CDB
	if (*((BYTE _FAR *) cdb) >= MIN_GRP_1 && *((BYTE _FAR *) cdb) < MIN_GRP_6)
		{								// CDB size is 10 bytes
		cdbsize = sizeof(group_1_t);
		}

	srb->su.s2.cdblength = cdbsize;		// set CDB length
	srb->su.s2.senselength = MAX_SENSE;	// sense sense data length

	memcpy(srb->su.s2.scsicdb, cdb, cdbsize);	// copy CDB to SRB
	srb_sense = srb->su.s2.scsicdb + cdbsize;	// point to sense data buffer

#if defined(__DLL__)					// DLL options
	if ((dwPtr[2] = AllocRealBuff((DWORD) dbytes)) != NULL)
		{								// allocated real mode buffer
		pptr = MAKELP(LOWORD(dwPtr[2]), 0);	// create protected pointer
		rptr = MAKELP(HIWORD(dwPtr[2]), 0);	// create real pointer

		memcpy(pptr, dbuff, dbytes);	// copy data to real buffer
		srb->su.s2.databufptr = rptr;	// set pointer to real mode buffer
#endif

	if (aspi_func(srb))
		{								// ASPI call succeeded
		timeout = BUSY_WAIT;			// set timeout counter
		
		while (srb->status == REQ_INPROG && timeout > 0)
			{							// request in progress - keep polling
			timeout--;					// decrement timeout counter
			}

		retval = aspi_stat = srb->status;	// save ASPI status

		if (aspi_stat != REQ_INPROG)
			{							// request completed
			host_stat = srb->su.s2.hoststat;	// save host status
			targ_stat = srb->su.s2.targstat;	// save target status
			*stat = ((WORD) host_stat << 8) | targ_stat;
										// return combined SCSI status
			}
		}

#if defined(__DLL__)					// DLL options
		memcpy(dbuff, pptr, dbytes);	// copy data back to protected buffer
		dwPtr[2] = FreeRealBuff(dwPtr[2]);	// free real mode buffer
		}
	else
		{								// memory allocate failed
		retval = -1;					// return error
		}
#endif

	return(retval);
	}


// ----------------------------------------------------------------------
// Abort pending SCSI I/O request.
//
// Usage:	int FUNC aspi_abort_io(void);
//
// Called with nothing.
// Returns ASPI status of aborted SRB on success, -1 on error.
// ----------------------------------------------------------------------

int FUNC aspi_abort_io(void)
	{
	int timeout;						// timeout counter for polling
	int retval = -1;

	memset(abortsrb, 0, sizeof(abort_req_t));	// clear abort SRB
	abortsrb->command = ABORT_IO;		// set command byte

	abortsrb->hostnum = host_num;		// set host adapter number
	abortsrb->s3.srbptr = (void far *) srb;	// point to current SRB

	if (aspi_func((aspi_req_t _FAR *) abortsrb))
		{								// ASPI call succeeded
		timeout = BUSY_WAIT;			// set timeout counter
		
		while (srb->status == REQ_INPROG && timeout > 0)
			{							// request in progress - keep polling
			timeout--;					// decrement timeout counter
			}

		retval = aspi_stat = srb->status;	// save ASPI status
		}

	return(retval);
	}


// ----------------------------------------------------------------------
// Reset SCSI device through ASPI driver.                           
//
// Usage:	int FUNC aspi_reset_dev(BYTE id);
//
//
// Called with target ID.
// Returns ASPI status on success, -1 on error.
// ----------------------------------------------------------------------

int FUNC aspi_reset_dev(BYTE id)
	{
	int timeout;						// timeout counter for polling
	int retval = -1;

	memset(srb, 0, sizeof(aspi_req_t));	// clear SRB
	srb->command = SCSI_RESET;			// set command byte

	srb->hostnum = host_num;			// set host adapter number
	srb->su.s4.targid = id;				// set target SCSI ID

	if (aspi_func(srb))
		{								// ASPI call succeeded
		timeout = BUSY_WAIT;			// set timeout counter
		
		while (srb->status == REQ_INPROG && timeout > 0)
			{							// request in progress - keep polling
			timeout--;					// decrement timeout counter
			}

		retval = aspi_stat = srb->status;	// save ASPI status

		if (aspi_stat != REQ_INPROG)
			{							// request completed
			host_stat = srb->su.s4.hoststat;	// save host status
			targ_stat = srb->su.s4.targstat;	// save target status
			}
		}

	return(retval);
	}


// ----------------------------------------------------------------------
// Set host adapter parameters.
//
// Usage:	int FUNC aspi_set_hostprm(BYTE _FAR *hprm, int hbytes);
//
// Called with pointer to vendor specific host parameters buffer.
// Returns ASPI status on success, -1 on error.
//
// Note:
//	Host parameter support depends on vendor and hardware.
// ----------------------------------------------------------------------

int FUNC aspi_set_hostprm(BYTE _FAR *hprm, int hbytes)
	{
	int nbytes;
	int retval = -1;

	memset(srb, 0, sizeof(aspi_req_t));	// clear SRB
	srb->command = HOST_SET;			// set command byte

	srb->hostnum = host_num;			// set host adapter number
	nbytes = (hbytes < MAX_IDSTR) ? hbytes : MAX_IDSTR;	// set transfer length
	memcpy(srb->su.s5.hostparams, hprm, nbytes);	// copy host parameters

	if (aspi_func(srb))
		{								// ASPI call succeeded
		retval = aspi_stat = srb->status;	// save ASPI status
		}

	return(retval);
	}


// ----------------------------------------------------------------------
// Get disk drive parameters.
//
// Usage:	int FUNC aspi_get_driveprm(BYTE id, BYTE _FAR *flags,
//	 BYTE _FAR *drvnum, int _FAR *heads, int _FAR *sectsize);
//
// Called with SCSI ID and pointers to flags byte, INT 13 drive number,
//	number of heads, bytes per sector.
// Returns ASPI status on success, -1 on error.
//
// Note:
//	This function is not supported by all manufacturers.
// ----------------------------------------------------------------------

int FUNC aspi_get_driveprm(BYTE id, BYTE _FAR *flags, BYTE _FAR *drvnum,
	int _FAR *heads, int _FAR *sectsize)
	{
	int retval = -1;

	memset(srb, 0, sizeof(aspi_req_t));	// clear SRB
	srb->command = DISK_INFO;			// set command byte

	srb->hostnum = host_num;			// set host adapter number
	srb->su.s6.targid = id;				// set target ID

	if (aspi_func(srb))
		{								// ASPI call succeeded
		if ((aspi_stat = srb->status) == REQ_NOERR)
			{							// request completed without error
			*flags = srb->su.s6.driveflags;	// return disk drive flags
			*drvnum = srb->su.s6.drivenum;	// return INT 13 drive number
			*heads = srb->su.s6.headtrans;	// return number of heads
			*sectsize = srb->su.s6.secttrans;	// return bytes per sector
			}

		retval = aspi_stat;				// return ASPI status
		}

	return(retval);
	}


// ----------------------------------------------------------------------
// Retrieve sense data from last ASPI I/O call.
//
// Usage:	int FUNC aspi_sense(BYTE _FAR *sb, int sbytes);
//
// Called with pointer to sense data buffer and data buffer length.
// Returns number of bytes transferred on success, -1 on error.
//
// Note:
//	Reads static data from last call where targ_stat was T_CHKSTAT.
// ----------------------------------------------------------------------

int FUNC aspi_sense(BYTE _FAR *sb, int sbytes)
	{
	int nbytes;
	int retval = -1;

	if (targ_stat == T_CHKSTAT)
		{								// check for valid sense data
		nbytes = (sbytes < srb->su.s2.senselength) ?
			sbytes : srb->su.s2.senselength;	// set transfer length
		memcpy(sb, srb_sense, nbytes);
										// copy sense data
		retval = nbytes;				// return byte count
		}

	return(retval);
	}

