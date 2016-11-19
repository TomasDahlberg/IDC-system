/* runtime_scan.c  1993-08-30 TD,  version 1.32 */
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
! runtime_scan.c
! Copyright (C) 1991-1993 IVT Electronic AB.
*/


/*
!   runtime_scan  - contains routines used in scan module
!
!   history:
!   date       by  rev  what
!   ---------- --- ---  ------------------------------------
!   1990-??-?? td  1.00 initial coding
!
!     91-05-02 td  1.01 Added Ni1000LG
!     91-05-10 td  1.02 Added Ni1000DIN, argument r0 = serial wire resistance
!                       and TEMP_1
!
!     91-05-21 td  1.03 adjustments for incl. com, ref+, ref- and rs in Ni1000
!                       calls
!     91-05-22 td  1.10 call frame for filter functions has changed.
!                       matched IDCC v1.1
!                       
!                       call frame is now;
!                                            previously returned value
!                                            module interface returned value
!                                            filter parameters
!     91-07-20 td  1.11 Ni1000 only one parameter, PULS_1 added
!                       checkCalendarRaise & Fall
!
!     91-07-20 td  1.12 Ni1000LG bugfix !!! rs was only 1000, now 10000 !!!
!
!     91-07-31 td  1.14 DIGOUT_1 channel 0 -> all eight bits
!
!     91-08-16 td  1.15 checkCalendarLevel stopday ok !
!
!     92-01-14 td  1.20 To minimize the size of idcio to fit into 256KB prom
!                       the following routines are now obsolete (for a while!)
!                       checkCalendarRaise(), checkCalendarFall()
!                       i2bin(), oldDIGIN_1
!
!     92-09-18 td  1.30 Added Staefa_PTC_150 filter function
!     92-11-24 td  1.31 Bugfix in Staefa 10.0 -> 10000.0 ohm !!
!     93-08-30 td  1.32 Added filter function res2PT100 and module fcn RES_280
*/
#define OBSOLETE_920114


#include <modes.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
 
#include "idcio.h"
/*
extern int DEBUG;
*/



#ifndef OBSOLETE_920114
/*
!    dm->b1_ta1 = Ni1000(0, 5, ANAIN_1(dm->b1_ta1, 3, 7, 0)); 
*/
/*
!   Ni1000 DIN
!
!   Ni1000 formula is: temp = 910.34 - (4551700 / (rt + 4000))
!
*/
double Ni1000(u_ref, rs, u_in)
double u_ref, rs;
double u_in;
{
  float rt, rt_noll, temp;

  if (u_ref - u_in) {
    rt = (u_in * rs) / (u_ref - u_in);
    temp = 910.34 - (4551700 / (rt + 4000));
  }
  else {
     temp = 910.34;
  }
  return temp;
}
#else
/* double Ni1000() { }  */
#endif
/*
!   Ni1000 DIN
!
!   Ni1000 formula is: temp = 910.34 - (4551700 / (rt + 4000))
!
*/
double Ni1000DIN(prev, u_in, rw)
double prev;
double u_in;
double rw;                   /* serial wire resistance */
{
  float rt, rt_noll, temp;
  
  double u_ref = 10.000;     /* 10V reference voltage */
  double rs = 10000;         /* 10kOhm serial resistance */

  u_in = (682.072 + u_in * ( (1666.667 - 682.072) / 10000.0)) / 1000.00;
/*
  u_in = 762.125 + u_in * ( (1575.400 - 762.125) / 10000);
*/
/*  u_in = u_in / 1000.00;  */

  if (u_ref - u_in) {
    rt = (u_in * rs) / (u_ref - u_in) - rw;
    temp = 910.34 - (4551700 / (rt + 4000));
  }
  else {
     temp = 910.34;
  }
  return temp;
}

/*
!   Solves t from R(t) = R0(1 + At + Bt2) for t > 0
*/
static double solve(R0, rt, A, B)
double R0, rt, A, B;
{
  double prefix, temp;

  prefix = -A/(2.0*B);
#ifdef DEBUG
  printf("          prefix=%g\n", prefix);
#endif
  temp = prefix + sqrt(prefix*prefix - (R0 - rt)/(B*R0));
#ifdef DEBUG
  printf("          temp=%g\n", temp);
#endif
  return temp;
}

