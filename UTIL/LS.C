/* ls.c  1993-04-19 TD,  version 1.42 */
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
! ls.c
! Copyright (C) 1992,1993 IVT Electronic AB.
*/


/*
!   ls          - variables List on Screen
! 
!   History:
!   Date        by   rev   ed#  what
!   ----------  ---  ----  ---  ---------------------------------------------
!   1991-02-19  TD   1.00  1    initial coding
!   1991-05-07  TD   1.01  2    code shaped to fit new alarm routines
!   1991-09-09  TD   1.02  3    patterns added
!   1991-12-10  TD   1.20  4    option -w added
!   1992-01-15  TD   1.30  5    speed up, removed func (- - -), initially
!                               seeks all patterns and maps it, print-cache of
!                               last row (alarm status row)
!   1992-09-28  TD   1.40  6    Fix of no of none sent alarms, added V/Z
!   1992-10-13  TD   1.41  7    Added ctrl-X to escape, (used by remote site)
!   1993-04-19  TD   1.42  8    Added check if initidcio() ok
!
!   Function: 
!   Presentates a continously update of the global variables declared in VARS
!
*/
@_sysedit: equ 8
@_sysattr: equ $8005

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <module.h>
#include <sgstat.h>
#define NO_OF_ALARMS 1
#define _V920914_1_40
#include "alarm.h"

struct _alarmModule *aldm;
char *dm, *dmBackup;
char *meta;
char *headerPtr1, *headerPtr2, *headerPtr3, *headerPtr4;
int first;

#define TYPE_INT    7
#define TYPE_FLOAT  8

#define NAMEOFDATAMODULE  "VARS"
#define NAMEOFDATAMODULE2 "VAR2"

char *nextVar();

char pattern[10][32];     /* 20 -> 32 */
int pattCnt = 0;
int width = 20;

#define DEBUG_2
#ifdef DEBUG_2
int DEBUG = 0;
#endif

int Abort()
{
  unlinkDataModule(headerPtr1);
  unlinkDataModule(headerPtr2);
  unlinkDataModule(headerPtr3);
  unlinkDataModule(headerPtr4);
  enableXonXoff(0);
  exit(0);
}

void icp(s)
int s;
{
  if (s == 257)
    return ;
    
  gotoxy(0, 21);
  Abort();
}

print(s)
char *s;
{
  write(1, s, strlen(s));
}

gotoxy(x, y)
int x, y;
{
  char buff[10];
  sprintf(buff, "%c%c%02d;%02d%c", 27, 91, y + 1, x + 1, 72);
  write(1, buff, 8);
}

visualAttribute(c)
int c;
{
  char buff[10];
  sprintf(buff, "%c[%dm", 27, c);
  write(1, buff, 4);
}

normal() {  visualAttribute(0); }
bold() {  visualAttribute(1); }
blinking() {  visualAttribute(5); }
inverse() {  visualAttribute(7); }

clearScreen()
{
  static char buf[4] = { 27, 91, '2', 'J' };
  write(1, buf, 4);
}


/*
!   gives a hint of usage of this program
*/
void usage()
{
  printf("Syntax: ls [<opts>] {<pattern> [<opts>]}\n");
  printf("Function: show variables on screen continuously\n");
  printf("Options:\n");
  printf("     -i=<time>    intervall betweeen scans, in 256th seconds\n");
  printf("     -s=<start>   start id of scan, a value from 1 and upwards\n");
  printf("     -w=<width>   width of variable name and value, default 20\n");
  printf("     -?           shows this list\n");
  printf("\n");
  printf("At run time, the cursor can move among the variables by the use\n");
  printf("of ctrl-F forward, ctrl-B backward, ctrl-P previous line, and\n");
  printf("ctrl-N next line. The value of the current variable can be modified\n");
  printf("by pressing the Enter key. Toggle between lock and unlock is\n");
  printf("accomplished by the use of ctrl-L.\n");
  printf("Change of search pattern, use the key '.' and to add an additional\n");
  printf("search pattern, use the key '+'\n");
  printf("The current width can be changed by pressing ctrl-W.\n");
}

