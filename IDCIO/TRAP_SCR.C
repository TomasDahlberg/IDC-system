/* trap_screen.c  1992-09-22 TD,  version 1.41 */
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
! trap_screen.c
! Copyright (C) 1991,1992 IVT Electronic AB.
*/

/*
!   history:
!   date       by  rev  ed#  what
!   ---------- --- ---  ---  ------------------------------------
!   1991-??-?? td  1.00   1  initial coding
!
!     92-06-17 td  1.30      right arrow once for alarms, new priviledge
!
!     92-09-14 td  1.40   x  bugfix in globalAlarmMask_x functionality
!                            activated by #define _V920914_1_40
!                            added disable/enableTime
!
!     92-09-22 td  1.41      bugfix. When disable-ing a non active alarm, the 
!                            alarm disappeared from screen since it now became
!                            both confirmed and inactive ! Added check for dis.
*/

#define _V920914_1_40

/*
#define RIGHT_ARROW_ONCE_ADDED_920617
*/

#include <stdio.h>
#include <strings.h>
#include <time.h>
#define NO_OF_ALARMS 1
#include "alarm.h"
#include "sysvars.h"

#define UPDATE_SCREEN

#include "phyio.h"
#include "idcio.h"
#include "priv.h"

#define TIMEOUT_VALUE 300

int display(s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12)
char *s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9, *s10, *s11, *s12;
{
  lcdprintf(s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12);
}

/*
int oldEnter(s, v)
char *s;
char *v;
{
  int index = 0, key;
  
  lcdprintf(s);
  while (1) {
    switch (key = getKey()) {
      case KEY_ENTER:
        return 0;
      case KEY_LEFT:
        if (index > 0) {
          v --;
          lcdpositRel(-1, 0);
          lcdwrite(' ');
          lcdpositRel(-1, 0);
        } 
        break;
      case KEY_RIGHT:
      case KEY_UP:
      case KEY_DOWN:
      case KEY_PLUS_MINUS:
      case KEY_ALARM:
      case KEY_CHANGE:
      case KEY_HELP:
        break;
      default:
        *v++ = '0' + key;
        lcdwrite('0' + key);
        index ++;
        break;
    }
  }
}
*/

double getValue();


#define NEW_CHANGE_VAR
#ifdef NEW_CHANGE_VAR
int editVariable(dm, meta, varId)
char *dm;
char *meta;
int varId;
{
  double v1, v2;

  lcdcursorOn();
  if (metaType(meta, varId) == 8) /* float */
    v1 = *((double *) metaValue(dm, meta, varId));
  else if (metaType(meta, varId) == 7) /* int */
    v1 = *((int *) metaValue(dm, meta, varId));
  else
    return 0; 
  v2 = getValue(v1);
  if (v1 != v2) {
    if (metaType(meta, varId) == 7)
        *((long *) metaValue(dm, meta, varId)) = (int) v2;
    else
        *((double *) metaValue(dm, meta, varId)) = v2;
  }
  return 0;
}

#else
int editVariable(dm, meta, varId)
char *dm;
char *meta;
int varId;
{
  char line[80];
  int pos, key;
  double tmpVar;         /* may hold integer */
  double decade;        /* 10** exp, but sometimes small errors can show up */
  int exp;              /* same as log10(decade) */
  time_t timer;

  lcdcursorOn();
  decade = 1; pos = 0; exp = 0;
  tmpVar = *((double *) metaValue(dm, meta, varId));

/*
! if float then pos should point to least digit in integer part
*/    
  if (metaType(meta, varId) == 8) /* float */
  {
    char *tmp;
    sprintf(line, "%s = %8f, %8f", metaName(meta, varId),
        *((double *) metaValue(dm, meta, varId)),
        *((double *) &tmpVar));
   if (tmp = rindex(line, '.')) {
      pos = strlen(tmp);
   }
  }
  timer = time(0);
  while (1) {
    if ((time(0) - timer) > TIMEOUT_VALUE) {
      lcdcursorOff();
      return 1;
    }
    if (metaType(meta, varId) == 7) /* int */
      sprintf(line, "%s = %8d, %8d", metaName(meta, varId),
        *((long *) metaValue(dm, meta, varId)),
        *((long *) &tmpVar));
    else if (metaType(meta, varId) == 8) /* float */
      sprintf(line, "%s = %8f, %8f", metaName(meta, varId),
        *((double *) metaValue(dm, meta, varId)), 
        *((double *) &tmpVar));

    lcdhome();
    display(line);

    lcdpos(0, strlen(line) - pos - 1);
  
    if (keyDown()) {
      key = getKey();
      timer = time(0);
    } else 
      key = NO_KEY;
        
    if (key == KEY_UP) {
      if (metaType(meta, varId) == 7)
        *((long *) &tmpVar) += decade;
      else
        *((double *) &tmpVar) += decade;
    } else if (key == KEY_DOWN)
      if (metaType(meta, varId) == 7)
        *((long *) &tmpVar) -= decade;
      else
        *((double *) &tmpVar) -= decade;
    else if ((key == KEY_LEFT) && (pos < 10)) {
      pos ++;
      decade *= 10;   exp ++;
      if (exp == 0)     /* decade == 1, not good, since decade may be 0.99999 */
        pos ++;
    } else if ((key == KEY_RIGHT) && (pos > 0)) {
      pos --;
      if (exp == 0) 
        pos --;
      decade /= 10;   exp --;
    } else if (key == KEY_HELP) {
      lcdcursorOff();
      return 0;
    } else if (key == KEY_CHANGE) {
      lcdcursorOff();
      return 0;                  /* 1 changed 920107 */
    } else if (key == KEY_ENTER) {
      if (metaType(meta, varId) == 7)
        *((long *) metaValue(dm, meta, varId)) = 
                *((long *) &tmpVar);
      else
        *((double *) metaValue(dm, meta, varId)) = 
                *((double *) &tmpVar);
    }
  }
}
#endif