#ifndef OBSOLETE_920114
/*
!   a fast running conversion from Ni1000 resistance to temperature
*/
static double res[] = {
        790.882, 851.151, 913.484, 977.994,
        1044.79, 1113.99, 1185.71, 1260.06,
        1337.15, 1417.09, 1500.01, 1586,
        1675.19, 1767.68, 1863.6, 1963.05
};

double delta[] = { 0.248884, 0.240643, 0.232522, 0.224564,
                   0.216763, 0.209147, 0.201748, 0.194578,
                   0.187641, 0.180897, 0.174439, 0.16818,
                   0.16218, 0.15638, 0.15083
};

#define RES 15

double Ni1000LG2temp(r)
double r;
{
  int i, a, b;
  double T1;
  double q;

  if (r < res[0])
    return -50;
  else if (r > res[15])
    return 175;

  b = 1;
  while (r > res[b])
    b++;
  a = b - 1;
  q = (r - res[a]) * delta[a];
  T1 = -50 + a * RES + q;
  if (q > 2.25 && q < 12.75)
    T1 += 0.06;

  return T1;
}
#endif

/*
!   Ni1000LG  routine
!   
!   Ni1000LG formula is: R(t) = R0(1 + At + Bt2 + Ct3)
!   where R0 = 1000, A = 4.427E-3, B = 5.172E-6
!
*/
double Ni1000LG(prev, u_in, rw)
double prev;
double u_in;
double rw;                   /* serial wire resistance */
{
  float rt, rt_noll;
  double temp;
  double u_ref = 10.000;                 /* 10V reference voltage */
  double rs = 10000;                     /* 10kOhm serial resistance */

#ifdef DEBUG
  printf("rw=%g,com=%g,vrm=%g,vrp=%g,rs=%g,uref=%g,uin=%g\n", 
             rw, com,vrefMinus, vrefPlus, rs, u_ref, u_in);
#endif

#ifdef DEBUG_2
  struct _test {
    long vcom1, vcom2, vref1, vref2;
  } *kalle;
  double t1, t2;
  
  kalle = (struct _test *) 0x003ffd0;
  
  t1 = kalle->vcom2/1000.0;
  t2 = kalle->vref2/1000.0;
  u_in = kalle->vcom1 + t1 + u_in * ( (kalle->vref1 + t2) / 10000);
  u_in = u_in / 1000.00;
#else  
  u_in = (682.072 + u_in * ( (1666.667 - 682.072) / 10000)) / 1000.00;
#endif
/*
  u_in = u_in / 1000.00;
*/

#ifdef DEBUG
  printf("Ni1000LG: u_in=%g\n", u_in);
#endif

  if (u_ref - u_in) {
    rt = (u_in * rs) / (u_ref - u_in) - rw;
/*    rt = rt - rw;   */
#ifdef DEBUG
    printf("          rt=%g\n", rt);
#endif
/*
!   Ni1000LG formula is: R(t) = R0(1 + At + Bt2 + Ct3)
!   while skipping t3 term, 
*/
    if (rt < 790.0)
      temp = -50.0;
    else if (rt > 1950.0)
      temp = 174.0;
    else {
/*
      temp = Ni1000LG2temp(rt);
*/
      temp = solve(1000.0, rt, 4.427E-3, 5.172E-6);
    }
  }
  else {
     temp = 910.34;
  }
  
#ifdef DEBUG
  printf("          temp=%g\n", temp);
#endif
  return temp;
}

static double intpol(r, noPts, resVec, tempVec)
int noPts;
double r, resVec[], tempVec[];
{
  int i, a, b;
  double T1, q, dx, dy;

  if (r < resVec[0])
    return tempVec[0];
  else if (r > resVec[noPts - 1])
    return tempVec[noPts - 1];

  b = 1;
  while (r > resVec[b])
    b++;
  a = b - 1;
  q = (r - resVec[a]);

  dx = tempVec[b] - tempVec[a];
  dy = resVec[b] - resVec[a];
  T1 = tempVec[a] + q * dx / dy;
/*
  printf("intpol: a=%d,b=%d,q=%g,dx=%g,dy=%g,T1=%g\n",
          a, b, q, dx, dy, T1);
*/
  return T1;
}

