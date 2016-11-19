/* notice.c  1992-04-07 TD,  version 1.0 */
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
! notice.c
! Copyright (C) 1992, IVT Electronic AB.
*/
#include <stdio.h>
/*
! The string 
'Copyright 1992, IVT Electronic AB'
! is encoded as;
*/
static unsigned char copyrightNotice[] = {
  33, 10, 101, 21, 108, 30, 119, 16, 120, 12, 44, 29, 
  36, 29, 47, 3, 35, 106, 60, 104, 72, 13, 97, 4, 
  103, 19, 97, 14, 96, 9, 106, 74, 11, 73
};
/*
! Use the following procedure to decode it;
*/
void printCopyright()
{
  int seed = 73;
  int len;
  unsigned char *str;
  str = copyrightNotice;
  len = *str++;
  for (;len; len--) {
    fprintf(stdout, "%c", *str ^ seed);
    seed = *str++;
  }
  fprintf(stdout, "\n");
}
