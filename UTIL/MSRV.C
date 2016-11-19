/*
!	route.c
*/
@_sysedit: equ 2
@_sysattr: equ $8002

/*
Syntax:
	route 				

	PC connected to /n196
	IVTmasterNet connected to /n296

7.3kalle
Kr„ver idc-vektorn MasterNodes[]

Alarm:
	router pollar regelbundet noderna
Vars, &&&:
	H†ller reda p† vilka noder som har utest†ende variabler
CLOSE_PCT:
	
Stat:
	Svar fr†n alarmpoll


Route skickar regelbundet	!!+<pc>

Obs ! endast en PC ansluten ?

PC skickar:
2 1 kalle 2 2 3gustav 
och node 2 har master 7 och node 3 har master 5.

skicka dels
#7 2 1 kalle
#5 2 2 3gustav eller #5 3 2 gustav

... eller varf”r inte ...

#5,7 2 1 kalle 2 2 3gustav 


&&&
skickas till
#3-5,7 M&&&		-> inget svar

d„refter skickas
#3 &&&			-> svar: upp till sex vars
sedan
#4 &&&
sedan
#5 &&&
sedan
#7 &&&



Close_Pct:
skickas till alla:

## CLOSE_PCT

*/


/*
!	system include files
*/
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <sgstat.h>
#include <math.h>
#include <time.h>

/*
!	definitions
*/
#define READ_TIMEOUT	5	/* 5 sek timeout vid start av read */

	/* type of request from pc */
#define LOCAL_ALARM_REQ		1
#define NET_REQUEST		2
#define LOCAL_SYSTEM_REQ	3
#define TIMEOUT			4

	/* definitions for metavar */
#define TYPE_INT_VEC    	4
#define TYPE_FLOAT_VEC  	5
#define TYPE_CALENDAR 		15
#define TYPE_INT    		7
#define TYPE_FLOAT  		8

	/* signals received from SCF-driver */
#define PC_SIGNAL	267
#define NET_SIGNAL	268

	/* command seperator and timeout */
#define COMMAND_SEPERATOR '&'
#define POLL_TIMEOUT 10			/* max wait time for '#n !!+' */

	/* message types */
#define MSG_SACK		0x01
#define MSG_VERSION		0x02
#define MSG_GETSTAT		0x03
#define MSG_SETSTAT		0x04
#define MSG_SHELL		0x05
#define MSG_CLOSE_PCT		0x06
#define MSG_METHOD_I		0x07
#define MSG_METHOD_II		0x08
#define MSG_GETCAL		0x09
#define MSG_SETCAL		0x10
#define MSG_SETVAR		0x11
#define MSG_GETALARM		0x12
#define MSG_RECSTAT		0x13
#define MSG_GETALARM_MSG	0x14
#define MSG_CONFIRM_ALARM	0x15
#define MSG_GARBAGE		0x16

#define IDLE			1
#define PCWAIT_NET_REPLY	2
#define WAIT_MASTER		3

	/* debug definitions for led's */
#define PCled_OFF		0
#define PCled_ON		1
#define NETled_OFF		2
#define NETled_ON		3
#define RUNled_OFF		4
#define RUNled_ON		5

#define PCled	0		/* red led */
#define NETled	1		/* green led */
#define RUNled	2		/* yellow led */

	/* name of variable data module */
#define NAMEOFDATAMODULE  "vars"

/*
!	typedef's
*/
typedef enum { _value, _name, _token, _nothing } item;

/*
!	global variables
*/
int signl;
int pathIn, pathOut, pathNetIn, pathNetOut;

	/* info for idc-variables 'masterVektor' and 'node2Master' */
int *masterVektor, *masterPoll;
int noOfMasters = 0;

int *Summa_A, *Summa_B, *Summa_C, *Summa_D;
int antalSumma_A = 0, antalSumma_B = 0, antalSumma_C = 0, antalSumma_D = 0;

#if 0
int *node2Master;		/* node# -> master# (idc-var) */
int noOfNodes = 0;
#endif

int *errorCount = 0;

	/* time when time was sent for pc/local master's */
static long whenSETPCTIME_wasSent;
static long whenSETTIME_wasSent;

