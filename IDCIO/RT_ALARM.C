/* runtime_alarm.c  1992-09-14 TD,  version 1.4 */
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
! runtime_alarm.c
! Copyright (C) 1991,1992 IVT Electronic AB.
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
!                            added disable/enableTime
*/

#define _V920914_1_40

#include <varargs.h>
#include <modes.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#define NO_OF_ALARMS 1
#include "alarm.h"

#include "sysvars.h"

struct _system *sysVars = SYSTEM_AREA;

#ifdef NOT_PROM
extern int screenPid;
extern int _any_sent_to_scan;
extern struct _alarmModule *aldm;
extern int DEBUG, LOCAL_PRINTER;
#endif

#define MAX_BUF 79 
static char pic[80];        /* 920129    400 -> 80 */

static int currentPos;
/*
!
!   Entries:
!      
!      _closeConnection(aldm, aldm2, alarmIndex, alarmNo, class)
!      struct _alarmModule *aldm;
!      struct _alarmModule2 *aldm2;
!      int alarmIndex, alarmNo, class;
!      
!      _clearContext()
!      
!      _alarmDisplay(va_alist)
!      va_dcl
!      
!      
*/

_clearContext()
{
  int i;
  for (i = 0; i < 80; i++)
    pic[i] = ' ';
  pic[0] = '\0';
  currentPos = 0;
}

/*
!   multiple arguments implemented
!   stores string in static buffer 'pic'
*/
_alarmDisplay(va_alist)
va_dcl
{
  char *stf, *p, line[40], *fmtPek, fmt[40], *sval;   /* 920129 line 80->40 */
  int ival, offset, width, i;
  double dval;
  va_list ap;

  va_start(ap);

  stf = va_arg(ap, char *);
  for (p = stf; *p; p++) {
    if (*p != '%') {
      if (currentPos < MAX_BUF) {
        pic[currentPos++] = *p;
        pic[currentPos] = 0;
      }
      continue;
    }
    ++p;
    fmtPek = fmt;
    *fmtPek++ = '%';
    while (isdigit(*p) || *p == '.' || *p == '-')
      *fmtPek++ = *p++;
    *fmtPek++ = *p;
    *fmtPek = '\0';   
   
    switch (*p) {
      case 'a':
        ival = va_arg(ap, int);
        --fmtPek;
        *fmtPek = '\0';  
        fmtPek = fmt; fmtPek ++;
        offset = 0;
        width = 8;
        if (*fmtPek) {
          if ((width = atoi(fmtPek)) > 32)
            width = 32;
          while (*fmtPek && *fmtPek != '.')
            fmtPek++;
          if (*fmtPek) {
            offset = width;
            fmtPek++;
            if ((width = atoi(fmtPek)) > 32)
              width = 32;
          }
        }
        line[0] = '\0';
        ival >>= offset;
        line[width] = '\0';
        while (width ) {
          line[--width] = (ival & 1) ? '\2' : '\1';
          ival >>= 1;
        }
        currentPos += strlen(line);
        if (currentPos < MAX_BUF)
          strcat(pic, line);  
        break;
      case 'c':
        ival = va_arg(ap, int);
        sprintf(line, fmt, ival);
        currentPos += strlen(line);
        if (currentPos < MAX_BUF)
          strcat(pic, line);  
        break;
      case 'd':
        ival = va_arg(ap, int);
        sprintf(line, fmt, ival);
        currentPos += strlen(line);
        if (currentPos < MAX_BUF)
          strcat(pic, line);  
        break;
      case 'f':
        dval = va_arg(ap, double);
        sprintf(line, fmt, dval);
        currentPos += strlen(line);
        if (currentPos < MAX_BUF)
          strcat(pic, line);  
        break;
      case 'g':
        dval = va_arg(ap, double);
        sprintf(line, fmt, dval);
        currentPos += strlen(line);
        if (currentPos < MAX_BUF)
          strcat(pic, line);  
        break;
      case 's':
        sval = va_arg(ap, char *);
        ival = currentPos;
        currentPos += strlen(sval);
        if (currentPos < MAX_BUF) {
          sprintf(&pic[ival], fmt, sval);
	}
/*         currentPos += strlen(sval);     size of fmt ! */

/*
        sprintf(line, fmt, sval);
        strcat(pic, line);  currentPos += strlen(line);
*/
        break;
      default:
        if (currentPos < MAX_BUF) {
          pic[currentPos++] = *p;
          pic[currentPos] = 0;
        }
        break;
    }
  }
  va_end(ap);
} 

