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
int totFiles = 0, foundFiles = 0;

long totSize = 0, foundSize = 0;

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
  printf("Syntax: since datum [filpath]\n");
  printf("\n");
  printf("val „r ett av f”ljande:\n");
  printf("-q         tyst mod\n");
  printf("-v         pratig mod\n");
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

#if 1
doPath(char *path, int *totFiles, int *foundFiles,
	    unsigned short int *from,
	    unsigned short int *to)
{
  struct ffblk ffblk;
  int done, no, fil;
  char currentWd[256];

  done = findfirst(path,&ffblk,0);	/* only files */
  fil = 0;
  while (!done) {
       if (*from <= ffblk.ff_fdate) {
	if (*to == 0 || ffblk.ff_fdate <= *to) {
	  if (fil == 0) {
	     getcwd(currentWd, 255);
	     printf("- directory '%s' -\n", currentWd);
	  }                              
	  doFile(&ffblk);
	  fil ++;
	  (*foundFiles) ++;
	  foundSize += ffblk.ff_fsize;
	}
       }
      (*totFiles) ++;
      totSize += ffblk.ff_fsize;
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
	 doPath(path, totFiles, foundFiles, from, to);
	 chdir(currentWd);
	 if (verbose) printf("- back to directory '%s' -\n", currentWd);
	}
      }
      (*totFiles) ++;
      totSize += ffblk.ff_fsize;
      done = findnext(&ffblk);
  }
}
#else
doPath(char *path, int *totFiles, int *foundFiles,
	    unsigned short int *from,
	    unsigned short int *to)
{
  struct ffblk ffblk;
  int done, no, fil;
  char currentWd[256];

  done = findfirst(path,&ffblk,0xff);
  no = 0;
  fil = 0;
  while (!done) {
      no ++;
      if (ffblk.ff_attrib & FA_DIREC) {
	if (no > 2) {
/*
	 doFile(&ffblk);
*/
	 getcwd(currentWd, 255);
	 chdir(ffblk.ff_name);
	 if (verbose) printf("- go down to directory '%s' -\n", ffblk.ff_name);
	 doPath(path, totFiles, foundFiles, from, to);
	 chdir(currentWd);
	 if (verbose) printf("- back to directory '%s' -\n", currentWd);

         fil = 0;

	}
      } else {
       if (*from <= ffblk.ff_fdate) {
	if (*to == 0 || ffblk.ff_fdate <= *to) {
	  if (fil == 0) {
	     getcwd(currentWd, 255);
	     printf("- directory '%s' -\n", currentWd);
	  }                              
	  doFile(&ffblk);
	  fil ++;
	  (*foundFiles) ++;
	  foundSize += ffblk.ff_fsize;
	}
       }
      }
      (*totFiles) ++;
      totSize += ffblk.ff_fsize;
      done = findnext(&ffblk);
  }
}
#endif

main(int argc, char *argv[])
{
    struct ffblk ffblk;
    int done, noOfFiles;
    unsigned short int from, to;

    printf("Since, version 1.0\n");
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
    if (argc == 2) {
      strcpy(path, "\*.*");
    } else if (argc == 3) {
      strcpy(path, argv[2]);
    } else {
	usage();
	exit(1);
    }
    strcpy(date, argv[1]);

    parseDate(date, &from, &to);
    noOfFiles = 0;

    doPath(path, &totFiles, &foundFiles, &from, &to);
    printf("\n");
    printf("%d filer av totalt %d\n", foundFiles, totFiles);
    printf("%ld byte av totalt %ld bytes\n", foundSize, totSize);
}



parseDate(char *s, unsigned short int *from, unsigned short int *to)
{
  int d, m, y;

  y = s[0] * 10 + s[1] - 528;
  m = s[2] * 10 + s[3] - 528;
  d = s[4] * 10 + s[5] - 528;
  y = y - 80;
  *from = d | (m << 5) | (y << 9);

  if (s[6] != '-')
    *to = (*from);				/* 920924 */
  else {
    if ('0' <= s[7] && s[7] <= '9')             
      *to = 0;                                  /* 920924- */
    else {
      y = s[7] * 10 + s[8] - 528;               /* 920924-921008 */
      m = s[9] * 10 + s[10] - 528;
      d = s[11] * 10 + s[12] - 528;
      y = y - 80;
      *to = d | (m << 5) | (y << 9);
    }
  }
}