double filterFkn(prev, u_in, rw, rs, nPts, rVec, tVec)
double prev;
double u_in;
double rw;                   /* serial wire resistance */
double rs;
int nPts;
double rVec[], tVec[];
{
  float rt, rt_noll;
  double temp;
  double u_ref = 10.000;                 /* 10V reference voltage */
/*  double rs = 10000;                     /* 10kOhm serial resistance */

  if (u_ref - u_in) {
    rt = (u_in * rs) / (u_ref - u_in) - rw;
/*
!   Ni1000LG formula is: R(t) = R0(1 + At + Bt2 + Ct3)
!   while skipping t3 term, 
*/
      temp = intpol(rt, nPts, rVec, tVec);
  }
  else {
     temp = tVec[nPts - 1] + 1;
  }
  return temp;
}

int optoPower()
{
  return opto_power();
}

double battVoltage()
{
  float tmp;
  tmp = get_batt_voltage();
  return tmp / 1000.0;
}

int battLow()
{
  return (get_batt_voltage() < 3000) ? 1 : 0;
}

#ifndef OBSOLETE_920114
int count_1() {}      /* obsolete function */
int count_2() {}      /* obsolete function */
#endif

int count(prevCnt, level, index)
int prevCnt;
int level;
int index;
{
  static int oldLevel[16];      /* 16*4 = 64 bytes */

  index &= 15;
/*
  printf("prev=%d, level=%d, oldLevel[%d]=%d\n", 
          prevCnt, level, index, oldLevel[index]);
*/  
  if (oldLevel[index] == 0 && level == 1)
    prevCnt ++;
  oldLevel[index] = level;
  return prevCnt;
}

double Pt(R0, rt)
double R0, rt;
{
  float temp;
  double A = 3.90802E-03;
  double B = -5.802E-07;
  double prefix;
/*
!   Pt100 formula is: R(t) = R0(1 + At + Bt2) for t > 0
*/
  if (rt >= R0) {
    if (rt < 4*R0) { 
      prefix = -A/(2.0*B);
      temp = prefix - sqrt(prefix*prefix - (R0 - rt)/(B*R0));
    } else {
      temp = 998;
    }
  } else {
/*
!   Pt100 formula is: R(t) ~ R0(1 + At + Bt2) for t < 0
*/
    if (rt >= 0.2*R0) {
      prefix = -A/(2.0*B);
      temp = prefix - sqrt(prefix*prefix - (R0 - rt)/(B*R0));
    } else {
      temp = -199;
    }
  }
  return temp;
}

/*
!   Pt100
*/
double Pt100_calc(rw, rs, u_in)
double rw;                   /* serial wire resistance */
double rs;
double u_in;
{
  float rt, temp;
  double R0 = 100.0;
  double prefix;
  double u_ref = 10.00;

  u_in = (215.264 + u_in * (395.697 / 10000.0)) / 1000.00;
/*   u_in = u_in / 1000.00;   */
  if (u_ref - u_in) {
    rt = (u_in * rs) / (u_ref - u_in) - rw;
/*    rt = rt - rw;   */
    temp = Pt(R0, rt);
  } else {
     temp = 999;
  }
  return temp;
}

double res2PT100(prev, r_in, rw)
double prev;
double r_in;
double rw;
{
  double R0 = 100.0;
  return Pt(R0, r_in);
}

/*
!   Pt1000
*/
/*
!
*/
double Pt1000(prev, u_in, rw)
double prev;
double u_in;
double rw;                   /* serial wire resistance */
{
  float rt, temp;
  double R0 = 1000.0;
  double prefix;
  double u_ref = 10.00;
  double rs = 10000.0;
  
  u_in = (682.072 + u_in * ( (1666.667 - 682.072) / 10000.0)) / 1000.0;
/*  u_in = u_in / 1000.00;    */
  if (u_ref - u_in) {
    rt = ((u_in * rs) / (u_ref - u_in)) - rw;
/*    rt = rt - rw; */
    temp = Pt(R0, rt);
  } else {
     temp = 999;
  }
  return temp;
}

/*
!
*/
double Pt100_150(prev, u_in, rw)
double prev;
double u_in;
double rw;                   /* serial wire resistance */
{
  return Pt100_calc(rw, 2490.00, u_in);
/*  return Pt100(prev, u_in, rw, com, vrefMinus, vrefPlus, rs, u_ref); */
}

/*
!
*/
double Pt100_500(prev, u_in, rw) /* , com, vrefMinus, vrefPlus, rs, u_ref)  */
double prev;
double u_in;
double rw;                   /* serial wire resistance */
/* double com, vrefMinus, vrefPlus, rs, u_ref;  */
{
  return Pt100_calc(rw, 4420.00, u_in);
}

