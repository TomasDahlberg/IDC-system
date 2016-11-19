/* init.c  1991-09-03 TD,  version 1.0 */
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
! init.c
! Copyright (C) 1991, IVT Electronic AB.
*/

#ifdef OSK
#include "../alarm.h"
#else
#include "alarm.h"
#endif

#include "net.h"

static struct _system *sysVars = SYSTEM_AREA;

init()
{
  if ((sysVars->netFree = _ev_link(NET_EVENT_FREE)) == -1)
  {
    
  }
  if ((sysVars->netTaskAccomplished = 
                    _ev_link(NET_EVENT_TASK_ACCOMPLISHED)) == -1)
  {
    
  }
  
}
