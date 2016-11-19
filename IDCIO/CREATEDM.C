/* createDM.c  1993-03-05 TD,  version 1.1 */
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
! createDM.c
! Copyright (C) 1992-1993, IVT Electronic AB.
*/

/*
!   history:
!   date       by  rev  ed#  what
!   ---------- --- ---  ---  ------------------------------------
!   1991-??-?? td  1.00   1  initial coding
!
!     93-03-05 td  1.10      added ->_msize 
!
*/

#include <stdio.h>
#include <module.h>

char *linkDataModule();
char *buildModuleAt();

char *createDM(name, size, address, totSize, found, header)
char *name;         /* name of module */
int size;           /* size of data */
char *address;      /* pointer to start of module */
int *totSize;     /* returned total size inclusive header and crc */
int *found;       /* returns 1 if already existed */
char **header;
{
  char *dm;
  if (dm = linkDataModule(name, header)) {
    *found = 1;
    *totSize = ((struct modhcom *) *header)->_msize;    /* changed from 0, 930305 */
    return dm;
  }
  *found = 0;
  *header = address;
  return buildModuleAt(address, size, name, totSize);
}

struct {
  struct modhcom _mh;     /* common header info */
  unsigned long dataptr;
  char name[12];
} dm;

char *buildModuleAt(address, size, name, totSize)
char *address;
int size;
char *name;
int *totSize;
{
  FILE *fp;
  static long *setMem = 0x410;
  int accum = -1, i;

  if (size & 1)
    size ++;  
  dm._mh._msync = MODSYNC;
  dm._mh._msysrev = 0x0001;
  *totSize = dm._mh._msize = size + sizeof(dm) + sizeof(long);
  dm._mh._mowner = 0x0000;
  dm._mh._mname = sizeof(dm) - 12;
  dm._mh._maccess =MP_OWNER_READ | MP_OWNER_WRITE | MP_OWNER_EXEC |
                    MP_GROUP_READ | MP_GROUP_WRITE | MP_GROUP_EXEC |
                    MP_WORLD_READ | MP_WORLD_WRITE | MP_WORLD_EXEC;
  dm._mh._mtylan = mktypelang(MT_DATA, 0);
  dm._mh._mattrev = mkattrevs(MA_REENT | MA_GHOST, 0);  /* ghost add. 920407 */

  dm._mh._medit = 0x0001;
  dm._mh._musage = 0x00000000;
  dm._mh._msymbol = 0x00000000;
  dm._mh._mident = 0x0000;
/*  dm._mh._mspare = 0;   */
  dm._mh._mparity = mkparity(&dm._mh);

  dm.dataptr = sizeof(dm);
  strcpy(dm.name, name);

  *setMem = (long) address;
  fp = fopen("/mem", "w");
  crc(&dm, sizeof(dm), &accum);
  fwrite(&dm, sizeof(dm), 1, fp);

#if 1
{ register char *p;
  register i;
  p = address + sizeof(dm);
  for (i = 0; i < size; i++)
    *p++ = 0;
  p = address + sizeof(dm);
  crc(p, size, &accum);
  fwrite(p, size, 1, fp);
}
#else
  size >>= 1;
  for (i = 0; i < size; i++) {
    short chunk = 0;
    crc(&chunk, sizeof(chunk), &accum);
    fwrite(&chunk, sizeof(chunk), 1, fp);
  }
#endif
  
  crc(NULL, 0, &accum);
  accum = ~accum;
  fwrite(&accum, sizeof(accum), 1, fp);
  fclose(fp);  
  return address + sizeof(dm);
}

mkparity(s)
unsigned short *s;
{
  int q = 0xffff, i;
  
  for (i = 0; i < 23; i++)
    q ^= *s++;
  return q;
}
