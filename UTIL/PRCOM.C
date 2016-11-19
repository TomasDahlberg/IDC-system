/* prCom.c  1993-09-23 TD,  version 1.31 */
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
! prCom.c
! Copyright (C) 1992,1993 IVT Electronic AB.
*/

/*
!   prCom     - alarm printer driver
!
!   history:
!   date       by  rev  what
!   ---------- --- ---  ------------------------------------
!   1992-02-13 td  1.00 initial coding
!
!   1992-06-25 td  1.10 Changed \6 to \x86 in "larmtillst\6nd"
!
!   1992-11-12 td  1.20 Added masterNo from idc-var 'masterNo' or dipswitch
!                       Edition #2
!
!   1993-03-19 td  1.30 BUGFIX !!  Changed alarm structure according 
!                       to 920914-fix
!
!   1993-09-23 td  1.31 Linking to idcutil trap module
!
! Function:
!     This program will run at master site. The program continously polls
!     for alarms and upon reception of an alarm, writes it to stdout.
!     This program must be started with a parameter telling what PC we
!     are simulating. Only alarms according to the 'globalAlarmMask_X' will
!     be fetched.
!
*/

@_sysedit: equ 4
@_sysattr: equ $8001

#define _V920914_1_40

#include <stdio.h>
#include <time.h>
#define NO_OF_ALARMS 1
#include "alarm.h"
#include "ivtnet/v1.1/net.h"
#include "meta.h"

int DEBUG = 0;
int MASTER_NO = 0;
int PCno;
struct _alarmModule *aldm;
char *dm, *meta, *dmBackup;
char *headerPtr1, *headerPtr2, *headerPtr3, *headerPtr4;
unsigned char abcdMask[4];
static int internalCounter;     /* next alarm text to send after request !!! */
static int alarmMarkPtr[8];
static int alarmMarkNode = 0;
static char p1[10], p2[10], p3[10], p4[10], p5[10], p6[100];
char outBuf[256];

int timestamp;
int nice;

timeStamp()
{
  struct tm tid;
  static long lastHourDone;
  long x, t1;
  
  t1 = time(0);
  memcpy(&tid, localtime(&t1), sizeof(struct tm));
  x = (((((tid.tm_year > 100) ? tid.tm_year - 100 : tid.tm_year) * 100 + 
            tid.tm_mon + 1)*100 + tid.tm_mday)*100 + tid.tm_hour);
  if (x == lastHourDone)
    return 0;
  lastHourDone = x;
  if ((tid.tm_hour & 1) == 0) {
    printf("\n---- Time stamp %02d.%02d.%02d  %02d:%02d:%02d ----\n\n",
              (tid.tm_year > 100) ? tid.tm_year - 100 : tid.tm_year,
              tid.tm_mon + 1, tid.tm_mday, 
              tid.tm_hour, tid.tm_min, tid.tm_sec);
    return 1;
  }
  return 0;
}

int getMasterNo(dm, meta)     /* added 920921 */
char *dm, *meta;
{
  int id;
  char name[20];

  strcpy(name, "masterNo");
  if ((id = metaId(meta, name)) < 0) /* name not found */
    return 0;
  if (metaType(meta, id) == TYPE_INT)
    return *((int *) metaValue(dm, meta, id));
  else if (metaType(meta, id) == TYPE_FLOAT)
    return (int) (*((double *) metaValue(dm, meta, id)));
  else
    return 0;
}

