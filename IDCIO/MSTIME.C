/* mstime.c  1991-08-03 TD,  version 1.1 */
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
! mstime.c
! Copyright (C) 1991, IVT Electronic AB.
*/


#include <time.h>
#include "idcio.h"

long deltatime(prev)
struct _dtime *prev;
{
  int format, time, date, tick;
  short day;
  long sec, tc, ticksPerSecond, ms;
    
  _sysdate(3, &time, &date, &day, &tick);
  if (!prev->date) 
    ms = 0;
  else if (tick == prev->tick && time == prev->time && date == prev->date)
    ms = 0;
  else {
    sec = (date - prev->date)*86400 + (time - prev->time);
    tc = (tick & 0xffff) - (prev->tick & 0xffff);
    ticksPerSecond = (tick >> 16) & 0xffff;
    if (tc < 0) {
      tc += ticksPerSecond;
      sec --;
    }
    ms = sec * 1000 + (1000 * tc)/ ticksPerSecond;
  }
  prev->date = date;
  prev->time = time;
  prev->tick = tick;
  return ms;
}
