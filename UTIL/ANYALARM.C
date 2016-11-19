/* visionCom.c */
/*
!   anyAlarms - a generic routine for retrieving number of pending alarms
*/
int anyAlarms(pcid)
int pcid;
{
  int cnt, count, i, m, pc, mask, as, ns, cs, ds, es = 0;
  struct _alarmEntry *entry;
  
  pc = (1 << pcid);
  m = mask = 0;
  if (abcdMask[0] & pc) { m |= 1;  mask |= 0x00c0; }
  if (abcdMask[1] & pc) { m |= 2;  mask |= 0x0030; }
  if (abcdMask[2] & pc) { m |= 4;  mask |= 0x000c; }
  if (abcdMask[3] & pc) { m |= 8;  mask |= 0x0003; }

  cnt = aldm->alarmListPtr;
  count = 0;
  entry = &aldm->alarmList[0];
  for (i = 0; i < cnt; i++) {
#ifdef _V920914_1_40
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
#else
    as = entry->sendAssert ^ entry->assertSent & entry->sendAssert;
    ns = entry->sendNegate ^ entry->negateSent & entry->sendNegate;
    cs = entry->sendConfirm ^ entry->confirmSent & entry->sendConfirm;
    ds = entry->sendDisable ^ entry->disableSent & entry->sendDisable;
#endif
/*    
    if ((aldm->alarmList[i].sendAssert && !aldm->alarmList[i].assertSent) ||
          (aldm->alarmList[i].sendNegate && !aldm->alarmList[i].negateSent) ||
          (aldm->alarmList[i].sendConfirm && !aldm->alarmList[i].confirmSent) ||
          (aldm->alarmList[i].sendDisable && !aldm->alarmList[i].disableSent))
*/
          
    if (pc & (as | ns | cs | ds | es))
    {
      if (m & (1 << aldm->alarmList[i].class))
        count++;
    }
    entry++;              /* added 920306 .... (what a bug !) */
  }
  count += getAlarmsForAllNodes(mask, pc);
  return count;
}


/* prCom.c */
/*
!   anyAlarms - a generic routine for retrieving number of pending alarms
*/
int anyAlarms(pcid)
int pcid;
{
  int cnt, count, i, m, pc, mask, as, ns, cs, ds, es;
  struct _alarmEntry *entry;
  
  pc = (1 << pcid);
  m = mask = 0;
  if (abcdMask[0] & pc) { m |= 1;  mask |= 0x00c0; }
  if (abcdMask[1] & pc) { m |= 2;  mask |= 0x0030; }
  if (abcdMask[2] & pc) { m |= 4;  mask |= 0x000c; }
  if (abcdMask[3] & pc) { m |= 8;  mask |= 0x0003; }

  cnt = aldm->alarmListPtr;
  count = 0;
  entry = &aldm->alarmList[0];
  for (i = 0; i < cnt; i++) {
#ifdef _V920914_1_40
    if (!(entry->sendStatus & ALARM_SEND_INIT))
      as = 0xff;      /* if not init, let's assume everybody needs it */
    else {
      as = (entry->sendStatus & ALARM_SEND_ASSERT) ? 
        entry->sendMask ^ entry->assertSent & entry->sendMask : 0;
    }
    ns = (entry->sendStatus & ALARM_SEND_NEGATE) ? 
      entry->sendMask ^ entry->negateSent & entry->sendMask : 0;
    cs = (entry->sendStatus & ALARM_SEND_CONFIRM) ? 
      entry->sendMask ^ entry->confirmSent & entry->sendMask : 0;
    ds = (entry->sendStatus & ALARM_SEND_DISABLE) ? 
      entry->sendMask ^ entry->disableSent & entry->sendMask : 0;
    es = (entry->sendStatus & ALARM_SEND_ENABLE) ?  /* added 920921 */
      entry->sendMask ^ entry->enableSent & entry->sendMask : 0;
#else
    as = entry->sendAssert ^ entry->assertSent & entry->sendAssert;
    ns = entry->sendNegate ^ entry->negateSent & entry->sendNegate;
    cs = entry->sendConfirm ^ entry->confirmSent & entry->sendConfirm;
    ds = entry->sendDisable ^ entry->disableSent & entry->sendDisable;
#endif
    if (pc & (as | ns | cs | ds | es))
    {
      if (m & (1 << aldm->alarmList[i].class))
        count++;
    }
    entry++;              /* added 920306 .... (what a bug !) */
  }
  count += getAlarmsForAllNodes(mask, pc);
  return count;
}

