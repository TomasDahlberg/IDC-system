/*
!	Getvar.c
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
int parseInput(char *line, int *typ, char *name);
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
  printf("GETVAR h„mtar variabler fr†n valfri IVT3208\n");
  printf("\n");
  printf("Use: GETVAR readfile writefile [-option]\n");
  printf("         readfile: filname for varlist\n");
  printf("         writefile: filename for backupfile\n");
  printf("Options:\n");
  printf("         [-2]    : Use COM2 for transfer, default is COM1\n");
  printf("         [-bn]   : Set baudrate n=1200..19200, default is 9600\n");
  printf("         [-dn]   : Directs all requests to specific node\n");
  printf("         [-?]    : Show this list\n");
}

void main(int noOfParams, char *param[])
{
  FILE *fpIn, *fpOut;
  long baudopt;
  int node = 0, comPort = 1, tmpNode;
  long bpsSpeed = 9600L;
  char inFile[60], outFile[60];      	/* file name */
  int optionState = 0;
  char cmd[256], *cmdPtr;
  int lineCached;
  int typ, nextTyp, len, i, noOfVars;
  char name[6][40];
  char *tmpNamePtr;
  char inBuf[256];
  char line[80];
  short VERBOSE = 0;
  PORT *port;

#define PROGRAM_NAME	"getvar, version 1.2"
  fprintf(stderr, "%s\n", PROGRAM_NAME);
  while (noOfParams >= 2) {
    if (param[1][0] != '-' ) {
      switch (optionState++) {
	case 0:
	  strcpy(inFile, param[1]);
	  break;
	case 1:
	  strcpy(outFile, param[1]);
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
  if (optionState < 2) {
	usage();
	exit(1);
  }

/*
! 	open input file and create output file
*/
  if ((fpIn = fopen(inFile, "r")) == NULL) {
	printf("cannot open input file '%s'\n", inFile);
	exit(1);
  }
  if ((fpOut = fopen(outFile, "w")) == NULL) {
	printf("cannot create output file '%s'\n", outFile);
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
!		if none calendar
!			concat up to six requests
!		if calendar
!			if var request already started
!				cache line
!			else
!				form getcal request
*/
  lineCached = 0;
  while (1) {
    cmdPtr = cmd;
    *cmdPtr = 0;
    typ = TYP_NOTYPE;
    for (i = 0; i < 6; i++)
    {
      if (!lineCached && !fgets(line, 80, fpIn)) {
	break;
      }
      lineCached = 0;
      if (!parseInput(line, &nextTyp, name[i]))
	break;
      if (nextTyp == TYP_COMMENT) {
	i --;
	continue;
      }
      if (nextTyp == TYP_CALENDAR) {
	if (i != 0) {
	  lineCached = 1;
	  break;
	}
	typ = nextTyp;
// added 941109
	tmpNode = node;
	tmpNamePtr = name[i];
	if (isdigit(*tmpNamePtr)) {
		tmpNode = atoi(tmpNamePtr);
		while (isdigit(*tmpNamePtr))
			tmpNamePtr++;
	}
//
	sprintf(cmdPtr, "%d getcal %s", tmpNode, tmpNamePtr);
	cmdPtr += strlen(cmdPtr);
	printf("calendar %s...\r", name[i]);
	break;
      }
      typ = nextTyp;
      sprintf(cmdPtr, "%d %d %s & ", node, i + 1, name[i]);
      cmdPtr += strlen(cmdPtr);
      printf("%s = ?, ", name[i]);
    }	/* end of for loop */
/*
!	remove any terminating ' & '
*/
    if (*(cmdPtr - 2) == '&')
	cmdPtr -= 3;
    if (*cmd == 0)
	break;

    *cmdPtr++ = 0x0d;
    *cmdPtr++ = 0x0a;
    *cmdPtr = 0x00;
    if (VERBOSE) printf("Sent:\n'%s'\n", cmd);

/*
!	output command...
*/
    noOfVars = i;
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

/*
!	if calendar
!		output calendar information
!	else
!		output value
*/
    if (typ == TYP_CALENDAR) {
	printf("calendar %s [ok]\n", name[i]);
	cvtStr2Calendar(&cal, inBuf);
	printCalendar(fpOut, name[0], &cal);
    } else {
      cmdPtr = inBuf;
      printf("\r");
      for (i = 0; i < noOfVars; i ++) {
	int node, idx;
	double value;

	if (!parseReply(&cmdPtr, &node, &idx, &value)) {
		printf("syntax error, bad answer\n");
		break;
	}
	if (idx < 1) {
		printf("not found...\n");
	}
//	fprintf(fpOut, "%d%s = %g\n", node, name[idx - 1], value);
	fprintf(fpOut, "%s = %g\n", name[idx - 1], value);
	if (VERBOSE) printf("%s = %g\n", name[idx - 1], value);
	printf("%s = %g, ", name[i], value);
      }
      printf("\n");
    }
  }	/* while for ever */
  PortClose(port);
  fclose(fpIn);
  fclose(fpOut);
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

int parseInput(char *line, int *typ, char *name)
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
