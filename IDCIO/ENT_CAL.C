/* ent_cal.c  1992-01-15 TD,  version 1.0 */
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
! ent_cal.c
! Copyright (C) 1992, IVT Electronic AB.
*/


#include <stdio.h>
#include <time.h>

#include "idcio.h"
#include "../phyio/phyio.h"
#include "priv.h"

#define TIMEOUT_VALUE 300


#define WEEKDAY_MASK 2048
#define ACTIVE_MASK  4096
#define DAYSPEC_MASK 2047
#define NO_OF_CAL_ENTRIES 10
struct _calendar
{
  unsigned short day[NO_OF_CAL_ENTRIES];
  unsigned short stopday[NO_OF_CAL_ENTRIES];
  unsigned char  color[NO_OF_CAL_ENTRIES];
  unsigned short start[NO_OF_CAL_ENTRIES];
  unsigned short stop[NO_OF_CAL_ENTRIES];
};


static char *month[] = {"jan", "feb", "mar", "apr", "maj", "jun",
                        "jul", "aug", "sep", "okt", "nov", "dec" };

static char *colorType[] = {"Alla  ", "Vardag", "F.Helg", "", "Helg  ", 
                              "", "", ""};

showRow(cal, row)
struct _calendar *cal;
int row;
{
  int i;

  i = row - 1;
  lcdprintf("%2d %s ", row, colorType[cal->color[i] & 0x07]);
  if (cal->day[i] & WEEKDAY_MASK)
    lcdprintf("MTOTFLS       ");
  else {
    int d1, m1, d2, m2;
    d1 = cal->day[i] % 100;
    m1 = cal->day[i] / 100;
    if (cal->stopday[i]) {
      d2 = cal->stopday[i] % 100;
      m2 = cal->stopday[i] / 100;
      lcdprintf("%2d %s-%2d %s ", d1, month[m1-1], d2, month[m2-1]);
    } else
      lcdprintf("%2d %s        ", d1, month[m1-1]);
  }
/*  if (cal->start[i] == 0 && cal->stop[i] == 2400)
    lcdprintf("hela dygnet");
  else  */
    lcdprintf("%02d:%02d-%02d:%02d", 
          cal->start[i] / 100,
          cal->start[i] % 100,
          cal->stop[i] / 100,
          cal->stop[i] % 100);

  lcdprintf("\n          ");  
  if (cal->day[i] & WEEKDAY_MASK)
    showBits((int) cal->day[i]);
}

showBits(w)
int w;
{
  int c1, c2;
  c1 = '\2'; c2 = ' ';
  lcdprintf("%c%c%c%c%c%c%c", (w & 1) ? c1 : c2,
                                (w & 2) ? c1 : c2,
                                (w & 4) ? c1 : c2,
                                (w & 8) ? c1 : c2,
                                (w & 16) ? c1 : c2,
                                (w & 32) ? c1 : c2,
                                (w & 64) ? c1 : c2);
}

modRow(cal, row)
struct _calendar *cal;
int row;
{
  int k, i;
  
  enum { 
      BEGIN, COLOR_OF_DAY, WEEK_DATE, START_HOUR, START_MIN,
      STOP_HOUR, STOP_MIN, END
  } changeState;

  i = row - 1;
  changeState = BEGIN;
  while (1) {
    k = NO_KEY;
    switch (changeState) {
      case BEGIN:
        changeState ++;
        continue;
      case COLOR_OF_DAY:
        k = changeColor(cal, row);
        break;
      case WEEK_DATE:
        if (cal->day[i] & WEEKDAY_MASK)
          k = changeWeekDay(cal, row);
        else
          k = changeDate(cal, row);
        break;
      case START_HOUR:
        k = changeTime(cal, row, 0);
        break;
      case START_MIN:
        k = changeTime(cal, row, 1);
        break;
      case STOP_HOUR:
        k = changeTime(cal, row, 2);
        break;
      case STOP_MIN:
        k = changeTime(cal, row, 3);
        break;
      case END:
        lcdpos(1, 37);
        lcdprintf("?");
        lcdpos(1, 37);
        if ((k = getKey()) == KEY_ENTER)
          return 1;
        else if (k == KEY_CHANGE) {
          return 0;
        }
        lcdpos(1, 37);
        lcdprintf(" ");
        lcdpos(1, 37);
        changeState --;
        continue;
    }
    if (k == KEY_LEFT)
      changeState --;
    else if (k == KEY_RIGHT)
      changeState ++;
    else if (k == KEY_ENTER)
      changeState ++;
    else if (k == KEY_CHANGE) {
      return 0;
    }
  }
}
       
