/* initDM.c  1992-09-24 TD,  version 1.2 */
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
! initDM.c
! Copyright (C) 1992, IVT Electronic AB.
*/

/*
!   history:
!   date       by  rev  ed#  what
!   ---------- --- ---  ---  ------------------------------------
!   1991-??-?? td  1.00   1  initial coding
!
!     92-04-27 td  1.10      
!
!     92-09-24 td  1.20      added link to overflow and zeroDivide idc variables
*/

#define NO_OF_ALARMS 1
#include "alarm.h"
#include "sysvars.h"
#define NAMEOFDATAMODULE "VARS"
#define NAMEOFDATAMODULE1 "VAR1"
#define NAMEOFDATAMODULE2 "VAR2"
#include "lock.h"
#include "meta.h"
#include <errno.h>

char *createDM();

#define SIGNAL_BASE 0x100
#define _CYCLE_256_TICKS        1280              /* 5 sec */
#define SCAN_SIGNAL             18 + SIGNAL_BASE
#define MAIN_SIGNAL             19 + SIGNAL_BASE

static char *createDataModule(modname, size, exist, header, dest)
char *modname;
int size, *exist;
char **header;
char **dest;
{
  int status, totsize;
  char *dataptr;
  
  dataptr = createDM(modname, size, *dest, &totsize, exist, header);
  (*dest) += totsize;
  return dataptr;
}

_initScan(dm, dm_size, aldm, noOfAlarms, alarm_size,
                aldm2, lock, lock_size, noOfTimers, _dest, _restart_after_powerfail)
char **dm, **aldm2, **_dest, **_restart_after_powerfail;
struct _lockModule1 **lock;
struct _alarmModule **aldm;
int dm_size, noOfAlarms, alarm_size, lock_size, noOfTimers;
{
  char *_headerPtr1, *_headerPtr2, *_headerPtr3, *_headerPtr4;
  char *_oldAlarmModule, *_oldLockModule;
  struct _lockModule2 *lock2;

  struct _system *sysVars;

  sysVars = (struct _system *) SYSTEM_AREA;
  sysVars->IF_INTERRUPT = 0;      /* Just until released (os9p2 etc) */
  sysVars->useSetB = 0;
  sysVars->scanCount = 0;
  sysVars->mainCount = 0;
  _initDecList();
  *dm = (char *)
     createDataModule(NAMEOFDATAMODULE, dm_size,
           _restart_after_powerfail, &_headerPtr1, _dest);
  createDataModule(NAMEOFDATAMODULE2, dm_size, 
           &_headerPtr3, &_headerPtr2, _dest);
  createDataModule(NAMEOFDATAMODULE1, dm_size,
           &_headerPtr3, &_headerPtr2, _dest);
  if (noOfAlarms > 0) {
    *aldm = (struct _alarmModule *) createDataModule("ALARM", alarm_size,
          &_oldAlarmModule, &_headerPtr3, _dest);
    if (!_oldAlarmModule) {
      (*aldm)->alarmListPtr = 0;
      (*aldm)->noOfAlarmEntries = NO_OF_ALARM_ENTRIES;
      (*aldm)->noOfAlarmPts = noOfAlarms;
    }
    *aldm2 = (char *)
            (((char *) (*aldm)) +
              ((*aldm)->noOfAlarmEntries * sizeof(struct _alarmEntry) +
               sizeof(short) + sizeof(long)));
    sysVars->ptr2AlarmModule = (char *) (*aldm2);
  } else {
    *aldm = 0;
    *aldm2 = 0;
  }
  
  if (noOfAlarms || noOfTimers) {
    *lock = (struct _lockModule1 *) 
      createDataModule("LOCK", lock_size, &_oldLockModule, &_headerPtr4, _dest);
    lock2 = (struct _lockModule2 *) (((char *) (*lock)) +
             (noOfAlarms * sizeof(int) + sizeof(int)));
    if (!_oldLockModule)
    {
      _initAlarm(*aldm2, *lock);
      _initTimer(lock2, noOfTimers);
    }
  } else {
    *lock = 0;
    lock2 = 0;
  }
  {
    char *meta, *h;
    if ((meta = (char *) linkDataModule("METAVAR", &h)) == 0)
      AbortLink("METAVAR");
    _startAlarm(*dm, meta);
    unlinkDataModule(h);
  }
  initIcp(SCAN_SIGNAL);
}