char *dm;
char *meta;

char *commandPtr;
static char p1[256], p2[256], p3[256], p4[256];
item t1, t2, t3, t4;
int lastMethodIAccess = -1;

#if 0
int queueMasterNo;
char queueCmd[256];
#endif

unsigned char *KEYBOARDPOINTER = 0x308002;

/*
!	function prototypes
*/
char skipWhiteSpace();
char skipToken();

/*
!	function icp() receives signals from SCF-drivers
*/
void icp(s)
int s;
{
  signl = s;
}

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

int openPorts(portPC, portNet, fpIn, fpOut, fNetIn, fNetOut)
char *portPC, *portNet;
FILE **fpIn, **fpOut, **fNetIn, **fNetOut;
{
  if ((*fpOut = fopen(portPC, "w")) == NULL) {
    fprintf(stderr, "cannot open '%s' for write access\n", portPC);
    exit(0);
  }
  if ((*fpIn = fopen(portPC, "r")) == NULL) {
    fprintf(stderr, "cannot open '%s' for read access\n", portPC);
    exit(0);
  }
  if ((*fNetOut = fopen(portNet, "w")) == NULL) {
    fprintf(stderr, "cannot open '%s' for write access\n", portNet);
    exit(0);
  }
  if ((*fNetIn = fopen(portNet, "r")) == NULL) {
    fprintf(stderr, "cannot open '%s' for read access\n", portNet);
    exit(0);
  }
  pathIn  = (*fpIn)->_fd;
  pathOut = (*fpOut)->_fd;
  disableXonXoff(pathIn);
  disableXonXoff(pathOut);

  pathNetIn  = (*fNetIn)->_fd;
  pathNetOut = (*fNetOut)->_fd;
  disableXonXoff(pathNetIn);
  disableXonXoff(pathNetOut);
}

int readCommand(fpIn, message)
FILE *fpIn;
char *message;
{
  char buf = 0, *p;
  int n;
  time_t t1, t2;

  p = message;
  t1 = time(0);
  while (buf != 10) {
    if (_gs_rdy(fpIn->_fd) > 0) {
	n = read(fpIn->_fd, &buf, 1);
	*message++ = buf;
    } else {
	if (abs(time(0) - t1) > READ_TIMEOUT)
		return TIMEOUT;
    }
  }
  message --;
  message --;
  if (*message != 13)
	message ++;
  *message = 0;
  if (!strncmp(p, "!!!", 3))
	return LOCAL_ALARM_REQ;
  if (!strncmp(p, "enable", 6))
	return LOCAL_SYSTEM_REQ;
  if (!strncmp(p, "disable", 7))
	return LOCAL_SYSTEM_REQ;
  return NET_REQUEST;
}

int checkSend_SETPCTIME(fp)
FILE *fp;
{
  if ((whenSETPCTIME_wasSent == 0) || 
          (abs(time(0) - whenSETPCTIME_wasSent) > 1800)) {
    struct tm tid;
    time_t t1;

    t1 = time(0);
    memcpy(&tid, localtime(&t1), sizeof(struct tm));
   
    if ((tid.tm_hour != 23 || tid.tm_min < 30) &&
        (tid.tm_hour != 0  || tid.tm_min > 30))
    {
      fprintf(fp, "SET PCTIME \"%02d:%02d:%02d\"\015",
              tid.tm_hour, tid.tm_min, tid.tm_sec);
    }
    whenSETPCTIME_wasSent = time(0);
  }
}

int checkSend_SETTIME(fp)
FILE *fp;
{
  int sent = 0;
  if ((whenSETTIME_wasSent == 0) || 
          (abs(time(0) - whenSETTIME_wasSent) > 1800)) {
    struct tm tid;
    time_t t1;

    t1 = time(0);
    memcpy(&tid, localtime(&t1), sizeof(struct tm));
   
    if ((tid.tm_hour != 23 || tid.tm_min < 30) &&
        (tid.tm_hour != 0  || tid.tm_min > 30))
    {
      fprintf(fp, "## set datetime \"%02d.%02d.%02d %02d:%02d:%02d\"\015",
              tid.tm_mday, tid.tm_mon + 1, tid.tm_year, 
              tid.tm_hour, tid.tm_min, tid.tm_sec);
      sent = 1;
    }
    whenSETTIME_wasSent = time(0);
  }
  return sent;
}