main(argc, argv)
int argc;
char *argv[];
{
  timestamp = 1;
  nice = 0;

  while( argc >= 2  && argv[1][0] == '-' ) {
    while( *++(argv[1]) ) {
      switch( *argv[1] ) {
        case 't':
        case 'T':
          timestamp = 0;
          continue;
	case 'n':
	case 'N':
          nice = 1;
          continue;
        case '?':
          usage();
          exit(0);
	default:
          fprintf(stderr, "illegal option: %c", (char *) *argv[1]);
      }
    }
    argv++;
    argc--;
  }
  if (argc == 2)
    PCno = atoi(argv[1]);
  else {
    usage();
    exit(1);
  }
  initidcio();
  
  initidcutil();    /* added 930923 */
/*
  initphyio();
*/

  while (1) {
    bind(&dm, &aldm, &meta, &dmBackup,
                    &headerPtr1, &headerPtr2, &headerPtr3, &headerPtr4);
    if (dm && aldm && meta)
      break;
    sleep(1);
  }


  if (MASTER_NO == 0)
    MASTER_NO = getMasterNo(dm, meta);      /* added 921112 */
    
  if (MASTER_NO == 0) {                     /* added 921112 */
    static unsigned short *node = 0x402;
    if (*node & 512) {
      MASTER_NO = *node & 511;
    }
  }

  while (1) {
    if (timestamp)
      timeStamp();
    makeAlarmMask(dm, meta, abcdMask);
    if (anyAlarms(PCno)) {
      internalCounter = 0;
      if (doRequestAlarmText(p1, p2, p3, p4, p5, p6)) {
        struct tm tid;
        int class, masterNo = MASTER_NO;
        char star[4];
        unsigned short int bitmask, bit;

        class = ( ( (int) *((int *) p4) ) >> 3) & 0x0f;
        memcpy(&tid, localtime((time_t *) p5), sizeof(struct tm));

    bitmask = receiveAlarmMask(dm, meta, class);

    if (((int) *((int *) p4)) & 0x01)
      sprintf(star, "%s\0",
               (class == 0) ? "***" : (class == 1) ? "** " : "*  ");
    else
      sprintf(star, "%s\0",
               (class == 0) ? "..." : (class == 1) ? ".. " : ".  ");

#if 0
     sprintf(outBuf, "%05d%03d%03d%03d%05d%03d%s %02d.%02d.%02d %02d:%02d:%02d \"%s\"",
              bitmask, 
              masterNo,
              (int) *((int *) p1), (int) *((int *) p2),
              (int) *((int *) p3), 
              ((int) *((int *) p4)) & 0x07,  /* remove class bits
                                              since 02 will remove from DHC
                                              but not 18 decimal !!! */     
              star, 
              (tid.tm_year > 100) ? tid.tm_year - 100 : tid.tm_year,
              tid.tm_mon + 1, tid.tm_mday, 
              tid.tm_hour, tid.tm_min, tid.tm_sec, p6);
#endif

if (!nice) {
  int state, node, larmpkt, serie;
  
  state = ((int) *((int *) p4)) & 0x07;
  node = ((int) *((int *) p1));
  larmpkt = ((int) *((int *) p2));
  serie = ((int) *((int *) p3));

  printf("\n%s %02d.%02d.%02d %02d:%02d:%02d \"%s\"\n", star,
              (tid.tm_year > 100) ? tid.tm_year - 100 : tid.tm_year,
              tid.tm_mon + 1, tid.tm_mday, 
              tid.tm_hour, tid.tm_min, tid.tm_sec, p6);
  printf("!%s! larmtillst\x86nd !%s! kvitterat !%s! blockerat   SYSINFO (%d:%d:%d:%d:%d)\n\n",
              (state & 1) ? "  " : "Ej",
              (state & 2) ? "  " : "Ej",
              (state & 4) ? "  " : "Ej",
              masterNo, node, larmpkt, serie, state);

} else {
  int state;
  state = ((int) *((int *) p4)) & 0x07;
     sprintf(outBuf, "Alarm class %c\nNode: %d\nState %s%s%s\nDate: %02d.%02d.%02d\nTime: %02d:%02d:%02d\n%s\n",
              class + 'A', 
              (int) *((int *) p1),
              (state & 4) ? "Blocked " : "",
              (state & 2) ? "Confirmed " : "",
              (state & 1) ? "Active " : "Inactive",
              (tid.tm_year > 100) ? tid.tm_year - 100 : tid.tm_year,
              tid.tm_mon + 1, tid.tm_mday, 
              tid.tm_hour, tid.tm_min, tid.tm_sec, p6);
  printf("%s\n", outBuf);
}

      if (1 || 0 /* print ok ! */) {   /* ACK */
          if (alarmMarkNode)
            netAckAlarm(alarmMarkNode, PCno, abcdMask);
          else {
           setAlarmMask(abcdMask);
           switch (getAlarmSequence(&aldm->alarmList[alarmMarkPtr[PCno & 7]], PCno)) {
            case 1:   /* send assert */
              aldm->alarmList[alarmMarkPtr[PCno & 7]].assertSent |= (1 << PCno);
              break;
            case 2:
              aldm->alarmList[alarmMarkPtr[PCno & 7]].negateSent |= (1 << PCno);
              break;
            case 3:
              aldm->alarmList[alarmMarkPtr[PCno & 7]].confirmSent |= (1 << PCno);
              break;
            case 4:
              aldm->alarmList[alarmMarkPtr[PCno & 7]].disableSent |= (1 << PCno);
              if (aldm->alarmList[alarmMarkPtr[PCno & 7]].confirm == 1)
                aldm->alarmList[alarmMarkPtr[PCno & 7]].confirmSent |= (1 << PCno); 
                            /* !! autoconfirm, new 910909 */
              break;
            case 5:
              aldm->alarmList[alarmMarkPtr[PCno & 7]].enableSent |= (1 << PCno);
              break;
            default:
              printf("error, ???\n");
           }
          }
      }     /* ack alarm */
      }   /* if doReqAlarm... */
    } else    /* if any alarms */
      sleep(2);     /* no alarms */
  }       /* while forever */
}   /* end main */


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

