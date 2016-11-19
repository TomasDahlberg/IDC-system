/* alarm.c  1994-03-23 TD,  version 1.43 */
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
! alarm.c
! Copyright (C) 1991-1994 IVT Electronic AB.
*/

/*
!   history:
!   date       by  rev  ed#  what
!   ---------- --- ---  ---  ------------------------------------
!   1991-??-?? td  1.00   1  initial coding
!
!     92-03-06 td  1.30      encapsulated in the IDCIO traphandler
!
!     92-09-14 td  1.40   x  bugfix in globalAlarmMask_x functionality
!                            activated by #define _V920914_1_40
!                            packAlarm does set mask according to glbAlarmMask
!
!     93-02-17 td  1.41      added call to _decrementList() in _packAlarms()
!                            move sysVars->alarmLoadCnt to idc-var defined as
!                            int alarmLoadCnt = 5;
!
!     93-09-21 td  1.42      Bugfix, when c&d-alarms are negated, they will only
!                            be confirmed if they haven't yet been confirmed
!
!     94-03-23 td  1.43      Added 'int clearAlarm;' which clears all alarms.
*/

#define _V920914_1_40
/**/
#ifndef NO_OF_ALARMS
#include <time.h>
#define NO_OF_ALARMS 1
#include "alarm.h"
#include "sysvars.h"
#include "lock.h"
#include "meta.h"
#endif
/**/

static struct _system *sysVars = SYSTEM_AREA;

/*
!   Entries:
!
!      int _startAlarm(dm, meta)
!      char *dm;
!      char *meta;
!
!      void _initAlarm(aldm2, lock)
!      struct _alarmModule2 *aldm2;
!      struct _lockModule1 *lock;
!      
!      void _initTimer(lock, noOfTimers)
!      struct _lockModule2 *lock;
!      int noOfTimers;
!      
!      int _markAlarm(aldm2, a, b, alarmNo, class)
!      struct _alarmModule2 *aldm2;
!      int a, b, alarmNo, class;
!      
!      int _unmarkAlarm(aldm, aldm2, a, alarmNo)
!      struct _alarmModule *aldm;
!      struct _alarmModule2 *aldm2;
!      int a, alarmNo;
!      
!      void _packAlarms(aldm, aldm2)
!      struct _alarmModule *aldm;
!      struct _alarmModule2 *aldm2;
*/

static int *okvittade_A;
static int *okvittade_B;
static int *okvittade_C;
static int *okvittade_D;

static int *kvittade_A;
static int *kvittade_B;
static int *kvittade_C;
static int *kvittade_D;

static int *glbAlarmMask_A;
static int *glbAlarmMask_B;
static int *glbAlarmMask_C;
static int *glbAlarmMask_D;

static int *alarmLoadCnt;

static int *clearAlarm;

static int *ptr2IntVar(dm, meta, name)
char *dm, *meta, *name;
{
  int id;
  if ((id = metaId(meta, name)) > 0) {
    if (metaType(meta, id) == TYPE_INT)
      return (int *) metaValue(dm, meta, id);
  }
  return (int *) 0;
}

int _startAlarm(dm, meta)
char *dm;
char *meta;
{
  okvittade_A = ptr2IntVar(dm, meta, "noOfNak_A");
  okvittade_B = ptr2IntVar(dm, meta, "noOfNak_B");
  okvittade_C = ptr2IntVar(dm, meta, "noOfNak_C");
  okvittade_D = ptr2IntVar(dm, meta, "noOfNak_D");

  kvittade_A = ptr2IntVar(dm, meta, "noOfAck_A");
  kvittade_B = ptr2IntVar(dm, meta, "noOfAck_B");
  kvittade_C = ptr2IntVar(dm, meta, "noOfAck_C");
  kvittade_D = ptr2IntVar(dm, meta, "noOfAck_D");

  glbAlarmMask_A = ptr2IntVar(dm, meta, "globalAlarmMask_A");
  glbAlarmMask_B = ptr2IntVar(dm, meta, "globalAlarmMask_B");
  glbAlarmMask_C = ptr2IntVar(dm, meta, "globalAlarmMask_C");
  glbAlarmMask_D = ptr2IntVar(dm, meta, "globalAlarmMask_D");
  
  alarmLoadCnt = ptr2IntVar(dm, meta, "sysAlarmBeep");	/* added 940704 */

  clearAlarm = ptr2IntVar(dm, meta, "clearAlarm");
}