int makeAlarmReqReply(message)
char *message;
{
  int i, x, alarm = 0, stat = 0;

/*  checkOurAlarms(&alarm, &stat);			/* call visionCom */

  for (i = 0; i < noOfMasters; i ++) {
	x = masterVektor[i];
	if (x > 0)
		alarm += x;
	else if (x == -2)
		stat ++;
  }
  if (alarm > 0)
	sprintf(message, "%d", alarm);
  else if (stat)
	strcpy(message, "STAT");
  else
	strcpy(message, "0");
  return (alarm > 0) || stat;
}

int typeOfCmd(p1, t1, p2, t2, p3, t3, p4, t4)
char *p1, *p2, *p3, *p4;
item t1, t2, t3, t4;
{
    if (t1 == _name && (!strncmp("SACK", p1, 7)))
	return MSG_SACK;
    if (t1 == _name && (!strncmp("version", p1, 7))) 
	return MSG_VERSION;
    if (t1 == _name && (!strncmp("getstat", p1, 7)) && 
                                          /* t2 == _name && */ t3 == _value)
	return MSG_GETSTAT;
    if (t1 == _name && (!strncmp("setstat", p1, 7)) && 
                                          /* t2 == _name && */ t3 == _value)
	return MSG_SETSTAT;
    if (t1 == _name && (!strncmp("SHELL", p1, 6)
                                 || !strncmp("shell", p1, 6)))
	return MSG_SHELL;
    if (t1 == _name && (!strncmp("CLOSE_PCT", p1, 9)
                                 || !strncmp("close_pct", p1, 9)))
	return MSG_CLOSE_PCT;
    if (t1 == _value && t2 == _value && t3 == _name)
	return MSG_METHOD_I;
    if (t1 == _token && !strncmp("&&&", p1, 3))
	return MSG_METHOD_II;
    if (t1 == _value && t2 == _name && t3 == _name && 
                                              (!strcmp("getcal", p2, 6)))
	return MSG_GETCAL;
    if (t1 == _value && t2 == _name && t3 == _name && 
                                              (!strcmp("setcal", p2, 6)))
	return MSG_SETCAL;
    if (t1 == _value && t2 == _name && t3 == _token && t4 == _value)
	return MSG_SETVAR;
    if (t1 == _token && !strncmp("!!!", p1, 3))
	return MSG_GETALARM;
    if (t1 == _name && (!strncmp("RECSTAT", p1, 7) || 
                                      !strncmp("RECSTAT", p1, 7)))
	return MSG_RECSTAT;
    if (t1 == _token && !strncmp("***", p1, 3))
	return MSG_GETALARM_MSG;
    if (t1 == _value && t2 == _value && t3 == _value && t4 == _value)
	return MSG_CONFIRM_ALARM;

    return MSG_GARBAGE;
}

int cnvNode2Master(currentMaster, node)
int currentMaster;
int node;
{

	currentMaster = node / 100;

/*
	if (node > 256)
		return node >> 8;

	if (node < noOfNodes)
		currentMaster = node2Master[node];
*/


	return currentMaster;
}

int convertNode2Master(toNode, masterNo)
int *toNode;
int *masterNo;
{
  *masterNo = (*toNode) / 100;
  *toNode = (*toNode) % 100;
}