struct _node *nodeMap = (struct _node *) 0x0003f000;
#define MAX_NODE_NO 64

int getAlarmsForAllNodes(mask, pc)
int mask, pc;
{
  int i, cnt;
  cnt = 0;
  
/*  mask |= (pc << 8);  */

  if (DEBUG) printf("getAlarmsForAllNodes: pc = %d, mask = %d\n", pc, mask);
  for (i = 1; i < MAX_NODE_NO; i++) {
    if ((mask & nodeMap[i].noOfAlarms) &&   /* changed 920203 */
        ((pc << 8) & nodeMap[i].noOfAlarms))
    {
      if (DEBUG) printf("node %d: mask(%d) & noOfAlarms(%d)\n", i, mask, nodeMap[i].noOfAlarms);
      
      cnt++;                  /*    cnt += nodeMap[i].noOfAlarms;   */
    }
  }
  if (DEBUG) printf("found %d alarms\n", cnt);
  return cnt;
}

int getNextAlarmNode(mask, pc)
int mask, pc;
{
  int i;

/*  mask |= (pc << 8);    */

  if (DEBUG) printf("getNextAlarmNode: pc = %d\n", pc);
  for (i = 1; i < MAX_NODE_NO; i++)
    if ((mask & nodeMap[i].noOfAlarms) &&     /* changed 920203,  > 0 -> & */
        ((pc << 8) & nodeMap[i].noOfAlarms))
      break;
  if (DEBUG) printf("Next node is %d (mask = %d, noOfAlarms = %d)\n", i,
                    mask, nodeMap[i].noOfAlarms);
  return (i < MAX_NODE_NO) ? i : 0;
}

/*
!   anyAlarms - a generic routine for retrieving number of pending alarms
*/
int anyAlarms(pcid)
int pcid;
{
  int cnt, count, i, m, pc, mask, as, ns, cs, ds, es;
  struct _alarmEntry *entry;
  
  pc = (1 << pcid);
  m = mask = 0;
  if (abcdMask[0] & pc) { m |= 1;  mask |= 0x00c0; }
  if (abcdMask[1] & pc) { m |= 2;  mask |= 0x0030; }
  if (abcdMask[2] & pc) { m |= 4;  mask |= 0x000c; }
  if (abcdMask[3] & pc) { m |= 8;  mask |= 0x0003; }

  cnt = aldm->alarmListPtr;
  count = 0;
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
    if (pc & (as | ns | cs | ds | es))
    {
      if (m & (1 << aldm->alarmList[i].class))
        count++;
    }
    entry++;              /* added 920306 .... (what a bug !) */
  }
  count += getAlarmsForAllNodes(mask, pc);
  return count;
}

usage()
{
  fprintf(stderr, "usage: prCom [option] PCno >/port\n");
  fprintf(stderr, "\n\nwhere PCno is valid 0 through 7\n");
  fprintf(stderr, "option:\n-n    nice output format\n-t    no time stamp information\n");
}

int makeAlarmMask(dm, meta, mask)
char *dm, *meta, *mask;
{
  int i;
  for (i = 0; i < 4; i++)
    mask[i] = receiveAlarmMask(dm, meta, i);
}