char *ctid(tid)
time_t tid;
{
  static char buf[25];
  struct tm tbuf;
  
  memcpy(&tbuf, localtime(&tid), sizeof(struct tm));
  sprintf(buf, "%02d%02d%02d %02d:%02d:%02d", 
                      tbuf.tm_year, tbuf.tm_mon + 1, tbuf.tm_mday, 
                      tbuf.tm_hour, tbuf.tm_min, tbuf.tm_sec);
  return buf;
}

int currentId = -1;
char prevBuff[100];

static int alarmStatus(entry)
struct _alarmEntry *entry;
{
  int as, ns, cs, ds, es, status;
  
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

  status = as | ns | cs | ds | es;
  
  return status;
}

main(argc, argv)
int argc;
char *argv[];
{
  char *str, *_new, *_dest, buff[80];
  int id, x, y, size, startId, xCur, yCur;
  int clearPrev = 0, c_y, c_x, update = 0, u_y, u_x;

  unsigned long sleepTime;
  int uc, cf, ns, puc = 0, pcf = 0, pns = 0, cnt, i;
  time_t cNow, cPrev = 0;

  intercept(icp);
  
  if (i = initidcio()) {
    if (i == 207)
      printf("Error 207, Memory Full\n");
    else if (i == 237)
      printf("Error 237, No RAM Available\n");
    else
      printf("Cannot link to trap module 'idcio', error %d\n", i);
  }
  bind(&dm, &aldm, &meta, &headerPtr1, &headerPtr2, &headerPtr3);
  if (!(dm && aldm && meta)) {
    exit(0);
  }
  disableXonXoff(0);
  
/*
!   create a new/or link to existing  dmBackup module. (for method II requests)
*/

  size = ((struct modhcom *) headerPtr1)->_msize;
  _dest = 0;
  dmBackup = (char *) createDataModule("VAR2_LS", size, 
                                             &_new, &headerPtr4, &_dest);
  if (!dmBackup)
  {
    printf("cannot create module 'VAR2_LS'\n");
    printf("size = %d\n", size);
    printf("vars headerptr = %d\n", headerPtr4);
    Abort();
  }  
  
  sleepTime = 0;
  startId = 0;
  while( argc >= 2 ) {
    if (argv[1][0] == '-' )
    {
        char c;
	while( *++(argv[1]) ) {
	    switch( *argv[1] ) {
	        case 'w':
	        case 'W':
	          width = 0;
		  if (*++argv[1] == '=') {
		    while (isdigit(c = *++argv[1]))
		      width = width * 10 + c - '0';
 		    --argv[1];
		  }
                  else
                    fprintf(stderr, "usage: -w=<width>\n");
                  break;
		case 's':
		case 'S':
		  if (*++argv[1] == '=') {
		    while (isdigit(c = *++argv[1]))
		      startId = startId*10 + c - '0';
		    --argv[1];
		  }
/*                   startId = atoi(&argv[1][2]);       */
                  else
                    fprintf(stderr, "usage: -s=<id>\n");
                  break;
		case 'i':
		case 'I':
		  if (*++argv[1] == '=') {
		    while (isdigit(c = *++argv[1]))
		      sleepTime = sleepTime*10 + c - '0';
		    --argv[1];
		  }
                  else
                    fprintf(stderr, "usage: -i=<256th-seconds>\n");
                  break;
#ifdef DEBUG_2
                case 'd':
                  DEBUG = 1;
                  break;
#endif                  
                case '?':
                    usage();
                    Abort();
		default:
		    fprintf(stderr, "illegal option: %c", (char *) *argv[1]);
                    Abort();
		}
	    }
	argv++;
	argc--;
    } else {
      strcpy(pattern[pattCnt++], argv[1]);
      argv++;
      argc--;
    }
  }
  if (sleepTime)
    sleepTime |= 1 << 31;   
  first = 1;

  clearScreen();
  initMatchTable(startId); 

  xCur = yCur = 0; 
  while (1) {           /* for ever */
    id = startId;
    x = 0; y = 0;
    id = -1;
#ifdef DEBUG_2
  if (DEBUG)
    dumpTable();
#endif
    while (1) {         /* do one screen */
      if (y >= 20) {
        y = 0;
        x += width;
        if (x > (80 - width)) {
          break;
        }
      }
      id ++;

      id = nextId(id);

      if (clearPrev && (c_y == y) && (c_x == x)) {
        clearVarBackup(id);
      }
      if (update && (yCur == y) && (xCur == x)) {
        clearVarBackup(id);
      }

      if ((str = nextVar(id)) == (char *) -1) {
        break;
      } else if (str == 0) {      /* skip this */
        ;
      } else if (*str == 0) {
          continue;
      } else { 
/*
        gotoxy(0,20);
        printf("x=%d,y=%d,xCur=%d,yCur=%d,c_x=%,c_y=%,up=%d,cl=%d\n",
                                    x,y,xCur,yCur,c_x,c_y,update,clearPrev);
*/
        if (x == xCur && y == yCur) {
          gotoxy(0,22);
          printf("%s\n", (*((char *) metaLock(dm, meta, id)) ? "disable" : "enable "));
          gotoxy(x,y);
          inverse();
          currentId = id;
        }
/*
        if (update)
          inverse();
*/        
        if (clearPrev && c_x == x && c_y == y) {
          normal();
          clearPrev = 0;
        }
        gotoxy(x,y);
        print(str);
/*        if (update && currentId == id && x == xCur && y == yCur) {  */
        if (x == xCur && y == yCur) {
          normal();
          update = 0;
        }
      }
      y ++;
    }       /* one screen done ! */

    if (_gs_rdy(0) > 0) {
      char c;
     
      c_y = yCur;
      c_x = xCur;

      read(0, &c, 1);
/*
      if (c < 0x20)
        c += 0x40;
      else if (c & 0x20)
        c ^= 0x20;
*/
      
      if (c == 'A' || c == 'P' - 0x40) {  /* ok, utom from (0,0) -> (max,max) */
        if (yCur)
          yCur --;
        else if (xCur >= width) {   
          yCur = 19;
          xCur -= width;
        } else {        /* from (0,0) <-  we get to (xMax,yMax) */
          xCur = x;
          yCur = y - 1;
        }
      } else if (c == 'B' || c == 'N' - 0x40) {
        if (xCur == x) {
          if (yCur < y - 1)
            yCur++;
          else {
            yCur = 0;
            xCur = 0;
          }
	} else if (yCur < 19)           /* < 20 */
          yCur++;
        else if (yCur == 19) {
          if (xCur < x) {
            yCur = 0;
            xCur += width;
          }
        } 
      } else if (c == 'V' - 0x40) {
        first = 1;
        clearScreen();
        startId += 20;
        initMatchTable(startId); 
        xCur = yCur = 0; 
        prevBuff[0] = 0;
        continue;
      } else if (c == 'Z' - 0x40) {
        if (startId >= 20) {
          first = 1;
          clearScreen();
          startId -= 20;
          initMatchTable(startId); 
          xCur = yCur = 0; 
          prevBuff[0] = 0;
        }
        continue;
      } else if (c == 'C' || c == 'F' - 0x40) {
        if (xCur < x) {         /* <= (80 - width))  */
          xCur += width;
          if (xCur >= x) {
            if (yCur >= y) {
              yCur ++;
              if (yCur >= 20) {
                yCur = 0;
              } else
                xCur = 0;
            }
          }
        } else /* if (x > 0) */ {
          xCur = 0;
          yCur ++;
          if (yCur >= 20) {
            yCur = 0;
            xCur = 0;
          }
        }
      } else if (c == 'D' || c == 'B' - 0x40) {
        if (xCur >= width)
          xCur -= width;
        else if (yCur > 0) {
          xCur = x;
          yCur --;
          if (yCur >= y)
            yCur = y - 1;
        } else {        /* from (0,0) <-  we get to (xMax,yMax) */
          xCur = x;
          yCur = y - 1;
        }
      } else if (c == 'L' - 0x40) {
        static int prevLockId;
        int i = 0;
        if (prevLockId == currentId) {
          char buf[20], locked;
          
          gotoxy(0,22);
          enableXonXoff(0);
          locked = *((char *) metaLock(dm, meta, currentId));
          if (locked)
            printf("Lock all : (n) ");
          else
            printf("Unlock all : (n) ");
          if (strlen(gets(buf))) {
            if (buf[0] == 'y' || buf[0] == 'Y' || 
                                 buf[0] == 'j' || buf[0] == 'J') {
              char *p;
              while (1) {
                if (p = ((char *) metaLock(dm, meta, ++i)))
                  *p = locked;
                else
                  break;
              }
            }
          }
        }
        if (i == 0)
          *((char *) metaLock(dm, meta, currentId)) ^= 1;
        prevLockId = currentId;
      } else if (c == 'W' - 0x40) {
        char buf[20];
        gotoxy(0,22);
        enableXonXoff(0);
        printf("Enter width: ");
        if (strlen(gets(buf))) {
          width = atoi(buf);
        }
        disableXonXoff(0);
        clearScreen();
        xCur = yCur = 0; 
        prevBuff[0] = 0;
        first = 1;
        continue;
      } else if (c == 'M' - 0x40) {
        double atof(), buf[3], v;
        gotoxy(0,22);
        enableXonXoff(0);
        printf("Enter value: ");
        if (strlen(gets(buf))) {
          v = atof(buf);
          if (metaType(meta, currentId) == TYPE_INT) 
            *((long *) metaValue(dm, meta, currentId)) = (long) v;
          else if (metaType(meta, currentId) == TYPE_FLOAT) 
            *((double *) metaValue(dm, meta, currentId)) = v;
          else
            ;
        }
        disableXonXoff(0);
        gotoxy(0,22);
        printf("                                   \n");
      } else if (c == '+') {
        char buf[40];
        gotoxy(0,22);
        if (pattCnt > 9)
          printf("no more patterns accepted\n");
        else {
          enableXonXoff(0);
          printf("Add search pattern : ");
          if (strlen(gets(buf))) {
            strcpy(pattern[pattCnt++], buf);
          }
        }
        disableXonXoff(0);
        gotoxy(0,22);
        printf("                                   \n");
        first = 1;
        clearScreen();
        initMatchTable(startId); 
        xCur = yCur = 0; 
        prevBuff[0] = 0;
        continue;
      } else if (c == '-') {

      } else if (c == '.') {
        char buf[40];
        gotoxy(0,22);
        enableXonXoff(0);
        printf("New search pattern : ");
        pattCnt = 0;
        if (strlen(gets(buf))) {
          strcpy(pattern[pattCnt++], buf);
        }
        disableXonXoff(0);
        gotoxy(0,22);
        printf("                                   \n");
        first = 1;
        clearScreen();
        initMatchTable(startId);
        xCur = yCur = 0; 
        prevBuff[0] = 0;
        continue;
      } else if (c == 'X' - 0x40) {
        gotoxy(0, 21);
        Abort();
      }
      update = 1;
      u_y = yCur;
      u_x = xCur;      
      if (c_y != yCur || c_x != xCur) {
        clearPrev = 1;
      }

      
    }
   
/*
    gotoxy(0,22);
    printf("up=%d,cl=%d, xCur=%d,yCur=%d, currentId=%d, c_x=%d,c_y=%d\n", update, clearPrev, xCur, yCur,currentId, c_x, c_y);
*/      
   
/*    
    if (xCur != xPrevCur || yCur != yPrevCur) {
      update = 1;
      xPrevCur = xCur;
      yPrevCur = yCur;
    }
*/ 
    uc = cf = ns = 0;
    cnt = aldm->alarmListPtr;
    for (i = 0; i < cnt; i++) {
      if (!aldm->alarmList[i].confirm)
        uc ++;
      else if (aldm->alarmList[i].active)
        cf ++;

      if (aldm->alarmList[i].sendStatus & ALARM_SEND_INIT) {
        if (alarmStatus(&aldm->alarmList[i]))
          ns++;
        else
          ; /* sent */
      } else 
        ns++;     /* not init -> not sent */
/*
      if ((aldm->alarmList[i].sendAssert && !aldm->alarmList[i].assertSent) ||
          (aldm->alarmList[i].sendNegate && !aldm->alarmList[i].negateSent) ||
          (aldm->alarmList[i].sendConfirm && !aldm->alarmList[i].confirmSent))
        ns++;
*/
    }
    cNow = time(0);
    if (puc != uc || pcf != cf || pns != ns || cNow != cPrev) {
      char buff[100];
      
      puc = uc;
      pcf = cf;
      pns = ns;
      cPrev = cNow;
      
      sprintf(buff,
         "Antal alarm: %d okvitterade, %d kvitterad, (%d Ej skickade) %s   \n",
          uc, cf, ns, ctid(cNow)); 
      
      printfDiff(0, 21, buff, prevBuff);
/*     
      gotoxy(0, 21);
      print(buff);
*/
    }

    if (!(x | y))
      break;
    first = 0;
    if (sleepTime)
      tsleep(sleepTime);
  }
  Abort();
}

