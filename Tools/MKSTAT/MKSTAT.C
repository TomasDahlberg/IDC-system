/* mkStat.c  1993-09-07 TD,  version 1.0 */
/*
 * This file contains proprietary information of IVT Electronic AB.
 * Copying or reproduction without prior written approval is prohibited.
 *
 * This file is furnished under a license agreement or nondisclosure
 * agreement. The software may be used or copied only in accordance 
 * with the terms of the agreement.
 *
 * In no event will IVT Electronic AB, be liable for any lost revenue or
 * profits or other special, indirect and consequential damages, even if
 * IVT has been advised of the possibility of such damages.
 *
 * IVT Electronic AB
 * Box 996
 * 191 29 Sollentuna
 * Sweden
 */

/*
! mkStat.c
! Copyright (C) 1993 IVT Electronic AB.
*/
/*
!	This program mkStat makes a meta statistical definition module to
!	be loaded at address $0003f000
*/
/*
!	History
!	When	By Rev 	What
!	------- -- ---  ----------------------------------------------
!	930907	TD 1.0	Initial coding
*/

#include <stdio.h>
#include <dir.h>
#include <errno.h>
#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include <io.h>
#include <string.h>

#include "stat.h"

void usageSWD(void)
{
  printf("Syntax: mkStat [option] <logg-fil>\n");
  printf("Skapar en statistikmodul fr†n en loggfil f†ngad fr†n setstat.\n");
  printf("Modulen laddas sedan till ducen p† adress 3C000\n");
  printf("Loggfilen ska inneh†lla f”ljande;\n");
  printf("\n");
  printf("Stat 'buffertnamn':\n");
  printf("  0: buff 17, intervall 3600, duc 0, var GT51\n");
  printf("  1: - empty -\n");
  printf("  2: - empty -\n");
  printf("	.\n");
  printf("	.\n");
  printf("  29: - empty -\n");
  printf("Stat 1 empty\n");
  printf("Stat 2 empty\n");
  printf("	.\n");
  printf("	.\n");
  printf("Stat 7 empty\n");
  printf("\n");
  printf("buff „r till vilken pc-buffert och intervall „r i sekunder.\n");
  printf("Buffertnamn „r namnet f”r detta formul„r, max 8 stycken, med\n");
  printf("max 30 rader (0-29) i varje formul„r.\n");
  exit(1);
}


/* pseudo code is:
  openLogFile
  create Stat chunk
  parse log file
  emit stat data
*/
static struct _metaStatS metaStat[BUFF_MAX];

unsigned short swapword(unsigned short);
unsigned long swaplong(unsigned long);

unsigned short swapword(w)
unsigned short w;
{
  return ((w & 0xff) << 8) | ((w >> 8) & 0xff);
}

unsigned long swaplong(l)
unsigned long l;
{
  unsigned short w1, w2;
  w1 = (l >> 16) & 0xffff;
  w2 = l & 0xffff;
  return (((unsigned long) swapword(w2)) << 16) | swapword(w1);
}

