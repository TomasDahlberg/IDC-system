/* dirsize.c  1993-01-15 TD,  version 1.0 */
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
! dirsize.c
! Copyright (C) 1992, IVT Electronic AB.
*/
#include <stdio.h>
#include <dos.h>
#include <dir.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int quiet = 0, verbose = 0;

int totTotFiles = 0;
long totTotSize = 0;

long shifter = 0;

char path[50];

/*
void printCopyright(void);
*/

usage(void)
{
  printf("Syntax: dirsize\n");
  printf("\n");
}

char *sizeSuffix(int shifter)
{
  static char buf[10];

  buf[0] = 0;
  if (shifter == 0)
    strcpy(buf, "B");
  else if (shifter == 10)
    strcpy(buf, "kB");
  else if (shifter == 20)
    strcpy(buf, "MB");
  return buf;
}


doPath(char *path, int *totFiles, long *totSize, int level)
{
  struct ffblk ffblk;
  int done, no, fil;
  char currentWd[256];

  done = findfirst(path,&ffblk,0);	/* only files */
  fil = 0;

  while (!done) {
      (*totFiles) ++;
      (*totSize) += ffblk.ff_fsize;
      if (level == 0) {
	   totTotFiles ++ ;
	   totTotSize += ffblk.ff_fsize;
      }
      done = findnext(&ffblk);
  }

  done = findfirst("*.*", &ffblk, FA_DIREC);
  no = 0;
  while (!done) {
      no ++;
      if (ffblk.ff_attrib & FA_DIREC) {
	if (strcmp(ffblk.ff_name, ".") && strcmp(ffblk.ff_name, "..")) { /* skip '.' and '..' */
	 getcwd(currentWd, 255);
	 chdir(ffblk.ff_name);
	 if (verbose) printf("- go down to directory '%s' -\n", ffblk.ff_name);
	 if (level == 0) {
	   *totFiles = 0;
	   *totSize = 0;
	 }
	 doPath(path, totFiles, totSize, level + 1);
	 if (level == 0) {
	   printf("%-8.8s %3s %9ld%s (%5d files)\n",
			ffblk.ff_name, "dir",
			(long) ((*totSize) >> shifter), sizeSuffix(shifter),
			*totFiles);
	   totTotFiles += (*totFiles);
	   totTotSize += (*totSize);
	 }
	 chdir(currentWd);
	 if (verbose) printf("- back to directory '%s' -\n", currentWd);
	}
      }
      done = findnext(&ffblk);
  }
}

main(int argc, char *argv[])
{
    struct ffblk ffblk;
    int done, noOfFiles;
    unsigned short int from, to;

    unsigned short int totFiles;
    unsigned long totSize;

    printf("dirsize, version 1.0\n");
/*
    printCopyright();
*/
    shifter = 0;
    while( argc >= 2  && argv[1][0] == '-' ) {
	while( *++(argv[1]) ) {
	    switch( *argv[1] ) {
		case 'q':
		case 'Q':
		    quiet = 1;
		    continue;
		case 'v':
		case 'V':
		    verbose = 1;
		    continue;
		case 'k':
		case 'K':
		    shifter = 10;
		    continue;
		case 'm':
		case 'M':
		    shifter = 20;
		    continue;
		case '?':
		    usage();
		    exit(0);
		default:
		    printf( "illegal option: %c\n", (char *) *argv[1]);
		}
	    }
	argv++;
	argc--;
    }
    strcpy(path, "*.*");
    noOfFiles = 0;
    doPath(path, &totFiles, &totSize, 0);

    printf("             %9ld%s (%5d files)\n",
		(long) ((totTotSize) >> shifter), sizeSuffix(shifter),
		totTotFiles);
}
