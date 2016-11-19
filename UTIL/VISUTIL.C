#include <ctype.h>
#include <time.h>
#include <stdio.h>

#define _V920914_1_40 /* globalAlarmMask system fix !! (see other modules !) */

#define TEST_920914           /* Bugfix-test */

#include "sysvars.h"
#define NO_OF_ALARMS 1
#include "alarm.h"

#include "ivtnet/v1.1/net.h"
#include "ivtnet/v1.1/stat.h"
#define NET_TIMEOUT 0x08

#define NET_NOSUCHALARM     0x0b

struct _system *sysVars = SYSTEM_AREA;

unsigned char abcdMask[4];
extern int PCno;

int internalCounter;     /* next alarm text to send after request !!! */
int alarmMarkPtr[8];
int alarmMarkNode;

/*
!   MAP_SIZE is max number from pc
*/
#define MAP_SIZE                  100     /* 1000 */
int idleCount[MAP_SIZE];            /* counter for each idle var in method ii */
#define IDLE_MAX_COUNT 20           /* idle for max no of polls */
static int map[MAP_SIZE];
extern unsigned short mapNode[];

extern char close_pct_atNode[];

extern int pathIn;
extern FILE *fpOut;


char *getNextUpdatedVar();

#define TYPE_INT_VEC    4
#define TYPE_FLOAT_VEC  5
#define TYPE_CALENDAR 15
#define TYPE_INT    7
#define TYPE_FLOAT  8

int method2running;

#define oldRevision 0

#define NO_OF_CAL_ENTRIES 10
struct _calendar
{
  unsigned short day[NO_OF_CAL_ENTRIES];
  unsigned short stopday[NO_OF_CAL_ENTRIES];
  unsigned char color[NO_OF_CAL_ENTRIES];
  unsigned short start[NO_OF_CAL_ENTRIES];
  unsigned short stop[NO_OF_CAL_ENTRIES];
};

#define MAX_NODE_NO 64

#define MAX_NODE4MASTER 100	/*	0-99 is valid numbers, 102 is master 1, slave 2	*/

extern int useVars;
extern struct _alarmModule *aldm;
extern char *dm, *dmBackup;
extern char *meta;
extern int OUR_NODE;
static int  currentId = 1,          /* next id for get-var-method ii */
            currentNode;
static int currentCalendarIdx = 0;


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

int makeAlarmMask(dm, meta, mask)
char *dm, *meta, *mask;
{
  int i;
  for (i = 0; i < 4; i++)
    mask[i] = receiveAlarmMask(dm, meta, i);
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

#define INDEXVAR
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
/*    checkAlias(name);    	*/
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
/*    checkAlias(name);    	*/
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
#if 0
  count += getAlarmsForAllNodes(mask, pc);
#endif
  return count;
}

#if 0
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
#endif

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

    makeAlarmMask(dm, meta, abcdMask);

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

	return 0;
#if 0
      if (!(nd = getNextAlarmNode(mask, pc)))
        return 0;
#endif
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
#ifdef NEW_ALARM_TEST
    alarmMarkPtr[PCno & 7] = *serie;
#else
    alarmMarkPtr[PCno & 7] = internalCounter;
#endif
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

int concat(outLinePtr, outLine, outBuf)
char **outLinePtr, *outLine, *outBuf;
{
  char buf[10], *bptr;
  bptr = buf;

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

