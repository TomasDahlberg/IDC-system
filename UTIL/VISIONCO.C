/* visionCom.c  1994-08-30 TD,  version 1.97 */
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
 * Heimdalsgatan 4
 * 113 28 Stockholm
 * Sweden
 */
#define DEBUG_MODE
/*
! visionCom.c 
! Copyright (C) 1991-1994 IVT Electronic AB.
*/

/*
!   visioncom     - master program for communication to ivt vision
!
!   history:
!   date       by  rev  what
!   ---------- --- ---  ------------------------------------
!   1991-02-03 td  1.00 initial coding
!     91-02-06 td  1.01 revision change:
!                         - concatination ' & '  (3 chars) will be removed !
!                         - setvar indication of error will be reflected
!                           in the negation of the index no 
!                           (previously index zero)
!                         - added checksum
!     91-02-12 td  1.02 revision change: (request from goran)
!                         - no reply of ack when pc has sent ack
!                         - modulo 256 of checksum
!                         - checksum as last byte before crlf with msb set
!     91-02-20 td  1.03 the backup data module will NOT be created here, only
!                       linked to.
!     91-03-01 td  1.04 a first approach to statistics
!
!     91-03-04 td  1.05 change, setvar will NOT update dmBackup module
!
!     91-03-12 td  1.06 A first Approach to Modem Communication Support
!
!   
! history:        (dec)
! date   by  rev  rev,ed# what
! ------ --- ---  --- --- -------------------------------------------------
! 910325 td  1.07  0    1 bugfix, flushbuf only if any data in out buffer !
!
! 910422 td  1.08  0    2 changed input from stdin to first arg, removes ctrl-C!
!
! 910422 td  1.08  0    2 temporary removed master no from alarm text to PC
!
! 910507 td  1.09  0    2 code shaped to fit new alarm routines
!
! 910517 td  1.10  0    3 idle counter for method ii, HELLO PC sync twice
!                         automatic confirm when disable of an alarm
!
! 910530 td  1.11  0    4 map[id] = index   -->>   map[index] = id  !!!
!                         reboot and antal commands 
!
! 910617 td  1.12  0    5 intVec and floatVec handled by method1
!
! 910617 td  1.14  0    6 alarmNo -> alarmIndex in routine confirmAlarm
!
! 910905 td  1.15  0    7 a) visionCom replys with NAK if PC tries to 
!                            disable alarms which are already disabled.
!                         b) When trying to enable alarms which either
!                            do not exist or already are enabled,
!                            we will answer with ACK.
!                         c) globalAlarmMask_A  - globalAlarmMask_D added
!                         d) minimumWaitTime changed 300 -> 1 sec for 
!                            next call after succesfull communication
!                         e) confirmSent asserted when disableSent 
!                            asserted (910909)
! 910911 td  1.16  6    8 status bits sent on alarms are masked with 0x07
!                         that is, removed class information
!
! 911127 td  1.17  9   12 changed nodemap structure to match slave/server !
!
! 920107 td  1.20         enlarged buf from 128 to 256 in routine varMethodII
!
! 920114 td  1.21         removed stat routines to gain free-eprom-storage
!
! 920303 td  1.22         Sleep 2sec then HELLO PC
!
! 920306 td  1.23         Real BUG ! (any alarms in master only first entry !)
!
! 920309 td  1.24         Updated nodeMap structure
!
! 920319 td  1.50         Changed p1-p6 to static,
!                         Stat routines support two PC #0 and #1
!
! 920330 td  1.51         SET PCTIME added, bugfix for PC #0,#1 and #8 and #9
!                         Recompiled to match 32bits intervall
!
!					01
! 920427 td  1.60         getTelephone routine added. Uses vectors
!                         int telephonePC[8] and int telephonePC_area[8] to
!                         retrieve phone numbers for pc#, new
!                         syntax is; visionCom /port pc# pc# pc# ... pc#
!                         Added new_binding #ifdef in routine bind.
!                         assure binding to data modules after initial start.
!
! 920527 td  1.70         In the parseCommand function, an if-clause was 
!                         added 920527 to enable the following command;
!
!                         2 7 9Utetemp     -> translated to ->   9 7 Utetemp
!
! 920602 td  1.71         Updated and setvar ok !
!
! 920609 td  1.72         Added hard coded variables set-time, set-date and
!                         set-hour. Supports method1 fetching, method2 updating
!                         and setvar.
!
! 920623 td  1.73         Oh,no. Well, now it's fixed. The '2 7 9Utetemp' didn't
!                         work properly. Added an '+1' in the memcpy function
!                         in routine parseCommand() to cope with C-strings
!                         terminated '\0'.
!
! 920914 td  1.74         Bugfix-test. Added #ifdef TEST_920914
!                         
! 920917 td  1.80  21  16 Update in entire system, the globalAlarmMask with
!                         its sendXxxxx masks have changed to use just one
!                         mask, sendMask and using the bits in sendStatus
!                         instead. Activated by the '#define _V920914_1_40'
!                         Added masterNo (920921)
!                         Correct order in routine doRequestAlarmText. Prev. 
!                         version put all trust in confirmSent etc wheter
!                         to set status bits or not. Now we compare all 
!                         timestamps such as confirmTime > disableTime. (920923)
!                         Added hard coded vars 'alarm-###' to retrieve
!                         alarm status such as 0,1,2,3 for no alarm, active,
!                         active+conf and inactive not conf.    (920923)
!
! 921016 td  1.81         Stats; when duc calls pc, duc will exchange up to 
!                         80 stat-messages, then hangup. If more stats exists,
!                         another call will be issued soon, otherwise wait at
!                         least 3 hours for the next call.
!                           The '80' variable is 'noOfStats2Send' which can
!                             be set from PC with 'STAT 120' eg.
!                           The '3hour' variable is 'MIN_WAIT_FOR_STAT' 
!                             which can be set from PC with 'WAITSTAT 3600' eg.
!
! 921103 td  1.82 22  17  Stats; added checksum as last char in stat-message.
!                         PC acknowledge every stat with "SACK"-message
!                         This version is asserted by the 
!                                   #define NEW_STAT_921103
!
! 921112 td  1.83 23  18  If master# is set by neither of -m option nor 
!                         masterNo idc-var, a check for b9 of the dip-switch is
!                         done. If b9 is set, the masterNo is the lsb 8 bits of
!                         the switch.
!
! 921117 td  1.84 24  19  Added var itsAStatCall, set to 1 if we call pc for 
!                         stats, and resets when pc polls for alarms.
!                         If set, we will respond with stat-string upon 
!                         reception of SACK. Otherwise we will only send
!                         stat-strings at the '!!!'-poll.
!                         Removed the space preceding the checksum in the 
!                         stat-string.
!
! 921119 td  1.85 24  20  Added new stat protocoll
!                         The answer from '!!!' is either of
!                             * number of alarms
!                             * 0, no alarms
!                             * "STAT", please start to poll stats
!
!                         PC sends 'RECSTAT' and we reply with 'S#....'
!                         When we call pc, we always start with 'HELLO PC'
!                         and then pc takes control.
!
! 921218 td  1.86 25  21  Replaced the 'always skip in-queue'
!                         with 'use-queue-first-but-not-next-time'
!                         in function 'flushOutBuf(outLine)'
!
! 930223 td  1.87 25  22  Added mask (& 15) for PCnumbers, i.e.
!                         not only binary 0-7 is valid but also 32-39
!                         Added "SET PCTIME"-message after !!! with result 0
!                         Added message 'version'
!
! 930511 td  1.88 25  23  Added kommand 'close_pct' / 'CLOSE_PCT'
! 930512 td  1.89 25  23  BUGFIX in setstat, inserted ! (not)
! 930519 td  1.89 25  23  Added pcStatMask, (previous not released)
! 930524 td  1.89 25  23  Solved the close_pct-slave problem
!
! 930709 td  1.90 25  24  Only one method2 request from each slave
! 930722 td  1.90 25  24  (idcio/net.c fix) returns NET_TIMEOUT if no server
!                         -> negativ index and use value 0
!
! 930830 td  1.91 25  25  Changed timout in connected state (modem) from
!                         20 seconds to 120 seconds
!                         The idc variabel 'int modemTimeout = 60;' can be 
!                         used to alter from this default value of 120 seconds.
!
! 930906 td  1.92 25  26  Bugfix in alarm, we used to send confirm before 
!                         inactive if both shared the same timestamp.
!                         This occurs for C and D alarms. The fix is to switch
!                         this order. The result was that vision got stuck 
!                         with a 'green' alarm !
!
! 930907 td  1.93 25  26  Quick fix in sendstatistic. If meta stat has been
!                         changed, i.e. the specific entry has been deleted,
!                         we would end up sending stat data with var[] empty
!                         resulting in NAK from vision. Quick fix is to replace
!                         any empty variabel name with dummy variable '????'.
!                         This is just a quick fix to prevent entering the 
!                         hangman state which occured before when we kept 
!                         sending the same string over and over again.
!                         The true problem will be fixed later. The reason
!                         is that stat data keeps pointers/indices into meta
!                         data.
!
! 930920 td  1.94 25  27  getAlarmSequence has changed completely
! 930923 td  1.94 25  27  and moved to trap module idcutil
!
! 931112 td  1.95 25  28  int telephonePC_ctrl[8]; added
!			  the entries hold control values for each pc#
!			  b31-b24 b23-b16 b15-b8 b7-b0
!			  na	  na	  na	 !
!						 !
!			  b7-b0
!			  76543210
!			      !!!!
!			      !!++
!			      !! !
!			      !! +-----	00   no pbx
!			      !!	01   one '0' and wait
!			      !!	10   double '00' and wait
!			      !!	11   na
!			      !+-------- 1 = pulse, 0 = tone
!			      +--------- 1 = minicall, 0 = pc
!			phone number is hardcoded (!) to 020-910037
!
#define CTRL_PBX_1 	0x01
#define CTRL_PBX_2 	0x02
#define CTRL_PULSE 	0x04
#define CTRL_MINICALL 	0x08
!
!
! 940509 td  1.96 27  30  int sysStatCtrl; added. If b0 true change stat-name
!			  to <master#>.<slave#>Var e.g.
!			  GT58 ->	3.17GT58
!
!			  S#00 yy.mm.dd hh.mm.ss ???? <value> changed to
!			  S#?  yy.mm.dd hh.mm.ss ???? <value>
!
! date   by  rev  rev,ed# what
! 940830 td  1.97 27  31  Added compatibility with IVTnet GROUP. parseCommand
!			  now returns master number. '#7 !!!<pc>' is for m7.
!
*/
#define PROTOCOL_MAJOR    1
#define PROTOCOL_MINOR    0

/*
!
!--------------------------------------------------------------------
!   commands:     program reads commands from specified port
!
!   request of var, method 1
!   nodeno index name & nodenr index name & ....  crlf
!
!   request of var, method 2
!   &&& crlf
!
!   updating var
!   nodeno name = value & nodenr name = value & ... crlf
!
!   request of no of alarms
!   !!!
!
!   request of alarm text
!   ***
!
!   confirm alarm
!   nodeno alarm-number serial-number status crlf
!
!--------------------------------------------------------------------
!   the index range are to be 1..50
!
!   revision change:  - concatination ' & '  (3 chars) will be removed !
!                     - setvar indication of error will be reflected
!                       in the negation of the index no (previously index zero)
!   
!   any nak received from pc will force a retransmission of the previously
!   sent command line. an ack received from pc will, if the previous command
!   was '_get_alarm_text', mark the current alarm item as being sent and 
!   as of revision 1.02, no reply to pc. (revision 1.01 had the
!   reply from master to pc as the same as the line received from pc
!   containing the ack).
!   
!   the '_get_alarm_text' command has added a simple checksum at the end of
!   the sent command line. the checksum is simply the sum of each ascii
!   character found at the line, before the checksum. i.e. including white 
!   space etc.
!   if the '_get_alarm_text' command is requested even though no alarms exists,
!   a nak will be replied.
!
! Modem Strategi as of first implementation 91-03-12;
!
!   Four states are valid, disconnected, connected, establish, shutdown.
!   At start up, the communication is in state connected. If the read()
!   function timeout after 60 seconds, the command '+++' are beeing sent
!   to the modem and state shutdown is entered. When an 'OK' acknowledge are
!   then received from the modem, the command 'ATH' will be sent and state
!   disconnected will be entered. In the disconnected state two possible
!   things may occure;
!       - either the modem will be called and after the modem
!         answers, the reply will be 'CONNECT 2400' and this
!         will enforce the enter of state connected.
!
!       - or, after a timeout of 5 seconds, if there is any alarm pending
!         the modem will call an appropriate number and enter state establish.
!
!   If the seconds choise appears, after a valid connection has been
!   established, the line will only be up for a specific amount of
!   time, e.g. 1 minute. Then a full shutdown will be performed.
!
! Predefined Modem Setup:
!     
!   ate0&w          no echo
!   at&d0&w         do not use dtr
!   at&j1&w         selects rj-12/rj-13 telephone jack
!   ats0=2&w        answer at second ring
!
*/
@_sysedit: equ 31
@_sysattr: equ $801c

#define TWO_WHEN_BLOCKED   /* when disabling an alarm, send first confirm
                                             then blocked */

#define _V920914_1_40 /* globalAlarmMask system fix !! (see other modules !) */

#define TEST_920914           /* Bugfix-test */

#define NEW_STAT_921103
#define NEW_STAT_921119

#define INDEXVAR 1

#define oldRevision 0

#include <stdio.h>
#include <errno.h>
#include <sgstat.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <module.h>
#include "sysvars.h"
#define NO_OF_ALARMS 1
#include "alarm.h"
#include "ivtnet/v1.1/net.h"
#include "ivtnet/v1.1/stat.h"
#define NET_TIMEOUT 0x08

#define NET_NOSUCHALARM     0x0b

#include "phyio/dt08io.h"

struct _system *sysVars = SYSTEM_AREA;		/* 0x003ffe0;	*/

unsigned char abcdMask[4];
int useVars = 0;

int noOfStats2Send = 80;

static long statisticSentAt[8];
long MIN_WAIT_FOR_STAT =  10800;         /* 3 hours */
  
/* #define OUR_NODE 0			/* OBS !! see mini.c also */

int OUR_NODE = 0;

#define NAMEOFDATAMODULE  "vars"
#define NAMEOFDATAMODULE1 "var1"
#define NAMEOFDATAMODULE2 "var2"
#define COMMAND_SEPERATOR '&'
struct _alarmModule *aldm;
char *dm, *dmBackup;
char *meta;

int ctrl;			/* current ctrl-codes from telephonePC_ctrl */

void skipIndex();

static int  currentId = 1,          /* next id for get-var-method ii */
            currentNode;
    
static int currentCalendarIdx = 0;

/*
!   MAP_SIZE is max number from pc
*/
#define MAP_SIZE                  100     /* 1000 */
int idleCount[MAP_SIZE];            /* counter for each idle var in method ii */
#define IDLE_MAX_COUNT 20           /* idle for max no of polls */
static int map[MAP_SIZE];
static unsigned short mapNode[MAP_SIZE];	/* char -> short 941116 */

static char close_pct_atNode[256];      /* added 930525 */


int internalCounter;     /* next alarm text to send after request !!! */
int alarmMarkPtr[8];
int alarmMarkNode;

typedef enum { _value, _name, _token, _nothing } item;


int netGetIdxVar();

char *doGetCalendar();
char *doSetCalendar();
char *doRequestVarMethod1();
int doRequestVarMethod2();
int doRequestSetVar();
int doRequestNoOfAlarms();
int doRequestConfirmAlarm();
item getItem();

char *getNextUpdatedVar();
char skipToken();
char skipWhiteSpace();

#define ACK    "ACK"
#define NAK    "NAK"

#define TYPE_INT_VEC    4
#define TYPE_FLOAT_VEC  5
#define TYPE_CALENDAR 15
#define TYPE_INT    7
#define TYPE_FLOAT  8

#define BAD_ASSIGNMENT  "ERROR 3"  /* bad assignment character, expecting '=' */

typedef enum { _idle, _get_alarm_text } command;

int method2running;

FILE *fpIn, *fpOut;
int pathIn, pathOut;
int originalSpeed = 0;

