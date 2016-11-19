/* enter.c  1992-06-17 TD,  version 1.3 */
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
! enter.c
! Copyright (C) 1991,1992 IVT Electronic AB.
*/

/*
!     enter.c is part of the IDCIO trap module, included in IVT 16 system
!
!     File: enter.c
!     
!     Contains main function for opening and starting parsing of IDC-files.
!
!     History     
!     Date        Revision Who  What
!     
!     5-feb-1992      1.1  TD   Updated set_time to handle year > 1999
!
!    17-jun-1992      1.3  TD   Added priviledges
!
!
*/

/*   define this if we should ask for password whenever insufficient level
/*                      
 #define ADDED_920615
*/

#include <time.h>
#include <math.h>
#include <varargs.h>
#include <ctype.h>

#include "idcio.h"
#include "phyio.h"
#include "priv.h"
#define TIMEOUT_VALUE 300

extern char *NAME_OF_VAR_MODULE;
extern char *NAME_OF_META_MODULE;

#define ADDED_PRIVILEGE_920617

int getLevel()
{
  char *dm, *meta, *headerPtr1, *headerPtr2;
  int level, id;
  
  dm = (char *) linkDataModule(NAME_OF_VAR_MODULE, &headerPtr1);
  meta = (char *) linkDataModule(NAME_OF_META_MODULE, &headerPtr2);
  if (!dm || !meta)
    return -1;

  if ((id = metaId(meta, "level")) > 0)
    level = *((int *) metaValue(dm, meta, id)) ;
  else
    level = 0;
  unlinkDataModule(headerPtr1);
  unlinkDataModule(headerPtr2);
  return level;
}

#ifdef ADDED_PRIVILEGE_920617
int checkAccess(access_mask)   /* do we have the privilege to 'access_mask' ? */
int access_mask;
{
  int i;
  if ((i = getLevel()) == -1)
    return 0;
  return getPrivilege(i) & access_mask;
}
  
int getPrivilege(level)
int level;
{
  char *dm, *meta, *headerPtr1, *headerPtr2;
  int priv, id;
  
  dm = (char *) linkDataModule(NAME_OF_VAR_MODULE, &headerPtr1);
  meta = (char *) linkDataModule(NAME_OF_META_MODULE, &headerPtr2);
  if (!dm || !meta)
    return -1;

  if ((id = metaId(meta, "userPrivileges")) > 0)
    priv = ((int *) metaValue(dm, meta, id))[level];
  else {
    /* not available, maybe old system, use default */
    if (level == 0)
      priv = 0;
    else if (level == 1)
      priv = PRIV_M_OLD_LEVEL_1;
    else if (level == 2)
      priv = PRIV_M_OLD_LEVEL_2;
    else if (level == 3)
      priv = PRIV_M_OLD_LEVEL_3;
    else if (level == 4)
      priv = PRIV_M_OLD_LEVEL_4;
    else if (level == 5)
      priv = PRIV_M_OLD_LEVEL_5;
  }
  unlinkDataModule(headerPtr1);
  unlinkDataModule(headerPtr2);
  return priv;
}
#endif

int sleeptight()
{
  int ticks;
  
  ticks = sleep(3);
  while (ticks) {
    ticks = tsleep(ticks);
  }
}

#define NEW_ENTER
#ifdef NEW_ENTER
double ptrprintf(ap)
va_list *ap;
{
  char *stf, *p, line[80], *fmtPek, fmt[40], *sval;
  int ival, offset, width;
  double dval;
  double v1 = 0.0;
  int first = 0;
  
  stf = va_arg(*ap, char *);
  for (p = stf; *p; p++) {
    if (*p != '%') {
      lcdwrite(*p);
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
        ival = va_arg(*ap, int);
        --fmtPek;
        *fmtPek = '\0';  
        fmtPek = fmt; fmtPek ++;
        offset = 0;
        width = 8;
        if (*fmtPek) {
          width = atoi(fmtPek);
          while (*fmtPek && *fmtPek != '.')
            fmtPek++;
          if (*fmtPek) {
            offset = width;
            fmtPek++;
            width = atoi(fmtPek);
          }
        }
        line[0] = '\0';
        (ival) >>= offset;
        line[width] = '\0';
        while (width ) {
          line[--width] = ((ival) & 1) ? '\2' : '\1';
          (ival) >>= 1;
        }
        lcdputs(line);
        break;
      case 'c':
        ival = va_arg(*ap, int);
        sprintf(line, fmt, ival);
        lcdputs(line);
        break;
      case 'd':
        ival = va_arg(*ap, int);
        sprintf(line, fmt, ival);
        lcdputs(line);
        if (!first++) {
          v1 = (double) ival;
        }
        break;
      case 'D':
        ival = va_arg(*ap, int);
        if (!first++) {
          v1 = (double) ival;
        }
        break;
      case 'f':
        dval = va_arg(*ap, double);
        sprintf(line, fmt, dval);
        lcdputs(line);
        if (!first++)
          v1 = dval;
        break;
      case 'F':
        dval = va_arg(*ap, double);
        if (!first++)
          v1 = dval;
        break;
      case 'g':
        dval = va_arg(*ap, double);
        sprintf(line, fmt, dval);
        lcdputs(line);
        if (!first++)
          v1 = dval;
        break;
      case 's':
        sval = va_arg(*ap, char *);
        sprintf(line, fmt, sval);
        lcdputs(line);
        break;
      default:
        lcdwrite(*p);
        break;
    }
  }
/*  va_end(*ap);    */
  return v1;
} 

