// ----------------------------------------------------------------------
// Module SCSILOOK.C
// Test program to illustrate ASPI routines.
// Performs inquiry of attached SCSI devices, reporting device type,
// model, and manufacturer.
//
// Copyright (C) 1993, Brian Sawert.
// All rights reserved.
//
// ----------------------------------------------------------------------


#include <stdio.h>
#include <stdlib.h>
#include <mem.h>

#include "aspi.h"						// ASPI definitions and constants
#include "scsi.h"						// SCSI definitions and constants


// -------------------- defines and macros --------------------

#define asize(x)	sizeof((x)) / sizeof(*(x))


// -------------------- global variables --------------------

char copyright[] = "SCSILOOK 1.0 SCSI inventory utility.\n"
	"Copyright (C) 1993, Brian Sawert.\n";

char aspi_id[MAX_IDSTR + 1];
char *devtype[] =
	{									// SCSI device type descriptions
	"Disk Drive",
	"Tape Drive",
	"Printer",
	"Processor",
	"WORM Drive",
	"CD-ROM Drive",
	"Scanner",
	"Optical Drive",
	"Autochanger",
	"Communications Device"
	};

group_0_t inq_cdb;						// CDB for Device Inquiry command
inquire_block_t	inq_data;				// buffer for inquiry data


// -------------------- start of code --------------------

void print_dev_info(int id, inquire_block_t *ib)
	{
	int dtype;
	char *dstr;

	dtype = (ib->devtype & 0x17);		// extract device type
	dstr = (dtype < asize(devtype)) ? devtype[dtype] : "Unknown";

	printf("\nDevice ID:    %-d\n", id);
	printf("Device Type:  %s\n", dstr);
	printf("Vendor ID:    %.8s\n", ib->vendid);
	printf("Product ID:   %.16s\n", ib->prodid);

	return;
	}


main(int argc, char *argv[])
	{
	int nhosts;
	int devid;
	int devtype;
	int stat, ht_stat;


	printf("%s\n", copyright);			// print startup message

	if (aspi_open() == 0)
		{								// ASPI access failed
		printf("Error accessing ASPI driver - check installation.\n");
		exit(1);
		}

	if ((nhosts = aspi_host_inq(aspi_id, NULL)) <= 0)
		{								// no host adapters found
		printf("Error - no host adapters found.\n");
		exit(1);
		}
	else
		{								// print SCSI host information
		printf("%d host adapter(s) installed.\n", nhosts); 
		printf("ASPI manager ID:  %s.\n", aspi_id);
		}

	for (devid = 0; devid <= MAX_TARG_ID; devid++)
		{								// query each device on SCSI bus
		if ((devtype = aspi_devtype(devid)) != -1)
			{							// found a valid device
			memset(&inq_cdb, 0, sizeof(group_0_t));	// clear CDB
			inq_cdb.opcode = SC_INQUIRY;	// set inquire command code
			inq_cdb.params[2] = sizeof(inquire_block_t);

			stat = aspi_io(&inq_cdb, &inq_data, sizeof(inquire_block_t),
				RF_DREAD, devid, &ht_stat);	// call ASPI I/O function

			if (stat == -1)
				{						// ASPI request failed
				printf("ASPI request failure on SCSI ID %d.\n", devid);
				}
			else if (stat != REQ_NOERR)
				{						// ASPI returned error
				printf("ASPI error on SCSI ID %n.\n", devid);
				printf("ASPI status:    %x\n", stat);
				printf("Host status:    %x\n", (ht_stat >> 8));
				printf("Target status:  %x\n", (ht_stat & 0xf));
				}
			else
				{						// ASPI request succeeded
				print_dev_info(devid, &inq_data);	// print device info
				}
			}
		}

	exit(0);
	}




