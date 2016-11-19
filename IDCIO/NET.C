/* net.c  1993-07-22 TD,  version 1.4 */
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
! net.c
! Copyright (C) 1991-1993 IVT Electronic AB.
*/

/*
!   history:
!   date       by  rev  ed#  what
!   ---------- --- ---  ---  ------------------------------------
!   1991-??-?? td  1.00   1  initial coding
!
!     92-03-17 td  1.10      ?
!
!     92-09-24 td  1.20      added netRebootNode(), netGetMem() & netPutMem()
!                            netNew()
!
!     93-02-25 td  1.30      Now uses sysVars for netAPIarea and netCommand
!                            and these have changed their absolute addresses
!
!     93-07-22 td  1.40      The netGetNIdx function returns NET_TIMEOUT
!                            if no server is available
*/

#include <math.h>      /* atof */
#include <time.h>

/* #define NO_OF_ALARMS 1 */
#ifdef OSK
/* #include "../alarm.h"  */
#include "sysvars.h"
#include "../ivtnet/v1.1/net.h"
#include "../ivtnet/v1.1/ivtnet.h"
#include "meta.h"
#else
#include "sysvars.h"
/* #include "alarm.h" */
#include "net.h"
#include "ivtnet.h"
#include "meta.h"
#endif
#include <errno.h>

struct _node
{
  unsigned char failCount;
  unsigned char full;               /*  added 920306 */  
  short size;
  unsigned short int rCount, sCount;
  unsigned short noOfAlarms;       /* come, gone, disable, confirm -> 4*100 */

  unsigned char pollCount;        /* incremented for each poll */
  unsigned char failFlag;
/*  added 920306 */
  unsigned long activeABCD;
  unsigned long confirmedABCD;
  unsigned short version;
};    /* size = 22 * MAX_NODES = 1408 */

/*
!   Needs the following vars from sys-area:
!
!     slavePid        - process id of slave process
!     varId           - buffer of variable identifier for 
!                       interprocess communication to slave
!     nodeId          - buffer of node identifier
!
*/

static struct _system *sysVars = SYSTEM_AREA;

/*
static unsigned char *currentCommand = COMMAND_ADDRESS;
*/

static int NetFree = 0, NetTaskAccomplished = 0;

/*
static unsigned char *netError = 0x003ffd9;
*/

init()
{
  if ((NetFree = _ev_link(NET_EVENT_FREE)) == -1) {
    return 0;
  }
  if ((NetTaskAccomplished = 
                    _ev_link(NET_EVENT_TASK_ACCOMPLISHED)) == -1) {
    NetFree = 0;
    return 0;
  }
  return 1;
}

int WAIT_FOR_EVENT(resource) 
int resource;
{
    int sts;
    do {
      if ((sts = _ev_wait(resource, 1, 1)) == -1) {
        NetTaskAccomplished = 0;
        NetFree = 0;
        return 0;
      }
    } while (sts != 1) ;
    return 1;
}
  

int getExtern(id, node)
int id, node;
{

  return 0;
 
#if 0
  if (!sysVars->slavePid)
    return 0;
  if ((NetTaskAccomplished > 0) || init())
  {
    if (!WAIT_FOR_EVENT(NetFree)) return 0;
    sysVars->varId = id;
    sysVars->nodeId = node;

    sysVars->netCommand = CMD_GETVAR;
/*    *currentCommand = CMD_GETVAR;
/*
    kill(sysVars->slavePid, CMD_GETVAR);
*/
    if (!WAIT_FOR_EVENT(NetTaskAccomplished)) return 0;
    _ev_signal(NetFree, 0);
  }
#endif
}

int netGetNoOfAlarms(node)
int node;
{
  unsigned short int noOfAlarms = 0;
  if (!sysVars->slavePid)
    return 0;
  if ((NetTaskAccomplished > 0) || init())
  {
    if (!WAIT_FOR_EVENT(NetFree)) return 0;
    sysVars->nodeId = node;
    sysVars->netCommand = CMD_ALARMS;
/*    *currentCommand = CMD_ALARMS; */
    if (!WAIT_FOR_EVENT(NetTaskAccomplished)) return 0;
    memcpy(&noOfAlarms, &sysVars->varId, 2);
    _ev_signal(NetFree, 0);
  }
  return noOfAlarms;
}

