/* look.c  1992-01-23 TD,  version 1.3 */
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
! look.c
! Copyright (C) 1991,1992 IVT Electronic AB.
*/


/*
!   look        - list and change variables and locks on alarms and timers
! 
!   History:
!   Date        by   rev   what
!   ----------  ---  ----  ---------------------------------------------
!   1991-02-19  TD   1.00  initial coding
!   1991-05-06  TD   1.01  code shaped to fit new alarm routines
!   1991-07-24  TD   1.02  first test of calendar and vector
!   1991-08-05  TD   1.03  locked -> disable on alarms
!   1991-12-11  TD   1.20  added () around lock2 calculation
!   1992-01-23  TD   1.30  changed old timer funktion to new (i.e. timerReady)
!
!   Function:
!   Presentates a menu for possibilities to modify variables and locks on
!   variables, alarms and timers
!
*/
@_sysedit: equ 7
@_sysattr: equ $8004

#include <time.h>
#include "idcio/idcio.h"
#define  NO_OF_ALARMS 1
#include "alarm.h"
#define NAMEOFDATAMODULE "VARS"

struct _lockModule {
    int noOfAlarms;
    int alarmLock[1];
} *lock;

struct _lockModule2 {
    int noOfTimers;
    int timerLock[1];
} *lock2;

char *dm;

struct _alarmModule *aldm;
struct _alarmModule2 *aldm2;

#define NO_OF_CAL_ENTRIES 10
struct _calendar {
  unsigned short day[NO_OF_CAL_ENTRIES];
  unsigned short stopday[NO_OF_CAL_ENTRIES];
  unsigned char color[NO_OF_CAL_ENTRIES];
  unsigned short start[NO_OF_CAL_ENTRIES];
  unsigned short stop[NO_OF_CAL_ENTRIES];
};

#ifndef OLD_TIMER
static struct {
  long initTime;
  short int delay;
  short int check;
} *tidkanal = 0x003fe00;   /* [32]; */    /* size = 8 * 32 = 256 bytes */
#endif