#define NO_OF_CAL_ENTRIES 10
struct _calendar
{
  unsigned short day[NO_OF_CAL_ENTRIES];
  unsigned short stopday[NO_OF_CAL_ENTRIES];
  unsigned char color[NO_OF_CAL_ENTRIES];
  unsigned short start[NO_OF_CAL_ENTRIES];
  unsigned short stop[NO_OF_CAL_ENTRIES];
};

/* 
#s0 30 3:kalle

*/
/*
!   New stat as of 920305
*/
#if 1
static struct _metaStatS *statS = METASTAT_ADDRESS;
static struct _statBuff *statPtr = SAVE_STAT_ADDRESS;
short int *saveIdx = SAVE_IDX_ADDRESS;
/* short int *readIdx = READ_IDX_ADDRESS; */
short int *readIdx_PC0 = READ_IDX_ADDRESS_PC0;
short int *readIdx_PC1 = READ_IDX_ADDRESS_PC1;
#else
struct _statBuff {
  short intervall;
  char buff, duc;
  char var[32];
/*  short int chs;    */
};

struct _statS {
  char name[32];
  struct _statBuff rows[10];
} *statS = (struct _statS *) 0x3e000;
#endif
/*  
    Old stat
*/

int currentScanStat = 0, nextFreeStat = 0, currentStatId = 0;

struct _statBlock {
  short id[51];
  time_t lastSample;
  long intervall;
} statBuf[51];

int DEBUG = 0;

int PCno = -1;  /* for stat ? , 0; */    /* 0 - 7 */

int noOfStatBuf = sizeof(statBuf) / sizeof(statBuf[0]);

struct {
  char *alias;
  char *real;
} alias[] = {
    { "loppa", "Loop" }
/*
    { "pid1_RGut", "sv1_ta1" },
    { "pid1_RGin", "b91_ta1_reg" },
    { "pid1_RGbv", "stop_ta1" },
    { "pid1_RGmax", "pid1_RGmax" },
    { "pid1_RGmin", "pid1_RGmin" },
    { "pid1_RGpf", "ta1_s_p" },
    { "pid1_RGig", "ta1_s_i" },
    { "pid1_RGde", "pid1_RGde" },
    { "b91_AImv",  "b91_ta1" },
    { "b91_AIhl",  "b91_AIhl" },
    { "b91_AInl",  "frysgrans" },
    { "b91_AIls",  "fryslarm_ta1_m" },
    { "b91_AIlp",  "b91_AIlp" }
*/

};

int noOfAliases = sizeof(alias) / sizeof(alias[0]);

int timeoutFlag;

typedef enum { connected, disconnected, shutdown, establish } modType;
modType mode;

#define TEST_MANY_PC
#ifdef TEST_MANY_PC            
char *telephoneNumber;
int maxPcs = 0;           /* antal pc's som vi ska ringa */
int mapPcs[8];        /* map till PC# */
#else
char telephoneNumber[50];
#endif
int telephonePC = 0;

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

#define MAX_NODE4MASTER 100	/*	0-99 is valid numbers, 102 is master 1, slave 2	*/

