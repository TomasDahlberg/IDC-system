/* counter.c  1991-08-03 TD,  version 1.1 */
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
! counter.c
! Copyright (C) 1991, IVT Electronic AB.
*/


/*
!       counter
*/
#include <modes.h>
#include <stdio.h>
#include <time.h>

#include "idcio.h"

struct _count
{
  struct _dtime tid;              /* previous time */
  long value;                     /* previous value */
};


#ifdef VERY_MUCH_EPROM
long COUNTER_1(prev, module, ch, dur, store)
long prev;
int module, ch, dur;
long *store;
{
  long sample, dms, dpulse, frekvens;
#define BITS 9  
/*
  sample = xread(mod, ch, BITS);
*/
  sample = get_di(module);
  
  if (!*store) {
    *store = (long) malloc(sizeof(struct _count));
    dms = deltatime(&  (((struct _count *) (*store))->tid));
    frekvens = prev;
  } else {
    dms = deltatime(&  (((struct _count *) (*store))->tid));
    if (dms == 0) {     /* scans too tight */
      return prev;
    }
    dpulse = sample - ((struct _count *) (*store))->value;
    if (dpulse < 0)
      dpulse += (1 << BITS);
    frekvens = ( 1000 * dpulse ) / dms;
  }
  ((struct _count *) (*store))->value = sample;
  return frekvens;
}
#else
/* long COUNTER_1() {} */
#endif

/* main()
{
  long frek;
  while (1) {
      {
        static long store;
        frek = COUNTER_1(frek, 1, 2, 10, &store);
      }
      printf("frek = %d\n", frek);
      sleep(3);
  }
}*/

