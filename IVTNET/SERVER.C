/* server.c  1993-11-24 TD,  version 1.65 */
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
! server.c
! Copyright (C) 1991-1993 IVT Electronic AB.
*/

/*
!     server.c is part of the IVTnet product
!
!     File: server.c
!     
!     Contains main function for server process at master site
!
!     History     
!     Date        Revision Who  What
!     
!      3-sep-1991     1.0  TD   Start of coding
!
!     27-nov-1991     1.1  TD   GET_VAR request updated to match slave
!                               process_request() accepts parameter rBuf !!!!
!                               size in nodemap structure changed to short !
!                               still something strange (currentNode->size -97)
!                     1.11      get_var request replies with name always
!
!      1-feb-1992     1.2  TD   Added alarm to more than one PC
!                               Added PC-var requests from more than one PC
!                               Distribution of variables from slaves
!
!     20-feb-1992     1.21 TD   Changed static map2Node buffer to area 3f400
!                               Added 5kByte stack  (cc -m=5 ...)
!                               Added find in slow/slowest adds 0 in next free
!
!     21-feb-1992     1.22 TD   Added fastNode_1 and fastNode_2,
!                               DISTR_PACKET_SIZE is now 8 (8*8 = 64nodes)
!
!     26-feb-1992     1.23 TD   A try for success.. if size > 0 -> size = -1;
!
!      1-apr-1992     1.30 TD   Bugfix, removed bit REGISTRATE from switch 
!                               expression in function processRequest.
!                               -> remote 0 is now ok at slave sites
!
!     11-may-1992     1.40 TD   THE FINAL Bug found !!!
!                               The bufptr in routine getPacket was
!                               previously holding signed values !!!
!                               Has now been changed to unsigned to manage
!                               buffer sizes above 127 bytes.
!                               Yes, previous version DID call memcpy with 
!                               a block size that could happen to be negative !!
!
!     12-jun-1992     1.50 TD   The Very Ultimate Bug has been located !
!                               When a slave tells another slave it wants
!                               to become a subscriber, the other slave reply
!                               with the value and a message (in the error 
!                               field) saying NET_REGISTRATE. This tells the 
!                               slave not to try to update its subscription
!                               within the next 10 minutes (600sec).
!                               Otherwise if this message is absent from the
!                               reply, the slave will force to update its
!                               subscription within the next 20 sec !!.
!
!                               Now the error was that anytime when a slave
!                               sent the issue to a subscriber, the subscriber
!                               received the packet without this message.
!                               But since the server distributes this packet,
!                               we are able to add this field here, so the 
!                               following line has been added in the 
!                               distribution function !
!
!             packet->data[1] = NET_REGISTRATE;      Added 920612 !!! 
!
!     23-jun-1992     1.51 TD   YAB !, (Yet Another Bug) (Gave Err#107)
!                               Solution, added the following if-case
!
!        if (packet->data[1] != NET_NOSUCHVAR)
!          distributePacket(packet);
!
!      8-jul-1992     1.52 TD   Added user idc variable map 'nodeUp[]'
!                               This variable will reflect available nodes as 1
!                               and others as 0
!
!     23-sep-1992     1.60 TD   Added first test of MEM-functions and REBOOT
!                               Edition 7
!
!      7-dec-1992     1.61 TD   Bugfix in 'nodeUp'-handling
!
!     20-jul-1993     1.62 TD   Bugfix in 'tNoOfAck_x/tNoOfNak_x'-handling
!                               Edition 8
!
!     14-sep-1993     1.63 TD   Added reset of 'failCount' when node returns 0
!                               Edition 9
!
!     #define NEW_930928 to use an IVTnet data module and 255 nodes to poll
!     #define NEW_931025 to use idc-variable 'int pollNode[64]' to tell which
!                        nodes to poll and option '-n=<node-list>'
!
!     #define NEW_STAT_AS_OF_931021 to use a metastat data module (prim-mem!)
!
!     25-oct-1993     1.64 TD   Added glb-var tries[][] in doStat to re-issue
!                               stat requests that hasn't arrive during 10 min.
!                               Re-requests are issued every minute.
!                               Edition 10 is compile with
!     #undef  NEW_930928
!     #define NEW_931025
!     #undef  NEW_STAT_AS_OF_931021
!
!     23-nov-1993     1.65 TD   Bugfix. TRAPV could occur when casting double 
!			        to float during GET_STAT_VAR. Added conversion
!				with function Float()
!     24-nov-1993     1.65 TD   Added checkCommand call after for(slowNode...
!				since we never called it when no nodes were up
*/

@_sysedit: equ 11
@_sysattr: equ $8016

#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <module.h>

#include "ivtnet.h"     /* layer 3 protocol */
#include "net.h"        /* interprocess communication protocol */

/* #define NEW_930928   */
#ifdef NEW_930928
#include "nodes.h"
#endif

#define MASTER 1

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

#ifdef NEW_930928
#define MAX_NODE_NO    256
#else
#define MAX_NODE_NO    64     /* 15 */
#endif

#define MAX_FAIL4RETRY 20     /* 10 */
int mode2 = 0;

unsigned short *nodeaddr= 0x402; /* node address used by this node */

struct _system *sysVars = SYSTEM_AREA;

/*
!   level 2, data link layer 
*/
struct _header
{
  unsigned char size,
                version,
                targetMaster, targetNode,
                sourceMaster, sourceNode,
                command;
};

struct _packet {
  struct _header header;
  unsigned char data[251];
};

struct _packet *packet;

/*
!   level 3, transport/network layer
*/
struct _message *message;

#ifdef NEW_930928
char *rBuf, *sBuf;
#else
char rBuf[256], sBuf[512];  /* changed temp. 920130 256->512 */
#endif

long int byteCounter = 0;
/*
!   server specific information
*/
#ifndef NEW_930928
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
#endif

/*      Added 920708
!  a pointer to user programs idc-variable declared as    
!       int nodeUp[MAX];
!  
!  our initialization routine declares our nodeUpPtr to point to their vector
!  if it exists and then nodeUpMax will hold the size of the vector (MAX)
*/
long int *nodeUpPtr = 0;  
int nodeUpMax = 0;

/*
!       Added 931025
!  a pointer to user program idc-variable declared as
!       int pollNode[MAX];
!  if it exists, each positive entry tells us to poll that node
!  entry 0 is used by us as our node counter, next node to poll
*/
#define NEW_931025
#ifdef NEW_931025
long int *pollNodePtr = 0;
int pollNodeMax = 0;
long pollNodes[64];       /* used with -n option */
#endif  

static int globalFlagAnyNodeFailed;

static unsigned char *netError = 0x003ffd9;

/*
typedef struct _node noder[256];
noder nodeMap;
noder *nodeMapPtr;
*/

/* struct _node nodeMap[MAX_NODE_NO];  */

struct _ivtnet *ivtnet;

#ifdef NEW_930928
struct _node *nodeMap;
#else
struct _node *nodeMap = (struct _node *) 0x0003f000;
#endif

static short int fastNode_1, fastNode_2;
#if 1
static unsigned char *map2Node = (unsigned char *) 0x0003f600;
static int nextFree_inMap2Node;
#else
static unsigned char map2Node[256];
static int nextFree_inMap2Node;
#endif

static long sendTime[256];

/*
static unsigned char *alarmTextBufPtr = (unsigned char *) 0x3ff00;
*/

struct _method1Arr *localArrPtr = 0;

/*struct _node (*nodeMapPtr)[];*/

struct _node *currentNode;

static long issuedCommandAt;      /* time when command issued */
static long issuedCommand;        /* type of command */
static short issuedCommandToNode;
static unsigned char issuedCommandFromPc;

int netFree;
int netTaskAccomplished;

/*
unsigned char *currentCommand = COMMAND_ADDRESS;
*/

/*
int currentCommand;
*/


char *dm = 0, *meta = 0;
struct _alarmModule *aldm = 0;
static char *headerPtr1 = 0, *headerPtr2 = 0, *headerPtr3 = 0;

char *calendarPtr = 0;
int *calendarIdx = 0;

char *getMemPtr = 0;

/*
! remote display message information (RDMI)
*/
static int setHostNode = 0, setHost_active = 0;

struct _screenContext {
  unsigned char display[2][40];
  unsigned char map[10];
  unsigned char keyCode, keyDown;
/*  unsigned char flashLed, currentLed;   not used !! */
  int spawnedScreenPid;
} *screenContext;

char screenBuff[100];


/*
!   
*/

typedef enum { _idle, _unknown, _packetSent, _packetReceived, _timeout } sts;
sts status;


int DEBUG = 0, DEBUG2 = 0, TELL = 0;
int verbose = 0;
int failures = 0;
int quickMode = 0;
int quickSleep = 1;
int onePoll = 0;

int bpsList[] = {
  50, 75, 110, 134, 
  150, 300, 600, 1200, 
  1800, 2000, 2400, 3600, 
  4800, 7200, 9600, 19200, 
  38400
};

int bpsSelected = -1;              /* use default (9600) */

/*
!   end - of - variable - declaration
*/



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
!     signals initiate commands, global variable 'sysVars->netCommand' will be set.
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
      fprintf(stderr, "cant link '%s',err%d\n", name, errno);
/*
      fprintf(stderr, "can't link to event '%s', errno = %d\n", name, errno);
*/
      Abort();
    }
    fprintf(stderr, "cant create '%s',err%d\n", name, errno);
/*    
    fprintf(stderr, "can't create event '%s', errno = %d\n", name, errno);
*/
    Abort();
  }
}