int findMasterNo(msg, currentMaster)
char *msg;
int currentMaster;
{
  int node, i, typ;

  if (*msg == '#')
	return atoi(msg + 1);

  commandPtr = msg;
  parseCommand(&commandPtr, p1, &t1, p2, &t2, p3, &t3, p4, &t4);
  typ = typeOfCmd(p1, t1, p2, t2, p3, t3, p4, t4);

  switch (typ) {
	case MSG_CLOSE_PCT:
        case MSG_SACK:
		break;		/* use last master */
        case MSG_GETSTAT:	/* getstat 3kalle 1 */
        case MSG_SETSTAT:	/* setstat 3kalle 2 */
/* obs ! problem, parseCommand() tolkar fel ! */
		/* extrahera masternummer framf”r formul„rnamnet */
		node = *((double *) p2);
			/* mappa nod till master */
		currentMaster = cnvNode2Master(currentMaster, node);
		break;
        case MSG_RECSTAT:
		/* unders”k vilken nod som svarade med 'STAT' */

		for (i = 0; i < noOfMasters; i ++) {
			if (masterVektor[i] == -2)
				break;
		}
		if (i < noOfMasters)
			currentMaster = i;
		break;
        case MSG_VERSION:
        case MSG_SHELL:
		break;
        case MSG_CONFIRM_ALARM:		/* 3 17 1231 2 */
        case MSG_METHOD_I:		/* 3 1 kalle */
        case MSG_GETCAL:		/* 3 getcal kalle */
        case MSG_SETCAL:		/* 3 setcal kalle */
        case MSG_SETVAR:
		node = *((double *) p1);
			/* mappa nod till master */
		currentMaster = cnvNode2Master(currentMaster, node);
		if (typ == MSG_METHOD_I)
			lastMethodIAccess = currentMaster;
		break;
        case MSG_METHOD_II:
		/* vilken master fick sista Metod I - f”rfr†gan */

		currentMaster = lastMethodIAccess;
		break;
        case MSG_GETALARM:		/* doesn't matter, local copy used */
		break;
        case MSG_GETALARM_MSG:
		/* unders”k vilken nod som hade larm */
		for (i = 0; i < noOfMasters; i ++) {
			if (masterVektor[i] > 0)
				


				break;
		}
		if (i < noOfMasters)
			currentMaster = i;
		break;
	case MSG_GARBAGE:
		break;
  }
  return currentMaster;
}

int getNumItem(inBuf, sep, item, deflt)	/* "0,2,1,0,0"  item = 3 -> 1 */
char *inBuf;
char sep;
int item;
int deflt;
{
  while (item -- > 0) {
    if (!*inBuf)
	break;
    if (!item)
	return atoi(inBuf);

    while (*inBuf && *inBuf != sep)
	inBuf++;
    if (*inBuf == sep)
	inBuf++;
  }
  return deflt;
}

void analyzeAlarmReply(inBuf, currentMaster)	/* reply to '#m !!+' */
char *inBuf;
int currentMaster;
{
  if (currentMaster >= noOfMasters)
	return;				/* sorry, out of range */

  if (!strcmp(inBuf, "STAT"))
	masterVektor[currentMaster] = -2;
  else {
	masterVektor[currentMaster] = atoi(inBuf);
	if (currentMaster < antalSumma_A)
		Summa_A[currentMaster] = getNumItem(inBuf, ',', 2, 0);
	if (currentMaster < antalSumma_B)
		Summa_B[currentMaster] = getNumItem(inBuf, ',', 3, 0);
	if (currentMaster < antalSumma_C)
		Summa_C[currentMaster] = getNumItem(inBuf, ',', 4, 0);
	if (currentMaster < antalSumma_D)
		Summa_D[currentMaster] = getNumItem(inBuf, ',', 5, 0);
  }
}

typedef struct _link {
	char queueCmd[256];
	int queueMasterNo;
	struct _link *next;
} QUEUE;

typedef struct {
	struct _link *next;
	struct _link *last;
	int noOfLinks;
} QUEUE_HEAD;

QUEUE_HEAD head;

#define anyInQueue()	(head.noOfLinks)
#define initQueue()	(head.noOfLinks = 0)

void queueCommand(message, masterNo)
char *message;
int masterNo;
{
  QUEUE *tmp;

  if (head.noOfLinks > 5)
	return;

  tmp = (QUEUE *) malloc(sizeof(QUEUE));
  strcpy(tmp->queueCmd, message);
  tmp->queueMasterNo = masterNo;
  tmp->next = 0;
  if (head.last)
    head.last->next = tmp;
  else
    head.next = tmp;
  head.last = tmp;
  head.noOfLinks ++;
}