int receiveAlarmMask(dm, meta, class)
char *dm, *meta;
int class;    /* 0-3 = A - D */
{
  int id;
  char name[20];
  sprintf(name, "globalAlarmMask_%c", 'A' + class);

  if ((id = metaId(meta, name)) < 0) /* name not found */
    return 0;
  if (metaType(meta, id) == TYPE_INT)
    return *((int *) metaValue(dm, meta, id));
  else if (metaType(meta, id) == TYPE_FLOAT)
    return (int) (*((double *) metaValue(dm, meta, id)));
  else
    return 0;
}

bind(dm, aldm, meta, dmBackup, headerPtr1, headerPtr2, headerPtr3, headerPtr4)
char **dm, **meta, **dmBackup, 
        **headerPtr1, **headerPtr2, **headerPtr3, **headerPtr4;
struct _alarmModule **aldm;
{
/*
!   bind to data module VARS, storage location for variables
*/  
#define NAMEOFDATAMODULE "VARS"
  *dm = (char *) linkDataModule(NAMEOFDATAMODULE, headerPtr1);
  if (!*dm) {
    fprintf(stderr, "cannot link to datamodule '%s'\n", NAMEOFDATAMODULE);
    fprintf(stderr, "check if process 'scan' is running\n");
    return 0;
  }
/*
!   bind to data module META, storage for meta description of VARS module
*/
  *meta = (char *) linkDataModule("METAVAR", headerPtr2);
  if (!*meta) {
    fprintf(stderr, "cannot link to datamodule '%s'\n", "METAVAR");
    fprintf(stderr, "check if process 'scan' is running\n");
    return 0;
  }
/*
!   bind to data module ALARM, storage for alarm texts
*/
  *aldm = (struct _alarmModule *) linkDataModule("ALARM", headerPtr3);
  if (!*aldm) {
    fprintf(stderr, "cannot link to datamodule '%s'\n", "ALARM");
    fprintf(stderr, "check if process 'scan' is running\n");
    return 0;
  }
/*
!   bind to data module VAR1/VAR2, storage location for backup variables
*/  
#if 0
  *dmBackup = (char *) linkDataModule(
                    (useVars == 0) ? NAMEOFDATAMODULE1 : NAMEOFDATAMODULE2,
                                 headerPtr4);
  if (!*dmBackup) {
    fprintf(stderr, "cannot link to datamodule '%s'\n", 
                    (useVars == 0) ? NAMEOFDATAMODULE1 : NAMEOFDATAMODULE2);
    fprintf(stderr, "check if process 'scan' is running\n");
    return 0;
  }
#endif  
}

/*
!   All alarms are found at master site.
!   At the future, when the master table is full, a request
!   will be formed for each slave. 
*/
int doRequestNoOfAlarms(pcid)
int pcid;
{
  int count;
  
  if (pcid < 8) {
    count = anyAlarms(pcid);
    internalCounter = 0;
  } else
    count = 0;
  return count;
}

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
    if (entry->sendAssert == 0xff)
      entry->sendAssert = mask[entry->class & 0x03];
    if (entry->sendNegate == 0xff)
      entry->sendNegate = mask[entry->class & 0x03];
    if (entry->sendConfirm == 0xff)
      entry->sendConfirm = mask[entry->class & 0x03];
    if (entry->sendDisable == 0xff)
      entry->sendDisable = mask[entry->class & 0x03];
#endif
    entry++;
  }        
}

