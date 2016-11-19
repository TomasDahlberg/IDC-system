/* slave.c  1993-11-17 TD,  version 1.61 */
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
! slave.c
! Copyright (C) 1991-1993 IVT Electronic AB.
*/

/*
!     slave.c is part of the IVTnet product
!
!     File: slave.c
!     
!     Contains main function for slave process which operates at each slave
!
!     History     
!     Date        Revision Who  What
!     
!      3-sep-1991     1.0  TD   Start of coding
!
!     27-nov-1991     1.1  TD   Check of activateTime for next item in queue
!                               for GET_VAR requests
!                     1.11 TD   GET_VAR always replies with full name
!
!     30-jan-1992     1.2  TD   a first approach to more than one pc
!                               added registration of variables
!
!     18-feb-1992     1.21 TD   Each PC has its own copy of 'alarmMarkPtr'
!
!      6-mar-1992     1.30 TD   changed linkmodule("VAR2", headerptr) ->
!                                       linkmodule("VAR2", &headerptr) !!!
!                               This bug was held resp. for var1 disapeared
!                               Appeared when either
!                               1) Run PC on port /n196 and when changed to
!                                  /n296 first time, the header address for
!                                  'VAR2' was written at the header address for
!                                  'VAR1'
!                               2) When the slave process was created the
!                                  variable 'int useVars' which was not static,
!                                  was holding a random value. If this was 0,
!                                  then 'VAR1' was choosed and when /n296 at
!                                  the master site was used, the 'VAR1' module
!                                  would be destroyed !
!
!     30-mar-1992     1.40 TD   Added function Float() which converts 
!                               double to float. If out of range value, 
!                               the nearest float value is returned
!
!     31-mar-1992     1.41 TD   Bugfix in function get_idx_var for vector types
!
!      6-apr-1992     1.42 TD   Bugfix in function getNextUpdatedVar for
!                               vector types (did always update !).
!
!     10-apr-1992     1.43 TD   Bugfix, missed '& PCno' which appeared as
!                               alarms sent with wrong status etc
!
!     21-apr-1992     1.44 TD   get_idx_var fix.
!                               When Vision runs in method 1 mode and enters
!                               an BLK-picure, vision restarts its initializ'n
!                               of idx-vars. Later, when the BLK picture enters
!                               method II, vars from the previous picture could
!                               be sent. This appears as garbage at the lower
!                               left corner of the BLK-picture.
!                               Temporary solution: When Idx var 1 is set in
!                               routine get_idx_var and method 2 is NOT running
!                               the map[] array is cleared iff the var is 
!                               different from the previously set.
!
!      9-jul-1992     1.45 TD   Added netUp map to users idc variable 'nodeUp'
!                               
!     18-sep-1992     1.50 TD   Bugfix in globalAlarmMask system, activated by
!                               '#define _V920914_1_40'
!                         Edition 5,
!                         Correct order in routine doRequestAlarmText. Prev. 
!                         version put all trust in confirmSent etc wheter
!                         to set status bits or not. Now we compare all 
!                         timestamps such as confirmTime > disableTime. (920923)
!                         Added ALLOCATE/GET/PUT-MEM and LOAD_PROGRAM tests
!                         Added 'alarm-###' vars which retrieves the alarm 
!                         status.
!
!     12-nov-1992     1.51 TD   Added netcheck, resets net if any bytes in out
!                               buf and we haven't been polled by master for
!                               at least one minute.
!
!      4-dec-1992     1.52 TD   Added useSetB system var for logical display
!                               context
!
!      7-dec-1992     1.53 TD   Bugfix in 'nodeUp'-handling
!
!      2-feb-1993     1.54 TD   Netcheck only checks if nrOfPolls == prevPolls
!                               
!     17-may-1993     1.55 TD   Changed outbuf size limit from 300 -> 600
!                               Quick fix
!     24-may-1993     1.55 TD   If idx >= MAP_SIZE we will force clearMethod2()
!
!      8-jul-1993     1.56 TD   Request to include Array variables in var req
!     16-jul-1993     1.57 TD   and put_var Request also accepts Array vars!!
!     30-aug-1993     1.58 TD   Bugfix in function skipIndex, added terminating 
!                               zero which appeared as indices > 9 not working 
!                               properly.
!
! 930906 TD  1.59 13  11  Bugfix in alarm, we used to send confirm before 
!                         inactive if both shared the same timestamp.
!                         This occurs for C and D alarms. The fix is to switch
!                         this order. The result was that vision got stuck 
!                         with a 'green' alarm !
!
! 930923 TD  1.60 13  12  Switched to use the getAlarmSequence which resides in
!                         trap module 'idcutil'
!
! 931117 TD  1.61 13  13  and fixed the following bug! 
!			  during the switch of the getAlarmSequence, we
!			  forgot to change PCno (mask) -> pc (vector)
*/
#define _V920914_1_40

@_sysedit: equ 13
@_sysattr: equ $800d

#define NO_OF_ALARMS 1
#ifndef DOS
#include "../../alarm.h"
#include "../../sysvars.h"
#include "../../meta.h"
#else
#include "alarm.h"
#include "sysvars.h"
#include "meta.h"
#endif

/*
!   Needs the following vars from sys-area:
!
!     slavePid        - process id of slave process
!     varId           - buffer of variable identifier for 
!                       interprocess communication to slave
!     nodeId          - buffer of node identifier
!
*/

/* static */ struct _system *sysVars = SYSTEM_AREA;

static int NetFree, NetTaskAccomplished;

/*
!   The rest ...
*/

#include <stdio.h> 
#include <ctype.h> 
#include <time.h> 
#include <signal.h> 
#include <errno.h> 
#include <modes.h> 

#include "layer2.h"
#include "ivtnet.h"
#include "net.h"

unsigned short *nodeaddr= 0x402; /* node address used by this node */

union _mix receiver, transmitter;

typedef enum { _idle, _unknown, _packetSent, _packetReceived, _timeout } sts;

sts status;
unsigned char rPrevSeqNo, rSeqNo, sPrevSeqNo, sSeqNo;
union _mix bufPtr;
int expBackOff;
int resendCount;

int useVars;      /* 0 -> VAR1,  1 -> VAR2 */

struct _reply *reply;
int DEBUG = 0, DEBUG2 = 0;


int netFree;
int netTaskAccomplished;
/* int currentCommand;   */

/*
unsigned char *currentCommand = COMMAND_ADDRESS;
*/

char *dm = 0, *meta = 0, *dmBackup = 0;
struct _alarmModule *aldm = 0;
static char *headerPtr1 = 0, *headerPtr2 = 0, *headerPtr3 = 0, *headerPtr4 = 0;
struct _message *message;

static int setHostNode = 0, setHost_active = 0;

/* added 921112 */
static long checkNetStatus; /* rel-time when last checked for net polls */

/*
!    the node issued set_host, ie. holding keyboard
*/
static int setHostNode_tx = 0; 

static int setHost_recTime = 0, setHost_sendTime = 0;
static int setHost_active_tx = 0, setHost_sendTime_tx = 0;
static setHost_try_sendTime_tx = 0;


struct _screenContext {
  unsigned char display[2][40];
  unsigned char map[10];
  unsigned char keyCode, keyDown, keyWasDown, status;
  unsigned char x, y;
  int spawnedScreenPid;
} *screenContext;


char screenBuff[100];

int shellPid = 0;
int setRemoteNode = 0;
int setRemoteTime = 0;
int writePath, readPath;
int remoteDataSent = 0;

char *getMemPtr = 0;

static unsigned char *netError = 0x003ffd9;

int netUp = 0;         /* start with net down and wait for master call ! */
/* added 920709 */
int *netUpPtr = 0;     /* pointer to users idc variable 'netUp', if available */


time_t alarmNoRequest_sent = 0;

int internalCounter = 0;
/*
!  one for each PC
*/
static short int alarmMarkPtr[8];

int previousNoOfAlarms = 0;       /* 920205, -1 -> 0 */
      
#define MAX_NO_OF_REQUESTS 40   /* 20 */
struct _getVarRequest {
  short int nodeId, varId;
  time_t insertTime, activateTime;
} varRequest[MAX_NO_OF_REQUESTS];
int noOfVarRequests = 0;

#define MAP_SIZE                  100     /* 1000 */
int idleCount[MAP_SIZE];            /* counter for each idle var in method ii */
#define IDLE_MAX_COUNT 20           /* idle for max no of polls */
static int map[2][MAP_SIZE];
int method2running = 0;

int bpsList[] = {
  50, 75, 110, 134, 
  150, 300, 600, 1200, 
  1800, 2000, 2400, 3600, 
  4800, 7200, 9600, 19200, 
  38400
};

int bpsSelected = -1;              /* use default (9600) */

float Float(dd)     /* added 920330 */
double dd;
{
  if (dd > 1e+37)
    return 1e+37;
  if (dd < -1e+37)
    return -1e+37;
  if (0 > dd && dd > -1e-37)
    return 0;
  if (1e-37 > dd && dd > 0)
    return 0;
  return dd;
}

int lookUpRequest(id)
int id;
{
  int i;
  for (i = 0; i < noOfVarRequests; i++)
    if (varRequest[i].varId == id)
      return i + 1;
  return 0;
}

int timeStampRequest(id)
int id;
{
  int i;
  if (i = lookUpRequest(id)) {
    varRequest[i - 1].activateTime = getRelTime();
    return 1;
  }
  return 0;
}

rotateRequest()    /* circular shift of buffer, first is placed last in queue */
{
  int id, node;
  if (noOfVarRequests) {
    id = varRequest[0].varId;
    node = varRequest[0].nodeId;
    removeRequest(id, node);      /* remove first -> shift */
    insertRequest(id, node);      /* insert last */
  }
}

int insertRequest(id, node)
int id, node;
{
  if (noOfVarRequests < MAX_NO_OF_REQUESTS) {
    varRequest[noOfVarRequests].insertTime = getRelTime();
    varRequest[noOfVarRequests].activateTime = 0;
    varRequest[noOfVarRequests].varId = id;
    varRequest[noOfVarRequests++].nodeId = node;
    return 1;
  }
  return 0;
}

int removeRequest(id, node)
int id, node;
{
  int i;

  if (i = lookUpRequest(id, node)) {
    for (; i < noOfVarRequests; i++)
      memcpy((char *) &varRequest[i - 1], (char *) &varRequest[i],
                sizeof(struct _getVarRequest));
    noOfVarRequests --;
  }
  return 0;
}

int firstRequest(id, node)
int *id, *node;
{
  *id = varRequest[0].varId;
  *node = varRequest[0].nodeId;
}

/*
!   abort routine
!     removes all events and stops execution
*/
void Abort()
{
  int i = 0;
  do {
    _ev_unlink(netFree);
    if (i++ > 10)
      break;
  } while (_ev_delete(NET_EVENT_FREE) == -1);
  do {
    _ev_unlink(netTaskAccomplished);
    if (i++ > 20)
      break;
  } while (_ev_delete(NET_EVENT_TASK_ACCOMPLISHED) == -1);
  exit(0);
}

/*
!  intercept routine.
!     signals initiate commands, global variable 'currentCommand' will be set.
*/
void icp(code)
int code;
{
  if (code == 2 || code == 3)
    Abort();
/*  currentCommand = code;  */
  sysVars->reboots ++;         /* just temporary */
}

/*
!   encapsulates events functions, signals event ready
*/
int taskAccomplished()
{
  _ev_signal(netTaskAccomplished, 0);
}

/*
!   encapsulates events functions, signals event ready
*/
int createEvent(id, name, start)
int *id;
char *name;
int start;
{
  if ((*id = _ev_creat(start, -1, 1, name)) == -1)
  {
    if ((*id = _ev_link(name)) == -1)
    {
      fprintf(stderr, "cant link to event '%s', err=%d\n", name, errno);
      exit(1);
    }
    fprintf(stderr, "cant create event '%s', err=%d\n", name, errno);
    Abort();
  }
}

