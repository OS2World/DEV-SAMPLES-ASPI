// ----------------------------------------------------------------------
// Module ASPI.H
// Declarations for ASPI constants and structures.
//
// Copyright (C) 1993, Brian Sawert.
// All rights reserved.
//
// ----------------------------------------------------------------------


#ifndef	_ASPI_H							// check for multiple inclusion
#define _ASPI_H


// -------------------- constant definitions --------------------

#define MAX_CDB			10				// maximum CDB size
#define MAX_SENSE		32				// maximum sense data size
#define MAX_IDSTR		16				// maximum ID string size


// -------------------- type definitions --------------------

#if !defined(__WINDOWS_H)
typedef unsigned char	BYTE;			// variable type definitions
typedef unsigned int	WORD;
typedef unsigned long	DWORD;
#endif

// -------------------- ASPI command codes --------------------

#define HOST_INQ		0				// host adapter inquiry
#define GET_DEV			1				// get device type
#define SCSI_IO			2				// execute SCSI I/O command
#define ABORT_IO		3				// abort SCSI I/O command
#define SCSI_RESET		4				// reset SCSI device
#define HOST_SET		5				// set host adapter parameters
#define DISK_INFO		6				// get disk drive information


// -------------------- ASPI status codes --------------------

#define REQ_INPROG		0x0				// SCSI request in progress
#define REQ_NOERR		0x1				// SCSI request completed without error
#define REQ_ABORT		0x2				// SCSI request aborted by host
#define REQ_ERR			0x4				// SCSI request completed with error
#define BAD_REQ			0x80			// invalid SCSI request
#define BAD_HOST		0x81			// invalid host adapter number
#define BAD_DEV			0x82			// SCSI device not installed


// -------------------- SCSI request flag description --------------------

#define RF_POST			0x1				// posting enabled (bit 0)
#define RF_LINK			0x2				// linking enabled (bit 1)
#define RF_DCMD			0x0				// direction set by SCSI command
#define RF_DREAD		0x8				// transfer from SCSI target to host
#define RF_DWRITE		0x10			// transfer from host to SCSI target
#define RF_DNONE		0x18			// no transfer


// -------------------- host adapter status codes --------------------

#define H_OKAY			0x0				// host adapter did not detect error
#define H_TIMEOUT		0x11			// selection timeout
#define H_OVERRUN		0x12			// data overrun or underrun
#define H_BUSFREE		0x13			// unexpected bus free
#define H_SEQFAIL		0x14			// target bus phase sequence failure


// -------------------- target status codes --------------------

#define T_NOSTAT		0x0				// no target status
#define T_CHKSTAT		0x2				// check status (sense data available)
#define T_BUSY			0x8				// specified target/LUN is busy
#define T_CONF			0x18			// reservation conflict


// -------------------- disk drive flags --------------------

#define DF_NOINT13		0x0				// no INT 13 access
#define DF_INT13DOS		0x1				// INT 13 with access
#define DF_INT13NODOS	0x2				// INT 13 without DOS access
#define DF_INVALID		0x3				// invalid flags


// -------------------- SCSI Request Block (SRB) definitions --------------------

typedef struct srb_0
	{									// function 0:  Host adapter inquiry
	BYTE numadapt;						// number of host adapters (R)
	BYTE targid;						// target ID of host adapter (R)
	char manageid[MAX_IDSTR];			// SCSI manager ID string (R)
	char hostid[MAX_IDSTR];				// host adapter ID string (R)
	BYTE hostparams[MAX_IDSTR];			// host adapter unique parameters (R)
	} srb_0_t;

typedef struct srb_1
	{									// function 1:  Get device type
	BYTE targid;						// device target ID (W)
	BYTE lun;							// logical unit number (W)
	BYTE devtype;						// peripheral device type (R)
	} srb_1_t;

