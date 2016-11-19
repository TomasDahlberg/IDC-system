/* screen.c  1991-12-19 TD,  version 1.2 */
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
! screen.c
! Copyright (C) 1991, IVT Electronic AB.
*/


#include <stdio.h>
#include <strings.h>
/* #include <time.h>    */
#include "alarm.h"

#define UPDATE_SCREEN

int LOCAL_PRINTER = 0;

#ifdef OSK
#include "phyio/phyio.h"
#else
#include "phyio.h"
#endif
/*
#include "idcio/idcio.h"
*/
#define TIMEOUT_VALUE 300

backLight(on)
int on;
{
  lys(6, 1 - on);
}

/*
int tsleeptight()
{
  int ticks;
  
  ticks = sleep(3);
  while (ticks) {
    ticks = tsleep(ticks);
  }
}
*/

beep()
{
  int i;
  
  lys(7, 1);
  tsleep(0x80000040);
  lys(7, 0);
}

left(id)
int id;
{
  if (screens[id-1].left)
    return screens[id-1].left;
  else if (screens[id-1].up)
    return left(screens[id-1].up);
  else
    return 0;     /* no left ! */
}

doScreen(dm, meta, aldm, currentId, local_printer)
char *dm;
char *meta;
struct _alarmModule *aldm;
int currentId;
int local_printer;
{
    int keyCode, prev, prevId;
    time_t timer, timer2;
    
    prevId = currentId + 1;
    while (1) {
        if (currentId != prevId) {
            lcdinit();
            lcdcld();
            prevId = currentId;
            lcdshowdirections(1, screens[currentId-1].up);
/*            lcdshowdirections(2, screens[currentId-1].left);  */
            lcdshowdirections(2, left(currentId));    
            lcdshowdirections(3, screens[currentId-1].right);
            lcdshowdirections(4, screens[currentId-1].down);
            timer = time(0);
        }
        lcdhome();

        if ((time(0) - timer) > TIMEOUT_VALUE) {
          keyDown();
          return 1;
        }

        timer2 = time(0);
	(screens[currentId - 1].fcn)();
	tsleep(10);                         /* save CPU-time */
        if ((time(0) - timer2) > TIMEOUT_VALUE)
          return 1;
 	
#ifdef UPDATE_SCREEN
        lcdsetCacheCursor(1);     /* don't want cursor sweeping over display */
/*        updateLed();      */
        if (keyDown()) {
          if (key() != KEY_CHANGE)      /* added 920107 */
    	    keyCode = getKey();
  	} else 
  	  keyCode = NO_KEY;
#else
        keyCode = getKey();
        if (keyCode == NO_KEY)
          return 1;
#endif
        if (keyCode != NO_KEY) {
          backLight(1);   /* turn on back-light */
        }

/*
	if (keyCode == KEY_LEFT && screens[currentId-1].left)
	    currentId = screens[currentId-1].left;
*/
	if (keyCode == KEY_LEFT) {
	    int x;
	    if (x = left(currentId))
      	      currentId = x;
	}
	else if (keyCode == KEY_RIGHT && screens[currentId-1].right)
	    currentId = screens[currentId-1].right;
	else if (keyCode == KEY_UP    && screens[currentId-1].up)
	    currentId = screens[currentId-1].up;
	else if (keyCode == KEY_DOWN  && screens[currentId-1].down)
	    currentId = screens[currentId-1].down;
	else if (keyCode == KEY_HELP  && screens[currentId-1].help)
	{
	    if (screens[currentId-1].help == -1)
	      return 0;                             /* ok, no timeout ! */
            if (doScreen(dm, meta, aldm, 
                    screens[currentId-1].help, local_printer))    /* timeout?*/
              return 1;                       /* yes, return next level ! */
            prevId = -1;    /* triggers update of current screen */
        }
#define NEW_ENTER
#ifdef NEW_ENTER
	else if (keyCode == KEY_CHANGE) {
	  if (getLevel() >= 5) {
            lcdshowdirections(1, 0);
            lcdshowdirections(2, 0);
            lcdshowdirections(3, 0);
            lcdshowdirections(4, 0);
            lcdsetCacheCursor(1);
	    if (showVariables(dm, meta))      /* if timeout return */
	      return 1;
            lcdsetCacheCursor(0);
	    lcdcld();
            prevId = -1;    /* triggers update of current screen */
	    timer = time(0);
	  } else
	    beep();
	}
#endif
	else if (keyCode == KEY_ALARM) {
          lcdshowdirections(1, 0);
          lcdshowdirections(2, 0);
          lcdshowdirections(3, 0);
          lcdshowdirections(4, 0);
          if (readAlarm(aldm, local_printer))   /* if timeout return */
            return 1;
          prevId = -1;    /* triggers update of current screen */
/*	  lcdcld(); */
	}
    }
}

main(argc, argv)
int argc;
char *argv[];
{
  char *headerPtr1, *headerPtr2, *headerPtr3;
  struct _alarmModule *aldm;
  char *meta;
  struct _system *sysVars;
  int selCode = 0;
  
  if (argc == 2 && argv[1][0] == '-') {
    selCode = (argv[1][1] - '0') % 4;
  }
  sysVars = (struct _system *) SYSTEM_AREA;  
  initphyio();
  initidcio();

  if (selCode)
    lcdSetSelect(selCode);

  dm = (struct _datamodule *) linkDataModule(NAMEOFDATAMODULE, &headerPtr1);
  if (!dm) {
    printf("cannot link to datamodule '%s'\n", NAMEOFDATAMODULE);
    printf("check if process 'scan' is running\n");
    return 0;
  }
  meta = (char *) linkDataModule("METAVAR", &headerPtr2);
  if (!meta) {
    printf("cannot link to datamodule '%s'\n", "METAVAR");
    printf("check if process 'scan' is running\n");
    return 0;
  }
#if NO_OF_ALARMS > 0
  aldm = (struct _alarmModule *) linkDataModule("ALARM", &headerPtr3);
  if (!aldm) {
    printf("cannot link to datamodule '%s'\n", "ALARM");
    printf("check if process 'scan' is running\n");
    return 0;
  }
#else
  aldm = 0;
#endif
     
  init(meta, dm);
  lcdinit();
  lys(0, 0);      /* shut of alarm light */
  lys(1, 1);      /* assert cpu led */
  lys(2, 0);
  lys(3, 0);
  lys(4, 0);
  lys(5, 0);
  beep();
  backLight(1);   /* turn on back-light */
  keyDown();      /* strobe watch dog */
  sysVars->tid.dummy = 0;   /* time offset not locked */
  while (1) {
    if (_ROOT_SCREEN_POINTER == 0)
    {
      sleep(0);
      continue;
    }
/*
!   show main display, routine doScreen() returns after timeout
*/
    if (doScreen(dm, meta, aldm, _ROOT_SCREEN_POINTER, LOCAL_PRINTER)) {
      int id;
      keyDown();
      if ((id = metaId(meta, "level")) > 0)
        *((int *) metaValue(dm, meta, id)) = 1;
      keyDown();
      backLight(0);   /* turn off back-light */
      keyDown();
    }
  }
}