/*
!
*/
int bindModules(dm, meta, aldm, dmBackup)
char **dm, **meta, **aldm, **dmBackup;
{
  static char *dmBackup1, *dmBackup2;
  
  if (!*dm)
    *dm = (char *) linkDataModule("VARS", &headerPtr1);
  if (!*meta)
    *meta = (char *) linkDataModule("METAVAR", &headerPtr2);
  if (!*aldm)  
    *aldm = (char *) linkDataModule("ALARM", &headerPtr3);
  if (useVars == 0) {
    if (!dmBackup1)
      dmBackup1 = (char *) linkDataModule("VAR1", &headerPtr4);
    *dmBackup = dmBackup1;
  } else {
    if (!dmBackup2)
      dmBackup2 = (char *) linkDataModule("VAR2", &headerPtr4);
    *dmBackup = dmBackup2;
/*
    if (!*dmBackup) 
      *dmBackup = (char *) linkDataModule("VAR2", &headerPtr4);
*/
  }
  return (*dm && *meta && *aldm && *dmBackup); /* !!!! 920511, &dmBackup); */
}

#if 0
/*
!   for one alarm entry, checks what alarm to send
!   0 - no alarm pending for this entry
!   1 - assert state pending
!   2 - negate state pending
!   3 - confirm state pending
!   4 - disable state pending     (new 910517)
!   5 - enable state pending      (new 920921)
*/
getAlarmSequence(entry, PCno)
struct _alarmEntry *entry;
int PCno;
{
#ifdef _V920914_1_40
  if ((entry->sendStatus & ALARM_SEND_ASSERT) &&
      (entry->sendMask & PCno) && !(entry->assertSent & PCno))
    return 1;
  else if ((entry->sendStatus & ALARM_SEND_NEGATE) &&
           (entry->sendMask & PCno) && 
          !(entry->negateSent & PCno)) {                /* check inactive */
    if ((entry->sendStatus & ALARM_SEND_CONFIRM) &&
        (entry->sendMask & PCno) &&
       !(entry->confirmSent & PCno)) {                  /* and confirmed ? */
/*
!   930906, changed from > to >=. Reason: if C/D alarms; inactive first 
*/
      if (entry->confirmTime >= entry->offTime)         /* yes, who's first ?*/
        return 2;                                       /* inactive first */
      else
        return 3;
    } else                                              /* none confirmed */
      return 2;
  } else if ((entry->sendStatus & ALARM_SEND_CONFIRM) &&
             (entry->sendMask & PCno) &&
            !(entry->confirmSent & PCno))               /* still active */
    return 3;
  else if ((entry->sendStatus & ALARM_SEND_DISABLE) &&
           (entry->sendMask & PCno) && 
          !(entry->disableSent & PCno))                 /* new 910729 */
    return 4;
  else if ((entry->sendStatus & ALARM_SEND_ENABLE) &&
           (entry->sendMask & PCno) && 
          !(entry->enableSent & PCno))                  /* new 920921 */
    return 5;
  else
    return 0;
#else
  if ((entry->sendAssert & PCno) && !(entry->assertSent & PCno))
    return 1;
  else if ((entry->sendNegate & PCno) && 
          !(entry->negateSent & PCno)) {                /* check inactive */
    if ((entry->sendConfirm & PCno) &&
       !(entry->confirmSent & PCno)) {                  /* and confirmed ? */
      if (entry->confirmTime > entry->offTime)          /* yes, who's first ?*/
        return 2;                                       /* inactive first */
      else
        return 3;
    } else                                              /* none confirmed */
      return 2;
  } else if ((entry->sendConfirm & PCno) &&
            !(entry->confirmSent & PCno))               /* still active */
    return 3;
  else if ((entry->sendDisable & PCno) && 
          !(entry->disableSent & PCno))                 /* new 910729 */
    return 4;
  else
    return 0;
#endif
}
#endif

int ackAlarm(pc, abcdMask)
unsigned char pc;               /* no 0 - 7 */
unsigned char *abcdMask;
{
  int PCno;
  PCno = 1 << pc;
  setAlarmMask(abcdMask);
/* bugfix 931117, PCno -> pc */
  switch (getAlarmSequence(&aldm->alarmList[alarmMarkPtr[pc & 7]], pc /*PCno*/)) {
    case 1:   /* send assert */
              aldm->alarmList[alarmMarkPtr[pc & 7]].assertSent |= PCno;
              break;
    case 2:
              aldm->alarmList[alarmMarkPtr[pc & 7]].negateSent |= PCno;
              break;
    case 3:
              aldm->alarmList[alarmMarkPtr[pc & 7]].confirmSent |= PCno;
              break;
    case 4:
              aldm->alarmList[alarmMarkPtr[pc & 7]].disableSent |= PCno;
              if (aldm->alarmList[alarmMarkPtr[pc & 7]].confirm == 1)
                aldm->alarmList[alarmMarkPtr[pc & 7]].confirmSent |= PCno;
                            /* !! autoconfirm, new 910909 */
              break;
    case 5:     /* added 920921 */
              aldm->alarmList[alarmMarkPtr[pc & 7]].enableSent |= PCno;
              break;
    default:
              if (DEBUG) 
                  printf("error, ???, ackAlarm:\n");
              break;
  }
}
/*
!   Old approach:
!         the alarm mask is updated regulary
!
!
!   New approach:
!         the alarm mask is set once for all when the alarm is asserted
!
*/
int setAlarmMask(mask)
unsigned char *mask;
{
  int cnt, i;
  struct _alarmEntry *entry;
  
  cnt = aldm->alarmListPtr;
  entry = &aldm->alarmList[0];
  for (i = 0; i < cnt; i++) {
#ifdef _V920914_1_40
    if (!(entry->sendStatus & ALARM_SEND_INIT)) {
      entry->sendStatus |= ALARM_SEND_INIT;
      entry->sendMask = mask[entry->class & 0x03];
    }
#else
#if 1                           /* new 920212 */
    if (entry->sendAssert == 0xff)
      entry->sendAssert = mask[entry->class & 0x03];
    if (entry->sendNegate == 0xff)
      entry->sendNegate = mask[entry->class & 0x03];
    if (entry->sendConfirm == 0xff)
      entry->sendConfirm = mask[entry->class & 0x03];
    if (entry->sendDisable == 0xff)
      entry->sendDisable = mask[entry->class & 0x03];
#else
    if (entry->sendAssert)
      entry->sendAssert = mask[entry->class & 0x03];
    if (entry->sendNegate)
      entry->sendNegate = mask[entry->class & 0x03];
    if (entry->sendConfirm)
      entry->sendConfirm = mask[entry->class & 0x03];
    if (entry->sendDisable)
      entry->sendDisable = mask[entry->class & 0x03];
#endif
#endif
    entry++;
  }        
}
/*
!   anyAlarms - a generic routine for retrieving number of pending alarms
!   returns an 32bit integer unionfied with no of A,B,C and D alarms as 8bits
*/
int anyAlarms(pcSum, activeABCD, confirmedABCD)
int *pcSum;
unsigned char *activeABCD, *confirmedABCD;
{
  int cnt, count, i, c[4], as, ns, cs, ds, es = 0, class;
  union { char types[4]; int ret; } u;
  struct _alarmEntry *entry;
  
  cnt = aldm->alarmListPtr;
  count = (*pcSum) = 0;
  c[0] = c[1] = c[2] = c[3] = 0;
  entry = &aldm->alarmList[0];
  for (i = 0; i < cnt; i++) {
#ifdef _V920914_1_40
    if (!(entry->sendStatus & ALARM_SEND_INIT))
      as = 0xff;      /* if not init, let's assume everybody needs it */
    else {
      as = (entry->sendStatus & ALARM_SEND_ASSERT) ? 
        entry->sendMask ^ entry->assertSent & entry->sendMask : 0;
    }
    ns = (entry->sendStatus & ALARM_SEND_NEGATE) ? 
      entry->sendMask ^ entry->negateSent & entry->sendMask : 0;
    cs = (entry->sendStatus & ALARM_SEND_CONFIRM) ? 
      entry->sendMask ^ entry->confirmSent & entry->sendMask : 0;
    ds = (entry->sendStatus & ALARM_SEND_DISABLE) ? 
      entry->sendMask ^ entry->disableSent & entry->sendMask : 0;
    es = (entry->sendStatus & ALARM_SEND_ENABLE) ?  /* added 920921 */
      entry->sendMask ^ entry->enableSent & entry->sendMask : 0;
#else
    as = entry->sendAssert ^ entry->assertSent & entry->sendAssert;
    ns = entry->sendNegate ^ entry->negateSent & entry->sendNegate;
    cs = entry->sendConfirm ^ entry->confirmSent & entry->sendConfirm;
    ds = entry->sendDisable ^ entry->disableSent & entry->sendDisable;
#endif
    class = entry->class & 0x03;
    if (!entry->confirm) {                /* new 920211 */
      if (activeABCD)
        activeABCD[class] ++;
    } else if (entry->active) {
      if (confirmedABCD)
        confirmedABCD[class] ++;
    }
    *pcSum = (*pcSum | as | ns | cs | ds | es);      /* new 920210 */
    if (as || ns || cs || ds || es) {
      c[class] ++; 
      count++;
    }
    entry++;
  }
  u.types[0] = (c[0] > 255) ? 255 : c[0];
  u.types[1] = (c[1] > 255) ? 255 : c[1];
  u.types[2] = (c[2] > 255) ? 255 : c[2];
  u.types[3] = (c[3] > 255) ? 255 : c[3];
  return u.ret;
}
  
/* struct _alarmPacket {
  unsigned short int alarmNo, serieNo, status;
  long dtime;
  char text[80];
}; */
 
