// ----------------------------------------------------------------------
// Module SCSI.H
// Declarations for SCSI constants and structures.
//
// Copyright (C) 1993, Brian Sawert.
// All rights reserved.
//
// ----------------------------------------------------------------------


#ifndef	_SCSI_H							// check for multiple inclusion
#define _SCSI_H


// ----------------------------------------------------------------------
// Equates and defines.
// ----------------------------------------------------------------------

#define MIN_GRP_1		0x20			// minimum group 1 command
#define MIN_GRP_6		0xc0			// minimum group 6 command
										// (vendor specific)
#define MAX_TARG_ID		6				// ID 7 reserved for host
#define SDM_VALID 		0x80			// mask for sense data valid bit
#define SDM_ERRCODE		0x1				// mask for sense data error code
#define SDM_EOM			0x40			// mask for sense data EOM bit
#define SDM_ILI			0x20			// mask for sense data ILI bit
#define SDM_SKEY		0x7				// mask for sense data sense key
#define IDM_PTYPE		0x1ff			// mask for inquiry peripheral type
#define IDM_PTQUAL		0xe00			// mask for inquiry peripheral qualifier
#define IDM_DTQUAL		0x7ff			// mask for inquiry device qualifier
#define IDM_DATAFMT		0x7				// mask for inquiry response format
#define DIAG_STEST		0x4				// bit for self test in diagnostic
#define MODE_PG_FMT		0x10			// page format bit for mode select


// ----------------------------------------------------------------------
// SCSI structure definitions.
// ----------------------------------------------------------------------

// -------------------- SCSI Command Descriptor Block (CDB) --------------------

typedef struct group_0
	{									// 6 byte CDB
	BYTE opcode;						// command code
	BYTE lun;							// logical unit number
	BYTE params[3];						// command specific parameters
	BYTE reserved;						// reserved byte
	} group_0_t;

typedef struct group_1
	{									// 10 byte CDB
	BYTE opcode;						// command code
	BYTE lun;							// logical unit number
	BYTE params[7];						// command specific parameters
	BYTE reserved;						// reserved byte
	} group_1_t;


// -------------------- SCSI Sense Data Block --------------------

typedef struct sense_block
	{									// sense data block
	BYTE errcode;						// error code
										// bit 7 = valid
										// bits 0-6 = error code
										// (070h current, 071h deferred)	
	BYTE segment;						// segment number
	BYTE skey;							// sense key
										// bit 6 = EOM
										// bit 5 = ILI
										// bit 0-3 = sense key
	BYTE info[4];						// information bytes
										// (remaining bytes if ILI set)
	BYTE length;						// additional sense length (0ah)
	BYTE specific[4];					// command specific information
	BYTE scode;							// additional sense code
	BYTE scodequal;						// additional sense code qualifier
	BYTE frucode;						// field replaceable unit code
	BYTE skspec[3];						// sense key specific bytes
	} sense_block_t;


// -------------------- SCSI Inquiry Data Block --------------------

typedef struct inquire_block
	{									// block for inquiry data
	BYTE devtype;						// device type (6 for scanner)
	BYTE devqual;						// device type qualifier
	BYTE version;						// version codes
	BYTE format;						// data format codes
	BYTE length;						// additional data length
	BYTE reserved_1[2];					// reserved bytes
	BYTE capable;						// capability bit flags
	char vendid[8];						// vendor ID field
	char prodid[16];					// product ID field
	char revlev[4];						// product revision level
	char reserved_2[60];				// more reserved bytes
	} inquire_block_t;


// ----------------------------------------------------------------------
// SCSI command constants.
// ----------------------------------------------------------------------

#define SC_TEST_UNIT_READY	0x0			// test unit ready
#define SC_READ_SENSE		0x3			// request sense data
#define SC_READ_6			0x8			// Group 0 read command
#define SC_WRITE_6			0xa			// Group 0 write command
#define SC_INQUIRY			0x12		// SCSI device inquiry
#define SC_MODE_SELECT		0x15		// set mode information
#define SC_RESERVE_UNIT		0x16		// reserve SCSI device
#define SC_RELEASE_UNIT		0x17		// release SCSI device
#define SC_MODE_SENSE		0x1a		// request mode information
#define SC_SCAN_WINDOW		0x1b		// SCSI scan command
#define SC_SEND_DIAGNOSTIC	0x1d		// request target self check
#define SC_SET_WINDOW		0x24		// set window parameters
#define SC_GET_WINDOW		0x25		// get window parameters
#define SC_READ_G1			0x28		// Group 1 read command
#define SC_SEND_G1			0x2a		// Group 1 send command
#define SC_OBJECT_POSITION	0x31		// document feeder control
#define SC_DATA_STATUS		0x34		// get scan data status


// -------------------- vendor specific commands --------------------

#define SC_GET_ADF_STATUS	0xd3h		// get doc feeder status
#define SC_SET_ADF_MODE		0xd4h		// set doc feeder mode


// ----------------------------------------------------------------------
// SCSI sense key values.
// ----------------------------------------------------------------------

#define SK_NO_SENSE			0x0			// no sense data
#define SK_NOT_READY		0x2			// SCSI unit not ready
#define SK_MEDIUM_ERROR		0x3			// medium error (paper feed)
#define SK_HARDWARE_ERROR	0x4			// unrecoverable hardware error
#define SK_ILLEGAL_REQUEST	0x5			// illegal parameter in CDB
#define SK_UNIT_ATTENTION	0x6			// target has been reset
#define SK_ABORTED_COMMAND	0xb			// target aborted command

#endif