int changeColor(cal, row)
struct _calendar *cal;
int row;
{
  int i, k, choice;

  i = row - 1;
  choice = cal->color[i] & 0x07;
  while (1) {
    lcdpos(1, 3);
    lcdprintf("%s ", colorType[choice]);
    lcdpos(1, 3);
    k = getKey();
    if (k == KEY_PLUS_MINUS) {    /* 0, 1, 2, 4 */
      choice++;
      if (choice == 3)
        choice++;
      else if (choice >= 4)
        choice = 0;
    } else if (k == KEY_ENTER)
      break;
    else if (k == KEY_RIGHT)
      break;
    else if (k == KEY_LEFT)
      break;
    else if (k == KEY_CHANGE)
      break;
  }
  cal->color[i] = choice;   
  return k;
}


int changeWeekDay(cal, row)
struct _calendar *cal;
int row;
{
  int i, k;
  int weekDay = 1, xPos = 10;

  i = row - 1;
  while (1) {
    lcdpos(1, 10);
    showBits((int) cal->day[i]);
    lcdpos(1, xPos);
    k = getKey();
    if (k == KEY_PLUS_MINUS) {    /* 0, 1, 2, 4 */
      cal->day[i] ^= weekDay;
      if (weekDay < 64) {
        weekDay <<= 1;
        lcdpos(1, ++xPos);
      } else {
        k = KEY_RIGHT;
        break;
      }
    } else if (k == KEY_ENTER) {
      break;
    } else if (k == KEY_RIGHT) {
      if (weekDay < 64) {
        weekDay <<= 1;
        lcdpos(1, ++xPos);
      } else 
        break;
    } else if (k == KEY_LEFT) {
      if (weekDay > 1) {
        weekDay >>= 1;
        lcdpos(1, --xPos);
      } else
        break;
    } else if (k == KEY_CHANGE)
      break;
  }
  lcdpos(1, 10);
  showBits((int) cal->day[i]);
  return k;
}

int changeDate(cal, row)
struct _calendar *cal;
int row;
{
  int i, k, state = 0, tal;
  int d1, m1, d2, m2, m;

  i = row - 1;
  while (1) {
    d1 = cal->day[i] % 100;
    m1 = cal->day[i] / 100;
    if (cal->stopday[i]) {
      d2 = cal->stopday[i] % 100;
      m2 = cal->stopday[i] / 100;
    } else {
      d2 = -1;
      m2 = -1;
    }
    
    switch (state) {
      case 0:
        k = get2(10, &tal, d1);
        cal->day[i] = m1 * 100 + tal;
        break;
      case 1:
        m1 --;
        do {
          lcdpos(1, 13);
          lcdprintf("%s", month[m1]);
          lcdpos(1, 13);
          m = m1;
          m1 ++;
          m1 %= 12;
        } while ((k = getKey()) == KEY_PLUS_MINUS) ;
        m1 = m + 1;
        cal->day[i] = m1 * 100 + d1;
        break;
      case 2:
        k = get2(17, &tal, d2);
        cal->stopday[i] = m2 * 100 + tal;
        break;
      case 3:
        m2 --;
        do {
          lcdpos(1, 20);
          lcdprintf("%s", month[m2]);
          lcdpos(1, 20);
          m = m2;
          m2 ++;
          m2 %= 12;
        } while ((k = getKey()) == KEY_PLUS_MINUS) ;
        m2 = m + 1;
        cal->stopday[i] = m2 * 100 + d2;
        break;
    }   
    if (k == KEY_LEFT) {
      if (state)
        state --;
    } else if (k == KEY_CHANGE) {
      break;
    } else if (k == KEY_ENTER) {
      break;
    } else {
      if (state < 3)
        state ++;
      else {
        k = KEY_RIGHT;
        break;          /* new ... */
      }
    }
  }
  return k;
}

int get2(x, tal, def)
int x, *tal, def;
{
  int k, xPos = 0, first = 1;
  
  while (1) {
    lcdpos(1, x + xPos);
    k = getKey();
    if (k == KEY_ENTER) {
      break;
    } else if (k == KEY_LEFT) {
      if (xPos)
        xPos--;
      else
        break;
    } else if (k == KEY_RIGHT) {
      if (xPos)
        break;
      else
        xPos++;
    } else if (k == KEY_CHANGE) {
      first = 1;
      break;
    } else if (k < 10) {
      if (first) {
        *tal = first = 0;
      }
      (*tal) = (*tal)*10 + k;
      lcdprintf("%c", k + '0');
      if (xPos) {
        k = KEY_RIGHT;
        break;
      }
      xPos ++;
    }
  }
  if (first)
    *tal = def;
  return k;
}