/*
int edit(dm, meta, var)
char *dm;
char *meta;
char *var;
{
  int id;
  
  for (id = 1; metaName(meta, id) ; id++) {
    if (metaName(dm, meta, id) == var) {
      editVariable(dm, meta, id); 
      break;
    }
  }
}
*/

int showVariables(dm, meta)
char *dm;
char *meta;
{
  int varId, prevId, keyvalue, prev;
  time_t timer;
  static struct _dtime tid1 = { 0, 0, 0};
  short int noOfItems;                                 /* !!!! */
#define IDLE 0
#define REPEAT 1
#define REPEAT_TIME  100
#define START_REPEAT_TIME 700

  int REPEATstate;
  long REPEATdelayed;
  
  noOfItems = *((short int *) meta);
  lcdcld();
  REPEATstate = 0;
  varId = 1; prevId = -1;
  while (1) {
    if (varId != prevId) {
      lcdcld_through_cache();
      prevId = varId;
      timer = time(0);
    }
    if ((time(0) - timer) > TIMEOUT_VALUE) {
      return 1;
    }
    lcdhome();
    if (metaType(meta, varId) == 7) /* int */
      display("%s = %d          ", metaName(meta, varId),
        *((int *) metaValue(dm, meta, varId)));
    else if (metaType(meta, varId) == 8) /* float */
      display("%s = %g          ", metaName(meta, varId),
        *((double *) metaValue(dm, meta, varId)));
    else if (metaType(meta, varId) == 15) /* calendar */
      display("%s, kalender  ->", metaName(meta, varId));
    else if (metaType(meta, varId) == 4) /* int vec */
      display("%s, int[]  ->", metaName(meta, varId));
    else if (metaType(meta, varId) == 5) /* float vec */
      display("%s, float[]  ->", metaName(meta, varId));
    else 
      display("%s - - -", metaName(meta, varId));

    if (keyDown()) {
      if (REPEATstate == IDLE) {
        deltatime(&tid1);     /* init counter */
/*         thisKey = key();  */
      }
      REPEATdelayed = 0;
      while (keyDown()) {
        REPEATdelayed += deltatime(&tid1);
        if (REPEATstate == IDLE && REPEATdelayed > START_REPEAT_TIME) {
          REPEATstate = REPEAT;
          break;
        }
        if (REPEATstate == REPEAT && REPEATdelayed > REPEAT_TIME) {
          break;
        }
      }
      keyvalue = key();
    } else {
      keyvalue = NO_KEY;
      REPEATstate = IDLE;
    }
        
    if (keyvalue == KEY_HELP /* KEY_ENTER */ /* CHANGE    changed 920107 */ )
      return 0;
    else if (keyvalue == KEY_DOWN) {
      varId ++;
      if (!metaName(meta, varId))
        varId --;
    } else if (keyvalue == KEY_UP) {
      if (varId > 1)
        varId --;
    } else if (keyvalue == KEY_CHANGE  /* HELP changed 920107 */ ) {
      if (editVariable(dm, meta, varId))
        return 0;
      timer = time(0);
      lcdcld();
    } else if (keyvalue == KEY_RIGHT) {
      if (metaType(meta, varId) == 15)
        enter_calendar(metaValue(dm, meta, varId));
      else if (metaType(meta, varId) == 4)
        enter_vec(metaName(meta, varId), metaValue(dm, meta, varId), 4, metaSize(meta, varId));
      else if (metaType(meta, varId) == 5)
        enter_vec(metaName(meta, varId), metaValue(dm, meta, varId), 5, metaSize(meta, varId));
      lcdprintf("\f");
    } else if ((keyvalue >= 0) && (keyvalue < 10)) {
      varId = keyvalue * (noOfItems - 1);
      varId /= 9;
      varId ++;
    }
  }  
}