#ifdef THIS_IS_IDC_CODE
/* ------------- alarm no 0 ----------------- */
void alarm_0()
{
  _clearContext();
  if (!aldm2->alarmPts[0].disable) {
    if (dm->gt51) {
      if (_markAlarm(aldm2, 0,(int) (0), 0, (int) (dm->klass_Givare) - 1))
        {
          display("Gt51-larm");
        }
    }
    else if (_unmarkAlarm(aldm, aldm2, 0, 0))
      ;
  }
  dm->gt51_alarm = _closeConnection(aldm, aldm2, 0, 0, (int) (dm->klass_Givare) - 1);
}
#endif

/* find the appropriate alarm entry and free it */
int makeRoom(aldm, aldm2)
struct _alarmModule *aldm;
struct _alarmModule2 *aldm2;
{
  int i, j, found = 0;
  struct _alarmEntry *entry;

  for (j = 3; j >= 0; j --) {
    entry = &aldm->alarmList[0];
    for (i = 0; i < aldm->alarmListPtr; i++, entry++) {
      if (j == entry->class) {
        if (found = (!entry->active && entry->confirm && !entry->disable))
	  break;
      }
    }
    if (found)
      break;
    entry = &aldm->alarmList[0];
    for (i = 0; i < aldm->alarmListPtr; i++, entry++) {
      if (j == entry->class) {
        if (found = (!entry->active && !entry->disable))
	  break;
      }
    }
    if (found)
      break;
  }
  if (found) {
    entry->sendMask = 0;
    entry->confirm = 1;
    entry->active = 0;
  }
  return found;
}

/* find the appropriate alarm index and free it */
int freeAlarmIndex(aldm, aldm2, alarmIndex)
struct _alarmModule *aldm;
struct _alarmModule2 *aldm2;
int alarmIndex;
{
  int i, j, found = 0;
  struct _alarmEntry *entry, *lastEntry = 0;

  entry = &aldm->alarmList[0];
  for (i = 0; i < aldm->alarmListPtr; i++, entry++) {
#if 1
    if (alarmIndex == entry->alarmIndex) {
      lastEntry = entry;
      if (entry->active) {
        entry->sendStatus |= ALARM_SEND_NEGATE;
	entry->active = 0;
	entry->offTime = time(0);
      }
      if (!entry->confirm) {
	entry->confirm = 1;
	entry->confirmTime = time(0);
	entry->sendStatus |= ALARM_SEND_CONFIRM;
      }
      found = 1;
    }
#else
    if (alarmIndex == entry->alarmIndex) {
      entry->sendMask = 0;
      entry->confirm = 1;
      entry->active = 0;
      entry->disable = 0;
      found = 1;
    }
#endif
  }
  if (lastEntry && aldm2->alarmPts[alarmIndex].disable) {
    lastEntry->sendStatus |= ALARM_SEND_ENABLE;
    lastEntry->enableTime = time(0);
    aldm2->alarmPts[alarmIndex].disable = 0;
  }
  return found;
}

/*
!   returns -1 if blocked
!	     0 if not active and no previous none confirmed entry exists
!	     1 if not active but there exists previous none confirmed entry
!	     2 if active and all previous entries are confirmed
!	     3 if active but one or some are none confirmed
*/
static int alarmStatus(aldm, aldm2, alarmIndex, alarmNo)
struct _alarmModule *aldm;
struct _alarmModule2 *aldm2;
int alarmIndex, alarmNo;
{
  int i, sts = 0;
  struct _alarmEntry *entry;
  int confirmed = 0, noneconfirmed = 0, entries = 0;

  if (aldm2->alarmPts[alarmIndex].disable)
	return -1;

/* scan all entries and set 0 if all are confirmed, 
			1 if all are none confirmed and 2 if mixed */
  entry = &aldm->alarmList[0];
  for (i = 0; i < aldm->alarmListPtr; i++, entry++) {
    if (alarmIndex != entry->alarmIndex)
      continue;
    if (entry->confirm)
	confirmed ++;
    else
	noneconfirmed ++;
    entries ++;
  }
  if (aldm2->alarmPts[alarmIndex].active) {
    if (entries == confirmed) /* all entries were confirmed */
      return 2;   /* if active and all previous entries are confirmed */
    else
      return 3;   /* if active but one or some are none confirmed */
  } else {
    if (entries == confirmed) /* there exists no none confirmed entries */
      return 0;   /* if not active and no previous none confirmed entry exists */
    else
      return 1;   /* if not active but there exists previous none confirmed entry */
  }
}