/*
!
*/
int bindModules(dm, meta, aldm)
char **dm, **meta, **aldm;
{
  if (!*dm)
    *dm = (char *) linkDataModule("VARS", &headerPtr1);
  if (!*meta)
    *meta = (char *) linkDataModule("METAVAR", &headerPtr2);
  if (!*aldm)  
    *aldm = (char *) linkDataModule("ALARM", &headerPtr3);
  return (*dm && *meta && *aldm);
}

#ifdef WHAT_IS_THIS
/*
!   anyAlarms - a generic routine for retrieving number of pending alarms
*/
int anyAlarms()
{
  int cnt, count, i;
  
  cnt = aldm->alarmListPtr;
  count = 0;
  for (i = 0; i < cnt; i++) {
    if ((aldm->alarmList[i].sendAssert && !aldm->alarmList[i].assertSent) ||
          (aldm->alarmList[i].sendNegate && !aldm->alarmList[i].negateSent) ||
          (aldm->alarmList[i].sendConfirm && !aldm->alarmList[i].confirmSent) ||
          (aldm->alarmList[i].sendDisable && !aldm->alarmList[i].disableSent))
      count++;
  }
  return count;
}
#endif

#if 0
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
}

/*
!   takes the message blocks and unpack, includes key codes
!   then, pack our display context in the same message
*/
void captureDisplay(message)
struct _message *message;
{    /* move bytes for led's into our sys structure */
  int xPos, i, j, next, q, p;
  static char yPos = 0;
#define MAX_NEXT 40
  
  screenContext->keyCode = message->mix.keyDisplay.key.keyCode;
  screenContext->keyDown = message->mix.keyDisplay.key.keyDown;
  message->mix.keyDisplay.display.flashLed = sysVars->flashBits;
  message->mix.keyDisplay.display.currentLed = sysVars->currentBits;
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

  sprintf(param, "%d", screenContext);
  paramSize = strlen(param);
  if ((screenContext->spawnedScreenPid = 
                    os9fork("screen", param, paramSize, 0, 0, 0, 0)) == -1) {
    return 0; /* cannot fork to screen module */
  }
  return 1;       /* ok ! */
}


int ntread(node, buf, size)
int node;
unsigned char *buf;
int size;
{
  int bt, pt, i;
  pt = 0;
  
  if (DEBUG2) printf("ntread: siz=%d\n", size);
  while ((bt = allNetRead(node, &buf[pt], size)) > 0) {
    if (DEBUG2) printf("ntread: bt=%d\n", bt);

  if (DEBUG2) {
    printf("Packet data:\n");
    for (i = pt; i < pt + bt; i++)
      printf("%02x, ", buf[i]);
    printf("\n"); 
  }

    size -= bt;
    pt += bt;
    if (size >= 0)
      break;
  }
  if (DEBUG2) printf("ntread: ok %d bytes read\n", pt);
  return (bt == -1) ? -1 : pt;
}

static char *cmdTexts[] = { "GET_VAR", "PUT_VAR", "ALARM_NO", 
                  "ALARM_TEXT_1","ALARM_TEXT_2","CONFIRM_ALARM",
                  "SET_TIME","SET_HOST","KEY_DISPLAY",
                  "GETIDX_VAR_1","GET_UPDATED_1","SET_REMOTE",
                  "REMOTE_DATA","GET_VARIDX","GET_CAL",
                  "SET_CAL","GETNIDX_VAR_1", "VERSION",
                  "GETIDX_VAR_2", "GET_UPDATED_2", "GETNIDX_VAR2",
                  "CLEAR_ALARM", "GET_STAT_VAR",
                  "ALLOCATE_MEM", "GET_MEM", "PUT_MEM", "LOAD_PROGRAM",
                  "REBOOT", "NEW"
};
static char *netErrors[] = {
"NET_NOERROR",
"NET_NOSUCHVAR","NET_ILLEGALTYPE","NET_CANCEL",
"NET_NOBINDING","NET_NOALARMS","NET_BUSY",
"NET_SPAWN_ERROR","NET_TIMEOUT","NET_LOGOUT",
"NET_CTRL_C","NET_NOSUCHALARM","NET_NOMOREALARMS",
"NET_REGISTRATE"
};

int tellPacket(currentNode, node, buf)
struct _node *currentNode;
int node;
unsigned char *buf;
{
  int i, cmd;

  if (DEBUG2) printf("Node %d, rCnt = %d, sCnt = %d\n", node,
                               currentNode->rCount, currentNode->sCount);
  printf("Packet size %d bytes, protocol version %d\n", buf[0], buf[1]);
  printf("To node %d, from node %d, command %d\n", buf[3], buf[5], buf[6]);

  cmd = buf[7] & 0x7f;
  if (buf[7] & 0x80)
    printf("REQUEST for ");
  else
    printf("REPLY to ");
  if (cmd & REGISTRATE) {
    cmd &= ~REGISTRATE;
    printf("REGISTRATE ");
  }
  if (cmd > 0 && cmd <= (sizeof(cmdTexts) / sizeof(cmdTexts[0])))
    printf("Command: '%s'\n", cmdTexts[cmd - 1]);
  else
    printf("Command: UNKNOWN COMMAND !!, %d\n", cmd);
  if (buf[8] != 0)
    printf("!!!! ---  ERROR %d, '%s' --- !!!!\n", buf[8], netErrors[buf[8]]);
  if (cmd < 2) {
    printf("Variable: '%s'\n", &buf[9]);
  }
  printf("\n");
}

int dumpPacket(currentNode, node, buf)
struct _node *currentNode;
int node;
unsigned char *buf;
{
  int i;

  if (DEBUG2) printf("Node %d, rCnt=%d, sCnt=%d\n", node,
                               currentNode->rCount, currentNode->sCount);
  printf("Packet size %d bytes, protocol version %d\n", buf[0], buf[1]);
  printf("To node %d, from node %d, command %d\n", buf[3], buf[5], buf[6]);

  if (DEBUG2) printf("Packet data:\n");
  for (i = 7; i < buf[0]; i++)
    printf("%02x, ", buf[i]);
  printf("\n");

}

int checkNodeStatus(currentNode, node, DEBUGing)
struct _node *currentNode;
int node;
int DEBUGing;
{
  int debugInfo = 0;
  int halt = 0;
  int prev;


/*
  if (currentNode->failCount > MAX_FAIL4RETRY) {
  }
*/
  prev = currentNode->size;
  
  if (!currentNode->failCount) {
    currentNode->pollCount ++;
    if ((currentNode->size = netpoll(node)) == -1) {
      currentNode->failCount = 1;
      if (prev != -1) {
        currentNode->failFlag = 1;
        globalFlagAnyNodeFailed ++;
        currentNode->noOfAlarms = 0;   /* clear any knowledge about alarms ! */
      } else {
        if (currentNode->failFlag)
          globalFlagAnyNodeFailed ++;   /* still exist, not polled */
      }
    } else {
      currentNode->failFlag = 0;
      if (prev == -1)
        sendTime[node] = 0;
 if (!onePoll) {                          /* just so we can try */
      currentNode->size = netpoll(node);
      currentNode->size = netpoll(node);    /* this is default executed */
      currentNode->size = netpoll(node);    /* use option -1 to prohibit */
 }
    }
  } else {
    currentNode->failCount ++;
    currentNode->size = -1;
  }
  return (currentNode->size >= 0);
}

int getPacket(currentNode, node, buf)
struct _node *currentNode;
int node;
unsigned char *buf;       /* 920511 ! unsigned added !!!! */
{
  int pt, more, bt, sz;

  bt = ntread(node, &buf[0], 1);

  if (bt == -1) {       /* empty buffer and return with fail (0) */
    if (DEBUG) printf("Error, ntread returns -1, empty-ing buffer\n");
    emptyBuffer();    /*    myNetpoll(100);    myNetpoll(node);*/
    return 0;
  }
  if (buf[0] == 0) {
    if (DEBUG) printf("Error, getPacket: packet says 0 bytes !\n");
    emptyBuffer();
    return 0;
  }
  if (currentNode->size < 0) {
    if (DEBUG) {
      printf("ERROR!!\n");
      printf("getPacket: currentNode->size = %d\n", currentNode->size);
    }
  }
  if (DEBUG)  
    printf("       packet says size %d bytes (%d available)\n", 
                                                buf[0], currentNode->size);
  currentNode->size --;
  more = buf[0] - 1;
  pt = 1;
  while (more) {
    while (currentNode->size && more) {
      sz = (more > currentNode->size) ? currentNode->size : more;
      if (sz == -1) printf("getPacket: size = %d, more = %d\n", currentNode->size, more);
      bt = ntread(node, &buf[pt], sz);
      if (bt == -1) {       /* empty buffer and return with fail (0) */
        if (DEBUG) printf("Error, ntread returns -1, empty-ing buffer\n");
        emptyBuffer();
        return 0;
      }
      pt += bt; more -= bt; currentNode->size -= bt;
      if (DEBUG2 && node <= 2) printf("getPacket:  inner;  bt = %d, more = %d, size = %d\n", bt, more, currentNode->size);
    }
    if (more && ((currentNode->size = bufPoll(node)) < 1))
    {
      if (DEBUG)   printf("getPacket: returns FAIL !!!\n");
      return 0;
    }
  }
  currentNode->rCount ++;
  if (DEBUG2)   printf("getPacket: returns ok!\n");
  return 1;
}