/*
 static unsigned char *sysVars->netAPIarea = ALARM_TEXT_ADDRESS;
*/

int netConfirmAlarm(node, alarmNo, serialNo, status)
int node, alarmNo, serialNo, status;
{
  short int *buf;
  int errCode;
  if (!sysVars->slavePid)
    return 0;
  if ((NetTaskAccomplished > 0) || init())
  {
    if (!WAIT_FOR_EVENT(NetFree)) return 0;
    sysVars->nodeId = node;
    buf = (short int *) sysVars->netAPIarea;
    buf[0] = alarmNo;
    buf[1] = serialNo;
    buf[2] = status;
    sysVars->netCommand = CMD_CONFIRM_ALARM;
/*    *currentCommand = CMD_CONFIRM_ALARM;  */
    if (!WAIT_FOR_EVENT(NetTaskAccomplished)) return 0;
    if (errCode = sysVars->netError)
      sysVars->netError = 0;
    _ev_signal(NetFree, 0);
    return errCode;
  }
  return 0;
}

int netPutIdxVar(node, name, ASCIIvalue)
int node;
char *name, *ASCIIvalue;
{
  double dValue;
  long iValue;

  if (!sysVars->slavePid)
    return 0;
  if ((NetTaskAccomplished > 0) || init())
  {
    if (!WAIT_FOR_EVENT(NetFree)) return 0;
    sysVars->nodeId = node;

    iValue = atoi(ASCIIvalue);
    dValue = atof(ASCIIvalue);
    if (iValue == dValue) {
      sysVars->netAPIarea[8] = TYPE_INT;
      memcpy(sysVars->netAPIarea, &iValue, 4);
    } else {
      sysVars->netAPIarea[8] = TYPE_FLOAT;
      memcpy(sysVars->netAPIarea, &dValue, 8);
    }
    strcpy(&sysVars->netAPIarea[9], name);
    sysVars->netCommand = CMD_PUTIDX_VAR;
/*    *currentCommand = CMD_PUTIDX_VAR; */
    if (!WAIT_FOR_EVENT(NetTaskAccomplished)) return 0;
    _ev_signal(NetFree, 0);
  }
}

/*
        struct { short int node, idx; char name[32]; double value;} method1Arr[10];
*/

/*
struct _method1Arr {
  short int node, idx; char name[32]; double value;
};
*/

int netGetNIdxVar(node, N, arr, useVars)    /* this was double ??? */
int node, N;
struct _method1Arr *arr;
int useVars;
{
  short int *buf;
  long iValue;
  int errCode;

  if (!sysVars->slavePid)
    return NET_TIMEOUT;   /* changed 930722 */
  if ((NetTaskAccomplished > 0) || init())
  {
    if (!WAIT_FOR_EVENT(NetFree)) return 0;
    sysVars->nodeId = node;
    sysVars->varId = N | (useVars ? 0x0100 : 0);
    *(((struct _method1Arr **) sysVars->netAPIarea)) = arr;
/*
    buf = (short int *) sysVars->netAPIarea;
    buf[0] = PCidx;
    strcpy((char *) &buf[1], name);
*/
    sysVars->netCommand = CMD_GET_N_IDX_VAR;
/*    *currentCommand = CMD_GET_N_IDX_VAR;    */
    if (!WAIT_FOR_EVENT(NetTaskAccomplished)) return 0;
    if (errCode = sysVars->netError)
      sysVars->netError = 0;
/*
    if (sysVars->netAPIarea[0] == TYPE_INT) {
      memcpy(&iValue, &sysVars->netAPIarea[1], sizeof(long));
      *value = (double) iValue;
    } else
      memcpy(value, &sysVars->netAPIarea[1], sizeof(double));
*/
    _ev_signal(NetFree, 0);
    return errCode;
  }
  return NET_TIMEOUT;   /* changed 930722 */
}