double getValue(v1)
double v1;
{
  char buff[40];
  int pos, keyCode, point = 0;

  lcdpos(1, 0);
  lcdprintf("                                      ");
  lcdpos(1, 1);
  lcdcursorOn();
  buff[0] = ' '; buff[1] = '\0';
  pos = 1;
  while (1) {
    switch (keyCode = getKey()) {
      case NO_KEY:                  /* timeout, return old value */
        lcdcursorOff();
        lcdpos(1, 0);
        lcdprintf("                                      ");
        return v1;
      case KEY_CHANGE:              /* no change, return old value */
/*        if (changed) {    */
          lcdcursorOff();
          lcdpos(1, 0);
          lcdprintf("                                      ");
          return v1;
/*        }     */
        break;
      case KEY_ENTER:               /* ok, enter finished */
        lcdcursorOff();
        lcdpos(1, 0);
        lcdprintf("                                      ");
        if (buff[1])
          return atof(buff);
        return v1;
      case KEY_LEFT:                /* dismis last digit entered, if: */
        if (pos > 1) {              /* any has been entered */
          pos --;
          if (buff[pos] == '.')
            point = 0;
          buff[pos] = '\0';
          lcdpositRel(-1, 0);
          lcdwrite(' ');
          lcdpositRel(-1, 0);
          if (pos == 1) {           /* if no more digits reset any sign */
            lcdpositRel(-1, 0);
            lcdwrite(' ');
          }
        }
        break;
      case KEY_PLUS_MINUS:        /* change sign */
        if (pos > 1) {
          lcdpos(1, 0);
          if (buff[0] == '-')
            lcdwrite(buff[0] = ' ');
          else
            lcdwrite(buff[0] = '-');
          lcdpos(1, pos);
        }
        break;
      case KEY_POINT:
        if (!point) {
          point = 1;
          if (pos == 1) {
            lcdpos(1, pos);
            buff[pos++] = '0';
            lcdwrite('0');
          }
          lcdpos(1, pos);
          buff[pos++] = '.';
          buff[pos] = '\0';
          lcdwrite('.');
        }
        break;
      case KEY_RIGHT:
      case KEY_UP:
      case KEY_DOWN:
      case KEY_ALARM:
      case KEY_HELP:
        break;
      default:
        lcdpos(1, pos);
        buff[pos++] = '0' + keyCode;
        buff[pos] = '\0';
        lcdwrite('0' + keyCode);
        break;
    }
  }
}