int usage()
{
  fprintf(stderr, "Syntax: server [<opt>] {<bps>}\n");
  fprintf(stderr, "Function. slave process interface to IVTnet\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "    -t     tell you about high level requests/replies\n");
  fprintf(stderr, "    -d     shows debug information\n");
  fprintf(stderr, "    -e     shows more debug information\n");
  fprintf(stderr, "    -v     verbose mode\n");
#ifdef NEW_931025
  fprintf(stderr, "    -n=1-3,7-12,14,15,17-22    nodes to poll");
#endif
  fprintf(stderr, "\nbps argument is one of:\n");
  fprintf(stderr, "9600\n");
  fprintf(stderr, "19200\n");
  fprintf(stderr, "38400\n");
}

dsp(node, msg)
int node;
char *msg;
{
  static char buff[80];
  time_t t1;
  
  t1 = time(0);
  strcpy(buff, ctime(&t1));
/*  sprintf(&buff[strlen(buff) - 1], "Node %d: %s\n", node, msg); */
  buff[strlen(buff) - 1] = 0;

  
  printf("%s Node %2d: %s\n", buff, node, msg);
}

static int *okvittade_A;
static int *okvittade_B;
static int *okvittade_C;
static int *okvittade_D;

static int *kvittade_A;
static int *kvittade_B;
static int *kvittade_C;
static int *kvittade_D;
static int totSize = 256;

static int *ptr2IntVar(dm, meta, name)
char *dm, *meta, *name;
{
  int id, s;
  if ((id = metaId(meta, name)) > 0) {
    if (metaType(meta, id) == TYPE_INT_VEC) {
      s = metaSize(meta, id) / 4;
      if (s < totSize)
        totSize = s;
      return (int *) metaValue(dm, meta, id);
    }
  }
  return (int *) 0;
}

#ifdef NEW_931025
static int initPollNodePtr(dm, meta)          /* Added 920708  */
char *dm;
char *meta;
{
  int id, s;
  if ((id = metaId(meta, "nodePoll")) > 0) {
    if (metaType(meta, id) == TYPE_INT_VEC) {
      pollNodeMax = metaSize(meta, id) / 4;
      return pollNodePtr = (int *) metaValue(dm, meta, id);
    }
  }
  pollNodePtr = 0;
  pollNodeMax = 0;
  return 0;
}
#endif

static int initNodeUpPtr(dm, meta)          /* Added 920708  */
char *dm;
char *meta;
{
  int id, s;
  if ((id = metaId(meta, "nodeUp")) > 0) {
    if (metaType(meta, id) == TYPE_INT_VEC) {
      nodeUpMax = metaSize(meta, id) / 4;
      return nodeUpPtr = (int *) metaValue(dm, meta, id);
    }
  }
  nodeUpPtr = 0;
  nodeUpMax = 0;
  return 0;
}

static int initAlarmPtr(dm, meta)
char *dm;
char *meta;
{
  okvittade_A = ptr2IntVar(dm, meta, "tNoOfNak_A");
  okvittade_B = ptr2IntVar(dm, meta, "tNoOfNak_B");
  okvittade_C = ptr2IntVar(dm, meta, "tNoOfNak_C");
  okvittade_D = ptr2IntVar(dm, meta, "tNoOfNak_D");

  kvittade_A = ptr2IntVar(dm, meta, "tNoOfAck_A");
  kvittade_B = ptr2IntVar(dm, meta, "tNoOfAck_B");
  kvittade_C = ptr2IntVar(dm, meta, "tNoOfAck_C");
  kvittade_D = ptr2IntVar(dm, meta, "tNoOfAck_D");
}

static int updateAlarmPtr(node)
int node;
{
  unsigned char ack[4], nak[4];
  int oA = 0, oB = 0, oC = 0, oD = 0, kA = 0, kB = 0, kC = 0, kD = 0, i;

  if (!(okvittade_A || okvittade_B || okvittade_C || okvittade_D ||
      kvittade_A || kvittade_B || kvittade_C || kvittade_D)) {
    if (dm && meta)
      initAlarmPtr(dm, meta);
    else
      return 0;
  }
  if (node >= totSize)
    return 0;           /* out of range, skip it */

  memcpy(ack, &nodeMap[node].confirmedABCD, 4); /* bugfix 930720, added '&' ! */
  memcpy(nak, &nodeMap[node].activeABCD, 4);
  if (okvittade_A) okvittade_A[node] = ack[0];
  if (okvittade_B) okvittade_B[node] = ack[1];
  if (okvittade_C) okvittade_C[node] = ack[2];
  if (okvittade_D) okvittade_D[node] = ack[3];
  if (kvittade_A)  kvittade_A[node] = nak[0];
  if (kvittade_B)  kvittade_B[node] = nak[1];
  if (kvittade_C)  kvittade_C[node] = nak[2];
  if (kvittade_D)  kvittade_D[node] = nak[3];
  for (i = 1; i < totSize; i++) {
    if (okvittade_A) oA += okvittade_A[i];
    if (okvittade_B) oB += okvittade_B[i];
    if (okvittade_C) oC += okvittade_C[i];
    if (okvittade_D) oD += okvittade_D[i];
    if (kvittade_A) kA += kvittade_A[i];
    if (kvittade_B) kB += kvittade_B[i];
    if (kvittade_C) kC += kvittade_C[i];
    if (kvittade_D) kD += kvittade_D[i];
  }
  if (okvittade_A) okvittade_A[0] = oA;
  if (okvittade_B) okvittade_B[0] = oB;
  if (okvittade_C) okvittade_C[0] = oC;
  if (okvittade_D) okvittade_D[0] = oD;
  if (kvittade_A)  kvittade_A[0] = kA;
  if (kvittade_B)  kvittade_B[0] = kB;
  if (kvittade_C)  kvittade_C[0] = kC;
  if (kvittade_D)  kvittade_D[0] = kD;
}

main(argc, argv)
int argc;
char *argv[];
{
  int slowestNode, slowNode, node, i, j, setSpeed = 9600;

#ifdef NEW_931025
  int n, n2;
  for (i = 0; i < 64; i++)
    pollNodes[i] = 0;
#endif
  
  while( argc >= 2  && argv[1][0] == '-' ) {
    while( *++(argv[1]) ) {
      switch( *argv[1] ) {
        case '1':
          onePoll = 1;
          continue;
        case 'e':
        case 'E':
          DEBUG2 = 1;
        case 'd':
        case 'D':
          DEBUG = 1;
          printf("DEBUGing in progress...\n");
          continue;
        case 'v':
        case 'V':
          verbose = 1;
          continue;
        case 'f':
        case 'F':
          failures = 1;
          continue;
        case 'q':
        case 'Q':
          quickMode = 1;
          quickSleep = (*++argv[1] - '0');
          continue;
        case 'm':
        case 'M':
          mode2 = 1;
          continue;
        case 't':
        case 'T':
          TELL = 1;
          continue;
#ifdef NEW_931025
        case 'n':
        case 'N':     /* -n=1-3,7-12,14,15,17-22  */
          if (!*++argv[1])
            continue;
          while( *++(argv[1]) ) {
            n = *argv[1]++ - '0';
            if (*argv[1] >= '0')
              n = 10*n + *argv[1]++ - '0';
            if (*argv[1] == '-') {
              argv[1]++;
              n2 = *argv[1]++ - '0';
              if (*argv[1] >= '0')
                n2 = 10*n2 + *argv[1]++ - '0';
              for (i = n; i <= n2; i++) {
                if (i > 63) {
                  usage();
                  exit(1);
                }
                if (i > pollNodeMax)
                  pollNodeMax = i;
                pollNodes[i] = 1;
              }
              if (!*argv[1]) {
                --argv[1];            /* new */
                break;
              }
              continue;
            } else {
              if (n > 63) {
                usage();
                exit(1);
              }
              if (n > pollNodeMax)
                pollNodeMax = n;
              pollNodes[n] = 1;
              if (*argv[1] == ',')
                continue;
              if (!*argv[1]) {
                --argv[1];            /* new */
                break;
              }
            }
	  }
          pollNodePtr = pollNodes;
          pollNodeMax ++;
          continue;
#endif
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
    setSpeed = baud;
  }


  sysVars->tid.dummy = 0;
  sysVars->netCommand = CMD_NO;
  sysVars->slavePid = getpid();

  intercept(icp);

  initphyio();
  initidcio(); 

/*
! create events
*/
  createEvent(&netFree, NET_EVENT_FREE, 1);
  createEvent(&netTaskAccomplished, NET_EVENT_TASK_ACCOMPLISHED, 0);
/*
!   create data module ivtnet, added 930928
*/
#ifdef NEW_930928
  if ((ivtnet = (struct _ivtnet *) modlink("IVTnet", (short) 0 /* any */)) 
            == (struct _ivtnet *) -1) {
    ivtnet = (struct _ivtnet *) 
            ((char *) 
                   _mkdata_module("IVTnet", sizeof(struct _ivtnet), 
                        (short) mkattrevs(MA_REENT, 1), 
                        (short) MP_OWNER_READ | MP_OWNER_WRITE | MP_OWNER_EXEC)
                         + sizeof(struct modhcom));
  } else {
    ivtnet = (struct _ivtnet *) (((char *) ivtnet) + sizeof(struct modhcom));
  }
  nodeMap = (struct _node *) ivtnet->nodeMap;
  map2Node = ivtnet->map2Node;
  rBuf = ivtnet->rBuf;
  sBuf = ivtnet->sBuf;
#endif
/*
!   try to link to modules, VARS, METAVAR, ALARM
*/
  if (bindModules(&dm, &meta, &aldm))
    initNodeUpPtr(dm, meta);                /* Added 920708  */

/*
struct _node nodeMap[MAX_NODE_NO];
struct _node (*nodeMapPtr)[];
*/

/*  nodeMapPtr = nodeMap;
  nodeMapPtr = &nodeMap;
*/

  for (i = 0; i < MAX_NODE_NO ; i++) {
    if (nodeUpPtr && i < nodeUpMax)           /* Added 920708  */
      nodeUpPtr[i] = 0;                     /* tell user all nodes down */
    nodeMap[i].failCount = nodeMap[i].size = 0;
    nodeMap[i].rCount = nodeMap[i].sCount = 0;
    nodeMap[i].noOfAlarms = nodeMap[i].pollCount = 0;
    nodeMap[i].failFlag = 0; nodeMap[i].size = -1;    /* all nodes down ! */
    nodeMap[i].full = 0;
    nodeMap[i].version = 0;
    nodeMap[i].activeABCD = 0;
    nodeMap[i].confirmedABCD = 0;
#ifdef NEW_930928
/*    nodeMap[i].command = nodeMap[i].error = 0;  */
#endif
    updateAlarmPtr(i);
  }
  nodeMap[0].size = 0;
  if (netopen() == -1) {
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

#define MAX_WAIT_TIME 60      /* if no reply, try in 60 sec again */
#define SEC_TO_GO     20      /* update each var no more than each 20 sec */

/*
!  these two following lines are new, 920130
!  the byte counter counts bytes in both directions
*/
  sysVars->reboots = time(0);
  byteCounter = 0;

  initStat();
  for (node = 1; node < MAX_NODE_NO; node++)
    emptyNode(node);
  while (1) {
    time_t t1;
    int nb;
    
#ifdef NEW_930928
    ivtnet->outerMostCounter ++;
#endif
    if (nodeUpPtr == 0 && bindModules(&dm, &meta, &aldm)) /* Added 920708  */
        initNodeUpPtr(dm, meta);

#ifdef NEW_931025
    if (pollNodePtr == 0 && bindModules(&dm, &meta, &aldm)) /* Added 920708  */
        initPollNodePtr(dm, meta);
#endif

    for (slowestNode = 1; slowestNode < MAX_NODE_NO; slowestNode++) {
      t1 = time(0);
#ifdef NEW_930928
    ivtnet->outerCounter ++;
#endif
      netpoll(100);
      if (!quickMode)
        fastNode_1 = fastNode_2 = 0;
      for (slowNode = 1; slowNode < MAX_NODE_NO; slowNode++) {

        t1 = time(0);     /* added 930929 ! */

/* Added 920708  */
        if (nodeUpPtr) {      /* update users variable of this nodes state */
          if (slowNode < nodeUpMax)
            nodeUpPtr[slowNode] = (nodeMap[slowNode].size == 0);
	}
	
      message = (struct _message *) sBuf;
      sampleStat(message);          /* not so often... just for test */


        updateStatistic(setSpeed, t1);

/*  printf("Slowest=%2d,slow=%2d,t=%d, ", slowestNode, slowNode, t1);/*931124*/
/*  printf("issuedCmd=%d,issuedCmdAt=%d,ev=%d\n",
	issuedCommand, issuedCommandAt, _ev_read(netTaskAccomplished));  */

        checkCommand();		/* added 931124, (you should know what happened before... if no node up -> hangman) */

        checkTimeOut(t1);
        updateOurRemotes();
        globalFlagAnyNodeFailed = 0;
        
if (failures) {       
  static char mis[64];
  int first = 1;
  for (j = 2; j <= 11; j++) {
/*    if (j == 8 || j == 10)
      continue; */
        for (i = 0; i < nextFree_inMap2Node; i++) {
          if (map2Node[i] == j)
            break;
	}
        if (i >= nextFree_inMap2Node) {
          if (mis[j])
            continue;
          if (first) {
            first = 0;
            printf("Missing nodes: ");
          }
          printf("%d, ", j);  mis[j] = 1;
        } else
          mis[j] = 0;
  }
  if (!first)
    printf("\n");
}


#ifdef NEW_931025
  if (pollNodePtr) {
    for (i = 1; i < pollNodeMax; i++) {
      if (pollNodePtr[i] == 0)
        continue;

      pollNodePtr[0] = node;

      currentNode = &nodeMap[node = i];
      if ((nb = netpoll(node)) > 0) {
        pollNodePtr[node] = nb;
        currentNode->failCount = 0; /* Added 931025 */
        if (!doNode(currentNode, node, t1)) {
          currentNode->noOfAlarms = 0; /* clear any knowledge about alarms! */
          currentNode->size = -1;     /*    */
          nodeMap[node].size = -1;
          currentNode->failCount = 0; /* nodeMap[node].failCount = 0; */
          if (failures) dsp(node, "removed from poll list (1)");
        }
      } else if (nb == 0) {
        pollNodePtr[node] = 1;
        nodeMap[0].failCount = node;
        currentNode->pollCount ++;  /* nodeMap[node].pollCount ++; */
        currentNode->failCount = 0; /* Added 930914 */
        checkSendNewTime(currentNode, node, t1);
        checkCommand();
      } else {            /* node went down ! */
        if (nodeMap[node].failCount < 50) {   /* added 920221 */
          nodeMap[node].failCount ++;
        } else {
          pollNodePtr[node] = -1;
          currentNode->noOfAlarms = 0; /* clear any knowledge about alarms! */
          currentNode->size = -1;     /* nodeMap[node].size = -1; */
          nodeMap[node].size = -1;
          currentNode->failCount = 0; /* nodeMap[node].failCount = 0; */
          if (failures) dsp(node, "removed from poll list (2)");
        }
      }
    }     /* for next node */
  } else {
#endif
      do 
        for (i = 0; i < nextFree_inMap2Node; i++) {
          if (fastNode_1) {
            currentNode = &nodeMap[node = fastNode_1];
            currentNode->pollCount ++;
            if ((nb = netpoll(node)) > 0) {
              if (!doNode(currentNode, node, t1)) {
                ; /* let'em take care of this */
	      }
	    }
          }
          if (fastNode_2) {
            currentNode = &nodeMap[node = fastNode_2];
            currentNode->pollCount ++;
            if ((nb = netpoll(node)) > 0) {
              if (!doNode(currentNode, node, t1)) {
                ; /* let'em take care of this */
	      }
	    }
          }
          if (quickMode) {
            if (fastNode_1 || fastNode_2)
              continue;
          }
          
          currentNode = &nodeMap[node = map2Node[i]];
          if ((nb = netpoll(node)) > 0) {

            currentNode->failCount = 0; /* Added 931025 */

            if (!doNode(currentNode, node, t1)) {
              currentNode->noOfAlarms = 0; /* clear any knowledge about alarms! */
              currentNode->size = -1;     /*    */

              nodeMap[map2Node[i]].size = -1;

              currentNode->failCount = 0; /* nodeMap[node].failCount = 0; */
              nextFree_inMap2Node--;

if (failures) {       
              dsp(node, "removed from poll list (1)");
/*
                printf("1.Remove node %d from poll list: map2Node[%d]=%d, ",
                                               node, i, map2Node[i]);
                printf("nodeMap[%d].size=%d\n", node, nodeMap[node].size);
*/                
}
              map2Node[i] = map2Node[nextFree_inMap2Node];
              map2Node[nextFree_inMap2Node] = 0;
            }
          } else if (nb == 0) {
            nodeMap[0].failCount = node;
            currentNode->pollCount ++;  /* nodeMap[node].pollCount ++; */

            currentNode->failCount = 0; /* Added 930914 */
          
            checkSendNewTime(currentNode, node, t1);
            checkCommand();
          } else {            /* node went down ! */
            if (nodeMap[map2Node[i]].failCount < 50) {   /* added 920221 */
              nodeMap[map2Node[i]].failCount ++;
	    } else {
              currentNode->noOfAlarms = 0; /* clear any knowledge about alarms! */
              currentNode->size = -1;     /* nodeMap[node].size = -1; */

              nodeMap[map2Node[i]].size = -1;

              currentNode->failCount = 0; /* nodeMap[node].failCount = 0; */
              nextFree_inMap2Node--;

if (failures) {       
              dsp(node, "removed from poll list (2)");
/*
                printf("2.Remove node %d from poll list: map2Node[%d]=%d, ",
                                               node, i, map2Node[i]);
                printf("nodeMap[%d].size=%d\n", node, nodeMap[node].size);
*/
}
              map2Node[i] = map2Node[nextFree_inMap2Node];
              map2Node[nextFree_inMap2Node] = 0;
            }
          }
        }
        while (quickMode && (fastNode_1 || fastNode_2));

#ifdef NEW_931025
  }   /* end if (pollNodePtr) { */
#endif
        
        if (globalFlagAnyNodeFailed)
          nodeMap[0].failFlag = 1;

        if (nodeMap[slowNode].size > 0)       /* a try for success ... 920226 */
          nodeMap[slowNode].size = -1;


        if (nodeMap[slowNode].size == 0) {                  /* another try ... 920302 */
          for (i = 0; i < nextFree_inMap2Node; i++)
            if (slowNode == map2Node[i])
              break;
          if (i >= nextFree_inMap2Node) {
            /* not found !!! */
            if (failures) {
              dsp(slowNode, "not found in poll-list but has size 0 !!(ins)");
              printf("   nextFree = %d\n", nextFree_inMap2Node);
            }
            map2Node[nextFree_inMap2Node++] = slowNode;
            map2Node[nextFree_inMap2Node] = 0;
          }
        }


        if ((nodeMap[slowNode].size < 0) && (nodeMap[slowNode].failCount < 20)) {
          if (netpoll(slowNode) >= 0) {
            currentNode = &nodeMap[slowNode];
            currentNode->failCount = 0;
/*            nodeMap[slowNode].failCount = 0;    */
            if (doNode(currentNode, slowNode, t1)) {       /* update .size */
              map2Node[nextFree_inMap2Node++] = slowNode;
              map2Node[nextFree_inMap2Node] = 0;
              if (failures) dsp(slowNode, "is up ! (1)");
            }
          } else {
            nodeMap[slowNode].failCount ++;
          }
        }
      }
      if (nodeMap[slowestNode].size < 0) {
        if (netpoll(slowestNode) >= 0) {
          currentNode = &nodeMap[slowestNode];
          currentNode->failCount = 0;
/*          nodeMap[slowestNode].failCount = 0;   */
          if (doNode(currentNode, slowestNode, t1)) {       /* update .size */
            map2Node[nextFree_inMap2Node++] = slowestNode;
            map2Node[nextFree_inMap2Node] = 0;
            if (failures) dsp(slowestNode, "is up ! (2)");
          }
        } else {
          nodeMap[slowestNode].failCount ++;
        }
      }
    } /* for slowestNode */
  }   /* for ever */
}

doNode(currentNode, node, t1)
struct _node *currentNode;
int node;
long t1;
{
  nodeMap[0].failCount = node;
/*  currentNode = &nodeMap[node];   */
  if (!checkNodeStatus(currentNode, node, 0)) {
    checkCommand();
    return 0;     /* continue; */
  }
  if (currentNode->size == 0) {
    checkSendNewTime(currentNode, node, t1);
    checkCommand();
    return 1;     /* continue; */
  }
  byteCounter += fetchNodeData(node);
  if (verbose) printf("Node %d has %d bytes of data\n", node, bufPoll(node));
  while (currentNode->size = bufPoll(node)) {
    if (!getPacket(currentNode, node, rBuf))
      return 0;
    packet = (struct _packet *) rBuf;
    if (DEBUG)
      dumpPacket(currentNode, node, rBuf);
    if (TELL)
      tellPacket(currentNode, node, rBuf);
#ifdef NEW_930928
    nodeMap[node].trans.command = packet->data[0];
    nodeMap[node].trans.toNode = packet->header.targetNode;
    nodeMap[node].trans.error = 0;
    nodeMap[node].transTid = t1;

    nodeMap[packet->header.targetNode].rec.command = packet->data[0];
    nodeMap[packet->header.targetNode].rec.fromNode = node;
    nodeMap[packet->header.targetNode].rec.error = packet->data[1];
    nodeMap[packet->header.targetNode].recTid = t1;
#endif
    if (allBlockChs(rBuf))                  /* should be zero */
    {
#ifdef NEW_930928
      nodeMap[node].trans.error = NET_WRONG_CRC;
#endif
      if (DEBUG || TELL) printf("wrong checksum !\n");
      continue;   /* re-inserted 920511 */
    }
    if (packet->header.targetNode == 0)
    {
      if ((packet->data[0] & REGISTRATE) && !(packet->data[0] & REQUEST_MASK)) {
        if (packet->data[1] != NET_NOSUCHVAR)       /* added 920623 */
          distributePacket(packet);
      } else {
        layer3(packet);
        nodeMap[packet->header.targetNode].sCount ++;
      }
    } else {
      int nb, target;
         
      target = packet->header.targetNode;
/*
      if (nodeMap[target].size < 0)
            nodeMap[target].size = myNetpoll(target);
*/
      if (nodeMap[target].size >= 0) {
        if (netrdy(target)) {
          nb = netwrite(target, (char *) packet, packet->header.size);
          netflush();
          byteCounter += packet->header.size;
          nodeMap[target].sCount ++;
          nodeMap[target].full = 0;
        } else
          nodeMap[target].full = 1;
      }
      if (DEBUG2)
            printf("Netwrite = %d (%d) to node %d\n", nb, packet->header.size,
                                        packet->header.targetNode);
    }   /* if (!route-ing) ... else ... endif */
  }   /* while (this-node-has-any-data) ... */

  checkSendNewTime(currentNode, node, t1);
  checkCommand();
  return 1;
}      

updateOurRemotes()
{
  int id;

  if (bindModules(&dm, &meta, &aldm)) {
    if (id = checkNextRemote()) {
      long now, node, size;
      struct _remote { long timeStamp; } *remotePtr;
              
      remotePtr = (struct _remote *) metaRemoteData(dm, meta, id);
      now = getRelTime();
/*      if (abs(now - remotePtr->timeStamp) > MAX_WAIT_TIME)    */
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
/*
!	Now, check all registrated vars and send replies to those nodes
!	who have subscribed for these variables.
*/
    {
      int size;
      
      if (id = sendNextRegistrated(sBuf, &size)) {
        if (MASTER) {
          struct _packet packet;  /* pack it */
          packet.header.size = sizeof(struct _header) + size + sizeof(char);
          packet.header.version = 1;
          packet.header.targetMaster = 0;
          packet.header.targetNode = 0;
          packet.header.sourceMaster = 0;
          packet.header.sourceNode = 0;
          packet.header.command =  0;        /* data block */
          memcpy(packet.data, sBuf, size);
          packet.data[size] = 0;
          distributePacket(&packet);
	} else {
          sendPacket(0, sBuf, size);
	}
      }
    }
  }
}

checkTimeOut(t1)
time_t t1;
{
  if (issuedCommand != CMD_NO) {
    if ((t1 - issuedCommandAt) > 20) {
      if (issuedCommand == CMD_GETALARM_TEXT)
        nodeMap[issuedCommandToNode].noOfAlarms = 0;
      issuedCommand = CMD_NO;
      issuedCommandAt = 0;
        /* set time out error flag */
      *netError = NET_TIMEOUT;
      taskAccomplished();
    }
  }
}
/*
!   A few lines for statistic purpose only  
*/ 
updateStatistic(setSpeed, t1)
int setSpeed;
time_t t1;
{
  float elapsedTime, transportTime;
  
  nodeMap[0].size += (byteCounter >> 10);
  nodeMap[0].noOfAlarms = byteCounter &= 0x003ff;
  if (!(elapsedTime = (float) (t1 - sysVars->reboots)))
    elapsedTime++;
  transportTime = (float) 10240.0 * nodeMap[0].size / setSpeed;
  transportTime += (float) (10.0 * byteCounter / setSpeed);
  nodeMap[0].pollCount = 100.0 * transportTime / elapsedTime;
}

#define DISTR_PACKET_SIZE 8       /* 32 * 8 = 256, 8 * 8 = 64 */

distributePacket(packet)
struct _packet *packet;
{
  unsigned char *d, distr[32];
  int pos, node, i, j, sh, nb;

  if (DEBUG) printf("distributePacket:\n");
  pos = 2 + strlen(&packet->data[2]) + 1;
  if (packet->data[pos] == TYPE_INT)
    pos += 5;
  else
    pos += 9;

  memcpy(distr, &packet->data[pos], 32);

  packet->header.size = sizeof(struct _header) + pos + sizeof(char);
  packet->data[0] &= ~REGISTRATE;

  packet->data[1] = NET_REGISTRATE;     /* Added 920612 !!! */

  d = &distr[0];
  if (DEBUG) printf("*d = %02x\n", *d);
  for (i = 0, node = 0; i < DISTR_PACKET_SIZE; i++, d++) {
    if (*d == 0) {
	node += 8;
	continue;
    }
    for (j = 0, sh = 1; j < 8; j++, sh <<= 1, node ++) {
      if (DEBUG) printf("node = %d, j = %d, sh = %d, *d = %02x\n", 
                                        node, j, sh, *d);
      if (*d & sh) {
        packet->header.targetNode = node;
	packet->data[pos] = 0;
	packet->data[pos] = allBlockChs(packet);

        if (DEBUG) {
          unsigned char *p;
          int len, i;
          p = (unsigned char *) packet;
          len = *p;
          printf("Distribute to node %d\n", node);
          for (i = 0; i < len; i++)
            printf("%02x, ", p[i]);
          printf("\n");
        } 

        if (node == 0)    /* master */
        {
          layer3(packet);
          nodeMap[packet->header.targetNode].sCount ++;
	} else {
          if (nodeMap[node].size >= 0) {
            if (netrdy(node)) {
              nb = netwrite(node, packet, packet->header.size);
              netflush();
              byteCounter += packet->header.size;
              nodeMap[node].sCount ++;
              nodeMap[node].full = 0;
            } else
              nodeMap[node].full = 1;
          }
        }     /* end of if slave/master */
      }     /* end of if (this Node  */
    }   /* end of bit-loop */
  } /* end of byte-loop */
}


int checkSendNewTime(currentNode, node, tid)
struct _node *currentNode;
int node;
long tid;
{
  if (abs(tid - sendTime[node]) > 600)
  {
/*
    char sBuf[256];                 remove 920130
*/
    
    message = (struct _message *) sBuf;
    message->message_type = SET_TIME | REQUEST_MASK;
    message->error = 0;
    getime(&message->mix.setTime.tm);
    if (bindModules(&dm, &meta, &aldm)) {
      makeAlarmMask(dm, meta, message->mix.setTime.abcdMask);
    }
    sendPacket(node, sBuf, 12);     /* ??? 13 -> 12, 920210  */
    sendTime[node] = tid;
  }
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

int layer3(packet)
struct _packet *packet;
{
  int rSize, toNode;
  char rBuf[256];
  
  unfoldPacket(packet, rBuf, &rSize);           /* unpack it */

  toNode = packet->header.sourceNode;
  message = (struct _message *) rBuf;

  if (message->message_type & REQUEST_MASK)
    processRequest(message, toNode, rBuf);
  else
    processReply(message, toNode);
/* message will be discarded as local rBuf will be removed from stack */    
}

processRequest(message, toNode, rBuf)
struct _message *message;
int toNode;   /* from node ! */
char *rBuf;
{      
  int size, id;
  
  size = 0;
  message->message_type &= ~REQUEST_MASK;
  switch (message->message_type & ~REGISTRATE) {
    case GET_VAR:
      if (bindModules(&dm, &meta, &aldm)) /* ok ! */
      {
        if ((id = metaId(meta, message->mix.varRequest.varName)) < 0) 
        {
                message->error = NET_NOSUCHVAR;
                size = 2;
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
          }
          else if (metaType(meta, id) == TYPE_FLOAT) {
                  message->mix.varRequest.varName[pos] = TYPE_FLOAT;
                  memcpy(&message->mix.varRequest.varName[pos + 1], 
                              metaValue(dm, meta, id), sizeof(double));
                  size += sizeof(double);
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
      if (bindModules(&dm, &meta, &aldm)) /* ok ! */
      {
        if ((id = metaId(meta, message->mix.varRequest.varName)) < 0) 
        {
          message->error = NET_NOSUCHVAR;
        } else {
          int type, pos;
          long iValue;
          double dValue;
                
          pos = strlen(message->mix.varRequest.varName) + 1;
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
          } else {
                  if (DEBUG) printf("GET_VAR reply: illegal type\n");
                  message->error = NET_ILLEGALTYPE;
          }
	}
      }
      size = 0;       /* for now, no reply !! */
      break;
    case ALARM_NO:    /* slave informs us about number of alarms she has */
      {
        unsigned char abcd[4];
        unsigned short int n;
        memcpy(abcd, &message->mix.alarmNoRequest.noOfAlarms, 4);
        if (abcd[0] > 3) abcd[0] = 3;
        if (abcd[1] > 3) abcd[1] = 3;
        if (abcd[2] > 3) abcd[2] = 3;
        if (abcd[3] > 3) abcd[3] = 3;
        n = (abcd[0] << 6) |
            (abcd[1] << 4)  |
            (abcd[2] << 2)  |
            (abcd[3]);
        n |= (message->mix.alarmNoRequest.pcSum << 8);
        nodeMap[toNode].noOfAlarms = n;
/*
        if (abcd[0] > 15) abcd[0] = 15;
        if (abcd[1] > 15) abcd[1] = 15;
        if (abcd[2] > 15) abcd[2] = 15;
        if (abcd[3] > 15) abcd[3] = 15;
        n = (abcd[0] << 12) |
            (abcd[1] << 8)  |
            (abcd[2] << 4)  |
            (abcd[3]);
        nodeMap[toNode].noOfAlarms = n;
*/
      }
/*
      nodeMap[toNode].noOfAlarms = message->mix.alarmNoRequest.noOfAlarms;
*/
      nodeMap[toNode].activeABCD = message->mix.alarmNoRequest.activeABCD;
      nodeMap[toNode].confirmedABCD = message->mix.alarmNoRequest.confirmedABCD;
      updateAlarmPtr(toNode);
      size = 0;
      break;
    case ALARM_TEXT_1:   /* no need for this, we are the master */
    case ALARM_TEXT_2:
    case CONFIRM_ALARM:
    case SET_TIME:
      break;
    case SET_HOST:
                    /* they want us to;
                                        - spawn a new screen process
                                          with our buffer as context
                                        - reply with yes/no
                      */
            /* spawn process */
            message->mix.setHost.ok = spawnScreen();
            size = 3;
            break;
    case KEY_DISPLAY:
                   /* got a new key snapshot from ! */
                   /* return our display and led snapshot ! */
            if (message->error == NET_NOERROR)
              captureDisplay(message);
            else if (message->error = NET_CANCEL) {
              kill(screenContext->spawnedScreenPid, SIGINT);
            }
            break;
    case VERSION:
      message->mix.version[5] = 0;
      nodeMap[toNode].version = ((message->mix.version[0] - '0') << 10) | 
                      (atoi(&(message->mix.version[2])));
      size = 0;
      break;
    default:
      size = 0;
      break;
  }
  if (size)
    sendPacket(toNode, rBuf, size);   /* obs ! rBuf sent !! */
}

processReply(message, toNode)
struct _message *message;
int toNode;
{
  int id, size, n, i, j;
  char *buff;

  switch (message->message_type) {
    case GET_VAR:
      
/* check message->error ! */
          
            if (bindModules(&dm, &meta, &aldm)) /* ok ! */
            {
              if ((id = metaRemoteId(meta, 
                              message->mix.varRequest.varName, toNode)) < 0) 
/*              if ((id = metaId(meta, message->mix.varRequest.varName)) < 0) */
              {
                if (DEBUG) printf("GET_VAR reply: no such var \n");
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
#else
                  remotePtr->timeStamp = 0;    /* force a new update NOW ! */
#endif
                }
              }

            } else if (DEBUG)
              printf("get_var reply: cannot bind data modules\n");
            if (DEBUG)
              printf("TaskAccomplished, get var done\n");
/*            taskAccomplished();   */


            break;
    case PUT_VAR:
            break;
    case ALARM_NO:
/*
      sysVars->varId = message->mix.alarmNoRequest.noOfAlarms;
      taskAccomplished();
*/
      break;
    case ALARM_TEXT_1:
      if (message->error == NET_NOMOREALARMS) {
/*        nodeMap[toNode].noOfAlarms = 0;   */
        nodeMap[toNode].noOfAlarms &= ~(0x0100 << issuedCommandFromPc);
        message->error = 0;     /* this was just a warning ! */
      } else if (message->error == NET_NOALARMS) {
        nodeMap[toNode].noOfAlarms &= ~(0x0100 << issuedCommandFromPc);
      }
      *netError = message->error;
      if (!message->error) {
        size = 10 + strlen(message->mix.alarmTextRequest.text) + 1; 
        if (size > ALARM_TEXT_SIZE)
          size = ALARM_TEXT_SIZE;
        memcpy(sysVars->netAPIarea, &message->mix.alarmTextRequest, size);
      }
      issuedCommand = CMD_NO;
      issuedCommandAt = 0;
      taskAccomplished();
      break;
    case ALARM_TEXT_2:      /* no reply on this ! */
      break;
    case CONFIRM_ALARM:
      *netError = message->error;
      issuedCommand = CMD_NO;
      issuedCommandAt = 0;
      taskAccomplished();
      break;
    case SET_TIME:        /* no reply on this ! */
      break;
    case SET_HOST:                /* do something */
            if (message->mix.setHost.ok == 0)
            {
              setHost_active = 1;
/*
              setHost_recTime = 0;
              readLexicon();
*/
              sysVars->currentBitsB = 0;
              sysVars->flashBitsB = 0;
              sysVars->flashBits2B = 0;
              sysVars->useSetB = 1;     /* ok, started another set */
            } else {
/*
              taskAccomplished();
              *netError = message->mix.setHost.ok;
*/
              sysVars->nodeId = message->error;   /* WILL NOT WORK !! 
                                  CONFLICT WITH OTHER PROCESSES ??? or... */
            }
            break;
    case KEY_DISPLAY:     /* got a new display context, show it ! */
            showDisplayContext(message);
            break;
    case GETNIDX_VAR_1:
    case GETNIDX_VAR_2:
      *netError = message->error;
      if (message->message_type == GETNIDX_VAR_1)
        fastNode_1 = 0;
      else 
        fastNode_2 = 0;
      if (localArrPtr && (issuedCommand == CMD_GET_N_IDX_VAR)) {
        n = message->mix.getNIdxVar.n;
        if (n > 10)
          n = 0;
        buff = message->mix.getNIdxVar.buff;
        for (i = j = 0; i < n; i++) {      /* memcpy since not aligned ... */
          double v;
          long iv;
          memcpy(&localArrPtr[i].idx, &buff[j], sizeof(short int));  /* if neg..*/
          if (buff[j + 2] == TYPE_INT) {
            memcpy(&iv, &buff[j + 3], sizeof(long));
            v = iv;
            j += 7;
          } else {
            memcpy(&v, &buff[j + 3], sizeof(double));
            j += 11;
          }
          localArrPtr[i].value = v;
        }
      }
      issuedCommand = CMD_NO;
      issuedCommandAt = 0;
      localArrPtr = 0;
      taskAccomplished();
      if (quickMode && quickSleep)
        tsleep(quickSleep);       /* sleep 100ms */
      break;
    case GETIDX_VAR_1:
    case GETIDX_VAR_2:
/* copy type and long/double value */    
      *netError = message->error;
      memcpy(sysVars->netAPIarea, message->mix.getIdxVar.name, 11);
      issuedCommand = CMD_NO;
      issuedCommandAt = 0;
      taskAccomplished();
      break;
    case GET_UPDATED_1:
    case GET_UPDATED_2:
      *netError = message->error;
      memcpy(sysVars->netAPIarea, message->mix.getUpdated,
                                                     ALARM_TEXT_SIZE);
      issuedCommand = CMD_NO;
      issuedCommandAt = 0;
      taskAccomplished();
      break;
    case GET_VARIDX:
      *netError = message->error;
      memcpy(sysVars->netAPIarea, message->mix.getVarIdx.name, 2);
      taskAccomplished();
      issuedCommand = CMD_NO;
      issuedCommandAt = 0;
      break;
    case GET_CAL:      /* unpack bit-pack left for client */
      *netError = message->error;
      if (calendarIdx && (issuedCommand == CMD_GET_CAL)) {
        *calendarIdx = unpackIdxCalendar(message->mix.cal.bits.bitPack);
        unpackCalendar(message->mix.cal.bits.bitPack, calendarPtr);
      }
      if (fastNode_1 == toNode)
        fastNode_1 = 0;
      else if (fastNode_2 == toNode)
        fastNode_2 = 0;
      taskAccomplished();
      issuedCommand = CMD_NO;
      issuedCommandAt = 0;
      calendarIdx = 0;
      break;
    case SET_CAL:
      *netError = message->error;
      taskAccomplished();
      if (fastNode_1 == toNode)
        fastNode_1 = 0;
      else if (fastNode_2 == toNode)
        fastNode_2 = 0;
      issuedCommand = CMD_NO;
      issuedCommandAt = 0;
      break;
    case CLEAR_ALARM:
      *netError = message->error;
      taskAccomplished();
      issuedCommand = CMD_NO;
      issuedCommandAt = 0;
      break;
    case NEW:
      *netError = message->error;
      taskAccomplished();
      issuedCommand = CMD_NO;
      issuedCommandAt = 0;
      break;
    case GET_STAT_VAR:
      if (message->error == NET_NOERROR)
      {
        float value;
        long ivalue;
        double dvalue;
        if (message->mix.stat.varName[0] == TYPE_INT) {
          memcpy(&ivalue, &message->mix.stat.varName[1], 4);
          value = ivalue;
        } else {
	  extern float Float();
          memcpy(&dvalue, &message->mix.stat.varName[1], 8);
          value = Float(dvalue);
        }
        recStat(message->mix.stat.buffS, message->mix.stat.item, value);
      }
      break;
    case GET_MEM:         /* reply is block with data */
      *netError = message->error;
      if (getMemPtr)
        memcpy(getMemPtr, &message->mix.mem.buff[0], message->mix.mem.size);
/*
      if (fastNode_1 == toNode)
        fastNode_1 = 0;
      else if (fastNode_2 == toNode)
        fastNode_2 = 0;
*/
      taskAccomplished();
      issuedCommand = CMD_NO;
      issuedCommandAt = 0;
      getMemPtr = 0;
      break;
    case PUT_MEM:         /* reply is ok */
      *netError = message->error;
      taskAccomplished();
      issuedCommand = CMD_NO;
      issuedCommandAt = 0;
      break;
    case LOAD_PROGRAM:    /* reply is ok */
      *netError = message->error;
      taskAccomplished();
      issuedCommand = CMD_NO;
      issuedCommandAt = 0;
      break;
    case REBOOT:          /* no reply ! slave is down !!! */
      break;
    default:
      break;
  }
  status = _idle;           /* ??? one for send/receive ??? */
}

int checkCommand()
{
  int id, toNode, size, pos, typ, i, j;
  char *value, *buff;
  


  if (sysVars->netCommand == CMD_NO)
    return 0;

        size = 0;
        id = sysVars->varId;
        toNode = sysVars->nodeId;

/*  printf("checkCommand: toNode=%d,id=%d, cmd=%d\n", toNode, id, 
		sysVars->netCommand);		*/

        if (nodeMap[toNode].size < 0) {

          /* should not try ! */

/*	printf("checkCommand: taskAccomplished...\n"); /*931124*/



            *netError = NET_TIMEOUT;
            sysVars->netCommand = CMD_NO;
            taskAccomplished();
            return 0;
        }
     
        message = (struct _message *) sBuf;
        
  switch (sysVars->netCommand) {
    case CMD_GETVAR:
            if (bindModules(&dm, &meta, &aldm)) /* ok ! */
            {
              long now;
              struct _remote { long timeStamp; } *remotePtr;
              
              remotePtr = (struct _remote *) metaRemoteData(dm, meta, id);
              now = getRelTime();    /* time(0); */
                           
              if (abs(now - remotePtr->timeStamp) < 60) {
                size = 0;
              } else {
                remotePtr->timeStamp = now;
                strcpy(message->mix.varRequest.varName, 
                                  (char *) metaName(meta, id));
                message->message_type = GET_VAR | REQUEST_MASK;
                message->error = 0;
                size = 2 + strlen(message->mix.varRequest.varName) + 1;
/*                size = 34;    */
              }
              sysVars->netCommand = CMD_NO;
            } else
              if (DEBUG) printf("cannot bind data modules !\n");
            *netError = NET_NOERROR;
            taskAccomplished();
            break;
    case CMD_PUTVAR:
            if (bindModules(&dm, &meta, &aldm)) /* ok ! */
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
            *netError = NET_NOERROR;
            taskAccomplished();
            break;
    case CMD_SETHOST:
            message->message_type = SET_HOST | REQUEST_MASK;
            setHostNode = sysVars->nodeId;
            size = 3; 
            sysVars->netCommand = CMD_NO;
            break;
    case CMD_ALARMS:
/*
      memcpy(&sysVars->varId, &nodeMap[sysVars->nodeId].noOfAlarms, 2);
*/
      sysVars->varId = nodeMap[sysVars->nodeId].noOfAlarms;
      sysVars->netCommand = CMD_NO;
      *netError = NET_NOERROR;
      taskAccomplished();
      size = 0;
/*      message->message_type = ALARM_NO | REQUEST_MASK;
      message->error = 0;
      size = 3; */
      break;
    case CMD_GETALARM_TEXT:
      message->message_type = ALARM_TEXT_1 | REQUEST_MASK;
      message->error = 0;
      size = 8;                     /* bugfix 920210,  4 -> 8 */
      
/*  used to tell what PC */
      message->mix.alarmTextRequest.alarmNo = id;     /* this is PCno !! */
/* place abcd-mask in serialNo and status fields, i.e. 2+2=4bytes */
      makeAlarmMask(dm, meta, &message->mix.alarmTextRequest.serialNo);
      issuedCommand = sysVars->netCommand;
      issuedCommandAt = time(0);
      issuedCommandToNode = toNode;
      issuedCommandFromPc = id;           /* PCno 0 - 7 */
      sysVars->netCommand = CMD_NO;
      break;
    case CMD_ACK_ALARM:
      message->message_type = ALARM_TEXT_2 | REQUEST_MASK;
      message->error = 0;
      message->mix.ackAlarm.PCno = id;  /* PCno */
      memcpy(message->mix.ackAlarm.abcdMask, sysVars->netAPIarea, 4);
      size = 7;
      sysVars->netCommand = CMD_NO;
      *netError = NET_NOERROR;
      taskAccomplished();
      break;
    case CMD_CONFIRM_ALARM:
      message->message_type = CONFIRM_ALARM | REQUEST_MASK;
      message->error = 0;
      message->mix.confirmAlarm.alarmNo = ((short int *) sysVars->netAPIarea)[0];
      message->mix.confirmAlarm.serialNo = ((short int *) sysVars->netAPIarea)[1];
      message->mix.confirmAlarm.status = ((short int *) sysVars->netAPIarea)[2];
      size = 9;
      issuedCommand = sysVars->netCommand;
      issuedCommandAt = time(0);
      sysVars->netCommand = CMD_NO;
      break;
    case CMD_GET_N_IDX_VAR:
      if (id & 0xff00) {
        message->message_type = GETNIDX_VAR_1 | REQUEST_MASK;
        fastNode_1 = toNode;
      } else {
        message->message_type = GETNIDX_VAR_2 | REQUEST_MASK;
        fastNode_2 = toNode;
      }
      message->error = 0;
      message->mix.getNIdxVar.n = id;
/*
      char *buff;
*/
      localArrPtr = *((struct _method1Arr **) sysVars->netAPIarea);
      buff = message->mix.getNIdxVar.buff;
      id &= 0x00ff;
      for (i = j = 0; i < id; i++) {      /* memcpy since not aligned ... */
        memcpy((short int *) &buff[j], &localArrPtr[i].idx, sizeof(short int));
        j += 2;
        strcpy(&buff[j], localArrPtr[i].name);
        j += strlen(localArrPtr[i].name) + 1;
      }
      size = 3 + j;
      issuedCommand = sysVars->netCommand;
      issuedCommandAt = time(0);
      sysVars->netCommand = CMD_NO;
      break;
    case CMD_GETIDX_VAR:
      if (id & 0xff00)
        message->message_type = GETIDX_VAR_1 | REQUEST_MASK;
      else
        message->message_type = GETIDX_VAR_2 | REQUEST_MASK;
      message->error = 0;
      message->mix.getIdxVar.PCidx = ((short int *) sysVars->netAPIarea)[0];
      strcpy(message->mix.getIdxVar.name, 
                (char *) &((short int *) sysVars->netAPIarea)[1]);
      size = 2 + 2 + strlen(message->mix.getIdxVar.name) + 1;
      issuedCommand = sysVars->netCommand;
      issuedCommandAt = time(0);
      sysVars->netCommand = CMD_NO;
      break;
    case CMD_PUTIDX_VAR:
      message->message_type = PUT_VAR | REQUEST_MASK;
      message->error = 0;
      value = (char *) &sysVars->netAPIarea[0];
      strcpy(message->mix.varRequest.varName, &sysVars->netAPIarea[9]);
      typ = sysVars->netAPIarea[8];
      pos = strlen(message->mix.varRequest.varName) + 1;
      size = 2 + pos + 1;
      if (typ == TYPE_INT) {
        message->mix.varRequest.varName[pos] = TYPE_INT;
        memcpy(&message->mix.varRequest.varName[pos + 1], value, sizeof(long));
        size += sizeof(long);
      } else if (typ == TYPE_FLOAT) {
        message->mix.varRequest.varName[pos] = TYPE_FLOAT;
        memcpy(&message->mix.varRequest.varName[pos + 1],value, sizeof(double));
        size += sizeof(double);
      } else {
        message->error = NET_ILLEGALTYPE;
        size = 0;     /* MUST FLAG ERROR TO HOST PROCESS !!!! */
      }
      sysVars->netCommand = CMD_NO;
      *netError = NET_NOERROR;
      taskAccomplished();
      break;
    case CMD_GET_UPDATED:
      if (id & 0xff00) {
        fastNode_1 = 0;
        message->message_type = GET_UPDATED_1 | REQUEST_MASK;
      } else {
        fastNode_2 = 0;
        message->message_type = GET_UPDATED_2 | REQUEST_MASK;
      }
      message->error = 0;
      size = 2;
      issuedCommand = sysVars->netCommand;
      issuedCommandAt = time(0);
      sysVars->netCommand = CMD_NO;
      break;
    case CMD_GET_VARIDX:
      message->message_type = GET_VARIDX | REQUEST_MASK;
      message->error = 0;
      strcpy(message->mix.getVarIdx.name, sysVars->netAPIarea);
      size = 2 + strlen(message->mix.getVarIdx.name) + 1;
      issuedCommand = sysVars->netCommand;
      issuedCommandAt = time(0);
      sysVars->netCommand = CMD_NO;
      break;
    case CMD_GET_CAL:
      if ((fastNode_1 == 0) && (fastNode_2 != toNode))
        fastNode_1 = toNode;
      else if ((fastNode_2 == 0) && (fastNode_1 != toNode))
        fastNode_2 = toNode;
      message->message_type = GET_CAL | REQUEST_MASK;
      message->error = 0;
      strcpy(message->mix.cal.name, &sysVars->netAPIarea[8]);
      calendarPtr = (char *) (((long *) sysVars->netAPIarea)[0]);
      calendarIdx = (long *) (((long *) sysVars->netAPIarea)[1]);
      size = 2 + strlen(message->mix.cal.name) + 1;
      issuedCommand = sysVars->netCommand;
      issuedCommandAt = time(0);
      sysVars->netCommand = CMD_NO;
      break;
    case CMD_SET_CAL:
      message->message_type = SET_CAL | REQUEST_MASK;
      message->error = 0;
      if ((fastNode_1 == 0) && (fastNode_2 != toNode))
        fastNode_1 = toNode;
      else if ((fastNode_2 == 0) && (fastNode_1 != toNode))
        fastNode_2 = toNode;
      
      size = 2 + packCalendar(message->mix.cal.bits.bitPack,
                               ((short *) &((long *) sysVars->netAPIarea)[1])[0],
                                ((long *) sysVars->netAPIarea)[0]);
/*
    sysVars->netAPIarea[0] = packCalendar(&sysVars->netAPIarea[1], idx, calendar);
      memcpy(message->mix.cal.bits.bitPack, &sysVars->netAPIarea[1], 
                                                    sysVars->netAPIarea[0]);
      size = 2 + sysVars->netAPIarea[0];
*/
      issuedCommand = sysVars->netCommand;
      issuedCommandAt = time(0);
      sysVars->netCommand = CMD_NO;
      break;
    case CMD_CLEAR_ALARM:
      message->message_type = CLEAR_ALARM | REQUEST_MASK;
      message->error = 0;
      if (nodeMap[toNode].size >= 0) {
        if (netrdy(toNode)) {
          size = 2;
          issuedCommand = sysVars->netCommand;
          issuedCommandAt = time(0);
        }
      }
      sysVars->netCommand = CMD_NO;
      if (size == 0) {
        *netError = NET_NOSUCHNODE;
        taskAccomplished();
      }
      break;
    case CMD_NEW:
      message->message_type = NEW | REQUEST_MASK;
      message->error = 0;
      if (nodeMap[toNode].size >= 0) {
        if (netrdy(toNode)) {
          size = 2;
          issuedCommand = sysVars->netCommand;
          issuedCommandAt = time(0);
        }
      }
      sysVars->netCommand = CMD_NO;
      if (size == 0) {
        *netError = NET_NOSUCHNODE;
        taskAccomplished();
      }
      break;
    case CMD_ALLOCATE_MEM:
      size = 0;
      sysVars->netCommand = CMD_NO;
      break;
    case CMD_GET_MEM:
      message->message_type = GET_MEM | REQUEST_MASK;
      message->error = 0;
      if ((fastNode_1 == 0) && (fastNode_2 != toNode))
        fastNode_1 = toNode;
      else if ((fastNode_2 == 0) && (fastNode_1 != toNode))
        fastNode_2 = toNode;
      message->mix.mem.address = ((long *) sysVars->netAPIarea)[0];
      message->mix.mem.size    = ((long *) sysVars->netAPIarea)[1];
      getMemPtr                = (char *) ((long *) sysVars->netAPIarea)[2];
      size = 2 + 4 + 1;
      issuedCommand = sysVars->netCommand;
      issuedCommandAt = time(0);
      sysVars->netCommand = CMD_NO;
      break;
    case CMD_PUT_MEM:
      message->message_type = PUT_MEM | REQUEST_MASK;
      message->error = 0;
      if ((fastNode_1 == 0) && (fastNode_2 != toNode))
        fastNode_1 = toNode;
      else if ((fastNode_2 == 0) && (fastNode_1 != toNode))
        fastNode_2 = toNode;
/*
!   the api struct is 
  struct { long address; long size; char *sourcePtr };
*/
      message->mix.mem.address = ((long *) sysVars->netAPIarea)[0];
      message->mix.mem.size    = ((long *) sysVars->netAPIarea)[1];
      memcpy(message->mix.mem.buff, 
                ((long *) sysVars->netAPIarea)[2],
                message->mix.mem.size);
      size = 2 + 4 + 1 + message->mix.mem.size;
      issuedCommand = sysVars->netCommand;
      issuedCommandAt = time(0);
      sysVars->netCommand = CMD_NO;
      break;
    case CMD_LOAD_PROGRAM:
      size = 0;
      sysVars->netCommand = CMD_NO;
      break;
    case CMD_REBOOT:
      message->message_type = REBOOT | REQUEST_MASK;
      message->error = 0;
      if (nodeMap[toNode].size >= 0) {
        if (netrdy(toNode)) {
          size = 2;
/*  since no reply
          issuedCommand = sysVars->netCommand;
          issuedCommandAt = time(0);
*/
          *netError = NET_NOERROR;
          taskAccomplished();
        }
      }
      sysVars->netCommand = CMD_NO;
      if (size == 0) {
        *netError = NET_NOSUCHNODE;
        taskAccomplished();
      }
      break;
    default:
      sysVars->netCommand = CMD_NO;
      break;
  }
  if (size) {
#ifdef NEW_930928
    nodeMap[toNode].rec.command = sBuf[0];
    nodeMap[toNode].rec.fromNode = 0;
    nodeMap[toNode].rec.error = NET_NOERROR;
    nodeMap[toNode].recTid = time(0);
#endif
    
    sendPacket(toNode, sBuf, size);  /* must receive answer if failed */
  }
}

int unfoldPacket(packet, rBuf, rSize)
struct _packet *packet;
char *rBuf;
int *rSize;
{
  *rSize = packet->header.size - sizeof(struct _header) - sizeof(char);
  if (*rSize > 0)
    memcpy(rBuf, packet->data, *rSize);
  status = _idle;
}

int sendPacket(node, buf, size)	/* returns no of bytes sent, 931124 */
int node;
char *buf;
int size;
{
  struct _packet packet;
  int master = 0, nb;
  /* pack it */

  packet.header.size = sizeof(struct _header) + size + sizeof(char);
  packet.header.version = 1;
  packet.header.targetMaster = 0;
  packet.header.targetNode = node;
  packet.header.sourceMaster = 0;
  packet.header.sourceNode = *nodeaddr;
  packet.header.command =  0;        /* data block */
  memcpy(packet.data, buf, size);
  packet.data[size] = 0;
  packet.data[size] = allBlockChs(&packet);

  nb = 0;  
  if (nodeMap[node].size >= 0) {
    if (netrdy(node)) {
      nb = netwrite(node, &packet, packet.header.size);
      netflush();
      byteCounter += packet.header.size;
      nodeMap[node].full = 0;
    } else
      nodeMap[node].full = 1;
  }
  if (DEBUG) printf("netwrite = %d (%d) to node %d\n", nb, packet.header.size, node);
  if (DEBUG) {
    int i;
    unsigned char *p;
    p = (unsigned char *) &packet;
    for (i = 0; i < packet.header.size; i++)
      printf("%02x, ", *p++);
    printf("\n");
  }
            
  netflush();           
          
  if (DEBUG2) printf("sendPacket: netwrite ok!\n");
  status = _packetSent;
  return nb;
}