int doRequestAlarmText(alarm, serie, status, dtime, text)
short int *alarm, *serie, *status;
time_t *dtime;
char *text;
{
  int cnt, count, i, PCno, pcSum, pc;
  char abcdMask[4];


  PCno = 1 << (pc = (*alarm & 0x07));
  cnt = aldm->alarmListPtr;
  count = anyAlarms(&pcSum, 0, 0);
    

  *((short int *) &abcdMask[0]) = *serie;
  *((short int *) &abcdMask[2]) = *status;
  setAlarmMask(abcdMask);
  {
    struct _alarmEntry entry, *this;
      
    for (; internalCounter < cnt; internalCounter++) {
#ifdef _V920914_1_40
      this = &aldm->alarmList[internalCounter];
      if (!(this->sendStatus & ALARM_SEND_INIT))
        continue;
      if (!(this->sendMask & PCno))
        continue;
      if ((this->sendStatus & ALARM_SEND_ASSERT) &&
          !(this->assertSent & PCno))
        break;
      if ((this->sendStatus & ALARM_SEND_NEGATE) &&
          !(this->negateSent & PCno))
        break;
      if ((this->sendStatus & ALARM_SEND_CONFIRM) &&
          !(this->confirmSent & PCno))
        break;
      if ((this->sendStatus & ALARM_SEND_DISABLE) &&
          !(this->disableSent & PCno))
        break;
      if ((this->sendStatus & ALARM_SEND_ENABLE) &&   /* added 920921 */
          !(this->enableSent & PCno))
        break;
#else
      if (((aldm->alarmList[internalCounter].sendAssert & PCno) &&
          !(aldm->alarmList[internalCounter].assertSent & PCno)) ||
          ((aldm->alarmList[internalCounter].sendNegate & PCno) &&
          !(aldm->alarmList[internalCounter].negateSent & PCno)) ||
          ((aldm->alarmList[internalCounter].sendConfirm & PCno) &&
          !(aldm->alarmList[internalCounter].confirmSent & PCno)) ||
          ((aldm->alarmList[internalCounter].sendDisable & PCno) &&
          !(aldm->alarmList[internalCounter].disableSent & PCno)))
        break;
#endif
    }
    if (internalCounter >= cnt) {
      return 0;
    }
    memcpy(&entry, &aldm->alarmList[internalCounter], 
                                    sizeof(struct _alarmEntry));
    *alarm = entry.alarmNo;
    *serie = entry.serialNo & 0xffff;
    {
      struct _alarmModule2 *aldm2;
      aldm2 = (struct _alarmModule2 *)
            (((char *) aldm) +
              (aldm->noOfAlarmEntries * sizeof(struct _alarmEntry) +
               sizeof(short) + sizeof(long)));
      entry.disable = aldm2->alarmPts[entry.alarmIndex].disable;  /* add.920921 */
    }
/* bugfix 931117, PCno -> pc */
    switch (getAlarmSequence(&entry, pc /*PCno*/)) {
      case 1:      /* send assert event */
        *dtime = entry.initTime;
#ifdef _V920914_1_40
        *status = 1 | 0 /*cannot be disabled !*/ | ((entry.class & 0x0f) << 3);
#else
        *status = 1 | (entry.disable << 2) | ((entry.class & 0x0f) << 3);
#endif
        break;
      case 2:     /* send negate event */
        *dtime = entry.offTime;
        *status = ((entry.class & 0x0f) << 3);
#ifdef _V920914_1_40
        if (entry.confirmTime && (entry.offTime > entry.confirmTime))
          *status |= 2;
        if (entry.disableTime && (entry.offTime > entry.disableTime))
          *status |= 4;
#else
        if (entry.confirmSent & PCno)       /* added &PCno 920410 */
          *status = 2 | (entry.disable << 2) | ((entry.class & 0x0f) << 3);
        else
          *status = 0 | (entry.disable << 2) | ((entry.class & 0x0f) << 3);
#endif
        break;
      case 3:       /* send confirm event */
        *dtime = entry.confirmTime;
        *status = 2 | ((entry.class & 0x0f) << 3);
#ifdef _V920914_1_40
        if (entry.offTime && (entry.confirmTime >= entry.offTime))
          ;   /* inactive */
        else 
          *status |= 1;   /* active */
        if (entry.disableTime && (entry.confirmTime > entry.disableTime))
          *status |= 4;
#else
        if (entry.negateSent & PCno)      /* added &PCno 920410 */
          *status =  2 | (entry.disable << 2) | ((entry.class & 0x0f) << 3);
        else
          *status =  3 | (entry.disable << 2) | ((entry.class & 0x0f) << 3);
#endif
        break;
      case 4:       /* send disable event */
        *dtime = entry.disableTime;
        *status = 4 | ((entry.class & 0x0f) << 3);
#ifdef _V920914_1_40
        if (entry.offTime && (entry.disableTime >= entry.offTime))
          ;   /* inactive */
        else 
          *status |= 1;   /* active */
        if (entry.confirmTime && (entry.disableTime >= entry.confirmTime))
          *status |= 2;
#else
        *status =  ((entry.negateSent & PCno) ? 0 : 1)  /* added &PCno 920410 */
              | (entry.confirm << 1)
              | (entry.disable << 2) | ((entry.class & 0x0f) << 3);
#endif
        break;
      case 5:       /* send enable event */
        *dtime = entry.enableTime;
        *status = 0 | ((entry.class & 0x0f) << 3);
#ifdef _V920914_1_40
        if (entry.offTime && (entry.enableTime >= entry.offTime))
          ;   /* inactive */
        else 
          *status |= 1;   /* active */
        if (entry.confirmTime && (entry.enableTime >= entry.confirmTime))
          *status |= 2;
#else
        *status =  
                ((entry.negateSent & PCno) ? 0 : 1)  /* added &PCno 920410 */
              | (entry.confirm << 1)
              | (entry.disable << 2) | ((entry.class & 0x0f) << 3);
#endif
        break;
      default:
        if (DEBUG)        /* added if debug 920410 */
          printf("Program fault: alarmtext switch\n");
        break;
    }
    alarmMarkPtr[pc & 7] = internalCounter;
    strcpy(text, entry.string);
    internalCounter++;

/*
    if (DEBUG)  {
      int q;
      q = (int) *status;
      printf("Status = %d\n", q);
    }  
*/ 
    return 1;
  }
}

#define ALARM_ACTIVE   0x01
#define ALARM_CONFIRM  0x02
#define ALARM_DISABLE  0x04

int clearAlarm()
{
  int i;
  struct _alarmModule2 *aldm2;
  aldm2 = (struct _alarmModule2 *)
            (((char *) aldm) +
              (aldm->noOfAlarmEntries * sizeof(struct _alarmEntry) +
               sizeof(short) + sizeof(long)));
  aldm->alarmListPtr = 0;
  for (i = 0; i < aldm2->noOfAlarmPts; i++)
  {
    aldm2->alarmPts[i].active = 0;
    aldm2->alarmPts[i].disable = 0;
    aldm2->alarmPts[i].initTime = 0;
    aldm2->alarmPts[i].serialNo = 0;
  }
}

int doRequestConfirmAlarm(alarmNo, serialNo, status)
int alarmNo, serialNo, status;
{
  int i, cnt, count;
  struct _alarmModule2 *aldm2;
  
  cnt = aldm->alarmListPtr;
  count = 0;
  for (i = 0; i < cnt; i++) {
    if ((aldm->alarmList[i].alarmNo == alarmNo) &&
        ((aldm->alarmList[i].serialNo & 0xffff) == serialNo)) {
      break;
    }
  }
  if (i >= cnt) {
    if (DEBUG) printf("no such serial#\n");
    return 1;         /* no such serial number, but ACK !!! (910905) */
  }
  aldm2 = (struct _alarmModule2 *)
            (((char *) aldm) +
              (aldm->noOfAlarmEntries * sizeof(struct _alarmEntry) +
               sizeof(short) + sizeof(long)));


  if (status & ALARM_CONFIRM) {
    if (!aldm->alarmList[i].confirm) {
      aldm->alarmList[i].confirm = 1;
#ifdef _V920914_1_40
      aldm->alarmList[i].sendStatus |= ALARM_SEND_CONFIRM;
#else
      aldm->alarmList[i].sendConfirm = 0xff;    /* changed 920201 */
#endif
      aldm->alarmList[i].confirmTime = time(0);
      aldm->alarmList[i].confirmSent = 0;
    }
  }

#ifdef _V920914_1_40
  if (aldm2->alarmPts[aldm->alarmList[i].alarmIndex].disable !=
                                ((status & ALARM_DISABLE) != 0)) {

    if (aldm2->alarmPts[aldm->alarmList[i].alarmIndex].disable) {
      aldm->alarmList[i].sendStatus |= ALARM_SEND_ENABLE;
      aldm->alarmList[i].enableSent = 0;
      aldm->alarmList[i].enableTime = time(0);
      aldm->alarmList[i].disable =    /* use this ? */
        aldm2->alarmPts[aldm->alarmList[i].alarmIndex].disable = 0;
    } else {      
      aldm->alarmList[i].sendStatus |= ALARM_SEND_DISABLE;
      aldm->alarmList[i].disableSent = 0;
      aldm->alarmList[i].disableTime = time(0);
      aldm->alarmList[i].disable = 
        aldm2->alarmPts[aldm->alarmList[i].alarmIndex].disable = 1;
    }
/*
!   automatic confirm when disable of an alarm, 910517
*/
    if (aldm2->alarmPts[aldm->alarmList[i].alarmIndex].disable &&
          (aldm->alarmList[i].confirm == 0))
    {
      aldm->alarmList[i].sendStatus |= ALARM_SEND_CONFIRM;
      aldm->alarmList[i].confirm = 1; 
      aldm->alarmList[i].confirmTime = time(0); 
    }
  }
#else
  if (aldm2->alarmPts[aldm->alarmList[i].alarmIndex].disable !=
                                ((status & ALARM_DISABLE) != 0)) {
    aldm->alarmList[i].sendStatus |= ALARM_SEND_DISABLE;
    aldm->alarmList[i].disableSent = 0;
    aldm->alarmList[i].disable = 
        aldm2->alarmPts[aldm->alarmList[i].alarmIndex].disable = 
                                ((status & ALARM_DISABLE) != 0);
/*
!   automatic confirm when disable of an alarm, 910517
*/
    if (aldm2->alarmPts[aldm->alarmList[i].alarmIndex].disable &&
          (aldm->alarmList[i].confirm == 0))
    {
 /* ???  aldm->alarmList[i].sendDisable = 0xff;      /* changed 920201 */
      aldm->alarmList[i].confirm = 1; 
      aldm->alarmList[i].confirmTime = time(0); 
    }
  }
#endif
  else  /* when we hold the same status */
  {
    if ((status & ALARM_DISABLE) != 0)
      return 0;         /* trying to disable when already disabled -> NAK */
    else
      return 1;         /* trying to enable when already enabled -> ACK */
  }
  return 1;             /* otherwise reply with ACK */
}

#if 0     /* moved to file logdisp.c 920716 */
/*
!   takes the message blocks and unpack, show information on our display
*/
void showDisplayContext(message)
struct _message *message;
{    /* move bytes for led's into our sys structure */
  int xPos, yPos, i, j, next, ok;
  unsigned char prev, new, mess, newBits;
  
  prev = sysVars->flashBits;
  new = sysVars->flashBits = message->mix.keyDisplay.display.flashLed;
/*
!   prev new      -> currentBits    xor
!
!   0    0        -> copy           0
!   0    1        -> set            1
!   1    0        -> clear          1
!   1    1        ->  --            0
*/
  newBits = 0;
  mess = message->mix.keyDisplay.display.currentLed;
  for (j = 1; j < 256; j <<= 1)
  {
    if (((prev | new) & j) == 0)
      newBits |= mess & j;
    else if ((new ^ prev) & new & j)
      newBits |= j; 
    else if ((new ^ prev) & prev & j)
      newBits &= ~j; 
    else if (new & prev & j)
      ; 
  }
  sysVars->currentBits = newBits;
  
  next = yPos = xPos = ok = 0;
  for (i = 0; i < 10; i++) {
    for (j = 1; j < 256; j <<= 1) {
      if (message->mix.keyDisplay.display.map[i] & j) {
        if (!ok) {
          lcdpos(yPos, xPos);
          ok = 1;
        }
        lcdwr(message->mix.keyDisplay.display.buf[next++]);
      } else 
        ok = 0;
      xPos ++;
    } 
    if (xPos >= 40) {
      xPos = 0;
      yPos = 1; 
    }
  }
  lcdpos(message->mix.keyDisplay.display.y, 
                    message->mix.keyDisplay.display.x);
  if (message->mix.keyDisplay.display.status & 1)
    lcdcursorOn();
  else
    lcdcursorOff();
}


/*
!   takes the message blocks and unpack, includes key codes
!   then, pack our display context in the same message
*/
static char cap[2][40];

void captureDisplay(message)
struct _message *message;
{    /* move bytes for led's into our sys structure */
  int xPos, i, j, next, q, p;
  static char yPos = 0;
#define MAX_NEXT 40

  for (j = 0; j < 2; j++) 
    for (i = 0; i < 40; i++) {
      if (cap[j][i] != screenContext->display[j][i]) {
        cap[j][i] = screenContext->display[j][i];
        screenContext->map[(j*40+i)/8] |= (1 << ((j*40+i) % 8));
      }
    }
  
  screenContext->keyCode = message->mix.keyDisplay.key.keyCode;
  screenContext->keyDown = message->mix.keyDisplay.key.keyDown;
  message->mix.keyDisplay.display.flashLed = sysVars->flashBits;
  message->mix.keyDisplay.display.currentLed = sysVars->currentBits;
  
  message->mix.keyDisplay.display.status = screenContext->status;
  message->mix.keyDisplay.display.x = screenContext->x;
  message->mix.keyDisplay.display.y = screenContext->y;
  next = 0;
  for (q = 0; q < 2; q++, yPos ^= 1) {
    xPos = 0;
    for (i = yPos * 5, p = 0; p < 5; i++, p++) {
      message->mix.keyDisplay.display.map[i] = 0;
      for (j = 1; j < 256; j <<= 1, xPos++)
        if ((screenContext->map[i] & j) && (next < MAX_NEXT)) {
          message->mix.keyDisplay.display.buf[next++] = 
                        screenContext->display[yPos][xPos];
          screenContext->map[i] &= ~j;
          message->mix.keyDisplay.display.map[i] |= j;
        }
    }
  }
}
#endif

/*
!   starts a new screen process with our buffer as context
*/
int spawnScreen()
{
  int paramSize;
  char param[20];
  screenContext = (struct _screenContext *) screenBuff;

  sprintf(param, "-1 %d", screenContext);
  paramSize = strlen(param) + 1;
  if (DEBUG) printf("param='%s', paramSize=%d\n", param, paramSize);
  if ((screenContext->spawnedScreenPid = 
                    os9fork("screen", paramSize, param, 0, 0, 0, 0)) == -1) {
    return (errno) ? errno : 1; /* cannot fork to screen module */
  }
  return 0;       /* ok ! */
}