int netGetIdxVar(node, PCidx, name, value, useVars)   /* this was double ??? */
int node, PCidx;
char *name;
double *value;
int useVars;
{
  short int *buf;
  long iValue;
  int errCode;

  *value = 0;
  if (!sysVars->slavePid)
    return 0;
  if ((NetTaskAccomplished > 0) || init())
  {
    if (!WAIT_FOR_EVENT(NetFree)) return 0;
    sysVars->varId = (useVars ? 0x0100 : 0);
    sysVars->nodeId = node;
    buf = (short int *) sysVars->netAPIarea;
    buf[0] = PCidx;
    strcpy((char *) &buf[1], name);
    sysVars->netCommand = CMD_GETIDX_VAR;
/*    *currentCommand = CMD_GETIDX_VAR; */
    if (!WAIT_FOR_EVENT(NetTaskAccomplished)) return 0;
    if (errCode = sysVars->netError)
      sysVars->netError = 0;

    if (sysVars->netAPIarea[0] == TYPE_INT) {
      memcpy(&iValue, &sysVars->netAPIarea[1], sizeof(long));
      *value = (double) iValue;
    } else
      memcpy(value, &sysVars->netAPIarea[1], sizeof(double));
    _ev_signal(NetFree, 0);
    return errCode;
  }
  return 0;
}

int netGetUpdated(node, buf, useVars)
int node;
char *buf;
int useVars;
{
  int errCode;
  if (!sysVars->slavePid)
    return 0;
  if ((NetTaskAccomplished > 0) || init())
  {
    if (!WAIT_FOR_EVENT(NetFree)) return 0;
    sysVars->varId = (useVars ? 0x0100 : 0);
    sysVars->nodeId = node;
    sysVars->netCommand = CMD_GET_UPDATED;
/*    *currentCommand = CMD_GET_UPDATED;  */
    if (!WAIT_FOR_EVENT(NetTaskAccomplished)) return 0;
    if (errCode = sysVars->netError)
      sysVars->netError = 0;
    memcpy(buf, sysVars->netAPIarea, ALARM_TEXT_SIZE);
    _ev_signal(NetFree, 0);
    return errCode;
  }
  return 0;
}

int netGetCal(node, name, idx, calendar)
int node;
char *name;
int *idx;
char *calendar;
{
  int errCode;
  
  if (!sysVars->slavePid)
    return 0;
  if ((NetTaskAccomplished > 0) || init())
  {
    if (!WAIT_FOR_EVENT(NetFree)) return 0;
    sysVars->nodeId = node;
    sysVars->netCommand = CMD_GET_CAL;
/*    *currentCommand = CMD_GET_CAL;  */
    ((long *) sysVars->netAPIarea)[0] = (long) calendar;
    ((long *) sysVars->netAPIarea)[1] = (long) idx;
    strcpy(&sysVars->netAPIarea[8], name);
    if (!WAIT_FOR_EVENT(NetTaskAccomplished)) return 0;
    if (errCode = sysVars->netError)
      sysVars->netError = 0;
    _ev_signal(NetFree, 0);
    return errCode;
  }
  return 0;
}

int netSetCal(node, idx, calendar)
int node;
int idx;
char *calendar;
{
  int errCode;
  if (!sysVars->slavePid)
    return 0;
  if ((NetTaskAccomplished > 0) || init())
  {
    if (!WAIT_FOR_EVENT(NetFree)) return 0;
    sysVars->nodeId = node;
    ((long *) sysVars->netAPIarea)[0] = (long) calendar;
    ((short *) (&((long *) sysVars->netAPIarea)[1]))[0] = idx;
    sysVars->netCommand = CMD_SET_CAL;
/*    *currentCommand = CMD_SET_CAL;    */
    if (!WAIT_FOR_EVENT(NetTaskAccomplished)) return 0;
    if (errCode = sysVars->netError)
      sysVars->netError = 0;
    _ev_signal(NetFree, 0);
    return errCode;
  }
  return 0;
}

int netGetAlarmText(node, buf, pc)
int node;
char *buf;
int pc;
{
  int errCode;
  if (!sysVars->slavePid)
    return 0;
  if ((NetTaskAccomplished > 0) || init())
  {
    if (!WAIT_FOR_EVENT(NetFree)) return 0;
    sysVars->nodeId = node;
    sysVars->varId = pc;
    sysVars->netCommand = CMD_GETALARM_TEXT;
/*    *currentCommand = CMD_GETALARM_TEXT;    */
    if (!WAIT_FOR_EVENT(NetTaskAccomplished)) return 0;
    if (errCode = sysVars->netError)
      sysVars->netError = 0;
/*
      changed  ALARM_TEXT_SIZE to sizeof(struct _alarmStrct) !!!!
      bugfix 920302 !!!   100 -> 90 !!! 
*/
    memcpy(buf, sysVars->netAPIarea, sizeof(struct _alarmStrct));
    _ev_signal(NetFree, 0);
    return errCode;
  }
  return 0;
}