void _initAlarm(aldm2, lock)
struct _alarmModule2 *aldm2;
struct _lockModule1 *lock;
{
  int i;

  for (i = 0; i < aldm2->noOfAlarmPts; i++)
  {
    aldm2->alarmPts[i].active = 0;
    aldm2->alarmPts[i].disable = 0;
    aldm2->alarmPts[i].initTime = 0;
    aldm2->alarmPts[i].serialNo = 0;
    aldm2->alarmPts[i].alarmNo = -1;        /* new 910719 */
    lock->alarmLock[i] = 0;
  }
  lock->noOfAlarms = aldm2->noOfAlarmPts;
}

void _initTimer(lock, noOfTimers)
struct _lockModule2 *lock;
int noOfTimers;
{
  int i;

  for (i = 0; i < noOfTimers; i++)
    lock->timerLock[i] = 0;
  lock->noOfTimers = noOfTimers;
}

/*
! marks alarm and returns TRUE (1) if called at raising edge
*/
int _markAlarm(aldm2, a, b, alarmNo, class)
struct _alarmModule2 *aldm2;
int a, b, alarmNo, class;
{
  long now;
  
  if (class == -1) {				/* new 940704 */
    return 0;
  }
  aldm2->alarmPts[a].alarmNo = alarmNo;           /* new 910719 */
  if (aldm2->alarmPts[a].active == 1)
    return 0;
  now = time(0);
  if (!aldm2->alarmPts[a].initTime || aldm2->alarmPts[a].initTime == -1) {
    aldm2->alarmPts[a].initTime = now;
  }
 
  if (now - aldm2->alarmPts[a].initTime >= b) {
    if (alarmLoadCnt) 			  /* convert to seconds */
      sysVars->alarmCnt = abs((*alarmLoadCnt) << 1);
      
    aldm2->alarmPts[a].active = 1;
    return 1;
  }
  return 0;
}

/* 
! unmarks alarm and returns TRUE (1) if called at falling edge
*/
int _unmarkAlarm(aldm, aldm2, a, alarmNo)
struct _alarmModule *aldm;
struct _alarmModule2 *aldm2;
int a, alarmNo;
{
  int i;

  aldm2->alarmPts[a].active = 0;
  if (!aldm2->alarmPts[a].initTime || aldm2->alarmPts[a].initTime == -1)
    return 0;
  aldm2->alarmPts[a].initTime = 0;

  if (alarmLoadCnt)   			/* convert to seconds */
	if (*alarmLoadCnt > 0)
	    sysVars->alarmCnt = - ((*alarmLoadCnt) << 1);
  
  for (i = 0; i < aldm->alarmListPtr; i++) {
    if (aldm->alarmList[i].serialNo == aldm2->alarmPts[a].serialNo)
      break;
  }
  if (i < aldm->alarmListPtr) {
#ifdef _V920914_1_40
    aldm->alarmList[i].sendStatus |= ALARM_SEND_NEGATE;
/* just one sendMask */
#else
    aldm->alarmList[i].sendNegate = 0xff;     /* 920129 1 -> 255 */
#endif
    aldm->alarmList[i].active = 0;
    aldm->alarmList[i].offTime = time(0);
    if (aldm->alarmList[i].class >= 2)   /* C or D, automatic confirm */
    {
      if (!aldm->alarmList[i].confirm) {    /* added 930921 */
        aldm->alarmList[i].confirm = 1;
        aldm->alarmList[i].confirmTime = aldm->alarmList[i].offTime;
#ifdef _V920914_1_40
        aldm->alarmList[i].sendStatus |= ALARM_SEND_CONFIRM;  /* just one sendMask */
#else
      aldm->alarmList[i].sendConfirm = 0xff;    /* 920129 */
#endif
/*      aldm2->alarmPts[a].confirm = 1;   Not implemented elsewhere yet !! */
      }
    }
  } else {
    /* ??????????? couldn't  find serial no ?? */
  }
  return 1;
}

