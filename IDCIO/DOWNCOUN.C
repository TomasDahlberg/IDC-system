#define NO_OF_ALARMS 1
#include "sysvars.h"
/*
!   startDownCounter(pointer to var)      Enters var in linked list
!   _decrementList()                      Decrements each var in list with dT
!   _initDecList()                        Initialize decrement list
!
!   The function initDecList must be called before any other call at each start
!
!   This is done in _initScan() and in _initMain() after a delay of 1 sec.
!
!   The decrementList() function will be called in each cycle of the scan loop.
!   The startDownCounter will be called from IDC, e.g. the followin idc-code;
    
    int startDownCounter(int&);
    
    main
    {
      int qula;
      
      qula = 17;
      
      startDownCounter(qula);
      
      while (qula > 0)          // WAIT ~17 SECONDS
        ;
        
      // proceed
      
    }

*/

/*
!   This function enters a variable (the argument is a pointer to the var)
!   in a the linked lists.
*/
int startDownCounter(p)
long *p;
{
  int i, j;
  struct _system *sysVars;
  
  sysVars = (struct _system *) SYSTEM_AREA;
  while (lock(&sysVars->newTimerBusy))
    ;
/*    critical section */

  j = -1;
  for (i = 0; i < 32; i ++) {
    if (sysVars->newTimers.var[i] == p) 
      break;
    else if (!sysVars->newTimers.var[i] && j < 0) 
      j = i;
  }
  if (i >= 32 && j >= 0)
    sysVars->newTimers.var[j] = p;

/* end of critical section */  
  sysVars->newTimerBusy = 0;
}

/*
!   This function calculates the time difference, dT, since last call.
!   Then we decrement each variable in the linked list with this value, dT.
!   If any variable has reached/passed zero, the variable is set to zero
!   and removed from the linked list.
*/
int _decrementList()
{
  static long int previousRelTime;
  long t1, dT;
  int i;
  long *vp;
  struct _system *sysVars;

  t1 = getRelTime();
  dT = t1 - previousRelTime;
  previousRelTime = t1;
  if ((dT == 0) || (dT == t1))
    return 0;

  sysVars = (struct _system *) SYSTEM_AREA;
  
  while (lock(&sysVars->newTimerBusy))
    ;
/*    critical section */

  for (i = 0; i < 32; i ++) {
    if (!(vp = sysVars->newTimers.var[i]))
      continue;
    if (*vp == 0)
      sysVars->newTimers.var[i] = 0;
    if (((*vp) - dT) <= 0) {
      *vp = 0;
      sysVars->newTimers.var[i] = 0;
    } else
      (*vp) -= dT;
  }

/* end of critical section */  
  sysVars->newTimerBusy = 0;
}

int _initDecList()
{
  int i;
  struct _system *sysVars;

  sysVars = (struct _system *) SYSTEM_AREA;
  
  for (i = 0; i < 32; i ++) {
    sysVars->newTimers.var[i] = 0;
  }
  sysVars->newTimerBusy = 0;
}

#ifdef DEBUG
main()
{
  int i, j, k;
  
  initidcio();
  
  initDecList();
  
  i = 17;
  j = 3;
  k = 5;
  startDownCounter(&i);
  startDownCounter(&j);
  startDownCounter(&k);
  while (1) {
    printf("i = %d, j = %d, k = %d\n", i, j, k);
    decrementList();
    startDownCounter(&k);
  }
}

#asm
lock:
  movea.l d0,a0
  tas     (a0)
  bne.s   none_0
  clr.l   d0
  bra.s   ok
none_0
  moveq.l #1,d0
ok
  rts
#endasm

#endif