int printfDiff(x, y, buff, prevBuff)
int x, y;
char *buff, *prevBuff;
{
  while (*buff == *prevBuff) {
    x++;    buff++;    prevBuff++;
  }
  gotoxy(x, y);
  print(buff);
  strcpy(prevBuff, buff);
}  

checkMatch(id)
int id;
{
  int i;
  char *p;
  
  if (!(p = (char *) metaValue(dm, meta, id))) {
#ifdef DEBUG_2
    if (DEBUG) printf("checkMatch: returns -1, id = %d\n", id);
#endif
    return -1;
  }
  for (i = 0; i < pattCnt; i++)
    if (!_cmpnam(metaName(meta, id), pattern[i], strlen(pattern[i])))
      break;
  if (pattCnt && (i >= pattCnt)) {
    return 0;
  }

  if (metaType(meta, id) == TYPE_INT) {
    return 1;
  } else if (metaType(meta, id) == TYPE_FLOAT) {
    return 1;
  }    
  return 0;
}

short int matchTable[512];


initMatchTable(startId)
int startId;
{
  int i = 0, ok, id = 0;
  id++;
  while ((ok = checkMatch(id)) >= 0) {
    if (ok) {
      if (i >= startId) {
        if ((i - startId) > 510)
          break;
        matchTable[i - startId] = id;
      }
      i++;
    }
    id++;
  }
  matchTable[i] = -1;

#ifdef DEBUG_2
  if (DEBUG)
    printf("initMatchTable: i = %d, id = %d\n", i, id);
#endif
}