int doRequestAlarmText(node, alarm, serie, status, dtime, text)
int *node, *alarm, *serie, *status;
time_t *dtime;
char *text;
{
  int cnt, count, i, nd, m, mask;
  int PCvector;

  cnt = aldm->alarmListPtr;
  count = anyAlarms(PCno);

  PCvector = (1 << PCno);
  m = mask = 0;
  if (abcdMask[0] & PCvector) { m |= 1;  mask |= 0x00c0; }
  if (abcdMask[1] & PCvector) { m |= 2;  mask |= 0x0030; }
  if (abcdMask[2] & PCvector) { m |= 4;  mask |= 0x000f; }
  if (abcdMask[3] & PCvector) { m |= 8;  mask |= 0x0003; }

  setAlarmMask(abcdMask);
  {
    struct _alarmEntry entry, *this;
      
    for (; internalCounter < cnt; internalCounter++) {
#ifdef _V920914_1_40
      this = &aldm->alarmList[internalCounter];
      if (!(this->sendStatus & ALARM_SEND_INIT))
        continue;
      if (!(this->sendMask & PCvector))
        continue;
      if ((this->sendStatus & ALARM_SEND_ASSERT) &&
          !(this->assertSent & PCvector))
        break;
      if ((this->sendStatus & ALARM_SEND_NEGATE) &&
          !(this->negateSent & PCvector))
        break;
      if ((this->sendStatus & ALARM_SEND_CONFIRM) &&
          !(this->confirmSent & PCvector))
        break;
      if ((this->sendStatus & ALARM_SEND_DISABLE) &&
          !(this->disableSent & PCvector))
        break;
      if ((this->sendStatus & ALARM_SEND_ENABLE) &&   /* added 920921 */
          !(this->enableSent & PCvector))
        break;
#else
      if (((aldm->alarmList[internalCounter].sendAssert & PCvector) &&
          !(aldm->alarmList[internalCounter].assertSent & PCvector)) ||
          ((aldm->alarmList[internalCounter].sendNegate & PCvector) &&
          !(aldm->alarmList[internalCounter].negateSent & PCvector)) ||
          ((aldm->alarmList[internalCounter].sendConfirm & PCvector) &&
          !(aldm->alarmList[internalCounter].confirmSent & PCvector)) ||
          ((aldm->alarmList[internalCounter].sendDisable & PCvector) &&
          !(aldm->alarmList[internalCounter].disableSent & PCvector)))
        break;
#endif
    }

    alarmMarkNode = 0;
    if (internalCounter >= cnt) {
      struct _alarmStrct alarmBuf;

      if (!(nd = getNextAlarmNode(mask, PCvector)))
        return 0;

      if (netGetAlarmText(nd, &alarmBuf, PCno))
        return 0;
      
      *node = nd;
      *alarm = alarmBuf.alarmNo;
      *serie = alarmBuf.serialNo;
      *status = alarmBuf.status;
      *dtime = alarmBuf.dtime;
      convert(alarmBuf.text, text);
      alarmMarkNode = nd;
      return 1;
    }
    memcpy(&entry, &aldm->alarmList[internalCounter], 
                                    sizeof(struct _alarmEntry));
    *node = 0;
    *alarm = entry.alarmNo;
    *serie = entry.serialNo & 0xffff;
    switch (getAlarmSequence(&entry, PCno)) {
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
        if (entry.disableTime && (entry.offTime > entry.disableTime)) {
          if (entry.enableTime && (entry.offTime > entry.enableTime))
            ;
          else
            *status |= 4;
        }
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
        printf("Program fault: alarmtext switch\n");
        break;
    }        
    alarmMarkPtr[PCno & 7] = internalCounter;
    convert(entry.string, text);
    internalCounter++;
    return 1;
  }
}

convert(codestring, s)
char *codestring, *s;
{
  int i, len;

  strcpy(s, codestring);
  len = strlen(s);
  for (i = 0; i < len; i++) {
    if (s[i] == '\17') 
      s[i] = 0xf8;                 /* degree sign */
    else if (s[i] == '\06')
      s[i] = 0x86;
    else if (s[i] == '\04')
      s[i] = 0x84;
    else if (s[i] == '\24')
      s[i] = 0x94;
    else if (s[i] == '\20')        /* ??? code = ??? */
      s[i] = 0x8f;
    else if (s[i] == '\16')
      s[i] = 0x8e;
    else if (s[i] == '\31')
      s[i] = 0x99;
  }
}

