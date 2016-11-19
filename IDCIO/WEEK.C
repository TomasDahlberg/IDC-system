/* week.c  1991-08-03 TD,  version 1.1 */
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
! week.c
! Copyright (C) 1991, IVT Electronic AB.
*/


#include <time.h>
#define SECONDS_OF_DAY      86400
#define SECONDS_OF_WEEK    604800

/*
! Integer function weekNo(c_time) returns week number in format 'yyww'
! 
! By using functions
!     yearOfTime:  returns year since 1900 of C-time
!     dayofWeek1:  returns C-time of monday 0:00:00 week 1 this year
!  previous31dec:  returns C-time of 31-dec- previous-year
!  weekdayOf1Jan:  returns weekday 1 - 7 for mon-sun of 1-jan-'year'
!
! the algorithm for weekNo(time) is as follows:
!
!     if  time < dayofWeek1 then 
!       week is same as 31-dec of previous year
!     else
!       calculate weekNo as weeks since dayofWeek1(thisyear)
!       if calculated weekNo is 53 and
!          if weekdayOf1Jan(nextyear) is mon-thur
!             weekNo = 1 next year
*/
int weekNo(c_time)
time_t c_time;
{
  time_t c_time_week1;
  int week, year;
  
  year = yearOfTime(c_time);
  c_time_week1 = dayofWeek1(year);
  if ((c_time - c_time_week1) < 0) {     /* before monday, week 1 */
    return weekNo(previous31dec(year));
  }
  week = 1 + (c_time - c_time_week1) / SECONDS_OF_WEEK;
  if (week == 53) {
    if (weekdayOf1Jan(year + 1, 0) <= 4) {
      year ++;
      week = 1;
    }
  }
  return year * 100 + week;    
}

/*
!   utility functions for weekNo
*/

static int yearOfTime(c_time)
time_t c_time;
{
  struct tm t1;
  memcpy(&t1, localtime(&c_time), sizeof(struct tm));
  return t1.tm_year;
}

/* returns C-time for monday 0:00:00 week 1 year 'year' */
static long dayofWeek1(year)
int year;
{
  time_t firstJan, week1;
  int wday;
  
  if ((wday = weekdayOf1Jan(year, &firstJan)) <= 4)   
  {                       /*  it was any of monday - thursday */
    week1 = firstJan - (wday - 1) * SECONDS_OF_DAY;
  } else {
    week1 = firstJan + (8 - wday) * SECONDS_OF_DAY;
  }
  return week1;
}

static time_t previous31dec(year)
int year;
{
  struct tm t1;

  t1.tm_year = year - 1;
  t1.tm_mon = 11;
  t1.tm_mday = 31;
  t1.tm_hour = t1.tm_min = t1.tm_sec = 0;
  return mktime(&t1);
}

/* returns 1 - 7 for monday - sunday */
static int weekdayOf1Jan(year, firstJan)
int year;
time_t *firstJan;
{ 
  struct tm t1, t2;
  time_t t;

  t1.tm_year = year;
  t1.tm_mon = 0;
  t1.tm_mday = 1;
  t1.tm_hour = t1.tm_min = t1.tm_sec = 0;
  t = mktime(&t1);
  memcpy(&t2, localtime(&t), sizeof(struct tm));
  if (firstJan) {
    *firstJan = t;
  }
  return t2.tm_wday ? t2.tm_wday : 7;
}