clearVarBackup(id)
int id;
{
  char x;
  
  x = *((char *) metaValue(dmBackup, meta, id));
  x ++;
  *((char *) metaValue(dmBackup, meta, id)) = x;
}

#ifdef DEBUG_2
dumpTable()
{
  int i; 
  printf("dumpTable:\n");
  for (i = 0; i < 25; i++) {
    printf("  matchTable[%d] = %d\n", i, matchTable[i]);
  }
}
#endif

int nextId(id)
int id;
{
  int i;
  static int NEXT; 

#ifdef DEBUG_2
  if (DEBUG)
    printf("nextId: IDin = %d, NEXT = %d", id, NEXT);
#endif
  if (id == 0)
    NEXT = 0; 
  if ((id = matchTable[NEXT++]) == -1) {
    NEXT = 0;
#ifdef DEBUG_2
  if (DEBUG)
    printf(", returns -1\n");
#endif
    return -1;
  }
#ifdef DEBUG_2
  if (DEBUG)
    printf(", returns %d\n", id);
#endif
  return id;
}

/*
!   checks next variable in meta module
!   returns;
!     -1                      when the index is out of range
!      0                      variable has not changed, check next
!    outbuf[0] = '\0'         not match against pattern
!    outbuf = name + value    ok !
*/  
char *nextVar(id)
int id;
{
  static char outBuf[80];
  char buff[80];
  char *vPtr;
  int i;
  
  if (!(vPtr = (char *) metaValue(dm, meta, id))) {
#ifdef DEBUG_2
    if (DEBUG) printf("nextVar: metaValue -> returns -1, id = %d\n", id);
#endif
    return -1;
  }

  
  if (memcmp(vPtr, metaValue(dmBackup, meta, id),
               (metaType(meta, id) == TYPE_INT) ? 4 : 8) || first) {
    if (metaType(meta, id) == TYPE_INT) {
      *((int *) metaValue(dmBackup, meta, id)) = *((int *) vPtr);
      sprintf(buff, "%d", *((int *) vPtr));
    } else if (metaType(meta, id) == TYPE_FLOAT) {
      *((double *) metaValue(dmBackup, meta, id)) = *((double *) vPtr);
      sprintf(buff, "%g", *((double *) vPtr));
    } else {
/*      sprintf(buff, "- - -");*/          /* none implemented type */
      outBuf[0] = 0;              /* patterns exists but no match */
      return outBuf;

    }
    sprintf(outBuf, "%s = %s                                              ",
       metaName(meta, id), buff);
    outBuf[width] = 0;
    if (outBuf[width - 1] != ' ')
      outBuf[width - 1] = '*';
  }   /* else updated */
  else
    return 0;
  return outBuf; 
}