int getAlarmsForAllNodes(mask, pc)
int mask, pc;
{
  int i, cnt;
  cnt = 0;
  
/*  mask |= (pc << 8);  */

  if (DEBUG) printf("getAlForAllNod: pc=%d,mask=%d\n", pc, mask);
  for (i = 1; i < MAX_NODE_NO; i++) {
    if ((mask & nodeMap[i].noOfAlarms) &&   /* changed 920203 */
        ((pc << 8) & nodeMap[i].noOfAlarms))
    {
      if (DEBUG) printf("nod%d:mask(%d)&noOfAl(%d)\n", i, mask, nodeMap[i].noOfAlarms);
      
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

  if (DEBUG) printf("getNextAlarmNode:pc=%d\n", pc);
  for (i = 1; i < MAX_NODE_NO; i++)
    if ((mask & nodeMap[i].noOfAlarms) &&     /* changed 920203,  > 0 -> & */
        ((pc << 8) & nodeMap[i].noOfAlarms))
      break;
  if (DEBUG) printf("Next nod%d(mask=%d,noOfAlarms=%d)\n", i,
                    mask, nodeMap[i].noOfAlarms);
  return (i < MAX_NODE_NO) ? i : 0;
}


checkTimeout(sigcode)
int sigcode;
{
  if (sigcode == 3) {
/*    write(1, "Use EXIT to terminate program\n", 30); */
  } else if (sigcode == 2) {
    timeoutFlag = 1;
  }
}

checkAlias(name)
char *name;
{
  int i;
  for (i = 0; i < noOfAliases; i++) {
    if (!strcmp(alias[i].alias, name)) {
      strcpy(name, alias[i].real);
      break;
    }    
  }
  if (i >= noOfAliases)
    return 0;
  else
    return 1;
}


int atoin(s, n)
char *s;
int n;
{
  char buf[25];
  if (n > 24)
    n = 24;
  strncpy(buf, s, n);
  buf[n] = 0;
  return atoi(buf);
}


/* #define STATISTICS   */

#ifndef STATISTICS
initStatBuf()
{
  int i, row;
  
  for (i = 0; i < 8; i++) {
    statS[i].name[0] = 0;
    for (row = 0; row < 10; row++) {
      statS[i].rows[row].buff = 0;
      statS[i].rows[row].var[0] = 0;
    }
  }
}
int setStatistics() {}
/*
static struct _statBuff *statPtr = SAVE_STAT_ADDRESS;
short int *saveIdx = SAVE_IDX_ADDRESS;
*/

/* int pcStatMask(dm, meta, pcNo)  */
int conditionalAnyStatistics(pcNo)      /* added 930519 */
int pcNo;
{
  int x;
  struct tm tid;
  time_t t1;

  t1 = time(0);
  memcpy(&tid, localtime(&t1), sizeof(struct tm));

  x = pcStatMask(dm, meta, pcNo);
  if (x & (1 << tid.tm_hour))
    return anyStatistics(pcNo);
  else if (anyStatistics(pcNo) > 2867)     /* 70% of buffer occupied */
    return anyStatistics(pcNo);            /* so empty ! */
  return 0;
}

int anyStatistics(pc)
int pc;
{
  short int *myReadIdx;

  pc &= 7;

#ifndef NEW_STAT_921119
  if (abs(getRelTime(0) - statisticSentAt[pc]) < MIN_WAIT_FOR_STAT) {
    return 0;   /* no stat ! */
  }
#endif

  if (pc == 0)
    myReadIdx = readIdx_PC0;
  else if (pc == 1)
    myReadIdx = readIdx_PC1;
  else
    return 0;
  if (*myReadIdx == *saveIdx)
    return 0;
  if (*myReadIdx < 0)
    return 0;
  return (*saveIdx - *myReadIdx) + ((*saveIdx > *myReadIdx) ? 0 : 0x1000);
}

char *sprintfn(x, n)
double x;
int n;
{
  int pos;
  char fmt[20], buff[20], *bf, *str;
  static char strStart[20];

  str = strStart;
  pos = n;
  if (x < 0) {
    *str++ = '-';
    x = -x;
    pos --;
  }
  if (log10(x) < -2 || log10(x) >= pos) {   /* exponent form */
    double exp;
    exp = log10(x);
    if (exp < 0)
      exp --;
    exp = (int) exp;
    x = x / (pow(10.0,exp));
    pos -= 3;
    if (exp > 9)
      pos --;
    if (exp < -9)
      pos --;
    sprintf(fmt, "%%%dg", pos);
    sprintf(buff, fmt, x);
    buff[pos] = 0;
    bf = buff;
    while (*str++ = *bf++)
      ;
    str --;
    if (exp < 0) {
      exp = -exp;
      sprintf(str, "e-%d", (int) exp);
    } else
      sprintf(str, "e+%d", (int) exp);
  } else {
    sprintf(fmt, "%%%d.%dg", n, pos+1);
    sprintf(str, fmt, x);
  }
  return strStart;
}

static long whenSETPCTIME_wasSent;

checkSend_SETPCTIME()
{
  if ((whenSETPCTIME_wasSent == 0) || 
          (abs(getRelTime(0) - whenSETPCTIME_wasSent) > 1800 /* 600 */ )) {
    struct tm tid;
    time_t t1;

    t1 = time(0);
    memcpy(&tid, localtime(&t1), sizeof(struct tm));
    
    if ((tid.tm_hour != 23 || tid.tm_min < 30) &&
        (tid.tm_hour != 0  || tid.tm_min > 30))
    {
      fprintf(fpOut, "SET PCTIME \"%02d:%02d:%02d\"\015",
              tid.tm_hour, tid.tm_min, tid.tm_sec);
/*
      fprintf(fpOut, "SET PCDATE %02d.%02d.%02d\015",
              tid.tm_mday, tid.tm_mon + 1, tid.tm_year);
*/
    }
    whenSETPCTIME_wasSent = getRelTime(0);
  }
}

int *sysStatCtrlPtr = 0;
#define SYSSTATCTRL_MSLAVE	0x01

int initDivSysVars(dm, meta)     /* added 920921 */
char *dm, *meta;
{
  int id;
  char name[20];

  sysStatCtrlPtr = 0;
  strcpy(name, "sysStatCtrl");
  if ((id = metaId(meta, name)) < 0) /* name not found */
    ;
  else if (metaType(meta, id) == TYPE_INT)
    sysStatCtrlPtr = ((int *) metaValue(dm, meta, id));
}

int MASTER_NO = 0;

int sendStatistics(pc)
int pc;
{
  struct tm tid;
  long b, t, s, i;
  float v;
  short int *myReadIdx;
  char buf[120], smallBuf[10];
  
  pc &= 7;
  if (pc == 0)
    myReadIdx = readIdx_PC0;
  else if (pc == 1)
    myReadIdx = readIdx_PC1;
  else
    return 0;

  if (*myReadIdx < 0)
    return 0;

  *myReadIdx &= 0x0fff;
  if (*myReadIdx == *saveIdx)
    return 0;
  i = *myReadIdx;
  b = statPtr[i].bufItm;
  t = statPtr[i].Itm;
  s = statPtr[i].sampleTime;
  v = statPtr[i].value;
  memcpy(&tid, localtime(&s), sizeof(struct tm));
  if (v > 999999) {
    if (*sysStatCtrlPtr & SYSSTATCTRL_MSLAVE) {
      sprintf(buf, "S#%d %02d%02d%02d %02d:%02d:%02d %d.%d%s %s\0",
        statS[b].rows[t].buff,
              tid.tm_year, tid.tm_mon + 1, tid.tm_mday, 
              tid.tm_hour, tid.tm_min, tid.tm_sec,
	      MASTER_NO, statS[b].rows[t].duc, 
              statS[b].rows[t].var[0] ? statS[b].rows[t].var : "????", 
/*              statS[b].rows[t].var,   changed 930907 */
              sprintfn(v, 8));
    } else {
      sprintf(buf, "S#%d %02d%02d%02d %02d:%02d:%02d %s %s\0",
        statS[b].rows[t].buff, 
              tid.tm_year, tid.tm_mon + 1, tid.tm_mday, 
              tid.tm_hour, tid.tm_min, tid.tm_sec,
              statS[b].rows[t].var[0] ? statS[b].rows[t].var : "????", 
/*              statS[b].rows[t].var,   changed 930907 */
              sprintfn(v, 8));
    }
    if (!statS[b].rows[t].var[0]) {
	buf[2] = '?';
	buf[3] = ' ';
    }
#ifdef NEW_STAT_921103
    sprintf(smallBuf, "%c\0", calcCheckSum(buf) | 0x80);
    strcat(buf, smallBuf);
#endif
    fprintf(fpOut, "%s\n", buf);
  } else {
    if (*sysStatCtrlPtr & SYSSTATCTRL_MSLAVE) {
      sprintf(buf, "S#%d %02d%02d%02d %02d:%02d:%02d %d.%d%s %g\0",
        statS[b].rows[t].buff, 
              tid.tm_year, tid.tm_mon + 1, tid.tm_mday, 
              tid.tm_hour, tid.tm_min, tid.tm_sec,
	      MASTER_NO, statS[b].rows[t].duc, 
              statS[b].rows[t].var[0] ? statS[b].rows[t].var : "????", 
/*              statS[b].rows[t].var,   changed 930907 */
              v);
    } else {
      sprintf(buf, "S#%d %02d%02d%02d %02d:%02d:%02d %s %g\0",
        statS[b].rows[t].buff, 
              tid.tm_year, tid.tm_mon + 1, tid.tm_mday, 
              tid.tm_hour, tid.tm_min, tid.tm_sec,
              statS[b].rows[t].var[0] ? statS[b].rows[t].var : "????", 
/*              statS[b].rows[t].var,   changed 930907 */
              v);
    }
    if (!statS[b].rows[t].var[0]) {
	buf[2] = '?';
	buf[3] = ' ';
    }
#ifdef NEW_STAT_921103
    sprintf(smallBuf, "%c\0", calcCheckSum(buf) | 0x80);
    strcat(buf, smallBuf);
#endif
    fprintf(fpOut, "%s\n", buf);
  }
#ifdef NEW_STAT_921103
/*  ackStatistics(pc);  */
#else
  (*myReadIdx)++;
  if (*myReadIdx >= 0x1000)
    *myReadIdx = 0;
  return 1;
#endif
}

int ackStatistics(pc)
int pc;
{
  short int *myReadIdx;
  
  pc &= 7;
  if (pc == 0)
    myReadIdx = readIdx_PC0;
  else if (pc == 1)
    myReadIdx = readIdx_PC1;
  else
    return 0;
  if (*myReadIdx < 0)
    return 0;

  *myReadIdx &= 0x0fff;

  if (*myReadIdx == *saveIdx)
    return 0;
    
   
  (*myReadIdx)++;
  if (*myReadIdx >= 0x1000)
    *myReadIdx = 0;
  return 1;
}
#else
initStatBuf()
{
  int i, j;
  for (i = 0; i < noOfStatBuf; i++) {
    statBuf[i].intervall = statBuf[i].lastSample = 0;
    for (j = 0; j < 51; j++)
      statBuf[i].id[j] = 0;
  }
}

int setStatistics(buffer, intervall, name)
int buffer;
int intervall;
char *name;
{
  int j, id;
  
  if (*name == '\0') {              /* obs! funkar ej !! */
    statBuf[buffer].intervall = 0;
    for (j = 0; j < 51; j++)
      statBuf[buffer].id[j] = 0;
    return 1;
  }
  if ((id = metaId(meta, name)) <= 0) {
    fprintf(fpOut, "No such variable\n");
    return 0;
  }
  
  if (intervall == 0) {     /* ok, clear it */
    for (j = 0; j < 51; j++) {
      if (statBuf[buffer].id[j] == id)
        break;
    }
    if (j >= 51) {
      fprintf(fpOut, "cannot clear that id, not found\n");
      return 0;
    }
    statBuf[buffer].id[j] = 0;
    for ( ; j < 50; j++) {
      statBuf[buffer].id[j] = statBuf[buffer].id[j + 1];
    }
    statBuf[buffer].id[j] = 0;
    return 1;
  } else {
    for (j = 0; j < 51; j++) {
      if (statBuf[buffer].id[j] == 0) {
        break;
      }
    }
    if (j >= 51) {
      fprintf(fpOut, "No free storage in that buffer\n");
      return 0;
    }
  }
  statBuf[buffer].id[j] = id;
  statBuf[buffer].intervall = intervall;
  return 1;
}

int sendStatistics()
{
  long intervall;
  
  if (currentScanStat >= noOfStatBuf)
    currentScanStat = 0;
  while ((currentScanStat < noOfStatBuf) && 
         (((intervall = statBuf[currentScanStat].intervall) == 0) ||
         ((time(0) - statBuf[currentScanStat].lastSample) < intervall)))
    currentScanStat ++;
    
  if (currentScanStat < noOfStatBuf) {
    time_t now;
    struct tm tid;
    char *name, *vPtr;
    double fvalue;
    int id;
    
    if (statBuf[currentScanStat].id[currentStatId] == 0) {
      currentScanStat ++;
      currentStatId = 0;
      return 0;
    }

    name = (char *) metaName(meta, id = statBuf[currentScanStat].id[currentStatId]);
    vPtr = (char *) metaValue(dm, meta, id);
    if (metaType(meta, id) == TYPE_INT) {
      fvalue = (double) (*((int *) vPtr));
    } else if (metaType(meta, id) == TYPE_FLOAT) {
      fvalue = (double) (*((double *) vPtr));
    } else
      fvalue = 0;
      
    now = time(0);
    memcpy(&tid, localtime(&now), sizeof(struct tm));
    fprintf(fpOut, "S#%d %02d%02d%02d %02d:%02d:%02d %s %g\n",
        currentScanStat, 
              tid.tm_year, tid.tm_mon + 1, tid.tm_mday, 
              tid.tm_hour, tid.tm_min, tid.tm_sec,
              name, fvalue);
              
    currentStatId ++;
    if ((currentStatId > 51) || 
        (statBuf[currentScanStat].id[currentStatId] == 0)) {
      statBuf[currentScanStat].lastSample = time(0);
      currentScanStat ++;
      currentStatId = 0;
    }
  }
}
#endif

disableXonXoff(path)
int path;
{
  struct sgbuf buffer;
  
  if (_gs_opt(path, &buffer) == -1) {
    fprintf(stderr, "errGs_opt: %d\n", errno);
    exit(errno);
  }
  buffer._sgm._sgs._sgs_xon   = 0;
  buffer._sgm._sgs._sgs_xoff  = 0;
  buffer._sgm._sgs._sgs_kbich = 0;      /* keyboard interrupt character */
  buffer._sgm._sgs._sgs_kbach = 0;      /* keyboard abort character */

  buffer._sgm._sgs._sgs_echo= 0;
  buffer._sgm._sgs._sgs_pause=0;
  buffer._sgm._sgs._sgs_bspch=0;
  buffer._sgm._sgs._sgs_dlnch=0;
  buffer._sgm._sgs._sgs_rlnch=0;
  buffer._sgm._sgs._sgs_dulnch=0;
  buffer._sgm._sgs._sgs_kbich=0;
  buffer._sgm._sgs._sgs_bsech=0;
  buffer._sgm._sgs._sgs_xon=0;
  buffer._sgm._sgs._sgs_xoff=0;
  buffer._sgm._sgs._sgs_tabcr=0;
  buffer._sgm._sgs._sgs_delete= 0;
/*  buffer._sgm._sgs._sgs_eorch=13;	*/
/*  buffer._sgm._sgs._sgs_eofch=0;	*/
  buffer._sgm._sgs._sgs_psch=0;
  buffer._sgm._sgs._sgs_kbach=0;
  buffer._sgm._sgs._sgs_bellch=0;
  
  if (_ss_opt(path, &buffer) == -1) {
    fprintf(stderr, "errSs_opt: %d\n", errno);
    exit(errno);
  }
}

switch8_7_no_even(path, sw)	/* 1-> even/7, 0-> none/8 */
int path, sw;
{
  struct sgbuf buffer;
  
  if (_gs_opt(path, &buffer) == -1) {
    fprintf(stderr, "errGs_opt: %d\n", errno);
    exit(errno);
  }
  if (sw == 2) {
    buffer._sgm._sgs._sgs_parity = 0 | 4 | 128 |  /* no parity, 7 bits/char */
    		(buffer._sgm._sgs._sgs_parity & 0x70);
  } else if (sw == 1) {
    buffer._sgm._sgs._sgs_parity = 3 | 4 | 128 |  /* no check, even parity, 7 bits/char */
    		(buffer._sgm._sgs._sgs_parity & 0x70);
  } else {
    buffer._sgm._sgs._sgs_parity = 0 | 0 |  	/* no parity, 8 bits/char */
    		(buffer._sgm._sgs._sgs_parity & 0x70);
  }
  
  if (_ss_opt(path, &buffer) == -1) {
    fprintf(stderr, "errSs_opt: %d\n", errno);
    exit(errno);
  }
}

getSpeed(path)
int path;
{
  struct sgbuf buffer;
  
  if (_gs_opt(path, &buffer) == -1) {
    fprintf(stderr, "error during _gs_opt: %d\n", errno);
    exit(errno);
  }
  if (buffer._sgm._sgs._sgs_baud == 0x07)
    return 1200;
  else if (buffer._sgm._sgs._sgs_baud == 0x0a)
    return 2400;
  else
    return 0;
}

setSpeed(path, speed)
int path, speed;
{
  struct sgbuf buffer;
  
  if (_gs_opt(path, &buffer) == -1) {
    fprintf(stderr, "error during _gs_opt: %d\n", errno);
    exit(errno);
  }
  if (speed == 1200)
    buffer._sgm._sgs._sgs_baud   = 0x07;
  else if (speed == 2400)
    buffer._sgm._sgs._sgs_baud   = 0x0a;
  if (_ss_opt(path, &buffer) == -1) {
    fprintf(stderr, "error during _ss_opt: %d\n", errno);
    exit(errno);
  }
}

int makeAlarmMask(dm, meta, mask)
char *dm, *meta, *mask;
{
  int i;
  for (i = 0; i < 4; i++)
    mask[i] = receiveAlarmMask(dm, meta, i);
}

/*
! returns 1, 0.00 - 0.59, if idc-var pcStatMask doesn't exist
!  otherwise contents of pcStatMask[pcNo] if vector
!     or
!  if not vector the pcStatMask-var
*/
int pcStatMask(dm, meta, pcNo)
char *dm, *meta;
int pcNo;    /* 0-7 */
{
  int id;
  char name[20];

  pcNo &= 7;
  strcpy(name, "pCsTaTmAsK");
  if ((id = metaId(meta, name)) < 0) /* name not found */
#if 1   /* new */
    return 0x00000001;        /* bit 0 true, send between 0.00 - 0.59 */
#else   /* old */
    return 0x00ffffff;        /* bit 0 - bit 23 true */
#endif
  if (metaType(meta, id) == TYPE_INT)
    return *((int *) metaValue(dm, meta, id));
  else if (metaType(meta, id) == TYPE_FLOAT)
    return (int) (*((double *) metaValue(dm, meta, id)));
  else if (metaType(meta, id) == TYPE_INT_VEC)
    return *((int *) metaValue(dm, meta, id) + sizeof(long)*pcNo);
  else if (metaType(meta, id) == TYPE_FLOAT_VEC)
    return (int) (*((double *) metaValue(dm, meta, id) + sizeof(double)*pcNo));

#if 1   /* new */
  return 0x00000001;        /* bit 0 true, send between 0.00 - 0.59 */
#else   /* old */
  return 0x00ffffff;        /* bit 0 - bit 23 true */
#endif
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

checkHardCoded(name)                      /* added 920609 */
char *name;
{
  if (!strcmp("set-time", name))
    return -10;
  else if (!strcmp("set-date", name))
    return -11;
  else if (!strcmp("set-year", name))
    return -12;
  else if (!strcmp("set-month", name))
    return -13;
  else if (!strcmp("set-day", name))
    return -14;
  else if (!strcmp("set-hour", name))
    return -15;
  else if (!strcmp("set-min", name))
    return -16;
  else if (!strcmp("set-sec", name))
    return -17;
  else if (!strncmp("alarm-", name, 6))     /* added 920923 */
    return -100 - atoi(&name[6]);         /* alarm-17 ==>> -117 */
  return 0;
}

int setHardCoded(id, value)                    /* added 920609 */
int id;
double value;
{
  struct tm tid;
  time_t t1;
  long tt;
  
  tt = value;
  if (id >= -1)
    return 0;       /* not ours */
  t1 = time(0);
  memcpy(&tid, localtime(&t1), sizeof(struct tm));
  switch (id) {
    case -10:                 /* set-time */
      tid.tm_sec = tt % 100;
      tt /= 100;
      tid.tm_min = tt % 100;
      tt /= 100;
      tid.tm_hour = tt;
      break;
    case -11:                 /* set-date */
      tid.tm_mday = tt % 100;
      tt /= 100;
      tid.tm_mon = (tt % 100) - 1;
      tt /= 100;
      tid.tm_year = tt;
      break;
    case -12:                 /* set-year */
      tid.tm_year = tt;
      break;
    case -13:                 /* set-month */
      tid.tm_mon = tt - 1;
      break;
    case -14:                 /* set-day */
      tid.tm_mday = tt;
      break;
    case -15:                 /* set-hour */
      tid.tm_hour = tt;
      break;
    case -16:                 /* set-min */
      tid.tm_min = tt;
      break;
    case -17:                 /* set-sec */
      tid.tm_sec = tt;
      break;
    default:
      return 0;     /* not ours */
  }
  setTime(tid.tm_year, tid.tm_mon + 1, tid.tm_mday, 
                          tid.tm_hour, tid.tm_min, tid.tm_sec);
  return 1;   /* ok done ! */
}

int getHardCoded(id, buf)                    /* added 920609 */
int id;
char *buf;
{
  struct tm tid;
  time_t t1;

  if (id >= -1)
    return 0;       /* not ours */
  t1 = time(0);
  memcpy(&tid, localtime(&t1), sizeof(struct tm));
  switch (id) {
    case -10:                 /* set-time */
      sprintf(buf, "%02d%02d%02d\0", tid.tm_hour, tid.tm_min, tid.tm_sec);
      break;
    case -11:                 /* set-date */
      sprintf(buf, "%02d%02d%02d\0", tid.tm_year, tid.tm_mon + 1, tid.tm_mday);
      break;
    case -12:                 /* set-year */
      sprintf(buf, "%02d\0", tid.tm_year);
      break;
    case -13:                 /* set-month */
      sprintf(buf, "%02d\0", tid.tm_mon + 1);
      break;
    case -14:                 /* set-day */
      sprintf(buf, "%02d\0", tid.tm_mday);
      break;
    case -15:                 /* set-hour */
      sprintf(buf, "%02d\0", tid.tm_hour);
      break;
    case -16:                 /* set-min */
      sprintf(buf, "%02d\0", tid.tm_min);
      break;
    case -17:                 /* set-sec */
      sprintf(buf, "%02d\0", tid.tm_sec);
      break;
    default:
      if (id <= -100 && id > -200) {            /* added 920923 */
        sprintf(buf, "%d\0", alarmStatus(-id - 100));
        return 1;
      }
      return 0;     /* not ours */
  }
  return 1;   /* ok done ! */
}

/*
    0     No alarm exists, normal condition, inactive, confirmed, no alarm
    1     inactive, not confirmed,    GREEN
    2     active, confirmed           YELLOW
    3     active, not confirmed       RED
*/
static int alarmStatus(no)
int no;
{
  struct _alarmEntry *alarmList;
  int i, max, sts;
  char *headerPtr1;
  
  if (!aldm)
    return 0;
  sts = 0;
  alarmList = &aldm->alarmList[0];
  max = aldm->alarmListPtr;
  for (i = 0; i < max; i++, alarmList++) {
    if (alarmList->alarmNo == no) {
      if (alarmList->active)
        sts |= 2;
      if (!alarmList->confirm)
        sts |= 1;
    }
  }
  return sts;
}

#if 0                /* replaced by above function 930222 */
int alarmStatus(pkt)
int pkt;
{
  struct _alarmModule2 *aldm2;
  struct _alarmPt *entry;
  int idx;
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
  if (entry->active) {      /* a point confirmed ???? */
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
#endif

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

time_t initNextSample(inter)      /* added 920923 */
long inter;
{
  struct tm t1;
  time_t c_time, this;
/*
!   Every hour; sample next 'xx:58:00'
!   Every day;  sample next '23:58:00'
!   Every week; sample next 'Sun 23:58:00'
!   Every month;sample next '31/30/29/28 23:58:00'
*/
  if (inter < 3600)
    return 0;

  c_time = time(0);  
  memcpy(&t1, localtime(&c_time), sizeof(struct tm));
  if (inter >= 86400)     /* ??? */
    t1.tm_hour = 23;                    /* 0 */
  t1.tm_min = 58;                       /* 0 */
  t1.tm_sec = 0;
  if (inter == 2678400) {
    if (t1.tm_mon == 1)       /* feb */
      t1.tm_mday = 28 + (t1.tm_year == 100 /* year 2000 leap or not (?) */ ? 1 :
           ((t1.tm_year % 4) == 0 ? 1 : 0));
    else 
      t1.tm_mday = 31 - ((t1.tm_mon < 7) ? 
                         (t1.tm_mon & 1) :               /* 1 if odd */
                         (1 - (t1.tm_mon & 1)));         /* 1 if even */
  }
  this = mktime(&t1);
/*    t1.tm_wday  0 == sun, 1 = mon, 6 = sat */
  if (inter == 604800)
    this += 86400 * (t1.tm_wday ? (7 - t1.tm_wday) : 0);
  return this - inter;
  
/*  return now - (c_time - this); */
}

int itsAStatCall = 0;      /* added 921117 */


int main(argc, argv)
int argc;
char *argv[];
{
  int choise, id, size;
  char varName[31], commandLine[255], value[80], *commandPtr;
  char *headerPtr1, *headerPtr2, *headerPtr3, *headerPtr4, *_new, *_dest;

  static char p1[256], p2[256], p3[256], p4[256], p5[256], p6[256], *p;

  item t1, t2, t3, t4;
  char *outLinePtr, outLine[512], outBuf[256 /* 128 */],
                 previousLine[512], port[30], inPort[30], outPort[30];
  command previousCommand, currentCommand;

/*  fprintf(stderr, "%d aliases defined\n", noOfAliases);
*/
 
/*
    visionCom n2 7678265
*/

  if (argc >= 2)
  {
    strcpy(port, argv[1]);
    
    if (argc >= 3) {
      if (*argv[2] == 'd') {
        DEBUG = 1;
#ifdef TEST_MANY_PC            
        maxPcs = 0;
#else
        telephoneNumber[0] = 0;
#endif
      } else if (*argv[2] == 'm') {
        MASTER_NO = atoi(&argv[2][1]);
      } else {
#ifdef TEST_MANY_PC            
        argc -= 2;
        maxPcs = 0;
        if (argc > 8) {
          fprintf(stderr, "usage: visionCom port [pc# [pc# [pc# [pc# ...]]]]\n");
          exit(1);
        }
        while (argc-- > 0) {
          mapPcs[maxPcs++] = atoi(argv[2]);
          argv++;
        }
#else
        strcpy(telephoneNumber, argv[2]);
        telephonePC = atoi(argv[3]);
#endif
      }
    }
    else
      telephoneNumber[0] = 0;
  } else {
    fprintf(stderr, "usage: visionCom port [pc# [pc# [pc# [pc# ...]]]]\n");
    exit(1);
  }
  
  if (port[2] == '2')
    useVars = 0;
  else 
    useVars = 1;
  
  if (!strncmp(port, "/pipe", 5)) {
	strcat(strcpy(inPort, port), "/visIn");
	strcat(strcpy(outPort, port), "/visOut");
  } else {
	strcpy(inPort, port);
	strcpy(outPort, port);
  }

  if ((fpOut = fopen(outPort, "w")) == NULL) {
    fprintf(stderr, "cannot open '%s' for write access\n", outPort);
    exit(0);
  }
  if ((fpIn = fopen(inPort, "r")) == NULL) {
    fprintf(stderr, "cannot open '%s' for read access\n", inPort);
    exit(0);
  }
  
  pathIn  = fpIn->_fd;
  pathOut = fpOut->_fd;

  if (strncmp(port, "/pipe", 5)) {
	disableXonXoff(pathIn);
	disableXonXoff(pathOut);
  }

/*  originalSpeed = getSpeed(pathIn);   */

 
  fprintf(fpOut, "HELLO PC\n\015");

/*  initStatBuf();    */
  initidcio();
  initidcutil();    /* added 930923 */
  bind(&dm, &aldm, &meta, &dmBackup,
                    &headerPtr1, &headerPtr2, &headerPtr3, &headerPtr4);
  if (!(dm && aldm && meta && dmBackup)) {
    while(1) system("");
    exit(0);
  }
/*
!   create a new/or link to existing  dmBackup module. (for method II requests)
*/

/*
  size = ((struct modhcom *) headerPtr1)->_msize;
  _dest = 0;
  dmBackup = (char *) createDataModule("VAR2", size, 
                                             &_new, &headerPtr4, &_dest);
  if (!dmBackup)
  {
    fprintf(fpOut, "cannot create module 'VAR2'\n");
    fprintf(fpOut, "size = %d\n", size);
    fprintf(fpOut, "vars headerptr = %d\n", headerPtr4);
    exit(0);
  }
*/

  if (MASTER_NO == 0)
    MASTER_NO = getMasterNo(dm, meta);      /* added 920921 */
    
  if (MASTER_NO == 0) {                     /* added 921112 */
    static unsigned short *node = 0x402;
    if (*node & 512) {
      MASTER_NO = *node & 255;        /* do not use bits 8 & 9 */
    }
  }


  {
    static unsigned short *node = 0x402;
    if (*node & 512)			/* added 941116 */
	OUR_NODE = 0;
    else
    	OUR_NODE = *node & 255;        /* do not use bits 8 & 9 */
  }

  initDivSysVars(dm, meta);

/* set up intercept handler for handling timeout and CTRL-C */

  intercept(checkTimeout);

/*
!   main loop, reads commands from specified port
*/  
  commandPtr = 0;
  outLine[0] = '\0';
  method2running = 0;
  currentCommand = _idle;
  while (1) {
    makeAlarmMask(dm, meta, abcdMask);
    setAlarmMask(abcdMask);
    {
      static char prev;
      if (prev)
        *qsop2 = HBLED;
      else
        *qrop2 = HBLED;
      prev ^= 1;
    }
    previousCommand = currentCommand;
/*    currentCommand = _idle;		*/
#ifdef DEBUG_MODE
    if (DEBUG) printf("'%s'\n", commandPtr);
#endif    
    if (!skipUntilNextCommand(&commandPtr)) {
      if (DEBUG) printf("Read a new line;\n");
      do {
	int masterNo = MASTER_NO;

        if (outLine[0])
          flushOutBuf(outLine);
#ifdef TEST_MANY_PC
        if (maxPcs == 0) 
#else
        if (telephoneNumber[0] == 0) 
#endif
          readLine(commandLine);
        else
          getLine(commandLine);
        commandPtr = commandLine;

/* added 941116 */
	if (*commandPtr == '#') {
		commandPtr++;
		if (*commandPtr == '#') {
			masterNo = MASTER_NO;
		} else {
			masterNo = atoi(commandPtr);
			while (isdigit(*commandPtr))
				commandPtr++;
		}
	}
	if (masterNo != MASTER_NO)
		break;

      }
      while (_name == getItem(&commandPtr, p1, 0) && !strcmp(p1, NAK));
      
/*
      if (!sendStatistics(PCno)) {
        checkSend_SETPCTIME();
      }
*/      
     
/*
       while (commandLine[0] == NAK);
*/      
      if (!strcmp(p1, ACK)) {
        if (previousCommand == _get_alarm_text) {
	  currentCommand = _idle;		/* not get alarm text */
          if (alarmMarkNode)
            netAckAlarm(alarmMarkNode, PCno, abcdMask);
          else {
           setAlarmMask(abcdMask);

	if (alarmMarkPtr[PCno & 7] == -1) {
		;
	} else {

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
            case 5:     /* added 920921 */
              aldm->alarmList[alarmMarkPtr[PCno & 7]].enableSent |= (1 << PCno);
              if (aldm->alarmList[alarmMarkPtr[PCno & 7]].confirm == 1)
                aldm->alarmList[alarmMarkPtr[PCno & 7]].confirmSent |= (1 << PCno); 
                            /* !! autoconfirm, new 910909 */
              break;
            default:
		break;
           }
	   alarmMarkPtr[PCno & 7] = -1;
	}

          }
	}
/*
!   removed as a result of revision 1.02 changes
!   (obs! revision 1.01 had also a bug that an extra LF was inserted)
!
!        flushOutBuf(commandLine);
!
*/
#ifdef TEST_MANY_PC
        if (maxPcs == 0) 
#else
        if (telephoneNumber[0] == 0) 
#endif
          readLine(commandLine);
        else
          getLine(commandLine);

/*
! 1.04      sends any statistics after whatever command encountered
!
*/
/*
          if (!sendStatistics(PCno)) {
            checkSend_SETPCTIME();
          }
*/
      }
      outLinePtr = outLine;
      outLine[0] = '\0';
      commandPtr = commandLine;
/*
      printf("'%s'\n", commandLine);
*/
    }
#ifdef DEBUG_MODE
    if (DEBUG) printf("cmd='%s'\n", commandPtr);
#endif

/*
!	940830 Added compatibility with IVTnet-groups, parseCommand now
!	returns the master number for the accessed master.
*/
    if (parseCommand(&commandPtr, p1, &t1, p2, &t2, p3, &t3, p4, &t4)
		!= MASTER_NO)
	continue;

#ifdef DEBUG_MODE
    if (DEBUG)
      fprintf(stderr, "p1='%s',p2='%s',p3='%s',p4='%s'\n", p1, p2, p3, p4);
    if (DEBUG)
      fprintf(stderr, "t1='%d',t2='%d',t3='%d',t4='%d'\n", t1, t2, t3, t4);
#endif

    if (t1 == _name && t2 == _name && !strncmp("set", p1, 3) && 
		!strncmp("datetime", p2, 8)) {
/* format of p3 is: "94.08.31 10:24:17" */

	setTime(atoi(&p3[1]),		/* year */
		atoi(&p3[4]),		/* month */
		atoi(&p3[7]),		/* day */
		atoi(&p3[10]),		/* hour */
		atoi(&p3[13]),		/* minute */
		atoi(&p3[16]));		/* second */
    } else if (t1 == _name && (!strncmp("EXIT", p1, 4) || !strncmp("exit", p1, 4)))
    { 
      fprintf(fpOut, "VisionCom terminating\015");
      mode = connected;
      exit(0);
    } else if (t1 == _name && (!strncmp("getlog", p1, 7))) {
/*                  'getlog 2x[24] 2y[24]'          */
      char var[32], var2[40], *varPtr, indexVarBuf[20];
      int node, idx, idxMax;
      double dNode, dIdx;
      
      commandPtr = commandLine;
      skipWhiteSpace(&commandPtr);
      while (!isWhiteSpace(*commandPtr) && !isTerminator(*commandPtr))
          commandPtr++;
/* ' 2x[24] 2y[24] '  */
      while (!isTerminator(*commandPtr)) {    /* while each var */
        skipWhiteSpace(&commandPtr);
        node = atoi(commandPtr);
        dNode = node;
        while (*commandPtr && isdigit(*commandPtr))
          commandPtr++;
        varPtr = var;
        while (!isWhiteSpace(*commandPtr) && !isTerminator(*commandPtr))
          *varPtr++ = *commandPtr++;
        *varPtr = 0;
/*
!   now, var is 'x[24]' and node is '2'
*/
        (void) skipIndex(var, indexVarBuf);
        idxMax = atoi(indexVarBuf);
/*
!   idxMax = 24, var = 'x'
*/
        for (idx = 0; idx < idxMax; idx++) {
          sprintf(var2, "%s[%d]", var, idx);
#ifdef DEBUG_MODE          
          if (DEBUG) {
            fprintf(fpOut, "idxMax = %d, indexVarBuf='%s'\n", idxMax, indexVarBuf);
            fprintf(fpOut, "var='%s',nod=%d,var2='%s'\n", var, node, var2);
          }
#endif
          dIdx = idx;
          p = doRequestVarMethod1(&dNode, &dIdx, var2);
          fprintf(fpOut, "S#0 %d %s %s\n", idx, var, p);
        }
      }
      fprintf(fpOut, "EXIT\n");
    } else if (t1 == _name && (!strncmp("SACK", p1, 7)))
    {
      ackStatistics(PCno);
#ifndef NEW_STAT_921119
      if (itsAStatCall) {     /* added 921117 */
        if (!sendStatistics(PCno)) {
          fprintf(fpOut, "EXIT\n");           /* ???? */
        }
      }
#endif      
    } else if (t1 == _name && (!strncmp("initstat", p1, 8))) { /* 7->8 930223 */
      initStatBuf();
    } else if (t1 == _name && (!strncmp("version", p1, 7))) {  /* new 930223 */
/*
!     Message flow is;
<PC>version 
<DCU>version=1.0
*/
      sprintf(outBuf, "version=%d.%d", PROTOCOL_MAJOR, PROTOCOL_MINOR);
      concat(&outLinePtr, outLine, outBuf);
    } else if (t1 == _name && (!strncmp("getstat", p1, 7)) && 
                                          t2 == _name && t3 == _value)
    {
      struct _metaStatBuff *entry;
      int i, row;

      if (t2 == _value && *((double *) p2) != MASTER_NO)
	continue;

      if (t2 == _value) {
		/* skip value and get name */
      }

      mode = connected;
      row = *((double *) p3);
      for (i = 0; i < 8; i++)
        if (!strcmp(p2, statS[i].name))
          break;
      if ((i >= 8) || (row < 0 || row > 29)) {  /* added row 930223 */
/*        sprintf(outBuf, "NAK\0");       BUG in vision, displays 'NA' 'K   ' */
          sprintf(outBuf, " \0");
        concat(&outLinePtr, outLine, outBuf);
      } else {
        entry = &(statS[i].rows[row]);
        if (entry->var[0] == 0) {
#if 1          
          int j, more = 0;
          for (j = row+1; j < 30; j++)
            if (statS[i].rows[j].var[0])
              more ++;
          if (more)                            /* added 930406 */
            strcpy(outBuf, "          <tom>");
          else 
            sprintf(outBuf, " \0");
          concat(&outLinePtr, outLine, outBuf);
#else
          sprintf(outBuf, " \0");
          concat(&outLinePtr, outLine, outBuf);
#endif
        } else {
          char inter[6];
          if (entry->intervall == 2678400)           /* 31 days */
            strcpy(inter, "    M");
          else if (entry->intervall == 604800)       /* 7 days */
            strcpy(inter, "    V");
          else if (entry->intervall == 86400)        /* 1 day */
            strcpy(inter, "    D");
          else 
            sprintf(inter, "%05d", entry->intervall / 60);

          sprintf(outBuf, "%02d%s%03d%s\0", 
                  entry->buff, inter, entry->duc, entry->var);
          concat(&outLinePtr, outLine, outBuf);
/*
          checkSumOf(&outLinePtr, outLine, outBuf);
*/          
        }
      }
    } else if (t1 == _name && (!strncmp("setstat", p1, 7)) && 
                                          t2 == _name && t3 == _value)
    {
      struct _metaStatBuff *entry;
      int i, row, free = -1;
      mode = connected;
      row = *((double *) p3);
      for (i = 0; i < 8; i++) {
        if (statS[i].name[0] == 0)
          if (free == -1)
            free = i;
        if (!strcmp(p2, statS[i].name))
          break;
      }
      if ((i >= 8 && free == -1) || (row < 0 || row > 29)) {  /* added row 930223 */
        sprintf(outBuf, "NAK\0");
      } else {
        if (i >= 8) {
          i = free;
          strcpy(statS[i].name, p2, 31);
        }
        entry = &(statS[i].rows[row]);
        
        entry->buff = atoin(&p4[1], 2);
        if (p4[7] == 'M' || p4[7] == 'm')
          entry->intervall = 2678400;           /* 31 days */
        else if (p4[7] == 'V' || p4[7] == 'v')
          entry->intervall = 604800;            /* 7 days */
        else if (p4[7] == 'D' || p4[7] == 'd')
          entry->intervall = 86400;             /* 1 day */
        else 
          entry->intervall = atoin(&p4[3], 5) * 60;
/*        entry->sample = 0;    old */
        entry->sample = initNextSample(entry->intervall);  /* added 920923 */

        entry->duc = atoin(&p4[8], 3);
        if (entry->intervall) {         /* intervall 0 -> delete entry */
          strncpy(entry->var, &p4[11], 31);
          if (entry->var[strlen(entry->var) - 1] == '"')
            entry->var[strlen(entry->var) - 1] = 0;
        } else
          entry->var[0] = '\0';

        if (!strcmp(entry->var, "<tom>"))/* BUGFIX!!! inserter ! (not) 930512   /* added 930406 */
          entry->var[0] = '\0';          

        sprintf(outBuf, "ACK\0");
/*
        sprintf(outBuf, "%02d%05d%03d%s\0", 
                  entry->buff, entry->intervall, entry->duc, entry->var);
*/
      }
      concat(&outLinePtr, outLine, outBuf);
    } else if (t1 == _name && (!strncmp("anystat", p1, 7)))
    {
      printf("Stat for PC #%d is %d entries\n", PCno, anyStatistics(PCno));
      sprintf(outBuf, "\0");
      concat(&outLinePtr, outLine, outBuf);
    } else if (t1 == _name && (!strncmp("showstat", p1, 8)))
    {
      struct _metaStatBuff *entry;
      int i, row;
      mode = connected;

      for (i = 0; i < 8; i++) {
        if (statS[i].name[0] == 0) {
          printf("Stat %d empty\n", i);
          continue;
        } 
        printf("Stat '%s':\n", statS[i].name);
        for (row = 0; row < 10; row++) {
          entry = &(statS[i].rows[row]);
          if (entry->var[0] == 0)
            printf("  %d: - empty -\n", row);
          else
            printf("  %d: buff %d, intervall %d, duc %d, var %s\n", row,
                entry->buff, entry->intervall,
                entry->duc,  entry->var);
        }
      }
      sprintf(outBuf, "\0");
      concat(&outLinePtr, outLine, outBuf);
    } else if (t1 == _name && (!strncmp("REBOOT", p1, 6)
                                 || !strncmp("reboot", p1, 6)))
    {
      mode = connected;
      system(commandLine);
    } else if (t1 == _name && (!strncmp("SHELL", p1, 6)
                                 || !strncmp("shell", p1, 6)))
    {
      mode = connected;
      system("");
    } else if (t1 == _name && (!strncmp("ANTAL", p1, 5)
                                 || !strncmp("antal", p1, 5)))
    {
      printf("Antal reboots: %d\n", sysVars->reboots);
      mode = connected;
    } else if (t1 == _name && (!strncmp("CLOSE_PCT", p1, 9)
                                 || !strncmp("close_pct", p1, 9)))
    {
      mode = connected;

      closePCT_atNodes();       /* added 930525 */

      method2running = 1;   /* fejka */
      clearMethod2();
      strcpy(outBuf, "ACK");
      concat(&outLinePtr, outLine, outBuf);
    } else if (t1 == _name && (!strncmp("kermit", p1, 6)
                                 || !strncmp("kermit", p1, 6)))
    {
      mode = connected;
      system(commandLine);
    } else if (t1 == _value && t2 == _value && t3 == _name) {
      mode = connected;

#define YOU_GOTTA_BE_LUCKY

#ifdef YOU_GOTTA_BE_LUCKY
      if ((((int) *((double *) p1)) % MAX_NODE4MASTER) == OUR_NODE) {
        p = doRequestVarMethod1(p1, p2, p3);
        sprintf(outBuf, "%g %g %s\0", *((double *) p1), *((double *) p2), p);
        concat(&outLinePtr, outLine, outBuf);
      } else {
        int idx, node;
        struct { short int node, idx; char name[32]; double value;} method1Arr[10];
        short int arrPek = 0; 
       
        node = *((double *) p1);
        idx = ((int) *((double *) p2)) % MAP_SIZE;
        clearMethod2();
/*
!   close_pct, functionality added 930524 
*/

/*        if (mapNode[idx] != node) {           /* added 930524 */

        if (!close_pct_atNode[node]) {
          idx += MAP_SIZE;                    /* added 930524 */
          close_pct_atNode[node] = 1;
        }
          
        mapNode[idx % MAP_SIZE] = node;		/* 941116 % MAX_NODE4MASTER; */
        method1Arr[arrPek].idx = idx;
        method1Arr[arrPek].node = node;
        strcpy(method1Arr[arrPek].name, p3);
        arrPek++;
        while (skipUntilNextCommand(&commandPtr)) {
          parseCommand(&commandPtr, p1, &t1, p2, &t2, p3, &t3, p4, &t4);
          if (t1 == _value && t2 == _value && t3 == _name) {
            node = *((double *) p1);
            idx = ((int) *((double *) p2)) % MAP_SIZE;
            if ((node % MAX_NODE4MASTER) == OUR_NODE) {
              p = doRequestVarMethod1(p1, p2, p3);
              sprintf(outBuf, "%g %g %s\0", *((double *) p1), *((double *) p2), p);
              concat(&outLinePtr, outLine, outBuf);
            } else { 
/*
!   close_pct, functionality added 930524 
*/

/*              if (mapNode[idx] != node)             /* added 930524 */
      
              if (!close_pct_atNode[node]) {
                idx += MAP_SIZE;                    /* added 930524 */
                close_pct_atNode[node] = 1;
              }
/*
!   if a slave (ver 930524-) get a idx > MAP_SIZE, clearMethod2()
!   will be issued. In pre-930524 slave releases, nothing happens (% MAP_SIZE)
!
!   You just have to be sure, that this happens once and only once for each node
!   Since nodes are not sorted, the first point here, will also be the first
!   in the slave.
*/


              mapNode[idx % MAP_SIZE] = node;	/* 941116 % MAX_NODE4MASTER; */
              method1Arr[arrPek].idx = idx;
              method1Arr[arrPek].node = node;
              strcpy(method1Arr[arrPek].name, p3);
              arrPek++;
            }
          } else
            break;
        } /* end of while more commands ! */
        if (arrPek > 0) {
/* sortera noderna !   OBS then we must move the above change from 930524 */
          int currNode, i, j, x, errCode;
          i = 0; j = 0;
          while (i < arrPek) {
            currNode = method1Arr[i].node;
            while (i < arrPek && method1Arr[i].node == currNode)
              i++;
            if (errCode = netGetNIdxVar(currNode, i - j, &method1Arr[j], useVars)) {
              if (errCode == NET_TIMEOUT) {
                for (x = j; x < i; x++) {
                  sprintf(outBuf, "%d %d %g\0", currNode,
                              -(method1Arr[x].idx % MAP_SIZE), 
                              0 /* method1Arr[x].value changed 930722 */);
                  concat(&outLinePtr, outLine, outBuf);
                }
              } else {
                for (x = j; x < i; x++) {
                  sprintf(outBuf, "%d %d %g\0", currNode, 
                              method1Arr[x].idx % MAP_SIZE, 
                              method1Arr[x].value);
                  concat(&outLinePtr, outLine, outBuf);
                }
              }
            } else {
              for (x = j; x < i; x++) {
                sprintf(outBuf, "%d %d %g\0", currNode, 
                              method1Arr[x].idx % MAP_SIZE, 
                              method1Arr[x].value);
                concat(&outLinePtr, outLine, outBuf);
              }
            }
            j = i;
	  } /* next node, if any */
        } /* if any */
      }
#else 
      p = doRequestVarMethod1(p1, p2, p3);
      sprintf(outBuf, "%g %g %s\0", *((double *) p1), *((double *) p2), p);
      concat(&outLinePtr, outLine, outBuf);
#endif  

#ifdef DEBUG_MODE
      if (DEBUG) printf("Len %d (%d)\n", strlen(outBuf), strlen(outLine));
#endif
    } else if (t1 == _token && !strncmp("&&&", p1, 3)) {
      mode = connected;
      doRequestVarMethod2(&outLinePtr, outLine);
    } else if (t1 == _name && p1[1] == '#' && (p1[0] == 'S' || p1[0] == 's') &&
               t2 == _value) {
/*                
!   1.04 added command for statistics
!
S#01 30 3:gt50
S#01 30 kalle
*/
      mode = connected;
      if (setStatistics(atoi(&p1[2]), (long) (*((double *) p2)), p3))
        sprintf(outBuf, "%s\0", ACK);
      else
        sprintf(outBuf, "%s\0", NAK);
    } else if (t1 == _value && t2 == _name && t3 == _name && 
                                              (!strcmp("getcal", p2, 6)))
    {
      mode = connected;
      p = doGetCalendar(p1, p3);
      sprintf(outBuf, "%s\0", p);
      concat(&outLinePtr, outLine, outBuf);
      checkSumOf(&outLinePtr, outLine, outBuf);
    } else if (t1 == _value && t2 == _name && t3 == _name && 
                                              (!strcmp("setcal", p2, 6)))
    {
      mode = connected;
      p = doSetCalendar(commandLine, p1, p3, p4);
      sprintf(outBuf, "%s\0", p);
      concat(&outLinePtr, outLine, outBuf);
      checkSumOf(&outLinePtr, outLine, outBuf);
    } else if (t1 == _value && t2 == _name && t3 == _token && t4 == _value) {
      mode = connected;
      if (!strncmp("=", p3, 1)) {
        if (doRequestSetVar(p1, p2, p4))
          sprintf(outBuf, "%s\0", ACK);
        else
          sprintf(outBuf, "%s\0", NAK);
      } else {  /* error, bad assignment character, expecting '=' */
          sprintf(outBuf, "%s", BAD_ASSIGNMENT);
      }
      concat(&outLinePtr, outLine, outBuf);
    } else if (t1 == _token && !strncmp("!!!", p1, 3)) {
      int antal;
      mode = connected;
      PCno = p1[3] & 15;        /* added mask 0b00001111,  930223 */
      
      antal = doRequestNoOfAlarms(PCno);     /* p1[3] */
      if (antal > 0)
        antal += 10;         /* add 10,  since we 1) dont clear our flag, 2) since we dont now actual value ... */
#ifdef NEW_STAT_921119
      if (antal > 0)
        sprintf(outBuf, "%d", antal);
      else if (anyStatistics(PCno))
        strcpy(outBuf, "STAT");
      else {
        checkSend_SETPCTIME();            /* this line added 930223 */
        sprintf(outBuf, "%d", antal);
      }
#else    
      itsAStatCall = 0;     /* added 921117 */
      if (!sendStatistics(PCno)) {
        checkSend_SETPCTIME();
      }
      sprintf(outBuf, "%d", antal);
#endif      
      concat(&outLinePtr, outLine, outBuf);
    } else if (t1 == _token && !strncmp("!!+", p1, 3)) {
      int antal, summa[4];
      mode = connected;
      PCno = p1[3] & 15;        /* added mask 0b00001111,  930223 */
      
      antal = doRequestNoOfAlarms(PCno);
      getSummaLarm(summa);
      if (antal > 0)
        antal += 10;
      if (antal > 0)
        sprintf(outBuf, "%d,%d,%d,%d", antal, summa[0], summa[1], summa[2]);
      else if (anyStatistics(PCno))
        strcpy(outBuf, "STAT");
      else {
        checkSend_SETPCTIME();
        sprintf(outBuf, "%d,%d,%d,%d", antal, summa[0], summa[1], summa[2]);
      }
      concat(&outLinePtr, outLine, outBuf);
    } else if (t1 == _name && (!strncmp("RECSTAT", p1, 7) || 
                                      !strncmp("RECSTAT", p1, 7))) {
      if (!sendStatistics(PCno)) {
        fprintf(fpOut, "EXIT\n");           /* ???? */
      }
    } else if (t1 == _name && (!strncmp("WAITSTAT", p1, 8) || 
                                      !strncmp("waitstat", p1, 8))) {
      MIN_WAIT_FOR_STAT = *((double *) p2);
      printf("Wait for stat is %d hours %d min and %d seconds\n", 
                MIN_WAIT_FOR_STAT / 3600, 
                (MIN_WAIT_FOR_STAT / 60) % 60, 
                MIN_WAIT_FOR_STAT % 60);
    } else if (t1 == _name && (!strncmp("MAXSTAT", p1, 4) || 
                                      !strncmp("maxstat", p1, 4))) {
      noOfStats2Send = *((double *) p2);
      if (noOfStats2Send < 1 || noOfStats2Send > 400)
        noOfStats2Send = 4;
      printf("Max stats [1-400] : %d\n", noOfStats2Send);
    } else if (t1 == _name && (!strncmp("DEBUG", p1, 5) || 
                                      !strncmp("debug", p1, 5))) {
      DEBUG = 1;
    } else if (t1 == _name && (!strncmp("NODEBUG", p1, 7) ||
                                      !strncmp("nodebug", p1, 7))) {
      DEBUG = 0;
    } else if (t1 == _token && !strncmp("***", p1, 3)) {
      struct tm tid;
      mode = connected;
      currentCommand = _get_alarm_text;
      if (doRequestAlarmText(p1, p2, p3, p4, p5, p6)) {
        int class, masterNo = MASTER_NO;
        class = ( ( (int) *((int *) p4) ) >> 3) & 0x0f;
        memcpy(&tid, localtime((time_t *) p5), sizeof(struct tm));

  if (1) 
  {
    char star[4];
    unsigned short int bitmask, bit;
/*    
    bit = ((int) *((int *) p2));
    bit = bit % 16;
    bitmask = 1 << bit;
*/
    bitmask = receiveAlarmMask(dm, meta, class);

    if (((int) *((int *) p4)) & 0x01)
      sprintf(star, "%s\0",
               (class == 0) ? "***" : (class == 1) ? "** " : "*  ");
    else
      sprintf(star, "%s\0",
               (class == 0) ? "..." : (class == 1) ? ".. " : ".  ");

    if (1) {
     sprintf(outBuf, "%05d%03d%03d%03d%05d%03d%s %02d.%02d.%02d %02d:%02d:%02d \"%s\"",
              bitmask, 
              masterNo,
              (int) *((int *) p1), (int) *((int *) p2),
              (int) *((int *) p3), 
/*              (int) *((int *) p4),    changed 910911  */

              ((int) *((int *) p4)) & 0x07,  /* remove class bits
                                              since 02 will remove from DHC
                                              but not 18 decimal !!! */     
              star, 
              (tid.tm_year > 100) ? tid.tm_year - 100 : tid.tm_year,
              tid.tm_mon + 1, tid.tm_mday, 
              tid.tm_hour, tid.tm_min, tid.tm_sec, p6);
    } else {
     sprintf(outBuf, "%s %d.%d %d %d %d %02d.%02d.%02d %02d:%02d:%02d \"%s\"",
              star, 
              masterNo,
              (int) *((int *) p1), (int) *((int *) p2),
              (int) *((int *) p3), (int) *((int *) p4),
              tid.tm_year, tid.tm_mon + 1, tid.tm_mday, 
              tid.tm_hour, tid.tm_min, tid.tm_sec, p6);
    }
    
  } else {
        sprintf(outBuf, "%s %d %d %d %d %02d.%02d.%02d %02d:%02d:%02d \"%s\"",
              (class == 0) ? "***" : (class == 1) ? "** " : "*  ",
              (int) *((int *) p1), (int) *((int *) p2),
              (int) *((int *) p3), (int) *((int *) p4),
              tid.tm_year, tid.tm_mon + 1, tid.tm_mday, 
              tid.tm_hour, tid.tm_min, tid.tm_sec, p6);
  }           
             
        concat(&outLinePtr, outLine, outBuf);
        checkSumOf(&outLinePtr, outLine, outBuf);
      } else {
        sprintf(outBuf, "%s\0", NAK);   
/*        sprintf(outBuf, "\0");        */
        concat(&outLinePtr, outLine, outBuf);
      }
    } else if (t1 == _value && t2 == _value && t3 == _value && t4 == _value) {
      mode = connected;
      if (doRequestConfirmAlarm(p1, p2, p3, p4))
        sprintf(outBuf, "%s\0", ACK);
      else
	sprintf(outBuf, "no such serial number\n");
/*        sprintf(outBuf, "%s\0", NAK);		*/

/*      
      sprintf(outBuf, "\0");
*/      
      concat(&outLinePtr, outLine, outBuf);
    }
  }
}

#ifdef INDEXVAR
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
    *indexVarBuf = '\0';
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
#endif

int closePCT_atNodes()    /* should maybe be called from clearMethod2() ? */
{
  int i;

  for (i = 0; i < 256; i++)
    close_pct_atNode[i] = 0;
} 

int clearMethod2()
{
/*
  idx = ((int) *index) % MAP_SIZE;
*/
  if (method2running) {
    int i;
    method2running = 0;
    for (i = 0; i < MAP_SIZE; i++) {
      map[i] = 0;
      mapNode[i] = 0;
    }
  }
/*
  mapNode[idx] = (int) *node;
*/
}
/*
!
*/
char *doRequestVarMethod1(node, index, name)
double *node, *index;
char *name;
{
  int id, idx, indexVar, mType;
  char *vPtr, indexVarBuf[20];
  static char buff[200];
  
  idx = ((int) *index) % MAP_SIZE;
  clearMethod2();
  mapNode[idx] = ((int) *node);		/* 941116 (((int) *node) % MAX_NODE4MASTER); */
  if ((((int) *node) % MAX_NODE4MASTER) == OUR_NODE) {
    
#ifdef INDEXVAR
     skipIndex(name, indexVarBuf);
#endif
    checkAlias(name);    
    if (map[idx] = checkHardCoded(name)) {           /* added 920609 */
      getHardCoded(map[idx], buff);
      return buff;
    }

    if ((id = metaId(meta, name)) < 0) {      /* name not found */
      map[idx] = -1;
      if (oldRevision) 
        *index = 0;
      else
        *index = -(*index);
    } else {
      map[idx] = id;
/*
!   usually upper 16 bits is 0 but if we are dealing with INDEXVAR's
!   use them as the index telling
!   if bit31 == 0    ->  bit 16 - bit 30   (15bits) indexnumber as an integer
!            == 1    ->  bit 16 - bit 30  = metaId of index var
!    (a bit complex ?)
*/
#ifdef DEBUG_MODE
      if (DEBUG) printf("Known: map[%d] = %d\n", idx, map[idx]);
#endif
      vPtr = (char *) metaValue(dm, meta, id);

      mType = metaType(meta, id);
#ifdef INDEXVAR
      if (mType == TYPE_INT_VEC) {
        int *vec1, *vec2;
        
        vec1 = ((int *) vPtr);
        vec2 = ((int *) metaValue(dmBackup, meta, id));

#ifdef DEBUG_MODE          
          if (DEBUG) {
            fprintf(fpOut, "varMethod1: vec1 = %d, vec2 = %d\n", vec1, vec2);
            fprintf(fpOut, "varMethod1: indexVarBuf='%s'\n", indexVarBuf);
          }
#endif

        if (indexVarBuf[0])
        {
          int idxVar;
          indexVar = getIndex(dm, meta, indexVarBuf, &idxVar);
#ifdef DEBUG_MODE          
          if (DEBUG) {
            fprintf(fpOut, "varMethod1: idxVar = %d, indexVar = %d\n",
                                  idxVar, indexVar);
          }
#endif
/*
!   new 910722
*/
          if (idxVar)
            map[idx] |= (1 << 31) | (idxVar << 16);
          else
            map[idx] |= (indexVar << 16);    /* constant */
          sprintf(buff, "%d\0", vec1[indexVar]);
          vec2[indexVar] = vec1[indexVar];
#ifdef DEBUG_MODE          
          if (DEBUG) {
            fprintf(fpOut, "varMethod1: ok !, vec1[%d] = %d\n", 
                                              indexVar, vec1[indexVar]);
          }
#endif
        } else {
          int i, size;
          char bf[20];
          size = metaSize(meta, id) / sizeof(long);
          buff[0] = 0;
          for (i = 0; i < size; i++)
          {
            sprintf(bf, "%d \0", *vec1);
            *vec2++ = *vec1++;
            strcat(buff, bf);
          }
        }
      } else if (mType == TYPE_FLOAT_VEC) {
        double *vec1, *vec2;
        
        vec1 = ((double *) vPtr);
        vec2 = ((double *) metaValue(dmBackup, meta, id));
        if (indexVarBuf[0])
        {
          int idxVar;
          indexVar = getIndex(dm, meta, indexVarBuf, &idxVar);
/*
!   new 910722
*/
          if (idxVar)
            map[idx] |= (1 << 31) | (idxVar << 16);
          else
            map[idx] |= (indexVar << 16);    /* constant */
          sprintf(buff, "%g\0", vec1[indexVar]);
          vec2[indexVar] = vec1[indexVar];
        } else {
          int i, size;
          char bf[20];
          size = metaSize(meta, id) / sizeof(double);     
          buff[0] = 0;
          for (i = 0; i < size; i++)
          {
            sprintf(bf, "%g \0", *vec1);
            *vec2++ = *vec1++;
            strcat(buff, bf);
          }
        }
      }
      else 
#endif      
          if (mType == TYPE_INT) {
            
        *((int *) metaValue(dmBackup, meta, id)) = *((int *) vPtr);
        sprintf(buff, "%d\0", *((int *) vPtr));
      } else if (mType == TYPE_FLOAT) {
        *((double *) metaValue(dmBackup, meta, id)) = *((double *) vPtr);
        sprintf(buff, "%g\0", *((double *) vPtr));
/*
      } else if (mType == TYPE_CALENDAR) {
        struct _calendar *cal, *cal2;
        int i, j;
        
        cal2 = ((struct _calendar *) metaValue(dmBackup, meta, id));
        cal = (struct _calendar *) vPtr;
        for (i = j = 0; i < NO_OF_CAL_ENTRIES; i++, j += 13)
        {
          sprintf(&buff[j], "%04d%01d%04d%04d", (int) cal->day[i],
              (int) cal->color[i], (int) cal->start[i], (int) cal->stop[i]);
          cal2->day[i] = cal->day[i];     cal2->color[i] = cal->color[i];
          cal2->start[i] = cal->start[i]; cal2->stop[i] = cal->stop[i];
        }
*/
      } else 
        ;
        /* none implemented type */
    }
    return buff;
  } else {          /* net */
    int nd, idx, errCode;
    double value;

    nd = (int) *node;
    idx = (int) *index;
    if (errCode = netGetIdxVar(nd, idx, name, &value, useVars)) {
      *index = -(*index);
      value = 0;
    }
    sprintf(buff, "%g\0", value);
    return buff;
  }
}

char *doGetCalendar(node, calName)
double *node;
char *calName;
{
  int id, idx, indexVar, mType;
  char *vPtr, indexVarBuf[20];
  static char buff[256];

  if ((((int) *node) % MAX_NODE4MASTER) == OUR_NODE) {
    if ((id = metaId(meta, calName)) < 0) {      /* name not found */
      buff[0] = '\0';
      return buff;
    }
    vPtr = (char *) metaValue(dm, meta, id);
    if (metaType(meta, id) == TYPE_CALENDAR) {
      struct _calendar *cal;
      int i, j;
        
      cal = (struct _calendar *) vPtr;
      for (i = j = 0; i < NO_OF_CAL_ENTRIES; i++, j += 17)
      {
        sprintf(&buff[j], "%04d%04d%01d%04d%04d", (int) cal->day[i],
            (int) cal->stopday[i],
            (int) cal->color[i], (int) cal->start[i], (int) cal->stop[i]);
      }
    } else
      buff[0] = '\0';    
    return buff;
  } else {
    char bits[256];
    struct _calendar cal;
    int size, i, j;

    if (netGetCal((int) *node, calName, &currentCalendarIdx, &cal))
      buff[0] = '\0';
    {
      for (i = j = 0; i < NO_OF_CAL_ENTRIES; i++, j += 17)
      {
        sprintf(&buff[j], "%04d%04d%01d%04d%04d", (int) cal.day[i],
            (int) cal.stopday[i],
            (int) cal.color[i], (int) cal.start[i], (int) cal.stop[i]);
      }
    }
  }
  return buff;
}

int readNo(bufPtr, noOfBytes)
char **bufPtr;
int noOfBytes;
{
  int result = 0;
  
  while (noOfBytes--)
  {
    result = result * 10 + *(*bufPtr)++ - '0';
  }
  return result;
}

char *doSetCalendar(line, node, calName, value)
char *line;
double *node;
char *calName, *value;
{
  int id, idx, indexVar, mType;
  char *vPtr, indexVarBuf[20];
  static char buff[200];

  /* check chs !!!! */
  /* implemented later */
  /* ok ! */

  if ((((int) *node) % MAX_NODE4MASTER) == OUR_NODE) {
    if ((id = metaId(meta, calName)) < 0) {      /* name not found */
      sprintf(buff, "NAK");
      return buff;
    }
    if (*value == '"')
      value ++;
      
    vPtr = (char *) metaValue(dm, meta, id);
    if (metaType(meta, id) == TYPE_CALENDAR) {
      struct _calendar *cal;
      int i;

      cal = (struct _calendar *) vPtr;
      for (i = 0; i < NO_OF_CAL_ENTRIES; i++)
      {
        cal->day[i] = readNo(&value, 4);
        cal->stopday[i] = readNo(&value, 4);
        cal->color[i] = readNo(&value, 1);
        cal->start[i] = readNo(&value, 4);
        cal->stop[i] = readNo(&value, 4);
      }
    }
    sprintf(buff, "ACK");
    return buff;
  } else {        /* slave node ! */
    struct _calendar cal;
    int size, i;

    if (*value == '"')
      value ++;
      
    for (i = 0; i < NO_OF_CAL_ENTRIES; i++)
    {
      cal.day[i] = readNo(&value, 4);
      cal.stopday[i] = readNo(&value, 4);
      cal.color[i] = readNo(&value, 1);
      cal.start[i] = readNo(&value, 4);
      cal.stop[i] = readNo(&value, 4);
    }
    if (netSetCal((int) *node, currentCalendarIdx, &cal))
      sprintf(buff, "NAK");
    else
      sprintf(buff, "ACK");
  }
  return buff;
}


int doRequestVarMethod2(outLinePtr, outLine)
char **outLinePtr, *outLine;
{
  char outBuf[256];
  
  method2running = 1;

  while (strlen(outLine) < 100) { 
    if (!getNextUpdatedVar(outBuf))      /* returns zero when switching nodes */
    {
      if (outBuf[0])                    /* added these two lines, 930709 */
        concat(outLinePtr, outLine, outBuf);
      break;
    }
    if (outBuf[0] == 0)
      continue;
    concat(outLinePtr, outLine, outBuf);
  }
  if (outLine[0] == 0) {
    outLine[0] = ' ';
    outLine[1] = 0;
  }
}

char *getNextUpdatedVar(outBuf)
char *outBuf;
{
  char *vPtr, buff[128];
  int id;
  
  *outBuf = '\0';

#ifdef DEBUG_MODE
      if (DEBUG) printf("getNextUpdatedVar: currentNode %d, currentId %d\n", 
                          currentNode, currentId);
#endif
  if ((currentNode % MAX_NODE4MASTER) == OUR_NODE) {
    if (currentId < MAP_SIZE) {
      if ((mapNode[currentId] % MAX_NODE4MASTER) != OUR_NODE) {
        currentId++;
        return outBuf;
      }

      if (getHardCoded(map[currentId], buff)) {      /* added 920609 */
        sprintf(outBuf, "%d %d %s", mapNode[currentId] /* 941116 currentNode */, currentId, buff);
        currentId++;
        return outBuf;
      }

      if ((id = map[currentId]) == 0) {   /* <= -> == 920323 !! */
        currentId++;
        return outBuf;
      }
      id &= 0xffff;     /* take only lower 16 bits */
      if (!(vPtr = (char *) metaValue(dm, meta, id))) {   /* ?? wrong !! */
        currentId++;
        return outBuf;
      }
      if ((memcmp(vPtr, metaValue(dmBackup, meta, id),
                (metaType(meta, id) == TYPE_INT) ? 4 : 8)) ||
          (idleCount[currentId] > IDLE_MAX_COUNT) ||
          (metaType(meta, id) == TYPE_INT_VEC) || 
          (metaType(meta, id) == TYPE_FLOAT_VEC))
      {
        if (metaType(meta, id) == TYPE_INT) {
          *((int *) metaValue(dmBackup, meta, id)) = *((int *) vPtr);
          sprintf(buff, "%d", *((int *) vPtr));
        } else if (metaType(meta, id) == TYPE_FLOAT) {
          *((double *) metaValue(dmBackup, meta, id)) = *((double *) vPtr);
          sprintf(buff, "%g", *((double *) vPtr));
        } else if (metaType(meta, id) == TYPE_INT_VEC) {
          int *vec1, *vec2, indexVar;
          vec1 = (int *) vPtr;
          vec2 = ((int *) metaValue(dmBackup, meta, id));
          indexVar = (map[currentId] >> 16) & 0x7fff;
          if (map[currentId] & (1 << 31))    /* variable as index */
          {
            if (metaType(meta, indexVar) == TYPE_INT)
              indexVar = *((int *) metaValue(dm, meta, indexVar));
            else if (metaType(meta, indexVar) == TYPE_FLOAT)
              indexVar = (int) (*((double *) metaValue(dm, meta, indexVar)));
	  } /* else already constant */
          indexVar &= 0xffff;
          sprintf(buff, "%d", vec1[indexVar]);
          vec2[indexVar] = vec1[indexVar];

        } else if (metaType(meta, id) == TYPE_FLOAT_VEC) {
          double *vec1, *vec2;
          int indexVar;
          
          vec1 = (double *) vPtr;
          vec2 = ((double *) metaValue(dmBackup, meta, id));
          indexVar = (map[currentId] >> 16) & 0x7fff;
          if (map[currentId] & (1 << 31))    /* variable as index */
          {
            if (metaType(meta, indexVar) == TYPE_INT)
              indexVar = *((int *) metaValue(dm, meta, indexVar));
            else if (metaType(meta, indexVar) == TYPE_FLOAT)
              indexVar = (int) (*((double *) metaValue(dm, meta, indexVar)));
	  } /* else already constant */
          indexVar &= 0xffff;
          sprintf(buff, "%g", vec1[indexVar]);
          vec2[indexVar] = vec1[indexVar];
        } else {
          /* none implemented type */
        }
        idleCount[currentId] = 0;
        sprintf(outBuf, "%d %d %s", mapNode[currentId] /* 941116 currentNode */, currentId, buff);
      }   /* else updated */
      else {
#ifdef DEBUG_MODE
        if (DEBUG)
              printf("Inc: idle[%d] = %d, map=%d\n",
               currentId, idleCount[currentId], map[currentId]);
#endif
        idleCount[currentId]++;     /* idle-count is new for 910517 */
      }
      currentId++;
      return outBuf;
    } else {
      currentNode++;
      currentId = 1;
      return 0;             /* changed return outBuf; -> 0, 920331 */
    }
  } else {
    int i, node;
    char buf[256], smallBuf[20];
    char lastNode = 0; 
    
    outBuf[0] = '\0';
    for ( ; currentId < MAP_SIZE; currentId ++)
    {
      if ((((node = mapNode[currentId]) % MAX_NODE4MASTER) == OUR_NODE) || (node == lastNode))
        continue;

      node %= MAX_NODE4MASTER;		/* 941116 */

      lastNode = node;
#ifdef DEBUG_MODE
      if (DEBUG) printf("  getNextUpdatedVar: netGetUpdate from node %d\n", 
                          node);
#endif
      if (netGetUpdated(node, buf, useVars)) {
        currentNode = 0;                /* return to master */
        currentId = 1;            /* new 920331 */
        return 0;
      }
      for (i = 0; i < 50; i += 5) {
        if (buf[i] == 0)
          break;
        if (buf[i] & 0x80) {
          float v;
          memcpy(&v, &buf[i + 1], sizeof(float));
/*          sprintf(smallBuf, "%d %d %g ", node, buf[i] & 0x7f, v);	*/
          sprintf(smallBuf, "%d %d %g ", mapNode[currentId], buf[i] & 0x7f, v);	/* 941116 */
        } else {
          long v;
          memcpy(&v, &buf[i + 1], sizeof(long));
/*          sprintf(smallBuf, "%d %d %d ", node, buf[i] & 0x7f, v);	*/
          sprintf(smallBuf, "%d %d %d ", mapNode[currentId], buf[i] & 0x7f, v);	/* 941116 */
        }
        strcat(outBuf, smallBuf);
      } /* end for all slots in record */

#ifdef TEST_920914
      if (i == 0) {
        currentId ++;                   /* added 920602 */
        return 0;               /* empty from slave ! */
      }
#endif

      currentId ++;                   /* added 920602 */
      return 0;   /* changed 1->0, 930709, so just one req from each slave */
      

      if (i == 0)
        return 0;               /* empty from slave ! */

      currentNode = 0;      /* return ! */
      currentId = 1;        /* new 920331 */
      return 1;

    } /* end for currentId */
    currentNode = 0;                /* return to master */
    currentId = 1;              /* new 920331 */
    return 0;
  }
}

int doRequestSetVar(node, name, value)
double *node, *value;
char *name;
{
  int id, indexVar;
  char *vPtr, indexVarBuf[20];

  if ((((int) *node) % MAX_NODE4MASTER) == OUR_NODE) {
#ifdef INDEXVAR
     skipIndex(name, indexVarBuf);
#endif
    checkAlias(name);    
    if (id = checkHardCoded(name)) {           /* added 920609 */
      setHardCoded(id, *value);
      return 1;
    }
    if ((id = metaId(meta, name)) < 0) {
      /* name not found */
      return 0;
    } else {
      vPtr = (char *) metaValue(dm, meta, id);

#ifdef INDEXVAR
      if (metaType(meta, id) == TYPE_INT_VEC) {
        int *vec1, v;
        vec1 = ((int *) vPtr);
        v = (int) *value;
        if (indexVarBuf[0]) {
          indexVar = getIndex(dm, meta, indexVarBuf, 0);
          vec1[indexVar] = v;
        } else {
          int size;
          size = metaSize(meta, id) / sizeof(long);
          while (size --)
            *vec1++ = v;
        }
      } else if (metaType(meta, id) == TYPE_FLOAT_VEC) {
        double *vec1, v;
        vec1 = ((double *) vPtr);
        v = (double) *value;
        if (indexVarBuf[0]) {
          indexVar = getIndex(dm, meta, indexVarBuf, 0);
          vec1[indexVar] = v;
        } else {
          int size;
          size = metaSize(meta, id) / sizeof(long);
          while (size --)
            *vec1++ = v;
        }
      }
      else 
#endif      
      if (metaType(meta, id) == TYPE_INT) {
        *((int *) vPtr) = (int) *value;
      } else if (metaType(meta, id) == TYPE_FLOAT) {
/*        *((double *) metaValue(dmBackup, meta, id)) = (double) *value; */
        *((double *) vPtr) = (double) *value;
      } else 
        /* none implemented type */
        ;
    }
    return 1;
  } else {
    int nd;
    char buf[20];

    nd = (int) *node;
    sprintf(buf, "%g", *value);
    netPutIdxVar(nd, name, buf);
    return 1;
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

/*
!   anyAlarms - a generic routine for retrieving number of pending alarms
*/
int anyAlarms(pcid)
int pcid;
{
  int cnt, count, i, m, pc, mask, as, ns, cs, ds, es = 0;
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
    as = (entry->sendStatus & ALARM_SEND_ASSERT) ? 
        entry->sendMask ^ entry->assertSent & entry->sendMask : 0;
    ns = (entry->sendStatus & ALARM_SEND_NEGATE) ? 
        entry->sendMask ^ entry->negateSent & entry->sendMask : 0;
    cs = (entry->sendStatus & ALARM_SEND_CONFIRM) ? 
        entry->sendMask ^ entry->confirmSent & entry->sendMask : 0;
    ds = (entry->sendStatus & ALARM_SEND_DISABLE) ? 
        entry->sendMask ^ entry->disableSent & entry->sendMask : 0;
    es = (entry->sendStatus & ALARM_SEND_ENABLE) ? 
        entry->sendMask ^ entry->enableSent & entry->sendMask : 0;
#else
    as = entry->sendAssert ^ entry->assertSent & entry->sendAssert;
    ns = entry->sendNegate ^ entry->negateSent & entry->sendNegate;
    cs = entry->sendConfirm ^ entry->confirmSent & entry->sendConfirm;
    ds = entry->sendDisable ^ entry->disableSent & entry->sendDisable;
#endif
/*    
    if ((aldm->alarmList[i].sendAssert && !aldm->alarmList[i].assertSent) ||
          (aldm->alarmList[i].sendNegate && !aldm->alarmList[i].negateSent) ||
          (aldm->alarmList[i].sendConfirm && !aldm->alarmList[i].confirmSent) ||
          (aldm->alarmList[i].sendDisable && !aldm->alarmList[i].disableSent))
*/
          
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

int getSummaLarm(summa)
int summa[];
{
  int i, pek2;
  char *nak;

  summa[0] = summa[1] = summa[2] = summa[3] = 0;
/*
!	add up a,b,c&d for slaves
*/
  for (i = 1; i < MAX_NODE_NO; i++) {
    nak = (char *) &nodeMap[i].activeABCD;
    summa[0] += nak[0];
    summa[1] += nak[1];
    summa[2] += nak[2];
    summa[3] += nak[3];
  }
/*
!	now check master site
*/
  pek2 = 0;
  while (pek2 < aldm->alarmListPtr) {
    if (!aldm->alarmList[pek2].confirm)
      summa[aldm->alarmList[pek2].class & 0x03] ++;
    pek2 ++;
  }
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

int serial2Ptr(serie)
int serie;
{
  int i, cnt;

  cnt = aldm->alarmListPtr;
  for (i = 0; i < cnt; i++) {
    if ((aldm->alarmList[i].serialNo & 0xffff) == serie)
      break;
  }
  return i < cnt ? i : -1;
}

int doRequestAlarmText(node, alarm, serie, status, dtime, text)
int *node, *alarm, *serie, *status;
time_t *dtime;
char *text;
{
  int cnt, count, i, nd, pc, m, mask;

  cnt = aldm->alarmListPtr;
  count = anyAlarms(PCno);

  pc = (1 << PCno);
  m = mask = 0;
  if (abcdMask[0] & pc) { m |= 1;  mask |= 0x00c0; }
  if (abcdMask[1] & pc) { m |= 2;  mask |= 0x0030; }
  if (abcdMask[2] & pc) { m |= 4;  mask |= 0x000f; }
  if (abcdMask[3] & pc) { m |= 8;  mask |= 0x0003; }

  setAlarmMask(abcdMask);
  {
    struct _alarmEntry entry, *this;
      
    for (; internalCounter < cnt; internalCounter++) {
#ifdef _V920914_1_40
      this = &aldm->alarmList[internalCounter];
      if (!(this->sendStatus & ALARM_SEND_INIT))
        continue;                                           /* not set yet */
      if (!(this->sendMask & pc))
        continue;                                           /* not to us */
      if ((this->sendStatus & ALARM_SEND_ASSERT) &&
          !(this->assertSent & pc))
        break;                                       /* ok to send assert */
      if ((this->sendStatus & ALARM_SEND_NEGATE) &&
          !(this->negateSent & pc))
        break;                                       /* ok to send negate */
      if ((this->sendStatus & ALARM_SEND_CONFIRM) &&
          !(this->confirmSent & pc))
        break;                                       /* ok to send confirm */
      if ((this->sendStatus & ALARM_SEND_DISABLE) &&
          !(this->disableSent & pc))
        break;                                       /* ok to send disable */
      if ((this->sendStatus & ALARM_SEND_ENABLE) &&
          !(this->enableSent & pc))
        break;                                       /* ok to send enable */
#else
      if (((aldm->alarmList[internalCounter].sendAssert & pc) &&
          !(aldm->alarmList[internalCounter].assertSent & pc)) ||
          ((aldm->alarmList[internalCounter].sendNegate & pc) &&
          !(aldm->alarmList[internalCounter].negateSent & pc)) ||
          ((aldm->alarmList[internalCounter].sendConfirm & pc) &&
          !(aldm->alarmList[internalCounter].confirmSent & pc)) ||
          ((aldm->alarmList[internalCounter].sendDisable & pc) &&
          !(aldm->alarmList[internalCounter].disableSent & pc)))
      {
if (DEBUG) {
  printf("Found entry %d, pc = %d\n", internalCounter, pc);
}        
/*        if (m & (1 << aldm->alarmList[internalCounter].class))  */

          break;
      }
#endif
    }               /* end of for-loop */

    alarmMarkNode = 0;
    if (internalCounter >= cnt) {
      struct _alarmStrct alarmBuf;
      if (!(nd = getNextAlarmNode(mask, pc)))
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

    {                                    /* add.920921 */
      struct _alarmModule2 *aldm2;
      aldm2 = (struct _alarmModule2 *)
            (((char *) aldm) +
              (aldm->noOfAlarmEntries * sizeof(struct _alarmEntry) +
               sizeof(short) + sizeof(long)));
      entry.disable = aldm2->alarmPts[entry.alarmIndex].disable;
    }
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
        if (entry.confirmSent)
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
         if (entry.negateSent)
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
        *status =  ((entry.negateSent) ? 0 : 1)
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
        *status =  ((entry.negateSent) ? 0 : 1)
              | (entry.confirm << 1)
              | (entry.disable << 2) | ((entry.class & 0x0f) << 3);
#endif
        break;
      default:
        fprintf(fpOut, "Program fault: alarmtext switch\n");
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
#define ALARM_ACTIVE   0x01
#define ALARM_CONFIRM  0x02
#define ALARM_DISABLE  0x04

int doRequestConfirmAlarm(node, alarmNo, serialNo, status)
double *node, *alarmNo, *serialNo, *status;
{
  int i, cnt, serie, alarm, count;
  struct _alarmModule2 *aldm2;
  
  if ((((int) *node) % MAX_NODE4MASTER) != OUR_NODE) {
    int n, a, l, s;
    n = *node;
    a = *alarmNo;
    l = *serialNo;
    s = *status;
    if (netConfirmAlarm(n, a, l, s) == NET_NOSUCHALARM)
	return 0;
    return 1;
  }
  serie = *serialNo;
  alarm = *alarmNo;
  cnt = aldm->alarmListPtr;
  count = 0;

  for (i = 0; i < cnt; i++) {
    if ((aldm->alarmList[i].alarmNo == alarm) &&
        ((aldm->alarmList[i].serialNo & 0xffff) == serie)) {
      break;
    }
  }
  if (i >= cnt) {
    return 0;

/*    fprintf(fpOut, "no such serial number\n");	*/
/*    return 1;         /* no such serial number, but ACK !!! (910905) */
  }

  aldm2 = (struct _alarmModule2 *)
            (((char *) aldm) +
              (aldm->noOfAlarmEntries * sizeof(struct _alarmEntry) +
               sizeof(short) + sizeof(long)));


  if ((int) *status & ALARM_CONFIRM) {
    if (!aldm->alarmList[i].confirm) {
      aldm->alarmList[i].confirm = 1;
#ifdef _V920914_1_40
      aldm->alarmList[i].sendStatus |= ALARM_SEND_CONFIRM;
#else
      aldm->alarmList[i].sendConfirm = 0xff;
#endif
      aldm->alarmList[i].confirmTime = time(0);
      aldm->alarmList[i].confirmSent = 0;
    }
  }
/*
!             Do the disable/enable show
*/
#ifdef _V920914_1_40
  if (aldm2->alarmPts[aldm->alarmList[i].alarmIndex].disable !=
                                (((int) *status & ALARM_DISABLE) != 0)) {
    if (aldm2->alarmPts[aldm->alarmList[i].alarmIndex].disable) {
      aldm->alarmList[i].sendStatus |= ALARM_SEND_ENABLE;
      aldm->alarmList[i].enableSent = 0;
      aldm->alarmList[i].enableTime = time(0); 
      aldm->alarmList[i].disable = 
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
      aldm->alarmList[i].confirm = 1; 
      aldm->alarmList[i].confirmTime = aldm->alarmList[i].disableTime; 
      aldm->alarmList[i].sendStatus |= ALARM_SEND_CONFIRM;
#ifndef TWO_WHEN_BLOCKED
      aldm->alarmList[i].confirmSent = 0xff; /* fake sent to all */
#endif
    }
  }
#else
  if (aldm2->alarmPts[aldm->alarmList[i].alarmIndex].disable !=
                                (((int) *status & ALARM_DISABLE) != 0)) {
    aldm->alarmList[i].sendDisable = 0xff;
    aldm->alarmList[i].disableSent = 0;
    aldm->alarmList[i].disable = 
        aldm2->alarmPts[aldm->alarmList[i].alarmIndex].disable = 
                                (((int) *status & ALARM_DISABLE) != 0);
/*
!   automatic confirm when disable of an alarm, 910517
*/
    if (aldm2->alarmPts[aldm->alarmList[i].alarmIndex].disable &&
          (aldm->alarmList[i].confirm == 0))
    {
      aldm->alarmList[i].confirm = 1; 
      aldm->alarmList[i].confirmTime = time(0); 
    /*      aldm->alarmList[i].sendConfirm = 0xff;      ??? */
/*
      aldm->alarmList[i].sendConfirm = 1;        removed 910730
      aldm->alarmList[i].confirmSent = 0;
*/
    }
  }
#endif
  else  /* when we hold the same status */
  {
    if (((int) *status & ALARM_DISABLE) != 0)
      return 0;         /* trying to disable when already disabled -> NAK */
    else
      return 1;         /* trying to enable when already enabled -> ACK */
  }
  return 1;             /* otherwise reply with ACK */
}

item getItem(commandLine, p, pp)
char **commandLine;
char *p, *pp;
{
  char buff[100];
  if (!skipWhiteSpace(commandLine))
    return _nothing;

  if (isdigit(**commandLine) || (**commandLine == '-')) {
    moveIt(buff, commandLine);
    *((double *) p) = atof(buff);
    if (pp) strcpy(pp, buff);                         /* added 920601 */
    return _value;
  } else if (isalpha(**commandLine)) {
    moveIt(p, commandLine);
    return _name;
  } else {
    if ((**commandLine == COMMAND_SEPERATOR) && 
        ( isWhiteSpace(*(*commandLine + 1)) || 
          isTerminator(*(*commandLine + 1))))
      return _nothing;
    moveIt(p, commandLine);
/*
#ifdef DEBUG_MODE
    if (DEBUG) printf("getItem: token ! '%s'\n", p);   
#endif
*/
    return _token;
  }
}

int parseCommand(commandLine, p1, t1, p2, t2, p3, t3, p4, t4)
char **commandLine;
char *p1, *p2, *p3, *p4;
item *t1, *t2, *t3, *t4;
{
  char buf2[100], buf3[100];
  int masterNo = MASTER_NO;

  if (**commandLine == '#') {
	(*commandLine)++;
	if (**commandLine == '#') {
		masterNo = MASTER_NO;
	} else {
		masterNo = atoi(*commandLine);
		while (isdigit(**commandLine))
			(*commandLine)++;
	}
  }

  *t1 = getItem(commandLine, p1, 0);
  *t2 = getItem(commandLine, p2, buf2);
  *t3 = getItem(commandLine, p3, buf3);
  *t4 = getItem(commandLine, p4, 0);

/* the following if-clause was added 920527 to enable the following command; */
/*
!      2 7 9Utetemp     -> translated to ->   9 7 Utetemp
*/
  if (*t1 == _value && *t2 == _value && *t3 == _value && *t4 == _nothing) {
    char *pp;
    *((double *) p1) = *((double *) p3);    /*     atoi(p3);    */
    pp = buf3;    /* p3;    added 920601 */
    while (*pp && isdigit(*pp))
      pp++;
    memcpy(p3, pp, strlen(pp) + 1);       /* added '+1' 920623 */
    *t3 = _name;
/*
!     Added 920602 release 1.71
*/    
  } else if (*t1 == _value && *t2 == _value && *t3 == _token && *t4 == _value) {
    char *pp;
    *((double *) p1) = *((double *) p2);    /*     atoi(p3);    */
    pp = buf2;
    while (*pp && isdigit(*pp))
      pp++;
    memcpy(p2, pp, strlen(pp) + 1);     /* added '+1' 920623 */
    *t2 = _name;
  }
  return masterNo;
}

skipUntilNextCommand(commandLine)
char **commandLine;
{
  char c;
  while (c = skipWhiteSpace(commandLine)) {
    (*commandLine)++;
    if (c == '&' && isWhiteSpace(**commandLine)) {
      return 1;
    }
    skipToken(commandLine);
  }
  return 0;
}

char skipToken(commandLine)
char **commandLine;
{

  while (!isWhiteSpace(**commandLine) && !isTerminator(**commandLine))
    (*commandLine)++;
  return isTerminator(**commandLine) ? 0 : **commandLine;
}

char skipWhiteSpace(commandLine)
register char **commandLine;
{
  if (!*commandLine)
    return 0;
  while (isWhiteSpace(**commandLine))
    (*commandLine)++;
  return isTerminator(**commandLine) ? 0 : **commandLine;
}

moveIt(target, source)
char *target, **source;
{
  char *t;
  t = target;
  while (!isWhiteSpace(**source) && !isTerminator(**source))
    *target++ = *(*source)++;
  *target = '\0';  
}

/*
!   checks for white space, i.e. space or tabs, but here NOT LF and CR
*/
isWhiteSpace(c)
char c;
{
  return c == ' ' || c == '\011';
}

/*
!   checks for terminators, NULL, LF and CR
*/
isTerminator(c)
char c;
{
  return c == '\0' || c == '\012' || c == '\015';
}

bind(dm, aldm, meta, dmBackup, headerPtr1, headerPtr2, headerPtr3, headerPtr4)
char **dm, **meta, **dmBackup, 
        **headerPtr1, **headerPtr2, **headerPtr3, **headerPtr4;
struct _alarmModule **aldm;
{
/*
!   bind to data module VARS, storage location for variables
*/  
#define NEW_BINDING
#ifdef NEW_BINDING
  int i = 0;
  while (1) {
    *dm = (char *) linkDataModule(NAMEOFDATAMODULE, headerPtr1);
    if (!(*dm)) {
      i++;
      if (i < 10) {
        sleep(5);
        continue;
      }
    }
    break;
  }
#else
  *dm = (char *) linkDataModule(NAMEOFDATAMODULE, headerPtr1);
#endif
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
#ifdef NEW_BINDING
  i = 0;
  while (1) {
    *aldm = (struct _alarmModule *) linkDataModule("ALARM", headerPtr3);
    if (!(*aldm)) {
      i++;
      if (i < 10) {
        sleep(5);
        continue;
      }
    }
    break;
  }
#else
  *aldm = (struct _alarmModule *) linkDataModule("ALARM", headerPtr3);
#endif
  if (!*aldm) {
    fprintf(stderr, "cannot link to datamodule '%s'\n", "ALARM");
    fprintf(stderr, "check if process 'scan' is running\n");
    return 0;
  }
/*
!   bind to data module VAR1/VAR2, storage location for backup variables
*/  
#ifdef NEW_BINDING
  i = 0;
  while (1) {
    *dmBackup = (char *) linkDataModule(
                    (useVars == 0) ? NAMEOFDATAMODULE1 : NAMEOFDATAMODULE2,
                                 headerPtr4);
    if (!(*dmBackup)) {
      i++;
      if (i < 10) {
        sleep(5);
        continue;
      }
    }
    break;
  }
#else
  *dmBackup = (char *) linkDataModule(
                    (useVars == 0) ? NAMEOFDATAMODULE1 : NAMEOFDATAMODULE2,
                                 headerPtr4);
#endif
  if (!*dmBackup) {
    fprintf(stderr, "cannot link to datamodule '%s'\n", 
                    (useVars == 0) ? NAMEOFDATAMODULE1 : NAMEOFDATAMODULE2);
    fprintf(stderr, "check if process 'scan' is running\n");
    return 0;
  }
  return 1;         /* BUG !! this line missing previous 920427 !! */
}
/*
!   encapsulate alm_set function
*/
int timeoutSignal(s, intervall)
int s, intervall;
{
  int alarm_id;
  
  if ((alarm_id = alm_set(s, (intervall << 8) | 0x80000000)) == -1)
  {
    fprintf(stderr, "cannot set alarm\n");
    exit(1);
  }
  return alarm_id;
}

int minicall_vars(sn, l, spwd, pris, err)
int *sn, *l, **spwd, **err;
double **pris;
{
  int id;
  char name[25];
  static int junk[10];

  strcpy(junk, "xxxxxxxx");
  strcpy(name, "minicallSendNumber");
  if ((id = metaId(meta, name)) < 0) /* name not found */
    *sn = 0;
  else {
    if (metaType(meta, id) == TYPE_INT)
      *sn = *((int *) metaValue(dm, meta, id));
    else
      *sn = 0;
  }
  strcpy(name, "minicallLegitimering");
  if ((id = metaId(meta, name)) < 0) /* name not found */
    *l = 0;
  else {
    if (metaType(meta, id) == TYPE_INT)
      *l = *((int *) metaValue(dm, meta, id));
    else
      *l = 0;
  }
  strcpy(name, "minicallSendPassword");
  if ((id = metaId(meta, name)) < 0) /* name not found */
    *spwd = junk;
  else {
    if (metaType(meta, id) == TYPE_INT_VEC)
      *spwd = (int *) metaValue(dm, meta, id);
    else
      *spwd = junk;
  }
  strcpy(name, "minicallPris");
  if ((id = metaId(meta, name)) < 0) /* name not found */
    *pris = 0;
  else {
    if (metaType(meta, id) == TYPE_FLOAT)
      *pris = (double *) metaValue(dm, meta, id);
    else
      *pris = 0;
  }
  strcpy(name, "minicallError");
  if ((id = metaId(meta, name)) < 0) /* name not found */
    *err = 0;
  else {
    if (metaType(meta, id) == TYPE_INT)
      *err = (int *) metaValue(dm, meta, id);
    else
      *err = 0;
  }
}

char *getTelephone(pc, t3)	/* changed 931112 */
int pc, *t3;
{
  int id, t1, t2;
  char name[20];
  static char str[40];
  
  strcpy(name, "telephonePC_ctrl");
  if ((id = metaId(meta, name)) < 0) /* name not found */
    *t3 = 0;
  else {
    if (metaType(meta, id) == TYPE_INT_VEC)
      *t3 = *(((int *) metaValue(dm, meta, id)) + pc);
    else
      *t3 = 0;
  }
    
  strcpy(name, "telephonePC");
  if ((id = metaId(meta, name)) < 0) /* name not found */
    return 0;
  if (metaType(meta, id) == TYPE_INT_VEC)
    t1 = *(((int *) metaValue(dm, meta, id)) + pc);
  else
    return 0;
    
  strcpy(name, "telephonePC_area");
  if ((id = metaId(meta, name)) < 0) /* name not found */
    t2 = 0;
  else if (metaType(meta, id) == TYPE_INT_VEC)
    t2 = *(((int *) metaValue(dm, meta, id)) + pc);
  else
    t2 = 0;
    
  if (t2) 
    sprintf(str, "0%d%d\0", t2, t1);
  else if (t1)
    sprintf(str, "%d\0", t1);
  else {
#if 0  	
    strcpy(name, "minicall");
    if ((id = metaId(meta, name)) < 0) /* name not found */
      return 0;
    if (metaType(meta, id) == TYPE_INT_VEC)
      t1 = *(((int *) metaValue(dm, meta, id)) + pc);
    else
      return 0;         /* if both area and telephonePC is 0 */
    sprintf(str, "020910037");
#else
      return 0;         /* if both area and telephonePC is 0 */
#endif
  }
  return str;
}

int getTimeOut_Value()  /* added 930830 */
{
  int id;
  char name[20];

  if (dm == 0 || meta == 0)
    return 0;
  strcpy(name, "modemTimeout");
  if ((id = metaId(meta, name)) < 0) /* name not found */
    return 0;
  if (metaType(meta, id) == TYPE_INT)
    return *((int *) metaValue(dm, meta, id));
  else if (metaType(meta, id) == TYPE_FLOAT)
    return (int) (*((double *) metaValue(dm, meta, id)));
  else
    return 0;
}


/*
! getLine         - as readLine, but will shutdown/establish connection
*/
int getLine(commandLine)
char *commandLine;
{
  int alarm_id, intervall, timeout_intervall;
  static int firstTime;
  static long previousSuccessCall;
  static int tries, minimumWaitTime;
  
  timeout_intervall = getTimeOut_Value();  /* added 930830 */
  if (mode == connected) {
    tries = 0;
    minimumWaitTime = 0;
    previousSuccessCall = 0;
  }
  while (1) {
    if (mode == disconnected)
      PCno = -1;
    if (mode == disconnected)
      intervall = 5;                    /* check alarm every 5 sec */
    else if (mode == connected) {
                        /* disconnect after 120 (930830, was 10) sec */
      intervall = timeout_intervall ? timeout_intervall : 120; 
    } else if (mode == shutdown)
      intervall = 5;                    /* if no modem, no OK reply ! */
    else
      intervall = 60;                   /* during setup */
   
    alarm_id = timeoutSignal(2, intervall);
     
    timeoutFlag = 0;
    if (readLine(commandLine))
    {
      if (-1 == alm_delete(alarm_id)) {
        fprintf(stderr, "cannot delete alarm, error = %d\n", errno);
      }
      
#ifdef DEBUG_MODE
      if (DEBUG)
        fprintf(stderr, "%d: %s", mode, commandLine);
#endif
/*
! Strategi:
!
!   if we are trying to call PC and recieve
!
!         NO ANSWER or
!         NO CARRIER, then
!                         lets try one more time and if the same message,
!                         wait 30 min, then repeat 
!
!         BUSY, then
!                         lets try one more time and if the same message,
!                         wait 1 min, then repeat for ten times, then
!                         wait 10 min,
!
!         NO DIALTONE, then
! 
*/      
      if (!strncmp(commandLine, "CONNECT 2400", 12)) {
        previousSuccessCall = time(0);
        mode = connected;
/*        setSpeed(pathIn, 2400);   */
      } else if (!strncmp(commandLine, "ONNECT 2400", 11)) {
        previousSuccessCall = time(0);
        mode = connected;
/*        setSpeed(pathIn, 2400);   */
      } else if (!strncmp(commandLine, "CONNECT 1200", 12)) {
        previousSuccessCall = time(0);
        mode = connected;
/*        setSpeed(pathIn, 1200);   */
      } else if (!strncmp(commandLine, "CONNECT", 7)) {
        previousSuccessCall = time(0);
        mode = connected;
/*        setSpeed(pathIn, 1200);   */
      } else if (!strncmp(commandLine, "NO CARRIER", 10)) {
        previousSuccessCall = time(0);      /* fake it */
        if (mode == establish) {
          if (tries <= 2)
            minimumWaitTime = 30;
          else
            minimumWaitTime = 120;
        } else if (mode == connected) {
          minimumWaitTime = 1;                  /* removed 910905... 300; */
        }
/*        setSpeed(pathIn, originalSpeed);    */
        mode = disconnected;
      } else if (!strncmp(commandLine, "NO ANSWER", 9)) {
        previousSuccessCall = time(0);      /* fake it */
        if (mode == establish) {
          if (tries <= 2)
            minimumWaitTime = 30;
          else
            minimumWaitTime = 120;
        }
        mode = disconnected;
      } else if (!strncmp(commandLine, "BUSY", 4)) {
        if (mode == establish) {
          if (tries <= 2)
            minimumWaitTime = 30;
          else
            minimumWaitTime = 120;
        }
        mode = disconnected;
      } else if (!strncmp(commandLine, "NO DIALTONE", 11)) {
        if (mode == establish) {
          if (tries <= 2)
            minimumWaitTime = 30;
          else
            minimumWaitTime = 120;
        }
        mode = disconnected;
      } else if (!strncmp(commandLine, "RING", 4)) {
        mode = establish;
      } else if (!strncmp(commandLine, "OK", 2)) {
        if (mode == shutdown) {
          fprintf(fpOut, "ATH0\n");
          mode = disconnected;      /* we will not get any NO CARRIER message */
        }
      } else {
/*        mode = connected;    */          /* ???? */

        if (mode != establish) {
          return 1;       /* ok, return to caller */
        }
      }
      
      if (mode == connected && firstTime) {
/*        fprintf(fpOut, "HELLO PC\015");   */
        sleep(2);
#define CTRL_PBX_1 	0x01
#define CTRL_PBX_2 	0x02
#define CTRL_PULSE 	0x04
#define CTRL_MINICALL 	0x08

	if (ctrl & CTRL_MINICALL) {		/* added 3 line 931115 */
	  int sn, l, *spwd, *err;
	  double *pris;

	  PCno = telephonePC;

	  switch8_7_no_even(pathIn, 2);
	  switch8_7_no_even(pathOut, 1);

	  minicall_vars(&sn, &l, &spwd, &pris, &err);

	  execute_minicall(sn, l, spwd, telephoneNumber, pris, ctrl, err);


/* ??? eller shutdown forst ??? maste aterstalla ifall vi blir uppringda */
	  switch8_7_no_even(pathIn, 0);	/* 1-> even/7, 0-> none/8 */

	  PCno = -1;

	} else
{
#ifdef NEW_STAT_921103
  int pc;                         /* use global PCno */
#else
  int PCno, pc;
#endif

                        /* if pc calls us ?? */
  PCno = pc = telephonePC;

#ifdef NEW_STAT_921119
        fprintf(fpOut, "\015"); /* added 921119 */
        fprintf(fpOut, "HELLO PC\015");
#else

        if (conditionalAnyStatistics(pc) == 0) {
          fprintf(fpOut, "HELLO PC\015");   /* to sync, 910517 */
        } else {
#define CHEAPER_v920911
#ifdef CHEAPER_v920911
          int i;

          if (noOfStats2Send < 1 || noOfStats2Send > 400)
            noOfStats2Send = 4;
#ifdef NEW_STAT_921103
                /* just send one stat and main loop takes care of SACK */
          if (!sendStatistics(PCno))
                ;
          else
              itsAStatCall = 1;    /* added 921117 */

#else
          for (i = 0; i < noOfStats2Send; i++)
          {
            if (!sendStatistics(PCno))                 /* 1 */
              break;
          }
#endif          
/*
!   if all sent, no more calls until 3 hours (or so) has passed by
*/
          if (conditionalAnyStatistics(pc) == 0) {
            statisticSentAt[PCno & 7] = getRelTime(0);
          }
#else
          if (sendStatistics(PCno))                 /* 1 */
            if (sendStatistics(PCno))               /* 2 */
              if (sendStatistics(PCno))             /* 3 */
                sendStatistics(PCno);               /* 4 */
#endif                

#ifdef NEW_STAT_921103
#else
          fprintf(fpOut, "EXIT\015");
#endif          
	}
#endif                                /* NEW_STAT_921119 */	
        firstTime = 0;
}

      }     
     
    } else {     /* end of if (readline...) */
      if (-1 == alm_delete(0)) {
        fprintf(stderr, "cant del alarm, error = %d\n", errno);
      }
#ifdef DEBUG_MODE
      if (DEBUG)
        fprintf(stderr, "readLine returned zero: timeoutflag = %d, mode=%d\n", timeoutFlag, mode);
#endif
      if (timeoutFlag) {
        if (mode != disconnected) {
          if (mode == shutdown) {     /* ok, no OK reply == no modem or in command state ! */
            mode = disconnected;
          } else {
            write(pathOut, "+++", 3);/* use unbuffered since no \n can be used*/
            mode = shutdown;
          }
          continue;
        } else if (mode == disconnected) {
          if ((time(0) - previousSuccessCall) > minimumWaitTime) {
            int pc;
#ifdef TEST_MANY_PC            
            static int nextPcPtr;
            if (nextPcPtr >= maxPcs)
             nextPcPtr = 0;
            pc = mapPcs[nextPcPtr++];
            telephoneNumber = getTelephone(pc, &ctrl);
            telephonePC = pc;
#else
            pc = telephonePC;
#endif
/*
! OBS! call ONLY our PC's, there might be another visionCom running...
*/
            if ((anyAlarms(pc) || conditionalAnyStatistics(pc))
                  && telephoneNumber) {
              char mBuf[100];
#if 1
    	if (ctrl & CTRL_MINICALL) {
/* since os9 cannot handle different parities on input/output we choose p7 */
/*	  switch8_7_no_even(pathIn, 1);	/* 1-> even/7, 0-> none/8 */

	} else {
	  switch8_7_no_even(pathIn, 0);	/* 1-> even/7, 0-> none/8 */
	}
/*	sprintf(mBuf, "AT\015"); write(pathOut, mBuf, strlen(mBuf)); */


              sprintf(mBuf, "ATE0Q0D%s%s%s%s%s\015",
              		ctrl & CTRL_PULSE ? "P" : "T",
              		ctrl & CTRL_PBX_1 ? "0" : "",
              		ctrl & CTRL_PBX_2 ? "00" : "",
              		ctrl & (CTRL_PBX_1 | CTRL_PBX_2) ? "W" : "",
              		ctrl & CTRL_MINICALL ? "020-910037" :
							telephoneNumber);
#else              
              sprintf(mBuf, "ATE0Q0DT%s\015", telephoneNumber);
#endif
              write(pathOut, mBuf, strlen(mBuf));
              tries ++;
              mode = establish;
              firstTime = 1;
/*
              whenSETPCTIME_wasSent = 0;
*/
              continue;
            }
/*            pc ++;      */
          }
        }
      }
      else {
#ifdef DEBUG_MODE
        if (DEBUG)
          fprintf(stderr, "framing error\n");
#endif
      }
      mode = disconnected;        /* or idle ?????? */
    }
  }   /* end of while ( not read a line ) */
}

/*
! readLine        - reads a command line from specified port
*/
readLine(commandLine)
char *commandLine;
{
  int i, c;
  char buf;
  
  i = 0;
  while (c = read(pathIn, &buf, 1))
  {
    if (c == -1) {
#ifdef DEBUG_MODE
      if (DEBUG) printf("read ret.-1\n");
#endif
      return 0;
    }
    commandLine[i++] = buf;
    if (buf == '\012') {
      commandLine[i] = '\0'; 
      return i;
    }
  }
#ifdef DEBUG_MODE
  if (DEBUG)
    fprintf(stderr, "read ret. 0\n");
#endif
}

int concat(outLinePtr, outLine, outBuf)
char **outLinePtr, *outLine, *outBuf;
{
  char buf[10], *bptr;
  bptr = buf;
/*
  printf("*outLinePtr = %d, outLine = %d\n", *outLinePtr, outLine);
  printf("outBuf:'%s'\n", outBuf);
  printf("outLine:'%s'\n", outLine);
*/
  if (oldRevision) 
    strcpy(buf, " & ");
  else
    strcpy(buf, " ");
  
  if (*outLinePtr != outLine) {
    while (*(*outLinePtr)++ = *bptr++)
      ;
    --(*outLinePtr);
  }
 
  while (*(*outLinePtr)++ = *outBuf++)
    ;
  --(*outLinePtr);
/*
  printf("outLine:'%s'\n", outLine);
  printf("concat: slut, *outLinePtr = %d\n", *outLinePtr);
*/  
}

int skipInBuffer()
{
  int i, b, nr, skipped = 0;
  char buf[10];
  
/* #define TEST_SKIP  */            /* test case, removed 921119 */

  for (i = 0; i < 30; i++) {
    if ((b = _gs_rdy(pathIn)) > 0) {
#ifdef TEST_SKIP
      fprintf(stderr, "b = %d bytes was waiting, ", b);
#endif        
      if (b > 10)
        b = 10;
      nr = read(pathIn, buf, b);
      if (nr > 0)
        skipped = 1;
#ifdef TEST_SKIP
      fprintf(stderr, "nr = %d bytes skipped\n", nr);
      fprintf(stderr, "'%s'\n", buf);
#endif        
    } else
      break;
  }
  return skipped;
}

int flushOutBuf(outLine)
char *outLine;
{
  static int previous_Queued;

#if 1                             /* added 921218 */
  if (previous_Queued) {     /* previous fetched from queue !! */
    skipInBuffer();
    previous_Queued = 0;
  } else {                   /* previous not fetched from queue ! */
    if (_gs_rdy(pathIn) > 0) {    /* we will be using the queue ! */
      previous_Queued = 1;        /* but not next time ! */
    }
  }
#else     
  skipInBuffer();
#endif
  
#ifdef DEBUG_MODE
  if (DEBUG) printf("FlushLen=%d\n", strlen(outLine));
#endif
  fprintf(fpOut, "%s\015", outLine);
}

int calcCheckSum(outLine)
char *outLine;
{
  int i, len, chs;
  
  chs = 0;
  len = strlen(outLine);
  for (i = 0; i < len; i++) {
    chs += outLine[i];
  }
/*
  chs += 32;
  chs &= 0xff;
  chs |= 0x80;
*/  
  return chs & 0xff;
}

int checkSumOf(outLinePtr, outLine, outBuf)
char **outLinePtr, *outLine, *outBuf;
{
  int chs;
  
  chs = calcCheckSum(outLine) + 32;

  chs &= 0xff;
  chs |= 0x80;
  sprintf(outBuf, " %c\0", chs);

  while (*(*outLinePtr)++ = *outBuf++)
    ;
  --(*outLinePtr);
}