static int tobeRemoved(entry, aldm2)
struct _alarmEntry *entry;
struct _alarmModule2 *aldm2;
{
    /* bugfix 920306, alarmNo -> alarmIndex */
  if (aldm2->alarmPts[entry->alarmIndex].disable)    /* new 910730 */
    return 0;
/*
!   if send == 0xFF     -> not set yet
!      send == 0x00     -> nobody wants it, remove !
!      
!   if send & sent == send -> ok !
*/
#ifdef _V920914_1_40
  if (!(entry->sendStatus & ALARM_SEND_INIT))
    return 0;                         /* globalAlarmMask not set yet */
/*
!   If not pc needed -> xxSent == 0xff and sendMask == 0 -> remove
!   If pc needed and
!             all ok -> xxSent == mask and sendMask == mask -> remove
!      all not ready -> xxSent != mask and sendMask == mask -> not remove
*/ 
  if (entry->confirm == 0)
    return 0;
  else if (entry->active == 1)
    return 0;
  if (entry->sendMask == 0)          
    return 1;
  if (
         ((entry->sendMask & entry->assertSent) == entry->sendMask) &&
         ((entry->sendMask & entry->negateSent) == entry->sendMask) &&
         ((entry->sendMask & entry->confirmSent) == entry->sendMask) &&
         (((entry->sendStatus & ALARM_SEND_DISABLE) == 0) || 
          ((entry->sendMask & entry->disableSent) == entry->sendMask)) &&
         (((entry->sendStatus & ALARM_SEND_ENABLE) == 0) || 
          ((entry->sendMask & entry->enableSent) == entry->sendMask))
          )
    return 1;
  else
    return 0;
/*
   toggling disable on and off for a while (once or twice) will not 
    propagate properly
*/
#else
/*
!   if send & sent == send -> ok !
*/
  return entry->assertSent &&
         ((entry->sendAssert & entry->assertSent) == entry->sendAssert) &&
         entry->negateSent &&
         ((entry->sendNegate & entry->negateSent) == entry->sendNegate) &&
         entry->confirmSent &&
         ((entry->sendConfirm & entry->confirmSent) == entry->sendConfirm) &&
         (entry->sendDisable == 0 ||
          ((entry->sendDisable & entry->disableSent) == entry->sendDisable));
#endif
/*  
  return entry->assertSent && entry->negateSent && entry->confirmSent &&
              (entry->sendDisable == 0 || entry->disableSent == 1);
*/
}

/*
!     pack alarm-datamodule
*/
void _packAlarms(aldm, aldm2)
struct _alarmModule *aldm;
struct _alarmModule2 *aldm2;
{
    int pek1, pek2, flag, flag2;
    int okvittat[4], kvittat[4];

    { 
      static short int *bvPtr = 0x003ffd4;
      *bvPtr = get_batt_voltage();
    }
    _decrementList();

    if (clearAlarm && *clearAlarm) {		/* added 940323 */
	int i;
  	aldm->alarmListPtr = 0;
	for (i = 0; i < aldm2->noOfAlarmPts; i++)
	{
	    aldm2->alarmPts[i].active = 0;
	    aldm2->alarmPts[i].disable = 0;
	    aldm2->alarmPts[i].initTime = 0;
	    aldm2->alarmPts[i].serialNo = 0;
	}
	*clearAlarm = 0;
    }

    pek2 = 0;
    if (glbAlarmMask_A || glbAlarmMask_B || glbAlarmMask_C || glbAlarmMask_C) {
      while (pek2 < aldm->alarmListPtr) {
        if (!(aldm->alarmList[pek2].sendStatus & ALARM_SEND_INIT)) {
          int mask;
          if (aldm->alarmList[pek2].class == 0)
            mask = *glbAlarmMask_A;
          else if (aldm->alarmList[pek2].class == 1)
            mask = *glbAlarmMask_B;
          else if (aldm->alarmList[pek2].class == 2)
            mask = *glbAlarmMask_C;
          else if (aldm->alarmList[pek2].class == 3)
            mask = *glbAlarmMask_D;
          aldm->alarmList[pek2].sendMask = mask;
          aldm->alarmList[pek2].sendStatus |= ALARM_SEND_INIT;
        }
        pek2 ++; 
      }
    }
    
