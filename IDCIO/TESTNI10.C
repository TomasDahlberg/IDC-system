#include <stdio.h>
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

double diffNi1000(u_ref, rs, u_in, step)
double u_ref, rs;
double u_in, step;
{
  return Ni1000(u_ref, rs, u_in + step) - Ni1000(u_ref, rs, u_in);
}

main()
{
  double prevTemp, temp, Uref, Rs, Uin, step;
  double dTemp, dTempMax, dTempMaxVolt, dTempMin, dTempMinVolt;
  int bitar;
    
  Uref = 10;
  Rs = 1000;
  dTempMax = -1000;
  dTempMin = 1000;
  bitar = 8;
  for (step = 0.04; bitar <= 16; step -= step/2, bitar ++) {
    Uin = 0;
    printf("step %g (%d)\n", step, bitar);
    for (Uin = 0; Uin <= 10; Uin += 1) {
      printf("   vid %g V  \t%5.2f grader, \tdelta-temp = %4.3f\n", 
          Uin, Ni1000(Uref, Rs, Uin), diffNi1000(Uref, Rs, Uin, step));
    }
  }
}
