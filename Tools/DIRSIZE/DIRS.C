/* since.c  1992-10-08 TD,  version 1.0 */
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
! since.c
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

/*
int totFiles = 0, foundFiles = 0;
long totSize = 0, foundSize = 0;
*/

int duc0isduc1 = 0;
int SWD = 1;
char path[50];
char date[50];

/*
void printCopyright(void);
*/
char *strcpy2sp(char *s1, char *s2)
{
  char *p;
  p = s1;
  while ((*s1 = *s2) && (*s2 != ' '))
   { s1++; s2++; }
  *s1 = 0;
  return p;
}

void getPath(char *path, char *realPath)
{
  char *pk;

  strcpy(realPath, path);
  pk = realPath + strlen(realPath);
  pk --;
  while (*pk != '\\') {
    if (pk < realPath)
      break;
    *pk-- = '\0';
  }
}

processFile(char *file, char *path)
{
  struct fcb blk, blk2;

  if (parsfnm(file, &blk, 1) == NULL)
   printf("Error in parsfm call\n");
  else {
   char f1[30], f2[30];
   if (parsfnm(path, &blk2, 1) == NULL)
     printf("Error in parsfm call2\n");
   else {
     int i, q;
     char realPath[60];

     blk.fcb_name[8] = 0;

     getPath(path, realPath);

     i = strlen(strcpy2sp(blk.fcb_name, blk.fcb_name));
     q = strlen(strcat(strcat(strcpy(f1, realPath), blk.fcb_name), ".pct"));
     strcat(f1, "        ");
     f1[q+8-i] = '\0';
     strcat(strcat(strcpy(f2, realPath), blk.fcb_name), ".cmd");
/*
     doFile(f1, f2);
*/
   }
  }
}

usage(void)
{
  printf("Syntax: dirsize\n");
  printf("\n");
}

void doFile(struct ffblk *ffblk)
{
  struct fcb blk;
  int h, m, s;
  int y, mon, d;

  if (parsfnm(ffblk->ff_name, &blk, 1) == NULL)
    printf("Error in parsfm call\n");
/*
TEST1234 IDC        50 92-03-31    9.14
*/
  printf("%8.8s %3s %9ld %02d-%02d-%02d   %2d.%02d\n", 
			blk.fcb_name, blk.fcb_ext, ffblk->ff_fsize, 
			y = (ffblk->ff_fdate >> 9) + 80,
			mon = (ffblk->ff_fdate >> 5) & 15,
			d = ffblk->ff_fdate & 31,
			h = ffblk->ff_ftime >> 11,
			m = (ffblk->ff_ftime >> 5) & 63);
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
      done = findnext(&ffblk);
  }

  done = findfirst("*.*", &ffblk, FA_DIREC);
  no = 0;
  while (!done) {
      no ++;
      if (ffblk.ff_attrib & FA_DIREC) {
	if (no > 2) {			/* skip '.' and '..' */
	 getcwd(currentWd, 255);
	 chdir(ffblk.ff_name);
	 if (verbose) printf("- go down to directory '%s' -\n", ffblk.ff_name);
	 if (level == 0) {
	   *totFiles = 0;
	   *totSize = 0;
	 }
	 doPath(path, totFiles, totSize, level + 1);
	 if (level == 0) {
	   printf("'%12s' - ", ffblk.ff_name);
	   printf("Files = %5d, Size = %9ld\n", *totFiles, *totSize);
	 }
	 chdir(currentWd);
	 if (verbose) printf("- back to directory '%s' -\n", currentWd);
	}
      }
      (*totFiles) ++;
      totSize += ffblk.ff_fsize;
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
}
