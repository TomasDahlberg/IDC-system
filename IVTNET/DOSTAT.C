/* doStat.c  1993-10-25 TD,  version 1.2 */
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
! doStat.c
! Copyright (C) 1991-1993 IVT Electronic AB.
*/

/*
!     doStat.c is part of the IVTnet product
!
!     File: doStat.c
!     
!     Collects statistical data
!
!     History     
!     Date        Revision Who  What
!     
!      ??-??-1992     1.0  TD   Start of coding
!     13-nov-1992     1.1  TD   Months data are handled the last day of the m.
!                               month. Changed changed from 'getRelTime(0);'
!                               to time(0);
!     25-oct-1993     1.2  TD   Changed metaStat structure; added tries and
!                               noOfBuffs/noOfItemsPerBuff and moved to data
!                               module. The try fields is incremented and if 
!                               no reply within a minute another request will
!                               be issued etc for 10 minutes. 
*/
#include <stdio.h>
#include <time.h>
#include <module.h>
#include "ivtnet.h"

/*  #define NEW_STAT_AS_OF_931021   */

#include "stat.h"
#ifndef DOS
#include "../../meta.h"
#else
#include "meta.h"
#endif

#ifndef NEW_STAT_AS_OF_931021
char tries[BUFF_MAX][ITEM_MAX];
#endif

static struct _metaStatModule *metaStatModule;
static struct _metaStatS *statS; 	/* = METASTAT_ADDRESS;	*/
static struct _statBuff *statPtr; 	/* = SAVE_STAT_ADDRESS;	*/
short int *saveIdx = SAVE_IDX_ADDRESS;
/*
short int *readIdx = READ_IDX_ADDRESS;
*/
short int *readIdx_PC0 = READ_IDX_ADDRESS_PC0;
short int *readIdx_PC1 = READ_IDX_ADDRESS_PC1;

                            /* 12bit 0..4096, save at statPtr[saveIdx] */

extern char *dm, *meta, *aldm;
extern int DEBUG;

struct _metaStatS *statBufferPtr(metaStatModule, buf)
struct _metaStatModule *metaStatModule;
int buf;
{
#ifdef NEW_STAT_AS_OF_931021
  int siz, items, sizeOfOneBuffer;
  char *start;
  if (buf > metaStatModule->noOfBuffs)
    return 0;
  items = metaStatModule->noOfItemsPerBuff;
  start = metaStatModule->metaStatBuff;

  sizeOfOneBuffer = sizeof(struct _metaStatS) +
                                 sizeof(struct _metaStatBuff) * (items - 1);
  start += sizeOfOneBuffer * buf;
  return (struct _metaStatS *) start;
#else
  return &statS[buf];
#endif
}

initStat()
{
#ifndef NEW_STAT_AS_OF_931021
  int i, j;
  for (i = 0; i < BUFF_MAX; i++)
    for (j = 0; j < ITEM_MAX; j++)
        tries[i][j] = 0;

  statS = METASTAT_ADDRESS;
  statPtr = SAVE_STAT_ADDRESS;
#else
  if ((metaStatModule = 
	(struct _metaStatModule *) modlink("METASTAT", (short) 0 /* any */)) 
            == (struct _metaStatModule *) -1) {
    statS = 0;
    statPtr = 0;
    metaStatModule = 0;
  } else {
    metaStatModule = (struct _metaStatModule *) 
			(((char *) metaStatModule) + sizeof(struct modhcom));
    statS = (struct _metaStatS *) metaStatModule->metaStatBuff;
    statPtr = SAVE_STAT_ADDRESS;
  }
#endif
}

float Float(dd)
double dd;
{
  if (dd > 1e+37)
    return 1e+37;
  if (dd < -1e+37)
    return -1e+37;
  if (0 > dd && dd > -1e-37)
    return 0;
  if (1e-37 > dd && dd > 0)
    return 0;
  return dd;
}

/*
!   Called:
!
!        metaItm->sample = setNextSample(now, metaItm->intervall);
*/
time_t setNextSample(now, inter)      /* added 920410 */
time_t now;
long inter;
{
  struct tm t1;
  time_t c_time, this;
/*
    old code just returns now;
*/
  if (inter < 3600)
    return now;           /*      metaItm->sample = now;    */

  c_time = time(0);  
  memcpy(&t1, localtime(&c_time), sizeof(struct tm));
  if (inter >= 86400) {
    t1.tm_hour = 23;                    /* 0 */
    t1.tm_min = 57;
  } else
    t1.tm_min = 58;                       /* 0 */
  t1.tm_sec = 0;
  this = mktime(&t1);
/*
this = this + 27dagar
this = justera till sista dagen i month
this = this - intervall
*/
  if (inter == 2678400) {
    this = this + 27*86400;
    memcpy(&t1, localtime(&this), sizeof(struct tm));
    if (t1.tm_mon == 1)       /* feb */
      t1.tm_mday = 28 + (t1.tm_year == 100 /* year 2000 leap or not (?) */ ? 1 :
           ((t1.tm_year % 4) == 0 ? 1 : 0));
    else 
      t1.tm_mday = 31 - ((t1.tm_mon < 7) ? 
                         (t1.tm_mon & 1) :               /* 1 if odd */
                         (1 - (t1.tm_mon & 1)));         /* 1 if even */
    this = mktime(&t1);
    this = this - inter;
  }
  return now - (c_time - this);
}