_closeConnection(aldm, aldm2, alarmIndex, alarmNo, class)
struct _alarmModule *aldm;
struct _alarmModule2 *aldm2;
int alarmIndex, alarmNo, class;
{
  if (class == -1) {					/* added 940704 */
    if (aldm2->alarmPts[alarmIndex].initTime != -1) {
      aldm2->alarmPts[alarmIndex].initTime = -1;
      aldm2->alarmPts[alarmIndex].active = 0;
      if (freeAlarmIndex(aldm, aldm2, alarmIndex))
        _packAlarms(aldm, aldm2);
    }
    return alarmStatus(aldm, aldm2, alarmIndex, alarmNo);
  }
  if (!currentPos)
    return alarmStatus(aldm, aldm2, alarmIndex, alarmNo);

  if (aldm->alarmListPtr >= aldm->noOfAlarmEntries) {
    if (makeRoom(aldm, aldm2))
      _packAlarms(aldm, aldm2);
  }
  if (aldm->alarmListPtr < aldm->noOfAlarmEntries) {
/*
!     920921,   uses SIZE_OF_ALARM_TEXT, 
!                                 (changed to 72 in alarm.h -> max 71 chars)
*/
    pic[SIZE_OF_ALARM_TEXT - 1] = '\0';
    strcpy(aldm->alarmList[aldm->alarmListPtr].string, pic);
    
#ifdef _V920914_1_40
    aldm->alarmList[aldm->alarmListPtr].sendMask = 0;
    aldm->alarmList[aldm->alarmListPtr].sendStatus = ALARM_SEND_ASSERT;/*ejinit*/
/*    aldm->alarmList[aldm->alarmListPtr].sendDisable = 0;    */
    aldm->alarmList[aldm->alarmListPtr].enableSent = 0;
#else
    aldm->alarmList[aldm->alarmListPtr].sendAssert = 0xff;  /* 920129 1->255 */
    aldm->alarmList[aldm->alarmListPtr].sendNegate = 0;
    aldm->alarmList[aldm->alarmListPtr].sendConfirm = 0;
    aldm->alarmList[aldm->alarmListPtr].sendDisable = 0;
#endif
    aldm->alarmList[aldm->alarmListPtr].assertSent = 0;
    aldm->alarmList[aldm->alarmListPtr].negateSent = 0;
    aldm->alarmList[aldm->alarmListPtr].confirmSent = 0;
    aldm->alarmList[aldm->alarmListPtr].disableSent = 0;

    aldm->alarmList[aldm->alarmListPtr].confirm = 0;
    aldm->alarmList[aldm->alarmListPtr].disable = 0;
    aldm->alarmList[aldm->alarmListPtr].active = 1;
    aldm->alarmList[aldm->alarmListPtr].class = class & 3; /* &3 added 940618*/

    aldm->alarmList[aldm->alarmListPtr].alarmNo = alarmNo;
    aldm->alarmList[aldm->alarmListPtr].alarmIndex = alarmIndex;
    aldm2->alarmPts[alarmIndex].serialNo = 
                        aldm->alarmList[aldm->alarmListPtr].serialNo =
                        sysVars->serialNo++;
    aldm->alarmList[aldm->alarmListPtr].initTime = time(0);
    aldm->alarmList[aldm->alarmListPtr].offTime = 0;
    aldm->alarmList[aldm->alarmListPtr].confirmTime = 0;
    aldm->alarmList[aldm->alarmListPtr].disableTime = 0;
    aldm->alarmList[aldm->alarmListPtr].enableTime = 0;
    aldm->alarmListPtr ++;
  } else {
    ;             /* routine packAlarms manage this case ! */
  }
  return alarmStatus(aldm, aldm2, alarmIndex, alarmNo);
}

print(s)
char *s;
{
  FILE *fp;
 
/*  if (LOCAL_PRINTER) {    */
    char buff[255];
    convert(s, buff);
    fp = fopen("/t1", "w");
    fprintf(fp, "%s", buff);
    fclose(fp);
/*  }   */
}

convert(codestring, s)
char *codestring, *s;
{
  int i, len;

  strcpy(s, codestring);
  len = strlen(s);
  for (i = 0; i < len; i++) {
    if (s[i] == '\17') 
      s[i] = 'o';                 /* degree sign */
    else if (s[i] == '\06')
      s[i] = 'a';
    else if (s[i] == '\04')
      s[i] = 'a';
    else if (s[i] == '\24')
      s[i] = 'o';
    else if (s[i] == '\00')        /* ??? code = ??? */
      s[i] = 'A';
    else if (s[i] == '\16')
      s[i] = 'A';
    else if (s[i] == '\31')
      s[i] = 'O';
  }
}

