/* timer.c  1992-09-23 TD,  version 1.31 */
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
! timer.c
! Copyright (C) 1991, IVT Electronic AB.
*/

/*
!   history:
!   date       by  rev  ed#  what
!   ---------- --- ---  ---  ------------------------------------
!   1991-??-?? td  1.00   1  initial coding
!
!     91-12-19 td  1.20      
!
!     92-09-21 td  1.30      added firstFreeTimer(), returns the first free ch
*/

#include <time.h>
#define NO_OF_ALARMS 1
#include "alarm.h"
#include "sysvars.h"
#include "idcio.h"

char *NAME_OF_VAR_MODULE = {"VARS"};
char *NAME_OF_META_MODULE = {"METAVAR"};

static struct _system *sysVars = SYSTEM_AREA;

/*
!   getTime()            returns year,month,day,hour,min,sec,
!                        weekday,weekyear,weekno
!   getRelTime()         returns time independent of any setime
!   setTime()            sets the system time and updates the offset variable
!   account()            accumulate up to 16 drift counters
!   isTimerInit()        checks if timer is running
!   timerCancel()        cancels a timer
!   timerInit()          initialize a timer for specified time intervall
!   timerReady()         tests if timer has expired
!   timerLeft()          returns min and sec to go
!   firstFreeTimer()     returns the channel no of the first free timer, or -1
*/
int weekNo();
/* returns 1 if tomorrow is another month ! 920923 */
int getTime(year, month, day, hour, min, sec, weekday, weekyear, weekno, color)
long *year, *month, *day, *hour, *min, *sec, 
                    *weekday, *weekyear, *weekno, *color;
{
  long c_time;
  c_time = time(0);
  return getTime4(c_time,
      year, month, day, hour, min, sec, weekday, weekyear, weekno, color);
}
		    
int getTime4(c_time,
    year, month, day, hour, min, sec, weekday, weekyear, weekno, color)
long c_time, *year, *month, *day, *hour, *min, *sec, 
                    *weekday, *weekyear, *weekno, *color;
{
  static char cache_year, cache_month, cache_day, cache_color;
  static short int cache_week;
  short int week, last_day_in_month;
  struct tm t1;
  long c_time_tomorrow;
  
  c_time_tomorrow = c_time + 86400;
  memcpy(&t1, localtime(&c_time_tomorrow), sizeof(struct tm));
  last_day_in_month = t1.tm_mon;
  memcpy(&t1, localtime(&c_time), sizeof(struct tm));
  if (last_day_in_month == t1.tm_mon)
    last_day_in_month = 0;
  else
    last_day_in_month = 1;
  if (year) *year = t1.tm_year;
  if (month) *month = t1.tm_mon + 1;
  if (day) *day = t1.tm_mday;
  if (hour) *hour = t1.tm_hour;
  if (min) *min = t1.tm_min;
  if (sec) *sec = t1.tm_sec;
  if (weekday) *weekday = (t1.tm_wday == 0) ? 6 : t1.tm_wday - 1;
  if (cache_day == t1.tm_mday && cache_month == t1.tm_mon && 
                                              cache_year == t1.tm_year)
  {
    week = cache_week;
    if (color) *color = cache_color;
  }
  else
  {
    week = weekNo(c_time);
    if (color)
    {
      int cl;
      cl = colour_of_day( t1.tm_year + ((t1.tm_year > 70) ? 1900 : 2000),
                                t1.tm_mon + 1, t1.tm_mday);
      *color = (cl == 3) ? 4 : cl;
    }
  }
  if (weekyear) *weekyear = week / 100;
  if (weekno) *weekno = week % 100;
  return last_day_in_month;
}

static struct {
  long initTime;
  short int delay;
  short int check;
  
/*  long delay;     */
/*  int check;      */
} *tidkanal = 0x003fe00;   /* [32]; */    /* size = 8 * 32 = 256 bytes */


/*static long offset;*/

unsigned long *offset = 0x003ffdc;


/*
struct sgtbuf {
	char    t_year,
			t_month,
			t_day,
			t_hour,
			t_minute,
			t_second;
};
*/

/* unsigned int *debug = 0x003ffd8; */

/*
! Syntax:
!   lock(char *a)  
!                    set bit 7 of (*a) (i.e none zero)
!                    and returns 1 if bit7 already was set, otherwise 0
*/
#asm
lock:
  movea.l d0,a0
  tas     (a0)
  bne.s   none_0
  clr.l   d0
  bra.s   ok
none_0
  moveq.l #1,d0
ok
  rts
#endasm


long getRelTime(dummy)
int dummy;
{
  int sts;
  time_t x, off;

  while (lock(&sysVars->tid.dummy))
    ;
/*    critical section */

  x = time(0);
  off = *offset;
  
  sysVars->tid.dummy = 0;
  return x - off;
}

int getSummerPeriod(start, stop)
int *start, *stop;
{
  char *dm, *meta, *headerPtr1, *headerPtr2;
  int level, id;
  
  dm = (char *) linkDataModule(NAME_OF_VAR_MODULE, &headerPtr1);
  meta = (char *) linkDataModule(NAME_OF_META_MODULE, &headerPtr2);
  if (!dm || !meta)
    return -1;

  if ((id = metaId(meta, "summerStart")) > 0)
    *start = *((int *) metaValue(dm, meta, id)) ;
  else
    *start = 0;
  if ((id = metaId(meta, "winterStart")) > 0)
    *stop = *((int *) metaValue(dm, meta, id)) ;
  else
    *stop = 0;

  unlinkDataModule(headerPtr1);
  unlinkDataModule(headerPtr2);
  return (*start && *stop) ? 1 : 0;
}