#ifdef ADDED_PRIVILEGE_920617
int checkAccessLevel(access_mask)
int access_mask;
#else
int checkAccessLevel(lev)
int lev;
#endif
{
  int i;
  char buf[80];

#ifdef ADDED_PRIVILEGE_920617
  if (((i = getLevel()) != -1) && (!(getPrivilege(i) & access_mask))) {
#else    
  if (((i = getLevel()) < lev) && (i != -1)) {
#endif

    while (keyDown())
      ;

#ifdef ADDED_920615                     /* 920615, added these 3 lines*/
    lcdGetScreen(buf);    /* capture screen as is */
    lcdprintf("\f");
    lcdpos(1, 0);
    lcdprintf("F\024r l\06g beh\024righetsniv\06 !             ");
    lcdhome();
    password();
#ifdef ADDED_PRIVILEGE_920617
    if (((i = getLevel()) != -1) && (!(getPrivilege(i) & access_mask)))
#else    
    if (((i = getLevel()) < lev) && (i != -1))
#endif
      ;
    else {
      lcdPutScreen(buf);
      return 1;
    }
#endif
    
    lcdpos(1, 0);
    accessDenied();   /*    lcdprintf("F\024r l\06g beh\024righetsniv\06 !             ");    */
    lcdprintf("             ");
    sleeptight(2); 
    lcdprintf("\f");
    return 0;
  } else
    return 1;
}

double enter(va_alist)
va_dcl
{
  va_list ap;
  int pos, keyCode, point = 0, changed = 0;
  char buff[40], backup[40];
  time_t timer;
  double v1;

  /* bug !! cio cannot handle SMALL numbers e.g 3.8241e-112 !!! */
/*  if (abs(*v1) < 0.000010) {    */            /* changed 91-09-09 */

/*
  if ((*v1 < 0.000010) && (*v1 > -0.000010)) {
    *v1 = 0.0;
  }
*/

  lcdsetCacheCursor(1);     /* don't want cursor sweeping over display */
  timer = time(0);
/*  do {            */
    lcdhome();
    va_start(ap);
    v1 = ptrprintf(&ap);

/*
    lcdprintf(s, *v1, ((int) v2 > 0x3ffff) ? 0 : *v2,
                      ((int) v3 > 0x3ffff) ? 0 : *v3,
                      ((int) v4 > 0x3ffff) ? 0 : *v4,
                      ((int) v5 > 0x3ffff) ? 0 : *v5);
*/

    if ((time(0) - timer) > TIMEOUT_VALUE)
      return v1;
/*  } while (!keyDown());     
  if (key() != KEY_CHANGE)
    return v1;
*/
  if (!keyDown() || key() != KEY_CHANGE)      /* new 920107 */
    return v1;

  if (!checkAccessLevel(PRIV_M_ENTER /* MIN_LEVEL_ENTER */ ))
    return v1;
    
  while (keyDown())     /* wait for release of change-key */
    ;
      
    
  return getValue(v1); 
    
#ifdef OLD_ENTER

  lcdpos(1, 0);
  lcdprintf("                                      ");
  lcdpos(1, 1);
  lcdcursorOn();
  buff[0] = ' '; buff[1] = '\0';
  pos = 1;
  while (1) {
    switch (keyCode = getKey()) {
      case NO_KEY:                  /* timeout, return old value */
        lcdcursorOff();
        lcdpos(1, 0);
        lcdprintf("                                      ");
        return v1;
      case KEY_CHANGE:              /* no change, return old value */
        if (changed) {
          lcdcursorOff();
          lcdpos(1, 0);
          lcdprintf("                                      ");
          return v1;
        }
        break;
      case KEY_ENTER:               /* ok, enter finished */
        lcdcursorOff();
        lcdpos(1, 0);
        lcdprintf("                                      ");
        if (buff[1])
          return atof(buff);
        return v1;
      case KEY_LEFT:                /* dismis last digit entered, if: */
        if (pos > 1) {              /* any has been entered */
          pos --;
          if (buff[pos] == '.')
            point = 0;
          buff[pos] = '\0';
          lcdpositRel(-1, 0);
          lcdwrite(' ');
          lcdpositRel(-1, 0);
          if (pos == 1) {           /* if no more digits reset any sign */
            lcdpositRel(-1, 0);
            lcdwrite(' ');
          }
        }
        break;
      case KEY_PLUS_MINUS:        /* change sign */
        if (pos > 1) {
          lcdpos(1, 0);
          if (buff[0] == '-')
            lcdwrite(buff[0] = ' ');
          else
            lcdwrite(buff[0] = '-');
          lcdpos(1, pos);
        }
        break;
      case KEY_POINT:
        if (!point) {
          point = 1;
          if (pos == 1) {
            lcdpos(1, pos);
            buff[pos++] = '0';
            lcdwrite('0');
          }
          lcdpos(1, pos);
          buff[pos++] = '.';
          buff[pos] = '\0';
          lcdwrite('.');
        }
        break;
      case KEY_RIGHT:
      case KEY_UP:
      case KEY_DOWN:
      case KEY_ALARM:
      case KEY_HELP:
        break;
      default:
        lcdpos(1, pos);
        buff[pos++] = '0' + keyCode;
        buff[pos] = '\0';
        lcdwrite('0' + keyCode);
        break;
    }
    changed = 1;
  }
#endif

}
#else
double enter(s, v1, v2, v3, v4, v5)
char *s;
double *v1, *v2, *v3, *v4, *v5;
{
  int pos, keyCode, point = 0, changed = 0;
  char buff[40], backup[40];
  time_t timer;

  /* bug !! cio cannot handle SMALL numbers e.g 3.8241e-112 !!! */
/*  if (abs(*v1) < 0.000010) {    */            /* changed 91-09-09 */

  if ((*v1 < 0.000010) && (*v1 > -0.000010)) {
    *v1 = 0.0;
  }
  lcdsetCacheCursor(1);     /* don't want cursor sweeping over display */
  timer = time(0);
/*  do {          */
    lcdhome();
    lcdprintf(s, *v1, ((int) v2 > 0x3ffff) ? 0 : *v2,
                      ((int) v3 > 0x3ffff) ? 0 : *v3,
                      ((int) v4 > 0x3ffff) ? 0 : *v4,
                      ((int) v5 > 0x3ffff) ? 0 : *v5);
    if ((time(0) - timer) > TIMEOUT_VALUE)
      return *v1;
/*  } while (!keyDown());       */
/*  if (key() != KEY_CHANGE)
    return *v1;
*/

  if (!keyDown() || key() != KEY_CHANGE)      /* new 920107 */
    return *v1;

  if (getLevel() < 3) {
    while (keyDown())
      ;
    lcdpos(1, 0);
    accessDenied();
    lcdprintf("             ");
    sleeptight(2); /* tsleep(1 << 31 | 3 << 8); */
    lcdprintf("\f");
    return *v1;
  }
    
  lcdpos(1, 0);
  lcdprintf("                                      ");
  lcdpos(1, 1);
  lcdcursorOn();
  buff[0] = ' '; buff[1] = '\0';
  pos = 1;
  while (1) {
    switch (keyCode = getKey()) {
      case NO_KEY:                  /* timeout, return old value */
        lcdcursorOff();
        lcdpos(1, 0);
        lcdprintf("                                      ");
        return *v1;
      case KEY_CHANGE:              /* no change, return old value */
        if (changed) {
          lcdcursorOff();
          lcdpos(1, 0);
          lcdprintf("                                      ");
          return *v1;
        }
        break;
      case KEY_ENTER:               /* ok, enter finished */
        lcdcursorOff();
        lcdpos(1, 0);
        lcdprintf("                                      ");
        if (buff[1])
          return atof(buff);
        return *v1;
      case KEY_LEFT:                /* dismis last digit entered, if: */
        if (pos > 1) {              /* any has been entered */
          pos --;
          if (buff[pos] == '.')
            point = 0;
          buff[pos] = '\0';
          lcdpositRel(-1, 0);
          lcdwrite(' ');
          lcdpositRel(-1, 0);
          if (pos == 1) {           /* if no more digits reset any sign */
            lcdpositRel(-1, 0);
            lcdwrite(' ');
          }
        }
        break;
      case KEY_PLUS_MINUS:        /* change sign */
        if (pos > 1) {
          lcdpos(1, 0);
          if (buff[0] == '-')
            lcdwrite(buff[0] = ' ');
          else
            lcdwrite(buff[0] = '-');
          lcdpos(1, pos);
        }
        break;
      case KEY_POINT:
        if (!point) {
          point = 1;
          if (pos == 1) {
            lcdpos(1, pos);
            buff[pos++] = '0';
            lcdwrite('0');
          }
          lcdpos(1, pos);
          buff[pos++] = '.';
          buff[pos] = '\0';
          lcdwrite('.');
        }
        break;
      case KEY_RIGHT:
      case KEY_UP:
      case KEY_DOWN:
      case KEY_ALARM:
      case KEY_HELP:
        break;
      default:
        lcdpos(1, pos);
        buff[pos++] = '0' + keyCode;
        buff[pos] = '\0';
        lcdwrite('0' + keyCode);
        break;
    }
    changed = 1;
  }
}
#endif

static int checkLevel(passw)
int passw;
{
  char *dm, *meta, *headerPtr1, *headerPtr2;
  int level, id, j;
  
  dm = (char *) linkDataModule(NAME_OF_VAR_MODULE, &headerPtr1);
  meta = (char *) linkDataModule(NAME_OF_META_MODULE, &headerPtr2);
  if (!dm || !meta)
    return -1;

#ifdef ADDED_PRIVILEGE_920617
  level = -1;
  if (((id = metaId(meta, "levels")) > 0) && (metaType(meta, id) == 4)) {
    for (j = (metaSize(meta, id) / sizeof(long)) - 1; j >= 0; j--) {
      if (passw == ((int *) metaValue(dm, meta, id))[j]) {
        level = j;
        break;
      }
    }
  } else
#endif
  if (((id = metaId(meta, "level5")) > 0) && 
        (passw == *((int *) metaValue(dm, meta, id))))
    level = 5;
  else if (((id = metaId(meta, "level4")) > 0) && 
        (passw == *((int *) metaValue(dm, meta, id))))
    level = 4;
  else if (((id = metaId(meta, "level3")) > 0) && 
        (passw == *((int *) metaValue(dm, meta, id))))
    level = 3;
  else if (((id = metaId(meta, "level2")) > 0) && 
        (passw == *((int *) metaValue(dm, meta, id))))
    level = 2;
  else if (((id = metaId(meta, "level1")) > 0) && 
        (passw == *((int *) metaValue(dm, meta, id))))
    level = 1;
  else
    level = -1;

/*   if (level != 5)  */                     /* added 920107 */
  if (level != -1) {
    if ((id = metaId(meta, "level")) > 0)
      *((int *) metaValue(dm, meta, id)) = level;
  }
  unlinkDataModule(headerPtr1);
  unlinkDataModule(headerPtr2);
  return level;
}

struct _node
{
  unsigned char failCount;
  unsigned char full;               /*  added 920306 */  
  short size;
  unsigned short int rCount, sCount;
  unsigned short noOfAlarms;       /* come, gone, disable, confirm -> 4*100 */

  unsigned char pollCount;        /* incremented for each poll */
  unsigned char failFlag;
/*  added 920306 */
  unsigned char A,B,C,D,cA,cB,cC,cD;
/*
  unsigned long activeABCD;
  unsigned long confirmedABCD;
*/
  unsigned short version;
};    /* size = 22 * MAX_NODES = 1408 */

struct _node *nodeMap = (struct _node *) 0x0003f000;
/*
      printf(
"Node %2d fail %3d size %3d, count (%5d, %5d), alarms %5d, polls %d, (%d)\n",
          i, nodeMap[i].failCount, nodeMap[i].size,
             nodeMap[i].rCount, nodeMap[i].sCount,
             nodeMap[i].noOfAlarms, nodeMap[i].pollCount, nodeMap[i].failFlag);
*/
#define MAX_NODE_NO 14

static short int *nodeNo = 0x402;
static short int *iplCode = 0x440;
static char *iplString = 0x442;


#define InSize 1024 /* input buffer size */
#define OutSize 1024 /* output buffer size */
#define FrmSize 32 /* frame size */

typedef unsigned char * PTR;
typedef char BYTE;
typedef unsigned short WORD;

/* Device static storage */
typedef struct {
 PTR Shadow; /* ds.l 1 pointer to shadow register */
 PTR Handler; /* ds.l 1 current input interrupt handler */
 PTR InRead; /* ds.l 1 input-buffer read pointer */
 PTR InWrite; /* ds.l 1 input-buffer write pointer */
 PTR InEnd; /* ds.l 1 input-buffer end pointer */
 PTR OutRead; /* ds.l 1 output-buffer read pointer */
 PTR OutWrite; /* ds.l 1 output-buffer write pointer */
 PTR OutEnd; /* ds.l 1 output-buffer end pointer */
 PTR DevEven; /* ds.l 1 pointer to even device half */
 WORD InCount; /* ds.w 1 input-buffer level */
 WORD OutCount; /* ds.w 1 output-buffer level */
 WORD IRQMask; /* ds.w 1 device irq level mask */
 WORD Station; /* ds.w 1 netnode station id code */
 WORD MsbReg; /* ds.w 1 current address modifier */
 WORD LastAccs; /* ds.w 1 last accessed node */
 WORD CmdAccs; /* ds.w 1 node accessed during this command transfer */
 WORD NextAccs; /* ds.w 1 the next node to access */
 WORD PkSize; /* ds.w 1 current blocking size */
 WORD PkRdy; /* ds.w 1 current ready size */
 WORD PkIn; /* ds.w 1 current input block size */
 WORD PkOut; /* ds.w 1 current output block size */
 BYTE PkCmd; /* ds.b 1 the last transmitted command byte */
 BYTE PkMode; /* ds.b 1 current packet transfer mode */
 BYTE PkType; /* ds.b 1 current packet type (0= data, $80= commands) */
 BYTE PkFlag; /* ds.b 1 remote station accept flag */
 BYTE PkError; /* ds.b 1 accumulated errors */
 BYTE Select; /* ds.b 1 node select (0 = not selected) */
 BYTE MsgByte; /* ds.b 1 the last message byte sent to the other node */
 BYTE Parity; /* ds.b 1 parity code value */
 BYTE BaudRate; /* ds.b 1 baudrate code value */
 BYTE IrqFlags; /* ds.b 1 enabled irqs for this device */
 BYTE RtsBit; /* ds.b 1 enable for RTS output */
 BYTE InBuf[InSize]; /* ds.b InSize input data buffer */
 BYTE OutBuf[OutSize]; /* ds.b OutSize output data buffer */
 long FIX; /* testvalue */
 long NrofReads; /* ds.l 1 nr of good blocks read */
 long NrofWrites; /* ds.l 1 nr of good blocks written */
 long NrofErrors; /* ds.l 1 nr of errors */
 long NrofResets; /* ds.l 1 nr of resets */
 long TxStatus; /* ds.l 1 transmit status */
 WORD SigReady[3]; /* send signal on data ready */
 WORD SigPoll[3];  /* send signal on poll event */
 long Nrofpolls;
} netstatic;

show_netstat()
{
  netstatic *netstat;
  netstat = (netstatic *) *((long *) 0x40c);
      
  display("\f");
  if (*nodeNo == 0 || *nodeNo & 512)
    lcdshowdirections(4, 1);      /* down */
  do { 
    lcdhome();
    display("Tx=%1d,R=%d,W=%d,P=%d,Res=%d\n", 
                netstat->TxStatus,
                netstat->NrofReads,
                netstat->NrofWrites,
                netstat->Nrofpolls,        /* Errors, */
                netstat->NrofResets);
    display("InCnt=%4d,OutCnt=%4d, Pkerr=%d", 
                netstat->InCount, 
                netstat->OutCount,
                netstat->PkError);
  } while (!keyDown());
}

int netShow()
{
  int timer, key, screenId;
  register int node;
  
  display("\f");
/*
! new code !
*/
  { 
    char version[14];
    strncpy(version, (char *) 0x3fffd0, 13);      /* 920218 3bffd0 -> 3fffd0 */
    version[13] = 0;
    display("node %s%d: %s, memfree %d kB\n", (*nodeNo & 512) ? "M" : "",
                          ((int) *nodeNo) & 511, version, memfree() >> 10);
  }
  if (*iplCode != (short) 0xC0DE)
    display("No ipl string");
  else {
    char *p;
    int i;
    p = iplString;
    for (i = 0; i < 38; i++) {
      if (*p == 0x0d)
        break;
      display("%c", *p++);
    }
    if (i == 0)
      display("No ipl string");
  }

#define NEW_650  

#ifdef NEW_650
  screenId = 0;
  lcdshowdirections(4, 1);
  lcdshowdirections(1, 0);
  lcdshowdirections(2, 0);
  lcdshowdirections(3, 0);
  if (getKey() != KEY_DOWN) {
    display("\f");
    return 1;
  }
  while (1) {
    lcdshowdirections(4, (screenId < 6));    /* down */
    lcdshowdirections(1, (screenId > 0));    /* up */
    lcdshowdirections(2, (screenId >= 10));    /* left */
    lcdshowdirections(3, (screenId < 10));    /* right */
    showOurScreen(&screenId);
    switch(getKey()) {
      case KEY_DOWN:
        if (screenId < 6)
          screenId ++;
        break;
      case KEY_UP:
        if (screenId > 0)
          screenId --;
        break;
      case KEY_RIGHT:
        screenId += 10;
        break;
      case KEY_LEFT:
        display("\f");
        return 1;
    }    
  }
#else
  switch (getKey()) { 
    case KEY_DOWN:    /* show net status */
      show_netstat();
      if ((*nodeNo == 0 || (*nodeNo & 512)) && (getKey() == KEY_DOWN))
        show_all_nodes();
      break;
    case KEY_RIGHT:   /* change ipl string */
      change_ipl();
      break;
    case KEY_UP:
      display("\fPress Enter to clear all alarms !\n");
      if (KEY_ENTER == getKey())
        os9fork("clearalarm", 0, 0, 0, 0, 0, 0);
      break;
    case KEY_LEFT:
      display("\fPress Enter to reboot !\n");
      if (KEY_ENTER == getKey())
        os9fork("reboot", 0, 0, 0, 0, 0, 0);
      break;
    default:
      break;
  }
#endif
  display("\f");
  return 1;
}

#asm
memfree:
  moveq.l   #0,d0
  moveq.l   #0,d1
  OS9       F$GBlkMp
  move.l    d3,d0
  rts
#endasm

#ifdef NEW_650
showOurScreen(screenId)
int *screenId;
{
  display("\f");
  switch (*screenId) {
    case 0:
      display("Reboot ->");
      break;
    case 10:
      display("\fPress Enter to reboot !\n");
      if (KEY_ENTER == getKey()) {
        os9fork("reboot", 0, 0, 0, 0, 0, 0);
      }
      *screenId = 0;
      break;
    case 1:
      display("Clear alarms ->");
      break;
    case 11:
      display("\fPress Enter to clear all alarms !\n");
      if (KEY_ENTER == getKey()) {
        int status, pid;
        
        os9fork("clearalarm", 0, 0, 0, 0, 0, 0);
        display("Clearing...");
        while (!(pid = wait(&status)))
          ;
        if (status == 0)
          display("Ok !");
        else
          display("Err %d !", status);
/*        display("Cleared !");   */
      }
      *screenId = 1;
      break;
    case 2:
      display("Ipl string ->");
      break;
    case 12:
      change_ipl();
      display("\f");

  if (*iplCode != (short) 0xC0DE)
    display("No ipl string");
  else {
    char *p;
    int i;
    p = iplString;
    for (i = 0; i < 38; i++) {
      if (*p == 0x0d)
        break;
      display("%c", *p++);
    }
    if (i == 0)
      display("No ipl string");
  }

      *screenId = 2;
      break;
    case 3:
      display("Show variables ->");
      break;
    case 13:
      {
        char *dm, *meta, *headerPtr1, *headerPtr2;
        dm = (char *) linkDataModule(NAME_OF_VAR_MODULE, &headerPtr1);
        meta = (char *) linkDataModule(NAME_OF_META_MODULE, &headerPtr2);
        lcdshowdirections(1, 0);
        lcdshowdirections(2, 0);
        lcdshowdirections(3, 0);
        lcdshowdirections(4, 0);
        lcdsetCacheCursor(1);
        if (showVariables(dm, meta))      /* if timeout return */
          return 1;
        lcdsetCacheCursor(0);
        lcdprintf("\f");
/*        lcdcld();   */
        unlinkDataModule(headerPtr1);
        unlinkDataModule(headerPtr2);
      }
      *screenId = 3;
      break;
    case 4:
      display("Node info ->");
      break;
    case 14:
      show_netstat();
      if ((*nodeNo == 0 || *nodeNo & 512) && (getKey() == KEY_DOWN))
        show_all_nodes();
      *screenId = 4;
      break;
    case 5:
      display("SNPK info ->");
      break;
    case 15:
      display("1. Available nodes: %16a", get_avail_nodes(1));
      getKey();
      *screenId = 5;
      break;
    case 6:
      display("Logisk display-f\024rflyttning ->");
      break;
    case 16:
      display("Till node: ");
      {
        double f, getValue();
        int node, err;
        
        f = 0;
        node = getValue(f);
        
        display("\fV\04nta... (avbryt med '.') ");
        if ((err = netSetHost(node)) != 1) {
          display("error\n");
          if (err == 0)
            display("cannot find slave process");
          else if (err < 0)
            display("host duc not available: err=%d", err);
          else 
            display("Error message from node %d: err=%d", node, err);
        }
      }
      getKey();
      *screenId = 6;
      break;
  }
}
#endif

bitmask2Ipl(ip, mask)
char *ip;
int mask;
{
  int m;
  m = mask - 1;
      
  strcpy(ip, "verify;");
  if (m & 4) {
    if (*nodeNo == 0 || *nodeNo & 512)
      strcat(ip, "server&");
    else
      strcat(ip, "slave&");
  }
  if (m & 3)
    strcat(ip, "scan&");
  if (m & 2)
    strcat(ip, "main&");
  if (m & 8) 
    strcat(ip, "visionCom port 2");
  if (m & 16) 
    strcat(ip, "visionCom port 3");
}

change_ipl()
{
  char ip[80];
  int mask = 0;
  
  display("\f");
  while (1) {
    display("\f");
    if (mask > 0)
      lcdshowdirections(2, 1);      /* left */
    if (mask < 32)
      lcdshowdirections(3, 1);      /* right */

    if (mask == 0) {
      display("no - ipl - string\n");
      ip[0] = '\0';
    }
    else {
      bitmask2Ipl(ip, mask);
      display("%s", ip);
    }
     
    switch (getKey()) {
      case KEY_RIGHT:
        if (mask < 32) {
          mask ++;
          if ((mask & 3) == 3)
            mask ++;
        }
        break;
      case KEY_LEFT:
        if (mask > 0) {
          mask --;
          if ((mask & 3) == 3)
            mask --;
        }
        break;
      case KEY_ENTER:
        {
          int msk;
          char port[10], phone[10], pcno[2];
          msk = mask - 1;
          bitmask2Ipl(ip, (msk & 7) + 1);
          if (msk & 8) {
            getVisionSetup(2, port, phone, pcno, ip);
          }
          if (msk & 16) {
            getVisionSetup(3, port, phone, pcno, ip);
          }
          display("\f%s", ip);
          display("Ok ?");
          if (getKey() != KEY_ENTER)
            continue;
	}
        *iplCode = (short) 0xC0DE;
        memcpy(iplString, ip, strlen(ip));
        iplString[strlen(ip)] = 0x0d;
        return 1;
      default:
        return 1;
        break;
    }
  }
}

getVisionSetup(no, port, phone, pcno, ip)
int no;
char *port, *phone, *pcno, *ip;
{
  static char *port2[] = {"/n112", "/n124", "/n148", "/n196" };
  static char *port3[] = {"/n212", "/n224", "/n248", "/n296" };
  int k, st = 1, pn = 0;

  *port = *phone = *pcno = 0;
  while (1) {
    display("\fvisionCom %s %s %s\n", (no == 2) ? port2[pn] : port3[pn], 
                                              phone, pcno);
    if (st == 1) {
      display("Port nr: (+/-) ");
    } else if (st == 2) {
      display("Telefon: (\16ndra) ");
      while (!keyDown())
        ;
      if (key() == KEY_CHANGE) {
        long q;
        double f, getValue();
        
        k = getKey();
        f = 0;
        q = getValue(f);
        if (q > 0) {
          sprintf(phone, "%d", q);
          pcno[0] = '0';
          pcno[1] = 0;
        } else {
          *phone = 0;
          *pcno = 0;
        }
        st ++;
        continue;
      }
    } else if (st == 3) {
      display("Pcno: (+/-) ");
    }
    k = getKey();
    if (k == KEY_ENTER) {
     strcat(
     strcat(
       strcat(
        strcat(
          strcat(
            strcat(strcat(ip, "visionCom "), (no == 2) ? port2[pn] : port3[pn]),
              " "), phone), " "), pcno), "&");

      return 1;
    } else if (k == KEY_PLUS_MINUS) {
      if (st == 1) {
        pn ++;
        pn &= 3;
      } else if (st == 3) {
        if (*pcno != 0)
          (*pcno) ++;
        if (*pcno > '7')
          *pcno = '0';
      }
    } else if (k == KEY_RIGHT) {
      if (st < 3)
        st ++;
    } else if (k == KEY_LEFT) {
      if (st > 0)
        st --;
    }
  }
}

show_all_nodes()
{
  int node;
  
  display("\f");
  lcdshowdirections(1, 1);      /* up */
  lcdshowdirections(4, 1);      /* down */
  node = 1;
  while (1) {
    if (keyDown()) {
      switch (getKey()) {
        case KEY_UP:
          node --;
          if (node < 0)
            node = MAX_NODE_NO;
          break;
        case KEY_DOWN:
          node ++;
          if (node > MAX_NODE_NO)
            node = 0;
          break;
      case NO_KEY:                  /* timeout, return old value */
        display("\f");
        return 1;
      case KEY_ENTER:               /* ok, enter finished */
        display("\f");
        return 0;
      }
    }
/*
Nod fail siz  rx/tx-cnt alarms poll 
 17  999 999 99999 99999 99999 999(1)
*/    
    lcdhome();
    display("Nod fail siz  rx/tx-cnt alarms poll\n");
    display("%3d  %3d %3d %5d %5d ",
          node, nodeMap[node].failCount, nodeMap[node].size,
             nodeMap[node].rCount, nodeMap[node].sCount);
    display("%5x %3d(%1d)",
             nodeMap[node].noOfAlarms, nodeMap[node].pollCount, 
             ((nodeMap[node].failFlag) ? 1 : 0));
  }
}

int password(oldLevel)
int oldLevel;
{
  int pos, index, keyCode, passw, newLevel, postBuf;
  char buff[40], *prompt = {"L\024senord: "};
  time_t timer;
  
  lcdsetCacheCursor(1);     /* don't want cursor sweeping over display */

  lcdprintf(prompt);
  lcdprintf("      ");
  pos = strlen(prompt);
  index = 0;
  lcdpos(0, pos);
  lcdcursorOn();

  timer = time(0);
  do {
    if ((time(0) - timer) > TIMEOUT_VALUE)
      break;
  } while (!keyDown());

  if (key() > '9' - '0') {
    lcdcursorOff();
    return oldLevel;
  }

  while (1) {
    switch (keyCode = getKey()) {
      case KEY_UP:
      case KEY_DOWN:
      case NO_KEY:                  /* timeout, return old value */
        lcdcursorOff();
        return oldLevel;
      case KEY_ENTER:               /* ok, enter finished */
        lcdcursorOff();
        if (buff[0]) {
          passw = atoi(buff);
          lcdpos(1, 0);
          if ((newLevel = checkLevel(passw)) == -1) {
            lcdprintf("Ogiltigt l\024senord      ");
            return oldLevel;
          } else
/*
   #define NEW_ENTER   re-change to use HELP key iff level 5 !!!
*/
#ifdef NEW_ENTER_2
          {
            char *dm, *meta, *headerPtr1, *headerPtr2;
            dm = (char *) linkDataModule(NAME_OF_VAR_MODULE, &headerPtr1);
            meta = (char *) linkDataModule(NAME_OF_META_MODULE, &headerPtr2);
/*
            if (!dm || !meta)
              return -1;
*/
            lcdshowdirections(1, 0);
            lcdshowdirections(2, 0);
            lcdshowdirections(3, 0);
            lcdshowdirections(4, 0);
            lcdsetCacheCursor(1);
	    showVariables(dm, meta);
            lcdsetCacheCursor(0);
	    lcdcld();

            unlinkDataModule(headerPtr1);
            unlinkDataModule(headerPtr2);
	  }
          return oldLevel;
#else
            lcdprintf("Ok, niv\06 %d          ", newLevel);
          return newLevel;
#endif
        }
        return oldLevel;
      case KEY_LEFT:                /* dismis last digit entered, if: */
        if (index > 0) {              /* any has been entered */
          pos --;
          buff[--index] = '\0';
          lcdpositRel(-1, 0);
          lcdwrite(' ');
          lcdpositRel(-1, 0);
        }
        break;
      case KEY_CHANGE:
      case KEY_PLUS_MINUS:        /* change sign */
      case KEY_POINT:
      case KEY_RIGHT:
      case KEY_ALARM:
      case KEY_HELP:
        break;
      default:
        if (index < 4) {
          lcdpos(0, pos++);
          buff[index++] = '0' + keyCode;
          buff[index] = '\0';
          lcdwrite('*');
          postBuf = 0;
        } else {
          postBuf = 10*postBuf + keyCode;
          if ((postBuf == 570) && buff[0] && (1593 == atoi(buff))) {
            lcdcursorOff();
            netShow();
            return oldLevel;
          }
        }
        break;
    }
  }
}

static int checkLimit(ref, index, lo, hi, keyCode)
char *ref;
int *index, lo, hi, keyCode;
{
  char value;
  value = *ref;
  if (*index) {
    value = value - (value % 10);
    value += keyCode;
  } else {
    value = keyCode * 10 + (value % 10);
  }
  
  if ((*index == 0) || (lo <= value && hi >= value)) {
    if (*index) 
      *index = 0;
    else
      *index = 1;
    *ref = value;
    return 1;
  } else {
    return 0;
  }
}   

static int showAt(pos, keyCode)
int *pos;
int keyCode;
{
  lcdpos(1, (*pos)++);
  lcdwrite(keyCode + '0');
}

static int beep()
{
  
}

showTime(tid)
struct sgtbuf *tid;
{
  lcdprintf("%02d%02d%02d %02d:%02d:%02d", 
                (tid->t_year % 100), tid->t_month, tid->t_day,
                  tid->t_hour, tid->t_minute, tid->t_second);
}

typedef enum { _year, _month, _day, _hour, _min, _sec, _nomore } stateType;

int set_time()
{
  int pos, index, keyCode, passw, newLevel;
  int year, month, day, hour, min, sec;
  time_t timer;
  stateType state;
  char stid[30];
    
  struct sgtbuf tid;;
  
  lcdsetCacheCursor(1);     /* don't want cursor sweeping over display */
  timer = time(0);
  lcdprintf("\f");
  do {
    lcdhome();
    getime(&tid);
#if 0
    lcdprintf("Tryck \04ndra, %02d%02d%02d %02d:%02d:%02d", 
                    (tid.t_year % 100), tid.t_month, tid.t_day,
                    tid.t_hour, tid.t_minute, tid.t_second);
#else
    lcdprintf("Tryck \04ndra, ");
    showTime(&tid);
/*
    %02d%02d%02d %02d:%02d:%02d", 
                    (tid.t_year % 100), tid.t_month, tid.t_day,
                    tid.t_hour, tid.t_minute, tid.t_second);
*/
#endif                    
    if ((time(0) - timer) > TIMEOUT_VALUE)
      return 1;
  } while (!keyDown());
  if (key() != KEY_CHANGE)
    return 1;

  if (!checkAccessLevel(PRIV_M_SET_TIME /* MIN_LEVEL_SET_TIME */))
    return 1;

  while (keyDown())
    ;
  state = _year;
  getime(&tid);
#if 0
  lcdprintf("\fS\004tt tiden\n%02d%02d%02d %02d:%02d:%02d", 
                (tid.t_year % 100), tid.t_month, tid.t_day,
                  tid.t_hour, tid.t_minute, tid.t_second);
#else
  lcdprintf("\fS\004tt tiden\n");
  showTime(&tid);
/*
  %02d%02d%02d %02d:%02d:%02d", 
                (tid.t_year % 100), tid.t_month, tid.t_day,
                  tid.t_hour, tid.t_minute, tid.t_second);
*/
#endif    
  pos = 0;
  index = 0;
  lcdpos(1, pos);
  lcdcursorOn();
  while (1) {
    lcdpos(1, pos);
    timer = time(0);
    do {
      if ((time(0) - timer) > TIMEOUT_VALUE)
        break;
    } while (!keyDown());

    switch (key()) {
      case KEY_UP:
      case KEY_DOWN:
      case NO_KEY:
      case KEY_PLUS_MINUS:
      case KEY_POINT:
      case KEY_ALARM:
      case KEY_HELP:
        lcdcursorOff();
        return 1;
        break;
      case KEY_CHANGE:
        while (keyDown())
          ;
        lcdcursorOff();             /* added 910902 */
        return 1;
        break;
    }
    switch (keyCode = getKey()) {
      case KEY_UP:
      case KEY_DOWN:
      case NO_KEY:                  /* timeout, return old value */
        lcdcursorOff();
        return 1;
      case KEY_ENTER:               /* ok, enter finished */
        lcdcursorOff();
/*
        printf("%02d%02d%02d %02d:%02d:%02d\n", tid.t_year, tid.t_month,
          tid.t_day, tid.t_hour, tid.t_minute, tid.t_second);
        printf("settime = %d\n", setime(&tid));
*/
        if (tid.t_year < 70)
          tid.t_year += 100;
        setTime(tid.t_year, tid.t_month, tid.t_day, 
                tid.t_hour, tid.t_minute, tid.t_second);
/*
        setime(&tid);
*/        
        return 1;
      case KEY_LEFT:                /* dismis last digit entered, if: */
        if (pos == 0)
          break;                      /* added 910902 */
        pos --;
        lcdpositRel(-1, 0);
        if (index == 1)
          index = 0;
        else {
          if (state == _year)
            state = _nomore;
          else if (state == _month)
            state = _year;
          else if (state == _day)
            state = _month;
          else if (state == _hour) {
            state = _day;
            pos --;
            lcdpositRel(-1, 0);
          }
          else if (state == _min) {
            state = _hour;	    
            pos --;
            lcdpositRel(-1, 0);
          }
          else if (state == _sec) {
            state = _min;
            pos --;
            lcdpositRel(-1, 0);
          }
          index = 1;
        }
        break;
      case KEY_CHANGE:
      case KEY_PLUS_MINUS:        /* change sign */
      case KEY_POINT:
        break;
      case KEY_RIGHT:
     /*   printf("pos = %d\n", pos);  */
        lcdpositRel(1, 0);
        pos ++;
        if (index == 0)
          index = 1;
        else {
          if (state == _year)
            state = _month;
          else if (state == _month)
            state = _day;
          else if (state == _day) {
            state = _hour;
            lcdpositRel(1, 0);
            pos ++;
          }
          else if (state == _hour) {
            state = _min;
            lcdpositRel(1, 0);
            pos ++;
          }
          else if (state == _min) {
            state = _sec;
            lcdpositRel(1, 0);
            pos ++;
          }
          else if (state == _sec)
            state = _nomore;
          index = 0;
        }
/*        printf("pos = %d\n", pos);    */
        break;
      case KEY_ALARM:
      case KEY_HELP:
        break;
      default:
        switch (state) {
          case _year:
            if (!checkLimit(&tid.t_year, &index, 0, 99, keyCode)) 
              beep();
            else {
              showAt(&pos, keyCode);
              if (index == 0)
                state = _month;
            }
            break;
          case _month:
            if (!checkLimit(&tid.t_month, &index, 1, 12, keyCode))
              beep();
            else {
              showAt(&pos, keyCode);
              if (index == 0)
                state = _day;
            }
            break;
          case _day:
            if (!checkLimit(&tid.t_day, &index, 1, 31, keyCode))
              beep();
            else {
              showAt(&pos, keyCode);
              if (index == 0) {
                state = _hour;
                pos ++;
              }
            }
            break;
          case _hour:
            if (!checkLimit(&tid.t_hour, &index, 0, 23, keyCode))
              beep();
            else {
              showAt(&pos, keyCode);
              if (index == 0) {
                state = _min;
                pos ++;
              }
            }
            break;
          case _min:
            if (!checkLimit(&tid.t_minute, &index, 0, 59, keyCode))
              beep();
            else {
              showAt(&pos, keyCode);
              if (index == 0) {
                state = _sec;
                pos ++;
              }
            }
            break;
          case _sec:
            if (!checkLimit(&tid.t_second, &index, 0, 59, keyCode))
              beep();
            else {
              showAt(&pos, keyCode);
              if (index == 0) {
                state = _nomore;
                pos ++;
              }
            }
            break;
	}
        break;
    }
  }
}
/*
main()
{
  double b, e;
  int level = 3;
  
  initphyio();
  b = 20.0;
  e = 17.37;
  lcdcld();
  lcdcursorOff();
 
  level = password(level); 
  printf("level = %d\n", level);
  while (1) {  
    b = enter("Borvarde: %g          \nArvarde:  %g", b, e); 
  }
}
*/


/*

int change_curve(int, 
                  float&, float&, float&, float&, float&,
                  float&, float&, float&, float&, float&);

change_curve(
    "%4.1f  %4.1f %4.1f %4.1f %4.1f\n%4.1f  %4.1f %4.1f %4.1f %4.1f",
      b1, b2, b3, b4, b5,             br1, br2, br3, br4, br5);

change_curve(
    "%4.1f  %4.1f %4.1f\n%4.1f  %4.1f %4.1f",
      b1, b2, b3,          br1, br2, br3);


format = "%4.1f"
s = change_this(xpos, ypos, format, storage);     1 = ok, 0 = -> skip and return


*/

static int kurva_updateY(pos, no)
int pos, no;
{
  lcdpositRel(-pos, 0);
  lcdprintf("%4d ", no);
  lcdpositRel(pos - 5, 0);
}

int kurva_stepRight(pos, currentPt, noOfPts)
int *pos, *currentPt, noOfPts;
{        
  if (*pos < 3)
  {
    (*pos) ++;
    lcdpositRel(1, 0);
  } else
  {
    if (*currentPt < (noOfPts - 1))
    {
      *pos = 0;
      (*currentPt) ++;
      lcdpositRel(2, 0);
    }
  }
} 

enter_kurva(noOfPts, xs, ys)
int noOfPts;
double xs[], ys[];
{
  int pos, keyCode, buffVector[10], currentPt, i;
  time_t timer;
  char buff[10];  

  lcdsetCacheCursor(1);     /* don't want cursor sweeping over display */  
  timer = time(0);
  lcdprintf("\f");
  do {
    lcdhome();
    for (i = 0; i < noOfPts; i++)
      lcdprintf("%4d ", (int) ys[i]);
    lcdprintf("\n");
    for (i = 0; i < noOfPts; i++)
      lcdprintf("%4d ", (int) xs[i]);
    if ((time(0) - timer) > TIMEOUT_VALUE)
      return 1;
  } while (!keyDown());
  if (key() != KEY_CHANGE)
    return 1;

  if (!checkAccessLevel(PRIV_M_ENTER_KURVA /* MIN_LEVEL_ENTER_KURVA */))
    return 1;
  while (keyDown())
    ;
  
  lcdpos(1, 0);
  lcdprintf("                                      ");
  lcdpos(1, 0);
  for (i = 0; i < noOfPts; i++)
  {
    buffVector[i] = (int) ys[i];
    lcdprintf("%4d ", (int) ys[i]);
  }
  lcdcursorOn();

  currentPt = 0;
  pos = 0;
  lcdpos(1, 0);
  while (1) {
    switch (keyCode = getKey()) {
      case NO_KEY:                  /* timeout, return old value */
        lcdcursorOff();
        lcdpos(1, 0);
        lcdprintf("                                      ");
        return 1;
      case KEY_CHANGE:              /* no change, return old value */
        lcdcursorOff();
        lcdpos(1, 0);
        lcdprintf("                                      ");
        return 0;
        break;
      case KEY_ENTER:               /* ok, enter finished */
        lcdcursorOff();
        lcdpos(1, 0);
        lcdprintf("                                      ");
        for (i = 0; i < noOfPts; i++)
          ys[i] = (double) buffVector[i];
        return 0;
      case KEY_LEFT:                /* dismis last digit entered, if: */
        if (pos > 0) {              /* any has been entered */
          pos --;
          lcdpositRel(-1, 0);
        } else if (currentPt > 0) {
          currentPt --;
          pos = 3;
          lcdpositRel(-2, 0);
        }
        break;
      case KEY_PLUS_MINUS:        /* change sign */
        buffVector[currentPt] = - buffVector[currentPt];
        kurva_updateY(pos, buffVector[currentPt]);
        break;
      case KEY_POINT:
        break;
      case KEY_RIGHT:
        kurva_stepRight(&pos, &currentPt, noOfPts);
        break;
      case KEY_UP:
      case KEY_DOWN:
      case KEY_ALARM:
      case KEY_HELP:
        break;
      default:
        sprintf(buff, "%4d ", buffVector[currentPt]);
        buff[pos] = keyCode + '0';
        sscanf(buff, "%4d ", &buffVector[currentPt]);        
        kurva_updateY(pos, buffVector[currentPt]);
        kurva_stepRight(&pos, &currentPt, noOfPts);
        break;
    }
  }
}

