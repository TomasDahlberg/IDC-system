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

  ureal = umin + (u_in / 10000.0) * (umax - umin);       /* in Volt */
  rg = rs / (10.0 / ureal - 1.0) - rw;                   /* rg in Ohm */
  uSteafa_system = rg * 15.0 / (10000.0 + rg);           /* u in Volt */

  return -50.0 + 200.0 * (uSteafa_system - 2.231) / 2.000;
}