main(argc, argv)
int argc;
char *argv[];
{
  int choise, id;
  char *headerPtr1, *headerPtr2, *headerPtr3, *headerPtr4;
  char *meta, dmName[10];
  
  strcpy(dmName, "VARS");
  if (argv[1][0] == '-')
  {
    if (argv[1][1] == '2')
      strcpy(dmName, "VAR2");
    if (argv[1][1] == '1')
      strcpy(dmName, "VAR1");
    if (argv[1][1] == '?') {
      printf("Usage: \n  -1  uses data module VAR1 instead of VARS\n");
      printf("  -2  uses data module VAR2 instead of VARS\n");
      exit(0);
    }
  }
   
  initidcio();
  
  dm = (char *) linkDataModule(dmName, &headerPtr1);
  if (!dm) {
    printf("cannot find '%s'\n", dmName);
#ifdef VERY_MUCH_EPROM
    printf("cannot link to datamodule '%s'\n", dmName);
    printf("check if process 'scan' is running\n");
#endif
    return 0;
  }

  meta = (char *) linkDataModule("METAVAR", &headerPtr3);
  if (!meta) {
    printf("METAVAR ?\n");
#ifdef VERY_MUCH_EPROM
    printf("cannot link to datamodule '%s'\n", "METAVAR");
    printf("check if process 'scan' is running\n");
#endif
    return 0;
  }

  while (1) {
    printf("\n1. variable:  list all vars\n");
    printf("2.            change variable\n");
    printf("3.            lock/unlock all I/O bound variables\n");
    printf("4. alarm:     list all disables\n");
    printf("5.            enable/disable alarm\n");
    printf("6.            enable/disable all alarms\n");
#ifdef OLD_TIMER
    printf("7. timer:     list all locks\n");
    printf("8.            change locks on timer\n");
    printf("9.            lock/unlock all timers\n");
#else
    printf("7. timer:     list all timers\n");
    printf("8.            change delay of timer\n");
    printf("9.            clear all timers\n");
#endif
    printf("0. quit\n");
    printf("\nSelect an item : ");
    scanf("%d", &choise);
    if (choise == 1) {
      int mx;
      id = 1;
      mx = getSize(meta);
      while (metaName(meta, id)) {
        showVar(meta, id++, mx);
      }
    } else if (choise == 2) {
      printf("Var id: ");
      scanf("%d", &id);
      showVar(meta, id, 0);
      editVar(dm, meta, id);    
      showVar(meta, id, 0);
    } else if (choise == 3) {       /* unlock/lock all I/O bound variables */
      int id, value;
      printf("Var id : ");
      scanf("%d", &id);
      
      printf("0=unlock, 1=lock: ");
      scanf("%d", &value);
      if (id == -1) {
        id = 1;
        while (metaName(meta, id)) {
          *((char *) metaLock(dm, meta, id)) = value;
          id ++;
        }
      } else {
        if (metaName(meta, id)) 
          *((char *) metaLock(dm, meta, id)) = value;
      }
    } else if (choise == 4) {       /* list all locks on alarms */
      int id;
      for (id = 0; id < aldm2->noOfAlarmPts; id++) {
        printf("alarm %d  = %s\n", id, (aldm2->alarmPts[id].disable) ?
                                          "disabled" : "enabled");
      }
    } else if (choise == 5) {       /* change locks on alarms */
      int id, value;
      printf("select id (0..%d) : ", aldm2->noOfAlarmPts - 1);
      scanf("%d", &id);
      printf("alarmLock[%d]=%d, ", id, aldm2->alarmPts[id].disable);      
      printf(" new value (enable = 0/1 = disable) : ");
      scanf("%d", &value);
      aldm2->alarmPts[id].disable = value;
/*      lock->alarmLock[id] = value;
*/
      printf("alarmLock[%d] = %d\n", id, aldm2->alarmPts[id].disable);
    } else if (choise == 6) {       /* lock/unlock all alarms */
      int id, value;
      printf("0=enable,1=disable: ");
      scanf("%d", &value);
      
      for (id = 0; id < aldm2->noOfAlarmPts; id++) {
        aldm2->alarmPts[id].disable = value;
        printf("alarm %d  = %s\n", id, (aldm2->alarmPts[id].disable) ? 
                                          "disabled" : "enabled");
      }
#ifdef OLD_TIMERS
    } else if (choise == 7) {       /* list all locks on timers */
      int id;
      for (id = 0; id < lock2->noOfTimers; id++) {
        printf("timer %d  = %s\n", id, (lock2->timerLock[id]) ? 
                                          "locked" : "unlocked");
      }
    } else if (choise == 8) {       /* change locks on timers */
      int id, value;
      printf("select id (0.. %d) : ", lock2->noOfTimers - 1);
      scanf("%d", &id);
      printf("timerLock[%d] = %d, ", id, lock2->timerLock[id]);      
      printf(" new value (0/1) : ");
      scanf("%d", &value);
      lock2->timerLock[id] = value;
      printf("timerLock[%d] = %d\n", id, lock2->timerLock[id]);      
    } else if (choise == 9) {       /* lock/unlock all timers */
      int id, value;
      printf("0 = unlock, 1 = lock : ");
      scanf("%d", &value);
      for (id = 0; id < lock2->noOfTimers; id++) {
        lock2->timerLock[id] = value;
        printf("timer %d  = %s\n", id, (lock2->timerLock[id]) ? 
                                          "locked" : "unlocked");
      }
#else
    } else if (choise == 7) {       /* list all locks on timers */
      int id, now, left;
      char *buf;
      
      now = getRelTime(0);
      for (id = 0; id < 32; id++) {
        if (!tidkanal[id].check) {
          printf("- not initialized -\n");
        } else {
          left = (tidkanal[id].delay - (getRelTime(0) - tidkanal[id].initTime));
          printf("timer %d  %d:%02d  kvar %d:%02d\n", id,
              tidkanal[id].delay / 60, 
              tidkanal[id].delay % 60, 
              left / 60, left % 60);
        }
      }
    } else if (choise == 8) {       /* change locks on timers */
      int id, value, value2, left;
      printf("select id (0..31): ");
      scanf("%d", &id);
      printf("timer %d, delay = %d:%02d\n", id,
       tidkanal[id].delay / 60,
       tidkanal[id].delay % 60);
      printf("new minutes : ");
      scanf("%d", &value);
      printf("new seconds : ");
      scanf("%d", &value2);
      tidkanal[id].delay = value * 60 + value2;
      left = (tidkanal[id].delay - (getRelTime(0) - tidkanal[id].initTime));
      printf("timer %d  %d:%02d  kvar %d:%02d\n", id,
            tidkanal[id].delay / 60, 
            tidkanal[id].delay % 60, 
            left / 60, left % 60);
    } else if (choise == 9) {       /* lock/unlock all timers */
      int id;
      for (id = 0; id < 32; id++) {
        tidkanal[id].check = 0;
      }
#endif
    } else if (choise == 0) {
      break;
    }
  }
  unlinkDataModule(headerPtr1);
  unlinkDataModule(headerPtr2);
  unlinkDataModule(headerPtr3);
  unlinkDataModule(headerPtr4);
}

