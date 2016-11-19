float Float(dd)
double dd;
{
  if (dd > 1e+37)
    return 1e+37;
  if (dd < -1e+37)
    return -1e+37;
  if (0 > dd && dd > -1e-37)
    return -1e-37;
  if (1e37 > dd && dd > 0)
    return 1e+37;
  return dd;
}


main()
{
  double x;
  float y;
  
 
  x = 3.12e+57;
  printf("x = %g\n", x);
  y = Float(x);
  printf("y = %g\n", y);
}
