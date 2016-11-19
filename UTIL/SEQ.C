/*
!   for one alarm entry, checks what alarm to send
!   0 - no alarm pending for this entry
!   1 - assert state pending
!   2 - negate state pending
!   3 - confirm state pending
!   4 - disable state pending     (new 910517)
*/
getAlarmSequence(entry, PCno)
struct _alarmEntry *entry;
int PCno;
{
  int pc;
  int neg, conf, dis, en;
  pc = (1 << PCno);
  if (DEBUG) {
    printf("getAlarmSequence: pc = %d\n", pc);
  }
  if ((entry->sendStatus & ALARM_SEND_ASSERT) && 
      (entry->sendMask & pc) && !(entry->assertSent & pc))
    return 1;

  neg = ((entry->sendStatus & ALARM_SEND_NEGATE) && (entry->sendMask & pc) && 
          !(entry->negateSent & pc)) ? 1 : 0;
  conf = ((entry->sendStatus & ALARM_SEND_CONFIRM) && (entry->sendMask & pc) && 
          !(entry->confirmSent & pc)) ? 2 : 0;
  dis = ((entry->sendStatus & ALARM_SEND_DISABLE) && (entry->sendMask & pc) && 
            !(entry->disableSent & pc)) ? 4 : 0;
  en = ((entry->sendStatus & ALARM_SEND_ENABLE) && (entry->sendMask & pc) && 
            !(entry->enableSent & pc)) ? 8 : 0;

  switch (neg | conf | dis | en) {
    case 0:
      return 0; /*???*/
      break;
    case 1:
      return 2;	/* send negate event */
      break;
    case 2:
      return 3;	/* send confirm event */
      break;
    case 3:	/* negate & confirm */
      if (entry->confirmTime >= entry->offTime)         /* yes, who's first ?*/
        return 2;                                       /* inactive first */
      else
        return 3;
      break;
    case 4:
      return 4;	/* send disable event */
      break;
    case 5:	/* negate & disable */
      if (entry->disableTime >= entry->offTime)         /* yes, who's first ?*/
        return 2;                                       /* inactive first */
      else
        return 4;
      break;
    case 6:	/* confirm & disable */
      if (entry->disableTime <= entry->offTime)         /* yes, who's first ?*/
        return 4;                                       /* disable first */
      else
        return 2;
      break;
    case 7:	/* negate & confirm & disable */
      if (entry->disableTime >= entry->offTime) {
        if (entry->confirmTime >= entry->offTime)
	  return 2;
	else
	  return 3;
      } else {
        if (entry->confirmTime >= entry->disableTime)
	  return 4;
	else
	  return 3;
      }
      break;
    case 8:
      return 5;	/* send enable event */
      break;
    case 9:	/* negate & enable */
      if (entry->enableTime >= entry->offTime)         /* yes, who's first ?*/
        return 2;                                      /* inactive first */
      else
        return 5;
      break;
    case 10:	/* confirm & enable */
      if (entry->enableTime >= entry->confirmTime)      /* yes, who's first ?*/
        return 3;                                       /* confirm first */
      else
        return 5;
      break;
    case 11:	/* negate & confirm & enable */
      if (entry->enableTime >= entry->offTime) {
        if (entry->confirmTime >= entry->offTime)
	  return 2;
	else
	  return 3;
      } else {
        if (entry->confirmTime >= entry->enableTime)
	  return 5;
	else
	  return 3;
      }
      break;
    case 12:	/* enable & disable */
      if (entry->disableTime <= entry->enableTime)      /* yes, who's first ?*/
        return 4;                                       /* disable first */
      else
        return 5;
      break;
    case 13:	/* negate & disable & enable */
      if (entry->enableTime >= entry->offTime) {
        if (entry->disableTime >= entry->offTime)
	  return 2;
	else
	  return 4;
      } else {
        if (entry->disableTime >= entry->enableTime)
	  return 5;
	else
	  return 4;
      }
      break;
    case 14:	/* confirm & disable & enable */
      if (entry->enableTime >= entry->confirmTime) {
        if (entry->disableTime >= entry->confirmTime)
	  return 3;
	else
	  return 4;
      } else {
        if (entry->disableTime >= entry->enableTime)
	  return 5;
	else
	  return 4;
      }
      break;
    case 15:	/* negate & confirm & disable & enable */
      if (entry->disableTime >= entry->offTime) {
        if (entry->confirmTime >= entry->offTime) {
	  if (entry->enableTime >= entry->offTime)
	    return 2;
	  else
	    return 5;
	} else {
	  if (entry->enableTime >= entry->confirmTime)
	    return 3;
	  else
	    return 5;
        }
      } else {
        if (entry->confirmTime >= entry->disableTime) {
	  if (entry->enableTime >= entry->disableTime)
	    return 4;
	  else
	    return 5;
	} else {
	  if (entry->enableTime >= entry->confirmTime)
	    return 3;
	  else
	    return 5;
	}
      }
      break;
  }
}
















    switch (getAlarmSequence(&entry, PCno)) {
      case 1:      /* send assert event */
        *dtime = entry.initTime;
        *status = 1 | 0 /*cannot be disabled !*/ | ((entry.class & 0x0f) << 3);
        break;
      case 2:     /* send negate event */
        *dtime = entry.offTime;
        *status = ((entry.class & 0x0f) << 3);
        if (entry.confirmTime && (entry.offTime > entry.confirmTime))
          *status |= 2;
        if (entry.disableTime && (entry.offTime > entry.disableTime)) {
	  if (entry.enableTime && (entry.offTime > entry.enableTime))
	    ;
	  else
            *status |= 4;
	}
        break;
      case 3:       /* send confirm event */
        *dtime = entry.confirmTime;
        *status = 2 | ((entry.class & 0x0f) << 3);
        if (entry.offTime && (entry.confirmTime >= entry.offTime))
          ;   /* inactive */
        else 
          *status |= 1;   /* active */
        if (entry.disableTime && (entry.confirmTime > entry.disableTime))
          *status |= 4;
        break;
      case 4:       /* send disable event */
        *dtime = entry.disableTime;
        *status = 4 | ((entry.class & 0x0f) << 3);
        if (entry.offTime && (entry.disableTime >= entry.offTime))
          ;   /* inactive */
        else 
          *status |= 1;   /* active */
        if (entry.confirmTime && (entry.disableTime >= entry.confirmTime))
          *status |= 2;
        break;
      case 5:       /* send enable event */
        *dtime = entry.enableTime;
        *status = 0 | ((entry.class & 0x0f) << 3);
        if (entry.offTime && (entry.enableTime >= entry.offTime))
          ;   /* inactive */
        else 
          *status |= 1;   /* active */
        if (entry.confirmTime && (entry.enableTime >= entry.confirmTime))
          *status |= 2;
        break;
      default:
        fprintf(fpOut, "Program fault: alarmtext switch\n");
        break;
    }        
    alarmMarkPtr[PCno & 7] = internalCounter;
    convert(entry.string, text);
    internalCounter++;
    return 1;
  }
}