int netAckAlarm(node, pc, mask)
int node, pc;
unsigned char mask[4];
{
  if (!sysVars->slavePid)
    return 0;
  if ((NetTaskAccomplished > 0) || init())
  {
    if (!WAIT_FOR_EVENT(NetFree)) return 0;
    sysVars->nodeId = node;
    sysVars->varId = pc;
    memcpy(sysVars->netAPIarea, mask, 4);
    sysVars->netCommand = CMD_ACK_ALARM;
/*    *currentCommand = CMD_ACK_ALARM;    */
    if (!WAIT_FOR_EVENT(NetTaskAccomplished)) return 0;
    _ev_signal(NetFree, 0);
  }
}

int netClearAlarm(node)
int node;
{
  int errCode;
  if (!sysVars->slavePid)
    return -1;
  if ((NetTaskAccomplished > 0) || init())
  {
    if (!WAIT_FOR_EVENT(NetFree)) return 0;
    sysVars->nodeId = node;
    sysVars->netCommand = CMD_CLEAR_ALARM;
/*    *currentCommand = CMD_CLEAR_ALARM;  */
    if (!WAIT_FOR_EVENT(NetTaskAccomplished)) return 0;
    if (errCode = sysVars->netError)
      sysVars->netError = 0;
    _ev_signal(NetFree, 0);
    return errCode;
  }
  return -1;
}

#ifdef VERY_MUCH_EPROM
int putExtern(id, node)
int id, node;
{
  if (!sysVars->slavePid)
    return 0;
  if ((NetTaskAccomplished > 0) || init())
  {
    if (!WAIT_FOR_EVENT(NetFree)) return 0;
    sysVars->varId = id;
    sysVars->nodeId = node;
    sysVars->netCommand = CMD_PUTVAR;
/*    *currentCommand = CMD_PUTVAR;   */
    if (!WAIT_FOR_EVENT(NetTaskAccomplished)) return 0;
    _ev_signal(NetFree, 0);
  }
}
#else
int putExtern() {}
#endif

/*
!   set host, initiate slave process to take control
!   returns;
!               0   cannot find slave process
!               1   ok
!              <0   host duc not available  (unknown error from slave process)
*/
int netSetHost(node)
int node;
{
  int errCode;
  if (!sysVars->slavePid)
    return 0;
  if ((NetTaskAccomplished > 0) || init())
  {
    if (!WAIT_FOR_EVENT(NetFree)) return 0;
    sysVars->nodeId = node;
   
    sysVars->useSetB = 1;
    sysVars->currentBitsB = 0;
    sysVars->flashBitsB = 0;
    sysVars->flashBits2B = 0;

    sysVars->netCommand = CMD_SETHOST;
/*    *currentCommand = CMD_SETHOST;    */

/*
!   OBS!! HANGMAN !!! WILL NOT RETURN FROM NetTaskAccomplished
!   UNTIL SET HOST RETURNED, I.E. NO OTHER PROCESS CAN FETCH DATA
!   BUT THE MASTER WILL STILL BE ABLE TO FETCH ALARMS !!!!
*/

    if (!WAIT_FOR_EVENT(NetTaskAccomplished)) return 0;
    sysVars->useSetB = 0;

    _ev_signal(NetFree, 0);
    if (errCode = sysVars->netError)
      sysVars->netError = 0;
    return errCode;
  }
  return 0;
}

/*
!   set remote, spawn a shell at remote site
!   returns;
!               0   cannot find slave process
!               1   ok
!              <0   error spawning shell, ( -message->error)
*/
int netSetRemote(node)
int node;
{
  int errCode;
  if (!sysVars->slavePid)
    return 0;
  if ((NetTaskAccomplished > 0) || init())
  {
    if (!WAIT_FOR_EVENT(NetFree)) return 0;
    sysVars->nodeId = node;
    sysVars->netCommand = CMD_SET_REMOTE;
/*    *currentCommand = CMD_SET_REMOTE;   */
    if (!WAIT_FOR_EVENT(NetTaskAccomplished)) return 0;
    _ev_signal(NetFree, 0);
    if (!sysVars->netError)
      return 1;
    errCode = sysVars->netError;
    sysVars->netError = 0;
    return -errCode;
  }
  return 0;
}