/*
!   Added 920918, Staefa PTC, Uses the TEMP_1 module interface function
!   The I/O card must be a modified TI01 card with 
!   Rs = R6-R13 = 5kOhm
!   R2 = R4 = 10kOhm
!   R3 = 3.48kOhm
!   R5 = 2.26kOhm
*/
double Staefa_PTC_150(prev, u_in, rw)   /* u_in is 0 - 10000.0 */
double prev;
double u_in;
double rw;                   /* serial wire resistance */
{
  double rs = 5000.0, 
      umin = 2.5816024,         /* umin = 10 * r3 / (r3 + r2) */
      umax = 4.4249955;         /* umax = umin + 10 * r5 / (r5 + r4) */
  double ureal, rg, uSteafa_system;

  ureal = umin + (u_in / 10000.0) * (umax - umin);    /* in Volt */
  rg = rs / (10.0 / ureal - 1.0);                     /* rg in Ohm */
  uSteafa_system = rg * 15.0 / (10000.0 + rg);           /* u in Volt */

  return -50.0 + 200.0 * (uSteafa_system - 2.231) / 2.000;
}

/*
!    reads a voltage value from analog in
!
*/
double TEMP_1(dummy, module, channel, duration)
double dummy;
int module, channel, duration;
{
  float tmp, w1, w2;
  long v, prev;
  static long memory[17][10];
  static char flag[17][10];

  
  if (duration < 1)
    duration = 1;
  w1 = 1 / ((float) duration);
  w2 = 1 - w1;
  
  v = get_ad(module, channel - 1);
/*
!
*/

  if (v < 50 || v > 9900)       /* added 920527 */
    return (float) v;           /* doesn't work properly if noisy environment */


  if (flag[module][channel] == 0)
  {
    flag[module][channel] = 1;
    prev = v;
  } else
    prev = memory[module][channel];
  tmp = w1 * v + w2 * prev;
  memory[module][channel] = tmp;

/* if we can use the same TEMP_01 card for both pt100 ni1000 etc
!  use this formula and return an accurate voltage value between
!  let's say 0.570V and 3.700V.
!
!  otherwise, if we cannot use the same card for both sensors,
!  just return and let someone else calculate the correct voltage value
!
*/

/*
! tmp = 0     ->   570 mV
! tmp = 10000 ->  3700 mV
*/ 

/*  tmp = 570 + tmp * ( (3700 - 570) / 10000 ); */


/*  printf(" mV=%g\n", tmp);  */

  return tmp;
}

/*
!    reads a resistance from PT100 3-wire, 4channel i/o card
*/
double RES_280(dummy, module, channel, duration)
double dummy;
int module, channel, duration;
{
  float tmp, w1, w2, prev;
  static float memory[16][4];
  double v;
  static char flag[16][4];      /* total 320 + 24 bytes */
  double PT100_Res();
  
  if (duration < 1)
    duration = 1;
  w1 = 1 / ((float) duration);
  w2 = 1 - w1;
  
  v = PT100_Res(module, channel);
  module --;
  channel --;
  module &= 15;
  channel &= 3;
/*
!
*/
  if (v < 20.0 || v > 279.0)
    return v;

  if (flag[module][channel] == 0)
  {
    flag[module][channel] = 1;
    prev = v;
  } else
    prev = memory[module][channel];
  tmp = w1 * v + w2 * prev;
  memory[module][channel] = tmp;

  return tmp;
}

/* reads a voltage value from analog in */
double ANAIN_1(dummy, module, channel, duration)
double dummy;
int module, channel, duration;
{
  float tmp, w1, w2;
  long v, prev;
  static long memory[16][8];
  static char flag[16][8];
  
  if (duration < 1)
    duration = 1;
  w1 = 1 / ((float) duration);
  w2 = 1 - w1;
  
  v = get_ad(module, channel - 1);

  if (v < 50 || v > 9900)       /* added 920527 */
    return ((float) v)/1000.0;  /* doesn't work properly if noisy environment */
             
  if (flag[module - 1][channel - 1] == 0)
  {
    flag[module - 1][channel - 1] = 1;
    prev = v;
  } else
    prev = memory[module - 1][channel - 1];
  tmp = w1 * v + w2 * prev;
  memory[module - 1][channel - 1] = tmp;

  return tmp / 1000.0;
}

