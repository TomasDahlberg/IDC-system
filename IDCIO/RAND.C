/* rand.c  1991-08-03 TD,  version 1.1 */
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
! rand.c
! Copyright (C) 1991, IVT Electronic AB.
*/


#ifdef VERY_MUCH_PROM
static unsigned long int next = 1;

int srand()
{
  long rnd, time, date, tick;
  short day;
  _sysdate(3, &time, &date, &day, &tick);
  rnd = ((time ^ tick) ^ (time >> 8)) & 0xff;
  next = rnd;
  return rnd;
}

unsigned long int rand()
{
  next = next * 1103515245 + 12345;
  return (unsigned int) (next / 65536) % 32768;
}
#else
/*
int srand() {}
unsigned long int rand() {}
*/
#endif