int changeTime(cal, row, c)
struct _calendar *cal;
int row, c;
{
  int i, k, tal, def;

  i = row - 1;
  if (c == 0)
    def = cal->start[i] / 100;
  else if (c == 1)
    def = cal->start[i] % 100;
  else if (c == 2)
    def = cal->stop[i] / 100;
  else if (c == 3)
    def = cal->stop[i] % 100;
    
  k = get2(24 + c*3, &tal, def);
  
  if (c == 0)
    cal->start[i] = cal->start[i] % 100 + 100*tal;
  else if (c == 1)
    cal->start[i] = 100 * ((int) (cal->start[i] / 100)) + tal;
  else if (c == 2)
    cal->stop[i] = cal->stop[i] % 100 + 100*tal;
  else if (c == 3)
    cal->stop[i] = 100 * ((int) (cal->stop[i] / 100)) + tal;
  return k;
}

deleteEntry(cal, i)
struct _calendar *cal;
int i;
{
  int row;
  
  for (row = i + 1; row < NO_OF_CAL_ENTRIES; row++) {
    cal->day[row - 1] = cal->day[row];
    cal->stopday[row - 1] = cal->stopday[row];
    cal->color[row - 1] = cal->color[row];
    cal->start[row - 1] = cal->start[row];
    cal->stop[row - 1] = cal->stop[row];
  }
}
        
insertEntry(cal, i)
struct _calendar *cal;
int i;
{
  int row;
            
  for (row = NO_OF_CAL_ENTRIES - 1; row > i; row--) {
    cal->day[row] = cal->day[row - 1];
    cal->stopday[row] = cal->stopday[row - 1];
    cal->color[row] = cal->color[row - 1];
    cal->start[row] = cal->start[row - 1];
    cal->stop[row] = cal->stop[row - 1];
  }
}

int enter_calendar(cal)
struct _calendar *cal;
{
  int i, k, row;
  unsigned short CALday, CALstopday, CALstart, CALstop;
  unsigned char  CALcolor;
  struct _calendar calBackup;

  if (key() == KEY_UP) {
    for (i = 0; i < NO_OF_CAL_ENTRIES && cal->day[i]; i++)
      ;
    i--;
  } else
    i = 0; 

