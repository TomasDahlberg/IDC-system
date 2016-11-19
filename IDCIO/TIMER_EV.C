/* timer.c  1991-08-03 TD,  version 1.1 */
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


#include <time.h>
#define NO_OF_ALARMS 1
#include "../alarm.h"

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
*/
int weekNo();

int getTime(year, month, day, hour, min, sec, weekday, weekyear, weekno, color)
long *year, *month, *day, *hour, *min, *sec, 
                    *weekday, *weekyear, *weekno, *color;
{
  static char cache_year, cache_month, cache_day, cache_color;
  static short int cache_week;
  short int week;
  long c_time;
  struct tm t1;
  
  c_time = time(0);
  memcpy(&t1, localtime(&c_time), sizeof(struct tm));
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
}

static struct {
  long initTime;
  long delay;
  int check;
} tidkanal[32];


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
static int TimeResource = 0;

int initTimerResource()
{
  if ((TimeResource = _ev_link("TIMEresource")) == -1) {
    return 0;
  }
  return 1;
}
*/

long getRelTime(dummy)
int dummy;
{
  int sts;
  time_t x, off;

/*
  if ((TimeResource > 0) || initTimerResource())
  {
    do {
      if ((sts = _ev_wait(TimeResource, 1, 1)) == -1) {
        TimeResource = 0;
        return 0;
      }
    } while (sts != 1) ;

    x = time(0);
    off = *offset;
    _ev_signal(TimeResource, 0);
  }
*/
/* 
  if (*debug) printf("getRelTime: offset = %d, now = %d, returns = %d\n",
                        off, x, x - off);
*/                        
  return x - off;
}

int getSummerPeriod(start, stop)
int *start, *stop;
{
  char *dm, *meta, *headerPtr1, *headerPtr2;
  int level, id;
  
  dm = (char *) linkDataModule("VARS", &headerPtr1);
  meta = (char *) linkDataModule("METAVAR", &headerPtr2);
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

  if (summerTime(&timebuf))
    sysVars->summerTime = 1;
  else
    sysVars->summerTime = 0;
  
  previous = time(0);
  setime(&timebuf);
  now = time(0);      /* well, time may have changed slightly, but so what.. */
  (*offset) += (now - previous);
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
 
/*
  if (*debug) printf("account: start = %d, now = %d, diff = %d\n",
                        context[no].start, now, diff);
*/
  context[no].start = now;
  context[no].condition = condition;
  return (a_time) ? (*a_time) += diff : diff;
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