/* writes a voltage value to an analog out */
double ANAOUT_1(value, module, channel, duration)
double value;
int module, channel, duration;
{
  int tmp;

  if (value < 0 || value > 10) {
    return value;
  }
  tmp = value * 1000;
  put_da(module, channel - 1, tmp);
  return value;
}


#ifndef OBSOLETE_920114
/* reads a digital in */
int oldDIGIN_1(dummy, module, channel, duration)
int dummy;
int module, channel, duration;
{
  int tmp;
  
/*  tmp = get_di(module);
  return tmp & (1 << (channel - 1));
*/  
}
#endif

/*
!   read digital input from channel
!
!   if (duration != 0) prevSample must be address of an unsigned long word !
!   otherwise it can be zero !
!
!   Algorithm:
!             if a change has occured and time since last change is 
!             less than 'duration', ignore the change
!   
*/
int DIGIN_1(dummy, module, channel, duration, prevSample)
int dummy;
int module, channel, duration;
long *prevSample;
{
  int tmp, bit, prevBit;
  static struct { int value, bitmask; } port[17];

  if (channel == 0) {
    port[module].value = tmp = get_di(module);
    port[module].bitmask = 0xffffffff;
    return tmp;
  }
  if (port[module].bitmask & (1 << (channel - 1))) {
    tmp = port[module].value;
    port[module].bitmask &= ~(1 << (channel - 1));
  } else {
    port[module].bitmask = 0xffffffff;
    port[module].value = tmp = get_di(module);
    port[module].bitmask &= ~(1 << (channel - 1));
  }

  bit = (tmp & (1 << (channel - 1))) ? 1 : 0;
  
  if (duration) {                           /* if duration specified */
    long now, elapsed;
    prevBit = (*prevSample & (1 << 31)) ? 1 : 0;    /* get previous value */
    
    if (prevBit == bit) {
      (*prevSample) &= 0x80000000;      /*       stop timer     */
      return bit;
    }
    now = getRelTime(0);
    if (!((*prevSample) & 0x7fffffff))
      (*prevSample) |= now;
    elapsed = (now - ((*prevSample) & 0x7fffffff));
/*    if ((now - ((*prevSample) & 0x7fffffff)) >= duration) */

    if ((elapsed < 0) || (elapsed >= duration)) {     /* duration expired */
      (*prevSample) = 0;                /*    stop timer and clear bit  */
      (*prevSample) |= (bit << 31);     /*      previous bit = bit */
      return bit;
    }
    return prevBit;
  }
  return bit;
}


#ifndef OBSOLETE_920114
char *i2bin(x)
int x;
{
  static char str[10];
  int i;
  for (i = 0; i < 9; i++)
  {
    str[8 - i] = (x & 1) ? '1' : '0';
    x >>= 1;
  }
  str[9] = '\0';
  return str;  
}
#endif

/* writes a digital out */
int DIGOUT_1(value, module, channel, duration)
int value;
int module, channel, duration;
{
  static int port[17], cnts[17];
  int tmp, prev;
#define MAX_COUNTS 20

  prev = port[module];
  if (channel == 0)
    port[module] = value;  
  else {
    port[module] &= ~(1 << (channel - 1));
    if (value)
      port[module] |= (1 << (channel - 1));
  }
  put_do(module, port[module] & 0x1ff);
/*  
  if ((prev != port[module]) || (cnts[module] <= 0)) {
    put_do(module, port[module] & 0x1ff);
    cnts[module] = MAX_COUNTS;
  } else {
    cnts[module]--;
  }
*/  
  return value;
}

/* int PULS_1() */         /* dummy entry for PULSE counter */

int PULS_1(dummy, module, channel, duration, prevSample)
int dummy;
int module, channel, duration;
long *prevSample;
{
  int tmp, bit;

  tmp = get_di_SR(module);
  bit = (tmp & (1 << (channel - 1))) ? 1 : 0;
  return bit;
}

int PULS_2(prev, module, channel, duration, prevSample)
int prev;
int module, channel, duration;
long *prevSample;
{
  int rel;

  rel = get_pulse(module, channel - 1);

  if (rel < 0)
    rel = 0;
#if 0
  else if (rel > 1000)            /* temporary bugfix inserted 920410 */
    rel = 0;
  else if (rel > 100)
    rel = 1;
#endif

  return prev + rel;
}
/*
filter Freq;
moduletype PULS_2;
module 1 is PULS_2;
int pdummy[3];
float f1 is Freq(pdummy) at module 1 channel 1;
*/

