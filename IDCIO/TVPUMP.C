/* tvpump.c  1991-08-19 MS,  version 1.1 */
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
! tvpump.c
! Copyright (C) 1991, IVT Electronic AB.
*/
 
/*
!     File: tvpump.c
!     
!     Contains code for ease the usage of twin pumps
!
!     History     
!     Date        Revision Who  What
!     
!     3-jun-1991    1.0    MS   Initial release of code
!     19-aug-1991   1.1    MS   bugfix, released in prom 1.622
!
*/
#include <stdio.h>
#include <time.h>

static int hour()     /* returns 0 - 23  */
{
  struct tm tidStruct;
  time_t now;

  now = time(0);
  memcpy(&tidStruct, localtime(&now), sizeof(struct tm));
  return tidStruct.tm_hour;
}

static int wday()     /* returns 1 - 7 for monday - sunday */
{
  struct tm tidStruct;
  time_t now;

  now = time(0);
  memcpy(&tidStruct, localtime(&now), sizeof(struct tm));
  return (tidStruct.tm_wday == 0) ? 7 : tidStruct.tm_wday;
}

static int week_hour(nr)   /* returns 1 at monday 12.00 */
int nr;
{
  static unsigned char on;

  if ((wday() == 1) && (hour() == 12) )
  {
    if (on & (1 << nr))
      return 0;
    on |= 1 << nr;
    return 1;
  }
  else {
    on &= ~(1 << nr);
    return 0;
  }
} 

int tvpump(nr, enable, larmA, larmB, dqA, dqB, overTime, pumpByte)
int nr, enable, larmA, larmB, *dqA, *dqB, overTime, *pumpByte;
{
  static char driftfall[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  static time_t tid[8];
      
  if (enable) 
  {
   if ( larmA && !larmB )
   {
     *dqA = 0;
     *dqB = 1;
   }
   if ( larmB && !larmA )
   {
     *dqB = 0;
     *dqA = 1;
   }
   if ( larmA && larmB )
   {
     *dqA = 0;
     *dqB = 0;
     driftfall[nr] = 0;
   }               	 
   switch ( driftfall[nr] )
   {
   case 0:   
	 if ( !larmA || !larmB )
	 {
	   if (larmA)
	     *dqB = 1;
	   else
	     *dqA = 1;
	   driftfall[nr] = 1;
	 }   
	 break;
   case 1:
	 if ( week_hour(nr) || *pumpByte )
	 {
	   *pumpByte = 0;
	   tid[nr] = time(0);
	   if ( *dqA )
	   {
	     *dqB = 1;
	     driftfall[nr] = 2;
	   }
	   else
	   {
	     *dqA = 1;
	     driftfall[nr] = 3;
	   }
	 } 
	 break; 
   case 2:
   case 3:
	 if ( difftime(time(0), tid[nr]) >= overTime )
	 { 
	   if ( driftfall[nr] == 2 ) 
	     *dqA = 0;
	   else
	     *dqB = 0;
	   driftfall[nr] = 1;
	 }
	 break;
   } 
  }
  else
  {
    *dqA = 0;
    *dqB = 0;
    driftfall[nr] = 0;
  } 
}