int createShellPipe(name, node)
char *name;
int node;
{
  char buf[50];
  sprintf(buf, name, node);
  return creat(buf, S_IREAD | S_IWRITE);
}

/*
!   starts a new shell process with two different pipes as stdin and stdout
*/
char *argblk[] = {
    "forkshell",
    0
};

extern char **environ;

static unsigned char *shellError = 0x003ffd0; /* 0 = shell is running 
                                                 1 = shell has terminated ok,
                                                     otherwise error code */
static unsigned char *shellData = 0x003ffd1;
                                                     
int spawnShell(node)
int node;
{
  char buf[80], inPipe[40], outPipe[40];
  int x1, x2;
  int saveStdout, saveStdin, saveStderr, cnt;

  if (shellPid)
    return 0;     /* busy !! */

  sprintf(outPipe, "/pipe/from_%d", node);
  sprintf(inPipe,  "/pipe/to_%d", node);

  saveStderr = dup(2);
  saveStdout = dup(1);
  saveStdin  = dup(0);
  close(2);
  close(1);
  close(0);
 
  x1 = creat(outPipe, S_IREAD | S_IWRITE);      /* stdin */
  x2 = creat(inPipe,  S_IREAD | S_IWRITE);      /* stdout */
  x2 = creat(inPipe,  S_IREAD | S_IWRITE);      /* stderr */

  setRemoteNode = node;

  *shellError = 0;      /* no error, shell is running */
  *shellData = 0;
  shellPid = os9exec(os9fork, argblk[0], argblk, environ, 0, 0, 3);
  cnt = 0;
  while (!*shellData) {
    if (cnt++ > 50)
      break;
    tsleep(1);
  }
  shellPid = *shellData;

  writePath = dup(0);     /* process stdin is our write path */
  readPath = dup(1);     /* process stdin is our write path */
  close(2);
  close(1);
  close(0);
  dup(saveStdin);
  dup(saveStdout);
  dup(saveStderr);

  if (shellPid < 1)
    return 0; /* cannot fork to shell */

  return 1;       /* ok ! */
}