#if 0
/*
!   for one alarm entry, checks what alarm to send
!   0 - no alarm pending for this entry
!   1 - assert state pending
!   2 - negate state pending
!   3 - confirm state pending
!   4 - disable state pending     (new 910517)
*/
getAlarmSequence(entry, PCno)
struct _alarmEntry *entry;
int PCno;
{
  int pc;
  int neg, conf, dis, en;
  pc = (1 << PCno);
  if (DEBUG) {
    printf("getAlarmSequence: pc = %d\n", pc);
  }
  if ((entry->sendStatus & ALARM_SEND_ASSERT) && 
      (entry->sendMask & pc) && !(entry->assertSent & pc))
    return 1;

  neg = ((entry->sendStatus & ALARM_SEND_NEGATE) && (entry->sendMask & pc) && 
          !(entry->negateSent & pc)) ? 1 : 0;
  conf = ((entry->sendStatus & ALARM_SEND_CONFIRM) && (entry->sendMask & pc) && 
          !(entry->confirmSent & pc)) ? 2 : 0;
  dis = ((entry->sendStatus & ALARM_SEND_DISABLE) && (entry->sendMask & pc) && 
            !(entry->disableSent & pc)) ? 4 : 0;
  en = ((entry->sendStatus & ALARM_SEND_ENABLE) && (entry->sendMask & pc) && 
            !(entry->enableSent & pc)) ? 8 : 0;

  switch (neg | conf | dis | en) {
    case 0:
      return 0; /*???*/
      break;
    case 1:
      return 2;	/* send negate event */
      break;
    case 2:
      return 3;	/* send confirm event */
      break;
    case 3:	/* negate & confirm */
      if (entry->confirmTime >= entry->offTime)         /* yes, who's first ?*/
        return 2;                                       /* inactive first */
      else
        return 3;
      break;
    case 4:
      return 4;	/* send disable event */
      break;
    case 5:	/* negate & disable */
      if (entry->disableTime >= entry->offTime)         /* yes, who's first ?*/
        return 2;                                       /* inactive first */
      else
        return 4;
      break;
    case 6:	/* confirm & disable */
      if (entry->disableTime <= entry->offTime)         /* yes, who's first ?*/
        return 4;                                       /* disable first */
      else
        return 2;
      break;
    case 7:	/* negate & confirm & disable */
      if (entry->disableTime >= entry->offTime) {
        if (entry->confirmTime >= entry->offTime)
	  return 2;
	else
	  return 3;
      } else {
        if (entry->confirmTime >= entry->disableTime)
	  return 4;
	else 
	  return 3;
      }
      break;
    case 8:
      return 5;	/* send enable event */
      break;
    case 9:	/* negate & enable */
      if (entry->enableTime >= entry->offTime)         /* yes, who's first ?*/
        return 2;                                      /* inactive first */
      else
        return 5;
      break;
    case 10:	/* confirm & enable */
      if (entry->enableTime >= entry->confirmTime)      /* yes, who's first ?*/
        return 3;                                       /* confirm first */
      else
        return 5;
      break;
    case 11:	/* negate & confirm & enable */
      if (entry->enableTime >= entry->offTime) {
        if (entry->confirmTime >= entry->offTime)
	  return 2;
	else
	  return 3;
      } else {
        if (entry->confirmTime >= entry->enableTime)
	  return 5;
	else
	  return 3;
      }
      break;
    case 12:	/* enable & disable */
      if (entry->disableTime <= entry->enableTime)      /* yes, who's first ?*/
        return 4;                                       /* disable first */
      else
        return 5;
      break;
    case 13:	/* negate & disable & enable */
      if (entry->enableTime >= entry->offTime) {
        if (entry->disableTime >= entry->offTime)
	  return 2;
	else
	  return 4;
      } else {
        if (entry->disableTime >= entry->enableTime)
	  return 5;
	else
	  return 4;
      }
      break;
    case 14:	/* confirm & disable & enable */
      if (entry->enableTime >= entry->confirmTime) {
        if (entry->disableTime >= entry->confirmTime)
	  return 3;
	else
	  return 4;
      } else {
        if (entry->disableTime >= entry->enableTime)
	  return 5;
	else
	  return 4;
      }
      break;
    case 15:	/* negate & confirm & disable & enable */
      if (entry->disableTime >= entry->offTime) {
        if (entry->confirmTime >= entry->offTime) {
	  if (entry->enableTime >= entry->offTime)
	    return 2;
	  else
	    return 5;
	} else {
	  if (entry->enableTime >= entry->confirmTime)
	    return 3;
	  else
	    return 5;
        }
      } else {
        if (entry->confirmTime >= entry->disableTime) {
	  if (entry->enableTime >= entry->disableTime)
	    return 4;
	  else
	    return 5;
	} else {
	  if (entry->enableTime >= entry->confirmTime)
	    return 3;
	  else
	    return 5;
	}
      }
      break;
  }
}
#endif
#if 0
/*
!   for one alarm entry, checks what alarm to send
!   0 - no alarm pending for this entry
!   1 - assert state pending
!   2 - negate state pending
!   3 - confirm state pending
!   4 - disable state pending     (new 910517)
*/
getAlarmSequence(entry, PCno)
struct _alarmEntry *entry;
int PCno;
{
  int pc;
  pc = (1 << PCno);
  if (DEBUG) {
    printf("getAlarmSequence: pc = %d\n", pc);
  }
#ifdef _V920914_1_40
  if ((entry->sendStatus & ALARM_SEND_ASSERT) && 
      (entry->sendMask & pc) && !(entry->assertSent & pc))
    return 1;
  else if ((entry->sendStatus & ALARM_SEND_NEGATE) &&
           (entry->sendMask & pc) && 
          !(entry->negateSent & pc)) {                /* check inactive */
    if ((entry->sendStatus & ALARM_SEND_CONFIRM) &&
        (entry->sendMask & pc) && 
          !(entry->confirmSent & pc)) {               /* and confirmed ? */
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
             (entry->sendMask & pc) && 
            !(entry->confirmSent & pc))               /* still active */
    return 3;
  else if ((entry->sendStatus & ALARM_SEND_DISABLE) &&
            (entry->sendMask & pc) && 
            !(entry->disableSent & pc))                /* new 910729 */
    return 4;
  else if ((entry->sendStatus & ALARM_SEND_ENABLE) &&
            (entry->sendMask & pc) && 
            !(entry->enableSent & pc))                /* new 910729 */
    return 5;
  else
    return 0;
#else
  if ((entry->sendAssert & pc) && !(entry->assertSent & pc))
    return 1;
  else if ((entry->sendNegate & pc) && 
          !(entry->negateSent & pc)) {                /* check inactive */
    if ((entry->sendConfirm & pc) && 
          !(entry->confirmSent & pc)) {               /* and confirmed ? */
      if (entry->confirmTime > entry->offTime)          /* yes, who's first ?*/
        return 2;                                       /* inactive first */
      else
        return 3;
    } else                                              /* none confirmed */
      return 2;
  } else if ((entry->sendConfirm & pc) && 
            !(entry->confirmSent & pc))               /* still active */
    return 3;
  else if ((entry->sendDisable & pc) && 
            !(entry->disableSent & pc))                /* new 910729 */
    return 4;
  else
    return 0;
#endif
}
#endif

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
  int PCvector;

  PCvector = (1 << PCno);
#ifdef _V920914_1_40
  if ((entry->sendStatus & ALARM_SEND_ASSERT) &&
      (entry->sendMask & PCvector) && !(entry->assertSent & PCvector))
    return 1;
  else if ((entry->sendStatus & ALARM_SEND_NEGATE) &&
           (entry->sendMask & PCvector) && 
          !(entry->negateSent & PCvector)) {                /* check inactive */
    if ((entry->sendStatus & ALARM_SEND_CONFIRM) &&
        (entry->sendMask & PCvector) &&
       !(entry->confirmSent & PCvector)) {                  /* and confirmed ? */
      if (entry->confirmTime > entry->offTime)          /* yes, who's first ?*/
        return 2;                                       /* inactive first */
      else
        return 3;
    } else                                              /* none confirmed */
      return 2;
  } else if ((entry->sendStatus & ALARM_SEND_CONFIRM) &&
             (entry->sendMask & PCvector) &&
            !(entry->confirmSent & PCvector))               /* still active */
    return 3;
  else if ((entry->sendStatus & ALARM_SEND_DISABLE) &&
           (entry->sendMask & PCvector) && 
          !(entry->disableSent & PCvector))                 /* new 910729 */
    return 4;
  else if ((entry->sendStatus & ALARM_SEND_ENABLE) &&
           (entry->sendMask & PCvector) && 
          !(entry->enableSent & PCvector))                  /* new 920921 */
    return 5;
  else
    return 0;
#else
  if ((entry->sendAssert & PCvector) && !(entry->assertSent & PCvector))
    return 1;
  else if ((entry->sendNegate & PCvector) && 
          !(entry->negateSent & PCvector)) {                /* check inactive */
    if ((entry->sendConfirm & PCvector) &&
       !(entry->confirmSent & PCvector)) {                  /* and confirmed ? */
      if (entry->confirmTime > entry->offTime)          /* yes, who's first ?*/
        return 2;                                       /* inactive first */
      else
        return 3;
    } else                                              /* none confirmed */
      return 2;
  } else if ((entry->sendConfirm & PCvector) &&
            !(entry->confirmSent & PCvector))               /* still active */
    return 3;
  else if ((entry->sendDisable & PCvector) && 
          !(entry->disableSent & PCvector))                 /* new 910729 */
    return 4;
  else
    return 0;
#endif
}


#endif
