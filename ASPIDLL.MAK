# --------------------------------------------------------------------
# File ASPIDLL.MAK
# Makefile for ASPI Windows DLL.
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
LIB = tlib				# librarian
MODEL = m				# model size

# --------------------------------------------------------------------

%if "$(d)" != ""			# build a debug version
CFLAGS= -c -f- -2 -WD -m$(MODEL)! -I$(INCPATH) -L$(LIBPATH) -v
LFLAGS= /Twd /L$(LIBPATH) /v
%else					# build a normal version
CFLAGS= -c -f- -2 -WD -m$(MODEL)! -I$(INCPATH) -L$(LIBPATH)
LFLAGS= /Twd /L$(LIBPATH) /x
%endif

# --------------------------------------------------------------------

PROGNAME = aspidll
MODULES = aspi aspidll dpmi

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

$(PROGNAME).dll:  $(OMODULES) $(PROGNAME).def $(MAKEFILE)
	-$(LINK) $(LFLAGS) <@<
	$(LIBPATH)\c0d$(MODEL).obj +
	$[s," +\n",$[f,$(.PATH.obj),$(OMODULES),obj]]
	$@
	$*.map
	import.lib + cwl.lib
	$*.def
<
	implib $*.lib $*.dll


# explicit dependencies

aspi.obj:	aspi.c aspi.h scsi.h
aspidll.obj:	aspidll.c aspi.h scsi.h
dpmi.obj:	dpmi.c aspi.h

