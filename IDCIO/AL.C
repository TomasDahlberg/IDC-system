#define NO_OF_ALARMS 1
#include "../alarm.h"
/*
! marks alarm and returns TRUE (1) if called at raising edge
*/
int XmarkAlarm(aldm2, a, b, alarmNo, class)
struct _alarmModule2 *aldm2;
int a, b, alarmNo, class;
{
  struct _alarmPt *al;
  
  al = &aldm2->alarmPts[a];
  
  al->alarmNo = alarmNo;
  if (al->active == 1)
    return 0;
/*
  aldm2->alarmPts[a].alarmNo = alarmNo;
  if (aldm2->alarmPts[a].active == 1)
    return 0;
*/
  return _markAlarm(aldm2, a, b, alarmNo, class);
}

/* 
! unmarks alarm and returns TRUE (1) if called at falling edge
*/
int XunmarkAlarm(aldm, aldm2, a, alarmNo)
struct _alarmModule *aldm;
struct _alarmModule2 *aldm2;
int a, alarmNo;
{
  int i;
  struct _alarmPt *al;
  
  al = &aldm2->alarmPts[a];
  
  al->active = 0;
  if (!al->initTime)
    return 0;
/*
  aldm2->alarmPts[a].active = 0;
  if (!aldm2->alarmPts[a].initTime)
    return 0;
*/
  return _unmarkAlarm(aldm, aldm2, a, alarmNo);
}

