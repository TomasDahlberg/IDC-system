#hdr
#
#	Copyright Control Instrumentation, Australia
#
#	FILE NAME:	make1.mak
#
#	AUTHOR:		AMAKE.EXE   :-)
#
#	DATE:		Thu Jun 04 13:23:40 1992
#
#	FILE DESCRIPTION:
#			This file compiles all the
#			Objects, then links them
#			together.
#

#
#   COMPILE DEPENDANTS
#

OBJ\net.r : \
	net.c \
	xcc -ixR $(*B).c -v=..\defs


#
#   LINK THE OBJECT FILES
#

idcio : \
	OBJ\net.r
	echo.
	echo.		Linking Idcio Trap Module
	echo.
	xcc -ixF=idcio $(*B).r

