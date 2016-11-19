/* meta.c  1992-04-07 TD,  version 1.0 */
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
! meta.c
! Copyright (C) 1992, IVT Electronic AB.
*/
#include <stdio.h>
#include <dir.h>
#include <stdlib.h>

#define METAMOD "metaMod"

char *metaModHeader = 0;
char *metaMod;

struct _metaModHeader {
    unsigned short sync;
    unsigned short sysrev;
    unsigned long  size;
    unsigned long  owner;
    unsigned long  nameoffset;
    unsigned short access;
    unsigned short typelang;
    unsigned short attrrev;
    unsigned short edition;
    unsigned long  usage;
    unsigned long  symbol;
    unsigned short ident;
    char  spare[12];
    unsigned short parity;
    
    unsigned long  dataptr;
    char  name[12];
};

getFileSize(char *s)
{
   struct ffblk ffblk1;

   if (findfirst(s,&ffblk1,0)) {
     printf("Sorry, no such file as '%s'\n", s);
     return 0;
   }
   return ffblk1.ff_fsize;
}

int readMetaMod(char *file)
{
  int siz, err;
  FILE *fp;

  if (!(siz = getFileSize(file)))
    return 0;
/*
  if ((siz = getFileSize(METAMOD)) > 60000) {
    printf("To big file to handle, '%s'\n", METAMOD);
    exit(1);
  }
*/
  if (!(metaModHeader = (char *) malloc(siz))) {
    printf("not enough memory\n");
    exit(1);
  }
  if ((fp = fopen(file, "rb")) <= 0) {
    printf("Error opening '%s'\n", METAMOD);
    return 0;
  }
  err = fread(metaModHeader, siz, 1, fp);
  metaMod = metaModHeader + sizeof(struct _metaModHeader);
  fclose(fp);
  return 1;
}
	  
unsigned short swapword(unsigned short w)
{
  return ((w & 0xff) << 8) | ((w >> 8) & 0xff);
}

unsigned long swaplong(unsigned long l)
{
  unsigned short w1, w2;
  w1 = (l >> 16) & 0xffff;
  w2 = l & 0xffff;
  return (((unsigned long) swapword(w2)) << 16) | swapword(w1);
}

double swapdouble(d)
double d;
{
  union {
    double dd;
    struct { unsigned long l1, l2; } ll;
  } u1, u2;

  u1.dd = d;
  u2.ll.l1 = swaplong(u1.ll.l2);
  u2.ll.l2 = swaplong(u1.ll.l1);
  return u2.dd;
}



void swapMetaMod(char *metaMod)
{
  int id, max;
  struct _metaEntry {
     unsigned short nameOffset, size, offset, lockOffset, type;
  } *metaEntry;

  metaEntry = (struct _metaEntry *) metaMod;
  max = metaEntry->nameOffset = swapword(metaEntry->nameOffset);
  id = 1;
  while (id++ <= max) {
    metaEntry++;

    metaEntry->nameOffset = swapword(metaEntry->nameOffset);
    metaEntry->size       = swapword(metaEntry->size);
    metaEntry->offset     = swapword(metaEntry->offset);
    metaEntry->lockOffset = swapword(metaEntry->lockOffset);
    metaEntry->type       = swapword(metaEntry->type);
  }
}

