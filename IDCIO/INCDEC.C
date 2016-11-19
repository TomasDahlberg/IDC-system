/* incdec.c  1992-06-11 MS,  version 1.0 */
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
! incdec.c
! Copyright (C) 1992 IVT Electronic AB.
*/

#include <stdio.h>
#include <time.h>
#include <math.h>

int incDec(nr, styrSignal, actTime, inc, dec, hyst)
long nr;
double styrSignal, hyst;
long actTime, *inc, *dec;
{ 
  double d; 
  static double oStyrSignal[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  static time_t tid[8], onTid[8];
  int intInc, intDec;

  intInc = *inc;
  intDec = *dec;
  d = styrSignal - oStyrSignal[nr];

  if( intInc == 1 ) {
    if( difftime(time(0), tid[nr]) >= onTid[nr] ) 
      intInc = 0;
    if(d > hyst) {
      onTid[nr] = onTid[nr] - difftime(time(0), tid[nr]) + (d/100 * actTime);
      intInc = 1;
    }
  } 
  else {
    if(d > hyst) {                               /* Inc */
      intInc = 1;
      intDec = 0;
      tid[nr] = time(0);
      onTid[nr] = d/100 * actTime;               /* Gangtid i sekunder */
    }
  }

  if( intDec == 1 ) {
    if( difftime(time(0), tid[nr]) >= onTid[nr] ) 
      intDec = 0;
    if(d < -hyst) {
      onTid[nr] = onTid[nr] - difftime(time(0), tid[nr]) + (-d/100 * actTime);
      intDec = 1;
    }
  }
  else {
    if(d < -hyst) {                               /* Dec */
      intDec = 1;
      intInc = 0;
      tid[nr] = time(0);
      onTid[nr] = -d/100 * actTime;               /* G†ngtid i sekunder */
    }
  }

  if ((fabs(d) > hyst) && (!(intInc && *dec)) && (!(intDec && *inc)))
    oStyrSignal[nr] = styrSignal;
    
  *inc = intInc && !*dec;
  *dec = intDec && !*inc;
} 

#ifdef TEST
void main(argc, argv)
int argc;
char *argv[];
{
 double v, ANAIN_1(), vUt, bv, pid(), p, i, d, q;
 long up=0, down=0;
 
 bv = atof(argv[1]);
 p = atof(argv[2]);
 i = atof(argv[3]);
 d = atof(argv[4]);
 
 printf("bv = %g, p = %g, i = %g, d = %g\n", bv, p, i, d);

 initphyio();
 initidcio();

 vUt = 5;
 q = 50;
 while(1)
 {  
  v = ANAIN_1(0.0,1,1,0);
  printf("v=%3.1f, ",v);

  q = pid(1, v, bv, p, i, d, 0.0, 100.0, q);
  printf("q=%g, ", q);
  incDec(1, q, 100, &up, &down);
  DIGOUT_1(up,2,1,0);
  DIGOUT_1(down,2,2,0);

  if (up) {
    vUt = vUt + 0.1; 
    if (vUt > 10)
      vUt = 10;
  }
  if (down) {
    vUt = vUt - 0.1;
    if (vUt < 0)
     vUt = 0;
  }
  printf("vUt=%g\n", vUt);
  ANAOUT_1(vUt, 3, 1, 0);
  sleep(1);
  }
}
#endif
