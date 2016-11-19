#include <time.h>
#include <stdio.h>
#include <sgstat.h>

#define _V920914_1_40 /* globalAlarmMask system fix !! (see other modules !) */

#define NO_OF_ALARMS 1
#include "alarm.h"

extern int MASTER_NO;
extern int OUR_NODE;
extern int errno;

extern int PCno;

static int node, alarm, serie, status;

char *stat(s)
int s;
{
  static char buf[30];

  strcpy(buf, "inaktivt");
  if (s == 7)
    sprintf(&buf[8], "+kvitterat+blockerat");
  else if (s == 3 || s == 5 || s == 6)
    sprintf(&buf[8], "%s", s == 3 ? "+kvitterat" :
		(s == 5 ? "+blockerat" : "+kvitterat+blockerat"));
  else if (s == 2 || s == 4) 
    sprintf(buf, "%s", s == 2 ? "+kvitterat" : "+blockerat");

  return s & 1 ? &buf[2] : buf;
}

void cnvText(s)
unsigned char *s;
{
  int c;
  while (c = *s) {
    if (c == 0xf8)		/* degree sign */
	c = 96;
    else if (c == 0x86)		/* aa */
	c = '}';
    else if (c == 0x84)		/* ae */
	c = '{';
    else if (c == 0x94)		/* oe */
	c = '|';
    else if (c == 0x8f)		/* AA */
	c = ']';
    else if (c == 0x8e)		/* AE */
	c = '[';
    else if (c == 0x99)		/* OE */
	c = '\\';
    else if (c < 32)
        c = 'z';
    else if (c > 0x7e)
	c = 'z';
    *s++ = c;
  }
}

int makeAlarmText(msg, typ)
char *msg;
int typ;
{
  time_t dtime;
  char text[100];
  int class, masterNo = MASTER_NO;
  struct tm tid;

  if (doRequestAlarmText(&node, &alarm, &serie, &status, &dtime, text)) {
    cnvText(text);
    class = (status >> 3) & 0x0f;

    if (typ & 16) {
      if ((status & 7) != 0x01) {
        ackLastAlarmRead();
        return 0;
      }
    }

    memcpy(&tid, localtime(&dtime), sizeof(struct tm));
/*
	bbdca
		a = active 
		c = confirm
		d = enable/disable
		bb class
*/
/*
      sprintf(msg, "%s %02d%02d%02d %02d:%02d:%02d %c:%d.%d.%d.%d.%d",
    	      text, 
              (tid.tm_year > 100) ? tid.tm_year - 100 : tid.tm_year,
              tid.tm_mon + 1, tid.tm_mday, 
              tid.tm_hour, tid.tm_min, tid.tm_sec,
              class + 'A',
              masterNo, node, alarm, serie, status & 7);
*/
    if (typ & 16) 
      sprintf(msg, "%c: %s", 
              class + 'A',
    	      text);
    else
      sprintf(msg, "%c: %s %02d:%02d:%02d %s", 
              class + 'A',
	      stat(status & 7),
              tid.tm_hour, tid.tm_min, tid.tm_sec,
    	      text);
    return 1;
  } else {			/* error from Request alarm, no more */
    return 0;
  }
}

io_error(path)
int path;
{
  struct sgbuf buffer;
  
  if (_gs_opt(path, &buffer) == -1) {
    fprintf(stderr, "err io_opt: %d\n", errno);
    exit(errno);
  }
  fprintf(stderr, "I/O error %d\n", buffer._sgm._sgs._sgs_err);
  fprintf(stderr, "xon %d\n", buffer._sgm._sgs._sgs_xon);
  fprintf(stderr, "xoff %d\n", buffer._sgm._sgs._sgs_xoff);
  fprintf(stderr, "pause %d\n", buffer._sgm._sgs._sgs_pause);
  fprintf(stderr, "psch %d\n", buffer._sgm._sgs._sgs_psch);
  fprintf(stderr, "parity %d\n", buffer._sgm._sgs._sgs_parity);
}

extern unsigned char abcdMask[4];
extern struct _alarmModule *aldm;
extern int alarmMarkPtr[8];
extern int alarmMarkNode;

int ackLastAlarmRead()
{
/*  fprintf(stderr, "AckLast: %d.%d.%d.%d\n", node, alarm, serie, status); */


  if (alarmMarkNode != OUR_NODE) {
     netAckAlarm(alarmMarkNode, PCno, abcdMask);
  } else {
    setAlarmMask(abcdMask);
    switch (getAlarmSequence(&aldm->alarmList[alarmMarkPtr[PCno & 7]], PCno)) {
      case 1:   /* send assert */
        aldm->alarmList[alarmMarkPtr[PCno & 7]].assertSent |= (1 << PCno);
        break;
      case 2:
        aldm->alarmList[alarmMarkPtr[PCno & 7]].negateSent |= (1 << PCno);
        break;
      case 3:
        aldm->alarmList[alarmMarkPtr[PCno & 7]].confirmSent |= (1 << PCno);
        break;
      case 4:
        aldm->alarmList[alarmMarkPtr[PCno & 7]].disableSent |= (1 << PCno);
        if (aldm->alarmList[alarmMarkPtr[PCno & 7]].confirm == 1)
           aldm->alarmList[alarmMarkPtr[PCno & 7]].confirmSent |= (1 << PCno); 
                            /* !! autoconfirm, new 910909 */
          break;
      case 5:     /* added 920921 */
        aldm->alarmList[alarmMarkPtr[PCno & 7]].enableSent |= (1 << PCno);
        if (aldm->alarmList[alarmMarkPtr[PCno & 7]].confirm == 1)
          aldm->alarmList[alarmMarkPtr[PCno & 7]].confirmSent |= (1 << PCno); 
                            /* !! autoconfirm, new 910909 */
        break;
      default:
        ; /* fprintf(fpOut, "error, ???\n");	*/
    }
  }
}

