/*
.DOCSTART
!+
    !Title    : REDDAY.C
    !Subtitle :
    !Class    : Copyright(c)
    !Year     : 1990
    !Owner    : LEAB
    !Status   : Alfa
    !Version  : 0.1
    !History  :
    !Function : For a given year, month, date returns holiday (=RED day),
		day before holiday (=PINK day) or none of these (BLACK
		day)
    !Portable :
!-
!.DOCEND
*/
/*
!       Updated 920629 by Tomas Dahlberg.
!                       Bugfix in easter() function. Since prerequisite was that
!                       eastern and related holidays were align within months,
!                       the algorithm was changed to use julian days.
!                       Bugfix 2. The - 'midsommar' check didn't handle the 
!                       inequality operators correctely.
!                         
!
*/

/*
#define DEBUG_STATE
#define FIX_920629
*/

#include <stdio.h>
#include <time.h>

#define ERRDAY    0
#define DATE_OK   1
#define BLACK     1
#define PINK      2
#define RED       3
#define MONDAY    1
#define TUESDAY   2
#define WEDNESDAY 3
#define THURSDAY  4
#define FRIDAY    5
#define SATURDAY  6
#define SUNDAY    7

/* The method used here for determing the day of the week was presented by
L T Sacharovski 1956 */

static int is_sunday(year, month, day)
int year, month, day;
{
  int kyh, kdec, ky, kmon, ksum, leap, decade, last_year_digit, y_table;
  if ((year % 4) == 0) leap = 1; else leap = 0;

  /* kyh is determined for each century. We only deal with two centuries. */
  if (year >= 2000)
  {
    kyh = 6;
    last_year_digit = (year - 2000) % 10;
    decade = (year - 2000) - last_year_digit;
  }
  else
  {
    kyh = 0;
    last_year_digit = (year - 1900) % 10;
    decade = (year - 1900) - last_year_digit;
  }

  /* The decade will determine kdec. Later on, in the last digit of the year,
  there are two sets of values. The choice depends upon which decade. */

  switch (decade)
  {
    case  0: kdec = 0; y_table = 1; break;
    case 10: kdec = 5; y_table = 2; break;
    case 20: kdec = 4; y_table = 1; break;
    case 30: kdec = 2; y_table = 2; break;
    case 40: kdec = 1; y_table = 1; break;
    case 50: kdec = 6; y_table = 2; break;
    case 60: kdec = 5; y_table = 1; break;
    case 70: kdec = 3; y_table = 2; break;
    case 80: kdec = 2; y_table = 1; break;
    case 90: kdec = 0; y_table = 2; break;
    default: return(ERRDAY);
  }

  if (y_table == 1)
  {
    switch(last_year_digit)
    {
      case 0: ky = 0; break;
      case 1: ky = 1; break;
      case 2: ky = 2; break;
      case 3: ky = 3; break;
      case 4: ky = 5; break;
      case 5: ky = 6; break;
      case 6: ky = 0; break;
      case 7: ky = 1; break;
      case 8: ky = 3; break;
      case 9: ky = 4; break;
      default: return(ERRDAY);
    }
  }

  if (y_table == 2)
  {
    switch(last_year_digit)
    {
      case 0: ky = 0; break;
      case 1: ky = 1; break;
      case 2: ky = 3; break;
      case 3: ky = 4; break;
      case 4: ky = 5; break;
      case 5: ky = 6; break;
      case 6: ky = 1; break;
      case 7: ky = 2; break;
      case 8: ky = 3; break;
      case 9: ky = 4; break;
      default: return(ERRDAY);
    }
  }

  switch(month)    /* Jan is month 1, Feb is month 2 etc */
  {
    case  1: if (leap == 0) kmon = 0; else kmon = 6; break;
    case  2: if (leap == 0) kmon = 3; else kmon = 2; break;
    case  3: kmon = 3; break;
    case  4: kmon = 6; break;
    case  5: kmon = 1; break;
    case  6: kmon = 4; break;
    case  7: kmon = 6; break;
    case  8: kmon = 2; break;
    case  9: kmon = 5; break;
    case 10: kmon = 0; break;
    case 11: kmon = 3; break;
    case 12: kmon = 5; break;
    default: return(ERRDAY);
  }

  ksum = (kyh + kdec + ky + kmon + day) % 7;
  switch(ksum)
  {
    case 0: return(SUNDAY);
    case 1: return(MONDAY);
    case 2: return(TUESDAY);
    case 3: return(WEDNESDAY);
    case 4: return(THURSDAY);
    case 5: return(FRIDAY);
    case 6: return(SATURDAY);
    default: return(ERRDAY);
  }

}