int netActiveABCD(node, A, B, C, D)
int node, *A, *B, *C, *D;
{
  struct _node *nodeMap = (struct _node *) 0x0003f000;
  char alarms[4];

  node &= 63; 
  memcpy(alarms, &nodeMap[node].activeABCD, 4);
  *A = alarms[0];
  *B = alarms[1];
  *C = alarms[2];
  *D = alarms[3];
  return (nodeMap[node].size >= 0);
}

int netConfirmedABCD(node, A, B, C, D)
int node, *A, *B, *C, *D;
{
  struct _node *nodeMap = (struct _node *) 0x0003f000;
  char alarms[4];
 
  node &= 63; 
  memcpy(alarms, &nodeMap[node].confirmedABCD, 4);
  *A = alarms[0];
  *B = alarms[1];
  *C = alarms[2];
  *D = alarms[3];
  return (nodeMap[node].size >= 0);
}

int netRebootNode(node)
int node;
{
  int errCode;
  if (!sysVars->slavePid)
    return -1;
  if ((NetTaskAccomplished > 0) || init())
  {
    if (!WAIT_FOR_EVENT(NetFree)) return -1;    /* 0 */
    sysVars->nodeId = node;
    sysVars->netCommand = CMD_REBOOT;
/*    *currentCommand = CMD_REBOOT;   */
    if (!WAIT_FOR_EVENT(NetTaskAccomplished)) return 0;
    if (errCode = sysVars->netError)
      sysVars->netError = 0;
    _ev_signal(NetFree, 0);
    return errCode;
  }
  return -1;
}

/*
!   the api struct for GET_MEM and PUT_MEM is 
  struct { long address; long size; char *sourcePtr };
*/

int netGetMem(node, address, size, buf)
int node;
char *address, *buf;
long size;
{
  int errCode;
  if (!sysVars->slavePid)
    return -1;
  if ((NetTaskAccomplished > 0) || init())
  {
    if (!WAIT_FOR_EVENT(NetFree)) return -1;    /* 0; */
    sysVars->nodeId = node;
/*    sysVars->varId = pc;  context ?? */
    ((long *) sysVars->netAPIarea)[0] = (long) address;
    ((long *) sysVars->netAPIarea)[1] = (long) size;
    ((long *) sysVars->netAPIarea)[2] = (long) buf;
    sysVars->netCommand = CMD_GET_MEM;
/*    *currentCommand = CMD_GET_MEM;  */
    if (!WAIT_FOR_EVENT(NetTaskAccomplished)) return 0;
    if (errCode = sysVars->netError)
      sysVars->netError = 0;
    _ev_signal(NetFree, 0);
    return errCode;
  }
  return -1;
}

int netPutMem(node, address, size, buf)
int node;
char *address, *buf;
long size;
{
  int errCode;
  if (!sysVars->slavePid)
    return -1;
  if ((NetTaskAccomplished > 0) || init())
  {
    if (!WAIT_FOR_EVENT(NetFree)) return -1;    /* 0; */
    sysVars->nodeId = node;
/*    sysVars->varId = pc;  context ?? */
    ((long *) sysVars->netAPIarea)[0] = (long) address;
    ((long *) sysVars->netAPIarea)[1] = (long) size;
    ((long *) sysVars->netAPIarea)[2] = (long) buf;
    sysVars->netCommand = CMD_PUT_MEM;
/*    *currentCommand = CMD_PUT_MEM;    */
    if (!WAIT_FOR_EVENT(NetTaskAccomplished)) return 0;
    if (errCode = sysVars->netError)
      sysVars->netError = 0;
    _ev_signal(NetFree, 0);
    return errCode;
  }
  return -1;
}

int netNew(node)
int node;
{
  int errCode;
  if (!sysVars->slavePid)
    return -1;
  if ((NetTaskAccomplished > 0) || init())
  {
    if (!WAIT_FOR_EVENT(NetFree)) return -1;
    sysVars->nodeId = node;
    sysVars->netCommand = CMD_NEW;
/*    *currentCommand = CMD_NEW;  */
    if (!WAIT_FOR_EVENT(NetTaskAccomplished)) return 0;
    if (errCode = sysVars->netError)
      sysVars->netError = 0;
    _ev_signal(NetFree, 0);
    return errCode;
  }
  return -1;
}