/* slave.c */
/*
!   anyAlarms - a generic routine for retrieving number of pending alarms
!   returns an 32bit integer unionfied with no of A,B,C and D alarms as 8bits
*/
int anyAlarms(pcSum, activeABCD, confirmedABCD)
int *pcSum;
unsigned char *activeABCD, *confirmedABCD;
{
  int cnt, count, i, c[4], as, ns, cs, ds, es = 0, class;
  union { char types[4]; int ret; } u;
  struct _alarmEntry *entry;
  
  cnt = aldm->alarmListPtr;
  count = (*pcSum) = 0;
  c[0] = c[1] = c[2] = c[3] = 0;
  entry = &aldm->alarmList[0];
  for (i = 0; i < cnt; i++) {
#ifdef _V920914_1_40
    if (!(entry->sendStatus & ALARM_SEND_INIT))
      as = 0xff;      /* if not init, let's assume everybody needs it */
    else {
      as = (entry->sendStatus & ALARM_SEND_ASSERT) ? 
        entry->sendMask ^ entry->assertSent & entry->sendMask : 0;
    }
    ns = (entry->sendStatus & ALARM_SEND_NEGATE) ? 
      entry->sendMask ^ entry->negateSent & entry->sendMask : 0;
    cs = (entry->sendStatus & ALARM_SEND_CONFIRM) ? 
      entry->sendMask ^ entry->confirmSent & entry->sendMask : 0;
    ds = (entry->sendStatus & ALARM_SEND_DISABLE) ? 
      entry->sendMask ^ entry->disableSent & entry->sendMask : 0;
    es = (entry->sendStatus & ALARM_SEND_ENABLE) ?  /* added 920921 */
      entry->sendMask ^ entry->enableSent & entry->sendMask : 0;
#else
    as = entry->sendAssert ^ entry->assertSent & entry->sendAssert;
    ns = entry->sendNegate ^ entry->negateSent & entry->sendNegate;
    cs = entry->sendConfirm ^ entry->confirmSent & entry->sendConfirm;
    ds = entry->sendDisable ^ entry->disableSent & entry->sendDisable;
#endif
    class = entry->class & 0x03;
    if (!entry->confirm) {                /* new 920211 */
      if (activeABCD)
        activeABCD[class] ++;
    } else if (entry->active) {
      if (confirmedABCD)
        confirmedABCD[class] ++;
    }
    *pcSum = (*pcSum | as | ns | cs | ds | es);      /* new 920210 */
    if (as || ns || cs || ds || es) {
      c[class] ++; 
      count++;
    }
    entry++;
  }
  u.types[0] = (c[0] > 255) ? 255 : c[0];
  u.types[1] = (c[1] > 255) ? 255 : c[1];
  u.types[2] = (c[2] > 255) ? 255 : c[2];
  u.types[3] = (c[3] > 255) ? 255 : c[3];
  return u.ret;
}
/*
!   Old approach:
!         the alarm mask is updated regulary
!
!
!   New approach:
!         the alarm mask is set once for all when the alarm is asserted
!
*/
int setAlarmMask(mask)
unsigned char *mask;
{
  int cnt, i;
  struct _alarmEntry *entry;
  
  cnt = aldm->alarmListPtr;
  entry = &aldm->alarmList[0];
  for (i = 0; i < cnt; i++) {
#ifdef _V920914_1_40
    if (!(entry->sendStatus & ALARM_SEND_INIT)) {
      entry->sendStatus |= ALARM_SEND_INIT;
      entry->sendMask = mask[entry->class & 0x03];
    }
#else
#if 1                           /* new 920212 */
    if (entry->sendAssert == 0xff)
      entry->sendAssert = mask[entry->class & 0x03];
    if (entry->sendNegate == 0xff)
      entry->sendNegate = mask[entry->class & 0x03];
    if (entry->sendConfirm == 0xff)
      entry->sendConfirm = mask[entry->class & 0x03];
    if (entry->sendDisable == 0xff)
      entry->sendDisable = mask[entry->class & 0x03];
#else
    if (entry->sendAssert)
      entry->sendAssert = mask[entry->class & 0x03];
    if (entry->sendNegate)
      entry->sendNegate = mask[entry->class & 0x03];
    if (entry->sendConfirm)
      entry->sendConfirm = mask[entry->class & 0x03];
    if (entry->sendDisable)
      entry->sendDisable = mask[entry->class & 0x03];
#endif
#endif
    entry++;
  }        
}