typedef struct srb_2
	{									// function 2:  Execute SCSI I/O
	BYTE targid;						// device target ID (W)
	BYTE lun;							// logical unit number (W)
	DWORD datalength;					// data allocation length (W)
	BYTE senselength;					// sense allocation length (N) (W)
	BYTE far *databufptr;				// data buffer pointer (W)
	void far *srblinkptr;				// SRB link pointer (W)
	BYTE cdblength;						// SCSI CDB length (M) (W)
	BYTE hoststat;						// host adapter status (R)
	BYTE targstat;						// target status (R)
	void far *postptr;					// POST routine address (R)
	BYTE workspace[34];					// reserved ASPI workspace
// -------------------- see notes below --------------------
	BYTE scsicdb[MAX_CDB];				// SCSI command descriptor block
	BYTE sensealloc[MAX_SENSE];			// sense data allocation area

// -------------------- notes on variable length data --------------------
//	scsicdb - SCSI command descriptor block (CDB) - M bytes.
//	Note:  Length will vary with CDB type (6 byte or 10 byte).
//	sensealloc - sense allocation area - N bytes.
//	Note:  Position and length will vary with CDB type.
// ----------------------------------------------------------------------

	} srb_2_t;

typedef struct srb_3
	{									// function 3:  Abort SCSI I/O command
	BYTE far *srbptr;					// address of SRB to abort (W)
	} srb_3_t;

typedef struct srb_4
	{									// function 4:  Reset SCSI device
	BYTE targid;						// device target ID (W)
	BYTE lun;							// logical unit number (W)
	BYTE reserved[14];					// reserved bytes
	BYTE hoststat;						// host adapter status (R)
	BYTE targstat;						// target status (R)
	void far *postptr;					// POST routine address (R)
	BYTE workspace[34];					// reserved ASPI workspace
	} srb_4_t;

typedef struct srb_5
	{									// function 5:  Set host parameters
	BYTE hostparams[MAX_IDSTR];			// host adapter unique parameters (R)
	} srb_5_t;

typedef struct srb_6
	{									// function 6: Get disk information
	BYTE targid;						// device target ID (W)
	BYTE lun;							// logical unit number (W)
	BYTE driveflags;					// flags for drive parameters
	BYTE drivenum;						// INT 13 drive number
	BYTE headtrans;						// preferred head number translation
	BYTE secttrans;						// preferred sector size translation
	BYTE reserved[10];					// reserved bytes
	} srb_6_t;

typedef struct aspi_req
	{									// combined SRB structure
	BYTE command;						// command code (W)
	BYTE status;						// return status (R)
	BYTE hostnum;						// host adapter number (W)
	BYTE reqflags;						// SCSI request flags (W)
	BYTE expand[4];						// reserved for expansion
	union srb_union
		{								// individual SRB's
		srb_0_t s0;						// function 0 SRB
		srb_1_t s1;						// function 1 SRB
		srb_2_t s2;						// function 2 SRB
		srb_3_t s3;						// function 3 SRB
		srb_4_t s4;						// function 4 SRB
		srb_5_t s5;						// function 5 SRB
		srb_6_t s6;						// function 6 SRB
		} su;
	} aspi_req_t;

typedef struct abort_req
	{									// ASPI abort request
	BYTE command;						// command code (W)
	BYTE status;						// return status (R)
	BYTE hostnum;						// host adapter number (W)
	BYTE reqflags;						// SCSI request flags (W)
	BYTE expand[4];						// reserved for expansion
	srb_3_t s3;							// function 3 SRB
	} abort_req_t;


// -------------------- ASPI function declarations --------------------

#if defined(__WINDOWS_H)				// declare DLL functions
#define FUNC	FAR PASCAL _export
#ifndef _FAR
#define _FAR	far
#endif
#else									// non-Windows definitions
#define FUNC
#ifndef _FAR
#define _FAR
#endif
#endif

int FUNC aspi_open(void);				// initialize ASPI manager
void FUNC aspi_close(void);				// close ASPI manager
int FUNC aspi_host_inq(char _FAR *idstr, BYTE _FAR *hprm);
										// get host adapter info
int FUNC aspi_devtype(BYTE id);			// get SCSI device type
int FUNC aspi_io(BYTE _FAR *cdb, BYTE far *dbuff, WORD dbytes,
	BYTE flags, BYTE id, WORD _FAR *stat);	// perform SCSI I/O
int FUNC aspi_abort_io(void);			// abort SCSI request
int FUNC aspi_reset_dev(BYTE id);		// reset device
int FUNC aspi_set_hostprm(BYTE _FAR *hprm, int hbytes);	// set host parameters
int FUNC aspi_get_driveprm(BYTE id, BYTE _FAR *flags, BYTE _FAR *drvnum,
	int _FAR *heads, int _FAR *sectsize);	// get SCSI disk drive parameters
int FUNC aspi_sense(BYTE _FAR *sb, int sbytes);	// return SCSI sense info

#endif