void unQueueCommand(message, masterNo)
char *message;
int *masterNo;
{
  QUEUE *tmp;
  if (tmp = head.next) {
    strcpy(message, tmp->queueCmd);
    *masterNo = tmp->queueMasterNo;
    head.next = tmp->next;
    if (head.next == 0)
	head.last = 0;
    free(tmp);
    head.noOfLinks --;
  }
}

/*
!	basic functions for route is:
!	Any commands from PC will be routed to the correct 
!	local master, the master number will be retrieved from
!	idc-variables which will map node to master number.
!	Replys from local master will be directly routed to pc.
!
!	Specials:
!	Alarms will be handled like they are handled in the local master.
!	During idle time, each local master will be polled for number of 
!	alarms. 
!
*/

int led(ledFcn)
int ledFcn;
{
  static int currentBits = 0;

  switch (ledFcn) {
	case PCled_OFF:
		currentBits &= ~(1 << PCled);
		break;
	case PCled_ON:
		currentBits |= (1 << PCled);
		break;
	case NETled_OFF:
		currentBits &= ~(1 << NETled);
		break;
	case NETled_ON:
		currentBits |= (1 << NETled);
		break;
	case RUNled_OFF:
		currentBits &= ~(1 << RUNled);
		break;
	case RUNled_ON:
		currentBits |= (1 << RUNled);
		break;
  }
  *KEYBOARDPOINTER = ~currentBits;
}

bind(dm, meta, headerPtr1, headerPtr2)
char **dm, **meta, **headerPtr1, **headerPtr2;
{
/*
!   bind to data module VARS, storage location for variables
*/  
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
  return 1;         /* BUG !! this line missing previous 920427 !! */
}

initVars()
{
  char *headerPtr1, *headerPtr2;
  int id;
  char name[20];

  bind(&dm, &meta, &headerPtr1, &headerPtr2);
  if (!(dm && meta)) {
    while(1) system("");
    exit(0);
  }

#if 0
  strcpy(name, "node2Master");
  if ((id = metaId(meta, name)) < 0) /* name not found */
    return 0;
  if (metaType(meta, id) == TYPE_INT_VEC) {
    node2Master = ((int *) metaValue(dm, meta, id));
    noOfNodes = metaSize(meta, id) / sizeof(int);
  }
#endif

  strcpy(name, "masterVektor");
  if ((id = metaId(meta, name)) < 0) /* name not found */
    return 0;
  if (metaType(meta, id) == TYPE_INT_VEC) {
    masterVektor = ((int *) metaValue(dm, meta, id));
    noOfMasters = metaSize(meta, id) / sizeof(int);
  }

  strcpy(name, "Summa_A");
  if ((id = metaId(meta, name)) < 0) /* name not found */
    Summa_A = 0;
  else if (metaType(meta, id) == TYPE_INT_VEC) {
    Summa_A = ((int *) metaValue(dm, meta, id));
    antalSumma_A = metaSize(meta, id) / sizeof(int);
  }
  strcpy(name, "Summa_B");
  if ((id = metaId(meta, name)) < 0) /* name not found */
    Summa_B = 0;
  else if (metaType(meta, id) == TYPE_INT_VEC) {
    Summa_B = ((int *) metaValue(dm, meta, id));
    antalSumma_B = metaSize(meta, id) / sizeof(int);
  }
  strcpy(name, "Summa_C");
  if ((id = metaId(meta, name)) < 0) /* name not found */
    Summa_C = 0;
  else if (metaType(meta, id) == TYPE_INT_VEC) {
    Summa_C = ((int *) metaValue(dm, meta, id));
    antalSumma_C = metaSize(meta, id) / sizeof(int);
  }
  strcpy(name, "Summa_D");
  if ((id = metaId(meta, name)) < 0) /* name not found */
    Summa_D = 0;
  else if (metaType(meta, id) == TYPE_INT_VEC) {
    Summa_D = ((int *) metaValue(dm, meta, id));
    antalSumma_D = metaSize(meta, id) / sizeof(int);
  }

  strcpy(name, "masterPoll");
  if ((id = metaId(meta, name)) < 0) /* name not found */
    return 0;
  if (metaType(meta, id) == TYPE_INT_VEC) {
    masterPoll = ((int *) metaValue(dm, meta, id));
    if (noOfMasters != metaSize(meta, id) / sizeof(int)) {
	    fprintf(stderr, "masterPoll och masterVektor har olika storlek\n");
	    exit(1);
    }
  }

  strcpy(name, "errorCount");
  if ((id = metaId(meta, name)) < 0) /* name not found */
    return 0;
  if (metaType(meta, id) == TYPE_INT) {
    errorCount = ((int *) metaValue(dm, meta, id));
  }
}

