/*
!	Setvar.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dos.h>
#include "commlib.h"
#include "asciidef.h"
#include "ibmkeys.h"
#include "xfer.h"
#include "_xfer.h"

#define TYP_NOTYPE	0
#define TYP_VAR		1
#define TYP_CALENDAR	2
#define TYP_COMMENT	3

#define NO_OF_CAL_ENTRIES 10

typedef struct {
  unsigned short day[NO_OF_CAL_ENTRIES];
  unsigned short stopday[NO_OF_CAL_ENTRIES];
  unsigned char color[NO_OF_CAL_ENTRIES];
  unsigned short start[NO_OF_CAL_ENTRIES];
  unsigned short stop[NO_OF_CAL_ENTRIES];
} CALENDAR;

CALENDAR cal;

/*
! 	our prototyped functions
*/
int isWhiteSpace(int x);
int parseInput(char *line, int *typ, char *name, double *value);
int readPacket(PORT *port, char *buf, int maxLen);
int parseReply(char **cmdPtr, int *node, int *idx, double *value);

/*
!	imported functions
*/
void parseCalendar(char *line, CALENDAR *cal, int entry);
void cvtCalendar2Str(CALENDAR *cal, char *line);
void cvtStr2Calendar(CALENDAR *cal, char *line);
void printCalendar(FILE *fp, char *name, CALENDAR *cal);

void usage(void)
{
  printf("SETVAR s�tter variabler i valfri IVT3208\n");
  printf("\n");
  printf("Use: SETVAR readfile [-option]\n");
  printf("         readfile: filename for backupfile\n");
  printf("Options:\n");
  printf("         [-2]    : Use COM2 for transfer, default is COM1\n");
  printf("         [-bn]   : Set baudrate n=1200..19200, default is 9600\n");
  printf("         [-dn]   : Directs all requests to specific node\n");
  printf("         [-?]    : Show this list\n");
}

void main(int noOfParams, char *param[])
{
  FILE *fpIn;
  long baudopt;
  int node = 0, comPort = 1, tmpNode;
  long bpsSpeed = 9600L;
  char inFile[60];      	/* file name */
  int optionState = 0;
  char cmd[256], *cmdPtr;
  int typ, nextTyp, len, i, noOfVars;
  char name[32];
  char *tmpNamePtr;
  char inBuf[256];
  char line[256];
  short VERBOSE = 0;
  PORT *port;
  double value;

#define PROGRAM_NAME	"setvar, version 1.2"
  fprintf(stderr, "%s\n", PROGRAM_NAME);
  while (noOfParams >= 2) {
    if (param[1][0] != '-' ) {
      switch (optionState++) {
	case 0:
	  strcpy(inFile, param[1]);
	  break;
      }
    } else {
      while (*++(param[1])) {
	switch (*param[1]) {
	  case '?':
	    usage();
	    exit(1);
	  case 'v':
	  case 'V':
	    VERBOSE = 1;
	    continue;
	  case '1':
	    comPort = 1;
	    continue;
	  case '2':
	    comPort = 2;
	    continue;
	  case 'd':
	  case 'D':
	    node = 999;
	      if (param[1][1]> '0' && param[1][1]<= '9') {
		node= param[1][1] - 0x30;
		param[1]++;
		if (param[1][1]>= '0' && param[1][1]<= '9') {
		  node= 10*node + param[1][1] - 0x30;
		  param[1]++;
		}
	      }

	    if (node > 63) {
	      usage();
	      exit(1);
	    }
	    continue;
	  case 'b':
	  case 'B':
	    baudopt = 999;
	      if (param[1][1]> '0' && param[1][1]<= '9') {
		baudopt= param[1][1] - 0x30;
		param[1]++;
		if (param[1][1]>= '0' && param[1][1]<= '9') {
		  baudopt= 10*baudopt + param[1][1] - 0x30;
		  param[1]++;
		}
		if (param[1][1]>= '0' && param[1][1]<= '9') {
		  baudopt= 10*baudopt + param[1][1] - 0x30;
		  param[1]++;
		}
		if (param[1][1]>= '0' && param[1][1]<= '9') {
		  baudopt= 10*baudopt + param[1][1] - 0x30;
		  param[1]++;
		}
		if (param[1][1]>= '0' && param[1][1]<= '9') {
		  baudopt= 10*baudopt + param[1][1] - 0x30;
		  param[1]++;
		}
	      }
	    if (baudopt > 19200) {
	      usage();
	      exit(1);
	    }
		bpsSpeed = baudopt;
	    continue;
	  default:
	    printf("illegal option: %c\n", (char *) *param[1]);
	    usage();
	    exit(1);
	} /* end switch */
      } /* end while */
    } /* end if '-' */
    param++;
    noOfParams--;
  }
  if (optionState == 0) {
	usage();
	exit(1);
  }

/*
! 	open input file
*/
  if ((fpIn = fopen(inFile, "r")) == NULL) {
	printf("cannot open input file '%s'\n", inFile);
	exit(1);
  }

/*
!	open communication port
*/
  port = PortOpenGreenleaf(comPort == 1 ? COM1 : COM2, bpsSpeed, 'N', 8, 1);
  if (port->status < ASSUCCESS) {
	printf("Failed to open the port, status = %d\n", port->status);
	exit(1);
  }

/*
! 	for each line
!		parse input
!		send 'setcal' or 'var = value'
*/
  while (1) {
    cmdPtr = cmd;
    *cmdPtr = 0;
    nextTyp = TYP_NOTYPE;

    if (!fgets(line, 80, fpIn)) {
	break;
    }

    if (!parseInput(line, &nextTyp, name, &value))
	break;
    if (nextTyp == TYP_COMMENT)
	continue;

    if (nextTyp == TYP_CALENDAR) {
	for (i = 0; i < 10; i ++) {
		if (!fgets(line, 80, fpIn)) {
			break;
		}
		parseCalendar(line, &cal, i);
	}
	fgets(line, 80, fpIn);		/* skip ending brace */
	cvtCalendar2Str(&cal, line);
// added 941109
	tmpNode = node;
	tmpNamePtr = name;
	if (isdigit(*tmpNamePtr)) {
		tmpNode = atoi(tmpNamePtr);
		while (isdigit(*tmpNamePtr))
			tmpNamePtr++;
	}
//
	sprintf(cmd, "%d setcal %s %s\015\012", tmpNode, tmpNamePtr, line);
	printf("calendar %s...\r", name);
    } else {
      sprintf(cmd, "%d %s = %g\015\012", node, name, value);
      printf("s�tter %s = %g...\r", name, value);
    }
    if (*cmd == 0)
	break;

    if (VERBOSE) printf("Sent:\n'%s'\n", cmd);

/*
!	output command...
*/
    cmdPtr = cmd;
    while (*cmdPtr)
	WriteChar(port, (int) *cmdPtr++);

/*
!	... and wait for reply
*/
    if (!(len = readPacket(port, inBuf, 255))) {
	printf("read timeout\n");
	break;
    }
    if (VERBOSE) printf("Read %d bytes: \n%s\n\n", len, inBuf);

    if (nextTyp == TYP_CALENDAR) {
	printf("calendar %s [ok]\n", name);
    } else {
      printf("s�tter %s = %g [ok]\n", name, value);
    }

  }	/* while for ever */
  PortClose(port);
  fclose(fpIn);
}