sampleStat(message)
struct _message *message;
{
  int i, itm, toNode, size;
  struct _metaStatS *metaPtr;
  struct _metaStatBuff *metaItm;
  time_t now;

#ifdef NEW_STAT_AS_OF_931021
  int BUFF_MAX, ITEM_MAX;

  if (metaStatModule == 0)
    return ;

  BUFF_MAX = metaStatModule->noOfBuffs;			/* used to be 8 */
  ITEM_MAX = metaStatModule->noOfItemsPerBuff;		/* used to be 30 */
#endif
  
  now = time(0);          /* changed from 'getRelTime(0);' 921113 !! */
  for (i = 0; i < BUFF_MAX; i++) {
    metaPtr = statBufferPtr(metaStatModule, i);
    if (metaPtr->name[0] == 0)
      continue;
    metaItm = &metaPtr->rows[0];
    for (itm = 0; itm < ITEM_MAX; itm++, metaItm++) {
      
      if (metaItm->var[0] == 0)
        continue;

#ifdef NEW_STAT_AS_OF_931021
      if (metaItm->tries > 10)
        metaItm->tries = 0;
      if (metaItm->tries) {
        if (abs(now - metaItm->sample) >= 60*metaItm->tries) /* overdue X min */
	  ;
	else
	  continue;
      }

      if (metaItm->tries || 
		(abs(now - metaItm->sample) >= metaItm->intervall)) { /* abs() 920410 */
	if (metaItm->tries == 0)
          metaItm->sample = setNextSample(now, metaItm->intervall); /* 920410 */

	metaItm->tries ++;
	
#else
      if (tries[i][itm] > 10)
        tries[i][itm] = 0;
      if (tries[i][itm]) {
        if (abs(now - metaItm->sample) >= 60*tries[i][itm]) /* overdue X min */
	  ;
	else
	  continue;
      }

      if (tries[i][itm] || 
            (abs(now - metaItm->sample) >= metaItm->intervall)) { /* abs() 920410 */
        if (tries[i][itm] == 0)
          metaItm->sample = setNextSample(now, metaItm->intervall); /* 920410 */

        tries[i][itm] ++;
#endif

      if (DEBUG) {
        printf("Tid nu: %s", ctime(&now));
        printf("Tries = %d, var = %s, time = %s\n", 
#ifdef NEW_STAT_AS_OF_931021
            metaItm->tries,
#else
            tries[i][itm],
#endif
            metaItm->var, ctime(&metaItm->sample));
      }

        if (toNode = metaItm->duc) {
          message->message_type = GET_STAT_VAR | REQUEST_MASK;
          message->error = NET_NOERROR;
          message->mix.stat.buffS = i;
          message->mix.stat.item = itm;
          size = 5 + strlen(strcpy(message->mix.stat.varName, metaItm->var));
          sendPacket(toNode, message, size);
        } else {
          float value;
          if (getOurVar(metaItm->var, &value))
            recStat(i, itm, value);
          else {
		;
          }
        }     /* end if not master node */
      }   /* end if now - sample > intervall */
    }   /* end for item */
  }   /* end for buffer */
}   /* end of sampleStat() */

recStat(buffS, item, value)
int buffS, item;
float value;
{
#ifdef NEW_STAT_AS_OF_931021
  struct _metaStatS *metaPtr;
  struct _metaStatBuff *metaItm;

  if (metaStatModule == 0)
    return ;
#endif

  *saveIdx &= 0x0fff;
  statPtr[*saveIdx].bufItm = buffS;
  statPtr[*saveIdx].Itm = item;

#ifdef NEW_STAT_AS_OF_931021
  metaPtr = statBufferPtr(metaStatModule, buffS);
  if (metaPtr) {
    metaItm = &metaPtr->rows[item];
    statPtr[*saveIdx].sampleTime = metaItm->sample;
/*  statBufferPtr(metaStatModule, buffS)->rows[item].sample;    */

    metaItm->tries = 0;
/*    statS[buffS].rows[item].tries = 0;    */

  } else
    statPtr[*saveIdx].sampleTime = time(0);
#else
  statPtr[*saveIdx].sampleTime = time(0);

/* 940419 changed from following to above */

/*  statPtr[*saveIdx].sampleTime = statS[buffS].rows[item].sample; */
  tries[buffS][item] = 0;
#endif
  statPtr[*saveIdx].value = value;
  (*saveIdx)++;
  if ((*saveIdx == *readIdx_PC0) || (*saveIdx == *readIdx_PC1))
    (*saveIdx)--;
  if (*saveIdx > 0x1000)
    *saveIdx = 0;

}


getOurVar(var, value)
char *var;
float *value;
{
  long ivalue;
  double dvalue;
  int id;
  
  if (bindModules(&dm, &meta, &aldm)) /* ok ! */
  {
    if ((id = metaId(meta, var)) < 0) 
      return 0;
    if (metaType(meta, id) == TYPE_INT) {
      memcpy(&ivalue, metaValue(dm, meta, id), sizeof(long));
      *value = ivalue;
    } else if (metaType(meta, id) == TYPE_FLOAT) {
      memcpy(&dvalue, metaValue(dm, meta, id), sizeof(double));
      *value = Float(dvalue);
    } else 
      return 0;
    return 1;
  }
  return 0;
}
