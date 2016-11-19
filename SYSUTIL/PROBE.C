/* probe.c  1993-10-26 TD,  version 1.00 */
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
 * Heimdalsgatan 4
 * 113 28 Stockholm
 * Sweden
 */

/*
! probe.c
! Copyright (C) 1993 IVT Electronic AB.
*/

/*
!   probe     - probe utility functions
!
!   history:
!   date       by  rev  what
!   ---------- --- ---  ------------------------------------
!   1993-10-26 td  1.00 initial coding
!
!
*/

int testSocket(x)
unsigned long *x;
{
  unsigned long a;
  int ok = 1;

  a = *x;
  *x = 0xaaff5500;
  if (*x != 0xaaff5500)
    ok = 0;
  *x = a;

  return ok ? 1 : 0;
}

int chexpmem(ic)
int ic[];
{
  ic[0] = testSocket(0x40000) ? '1' : ' ';
  ic[1] = testSocket(0x60000) ? '2' : ' ';
  ic[2] = testSocket(0x80000) ? '3' : ' ';
  ic[3] = testSocket(0xA0000) ? 'A' : ' ';
  ic[4] = testSocket(0xC0000) ? 'B' : ' ';
  ic[5] = testSocket(0xE0000) ? 'C' : ' ';
}