bind(dm, aldm, meta, headerPtr1, headerPtr2, headerPtr3)
char **dm, **meta, **headerPtr1, **headerPtr2, **headerPtr3;
struct _alarmModule **aldm;
{
/*
!   bind to data module VARS, storage location for variables
*/  
  *dm = (char *) linkDataModule(NAMEOFDATAMODULE, headerPtr1);
  if (!*dm) {
    fprintf(stderr, "cannot link to datamodule '%s'\n", NAMEOFDATAMODULE);
    fprintf(stderr, "check if process 'scan' is running\n");
    return 0;
  }
/*
!   bind to data module META, storage for meta description of VARS module
*/
  *meta = (char *) linkDataModule("METAVAR", headerPtr2);
  if (!*meta) {
    fprintf(stderr, "cannot link to datamodule '%s'\n", "METAVAR");
    fprintf(stderr, "check if process 'scan' is running\n");
    return 0;
  }
/*
!   bind to data module ALARM, storage for alarm texts
*/
  *aldm = (struct _alarmModule *) linkDataModule("ALARM", headerPtr3);
  if (!*aldm) {
    fprintf(stderr, "cannot link to datamodule '%s'\n", "ALARM");
    fprintf(stderr, "check if process 'scan' is running\n");
    return 0;
  }
}

struct sgbuf copy;
enableXonXoff(path)
int path;
{
  if (_ss_opt(path, &copy) == -1) {
    fprintf(stderr, "error during _ss_opt: %d\n", errno);
    exit(errno);
  }
}


disableXonXoff(path)
int path;
{
  struct sgbuf buffer;
  
  if (_gs_opt(path, &buffer) == -1) {
    fprintf(stderr, "error during _gs_opt: %d\n", errno);
    exit(errno);
  }
  memcpy(&copy, &buffer, sizeof(struct sgbuf));
  buffer._sgm._sgs._sgs_xon   = 0;
  buffer._sgm._sgs._sgs_xoff  = 0;
  buffer._sgm._sgs._sgs_echo  = 0;
  buffer._sgm._sgs._sgs_kbach = 0;      /* keyboard abort character */
  
  if (_ss_opt(path, &buffer) == -1) {
    fprintf(stderr, "error during _ss_opt: %d\n", errno);
    exit(errno);
  }
}


