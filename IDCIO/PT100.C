double sqrt();

double Pt(R0, u_in, rs)
double R0;
double u_in;
double rs;
{
  float rt, rt_noll, temp;
  double A = 3.90802E-03;
  double B = -5.802E-07;
  double prefix;
  double u_ref = 10.000;
  
  if (u_ref - u_in) {
    rt = (u_in * rs) / (u_ref - u_in);
/*
!   Pt100 formula is: R(t) = R0(1 + At + Bt2) for t > 0
*/
    if (rt >= 100.0) {
     
      if (rt < 40000) { 
        prefix = -A/(2.0*B);
        temp = prefix - sqrt(prefix*prefix - (R0 - rt)/(B*R0));
      } else {
        temp = 998;
      }
    } else {
/*
!   Pt100 formula is: R(t) ~ R0(1 + At + Bt2) for t < 0
*/
      if (rt >= 20) {
        prefix = -A/(2.0*B);
        temp = prefix - sqrt(prefix*prefix - (R0 - rt)/(B*R0));
      } else {
        temp = -199;
      }
    }
  }
  else {
     temp = 999;
  }
  return temp;
}


/*
  return Pt100(rw, 2490.00, u_in);
  return Pt100(rw, 4420.00, u_in);
*/

main()
{
  double t;
  double R0, u_in, rs, rt;

  R0 = 1000;
  rs = 10000;

  for (u_in = 0.6; u_in < 1.7; u_in += 0.001)
  {
    rt = (u_in * rs) / (10.0 - u_in);
    t = Pt(R0, u_in, rs);
    printf("Uin=%g, rt = %g, T = %g\n", u_in, rt, t);
  }
}
