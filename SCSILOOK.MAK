# --------------------------------------------------------------------
# File ASPI.MAK
# Makefile for ASPI SCSI inventory utility.
#
# Copyright (C) 1993, Brian Sawert.
# All rights reserved.
#
# Notes:
#	Compatible with PolyMake.
# --------------------------------------------------------------------

INCPATH = c:\bc\include
LIBPATH	= c:\bc\lib

.PATH.obj = .\obj

.REMAKE

#note: set "d" to not null to invoke debugging info

CL = bcc				# C compiler
LINK = tlink				# linker
MODEL = s				# model size

# --------------------------------------------------------------------

%if "$(d)" != ""			# build a debug version
CFLAGS= -c -f- -k -N -m$(MODEL) -I$(INCPATH) -L$(LIBPATH) -v
LFLAGS= /c /m /v
%else					# build a normal version
CFLAGS= -c -f- -k -N -m$(MODEL) -I$(INCPATH) -L$(LIBPATH)
LFLAGS= /c
%endif

# --------------------------------------------------------------------

PROGNAME = scsilook
MODULES = scsilook aspi

# --------------------------------------------------------------------

OMODULES = $[f,,$(MODULES),obj]


# implicit rules

.c.obj :
	%if !%dir($(.PATH.obj))
		mkdir $(.PATH.obj)
	%endif
	-$(CL) $(CFLAGS) -o$@ $<
	%if %status > 0
		%exit %status
	%endif


# implicit dependencies

$(PROGNAME).exe:  $(OMODULES) $(MAKEFILE)
	-$(LINK) $(LFLAGS) <@<
	$(LIBPATH)\c0$(MODEL).obj +
	$[s," +\n",$[f,$(.PATH.obj),$(OMODULES),obj]]
	$@
	$*.map
	c$(MODEL).lib
<


# explicit dependencies

aspi.obj:	aspi.c aspi.h scsi.h
scsilook.obj:	scsilook.c aspi.h scsi.h