/* parses '0 1 3.1415 ...' */
int parseReply(char **cmdPtr, int *node, int *idx, double *value)
{
  int sign;

  while (isWhiteSpace(**cmdPtr))
	(*cmdPtr)++;
  if (!isdigit(**cmdPtr))
	return 0;
  *node = *(*cmdPtr)++ - '0';
  if (**cmdPtr != ' ') {
    if (!isdigit(**cmdPtr))
	return 0;
    *node = (*node * 10) + *(*cmdPtr)++ - '0';
  }
  if (**cmdPtr != ' ') {
    if (!isdigit(**cmdPtr))
	return 0;
    *node = (*node * 10) + *(*cmdPtr)++ - '0';
  }
  if (**cmdPtr != ' ')
	return 0;
  (*cmdPtr)++;
/*
!	now take index
*/
  while (isWhiteSpace(**cmdPtr))
	(*cmdPtr)++;
  if (**cmdPtr == '-')
    sign = -1;
  else
    sign = 1;
  if (!isdigit(**cmdPtr))
	return 0;
  *idx = *(*cmdPtr)++ - '0';
  if (**cmdPtr != ' ') {
    if (!isdigit(**cmdPtr))
	return 0;
    *idx = (*idx * 10) + *(*cmdPtr)++ - '0';
  }
  if (**cmdPtr != ' ') {
    if (!isdigit(**cmdPtr))
	return 0;
    *idx = (*node * 10) + *(*cmdPtr)++ - '0';
  }
  if (**cmdPtr != ' ')
	return 0;
  (*cmdPtr)++;
  *idx = *idx * sign;
/*
! 	now parse float value
*/
  while (isWhiteSpace(**cmdPtr))
	(*cmdPtr)++;

  *value = atof(*cmdPtr);

  while (!isWhiteSpace(**cmdPtr) && !iscntrl(**cmdPtr))
	(*cmdPtr)++;
  return 1;
}

int isWhiteSpace(int x)
{
  return x == ' ' || x == 0x09;
}

int parseInput(char *line, int *typ, char *name, double *value)
{
  if (*line == '*') {
	*typ = TYP_COMMENT;
	return 1;
  }
  *typ = TYP_VAR;

  while (isWhiteSpace(*line))
	line ++;
  if (!strnicmp(line, "CALENDAR", 8)) {
	*typ = TYP_CALENDAR;
	line += 8;
  }
  while (isWhiteSpace(*line))
	line ++;
  if (!isalpha(*line) && !isdigit(*line))
	return 0;

  while (isalpha(*line) || isdigit(*line) || *line == '_' || *line == '[' || *line == ']')
	*name ++ = *line ++;
  *name = 0;
  if (*typ == TYP_CALENDAR)
	return 1;

  while (isWhiteSpace(*line))
	line ++;
  if (*line == '=')
	line ++;
  else
	return 0;
  while (isWhiteSpace(*line))
	line ++;
  *value = atof(line);
  return 1;
}

int readPacket(PORT *port, char *buf, int maxLen)
{
    int timeout_timer = 5;
    int c, len = 0;

    for ( ; ; ) {
	c = ReadCharTimed(port, 1000);
	if (c == 0x0a)
	    break;
	if (c < 0) {
	    if (--timeout_timer == 0) {
		return 0;
	    }
            continue;
	}
	*buf ++ = c;
	len ++;
	if (len >= maxLen)
		break;
    }
    *buf = 0;
    return len;
}
