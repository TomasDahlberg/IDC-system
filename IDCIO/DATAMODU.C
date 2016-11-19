/* datamodule.c  1991-08-03 TD,  version 1.1 */
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
! datamodule.c
! Copyright (C) 1991, IVT Electronic AB.
*/


#include <module.h>
/*
!   links to a datamodule
*/

char *linkDataModule(modname, header)
char *modname, **header;
{
  int status;
  char *dataptr = 0;
  char *headerptr = 0;

  if (status = link_datmod(0, modname, &dataptr, &headerptr)) {
    return 0;
  }
  if (header)
    (*header) = headerptr;
  return dataptr;
}

  
char *createDataModule(modname, size, exist, header, dest)
char *modname;
int size, *exist;
char **header;
char **dest;
{
  int rev  = 32768;
  int perm = 768+48+3;
  int status;
  char *dataptr = 0, *headerptr = 0;

  *exist = 1;
  if (dataptr = linkDataModule(modname, header))
    return dataptr;
  *exist = 0;
  if (status = newdatamod(size, rev, perm, modname, &dataptr, &headerptr))
    return 0;
     
  if (dest && *dest) {
    long sizeofModule;
    
    sizeofModule = *((long *) (((char *) headerptr) + 4));
    memcpy(*dest, headerptr, sizeofModule);
    unlinkDataModule(headerptr);      /* unlink old */
    headerptr = *dest;   
    *dest = *dest + sizeofModule;
    dataptr = headerptr + *((long *) (((char *) headerptr) + 48));
  }
  if (header)
    (*header) = headerptr;
  return dataptr;
}

#asm

verifyModule:
         move.l   d0,a0
         moveq    #0,d0
         OS9      F$VModul
         bcs.s    verErr
         moveq    #0,d1
verErr   
         move.l   d1,d0
         rts                  

newdatamod:
          
         move.l   4(a7),d2
         movea.l  8(a7),a0
         OS9      F$DatMod
         bcs.s    error1
         moveq    #0,d1

error1
         movea.l  12(a7),a0
         move.l   a1,(a0)
         movea.l  16(a7),a0
         move.l   a2,(a0)
         move.l   d1,d0

         rts
         

unlinkDataModule:
         move.l   d0,a2
         OS9      F$UnLink
         rts

link_datmod:

         move.l   d1,a0             module name string pointer         
         OS9      F$Link
         bcs.s    error2
         moveq    #0,d1             no error occured
error2
         movea.l  4(a7),a0
         move.l   a1,(a0)           execution entry
         movea.l  8(a7),a0
         move.l   a2,(a0)           module pointer
         move.l   d1,d0             any error code
         rts
         
#endasm