  for ( ; i < NO_OF_CAL_ENTRIES; /* i++ */) {
    lcdprintf("\f");
    if (cal->day[i])
      showRow(cal, i+1);
    else
      lcdprintf(" - slut p\06 kalendern -");


    if (i == 0) {
      time_t timer;
      timer = time(0);
      while (!keyDown())
        if ((time(0) - timer) > TIMEOUT_VALUE)
          return 1;
      if ((key() != KEY_DOWN) && (key() != KEY_CHANGE))    /* == KEY_UP ! */
        return 0;
    } else if ((cal->day[i] == 0) || (i == NO_OF_CAL_ENTRIES)) {
      time_t timer;
      timer = time(0);
      while (!keyDown())
        if ((time(0) - timer) > TIMEOUT_VALUE)
          return 1;
      if ((key() != KEY_UP) && (key() != KEY_CHANGE))          /* == KEY_DOWN ! */
        return 0;
    } else {
      time_t timer;
      timer = time(0);
      while (!keyDown())
        if ((time(0) - timer) > TIMEOUT_VALUE)
          return 1;
      if (key() == KEY_LEFT)
        return 0;
    }
    k = getKey();

    if (k == NO_KEY)
      return 1;
    else if (k == KEY_CHANGE) {
      if (!checkAccessLevel(PRIV_M_ENTER_CALENDAR /* MIN_LEVEL_ENTER_CALENDAR */)) {
        continue;
      }
      lcdpos(1, 0);
      lcdprintf("                                      ");
      lcdpos(1, 0);
      lcdcursorOn();
      if (cal->day[i])
/*        lcdprintf("Mod, Ta bort, Ny dag/dat = Mod ? (+/-)");    */
        lcdprintf("1=Mod,2=Ta bort,3=Ny dag,4=Ny datum ?");
      else
        lcdprintf("                3=Ny dag,4=Ny datum ?");
      k = getKey();
      lcdcursorOff();
      if (k == 1 && cal->day[i]) {
        CALday = cal->day[i];        CALstopday = cal->stopday[i];
        CALcolor = cal->color[i];
        CALstart = cal->start[i]; CALstop = cal->stop[i];
        lcdprintf("\f");
        if (cal->day[i])
          showRow(cal, i+1);
        lcdcursorOn();
        memcpy(&calBackup, cal, sizeof(struct _calendar)); 
        if (!modRow(&calBackup, i+1)) {
          cal->day[i] = CALday;             /* these really not needed */
          cal->stopday[i] = CALstopday;
          cal->color[i] = CALcolor;
          cal->start[i] = CALstart;
          cal->stop[i] = CALstop;
        } else {
          cal->day[i] = calBackup.day[i];
          cal->stopday[i] = calBackup.stopday[i];
          cal->color[i] = calBackup.color[i];
          cal->start[i] = calBackup.start[i];
          cal->stop[i] = calBackup.stop[i];
        }
        lcdcursorOff();
      } else if (k == 2 && cal->day[i]) {
        lcdpos(1, 0);
        lcdprintf("                                      ");
        lcdpos(1, 0);
        lcdcursorOn();
        lcdprintf("Ta bort denna rad (ja = enter) ? ");
        k = getKey();
        lcdcursorOff();
        if (k == KEY_ENTER)
          deleteEntry(cal, i);
      } else if (k == 3) {
        for (row = i; row < NO_OF_CAL_ENTRIES; row++)
          if (cal->day[row] == 0)
            break;
        if (row >= NO_OF_CAL_ENTRIES) {
          lcdprintf("No free entries beyond this row");
          return 0;
        }
        insertEntry(cal, i);
/*
        cal->day[i] = 1 + WEEKDAY_MASK; cal->stopday[i] = 0;
        cal->color[i] = 0; cal->start[i] = 0; cal->stop[i] = 2400;
*/
        lcdprintf("\f");
        lcdcursorOn();

        calBackup.day[i] = 1 + WEEKDAY_MASK;
        calBackup.stopday[i] = 0;
        calBackup.color[i] = 0;
        calBackup.start[i] = 0;
        calBackup.stop[i] = 2400;

        showRow(&calBackup, i+1);
        if (!modRow(&calBackup, i+1)) {
          deleteEntry(cal, i);
        } else {
          cal->day[i] = calBackup.day[i];
          cal->stopday[i] = calBackup.stopday[i];
          cal->color[i] = calBackup.color[i];
          cal->start[i] = calBackup.start[i];
          cal->stop[i] = calBackup.stop[i];
        }
        lcdcursorOff();
      } else if (k == 4) {
        for (row = i; row < NO_OF_CAL_ENTRIES; row++)
          if (cal->day[row] == 0)
            break;
        if (row >= NO_OF_CAL_ENTRIES) {
          lcdprintf("No free entries beyond this row");
          return 0;
        }
        insertEntry(cal, i);
/*
        cal->day[i] = 101; cal->stopday[i] = 101;
        cal->color[i] = 0; cal->start[i] = 0; cal->stop[i] = 2400;
*/
        lcdprintf("\f");
        lcdcursorOn();
        calBackup.day[i] = 101;
        calBackup.stopday[i] = 101;
        calBackup.color[i] = 0;
        calBackup.start[i] = 0;
        calBackup.stop[i] = 2400;

        showRow(&calBackup, i+1);
        if (!modRow(&calBackup, i+1)) {
          deleteEntry(cal, i);
        } else {
          cal->day[i] = calBackup.day[i];
          cal->stopday[i] = calBackup.stopday[i];
          cal->color[i] = calBackup.color[i];
          cal->start[i] = calBackup.start[i];
          cal->stop[i] = calBackup.stop[i];
        }
        lcdcursorOff();
      }
    }
    else if (k == KEY_UP) {
      if (i > 0)
        i --;
    } else if (k == KEY_DOWN) {
      if (cal->day[i])
        i++;
    }
  }
}


struct _calendar test;

#ifdef DEBUG
main()
{
  int s, k;
  
  initphyio();
  initidcio();

  test.day[0] = 2048 + 63;
  test.start[0] = 1005;
  test.stop[0] = 1630;

  test.day[1] = 2048 + 64 + 32 ;
  test.start[1] = 1005;
  test.stop[1] = 1630;

  test.day[2] =  1222;
  test.stopday[2] =  0;
  test.start[2] = 1005;
  test.stop[2] = 1630;

  test.day[3] =  1222;
  test.stopday[3] =  1224;
  test.start[3] = 1005;
  test.stop[3] = 1630;

      lcdcursorOff();


  s = 1;
  while(1) {
    if (s == 1)
      display("\fScreen 1:\n");
    else if (s == 2)
      enter_calendar(&test);
    else if (s == 3)
      display("\fScreen 2:\n");
    
    if (keyDown()) {
      k = getKey();
      
      if (k == KEY_UP) {
        if (s > 1)
          s --;
      } else if (k == KEY_DOWN) {
        if (s < 3)
          s++;
      }
    }
  }
}

#endif
