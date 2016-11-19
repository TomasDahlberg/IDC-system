/* doStat.c  1992-11-13 TD,  version 1.1 */
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
! Copyright (C) 1991, IVT Electronic AB.
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
*/
#include <stdio.h>
#include <time.h>
#include "ivtnet.h"
#include "stat.h"
#include "../../meta.h"

static struct _metaStatS *statS = METASTAT_ADDRESS;
static struct _statBuff *statPtr = SAVE_STAT_ADDRESS;
short int *saveIdx = SAVE_IDX_ADDRESS;
/*
short int *readIdx = READ_IDX_ADDRESS;
*/
short int *readIdx_PC0 = READ_IDX_ADDRESS_PC0;
short int *readIdx_PC1 = READ_IDX_ADDRESS_PC1;

                            /* 12bit 0..4096, save at statPtr[saveIdx] */

extern char *dm, *meta, *aldm;
extern int DEBUG;

initStat()
{
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

  
/*  if (DEBUG) printf("Entered sampleStat:\n");   */
  now = time(0);          /* changed from 'getRelTime(0);' 921113 !! */
  metaPtr = &statS[0];
  for (i = 0; i < BUFF_MAX; i++, metaPtr++) {
/*    if (DEBUG) printf("  i = %d\n", i);
    if (DEBUG) {
      printf("statS = %d\n", statS);
      printf("metaPtr = %d\n", metaPtr);
      printf("metaPtr->name = %d\n", metaPtr->name);
    }
*/    
    if (metaPtr->name[0] == 0)
      continue;
/*    if (DEBUG) printf("metaPtr->name = '%s'\n", metaPtr->name);   */
    metaItm = &metaPtr->rows[0];
/*    if (DEBUG) printf("metaItm = %d\n", metaItm);     */
    for (itm = 0; itm < ITEM_MAX; itm++, metaItm++) {
/*      if (DEBUG) printf("  itm = %d\n", itm);         */
      if (metaItm->var[0] == 0)
        continue;

      if (metaItm->xxx && (abs(now - metaItm->xxx) < WAIT_FOR_VAR_DELAY))
        continue;

      metaItm->xxx = now;

      if (abs(now - metaItm->sample) >= metaItm->intervall) { /* abs() 920410 */
        metaItm->sample = setNextSample(now, metaItm->intervall); /* 920410 */
/*
        metaItm->sample = now;
*/
        if (toNode = metaItm->duc) {
/*          if (DEBUG) printf("  ok get stat remote\n");    */
          
          message->message_type = GET_STAT_VAR | REQUEST_MASK;
          message->error = NET_NOERROR;
          message->mix.stat.buffS = i;
          message->mix.stat.item = itm;
          size = 5 + strlen(strcpy(message->mix.stat.varName, metaItm->var));
/*
        metaItm->sample = now - ((now - metaItm->sample) % metaItm->intervall);
*/
          sendPacket(toNode, message, size);
        } else {
          float value;
/*          if (DEBUG) printf("  ok get stat local\n");   */
          if (getOurVar(metaItm->var, &value))
            recStat(i, itm, value);
          else {
/*            if (DEBUG) printf("  failed to get stat local!!!\n"); */
          }
        }
      }
    }
  }
/*  if (DEBUG) printf("exit sampleStat\n"); */
}

recStat(buffS, item, value)
int buffS, item;
float value;
{
  statS[buff].rows[itm].xxx = 0;
  
 /* obs setstat, visionCom must reset xxx when allocating new item !!! */

/*  if (DEBUG) printf("  entered recStat\n"); */
  *saveIdx &= 0x0fff;
  statPtr[*saveIdx].bufItm = buffS;
  statPtr[*saveIdx].Itm = item;
  statPtr[*saveIdx].sampleTime = time(0);
  statPtr[*saveIdx].value = value;
  (*saveIdx)++;
  if ((*saveIdx == *readIdx_PC0) || (*saveIdx == *readIdx_PC1))
    (*saveIdx)--;
  if (*saveIdx > 0x1000)
    *saveIdx = 0;
/*  if (DEBUG) printf("  exit recStat\n");    */
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