int enter_vec(name, varPtr, typ, siz)
char *name, *varPtr;
int typ, siz;
{
  int idx = 0, maxItms, key;
  double v1, v2;
  
  maxItms = siz / (typ == 4 ? sizeof(long) : sizeof(double));
  while (1) {
    lcdprintf("\f");
    if (typ == 4) 
      display("%s[%d] = %d          ", name, idx, ((int *) varPtr)[idx]);
    else if (typ == 5)
      display("%s[%d] = %g          ", name, idx, ((double *) varPtr)[idx]);

    key = getKey();
    if (key == KEY_CHANGE) {
      v1 = (typ == 4) ? ((int *) varPtr)[idx] : ((double *) varPtr)[idx];
      v2 = getValue(v1);
      if (v1 != v2) {
        if (typ == 4) 
          ((int *) varPtr)[idx] = v2;
        else
          ((double *) varPtr)[idx] = v2;
      }
    } else if (key == KEY_UP) {
      if (idx > 0)
        idx --;
    } else if (key == KEY_DOWN) {
      idx ++;
      if (idx >= maxItms)
        idx --;
    } else {
      return 0;
    }
  }
}


/*
!   save as ctime except in swedish,
!   "Sat Dec 15 19:20:08 1990"  ->
!   "L|r 15 Dec 1990 19:20:08"
*/
/* char *  */ int swdctime(tid)
time_t *tid;
{
  time_t tid2;
  static char buff[50];
  struct tm tmbuff;
  static char *weekdays[] = {"S\24n", "M\06n", "Tis", "Ons", "Tor", "Fre", 
                            "L\24r" };
/*
  static char *weekdays[] = {"Man", "Tis", "Ons", "Tors", "Fre", 
                            "Lor", "Son" };
*/
  static char *month[] = {"Jan", "Feb", "Mar", "Apr", "Maj", "Jun", "Jul",
      "Aug", "Sep", "Okt", "Nov", "Dec"};
    
/*  if (!tid) {   */      /* removed 921111, only supports current time */
    tid2 = time(0);
    tid = &tid2;
/*  }   */
  memcpy(&tmbuff, localtime(tid), sizeof(struct tm));
  sprintf(buff, "%s %2d %s %d, %02d:%02d:%02d\000",
    weekdays[tmbuff.tm_wday],
    tmbuff.tm_mday,
    month[tmbuff.tm_mon],
    tmbuff.tm_year + ((tmbuff.tm_year > 70) ? 1900 : 2000),
    tmbuff.tm_hour,
    tmbuff.tm_min,
    tmbuff.tm_sec);
  return buff;
}

/*
int xxxswdctime(tid)
int tid;
{
  return (int) swdctime(tid);
}
*/

