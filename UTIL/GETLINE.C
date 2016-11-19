/*
! getLine         - as readLine, but will shutdown/establish connection
*/
int getLine(commandLine)
char *commandLine;
{
  int alarm_id, intervall, timeout_intervall;
  static int firstTime;
  static long previousSuccessCall;
  static int tries, minimumWaitTime;
  
  timeout_intervall = getTimeOut_Value();  /* added 930830 */
  if (mode == connected) {
    tries = 0;
    minimumWaitTime = 0;
    previousSuccessCall = 0;
  }
  while (1) {
    if (mode == disconnected)
      PCno = -1;
    if (mode == disconnected)
      intervall = 5;                    /* check alarm every 5 sec */
    else if (mode == connected) {
                        /* disconnect after 120 (930830, was 10) sec */
      intervall = timeout_intervall ? timeout_intervall : 120; 
    } else if (mode == shutdown)
      intervall = 5;                    /* if no modem, no OK reply ! */
    else
      intervall = 60;                   /* during setup */
   
    alarm_id = timeoutSignal(2, intervall);
     
    timeoutFlag = 0;
    if (readLine(commandLine))
    {
      if (-1 == alm_delete(alarm_id)) {
        fprintf(stderr, "cannot delete alarm, error = %d\n", errno);
      }
      
#ifdef DEBUG_MODE
      if (DEBUG)
        fprintf(stderr, "%d: %s", mode, commandLine);
#endif
/*
! Strategi:
!
!   if we are trying to call PC and recieve
!
!         NO ANSWER or
!         NO CARRIER, then
!                         lets try one more time and if the same message,
!                         wait 30 min, then repeat 
!
!         BUSY, then
!                         lets try one more time and if the same message,
!                         wait 1 min, then repeat for ten times, then
!                         wait 10 min,
!
!         NO DIALTONE, then
! 
*/      
      if (!strncmp(commandLine, "CONNECT 2400", 12)) {
        previousSuccessCall = time(0);
        mode = connected;
/*        setSpeed(pathIn, 2400);   */
      } else if (!strncmp(commandLine, "ONNECT 2400", 11)) {
        previousSuccessCall = time(0);
        mode = connected;
/*        setSpeed(pathIn, 2400);   */
      } else if (!strncmp(commandLine, "CONNECT 1200", 12)) {
        previousSuccessCall = time(0);
        mode = connected;
/*        setSpeed(pathIn, 1200);   */
      } else if (!strncmp(commandLine, "NO CARRIER", 10)) {
        previousSuccessCall = time(0);      /* fake it */
        if (mode == establish) {
          if (tries <= 2)
            minimumWaitTime = 30;
          else
            minimumWaitTime = 120;
        } else if (mode == connected) {
          minimumWaitTime = 1;                  /* removed 910905... 300; */
        }
/*        setSpeed(pathIn, originalSpeed);    */
        mode = disconnected;
      } else if (!strncmp(commandLine, "NO ANSWER", 9)) {
        previousSuccessCall = time(0);      /* fake it */
        if (mode == establish) {
          if (tries <= 2)
            minimumWaitTime = 30;
          else
            minimumWaitTime = 120;
        }
        mode = disconnected;
      } else if (!strncmp(commandLine, "BUSY", 4)) {
        if (mode == establish) {
          if (tries <= 2)
            minimumWaitTime = 30;
          else
            minimumWaitTime = 120;
        }
        mode = disconnected;
      } else if (!strncmp(commandLine, "NO DIALTONE", 11)) {
        if (mode == establish) {
          if (tries <= 2)
            minimumWaitTime = 30;
          else
            minimumWaitTime = 120;
        }
        mode = disconnected;
      } else if (!strncmp(commandLine, "RING", 4)) {
        mode = establish;
      } else if (!strncmp(commandLine, "OK", 2)) {
        if (mode == shutdown) {
          fprintf(fpOut, "ATH0\n");
          mode = disconnected;      /* we will not get any NO CARRIER message */
        }
      } else {
/*        mode = connected;    */          /* ???? */

        if (mode != establish) {
          return 1;       /* ok, return to caller */
        }
      }
      
      if (mode == connected && firstTime) {
/*        fprintf(fpOut, "HELLO PC\015");   */
        sleep(2);

{
#ifdef NEW_STAT_921103
  int pc;                         /* use global PCno */
#else
  int PCno, pc;
#endif

                        /* if pc calls us ?? */
  PCno = pc = telephonePC;

#ifdef NEW_STAT_921119
        fprintf(fpOut, "\015"); /* added 921119 */
        fprintf(fpOut, "HELLO PC\015");
#else

        if (conditionalAnyStatistics(pc) == 0) {
          fprintf(fpOut, "HELLO PC\015");   /* to sync, 910517 */
        } else {
#define CHEAPER_v920911
#ifdef CHEAPER_v920911
          int i;

          if (noOfStats2Send < 1 || noOfStats2Send > 400)
            noOfStats2Send = 4;
#ifdef NEW_STAT_921103
                /* just send one stat and main loop takes care of SACK */
          if (!sendStatistics(PCno))
                ;
          else
              itsAStatCall = 1;    /* added 921117 */

#else
          for (i = 0; i < noOfStats2Send; i++)
          {
            if (!sendStatistics(PCno))                 /* 1 */
              break;
          }
#endif          
/*
!   if all sent, no more calls until 3 hours (or so) has passed by
*/
          if (conditionalAnyStatistics(pc) == 0) {
            statisticSentAt[PCno & 7] = getRelTime(0);
          }
#else
          if (sendStatistics(PCno))                 /* 1 */
            if (sendStatistics(PCno))               /* 2 */
              if (sendStatistics(PCno))             /* 3 */
                sendStatistics(PCno);               /* 4 */
#endif                

#ifdef NEW_STAT_921103
#else
          fprintf(fpOut, "EXIT\015");
#endif          
	}
#endif                                /* NEW_STAT_921119 */	
        firstTime = 0;
}

      }     
     
    } else {     /* end of if (readline...) */
      if (-1 == alm_delete(0)) {
        fprintf(stderr, "cant del alarm, error = %d\n", errno);
      }
#ifdef DEBUG_MODE
      if (DEBUG)
        fprintf(stderr, "readLine returned zero: timeoutflag = %d, mode=%d\n", timeoutFlag, mode);
#endif
      if (timeoutFlag) {
        if (mode != disconnected) {
          if (mode == shutdown) {     /* ok, no OK reply == no modem or in command state ! */
            mode = disconnected;
          } else {
            write(pathOut, "+++", 3);/* use unbuffered since no \n can be used*/
            mode = shutdown;
          }
          continue;
        } else if (mode == disconnected) {
          if ((time(0) - previousSuccessCall) > minimumWaitTime) {
            int pc;
#ifdef TEST_MANY_PC            
            static int nextPcPtr;
            if (nextPcPtr >= maxPcs)
             nextPcPtr = 0;
            pc = mapPcs[nextPcPtr++];
            telephoneNumber = getTelephone(pc);
            telephonePC = pc;
#else
            pc = telephonePC;
#endif
/*
! OBS! call ONLY our PC's, there might be another visionCom running...
*/
            if ((anyAlarms(pc) || conditionalAnyStatistics(pc))
                  && telephoneNumber) {
              char mBuf[100];
              sprintf(mBuf, "ATE0Q0DT%s\015", telephoneNumber);
              write(pathOut, mBuf, strlen(mBuf));
              tries ++;
              mode = establish;
              firstTime = 1;
/*
              whenSETPCTIME_wasSent = 0;
*/
              continue;
            }
/*            pc ++;      */
          }
        }
      }
      else {
#ifdef DEBUG_MODE
        if (DEBUG)
          fprintf(stderr, "framing error\n");
#endif
      }
      mode = disconnected;        /* or idle ?????? */
    }
  }   /* end of while ( not read a line ) */
}