    pek1 = pek2 = 0;
    while (pek2 < aldm->alarmListPtr)
    {
      if (tobeRemoved(&aldm->alarmList[pek1], aldm2))
      {
        if (!tobeRemoved(&aldm->alarmList[pek2], aldm2))
        {
          memcpy(&aldm->alarmList[pek1], 
                  &aldm->alarmList[pek2],
                      sizeof(struct _alarmEntry));
   /* this will remove it */                      
#ifdef _V920914_1_40
          aldm->alarmList[pek2].sendMask = 0;
#else
          aldm->alarmList[pek2].assertSent = 0xff;  /* 920129 */
          aldm->alarmList[pek2].negateSent = 0xff;
          aldm->alarmList[pek2].confirmSent = 0xff;
          aldm->alarmList[pek2].disableSent = 0xff;
#endif
          aldm->alarmList[pek2].confirm = 1;        /* this will not show it */
          aldm->alarmList[pek2].active = 0;
          pek1 ++;
        }
      } else
        pek1 ++;
      pek2 ++; 
    }

  aldm->alarmListPtr = pek1;
  pek2 = 0;
  flag = flag2 = 0;
  okvittat[0] = okvittat[1] = okvittat[2] = okvittat[3] = 0;
  kvittat[0] = kvittat[1] = kvittat[2] = kvittat[3] = 0;
  while (pek2 < aldm->alarmListPtr) {
    flag += (!aldm->alarmList[pek2].confirm);
    flag2 += aldm->alarmList[pek2].active;

    if (!aldm->alarmList[pek2].confirm)
      okvittat[aldm->alarmList[pek2].class & 0x03] ++;
    else if (aldm->alarmList[pek2].active)
      kvittat[aldm->alarmList[pek2].class & 0x03] ++;

    pek2 ++;
  }

/*
!   flash, if any none confirmed alarms exists
!   light, if all confirmed but alarm state remains
!   none,  else
*/      

/*
!   flag    - true if any active alarms         -> flash led
!   flag2   - true if only confirmed alarms     -> noflash led
!   otherwise turn off led
*/    
#define CPU_BIT 0x02
  if (aldm->alarmListPtr < aldm->noOfAlarmEntries) {
    if (sysVars->flashBits & CPU_BIT)       /* if flash     */
      sysVars->flashBits &= ~CPU_BIT;       /* turn it off  */
    sysVars->currentBits |= CPU_BIT;      /* turn it on   */
  } else {
    if (!(sysVars->flashBits & CPU_BIT))    /* if no flash, */
      sysVars->flashBits |= CPU_BIT;        /* turn it on   */
  }
    
#define ALARM_BIT 0x01
  if (flag) {
    if (!(sysVars->flashBits & ALARM_BIT))    /* if no flash, */
      sysVars->flashBits |= ALARM_BIT;        /* turn it on   */
  } else if (flag2) {
    if (sysVars->flashBits & ALARM_BIT)       /* if flash     */
      sysVars->flashBits &= ~ALARM_BIT;       /* turn it off  */
/*    if (!(sysVars->currentBits & ALARM_BIT))*/ /* if no led    */
      sysVars->currentBits |= ALARM_BIT;      /* turn it on   */
  } else {
    sysVars->flashBits &= ~ALARM_BIT;         /* turn off flash */
    sysVars->currentBits &= ~ALARM_BIT;       /* and turn off led */
  }
  
/*
!
*/ 
  if (okvittade_A) 
    *okvittade_A = okvittat[0];
  if (okvittade_B) 
    *okvittade_B = okvittat[1];
  if (okvittade_C) 
    *okvittade_C = okvittat[2];
  if (okvittade_D) 
    *okvittade_D = okvittat[3];
  if (kvittade_A) 
    *kvittade_A = kvittat[0];
  if (kvittade_B) 
    *kvittade_B = kvittat[1];
  if (kvittade_C) 
    *kvittade_C = kvittat[2];
  if (kvittade_D) 
    *kvittade_D = kvittat[3];
    
}