char *short_time(tid)
long *tid;
{
  static char buf[25];
  struct tm tmbuff;

  if (!*tid) {
    strcpy(buf, "               ");
  } else {
    memcpy(&tmbuff, localtime(tid), sizeof(struct tm));
    sprintf(buf, "%02d%02d%02d %02d:%02d:%02d", 
      tmbuff.tm_year,
      tmbuff.tm_mon + 1,
      tmbuff.tm_mday,
      tmbuff.tm_hour,
      tmbuff.tm_min,
      tmbuff.tm_sec);
  }
  return buf;
}                   
/*
!   Shows the main screen for alarms, it's either one of 
!   'Inga larm i listan'
!
!          or
!
!   'xx okvitterade alarm, yy kvitterade'
!   'xx A, xx B, xx C, xx D larm'
!
*/
showMainPicture(aldm, aldm2, antal, last)
struct _alarmModule *aldm;
struct _alarmModule2 *aldm2;
int *antal, *last;
{
  int cnt, i, count1, count2, blocked;
  char types[4];

  *antal = *last = 0;
  if (!aldm->alarmListPtr)
  {
/*
    display("Inga larm i listan                  \n");
    display("                                    ");
*/
    display("Inga larm i listan\03\n");
    return 0;
  }
  cnt = aldm->alarmListPtr;
  count1 = count2 = blocked = 0;
/*
!   count1 will be unconfirmed alarms
!   count2 will be confirmed but still active alarms
*/    
  types[0] = types[1] = types[2] = types[3] = 0;
  for (i = 0; i < cnt; i++) {
/*    if (aldm->alarmList[i].active) removed 920922   /* added 920617 !! */
      if (aldm2->alarmPts[aldm->alarmList[i].alarmIndex].disable)
        blocked ++;
    if (!aldm->alarmList[i].confirm) {
      count1 ++;
      types[aldm->alarmList[i].class & 0x03] ++;
    } else if (aldm->alarmList[i].active) {
      count2 ++;
      types[aldm->alarmList[i].class & 0x03] ++;
    }
  }
#define HUFVUD_REQUEST_ITEM_6
#ifdef HUFVUD_REQUEST_ITEM_6
/*
!   'xx okvitt., yy kvitterade, zz block.    '
!   'xx A, xx B, xx C, xx D larm'
*/
  display("%2d okvitt., %2d kvittera%s, %2d block.\n",
              count1, 
              count2, 
              (count2 == 1) ? "t" : "de",
              blocked
              );
#else
/*
!   'xx okvitterade larm, yy kvitterade      '
!   'xx A, xx B, xx C, xx D larm'
*/
  display("%2d okvittera%s larm, %2d kvittera%s\n",
              count1, 
              (count1 == 1) ? "t" : "de",
              count2, 
              (count2 == 1) ? "t" : "de"
              );
#endif              
  display("%2d A, %2d B, %2d C, %2d D larm", 
                      types[0], types[1], types[2], types[3]);
  lcdshowdirections(1, 1);              /* up */
  lcdshowdirections(2, 0);              /* left */
  lcdshowdirections(3, 0);              /* right */
  lcdshowdirections(4, 1);              /* down */
  *antal = cnt;
  *last = count1 + count2;
  return 1;
}  

/*
    0     No alarm exists, normal condition, inactive, confirmed, no alarm
    1     inactive, not confirmed,    GREEN
    2     active, confirmed           YELLOW
    3     active, not confirmed       RED
*/
int alarmStatus(no)
int no;
{
  struct _alarmModule *aldm;
  struct _alarmEntry *alarmList;
  int i, max, sts;
  char *headerPtr1;
  
  aldm = (char *) linkDataModule("ALARM", &headerPtr1);
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
  unlinkDataModule(headerPtr1);
  return sts;
}

int alarm_active(no)
int no;
{
  struct _alarmModule2 *aldm2;
  struct _system *sysVars;
  struct _alarmPt *alarmPt;
  int i, j;
  
  sysVars = (struct _system *) SYSTEM_AREA;
  aldm2 = (struct _alarmModule2 *) sysVars->ptr2AlarmModule;
  j = aldm2->noOfAlarmPts;
  alarmPt = aldm2->alarmPts;
  for (i = 0; i < j; i++, alarmPt++)
    if (no == alarmPt->alarmNo)
      return alarmPt->active;
  return 0;
}

int accessDenied()
{
    display("\fF\024r l\06g beh\024righetsniv\06 !");
}