int summerTime(tm)
struct sgtbuf *tm;
{
  int summerStart, winterStart;

  if (!getSummerPeriod(&summerStart, &winterStart))
    return 0;     /* not definied -> always none dayligth saving time */

  if ((summerStart / 10000) == tm->t_year) {
    if ((summerStart % 10000) < 100*tm->t_month + tm->t_day) {
      if ((winterStart / 10000) == tm->t_year) {
        if ((winterStart % 10000) > 100*tm->t_month + tm->t_day)
          return 1;
        else if ((winterStart % 10000) == 100*tm->t_month + tm->t_day) {
          if (tm->t_hour < 3)
            return 1;
        }
      } else
        return 1;
    } else if ((summerStart % 10000) == 100*tm->t_month + tm->t_day) {
      if (tm->t_hour >= 3)
        return 1;
    }
  }
  return 0;
}

static unsigned char *wdst = 0x00348000;

int setTime(year, month, day, hour, minute, second)
int year, month, day, hour, minute, second;
{
  long previous, now;
  struct sgtbuf timebuf;
  
  timebuf.t_year = year;
  timebuf.t_month = month;
  timebuf.t_day = day;
  timebuf.t_hour = hour;
  timebuf.t_minute = minute;
  timebuf.t_second = second;

  *wdst = 0;
  if (summerTime(&timebuf))
    sysVars->summerTime = 1;
  else
    sysVars->summerTime = 0;
  *wdst = 0;
  
  while (lock(&sysVars->tid.dummy))
    ;
/*    critical section */
  *wdst = 0;

  previous = time(0);
  setime(&timebuf);
  now = time(0);      /* well, time may have changed slightly, but so what.. */
  (*offset) += (now - previous);

  sysVars->tid.dummy = 0;

  *wdst = 0;
}

long account(no, condition, a_time)
int no, condition;
long *a_time;
{
  static struct _context { short condition; long start; } context[32];
  long now, diff;
  no &= 0x1f;
  now = getRelTime(0);
  if (!context[no].condition && condition)    /* rising edge */
  {
    context[no].start = now;  diff = 0;
  } else if (context[no].condition && !condition) /* falling edge */
  {
    diff = now - context[no].start;
  } else if (condition)
  {
    diff = now - context[no].start;
  } else
    diff = 0;
 
  context[no].start = now;
  context[no].condition = condition;
  return (a_time) ? (*a_time) += diff : diff;
}

/*   Pseudo for firstFreeTimer(0, 31)
! ch = 0;
! while (ch < 32 && isTimerInit(ch))
!   ch ++;
! if (ch >= 32) ch = -1;
*/
int firstFreeTimer(min, max)
int min, max;
{
  int ch;
  for (ch = min; ch <= max; ch++)
    if (tidkanal[ch & 0x1f].check == 0)
      return ch;
  return -1;
}

int timerCancel(ch)
{
  tidkanal[ch & 0x1f].check = 0;
}

int isTimerInit(ch)
int ch;
{
  return tidkanal[ch & 0x1f].check;
}

int timerInit(ch, min, sec)
int ch, min, sec;
{
  tidkanal[ch & 0x1f].initTime = getRelTime(0);
  tidkanal[ch & 0x1f].delay = min*60 + sec;
  tidkanal[ch & 0x1f].check = 1;
}
/*
!   Returns minutes, seconds and total seconds to go for timer channel
!
!   IDC syntax:   int timerLeft(int, int&, int&);
!   (or int timerLeft(int, int, int); and call s = timerLeft(3, 0, 0); )
*/
int timerLeft(ch, min, sec)
int ch, *min, *sec;
{
  int s;
  if (tidkanal[ch & 0x1f].check == 0) {
    if (min) *min = 0;
    if (sec) *sec = 0;
    return 0;
  }

  s = (tidkanal[ch & 0x1f].delay - 
                  (getRelTime(0) - tidkanal[ch & 0x1f].initTime));
  if (s < 0) {
    s = 0;
  }
  if (sec) (*sec) = s % 60;
  if (min) (*min) = s / 60;
  return s;
}

int timerReady(ch)
int ch;
{
  if (tidkanal[ch & 0x1f].check == 0)
    return 0;

  if (getRelTime(0) - tidkanal[ch & 0x1f].initTime >= 
                              tidkanal[ch & 0x1f].delay) {
    tidkanal[ch & 0x1f].check = 0;
    return 1;
  } else
    return 0;
}

#ifdef DEBUG
/*
!     exempel
*/
main()
{
  
  initTimer(1,    /* kanal */
            0,    /* minuter */
            10    /* sekunder */
            );
  
  while (1)
  {
    if (testTimer(1))      /* timer 1  ON */
    {
      initTimer(3,    /* kanal */
                0,    /* minuter */
                5     /* sekunder */
                );

      printf("start fan\n");
    }  

    if (testTimer(3))      /* timer 1  ON */
    {
      printf("stop fan\n");      
    }  
  }
}
#endif