double Freq(prev, pulses, temparea)
double prev;
int pulses;
int *temparea;
{
  int rel, ms;
  float kHz;

  if (temparea) {
    ms = deltatime(temparea);

    kHz = ((float) pulses) / ((float) ms);

    return kHz * 1000.0;
  }
  return 0;
}


/*
!   takes a C-time and returns one of
!
!   1      in argument is black day 
!   2         - " -    is pink  day
!   3         - " -    is red   day
*/
int color(now)
time_t now;
{
  int year, month, day, cl;
  struct tm buff;
  
  memcpy(&buff, localtime(&now), sizeof(struct tm));
  
  cl = colour_of_day( buff.tm_year + ((buff.tm_year > 70) ? 1900 : 2000),
                                buff.tm_mon + 1, buff.tm_mday);
/*  if (cl == 0) {  */
/*
    fprintf(stderr,
        "runtime_scan/color: illegal date, C-time = %d,  %d:%d:%d\n", now,
          buff.tm_year + ((buff.tm_year > 70) ? 1900 : 2000),
                                buff.tm_mon + 1, buff.tm_mday);
*/                                
/*  } */
  return (cl == 3) ? 4 : cl;
}

int checkLevel(now, prev, glitchPtr, abs, rel, day, includeFlag)
time_t now, prev;
struct _glitch *glitchPtr;
long abs, rel, day, includeFlag;
{
  int ok = 1;

  if (includeFlag & _GLITCH_INCLUDED_ABS) {
    time_t midNight;
    struct tm buff;
    if (abs < 86400) {  
      memcpy(&buff, localtime(&now), sizeof(struct tm));
      buff.tm_sec = buff.tm_min = buff.tm_hour = 0;
      midNight = mktime(&buff);
    } else
      midNight = 0;
    ok &= (now - midNight >= abs);
  }
/*
  if (glitchPtr->includeFlag & _GLITCH_INCLUDED_REL) {
    ok &= ((now - prev) >= glitchPtr->rel);
  }
*/
  if (includeFlag & _GLITCH_INCLUDED_DAY) {
    ok &= (color(now) & day);
  }
  return ok;
}

checkGlitchRaise(glitchPtr, abs, rel, day, includeFlag)
struct _glitch *glitchPtr;
long abs, rel, day, includeFlag;
{
  time_t now, nowRel;
  int ok, newState;

  now = time(0);
  nowRel = getRelTime(0);
  if (!glitchPtr->prevTime) { /* init sequence, new 910207 */
    glitchPtr->prevTime = nowRel;
    if ((includeFlag & _GLITCH_INCLUDED_DAY) ||
        (includeFlag & _GLITCH_INCLUDED_ABS)) {
      glitchPtr->prevState = 1;      /* checkGlitchRaise starts at high level */
    }
  }
  newState = checkLevel(now, glitchPtr->prevTime, glitchPtr,
                    abs, rel, day, includeFlag);

  ok = !(glitchPtr->prevState) && newState;
/*
!   moved from checkLevel to here so newState flag 
!   doesn't reflect relative time changes, obs ! glitchPtr->prevState
!   must be initialized to zero, i.e. false
*/
  if (includeFlag & _GLITCH_INCLUDED_REL) {
    ok &= ((nowRel - glitchPtr->prevTime) >= rel);
  }

/*
  if (DEBUG) printf("checkGlitchRaise: pS = %d, nS = %d, ok = %d\n", 
          glitchPtr->prevState, newState, ok);  
*/          
  if (ok) {
    glitchPtr->prevTime = nowRel;
  }

  if ((includeFlag & _GLITCH_INCLUDED_DAY) ||
      (includeFlag & _GLITCH_INCLUDED_ABS)) {
    glitchPtr->prevState = newState;
  }
  return ok;
}

/*  #ifndef OBSOLETE_920114 */
checkGlitchFall()
{
  return 1;     /* always, otherwise checkGlitchRaise will capture thread */
/*  
    The idc/c-code is as follows;
    
      if (checkGlitchRaise(...))
        ;
      else if (checkGlitchFall(...))
        ;
*/
}
/* #endif   */