int nextMasterPoll(n)
int n;
{
  n++;
  if (n >= noOfMasters) 
    n = 0;
  while (n < noOfMasters) {
    if (masterPoll[n])
	break;
    n++;
  }
  if (n >= noOfMasters) {
    n = 0;
    while (n < noOfMasters) {
      if (masterPoll[n])
	break;
      n++;
    }
    if (n >= noOfMasters)
      n = 0;
  }
  return n;
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

int main(argc, argv)
int argc;
char *argv[];
{
  long sTime, nTime;
  int nextMaster = 0, autoPoll = 1;
  FILE *fpOut, *fpIn, *fNetOut, *fNetIn;
  int node, currentMaster;	/* , queued = 0; */
  int pcNumber = 0, state, typ, masterNo;
  char portPC[20], portNet[20];
  char message[256], buf[80];
  int retryCount = 0;
#define MAX_RETRY_COUNT 1

  head.noOfLinks = 0;
  strcpy(portPC, "/n196");
  strcpy(portNet, "/n296");

  openPorts(portPC, portNet, &fpIn, &fpOut, &fNetIn, &fNetOut);
  initidcio();

  initVars();
  intercept(icp);
/*
*/
  initphyio();
  initidcio(); 

/*
! create events
*/
  createEvent(&netFree, NET_EVENT_FREE, 1);
  createEvent(&netTaskAccomplished, NET_EVENT_TASK_ACCOMPLISHED, 0);

  state = IDLE;
  nextMaster = nextMasterPoll(nextMaster);
  while (1) {
	led(RUNled_OFF);
	_ss_ssig(pathNetIn, NET_SIGNAL);
	led(RUNled_ON);

		tsleep(10);		???

	if (checkCommand(&masterNo, currentMaster)) {
		led(PCled_ON);		/* incoming message from PC */
		if (state == IDLE) {
			currentMaster = masterNo;
			fprintf(fNetOut, "#%d %s\015", 
				currentMaster, message);
			led(NETled_ON);
			if (strncmp(message, "ACK", 3))		/* if it's ACK, don't wait for reply */
				state = PCWAIT_NET_REPLY;	/* only if not ack */
		} else {
			queueCommand(message, masterNo);
		}
	} else if (signl == NET_SIGNAL) {
		typ = readCommand(fNetIn, message);
		if (typ == TIMEOUT) {
			;	/* just skip it */
		} else if (state == PCWAIT_NET_REPLY) {
			led(NETled_OFF);
			returnCommand(message);	/* fprintf(fpOut, "%s\015", message); */
			led(PCled_OFF);
			state = IDLE;
		} else if (state == WAIT_MASTER) {	/* wait for local !!+ */
			retryCount = 0;
			if (!strncmp(message, "SET PCTIME", 10))
				;	/* just skip it, we rule the time ! */
			else {
				led(NETled_OFF);
				analyzeAlarmReply(message, currentMaster);
				nextMaster = nextMasterPoll(nextMaster);
/*
				nextMaster ++;
				if (nextMaster > noOfMasters)
					nextMaster = 0;
*/
				state = IDLE;
			}
		} else {
			;	/* skip input ! */
		}
	} else if (state == IDLE) {
			checkSend_SETTIME(fNetOut);
	}

	signl = 0;

	switch (state) {
		case IDLE:
			if (anyInQueue()) {
				unQueueCommand(message, &masterNo);
				currentMaster = masterNo;
				fprintf(fNetOut, "#%d %s\015", 
						currentMaster, message);
				led(NETled_ON);
				if (strncmp(message, "ACK", 3))		/* if it's ACK, don't wait for reply */
					state = PCWAIT_NET_REPLY;	/* only if not ack */
			} else if (autoPoll) {
				if (checkSend_SETTIME(fNetOut)) 
					;
				else {
					currentMaster = nextMaster;
 					fprintf(fNetOut, "#%d !!+%c\015", 
							currentMaster, 
							'0' + pcNumber);
					led(NETled_ON);
					sTime = time(0);
					state = WAIT_MASTER;
				}
			}
			break;
		case WAIT_MASTER:
			nTime = time(0);
			if ((nTime - sTime) > POLL_TIMEOUT) {
			    if (++retryCount > MAX_RETRY_COUNT)
			    {
				nextMaster = nextMasterPoll(nextMaster);
				retryCount = 0;
				if (errorCount)
					(*errorCount)++;
			    }
			    state = IDLE;
			}
			break;
		case PCWAIT_NET_REPLY:
			nTime = time(0);
			if ((nTime - sTime) > POLL_TIMEOUT) {
				if (errorCount)
					(*errorCount)++;
				state = IDLE;
			}
			break;
	}
  }

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
    return _token;
  }
}