_initMain(dm, aldm, noOfAlarms, aldm2)
char **dm;
struct _alarmModule **aldm;
struct _alarmModule2 **aldm2;
int noOfAlarms;
{
  char *_headerPtr1, *_headerPtr2;
  int i = 0;

  initIcp(MAIN_SIGNAL);
  while (1) {  
    *dm = (char *) linkDataModule(NAMEOFDATAMODULE, &_headerPtr1);
    if (!(*dm)) {
      i++;
      if (i < 10) {
        sleep(5);
        continue;
      }
    }
    break;                              /* added 920427 */
  }                                     /* added 920427 */

  if (!(*dm)) {
    AbortLink(NAMEOFDATAMODULE);   /* exits with error 221 = Module not found */
  }
  if (noOfAlarms > 0) {
    i = 0;
    while (1) {                                /* added 920427 */
      *aldm = (struct _alarmModule *) linkDataModule("ALARM", &_headerPtr2);
      if (!(*aldm)) {
        i++;
        if (i < 10) {
          sleep(5);
          continue;
        }
      }
      break;
    }
    if (!(*aldm)) {
      AbortLink("ALARM");         /* exits with error 221 = Module not found */
    }
    *aldm2 = (struct _alarmModule2 *)
            (((char *) (*aldm)) +
              ((*aldm)->noOfAlarmEntries * sizeof(struct _alarmEntry) +
               sizeof(short) + sizeof(long)));
  } else {
    *aldm = 0;
    *aldm2 = 0;
  }
  
/* if vars 'zeroDivide' and 'overflow' exists link these and set up
    an intercept handler for those exceptions using F$STrap
*/
  {                         /* new 920924 */
    char *meta, *h;
    if ((meta = (char *) linkDataModule("METAVAR", &h)) == 0)
      AbortLink("METAVAR");
    _setUpOverflowHandler(*dm, meta);
    unlinkDataModule(h);
  }
  _initDecList();
  return 1;
}

static int *ptr2IntVar(dm, meta, name)
char *dm, *meta, *name;
{
  int id;
  if ((id = metaId(meta, name)) > 0) {
    if (metaType(meta, id) == TYPE_INT) {
      return (int *) metaValue(dm, meta, id);
    }
  }
  return (int *) 0;
}

/*
static long *our_overflow_pc, *our_zerodivide_pc;
*/

int _setUpOverflowHandler(dm, meta)
char *dm;
char *meta;
{
  
  struct _system *sysVars;
    
  sysVars = (struct _system *) SYSTEM_AREA;
  sysVars->overflowPtr = ptr2IntVar(dm, meta, "overflow");

/*
  our_overflow_pc = ptr2IntVar(dm, meta, "overflow");
  our_zerodivide_pc = ptr2IntVar(dm, meta, "zeroDivide");
  if (our_overflow_pc || our_zerodivide_pc) {
*/

/*  if (sysVars->overflowPtr)   */
    init();
}

#asm
SYSTEM_AREA   equ    $0003ffbc
overflowPtr   equ    $0000

ExcpTbl   
          dc.w    T_ZerDiv,Divide-*-4
          dc.w    T_TRAPV,Divide-*-4
          dc.w    -1

init     movem.l  a0-a1,-(a7)
         movea.l  #0,a0
         lea      ExcpTbl(pc),a1
         os9      F$STrap
         movem.l  (a7)+,a0-a1
         rts

Divide   andi.l   #$ffff,d7

         movea.l  #SYSTEM_AREA,a6
         move.l   overflowPtr(a6),a1
         
*         cmpi.w   #28,d7
*         bne.s    zerod
*         move.l   our_overflow_pc(a6),a1
*         beq.s    not_set
*         bra.s    ok
* zerod    move.l   our_zerodivide_pc(a6),a1


         beq.s    not_set
ok       move.l   a0,(a1)                       /* save pc at idc-var place */

*****************         move.w   line(a6),error_line(a6)

not_set  move.l   R$a7(a5),a7
         move.l   R$pc(a5),-(a7)
         move.w   R$sr(a5),-(a7)
         movem.l  (a5),a0-a6/d0-d7
         rtr
#endasm

AbortLink(s)
char *s;
{
  printf("cant link to '%s'\n", s);
  printf("Is 'scan' running ?\n");
  exit(E_MNF);    /* error 221, module not found */
}

void icp(s)
int s;
{
  struct _system *sysVars;
    
  sysVars = (struct _system *) SYSTEM_AREA;
  if (s == MAIN_SIGNAL) {
    if (sysVars->mainCount > 0)
      sysVars->mainCount = 20;
  } else if (s == SCAN_SIGNAL) {
    if (sysVars->scanCount > 0)
      sysVars->scanCount = 20;
  }
}

int initIcp(signalCode)
int signalCode;
{
  int _alarm_id;
  struct _system *sysVars;
    
  sysVars = (struct _system *) SYSTEM_AREA;
  
  intercept(icp);
  if (signalCode == MAIN_SIGNAL)
    sysVars->mainCount = 20;
  else if (signalCode == SCAN_SIGNAL)
    sysVars->scanCount = 20;

  if ((_alarm_id = alm_cycle(signalCode, _CYCLE_256_TICKS | 0x80000000)) == -1 )
    exit(_errmsg(errno, "can't set alarm - "));
}