editVar(dm, meta, varId)
char *dm;
char *meta;
int varId;
{
  int tp;
  int tmpInt;
  double tmpFloat;

  printf("Enter new value ");  
  if ((tp = metaType(meta, varId)) == 7)  /* int */
  {
    printf("(int) : ");
    scanf("%d", &tmpInt);
    *((int *) metaValue(dm, meta, varId)) = tmpInt;
  } else if (tp == 8) /* float */
  {
    float t;
    
    printf("(float) : ");
    scanf("%f", &t);
    tmpFloat = t;
    *((double *) metaValue(dm, meta, varId)) = tmpFloat;
  } else if (tp == 4)  /* int vec */
  {
/*
!   '  7:gt51vector
!   '              [0] = 23'
!   '              [1] = 17'
!   '              [2] = 17'
!   '              [3] = 17'
!   '  8:gt52          = 27.3'
*/    
    int size, *vec, i;
    size = metaSize(meta, varId) / sizeof(long);
    vec = ((int *) metaValue(dm, meta, varId));

    printf("(int []) : index # (0..%d) ", size - 1);
    scanf("%d", &i);
    if (i < 0 || i >= size) {
      printf("index out of range (0..%d)\n", size - 1);
    } else {
      printf("value (int) : ");
      scanf("%d", &tmpInt);
      vec[i] = tmpInt; 
    }
  }
  else if (tp == 5) /* float vec */
  {
    int size, i;
    double *vec;
    
    size = metaSize(meta, varId) / sizeof(double);
    vec = ((double *) metaValue(dm, meta, varId));
    printf("(float []) : index # (0..%d) ", size - 1);
    scanf("%d", &i);
    if (i < 0 || i >= size) {
      printf("index out of range (0..%d)\n", size - 1);
    } else {
      float t;
    
      printf("value (float) : ");
      scanf("%f", &t);
      tmpFloat = t;
      vec[i] = tmpFloat;
    }
  } else if (tp == 15) /* calendar */
  {
    struct _calendar *cal;
    int row, choise, i;

    cal = (struct _calendar *) metaValue(dm, meta, varId);
    
    printf("Row (1..10) ? : ");
    scanf("%d", &row);
    printf("1. Add\n");
    printf("2. Modify\n");
    printf("3. Delete\n");
    printf("choise ? : ");
    scanf("%d", &choise);
    if (choise == 3) {
      for (i = row; i < NO_OF_CAL_ENTRIES; i++)
      {
        cal->day[i - 1] = cal->day[i];
        cal->stopday[i - 1] = cal->stopday[i];
        cal->color[i - 1] = cal->color[i];
        cal->start[i - 1] = cal->start[i];
        cal->stop[i - 1] = cal->stop[i];
      }
    } else
    {
      if (choise == 1) {
        for (i = row - 1; i < NO_OF_CAL_ENTRIES; i++)
          if (cal->day[i] == 0)
            break;
        if (i >= NO_OF_CAL_ENTRIES) {
          printf("No more free entries\n");
          return 0;
        }
        for ( ; i > row - 1; i--)
        {
          cal->day[i] = cal->day[i - 1];
          cal->stopday[i] = cal->stopday[i - 1];
          cal->color[i] = cal->color[i - 1];
          cal->start[i] = cal->start[i - 1];
          cal->stop[i] = cal->stop[i - 1];
        }
      }
/* add entry for row */
      row --;
      printf("1. Date\n");
      printf("2. Weekday\n");
      printf("choise ? : ");
      scanf("%d", &choise);
      if (choise == 1) {
        int start, stop;
        printf("Start date: (ex. 0327) ");
        scanf("%d", &start);
        printf("Stop date: (ex. 0329) ");
        scanf("%d", &stop);
        cal->day[row] = start;
        cal->stopday[row] = stop;
      } else if (choise == 2) {
        printf("Weekday mask (0..127) ? ");
        scanf("%d", &choise);
        printDayMask(choise);
        cal->day[row] = choise | 2048;
      }
      printf("Color:\n0. no color\n1. black\n2. orange\n4. red\n");
      scanf("%d", &choise);
      cal->color[row] = choise;
      printf("%s\n", (cal->color[row] & 1) ? "Black " : 
                                  ((cal->color[row] & 2) ? "Orange" :
                                  ((cal->color[row] & 4) ? "Red   " : "      ")));
      {
        int start, stop;
        printf("Start time: (ex. 1005, hour min) ");
        scanf("%d", &start);
        printf("Stop time: (ex. 1630) ");
        scanf("%d", &stop);
        cal->start[row] = start;
        cal->stop[row] = stop;
      }
    }
  }
}