int parseCommand(commandLine, p1, t1, p2, t2, p3, t3, p4, t4)
char **commandLine;
char *p1, *p2, *p3, *p4;
item *t1, *t2, *t3, *t4;
{
  char buf2[100], buf3[100];
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

/*
!	interface to visionCom routines
*/
int checkOurAlarms(alarm, stat)		/* call visionCom */
int *alarm, *stat;
{
}

int callVisionCom(message)		/* call visionCom */
char *message;
{
}

/*
!	returns 0 if no new command request
*/
int checkCommand(masterNo, currentMaster)
int *masterNo;
int currentMaster;
{
  int id, toNode, q, typ, i;
  struct _method1Arr { 
		short int node, idx; char name[32]; double value;
  } *localArrPtr;
  char *value;
  static char currentCalendar[32];

  *masterNo = currentMaster;
  if (sysVars->netCommand == CMD_NO)
	return 0;

  id = sysVars->varId;
  toNode = sysVars->nodeId;
  
  issuedCommand = sysVars->netCommand;
  switch (sysVars->netCommand) {
    case CMD_GETALARM_TEXT:
	sprintf(message, "***");
	break;
    case CMD_ACK_ALARM:
	sprintf(message, "ACK");
	sysVars->netCommand = CMD_NO;
	*netError = NET_NOERROR;
	taskAccomplished();
	break;
    case CMD_CONFIRM_ALARM:
	convertNode2Master(&toNode, masterNo);
	sprintf(message, "%d %d %d %d", toNode, 
				((short int *) sysVars->netAPIarea)[0],
				((short int *) sysVars->netAPIarea)[1],
				((short int *) sysVars->netAPIarea)[2]);
	break;
    case CMD_GETIDX_VAR:
	convertNode2Master(&toNode, masterNo);
	sprintf(message, "%d %d %s", toNode,
				((short int *) sysVars->netAPIarea)[0],
			(char *) &((short int *) sysVars->netAPIarea)[1]);
	break;
    case CMD_GET_N_IDX_VAR:
	convertNode2Master(&toNode, masterNo);
	id &= 0x00ff;
	localArrPtr = *((struct _method1Arr **) sysVars->netAPIarea);
	for (i = q = 0; i < id; i ++) {
		sprintf(&message[q], "%d %d %s & ", toNode,
				localArrPtr[i].idx,
				localArrPtr[i].name);
		q += strlen(&message[q]);
	}
	if (q)
		message[q - 3] = 0;
	break;
    case CMD_GET_UPDATED:
	sprintf(message, "&&&");
	break;
    case CMD_PUTIDX_VAR:
	convertNode2Master(&toNode, masterNo);
	value = (char *) &sysVars->netAPIarea[0];
	typ = sysVars->netAPIarea[8];
	sprintf(message, "%d %s = %g", toNode, 
		&sysVars->netAPIarea[9],
		(typ == TYPE_FLOAT) ? 
			*((double *) value),
			*((long *) value));
	break;
    case CMD_GET_CAL:
	convertNode2Master(&toNode, masterNo);
	sprintf(message, "%d getcal %s", toNode,
			&sysVars->netAPIarea[8]);
	strcpy(currentCalendar, &sysVars->netAPIarea[8]);
	break;
    case CMD_SET_CAL:
	convertNode2Master(&toNode, masterNo);
	sprintf(message, "%d setcal %s \"%s\"", toNode,
			currentCalendar,
			((long *) sysVars->netAPIarea)[0]);
	break;
    default:
      	issuedCommand = 0;
  }
}

long atoin(s, n)
char *s;
int n;
{
  char b[10];
  memcpy(b, s, n);
  b[3] = 0;
  return atol(b);
}

long maketime(s)	/* 94.11.15 16:02:15 */
char *s;
{
  struct tm tp;
  tp.tm_year = atoi(s);
  tp.tm_mon = atoi(&s[3]);
  tp.tm_mday = atoi(&s[6]);
  tp.tm_hour = atoi(&s[9]);
  tp.tm_min = atoi(&s[12]);
  tp.tm_sec = atoi(&s[15]);
  return mktime(&tp);
}

int returnCommand(message)
char *message;
{

  switch (issuedCommand) {
    case CMD_GETALARM_TEXT:
/*
!
*/
    struct _alarmText
    {
      unsigned short int alarmNo, serialNo, status;
      long dtime;
      char text[80];
      unsigned short int master, node;
    } *alarmTextRequest;               /* 90 byte */


	alarmTextRequest = sysVars->netAPIarea;
	alarmTextRequest->alarmNo = atoin(&message[11], 3);
	alarmTextRequest->serialNo = atoin(&message[14], 5);
	alarmTextRequest->status = atoin(&message[19], 3);
	alarmTextRequest->dtime = maketime(&message[26]);
	strcpy(alarmTextRequest->text, &message[45]);
	alarmTextRequest->master = atoin(&message[5], 3);
	alarmTextRequest->node = atoin(&message[8], 3);
	taskAccomplished();
	break;
    case CMD_ACK_ALARM:
	break;			/* no reply */
    case CMD_CONFIRM_ALARM:
	*netError = (!strncmp("ACK", message, 3) ? 0 : 1);
	taskAccomplished();
	break;
    case CMD_GETIDX_VAR:
        *netError = message->error;
	sysVars->netAPIarea[0] = TYPE_FLOAT;
	double v;
	v = atof(...)
	memcpy(&sysVars->netAPIarea[1], &v, sizeof(double));
        taskAccomplished();
	break;
    case CMD_GET_N_IDX_VAR:
	convertNode2Master(&toNode, masterNo);
	id &= 0x00ff;
	localArrPtr = *((struct _method1Arr **) sysVars->netAPIarea);
	for (i = q = 0; i < id; i ++) {
		sprintf(&message[q], "%d %d %s & ", toNode,
				localArrPtr[i].idx,
				localArrPtr[i].name);
		q += strlen(&message[q]);
	}
	if (q) 
		message[q - 3] = 0;
	}
	break;
    case CMD_GET_UPDATED:
	sprintf(message, "&&&");
	break;
    case CMD_PUTIDX_VAR:
	convertNode2Master(&toNode, masterNo);
	value = (char *) &sysVars->netAPIarea[0];
	typ = sysVars->netAPIarea[8];
	sprintf(message, "%d %s = %g", toNode, 
		&sysVars->netAPIarea[9],
		(typ == TYPE_FLOAT) ? 
			*((double *) value),
			*((long *) value));
	break;
    case CMD_GET_CAL:
	convertNode2Master(&toNode, masterNo);
	sprintf(message, "%d getcal %s", toNode,
			&sysVars->netAPIarea[8]);
	strcpy(currentCalendar, &sysVars->netAPIarea[8]);
	break;
    case CMD_SET_CAL:
	convertNode2Master(&toNode, masterNo);
	sprintf(message, "%d setcal %s \"%s\"", toNode,
			currentCalendar,
			((long *) sysVars->netAPIarea)[0]);
	break;
    default:
      	issuedCommand = 0;
  }
}