int usage()
{
  fprintf(stderr, "Syntax: slave [<opt>] {<bps>}\n");
  fprintf(stderr, "Function. slave process interface to IVTnet\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "    -d  shows debug information\n");
  fprintf(stderr, "\nbps argument is one of:\n");
  fprintf(stderr, "9600\n");
  fprintf(stderr, "19200\n");
  fprintf(stderr, "38400\n");
}

static int currentId = 1;             /* ???????? 0 ????????? */

int getNextUpdatedVar(message, i)
struct _message *message;
int *i;
{
  int id, typ;
  char *vPtr;
  long ivalue;
  
  if (currentId >= MAP_SIZE) {
    currentId = 1;
    return 0;
  } 

  if (slave_getHardCoded(map[useVars & 1][currentId], &ivalue)) {/*add 920923 */
    message->mix.getUpdated[*i].buf[0] = currentId;
    memcpy(&message->mix.getUpdated[*i].buf[1], &ivalue, 4);
    currentId++;
    (*i)++;
    return 1;
  }

  if ((id = map[useVars & 1][currentId]) == 0) {   /* <= -> == 920323 */
    currentId++;
    return 1;
  }
  id &= 0xffff;     /* take only lower 16 bits */
  if (!(vPtr = (char *) metaValue(dm, meta, id))) {   /* ?? wrong !! */
    currentId++;
    return 1;
  }
  typ = metaType(meta, id);
  if ((memcmp(vPtr, metaValue(dmBackup, meta, id), (typ == TYPE_INT) ? 4 : 8)) ||
        (typ == TYPE_INT_VEC) || (typ == TYPE_FLOAT_VEC)) {
    long value;
    if (typ == TYPE_INT) {
      memcpy(metaValue(dmBackup, meta, id), vPtr, sizeof(long));
      memcpy(&value, vPtr, sizeof(long));
    } else if (typ == TYPE_FLOAT) {
      double dValue;
      float fValue;
      memcpy(metaValue(dmBackup, meta, id), vPtr, sizeof(double));
      memcpy(&dValue, vPtr, sizeof(double));
      fValue = Float(dValue);
      memcpy(&value, &fValue, sizeof(long));
    } else if (typ == TYPE_INT_VEC) {
      int *vec1, *vec2, indexVar;
      
      vec1 = (int *) vPtr;
      vec2 = (int *) metaValue(dmBackup, meta, id);
      indexVar = (map[useVars & 1][currentId] >> 16) & 0x7fff;
      if (map[useVars & 1][currentId] & (1 << 31)) { /* variable as index */
        if (metaType(meta, indexVar) == TYPE_INT)
          indexVar = *((int *) metaValue(dm, meta, indexVar));
        else if (metaType(meta, indexVar) == TYPE_FLOAT)
          indexVar = (int) (*((double *) metaValue(dm, meta, indexVar)));
      } /* else already constant */
      indexVar &= 0xffff;
      if (vec2[indexVar] == vec1[indexVar]) {     /* new 920406 */
        currentId++;
        return 1;
      }
      value = vec2[indexVar] = vec1[indexVar];
    } else if (typ == TYPE_FLOAT_VEC) {
      double *vec1, *vec2;
      int indexVar;
      double dValue;
      float fValue;
      
      vec1 = (double *) vPtr;
      vec2 = (double *) metaValue(dmBackup, meta, id);
      indexVar = (map[useVars & 1][currentId] >> 16) & 0x7fff;
      if (map[useVars & 1][currentId] & (1 << 31)) { /* variable as index */
        if (metaType(meta, indexVar) == TYPE_INT)
          indexVar = *((int *) metaValue(dm, meta, indexVar));
        else if (metaType(meta, indexVar) == TYPE_FLOAT) {
          dValue = (double) (*((double *) metaValue(dm, meta, indexVar)));
          indexVar = (int) (Float(dValue));
        }
      } /* else already constant */
      indexVar &= 0xffff;
      if (vec2[indexVar] == vec1[indexVar]) {     /* new 920406 */
        currentId++;
        return 1;
      }
      dValue = vec2[indexVar] = vec1[indexVar];
      fValue = Float(dValue);
      memcpy(&value, &fValue, sizeof(long));
      typ = TYPE_FLOAT;
    } else {
      ;
    }
    message->mix.getUpdated[*i].buf[0] = currentId | 
                                              ((typ == TYPE_FLOAT) ? 128 : 0);
    memcpy(&message->mix.getUpdated[*i].buf[1], &value, 4);
    currentId++;
    (*i)++;
    return 1;
  }   /* not changed */
  currentId++;
  return 1;                   /* see above (TYPE_INT_VEC & TYPE_FLOAT_VEC) */
}

int doRequestVarMethod2(message)
struct _message *message;
{
  int i;
  i = 0;
  while (i < 10) {
    if (!getNextUpdatedVar(message, &i))      /* returns zero when no more */
      break;
  }
  return i;
}

clearMethod2()
{             
  if (method2running) {
    int i;
    method2running = 0;
    for (i = 0; i < MAP_SIZE; i++)
      map[0][i] = map[1][i] = 0;
  }
}

/*
!   move index to 'indexVarBuf'
!   Before:   name         =   'xyz[va1]'
!
!   After:    name         = 'xyz'
!             indexVarBuf  = 'va1'
*/
void skipIndex(name, indexVarBuf)
char *name, *indexVarBuf;
{
  int i, size;

  size = strlen(name);
  *indexVarBuf = '\0';
  for (i = 0; i < size; i++)
    if (name[i] == '[')
      break;
  if (i < size)
  {
    name[i] = 0;
    for (i++; (i < size) && (name[i] != ']'); i++)
      *indexVarBuf++ = name[i];
    *indexVarBuf = '\0';                     /* bugfix added 930830 */
  }
}

/*
! input:  indexVarBuf = '17'
! returns 17
!
*/
int getIndex(dm, meta, indexVarBuf, idxId)
char *dm, *meta, *indexVarBuf;
int *idxId;
{
  int id;
  
  if (idxId)
    *idxId = 0;
  if (isdigit(indexVarBuf[0]))
    return atoi(indexVarBuf);
  if ((id = metaId(meta, indexVarBuf)) < 0)      /* name not found */
    return 0;
  if (idxId)
    *idxId = id;
  if (metaType(meta, id) == TYPE_INT)
    return *((int *) metaValue(dm, meta, id));
  else if (metaType(meta, id) == TYPE_FLOAT)
    return (int) (*((double *) metaValue(dm, meta, id)));
  else
    return 0;  
}

checkHardCoded(name)                      /* added 920923 */
char *name;
{
  if (!strncmp("alarm-", name, 6))
    return -100 - atoi(&name[6]);         /* alarm-17 ==>> -117 */
  return 0;
}

int slave_getHardCoded(id, buf)                    /* added 920923 */
int id;
long *buf;
{
  if (id <= -100 && id > -200) {
    *buf = alarmStatus(id + 100);
    return 1;
  }
  return 0;     /* not ours */
}

static int alarmStatus(pkt)
int pkt;
{
  struct _alarmModule2 *aldm2;
  struct _alarmPt *entry;
  int idx;
  
  if (!aldm)
    return 0;
  aldm2 = (struct _alarmModule2 *)
            (((char *) aldm) +
              (aldm->noOfAlarmEntries * sizeof(struct _alarmEntry) +
               sizeof(short) + sizeof(long)));
  for (idx = 0; idx < 100; idx++) {
    if (pkt == aldm2->alarmPts[idx].alarmNo)
      break;
  }
  if (idx >= 100)     /* not init or not exists */
    return 0;

  entry = &aldm2->alarmPts[idx];
/*
!   Return 0 no alarm, 1 active not conf, 2 active conf, 3 inactive not conf
*/  
  if (entry->active) {              /* a point confirmed ????  */
    if (entry->disable)
      return 2;                       /* YELLOW, active, confirmed */
    else
      return 1;                       /* RED,    active, not confirmed */
  } else {
    if (entry->disable)
      return 0;                       /* ----    not active, confirmed */
    else
      return 3;                       /* GREEN,  not active, not confirmed */
  }
}


/*
!   0.  checks for bindings for data modules
!   1.  copies data to backup module
!   2.  insert correct type in variable 'typ'
!   3.  copies bytes to non-aliged destination 'value' 
!   4.  returns any possible error, NOSUCHVAR, ILLEGALTYPE, NOBINDING
*/          
get_idx_var(inName, ptridx, typ, value, size)    /* returns error */
char *inName;
short int *ptridx;
unsigned char *typ;
char *value;
int *size;
{
  int idx, id, pM2 = 0;
  char indexVarBuf[32];

#if 0     /* changed on request from Kalle (Reuters) 930708, 1.56 */
  char *name;
  name = inName;
#else                             /* this makes indexed variables ok */
  char name[64];
  strncpy(name, inName, 60);
  name[60] = 0;
#endif

  if (bindModules(&dm, &meta, &aldm, &dmBackup)) /* ok ! */
  {
#if 0
    pM2 = method2running;
#else
/* added 930524 */

    if ((*ptridx) >= MAP_SIZE) {      /* if idx >= MAP_SIZE then */
      method2running = 1;             /* force a clearMethod2    */
      (*ptridx) %= MAP_SIZE;          /* and restore if negative is returned */
      {
        static char prev;
        if (prev)
          *((char *) 0x34001e) = 0x04;      /* *qsop2 = HBLED; */
        else
          *((char *) 0x34001f) = 0x04;      /* *qrop2 = HBLED; */
        prev ^= 1;
      }
    }

#endif
    

    clearMethod2();
    idx = (*ptridx) % MAP_SIZE;
    skipIndex(name, indexVarBuf);
    
    if (map[useVars & 1][idx] = checkHardCoded(name)) {    /* added 920923 */
      slave_getHardCoded(map[idx], value);
      *typ = TYPE_INT;
      (*size) += sizeof(long);
      return NET_NOERROR;
    }
 
    if ((id = metaId(meta, name)) < 0) {
      *ptridx = - (*ptridx);
      return NET_NOSUCHVAR;
    }

#if 0     /* used before 930524 */
    if (idx == 1 && pM2 == 0) {                             /* added 920421 */
      if (map[useVars & 1][idx] != id) {
        method2running = 1;   /* just fake */
        clearMethod2();
      }
    }
#endif

    map[useVars & 1][idx] = id;
    if (metaType(meta, id) == TYPE_INT) {
      *typ = TYPE_INT;
      (*size) += sizeof(long);
      memcpy(value, metaValue(dm, meta, id), sizeof(long));
      memcpy(metaValue(dmBackup, meta, id), 
                              metaValue(dm, meta, id), sizeof(long));
    } else if (metaType(meta, id) == TYPE_FLOAT) {
      *typ = TYPE_FLOAT;
      (*size) += sizeof(double);
      memcpy(value, metaValue(dm, meta, id), sizeof(double));
      memcpy(metaValue(dmBackup, meta, id),
                              metaValue(dm, meta, id), sizeof(double));
    } else if (metaType(meta, id) == TYPE_INT_VEC) {
      int *vec1, *vec2, idxVar, indexVar;
      
      vec1 = ((int *) metaValue(dm, meta, id));
      vec2 = ((int *) metaValue(dmBackup, meta, id));
      indexVar = getIndex(dm, meta, indexVarBuf, &idxVar);
      if (idxVar)
        map[useVars & 1][idx] |= (1 << 31) | (idxVar << 16);  /* var as index */
      else
        map[useVars & 1][idx] |= (indexVar << 16);    /* constant */
      *typ = TYPE_INT;
      (*size) += sizeof(long);
      vec2[indexVar] = vec1[indexVar];
      memcpy(value, &vec1[indexVar], sizeof(long));
    } else if (metaType(meta, id) == TYPE_FLOAT_VEC) {
      double *vec1, *vec2;
      int idxVar, indexVar;
      
      vec1 = ((double *) metaValue(dm, meta, id));
      vec2 = ((double *) metaValue(dmBackup, meta, id));
      indexVar = getIndex(dm, meta, indexVarBuf, &idxVar);
      if (idxVar)
        map[useVars & 1][idx] |= (1 << 31) | (idxVar << 16);  /* var as index */
      else
        map[useVars & 1][idx] |= (indexVar << 16);    /* constant */
      *typ = TYPE_FLOAT;
      (*size) += sizeof(double);
      vec2[indexVar] = vec1[indexVar];
      memcpy(value, &vec1[indexVar], sizeof(double));
    } else {
      *ptridx = - (*ptridx);
      return NET_ILLEGALTYPE;
    }
  } else {
    *ptridx = - (*ptridx);
    return NET_NOBINDING;
  }
  return NET_NOERROR;
}


/*
!   Main functions
!   accepts options 
!             slave noOfPackets DEBUG
*/

main(argc, argv)
int argc;
char *argv[];
{
  int rSize, toNode, size, cntBlock = 0, i, j, id;
  char rBuf[256], sBuf[256];
  struct _mem { short int PCidx; unsigned char typ; double value;
  } remember[10];
  int doNotRotate = 0;
  
  while( argc >= 2  && argv[1][0] == '-' ) {
    while( *++(argv[1]) ) {
      switch( *argv[1] ) {
        case 'e':
        case 'E':
          DEBUG2 = 1;
        case 'd':
        case 'D':
          DEBUG = 1;
          printf("DEBUGing in progress...\n");
          continue;
        case 'r':
        case 'R':
          doNotRotate = 1;
          continue;
        case '?':
          usage();
          exit(0);
	default:
          printf( "illegal option: %c", (char *) *argv[1]);
          exit(0);
      }
    }
    argv++;
    argc--;
  }
  if(argc > 1) {
    int baud, size, i;
    
    baud = atoi(argv[1]);
    size = sizeof(bpsList) / sizeof(bpsList[0]);
    for (i = 0; i < size; i++)
      if (bpsList[i] == baud)
        break;
    if (i < size)
      bpsSelected = i;
    else {
      fprintf(stderr, "cannot accept '%s'\n", argv[1]);
      usage();
      exit(0);
    }
  }
  
  {
    long *off = (long *) 0x003ffdc;
    
    *off = 0;  /*	3ffdc	      offset for relative time calculation  */
  }
  
  sysVars->tid.dummy = 0;
  sysVars->slavePid = getpid();
  intercept(icp);
  initphyio();
  initidcio(); 
  initidcutil();    /* added 930923 */
/*
! create events
*/
  createEvent(&netFree, NET_EVENT_FREE, 1);
  createEvent(&netTaskAccomplished, NET_EVENT_TASK_ACCOMPLISHED, 0);
/*
!   try to link to modules, VARS, METAVAR, ALARM
*/
  bindModules(&dm, &meta, &aldm, &dmBackup);


  if (netopen() == -1)
  {
    fprintf(stderr, "cannot open network !\n");
    exit(0);
  }

  if (bpsSelected >= 0) {
    int sts;
    if (sts = netSetBps(bpsSelected)) {
      fprintf(stderr, "cannot change baudrate, %s\n", bpsSelected);
      if (sts > 0)
        exit(sts);
      exit(0);
    }
  }

  if (bindModules(&dm, &meta, &aldm, &dmBackup))
  {
    int id = 1;
    struct _remote { long timeStamp; } *remotePtr;

    while (metaName(meta, id) > 0) {
      if (metaRemote(meta, id)) {
        remotePtr = (struct _remote *) metaRemoteData(dm, meta, id);
        remotePtr->timeStamp = 0;
      }
      id++;
    }
  }

  sleep(2);

  checkNetStatus = getRelTime(0);   /* don't start with 'restart-net' 930202 */
  while(1)
  {
    
#define VERSION_920911
#ifdef VERSION_920911
/* added 921112 */
    if (abs(getRelTime(0) - checkNetStatus) > 60) {   /* every minute */
      checkNetStatus = getRelTime(0);
      if (netCheck() == 0) {      /* check if we have been polled (only 930202) */
        netclose();
        netopen();
        if (bpsSelected >= 0) {
          netSetBps(bpsSelected);
        }
        netUp = 0;
        if (netUpPtr > (int *) 1) 
          *netUpPtr = 0;            /* tell user that net is down */
      }
    }
#endif

/* added 920709 */
    if (netUpPtr == (int *) 0) {         /* get users idc variable 'netUp' */
      if (bindModules(&dm, &meta, &aldm, &dmBackup)) {
        int id;
        if ((id = metaId(meta, "nodeUp")) >= 0) {
          netUpPtr = (int *) metaValue(dm, meta, id);
        } else
          netUpPtr = (int *) 1;
      }
    }
/*
!     We have previously issued a Set_Host Request, Now send our key codes...
*/
    if (setHost_active && (status == _idle) && 
          (abs(getRelTime(0) - setHost_sendTime) > 0))
    {
      static int prevCode = -1, checkPlusMinus = 0;

      setHost_sendTime = getRelTime(0);
      message = (struct _message *) sBuf;
      message->message_type = KEY_DISPLAY | REQUEST_MASK;
      message->mix.keyDisplay.key.keyCode = key();
      message->mix.keyDisplay.key.keyDown = keyDown();
      message->mix.keyDisplay.key.keyWasDown = keyWasDown();
      message->error = NET_NOERROR;
/*  check for escape ...    for now, escape character is '.'
*/

      if (setHost_recTime && (abs(getRelTime(0) - setHost_recTime) > 300))
      {                                       /* timeout */
        message->error = NET_CANCEL;
        sysVars->useSetB = 0;
        taskAccomplished();
        *netError = NET_TIMEOUT;
        setHost_active = 0;
      }
      if (keyDown() && key() == 10)
        checkPlusMinus = 1;
      else if (checkPlusMinus && keyDown() && key() == 11)
      {
        setHost_active = 0;
        message->error = NET_CANCEL;
        sysVars->useSetB = 0;
        *netError = NET_CANCEL;
        taskAccomplished();
      } else if (keyDown() && key() != 10)
        checkPlusMinus = 0;

      size = 5;
      if (message->error || message->mix.keyDisplay.key.keyDown) /* 920720 */
      {
        if (prevCode != message->mix.keyDisplay.key.keyCode) {
          prevCode = message->mix.keyDisplay.key.keyCode;
          sendPacket(setHostNode, sBuf, size);
        }
      } else {
        prevCode = -1;
      }
    }
/*
!     We have previously been requested a Set_Host issue,
!     Now send our display context...
*/
    if (setHost_active_tx && (status == _idle) && 
          (abs(getRelTime(0) - setHost_try_sendTime_tx) > 0))
    {
      setHost_try_sendTime_tx = getRelTime(0);
      message = (struct _message *) sBuf;
      message->message_type = KEY_DISPLAY | REPLY_MASK;
      message->error = NET_NOERROR;

      if (screenContext->keyWasDown == 17) {    /* try 920721 */
        tsleep(10);
        screenContext->keyDown = 0;
        screenContext->keyWasDown = 0;
      }

      size = captureDisplay(message);
      if ((size > 0) || (abs(getRelTime(0) - setHost_sendTime_tx) > 30)) {
        if (size < 0)
          size = -size;
        sendPacket(setHostNode_tx, sBuf, 2 + size);
        setHost_sendTime_tx = getRelTime(0);
      }
    }
    if (bindModules(&dm, &meta, &aldm, &dmBackup) && netUp) {
      int x, pcSum;
      unsigned long a = 0, b = 0;
      message = (struct _message *) sBuf;
      message->message_type = ALARM_NO | REQUEST_MASK;
      message->error = 0;
      size = 15;      /* 7 -> 15, new 920211 */
      if ((x = message->mix.alarmNoRequest.noOfAlarms = anyAlarms(&pcSum, &a, &b))
                    || 1) {
        int p;
        long t1;
        static int prevPcSum, pActive, pConfirmed;
        
        message->mix.alarmNoRequest.pcSum = pcSum;
        message->mix.alarmNoRequest.activeABCD = a;
        message->mix.alarmNoRequest.confirmedABCD = b;
        t1 = getRelTime();
        p = previousNoOfAlarms;     /* (!previousNoOfAlarms)) */
        if ((abs(t1 - alarmNoRequest_sent) > 100) ||  
            (pcSum != prevPcSum) ||
            ((x & 0xf000) && !(p & 0xf000)) ||
            ((x & 0x0f00) && !(p & 0x0f00)) ||
            ((x & 0x00f0) && !(p & 0x00f0)) ||
            ((x & 0x000f) && !(p & 0x000f)) ||
            (a != pActive) ||                        /* added 920312 */
            (b != pConfirmed)) {                     /* added 920312 */
          if (abs(t1 - alarmNoRequest_sent) > 10) {      /* added 920312 */
            previousNoOfAlarms = x;
            prevPcSum = pcSum;
            pActive = a;
            pConfirmed = b;
            sendPacket(0, sBuf, size);
            alarmNoRequest_sent = t1;
            internalCounter = 0;
          }
        }
      } else if (previousNoOfAlarms) {
/*
        sendPacket(0, sBuf, size);
*/
        previousNoOfAlarms = 0;
        internalCounter = 0;
      }
    }

    if (setRemoteTime && ((getRelTime() - setRemoteTime) > 10))
    {
      *netError = NET_TIMEOUT;
      setRemoteTime = 0;
      setRemoteNode = 0;
      taskAccomplished();
    }

    if (setRemoteNode && !setRemoteTime) {
      int nb;
      if ((nb = _gs_rdy(readPath)) > 0) {
        
       if ((getRelTime() - remoteDataSent) > 1) {
        if (nb > 40)
          nb = 40;
        message = (struct _message *) sBuf;
        read(readPath, message->mix.remoteData.buf, nb);

        message->message_type = REMOTE_DATA | REQUEST_MASK;

        if (message->mix.remoteData.buf[0] == 3)
          message->error = NET_CTRL_C;
        else
          message->error = 0;
          
        message->mix.remoteData.size = nb;
        size = 2 + 1 + nb;

        sendPacket(setRemoteNode, sBuf, size);
        remoteDataSent = getRelTime();
       }
      } else if (*shellError) {
        message = (struct _message *) sBuf;
        message->message_type = REMOTE_DATA | REQUEST_MASK;
        if (*shellError == 1)
          message->error = NET_LOGOUT;
        else
          message->error = *shellError;
        size = 2;
        sendPacket(setRemoteNode, sBuf, size);
        remoteDataSent = getRelTime();
        setRemoteNode = 0;
        shellPid = 0;
      }
    }

      if (noOfVarRequests > 0) {
        int id, node, i;
        struct _remote { long timeStamp; } *remotePtr;

/* #ifdef TESTing */
    if (!doNotRotate) {

        if (varRequest[0].activateTime) {     /* already tried ! */
          if (abs(getRelTime() - varRequest[0].activateTime) > 20) {
            rotateRequest();
          }
        }
    }
/* #endif */
        firstRequest(&id, &node);
        i = lookUpRequest(id);
        if (abs(getRelTime() - varRequest[i-1].insertTime) > 30) {
          varRequest[i-1].insertTime = getRelTime();
          message = (struct _message *) sBuf;
          message->message_type = GET_VAR | REQUEST_MASK;
          message->error = NET_NOERROR;
          remotePtr = (struct _remote *) metaRemoteData(dm, meta, id);
          timeStampRequest(id);
          remotePtr->timeStamp = getRelTime(); /* time(0); */
          strcpy(message->mix.varRequest.varName, 
                                  (char *) metaName(meta, id));
          size = 2 + strlen(message->mix.varRequest.varName) + 1;
          sendPacket(node, sBuf, size);
        }
      }

#define MAX_WAIT_TIME 60      /* if no reply, try in 60 sec again */
#define SEC_TO_GO     20      /* update each var no more than each 20 sec */

if (bindModules(&dm, &meta, &aldm, &dmBackup) && netUp)
if (netOutBufSize() < 300) {
      if (id = checkNextRemote()) {
        long now, node;
        struct _remote { long timeStamp; } *remotePtr;
              
        remotePtr = (struct _remote *) metaRemoteData(dm, meta, id);
        now = getRelTime();
/*        if (abs(now - remotePtr->timeStamp) > MAX_WAIT_TIME) {  */
        if (((now - remotePtr->timeStamp) > MAX_WAIT_TIME) ||
            ((now - remotePtr->timeStamp) < -600))
        {
          char *p;
          node = metaRemoteNode(meta, id);
          message = (struct _message *) sBuf;
          message->message_type = GET_VAR | REQUEST_MASK | REGISTRATE;
          message->error = NET_NOERROR;
          if (p = (char *) metaAlias(meta, id))
            strcpy(message->mix.varRequest.varName, p);
          else
            strcpy(message->mix.varRequest.varName, 
                                  (char *) metaName(meta, id));
          size = 2 + strlen(message->mix.varRequest.varName) + 1;
          remotePtr->timeStamp = getRelTime();
          sendPacket(node, sBuf, size);
        }
      }
}

/*
!	Now, check all registrated vars and send replies to those nodes
!	who have subscribed for these variables.
*/
if (bindModules(&dm, &meta, &aldm, &dmBackup) && netUp)
if (netOutBufSize() < 300) {
      int size;
      if (id = sendNextRegistrated(sBuf, &size)) {
        sendPacket(0, sBuf, size);
      }
}

    if ((status = layer2()) == _packetReceived)
    {
      int id;
      unfoldPacket(rBuf, &rSize);           /* unpack it */

      netUp = 1;
/* added 920709 */
      if (netUpPtr > (int *) 1)         /* tell users idc variable 'netUp' */
        *netUpPtr = 1;      /* that we are up and running */

      toNode = receiver.frame.head.sourceNode;
      message = (struct _message *) rBuf;
      if (message->message_type & REQUEST_MASK)
      {
        size = 0;
        message->message_type &= ~REQUEST_MASK;
        switch (message->message_type & ~REGISTRATE) {
          case GET_VAR:
            if (bindModules(&dm, &meta, &aldm, &dmBackup)) /* ok ! */
            {
              if ((id = metaId(meta, message->mix.varRequest.varName)) < 0) 
              {
                message->error = NET_NOSUCHVAR;
                size = 2 + strlen(message->mix.varRequest.varName) + 1;
              } else {
                int pos;


                if (message->message_type & REGISTRATE) {
                  if (registrate(id, toNode))
                    message->error = NET_REGISTRATE;   /* just a notice */
                  message->message_type &= ~REGISTRATE;
                }
                
                size = 2 + strlen(message->mix.varRequest.varName) + 1 + 1;
                pos = strlen(message->mix.varRequest.varName) + 1;
                if (metaType(meta, id) == TYPE_INT) {
                  message->mix.varRequest.varName[pos] = TYPE_INT;
                  memcpy(&message->mix.varRequest.varName[pos + 1], 
                              metaValue(dm, meta, id), sizeof(long));
                  size += sizeof(long);
/*
                  *((int *) &message->mix.varRequest.varValue) = 
                        *((int *) metaValue(dm, meta, id));
                  message->mix.varRequest.valueType = TYPE_INT;
*/
                }
                else if (metaType(meta, id) == TYPE_FLOAT) {
                  message->mix.varRequest.varName[pos] = TYPE_FLOAT;
                  memcpy(&message->mix.varRequest.varName[pos + 1], 
                              metaValue(dm, meta, id), sizeof(double));
                  size += sizeof(double);
/*
                  *((double *) &message->mix.varRequest.varValue) = 
                        *((double *) metaValue(dm, meta, id));
                  message->mix.varRequest.valueType = TYPE_FLOAT;
*/
                }
                else {
                  message->error = NET_ILLEGALTYPE;
                  size = 2 + strlen(message->mix.varRequest.varName) + 1;
                }
              }
            } else {
              message->error = NET_NOBINDING;
              size = 2 + strlen(message->mix.varRequest.varName) + 1;
            }
            break;
          case PUT_VAR:
            if (bindModules(&dm, &meta, &aldm, &dmBackup)) /* ok ! */
            {
              char indexVarBuf[32];
              int type, pos;

              pos = strlen(message->mix.varRequest.varName) + 1;
/*
! Added 930716, insert \0 at '[' position, and put index in 'indexVarBuf'
*/
              skipIndex(message->mix.varRequest.varName, indexVarBuf);

              if ((id = metaId(meta, message->mix.varRequest.varName)) < 0) 
              {
                message->error = NET_NOSUCHVAR;
              } else {
                long iValue;
                double dValue;

                type = message->mix.varRequest.varName[pos];
                message->error = NET_NOERROR;
                if (type == TYPE_INT) {
                  memcpy(&iValue,
                     &message->mix.varRequest.varName[pos + 1], sizeof(long));
		} else {
                  memcpy(&dValue,
                     &message->mix.varRequest.varName[pos + 1], sizeof(double));
                }

                if (metaType(meta, id) == TYPE_INT) {
                  *((int *) metaValue(dm, meta, id)) = 
                      (int) ((type == TYPE_INT) ? iValue : dValue);
                } else if (metaType(meta, id) == TYPE_FLOAT) {
                  *((double *) metaValue(dm, meta, id)) = 
                      (double) ((type == TYPE_INT) ? iValue : dValue);
                } else if (metaType(meta, id) == TYPE_INT_VEC) {
                  int *vec, indexVar;

                  vec = ((int *) metaValue(dm, meta, id));
                  indexVar = getIndex(dm, meta, indexVarBuf, (int *) 0);
                  vec[indexVar] = (int) ((type == TYPE_INT) ? iValue : dValue);
                } else if (metaType(meta, id) == TYPE_FLOAT_VEC) {
                  double *vec;
                  int indexVar;

                  vec = ((double *) metaValue(dm, meta, id));
                  indexVar = getIndex(dm, meta, indexVarBuf, (int *) 0);
                  vec[indexVar] = 
                      (double) ((type == TYPE_INT) ? iValue : dValue);
                } else {
                  if (DEBUG) printf("PUT_VAR req: illegal type\n");
                  message->error = NET_ILLEGALTYPE;
                }
	      }
            }
            size = 0;       /* for now, no reply !! */
            break;
          case ALARM_NO:
            if (bindModules(&dm, &meta, &aldm, &dmBackup)) /* ok ! */
            {
              int pcSum;
              message->error = 0;
              message->mix.alarmNoRequest.noOfAlarms = anyAlarms(&pcSum, 0, 0);
              message->mix.alarmNoRequest.pcSum = pcSum;
              size = 7;
            }
            break;
          case ALARM_TEXT_1:
            if (bindModules(&dm, &meta, &aldm, &dmBackup)) /* ok ! */
            {
              message->error = 0;
              if (!doRequestAlarmText(
                    &message->mix.alarmTextRequest.alarmNo,
                    &message->mix.alarmTextRequest.serialNo,
                    &message->mix.alarmTextRequest.status,
                    &message->mix.alarmTextRequest.dtime,
                    message->mix.alarmTextRequest.text))
              {
                internalCounter = 0;        /* no more, try again from start */
                if (!doRequestAlarmText(
                    &message->mix.alarmTextRequest.alarmNo,
                    &message->mix.alarmTextRequest.serialNo,
                    &message->mix.alarmTextRequest.status,
                    &message->mix.alarmTextRequest.dtime,
                    message->mix.alarmTextRequest.text))
                 message->error = NET_NOALARMS;
              }
              if (!message->error) {

                message->mix.alarmTextRequest.text[60] = 0;  
/*                message->mix.alarmTextRequest.text[40] = 0;  
*/
                
                size = 2 + 10 + strlen(message->mix.alarmTextRequest.text) + 1;
              } else
                size = 2;
            } else {
              message->error = NET_NOBINDING;
              size = 2;
            }
            break;
          case ALARM_TEXT_2:
            ackAlarm(message->mix.ackAlarm.PCno, 
                                  message->mix.ackAlarm.abcdMask);
            size = 0;
            break;
          case CONFIRM_ALARM:
            message->error = (doRequestConfirmAlarm(
                        message->mix.confirmAlarm.alarmNo,
                        message->mix.confirmAlarm.serialNo,
                        message->mix.confirmAlarm.status)) ? 
                                  0 : NET_NOSUCHALARM;
            size = 2;
            break;
          case SET_TIME:
            if (bindModules(&dm, &meta, &aldm, &dmBackup)) /* ok ! */
            {
              setAlarmMask(message->mix.setTime.abcdMask);
            }
            setTime(message->mix.setTime.tm.t_year, 
                    message->mix.setTime.tm.t_month,
                    message->mix.setTime.tm.t_day,
                    message->mix.setTime.tm.t_hour,
                    message->mix.setTime.tm.t_minute,
                    message->mix.setTime.tm.t_second);
            alarmNoRequest_sent = 0;        /* new 920205 */
            strncpy(message->mix.version, (char *) 0x3fffd8, 5);
            size = 7;           /* reply with version ! */
            message->message_type = VERSION | REQUEST_MASK;
            break;
          case SET_HOST:
                    /* they want us to;
                                        - spawn a new screen process
                                          with our buffer as context
                                        - reply with yes/no
                      */
            /* spawn process */
            message->mix.setHost.ok = spawnScreen();
	    setHost_active_tx = 1;
            setHostNode_tx = toNode;
            readLexicon();
            size = 3;
            break;
          case KEY_DISPLAY:   /* WE ARE RUNNING THE SCREEN PROCESS !! */
/*
! got a new key snapshot from 
! return our display and led snapshot
*/
            if (message->error == NET_NOERROR) {
              unpackKeyCodes(message);
#if 0
              sleep(2);                    /* a try,,,  920720 */
              screenContext->keyDown = 0;  /* release key ... (a try...) */
#endif
              size = 0;   /* no reply !! */

/*              size = 2 + captureDisplay(message);     */
/*              size = 57;    */
            } else if (message->error = NET_CANCEL) {
    /* obs ! if no other screen is currently running, we must run PULSE ! */
              kill(screenContext->spawnedScreenPid, SIGINT);
              setHost_active_tx = 0;
              size = 2;
            }
            break;
          case SET_REMOTE:
/*  spawn shell 
      for node 17:
    
    shell </pipe/from_17 >/pipe/to_17
*/
            if (shellPid)
              message->error = NET_BUSY;
            else if (!(shellPid = spawnShell(toNode)))
              message->error = (errno) ? errno : NET_SPAWN_ERROR;
            else
              message->error = 0;
            size = 2;
            break;
          case REMOTE_DATA:
            size = 0;
            if (message->error == NET_CTRL_C)
            {
              if (shellPid)
                kill(shellPid, SIGINT);
              message->error = 0;
            }
            if (message->error) {
              if (shellPid)
                kill(shellPid, SIGKILL);
              setRemoteNode = 0;
              shellPid = 0;
              *shellError = (message->error == NET_LOGOUT) ? 1 : message->error;
              close(writePath);
              close(readPath);
            } else if (!*shellError                  /* 1*/) {
              int nb;
              if ((nb = write(writePath, message->mix.remoteData.buf, 
                              message->mix.remoteData.size)) != 
                              message->mix.remoteData.size)
              {
                 if (DEBUG) printf("Request remote_data: nb=%d,size=%d\n",
                      nb,           message->mix.remoteData.size);
              }
            } else {
              message->error = NET_CANCEL;
              size = 2;
            }
            break;
          case GETNIDX_VAR_1:
          case GETNIDX_VAR_2:
            message->error = 0;
            size = 3;
            useVars = (message->message_type == GETNIDX_VAR_2) ? 1 : 0;
            
            for (i = j = 0; i < message->mix.getNIdxVar.n; i++) {
              memcpy(&remember[i].PCidx, &message->mix.getNIdxVar.buff[j], 2);
              j += 2;
              if ((message->error = get_idx_var(
                              &message->mix.getNIdxVar.buff[j],
                              &remember[i].PCidx,
                              &remember[i].typ,
                              &remember[i].value, &size)) == NET_NOERROR)
              {
                size += 3;
              } else {
                remember[i].typ = TYPE_INT;   /* just to pad out */
                size += 2 + 1 + 4;
              }
              j = j + strlen(&message->mix.getNIdxVar.buff[j]) + 1;
            }
/* pack ! */
            for (i = j = 0; i < message->mix.getNIdxVar.n; i++)
            {
              memcpy(&message->mix.getNIdxVar.buff[j], &remember[i].PCidx, 2);
              message->mix.getNIdxVar.buff[j + 2] = remember[i].typ;
              if (remember[i].typ == TYPE_INT) {
                memcpy(&message->mix.getNIdxVar.buff[j + 3],
                     &remember[i].value, sizeof(long));
                j += 7;
              } else {
                memcpy(&message->mix.getNIdxVar.buff[j + 3],
                     &remember[i].value, sizeof(double));
                j += 11;
              } 
            }
            break;
          case GETIDX_VAR_1:
          case GETIDX_VAR_2:
            message->error = 0;
            useVars = (message->message_type == GETIDX_VAR_2) ? 1 : 0;
            size = 4;
            if ((message->error = get_idx_var(message->mix.getIdxVar.name,
                              &message->mix.getIdxVar.PCidx, 
                              &message->mix.getIdxVar.name[0],
                              &message->mix.getIdxVar.name[1], &size)) == 
                              NET_NOERROR)
            {
              size += 1;
	    } else {
	      size = 2 + 2;
	    }
            break;
          case GET_UPDATED_1:
          case GET_UPDATED_2:
            useVars = (message->message_type == GET_UPDATED_2) ? 1 : 0;
            if (bindModules(&dm, &meta, &aldm, &dmBackup)) /* ok ! */
            {
              method2running = 1;
              message->error = 0;
/* no in params, out params: array of 10

              message->mix.getUpdated[i].PCidx_and_type
              message->mix.getUpdated[i].value   32bit float/long 
*/
              size = doRequestVarMethod2(message);
              if (size < 10) {
                message->mix.getUpdated[size].buf[0] = 0;
                size = 2 + 5 * size + 1;
              } else 
                size = 2 + 5 * size;
            } else {
              message->error = NET_NOBINDING;
              size = 2;
            }
            break;
          case GET_VARIDX:
            size = 2;
            if (bindModules(&dm, &meta, &aldm, &dmBackup)) {
              if ((id = metaId(meta, message->mix.getVarIdx.name)) < 0) 
                message->error = NET_NOSUCHVAR;
              else {
	        ((short int *) message->mix.getVarIdx.name)[0] = id;
	        size = 4;
                message->error = 0;
              }              
            } else
              message->error = NET_NOBINDING;
            break;
          case GET_CAL:
            size = 2;
            if (bindModules(&dm, &meta, &aldm, &dmBackup)) {
              if ((id = metaId(meta, message->mix.cal.name)) < 0) 
                message->error = NET_NOSUCHVAR;
              else {
	        size = packCalendar(message->mix.cal.bits.bitPack, id, 
	                          metaValue(dm, meta, id));
	        size = 2 + size;
                message->error = 0;
              }              
            } else
              message->error = NET_NOBINDING;
            break;
          case SET_CAL:
            size = 2;
            if (bindModules(&dm, &meta, &aldm, &dmBackup)) {
              id = unpackIdxCalendar(message->mix.cal.bits.bitPack);
              if (unpackCalendar(message->mix.cal.bits.bitPack, 
                                  metaValue(dm, meta, id)))
                message->error = NET_NOSUCHVAR;
            } else
              message->error = NET_NOBINDING;
            break;
          case CLEAR_ALARM:
            if (bindModules(&dm, &meta, &aldm, &dmBackup))
              clearAlarm();
            else
              message->error = NET_NOBINDING;
            size = 2;
            break;
          case GET_STAT_VAR:
            size = 4;
            if (bindModules(&dm, &meta, &aldm, &dmBackup)) {
              if ((id = metaId(meta, message->mix.stat.varName)) < 0)
                message->error = NET_NOSUCHVAR;
              else {
                if (metaType(meta, id) == TYPE_INT) {
                  message->mix.stat.varName[0] = TYPE_INT;
                  memcpy(&message->mix.stat.varName[1], 
                              metaValue(dm, meta, id), sizeof(long));
                  size += 1 + sizeof(long);
                } else if (metaType(meta, id) == TYPE_FLOAT) {
                  message->mix.stat.varName[0] = TYPE_FLOAT;
                  memcpy(&message->mix.stat.varName[1], 
                              metaValue(dm, meta, id), sizeof(double));
                  size += 1 + sizeof(double);
                } else
                  message->error = NET_ILLEGALTYPE;
              }
            } else
              message->error = NET_NOBINDING;
            break;
          case ALLOCATE_MEM:
            size = 0;
            break;
          case GET_MEM:
              /* some integrity check */
            memcpy(message->mix.mem.buff, 
                    message->mix.mem.address, 
                    message->mix.mem.size);
            size = 2 + message->mix.mem.size;
            break;
          case PUT_MEM:   /* address, size, block */
            size = 2; 
              /* some integrity check */
            memcpy(message->mix.mem.address, 
                    message->mix.mem.buff, 
                    message->mix.mem.size);
            break;
          case LOAD_PROGRAM:
            size = 2; 
              /* some integrity check */
/*            F$Verify(message->mix.mem.address, message->mix.mem.size);  */
            break;
          case REBOOT:
            os9fork("reboot", 0, 0, 0, 0, 0, 0);    /* crash ! */
            break;
          case NEW:
            {
              int status, pid;
        
              os9fork("new", 0, 0, 0, 0, 0, 0);
              while (!(pid = wait(&status)))    /* not forever ... (?) */
                ;
              if (status == 0)
                ;
              else
                message->error = status;
	    }
            size = 2; 
            break;
          default:
            size = 0;
            break;
        }
        if (size)
          sendPacket(toNode, rBuf, size);   /* obs ! rBuf sent !! */
      } else {  /* reply */

        switch (message->message_type) {
          case GET_VAR:

/* check message->error ! */

          
            if (bindModules(&dm, &meta, &aldm, &dmBackup)) /* ok ! */
            {
              if ((id = metaRemoteId(meta, 
                              message->mix.varRequest.varName, toNode)) < 0) 
/*              if ((id = metaId(meta, message->mix.varRequest.varName)) < 0) */
              {
                if (DEBUG) printf("GET_VAR reply: no such var\n");
                /*  NET_NOSUCHVAR */
              } else {
                int type, pos, delay = SEC_TO_GO;
                long iValue;
                double dValue;
               
                if (message->error == NET_REGISTRATE) /* just notice */  
                {
/*
!  its registrated, but just to be sure,
!  updated every 10th minute anyway
*/                  
                  delay = 600;      /* no need since we are a subscriber */
                  message->error = NET_NOERROR;
                }
                if (message->error == NET_NOERROR) {
                  pos = strlen(message->mix.varRequest.varName) + 1;
                  type = message->mix.varRequest.varName[pos];
                
                  if (type == TYPE_INT) {
                    memcpy(&iValue,
                     &message->mix.varRequest.varName[pos + 1], sizeof(long));
	  	  }
                  else {
                    memcpy(&dValue,
                     &message->mix.varRequest.varName[pos + 1], sizeof(double));
                  }
                  if (metaType(meta, id) == TYPE_INT) {
                    *((int *) metaValue(dm, meta, id)) = 
                      (int) ((type == TYPE_INT) ? iValue : dValue);
                  } else if (metaType(meta, id) == TYPE_FLOAT) {
                    *((double *) metaValue(dm, meta, id)) = 
                      (double) ((type == TYPE_INT) ? iValue : dValue);
                  } else {
                    if (DEBUG) printf("GET_VAR reply: illegal type\n");
                  /*  NET_ILLEGALTYPE */
                  }
                }
                {
                  struct _remote { long timeStamp; } *remotePtr;
              
                  remotePtr = (struct _remote *) metaRemoteData(dm, meta, id);
#define SLOW_NET
#ifdef SLOW_NET
                  remotePtr->timeStamp =
                                getRelTime() - (MAX_WAIT_TIME - delay);
/*
                  remotePtr->timeStamp = 
                                getRelTime() - (MAX_WAIT_TIME - SEC_TO_GO);
*/
#else
                  remotePtr->timeStamp = 0;    /* force a new update NOW ! */
#endif
                }
              }
            } else if (DEBUG)
              printf("get_var reply: cant bind data modules\n");
            if (DEBUG)
              printf("TaskAccomplished, get var done\n");

      removeRequest(id);

      if (noOfVarRequests > 0) {
        int id, node, i;
        struct _remote { long timeStamp; } *remotePtr;

        firstRequest(&id, &node);
        i = lookUpRequest(id);
        
        if ((varRequest[i-1].activateTime == 0) || 
              (abs(getRelTime() - varRequest[i-1].activateTime) > 30)) {
/*          varRequest[i-1].insertTime = getRelTime();    */
          message = (struct _message *) sBuf;
          message->message_type = GET_VAR | REQUEST_MASK;
          message->error = NET_NOERROR;
          remotePtr = (struct _remote *) metaRemoteData(dm, meta, id);
          timeStampRequest(id);
          remotePtr->timeStamp = getRelTime(); /* time(0); */
          strcpy(message->mix.varRequest.varName, 
                                  (char *) metaName(meta, id));
          size = 2 + strlen(message->mix.varRequest.varName) + 1;
          sendPacket(node, sBuf, size);
        }
      }
              
/*            taskAccomplished();   */
            break;
          case PUT_VAR:
            break;
          case ALARM_NO:    /* shouldn't occur, we never send this message */
          case ALARM_TEXT_1:
            break;
          case ALARM_TEXT_2:
            break;
          case CONFIRM_ALARM:
          case SET_TIME:        /* no reply on this ! */
            break;
          case SET_HOST:                /* do something */
            if (message->mix.setHost.ok == 0)
            {
              setHost_active = 1;
              setHost_recTime = 0;
              readLexicon();
              sysVars->currentBitsB = 0;
              sysVars->flashBitsB = 0;
              sysVars->flashBits2B = 0;
              sysVars->useSetB = 1;     /* ok, started another set */
            } else {
              taskAccomplished();
              *netError = message->mix.setHost.ok;
            }
            break;
          case KEY_DISPLAY:     /* got a new display context, show it ! */
            setHost_recTime = getRelTime(0);
            if (setHost_active && rSize > 2)
              showDisplayContext(message, rSize);
            break;
          case SET_REMOTE:      /* reply ! */
            if (message->error) {
              *netError = message->error;
              
/*              message->error = NET_BUSY;
                message->error = (errno) ? errno : NET_SPAWN_ERROR;
*/
              setRemoteTime = 0;
              setRemoteNode = 0;
            } else {  /* ok, create /pipe/from_7 /pipe/to_7 */
              readPath = createShellPipe("/pipe/to_%d", toNode);
              writePath = createShellPipe("/pipe/from_%d", toNode);
              *netError = 0;
              setRemoteTime = 0;
            }
            taskAccomplished();
            break;
          case REMOTE_DATA:     /* no reply on this */
            break;
          case GETIDX_VAR_1:      /* no reply on this */
          case GETIDX_VAR_2:
            break;
          case GET_UPDATED_1:
          case GET_UPDATED_2:
            break;
          case GET_MEM:         /* reply is block with data */
            *netError = message->error;
            if (getMemPtr)
              memcpy(getMemPtr, 
                    &message->mix.mem.buff[0], message->mix.mem.size);
            taskAccomplished();
/*
            issuedCommand = CMD_NO;
            issuedCommandAt = 0;
*/
            getMemPtr = 0;
            break;
          case PUT_MEM:         /* reply is ok */
            *netError = message->error;
            taskAccomplished();
/*
            issuedCommand = CMD_NO;
            issuedCommandAt = 0;
*/
            break;
          case LOAD_PROGRAM:    /* reply is ok */
            *netError = message->error;
            taskAccomplished();
/*
            issuedCommand = CMD_NO;
            issuedCommandAt = 0;
*/
            break;
          case REBOOT:          /* no reply ! slave is down !!! */
            break;
          case NEW:
            *netError = message->error;
            taskAccomplished();
/*
            issuedCommand = CMD_NO;
            issuedCommandAt = 0;
*/
            break;
          default:
            break;
        }
        status = _idle;           /* ??? one for send/receive ??? */
      }
     } else if (status == _idle)
    {
      if (sysVars->netCommand != CMD_NO)
      {
        int id;
        size = 0;
        id = sysVars->varId;
        toNode = sysVars->nodeId;
        message = (struct _message *) sBuf;
        message->error = NET_NOERROR;
        switch (sysVars->netCommand) {
          case CMD_GETVAR:
            if (bindModules(&dm, &meta, &aldm, &dmBackup)) /* ok ! */
            {
              long now;
              struct _remote { long timeStamp; } *remotePtr;
              
              remotePtr = (struct _remote *) metaRemoteData(dm, meta, id);
              now = getRelTime();

              if (DEBUG) printf("CMD_GETVAR: id=%d\n", id);
              if (abs(now - remotePtr->timeStamp) < 60) {
                size = 0;
                if (DEBUG) printf("CMD_GETVAR: d=%d\n", now - remotePtr->timeStamp);
              } else {
                if (lookUpRequest(id)) {
                  size = 0;
                  if (DEBUG) printf("CMD_GETVAR: already in queue\n");
                }
                else {
                  int sts;
                  sts = insertRequest(id, toNode); /* if too many, it will be skipped */
                  if (DEBUG) printf("CMD_GETVAR: insert says %d\n", sts);
               
                  if (noOfVarRequests == 1) { 
                    timeStampRequest(id);
                    remotePtr->timeStamp = now;
                    strcpy(message->mix.varRequest.varName, 
                                  (char *) metaName(meta, id));
                    message->message_type = GET_VAR | REQUEST_MASK;
                    message->error = 0;
                    size = 2 + strlen(message->mix.varRequest.varName) + 1;
                  } else 
                    size = 0;
                }
              }
            } else
              if (DEBUG) printf("cant bind data modules !\n");
      
if (DEBUG) {
  int i;           
  printf("CMD_GETVAR: siz=%d (reqs:%d)\n", size, noOfVarRequests);
  for (i = 0; i < noOfVarRequests; i++)
    printf("%02d,", varRequest[i].varId);
  printf("\n");
}


            sysVars->netCommand = CMD_NO;
            taskAccomplished();
            break;
          case CMD_PUTVAR:
            if (bindModules(&dm, &meta, &aldm, &dmBackup)) /* ok ! */
            {
              int pos;
              
              message->message_type = PUT_VAR | REQUEST_MASK;
              message->error = 0;
              strcpy(message->mix.varRequest.varName, 
                                (char *) metaName(meta, id));
              pos = strlen(message->mix.varRequest.varName) + 1;
              size = 2 + pos + 1;
              if (metaType(meta, id) == TYPE_INT) {
                message->mix.varRequest.varName[pos] = TYPE_INT;
                memcpy(&message->mix.varRequest.varName[pos + 1], 
                              metaValue(dm, meta, id), sizeof(long));
                size += sizeof(long);
              } else if (metaType(meta, id) == TYPE_FLOAT) {
                message->mix.varRequest.varName[pos] = TYPE_FLOAT;
                memcpy(&message->mix.varRequest.varName[pos + 1], 
                              metaValue(dm, meta, id), sizeof(double));
                size += sizeof(double);
              } else {
                message->error = NET_ILLEGALTYPE;
                size = 0;     /* MUST FLAG ERROR TO HOST PROCESS !!!! */
              }
            }
            sysVars->netCommand = CMD_NO;
            taskAccomplished();
            break;
          case CMD_SETHOST:
            message->message_type = SET_HOST | REQUEST_MASK;
            setHostNode = sysVars->nodeId;
            sysVars->netCommand = CMD_NO;
            size = 3;
            break;
          case CMD_SET_REMOTE:
            message->message_type = SET_REMOTE | REQUEST_MASK;
            setRemoteNode = sysVars->nodeId;
            setRemoteTime = getRelTime();
            sysVars->netCommand = CMD_NO;
            *shellError = 0;
            size = 2;
            break;
          case CMD_NEW:
            message->message_type = NEW | REQUEST_MASK;
            message->error = 0;
            size = 2;
/*
            issuedCommand = sysVars->netCommand;
            issuedCommandAt = time(0);
*/
            sysVars->netCommand = CMD_NO;
            break;
          case CMD_ALLOCATE_MEM:
            size = 0;
            sysVars->netCommand = CMD_NO;
            break;
          case CMD_GET_MEM:
            message->message_type = GET_MEM | REQUEST_MASK;
            message->error = 0;
/*  not impl yet !!
            message->mix.mem.address = ((long *) sysVars->netAPIarea)[0];
            message->mix.mem.size    = ((long *) sysVars->netAPIarea)[1];
            getMemPtr                = (char *) ((long *) sysVars->netAPIarea)[2];
*/
            size = 2 + 4 + 1;
/*
            issuedCommand = sysVars->netCommand;
            issuedCommandAt = time(0);
*/
            sysVars->netCommand = CMD_NO;
            break;
          case CMD_PUT_MEM:
            message->message_type = PUT_MEM | REQUEST_MASK;
            message->error = 0;
/*
!   the api struct is 
  struct { long address; long size; char *sourcePtr };
*/
/*  not impl yet !!
            message->mix.mem.address = ((long *) sysVars->netAPIarea)[0];
            message->mix.mem.size    = ((long *) sysVars->netAPIarea)[1];
            memcpy(message->mix.mem.buff, 
                ((long *) sysVars->netAPIarea)[2],
                message->mix.mem.size);
            size = 2 + 4 + 1 + message->mix.mem.size;
*/
/*
            issuedCommand = sysVars->netCommand;
            issuedCommandAt = time(0);
*/
            sysVars->netCommand = CMD_NO;
            break;
          case CMD_LOAD_PROGRAM:
            size = 0;
            sysVars->netCommand = CMD_NO;
            break;
          case CMD_REBOOT:
            message->message_type = REBOOT | REQUEST_MASK;
            message->error = 0;
            size = 2;
            *netError = NET_NOERROR;
            taskAccomplished();
            sysVars->netCommand = CMD_NO;
            break;
          default:
            break;
        }
        if (size)
          sendPacket(toNode, sBuf, size);  /* must receive answer if failed */

/*         sysVars->netCommand = CMD_NO;  */
      }
    } else if (status == _timeout)
    {
      if (DEBUG)
         printf("TaskAccomplished, timeout received\n");
      taskAccomplished();
      status = _idle;
    }
  }
}

int unfoldPacket(rBuf, rSize)
char *rBuf;
int *rSize;
{
  *rSize = receiver.frame.head.size - sizeof(struct _header) - sizeof(char);
  if (*rSize < 0) {
    *rSize = 0;
    rBuf[0] = 0xff;       /* no message type ! */
  } else 
    memcpy(rBuf, receiver.frame.buf, *rSize);
  status = _idle;
}

int sendPacket(node, buf, size)
int node;
char *buf;
int size;
{
  int master = 0, nb;
  /* pack it */

  if ((netUp == 0) || (netOutBufSize() > 600)) { /* changed 300->600, 930517 */
    if (netUpPtr > (int *) 1)  /* tell users idc variable 'netUp' */ /* added 920709 */
      *netUpPtr = 0;   /* that net is down */
    return 0;
  } else {
    if (netUpPtr > (int *) 1)  /* tell users idc variable 'netUp' */ /* added 920709 */
      *netUpPtr = 1;   /* that net is up */
  }
  transmitter.frame.head.size = sizeof(struct _header) + size + sizeof(char);
  transmitter.frame.head.version = 1;
  transmitter.frame.head.targetMaster = 0;
  transmitter.frame.head.targetNode = node;
  transmitter.frame.head.sourceMaster = 0;
  transmitter.frame.head.sourceNode = *nodeaddr;
  transmitter.frame.head.command =  0;        /* data block */
  memcpy(transmitter.frame.buf, buf, size);
  transmitter.frame.buf[size] = 0;
  transmitter.frame.buf[size] = allBlockChs(&transmitter);
  if (netUp)
    nb = netwrite(0, &transmitter, transmitter.frame.head.size);

  if (DEBUG) printf("netwrite=%d (%d)\n", nb, transmitter.frame.head.size);

  if (DEBUG) {
    int i;
    unsigned char *p;
    p = (unsigned char *) &transmitter;
    for (i = 0; i < transmitter.frame.head.size; i++)
      printf("%02x, ", *p++);
    printf("\n");
  }
  if (netUp)
    netflush();           
  if (DEBUG)
    printf("sendPacket: netwrite ok!-----------\n");

  status = _packetSent;
}

int ntread(node, buf, size)
int node;
char *buf;
int size;
{
  int bt, pt;
  pt = 0;
  while ((bt = netread(node, &buf[pt], size)) > 0) {
/*
    printf("ntread: size = %d, bt = %d, -> more = %d\n", size, bt, size - bt);
*/    
    size -= bt;
    pt += bt;
  }
  return pt;
}

int getPacket(buf, size)
unsigned char *buf;               /* new 920313 */
int size;
{
  int pt, more, bt, sz;

  ntread(0, &buf[0], 1);
  size --;
  more = buf[0] - 1;
  pt = 1;
  while (more > 0) {
    while (size && more) {
      sz = (more > size) ? size : more;
      bt = ntread(0, &buf[pt], sz);
/*
    printf("gP: sz = %d, bt = %d, -> more = %d\n", sz, bt, sz - bt);
*/
      pt += bt; more -= bt; size -= bt;
    }
    if (more && ((size = netpoll(0)) == -1)) {
/*      return 0;   */
      if (DEBUG)
        printf("getPacket: ??? error ?? (bt = %d)\n", bt);
      size = 0;
    }
  }
  if (DEBUG)
    printf("getPacket: bt was %d\n", bt);
  return 1;
}

int dumpPacket(buf)
unsigned char *buf;
{
  int i;

  printf("Packet size %d bytes, protocol version %d\n", buf[0], buf[1]);
  printf("To node %d, from node %d, command %d\n", buf[3], buf[5], buf[6]);

  printf("Packet data:\n");
  for (i = 7; i < buf[0]; i++)
    printf("%02x, ", buf[i]);
  printf("\n");
}

int layer2()
{
  int i, bt, nb;
  int cnt = 0;
  
  bt = 0;


  if ((nb = netpoll(0)) > 0) {          /* removed new 920313 */
/*
    nb = netpoll(0);                    
    nb = netpoll(0);
*/
    getPacket(&receiver, nb);
    if (DEBUG)
      printf("Efter: nb = %d\n", nb);
  }
  else
    return _idle;

  if (DEBUG)
    dumpPacket(&receiver);


  if (receiver.frame.head.size < (sizeof(struct _header) + sizeof(char))) {
    if (DEBUG) printf("framing error. size must be gte %d\n", 
                              sizeof(struct _header) + sizeof(char));
    return _idle;
  }

    if (allBlockChs(&receiver))        /* should be zero */
    {  /* wrong checksum, reply with previously correct packet received */

      if (DEBUG) printf("wrong checksum !\n");
      return _idle;
    }


  return _packetReceived;
   
} /* end of layer2 */