/*
!   shows a list of alarms, both confirmed and not
!
!   Main display consists of either 
!
!   'Inga larm i listan' 
!
!         or 
! 
!   'xx okvitterade alarm, yy kvitterade'
!   'Tryck nedatpil for forsta'
*/
int readAlarm(aldm, LOCAL_PRINTER)
struct _alarmModule *aldm;
int LOCAL_PRINTER;
{
  int cnt, key, i, quit, last, showNo, stepUp, 
      serialNo, status, timer;
  char sInit[25], sOff[25], types[4];
  struct _alarmModule2 *aldm2;
    
#define ADDED_920615
#ifdef ADDED_920615
  if (!checkAccessLevel(PRIV_M_READ_ALARM /* MIN_LEVEL_READ_ALARM */))
    return 0;
#else
  if (getLevel() < MIN_LEVEL_READ_ALARM) {
    accessDenied();       /* write the message 'for lag behorig..'  */
    sleeptight(2);
    return 0;
  }
#endif

  aldm2 = (struct _alarmModule2 *)
            (((char *) aldm) +
              (aldm->noOfAlarmEntries * sizeof(struct _alarmEntry) +
               sizeof(short) + sizeof(long)));
/*
!
!
!
*/
  while (1)
  {
    display("\f");
    lcdsetCacheCursor(1);     /* don't want cursor sweeping over display */
    timer = time(0);
    do {
      lcdhome();
      status = showMainPicture(aldm, aldm2, &cnt, &last);
      if ((time(0) - timer) > TIMEOUT_VALUE)
        return 1;               /* 1 == timeout */
    } while (!keyDown());
    if (((key = getKey()) != KEY_DOWN) && (key != KEY_UP))
      return (key == NO_KEY) ? 1 : 0;
    if (status == 0) {          /* 'Inga larm i listan' */
      return 0;
    }
    cnt = aldm->alarmListPtr;
    display("\f");
    if (key == KEY_DOWN) {
      i = 0;
      showNo = 1;
      stepUp = 1;
    } else {
      i = cnt - 1;
      showNo = last;
      stepUp = -1;
    }
/*
!     algorithm:
!                - find next alarm entry to show, i.e. the alarm
!                  entry that is active or has not been confirmed.
!
!                - get its serial number
!
!                - start a do-while loop
!                - lookup information about current serial number
!                  if not available show the text 'Alarm entry removed'
!
!                - repeat from stage 1 when the user has pressed any valid key
*/

    for (quit = 0; i >= 0 && i < cnt && !quit; /* i++ */ )
    {
      char buf[255], dispBuf[40], bufIndex = 0;

      if (aldm->alarmList[i].confirm && !aldm->alarmList[i].active &&
          !aldm2->alarmPts[aldm->alarmList[i].alarmIndex].disable) { /* 920922 */
        i += stepUp;
        continue;
      }
      serialNo = aldm->alarmList[i].serialNo;
      strcpy(buf, aldm->alarmList[i].string);
      strncpy(dispBuf, buf, 38);
      dispBuf[38] = 0;
      bufIndex = 0;
      display("\f");
      lcdsetCacheCursor(1);     /* don't want cursor sweeping over display */
      timer = time(0);
      do {
        lcdhome();
        cnt = aldm->alarmListPtr;
        for (i = 0; i < cnt; i++) {
          if (aldm->alarmList[i].serialNo == serialNo)
            break;
        }
        if (i >= cnt) {
          display("Larmet \004r raderat !                   \n");
          display("                                      ");
        } else if (aldm->alarmList[i].confirm && !aldm->alarmList[i].active &&
            !aldm2->alarmPts[aldm->alarmList[i].alarmIndex].disable) /* 920922 */
        {
          display("Larmet \004r raderat !                   \n");
          display("                                      ");
        } else {
          strcpy(sInit, short_time(&aldm->alarmList[i].initTime));
          strcpy(sOff, short_time(&aldm->alarmList[i].offTime));
          display("%c%c%c %02d,%s,%s", 
    (aldm2->alarmPts[aldm->alarmList[i].alarmIndex].disable ? 'b' :
               (aldm->alarmList[i].confirm) ? '*' : ' '),
                  aldm2->alarmPts[aldm->alarmList[i].alarmIndex].active ?
                             '\x08' : '\x09',
                  aldm->alarmList[i].class + 'A',
                  showNo, /* i + 1, */
                  sInit, sOff);
          lcdpos(1, 0);
          display(dispBuf);
          lcdshowdirections(1, 1);                               /* up */
          lcdshowdirections(4, 1);                               /* down */
          lcdshowdirections(3, ((strlen(buf) - bufIndex) > 38)); /* right */
          lcdshowdirections(2, (bufIndex > 0));                  /* left */
        }
        if ((time(0) - timer) > TIMEOUT_VALUE)
          return 1;                               /* 1 == timeout */
        if (keyDown()) {
          key = getKey();
          if (key == KEY_LEFT) {
            if (bufIndex > 0) {
#ifdef RIGHT_ARROW_ONCE_ADDED_920617
              bufIndex = 0;
#else
              bufIndex --;
#endif
              strncpy(dispBuf, &buf[bufIndex], 38);
            }
          } else if (key == KEY_RIGHT) {
            if ((strlen(buf) - bufIndex) > 38) {
#ifdef RIGHT_ARROW_ONCE_ADDED_920617
              bufIndex = strlen(buf) - 38;
#else
              bufIndex ++;
#endif
              strncpy(dispBuf, &buf[bufIndex], 38);
            }
          } else if (key == KEY_HELP) {
            display("\f\x08 larmet \004r fortfarande aktivt\n");
            display("\x09 larmet \004r ej l\004ngre aktivt");
            lcdshowdirections(1, 0);
            lcdshowdirections(4, 0);
            lcdshowdirections(3, 0);
            lcdshowdirections(2, 0);
            if ((key = getKey()) == NO_KEY) 
              return 1;                         /* 1 == timeout */
            display("\f");
          } else {    /* another key */
            break;
          }
        }
      } while (1);

      if (key == KEY_ENTER) 
      {
        if (aldm->alarmList[i].confirm == 0)
        {
#ifdef ADDED_920615
          if (!checkAccessLevel(PRIV_M_CONFIRM_ALARM /* MIN_LEVEL_CONFIRM_ALARM */))
            ;
          else {
#else
          if (getLevel() >= MIN_LEVEL_CONFIRM_ALARM) {
#endif            

#ifdef _V920914_1_40
            aldm->alarmList[i].confirm = 1;
            aldm->alarmList[i].sendStatus |= ALARM_SEND_CONFIRM;
            aldm->alarmList[i].confirmTime = time(0);
#else
            aldm->alarmList[i].confirm = 1;
            aldm->alarmList[i].sendConfirm = 0xff;      /* 920129 1->255 */
            aldm->alarmList[i].confirmTime = time(0);
#endif
            i++;                                  /* enter => next */
            showNo++;
            stepUp = 1;
#ifdef ADDED_920615
	  }
#else
  	  } else {      /* no priveledge for operation */
            accessDenied();       /* write the message 'for lag behorig..'  */
            sleeptight(2);
  	  }
#endif
  	} else {        /* already confirmed */
  	  i++;
          showNo++;
          stepUp = 1;
  	}         /* end of if (confirmed == 0) */
      } else if (key == KEY_PLUS_MINUS) {
#ifdef ADDED_920615
        if (!checkAccessLevel(PRIV_M_DISABLE_ALARM /* MIN_LEVEL_DISABLE_ALARM */))
          ;
        else {
#else
        if (getLevel() >= MIN_LEVEL_DISABLE_ALARM) {
#endif            

          int currentState;
          
          currentState = aldm2->alarmPts[aldm->alarmList[i].alarmIndex].disable;
#ifdef _V920914_1_40
          if (currentState) {   /* enable alarm */
            aldm->alarmList[i].enableSent = 0;
            aldm->alarmList[i].enableTime = time(0); 
            aldm->alarmList[i].sendStatus |= ALARM_SEND_ENABLE;
          } else {              /* disabel alarm */
            aldm->alarmList[i].disableSent = 0;
            aldm->alarmList[i].disableTime = time(0); 
            aldm->alarmList[i].sendStatus |= ALARM_SEND_DISABLE;
          }
#else
          aldm->alarmList[i].sendDisable = 0xff;      /* 920129 1->255 */
          aldm->alarmList[i].disableSent = 0;
#endif          
          aldm->alarmList[i].disable = 
              aldm2->alarmPts[aldm->alarmList[i].alarmIndex].disable = 
                (currentState ? 0 : 1);
/*
!   automatic confirm when disable of an alarm
*/
          if (aldm2->alarmPts[aldm->alarmList[i].alarmIndex].disable &&
                  (aldm->alarmList[i].confirm == 0))
          {
#ifdef _V920914_1_40
            aldm->alarmList[i].sendStatus |= ALARM_SEND_CONFIRM;
#else
#endif
            aldm->alarmList[i].confirm = 1;
            aldm->alarmList[i].confirmTime = time(0); 
          }
#ifdef ADDED_920615
        }
#else
	} else {      /* no priveledge for operation */
	    accessDenied();
            sleeptight(2);
  	}
#endif
      } else if (key == KEY_DOWN) {
        i++;
        showNo++;
        stepUp = 1;
      } else if (key == KEY_UP) {
        i--;
        showNo--;
        stepUp = -1;
      } else if (key == NO_KEY) {
        return 1;   /* timeout */
      } else {
        quit = 1;
      }
    }       /* end for */
    if (i >= cnt) {
      display("\fInga fler larm");
      lcdshowdirections(1, 1);              /* up */
      lcdshowdirections(2, 0);              /* left */
      lcdshowdirections(3, 0);              /* right */
      lcdshowdirections(4, 1);              /* down */
      key = getKey();
      continue;
    }
    if (i >= cnt || quit)
      break;
  }     /* end while (1) */
  return 0;
}
