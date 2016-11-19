/* pid.c  1993-05-19 TD,  version 1.41 */
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
! pid.c
! Copyright (C) 1991-1993 IVT Electronic AB.
*/

/*
!   history:
!   date       by  rev  ed#  what
!   ---------- --- ---  ---  ------------------------------------
!   1991-??-?? td  1.00   1  initial coding
!
!     92-01-14 td  1.20      encapsulated in the IDCIO traphandler
!
!     92-03-18 td  1.30      changed elapsed time unit from millisec to sec
!                            to work properly during system time change
!
!     92-09-16 td  1.40      changed no of pids from 16 to 32, (see idcio.h)
!     93-05-19 td  1.41      ok, pids 16-31 now ok (previous bug was & 0x0f)
*/

#include <stdio.h>
#include <time.h>
#include "idcio.h"
#define NO_OF_ALARMS 1
#include "alarm.h"

static double _localPid();

double pid(regNo, actual, set_val, P, I, D, min, max, uPrev)
int regNo;
double actual, set_val, P, I, D, min, max, uPrev;
{
  static struct _pid context[32];     /* 24 * 32 = 768 bytes, 920916 */
  return _localPid(&context[regNo & 0x1f], actual, set_val, P, I, D, 
                  min, max, uPrev);
}

#define scale(in, a, b, c, d) (((in-a)/(b-a)) * (d - c) + c)

static double _localPid(context, actual, set_val, P, I, D, min, max, uPrev)
struct _pid *context;
double actual, set_val, P, I, D, min, max, uPrev;
{
    double output, e_n, integral, T, up;
    long dms;
    int first;
    static double pSetVal, pP, pI, pD;
    
    first = (context->previous.date == 0);

#define NEW_AS_OF_920318    
#ifdef NEW_AS_OF_920318    
    {
      long old;
      old = context->previous.time;
      context->previous.time = getRelTime(0);
      dms = context->previous.time - old;       /* NOW IN SEC !! */
      context->previous.date = 1;
    }
#else    
    dms = deltatime(&context->previous);
#endif
    
    e_n = set_val - actual;
    if (first) {
      context->e_n_1 = e_n;             /* ? */
      return uPrev;
    }
    if (dms == 0)
      return uPrev;

/*
    {
      static struct _system *sysVars = SYSTEM_AREA;
      if (context->e_n_1_dms == sysVars->newFlex)
        return uPrev;
      context->e_n_1_dms = sysVars->newFlex;
    }
*/

#ifdef NEW_AS_OF_920318    
    T = dms;
#else
    T = dms / 1000.0;
#endif
    if (I < 1.0)
      integral = 0;
    else
      integral = (T * e_n) / I;

    up = scale(uPrev, min, max, 0.0, 100.0);

    output = up +
                 P * (
                        (e_n - context->e_n_1) +
                        integral +
                        D * (e_n - 2 * context->e_n_1 + context->e_n_2) / T
                      );

    context->e_n_2 = context->e_n_1;
    context->e_n_1 = e_n;

    if (output > 100.0)
      output = 100.0;
    if (output < 0.0)
      output = 0.0;         /* -10.0;   changed 911008 */
/*
    context->oldout = output;     removed 920916
*/
    return(scale(output, 0.0, 100.0, min, max));
}