static char *maskDay[] = {"Mo,","Tu,","We,","Th,","Fr,","Sa,","Su"};

printDayMask(mask)
int mask;
{
  int i;
  
  for (i = 0; i < 7; i++)
  {
    if (mask & (1 << i))
      printf("%s", maskDay[i]);
    else
      printf("%s", i == 6 ? "  " : "   ");    
/*      printf("  %s", i != 6 ? " " : "");    */
  }
}

padString(line, s, pos)
char *line, *s;
int pos;
{
  int i;
  if (pos) {
    pos += 4;
    for (i = 0; i < pos; i++) {
      if (i < strlen(s))
        line[i] = s[i];
      else
        line[i] = ' ';
    }
    line[i] = 0;
  } else
    strcpy(line, s);
}

showVar(meta, varId, pos)
char *meta;
int varId, pos;
{
  char line[160], s[80];
  int i, tp;
  
  sprintf(s, "%3d:%s  ", varId, metaName(meta, varId));
  padString(line, s, pos);
   
  if ((tp = metaType(meta, varId)) == 7) /* int */
    printf("%s = %d\n", line, *((int *) metaValue(dm, meta, varId)));
  else if (tp == 8) /* float */
    printf("%s = %g\n", line, *((double *) metaValue(dm, meta, varId)));
  else if (tp == 15) /* calendar */
  {
    struct _calendar *cal;
    int i;
    
    cal = (struct _calendar *) metaValue(dm, meta, varId);
    printf("%s\n", line);
    for (i = 0; i < NO_OF_CAL_ENTRIES; i++)
    {
      if (cal->day[i] == 0)
      {
        printf("                     !        !               !\n");
        continue;
      }
      if (cal->day[i] & 2048)
      {
        printDayMask(cal->day[i] - 2048);
        printf(" ! ");
      }
      else
        printf("%04d - %04d          ! ", cal->day[i], cal->stopday[i]);
        printf("%s ! ", (cal->color[i] & 1) ? "Black " : 
                                  ((cal->color[i] & 2) ? "Orange" :
                                  ((cal->color[i] & 4) ? "Red   " : "      ")));
      if (cal->start[i] == 0 && cal->stop[i] == 2400)
        printf("Hela dygnet   !\n");
      else
        printf("%02d.%02d - %02d.%02d !\n", 
          cal->start[i] / 100,
          cal->start[i] % 100,
          cal->stop[i] / 100,
          cal->stop[i] % 100);
    }
  }
  else if (tp == 4) /* int vec */
  {
/*
!   '  7:gt51vector
!   '              [0] = 23'
!   '              [1] = 17'
!   '              [2] = 17'
!   '              [3] = 17'
!   '  8:gt52          = 27.3'
*/    
    int size, *vec, i;
    size = metaSize(meta, varId) / sizeof(long);
    vec = ((int *) metaValue(dm, meta, varId));
    printf("%s\n", line);
    for (i = 0; i < 80; i++)
      if (line[i])
        line[i] = ' ';
    for (i = 0; i < size; i++)
    {
      printf("%s[%d] = %d\n", line, i, *vec++);
    }
  }
  else if (tp == 5) /* float vec */
  {
    int size, i;
    double *vec;
    
    size = metaSize(meta, varId) / sizeof(double);
    vec = ((double *) metaValue(dm, meta, varId));
    printf("%s\n", line);
    for (i = 0; i < 80; i++)
      if (line[i])
        line[i] = ' ';
    for (i = 0; i < size; i++)
    {
      printf("%s[%d] = %g\n", line, i, *vec++);
    }
  }
}

getSize(meta)
char *meta;
{
  int i, len, mx;
  mx = 0;
  i = 1;
  while (metaName(meta, i)) {
    len = strlen(metaName(meta, i));
    if (mx < len)
      mx = len;
    i ++;
  }
  return mx;
}
