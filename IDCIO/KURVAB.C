/* kurvab.c  1991-08-03 TD,  version 1.1 */
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
! kurvab.c
! Copyright (C) 1991, 1992 IVT Electronic AB.
*/
#define REQUEST_911218 


/*	Kurvfunktion MS 910603  */

#include <stdio.h>

double kurva(regTemp, antPkt, x, y)
double regTemp;
int antPkt;
double x[], y[];
{
  int i;

#ifndef REQUEST_911218 
  if (regTemp <= x[0])
    return y[0];
#endif
  for (i = 1; i < antPkt; i++)  
    if (regTemp <= x[i])
      return ( y[i] - y[i-1] ) / ( x[i] - x[i-1] ) * ( regTemp - x[i-1] ) + y[i-1];
#ifdef REQUEST_911218
  i--;
  return ( y[i] - y[i-1] ) / ( x[i] - x[i-1] ) * ( regTemp - x[i-1] ) + y[i-1];
#else
  return y[antPkt-1];
#endif
}
    
#ifdef TEST
void main()
{ 	
 float k[] = { -20, -10, -5, 0, 5, 10, 20 };
 float g[] = {  70,  60, 50, 40,30,20, 10 };
 float kalle;
 int i;

 system("cls");

 for (i = -20; i < 20; i++)
 {
   kalle = kurva (i, 7, k, g);
   printf(" %1.1f",kalle);
 }
}
#endif