main(int argc, char *argv[])
{
  FILE *fpLog, *fpUt;
  int option_O = 0, option_P = 0;
  char *logFile, *varChunk, line[80], var[32], value[32];
  int start, currBuff, currRow, i;
  char currBuffName[60];

  short int hilo;


  fprintf(stderr, "mkStat, version 1.0\n");
  if (1 == 2) printf("Copyright 1993, IVT Electronic AB\n");  /* luring ! */
  printCopyright();

  while( argc >= 2  && argv[1][0] == '-' ) {
    while( *++(argv[1]) ) {
      switch( *argv[1] ) {
	case 'o':
	case 'O':
	  option_O = 1;
	  continue;
	case 'p':
	case 'P':
	  option_P = 1;
	  continue;
	case '?':
	  usageSWD();
	  exit(0);
	default:
	  printf( "ogiltigt val: %c", (char *) *argv[1]);
      }
    }
    argv++;
    argc--;
  }
  if (argc < 2) {
    usageSWD();
    exit(1);
  }
  logFile = argv[1];
  if ((fpLog = fopen(logFile, "r")) <= 0) {
    printf("Cannot open log file '%s'\n", logFile);
    exit(1);
  }
  hilo = 0x4afc;
  if (*((char *) &hilo) == 0x4a)
    hilo = 1;                             /* like MC68K */
  else
    hilo = 0;                             /* maybe intel */

#if 0
#define BUFF_MAX 8
#define ITEM_MAX 30

static struct _metaStatBuff {
  long int /* short changed 920330 ! */ intervall;   /* 2*30*8 = 480byte extra*/
  unsigned long sample;
  char buff, duc;
  char var[32];
};

static struct _metaStatS {
  char name[32];
  struct _metaStatBuff rows[ITEM_MAX];
} metaStat[BUFF_MAX];    /* BUFF_MAX */
#endif

  for (i = 0; i < BUFF_MAX; i++) {
    metaStat[i].name[0] = 0;
  }
  currBuff = -1;
  start = 0;
  while (1) {
    fgets(line, 80, fpLog);
/* try for
Show next sample ?: (n) Stat 'kalle':"
*/
     if (!strncmp(line, "Show next sample ?: (n) Stat ", 29) ||
		!strncmp(line, "Stat", 4)) {
/*
Stat 'kalle':
*/
      if (!strchr(line, '\''))
	continue;
      start = 1;
      currBuff ++;
      strcpy(currBuffName, strchr(line, '\'') + 1);
      if (strchr(currBuffName, '\''))
	*strchr(currBuffName, '\'') = 0;
      printf("Buffert '%s' will hold buffer %d\n", currBuffName, currBuff);
      strcpy(metaStat[currBuff].name, currBuffName);
    } else if (!strncmp(line, "SaveIdx", 7)) {
      if (start)
	break;
    } else if (start) {
/*
  0: buff 17, intervall 60, duc 0, var level
  1: - empty -
*/
      currRow = atoi(line);
      if (currRow < 0 || currRow > 30) {
	printf("'%s'", line);
	printf("Fel vid l„sning av loggfilen, currRow = %d\n", currRow);
	exit(1);
      }
      metaStat[currBuff].rows[currRow].intervall = 0;
      metaStat[currBuff].rows[currRow].sample = 0;
      metaStat[currBuff].rows[currRow].buff = 0;
      metaStat[currBuff].rows[currRow].duc = 0;
      metaStat[currBuff].rows[currRow].var[0] = 0;
      if (strstr(line, "- empty -")) {
	;
      } else {
	if (strstr(line, "buff"))
	  metaStat[currBuff].rows[currRow].buff =
		atoi(strstr(line, "buff") + strlen("buff"));
	if (strstr(line, "intervall"))
	  metaStat[currBuff].rows[currRow].intervall =
	    swaplong(atol(strstr(line, "intervall") + strlen("intervall")));
	if (strstr(line, "duc"))
	  metaStat[currBuff].rows[currRow].duc =
		atoi(strstr(line, "duc") + strlen("duc"));

	line[strlen(line) - 1] = 0; 	/* remove last char (lf) */
	if (strstr(line, "var"))
		strcpy(metaStat[currBuff].rows[currRow].var,
			strstr(line, "var") + strlen("var "));
	printf("buff %d, row %d, '%s'\n", currBuff, currRow,
		metaStat[currBuff].rows[currRow].var);
      }
    }
  }
  fclose(fpLog);

  if ((fpUt = fopen("metastat", "wb")) <= 0) {
    printf("Cannot create output file '%s'\n", "metastat");
    exit(1);
  }
  if (1 != fwrite(metaStat, sizeof(metaStat), 1, fpUt))
    printf("Fel vid skrivning till metastat\n");
  fclose(fpUt);
  printf("Ladda modulen 'metastat' p† adressen %lx\n",
	METASTAT_ADDRESS);
  printf("Dvs. 'sendmod metastat %lx -r'\n",
	METASTAT_ADDRESS);
}
