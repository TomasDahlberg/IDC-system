
float Float(dd)
double dd;
{
  if (dd > 1e+37)
    return 1e+37;
  if (dd < -1e+37)
    return -1e+37;
  if (0 > dd && dd > -1e-37)
    return 0;
  if (1e-37 > dd && dd > 0)
    return 0;
  return dd;
}