/* Since Nicea 325, easter is defined as the first sunday after the first
full moon after vernal equnox. The method used here was first described
by Gauss 1800 */

static int easter(year, month, day)
int year, month, day;
{
#ifdef FIX_920629
  int julianDate, time1 = 0, julianEaster, time2 = 0;
#endif  
  int a, b, c, d, e, easter_day, easter_month, tmp_month, tmp_day;
  a = (year % 19);                  /* a .. e are temporary results */
  b = (year % 4);
  c = (year % 7);
  d = (19 * a + 24) % 30;           /* 24 is valid 1900 to 2099 */
  e = (2*b + 4*c + 6*d + 5) % 7;    /*  5 is valid 1900 to 2099 */
  easter_day =  22 + d + e;
  easter_month = 3;
  if (easter_day > 31)
  {
    easter_day = easter_day - 31;
    easter_month = 4;
  }
  /* Now check a few irregularities */
  if ((d == 29) && (e == 6))             easter_day = 19;
  if ((d == 28) && (e == 6) && (a > 10)) easter_day = 18;

#ifdef FIX_920629
/* new since 1992-06-29 */

  julianEaster = julianDate = year << 16;
  julianDate |= (month << 8) | day;
  julianEaster |= (easter_month << 8) | easter_day;
  _julian(&time1, &julianDate);
  _julian(&time2, &julianEaster);


  if (julianDate == julianEaster + 1) return RED;
  if (julianDate == julianEaster - 1) return PINK;
  if (julianDate == julianEaster - 2) return RED;
  if (julianDate == julianEaster - 3) return PINK;
#else
  /* Now check for day before or after Easter or Black Friday */
  if ((month == easter_month) && (day == easter_day +1)) return(RED);
  if ((month == easter_month) && (day == easter_day -1)) return(PINK);
  if ((month == easter_month) && (day == easter_day -2)) return(RED);
  if ((month == easter_month) && (day == easter_day -3)) return(PINK);
#endif

  /* Now check for days related to easter */
#ifdef FIX_920629
  if (julianDate == julianEaster + 39) return RED;
  if (julianDate == julianEaster + 38) return PINK;
#else
  tmp_day = easter_day + 39;
  if (easter_month == 3)
  {
    tmp_month = 4;
    tmp_day   = tmp_day - 31;
  }

  if (tmp_day > 30)
  {
    tmp_month = 5;
    tmp_day   = tmp_day - 30;
  }

  if ((month == tmp_month) && (day == tmp_day - 1)) return(PINK);
  if ((month == tmp_month) && (day == tmp_day))     return(RED);
#endif

  /* More days related to easter */
#ifdef FIX_920629
  if (julianDate == julianEaster + 48) return PINK;
  if (julianDate == julianEaster + 50) return RED;
#else
  tmp_day = easter_day + 49;
  if (easter_month == 3)
  {
    tmp_month = 4;
    tmp_day   = tmp_day - 31;
  }

  if (tmp_day > 30)
  {
    tmp_month = 5;
    tmp_day   = tmp_day - 30;
  }

  if (tmp_day > 31)
  {
    tmp_month = 6;
    tmp_day   = tmp_day - 31;
  }

  if ((month == tmp_month) && (day == tmp_day - 1)) return(PINK);
  if ((month == tmp_month) && (day == tmp_day + 1)) return(RED);
#endif
  return(BLACK);
}

static int check_date(year, month, day)
int year, month, day;
{
  int leap;
  if ((year % 4) == 0) leap = 1; else leap = 0;
  switch(month)
  {
    case  1: if (day > 31) return(ERRDAY); else return(DATE_OK); break;
    case  2: if (((day >28) && (leap == 0)) || ((day >29) && (leap == 1)))
	     return(ERRDAY); else return(DATE_OK); break;
    case  3: if (day > 31) return(ERRDAY); else return(DATE_OK); break;
    case  4: if (day > 30) return(ERRDAY); else return(DATE_OK); break;
    case  5: if (day > 31) return(ERRDAY); else return(DATE_OK); break;
    case  6: if (day > 30) return(ERRDAY); else return(DATE_OK); break;
    case  7: if (day > 31) return(ERRDAY); else return(DATE_OK); break;
    case  8: if (day > 31) return(ERRDAY); else return(DATE_OK); break;
    case  9: if (day > 30) return(ERRDAY); else return(DATE_OK); break;
    case 10: if (day > 31) return(ERRDAY); else return(DATE_OK); break;
    case 11: if (day > 30) return(ERRDAY); else return(DATE_OK); break;
    case 12: if (day > 31) return(ERRDAY); else return(DATE_OK); break;
    default: return(ERRDAY);
  }
}