#define WEEKDAY_MASK 2048
#define ACTIVE_MASK  4096
#define DAYSPEC_MASK 2047
#define NO_OF_CAL_ENTRIES 10
struct _calendar
{
  unsigned short day[NO_OF_CAL_ENTRIES];
  unsigned short stopday[NO_OF_CAL_ENTRIES];
  unsigned char  color[NO_OF_CAL_ENTRIES];
  unsigned short start[NO_OF_CAL_ENTRIES];
  unsigned short stop[NO_OF_CAL_ENTRIES];
};

#define WEEKDAY 1
#define DAYSPEC 2

checkCalendarLevel(cal)
struct _calendar *cal;
{
  return calActiveIn(cal, 0);
}

calActiveIn(cal, in)
struct _calendar *cal;
int in;
{
  int year, month, day, hour, min, sec, weekday, weekyear, weekno, color;
  int dayspec, i, timespec, oktyp, typ, okrow, ok;
  long c;
  
  c = time(0) + in * 60;
  getTime4(c, &year, &month, &day, 
          &hour, &min, &sec, 
          &weekday, &weekyear, &weekno, &color);

  dayspec = month * 100 + day; 
  timespec = hour * 100 + min;
  okrow = ok = 0;
  for (i = 0; i < NO_OF_CAL_ENTRIES; i++)
  {
    if (cal->day[i] & WEEKDAY_MASK)
    {
      if (cal->day[i] & (1 << weekday))               /* Mon-Thu */
      {
        typ = WEEKDAY;
        ok = 1; 
        /* ok ! */
      } else {
        ok = 0;
        /* not today ! */
      }
    } else if (cal->day[i]) {                       /* date but not zero */
      int day1, day2;
      day1 = cal->day[i] & DAYSPEC_MASK;
      day2 = cal->stopday[i] & DAYSPEC_MASK;
      
      if ((day1 == dayspec) || (day2 == dayspec))
        ok = 1;
      else if (day2)    /* check between */
      {
        ok = (day1 < dayspec) && (dayspec < day2);
        if (day1 > day2)
          ok = ! ok;
      } else
        ok = 0;
      if (ok)
        typ = DAYSPEC;
    }
   
    if (cal->color[i])
    {
      if (cal->color[i] & color)
        ok &= 1;        /* ok ! */
      else
        ok = 0;        /* not today ! */
    } else {
      /* color doesn't matter */
    }
    if (ok) {     /* check intervall */
      /* if day and color ok, cancel any previous ok row with different type */
      
      if (okrow && (oktyp != typ))
      {
        okrow = 0;
      }

      if (cal->start[i] != 2400) {
        if (cal->start[i] > timespec)
          ok = 0;
      }
      if ((cal->stop[i] != 2400) && (cal->stop[i] != 0000)) {
        if (cal->stop[i] <= timespec)
          ok = 0;
      }
    }
    if (ok)     /* check only day and color */
    {     /* if above ok, remember row, since if many makes conflict get last!*/
      oktyp = typ; 
      okrow = i + 1;
    }
  }     /* next row */

  return okrow;
}

#ifndef OBSOLETE_920114
checkCalendarRaise(cal)
struct _calendar *cal;
{
  int i, prevActive = 0, okrow;
  
  for (i = 0; i < NO_OF_CAL_ENTRIES; i++)
    if (cal->day[i] & ACTIVE_MASK)
      prevActive = 1;
  okrow = checkCalendarLevel(cal);
  if (okrow)
  {
    cal->day[okrow - 1] |= ACTIVE_MASK;
    for (i = 0; i < 8; i++)
      if ((cal->day[i] & ACTIVE_MASK) && (i != (okrow - 1)))
        cal->day[i] &= ~ACTIVE_MASK;
  }
/*  return (prevActive == 0 && okrow != 0); */
  return (okrow != 0);
}

checkCalendarFall(cal)
struct _calendar *cal;
{
  int active, i, prevActive = 0;
  
  for (i = 0; i < NO_OF_CAL_ENTRIES; i++)
    if (cal->day[i] & ACTIVE_MASK)
      prevActive = 1;
  active = checkCalendarLevel(cal);
  if (active == 0)
  {
    for (i = 0; i < NO_OF_CAL_ENTRIES; i++)
      if (cal->day[i] & ACTIVE_MASK)
        cal->day[i] &= ~ACTIVE_MASK;
  }
/*  return (prevActive == 1 && active == 0);  */
  return (active == 0);
}
#else
/*
int checkCalendarRaise()  { }
int checkCalendarFall() { }
*/
#endif