/* This routine is called with a year (4 digits), a month and a day. The
program returns an int, BLACK, PINK, or RED.
RED are Sundays and public holidays.
PINK is day before a RED day
BLACK is all other days.
On error the program returns ERRDAY */

int colour_of_day(sent_year, sent_month, sent_day)
int sent_year, sent_month, sent_day;
{
  int day_colour, day_code, year, month, day;
  year = sent_year;
  month = sent_month;
  day = sent_day;

  /* First, check that date is after 1980-01-01 to ensure compability
  with later DOS versions, and before 2099-12-31 to fit this program */
  if ((year < 1980)||(year > 2099)) return(ERRDAY);

  /* Now we will check that we got a reasonable date */
  day_code = check_date(year, month, day);
  if (day_code == ERRDAY) return(ERRDAY);

  /* Now check if the date is a regular sunday. If so, we need not
  investigate further. However, if it is a Saturday, we will remember
  this and possibly use later.*/
  day_code = is_sunday(year, month, day);
  if (day_code == SUNDAY) return(RED);

  /* Now check some fixed dates */
  if ((month == 1)  && (day == 1))  return(RED);
  if ((month == 1)  && (day == 5))  return(PINK);
  if ((month == 1)  && (day == 6))  return(RED);
  if ((month == 4)  && (day == 30)) return(PINK);
  if ((month == 5)  && (day == 1))  return(RED);
  if ((month == 12) && (day == 24)) return(PINK);
  if ((month == 12) && (day == 25)) return(RED);
  if ((month == 12) && (day == 26)) return(RED);
  if ((month == 12) && (day == 31)) return(PINK);

  /* Now check for some holidays that use Saturday as a RED day */
#ifdef FIX_920629
  if ((month == 6) && (day >= 19) && (day <= 25) && (day_code == FRIDAY))
       return(PINK);
  if ((month == 6) && (day >= 20) && (day <= 26) && (day_code == SATURDAY))
       return(RED);
#else
  if ((month == 6) && (day > 20) && (day <= 26) && (day_code == FRIDAY))
       return(PINK);
  if ((month == 6) && (day > 20) && (day <= 26) && (day_code == SATURDAY))
       return(RED);
#endif

   /* Is it All Saints? */
  if ((month == 10) && (day >= 30) && (day_code == FRIDAY))   return(PINK);
  if ((month == 10) && (day >= 31) && (day_code == SATURDAY)) return(RED);
  if ((month == 11) && (day <=  5) && (day_code == FRIDAY))   return(PINK);
  if ((month == 11) && (day <=  6) && (day_code == SATURDAY)) return(RED);


  /* If month is 3 to 6 it might be something connected to
     easter, better check */
  if ((month >= 3) && (month <= 6))
  {
    day_colour = easter(year, month, day);
    if ((day_colour == RED) || (day_colour == PINK)) return(day_colour);
  }
  if (day_code == SATURDAY) return(PINK); else return(BLACK);
}

#ifdef DEBUG_STATE
int whatday(y, m, d)
int y, m, d;
{
  static struct tm tp;
  time_t x;
  
  tp.tm_wday = 8;  
  tp.tm_year = y - 1900;
  tp.tm_mon  = m - 1;
  tp.tm_mday = d;
  x = mktime(&tp);
/*  printf("%s", ctime(&x));  */
  return (tp.tm_wday == 0) ? 7 : tp.tm_wday;  /* 1 == mon, 7 == sun */
}
/*int colour_of_day(sent_year, sent_month, sent_day)  */
main()
{
  int y, m, d, s, wd;

  for (y = 1990; y < 2010; y++)
    for (m = 1; m <= 12 ; m++)
      for (d = 1; d <= 31; d++) {
        s = colour_of_day(y, m, d);
        if (s == ERRDAY) 
          continue;
        wd = whatday(y, m, d);

/*        printf("%d-%02d-%02d : wd=%d\n", y, m, d, wd);  */

        if (wd == 1) {
          printf("\n");
          printf("%d-%02d-%02d : ", y, m, d);
        }
        printf(" %c ", 
              (s == RED) ? 'R' : ((s == PINK) ? 'P' : '-'));
          
/*
        else
        printf("Wd=%d:  %d-%02d-%02d is %s\n", wd, y, m, d, 
              (s == RED) ? "red" : ((s == PINK) ? "pink" : "black"));
*/        
      }
}
#endif
