/*
!   a fast running conversion from Ni1000 resistance to temperature
*/
static double ni1000_resVec[] = {
        790.882, 851.151, 913.484, 977.994,
        1044.79, 1113.99, 1185.71, 1260.06,
        1337.15, 1417.09, 1500.01, 1586,
        1675.19, 1767.68, 1863.6, 1963.05
};

static double ni1000_tempVec[] = {
        -50, -35, -20,  -5,
         10,  25,  40,  55,
         70,  85, 100, 115,
        130, 145, 160, 175
};

double intpol(r, noPts, resVec, tempVec)
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
  printf("intpol: a=%d,b=%d,q=%g,dx=%g,dy=%g,T1=%g\n",
          a, b, q, dx, dy, T1);
  return T1;
}

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

  u_in = (682.072 + u_in * ( (1666.667 - 682.072) / 10000)) / 1000.00;
  if (u_ref - u_in) {
    rt = (u_in * rs) / (u_ref - u_in) - rw;
/*
!   Ni1000LG formula is: R(t) = R0(1 + At + Bt2 + Ct3)
!   while skipping t3 term, 
*/
    if (rt < 790.0)
      temp = -50.0;
    else if (rt > 1950.0)
      temp = 174.0;
    else {
      printf("rt = %g\n", rt);
      temp = intpol(rt, 16, ni1000_resVec, ni1000_tempVec);
      printf("temp = %g\n", temp);
/*
      temp = Ni1000LG2temp(rt);
      temp = solve(1000.0, rt, 4.427E-3, 5.172E-6);
*/
    }
  }
  else {
     temp = 910.34;
  }
  return temp;
}

double FilterFkn(prev, u_in, rw, rs, nPts, rVec, tVec)
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

/*
  u_in = (682.072 + u_in * ( (1666.667 - 682.072) / 10000)) / 1000.00;
*/
  u_in = u_in / 1000.00;
  if (u_ref - u_in) {
    rt = (u_in * rs) / (u_ref - u_in) - rw;
/*
!   Ni1000LG formula is: R(t) = R0(1 + At + Bt2 + Ct3)
!   while skipping t3 term, 
*/
/*    if (rt < 790.0)
      temp = -50.0;
    else if (rt > 1950.0)
      temp = 174.0;
    else {
*/      temp = intpol(rt, nPts, rVec, tVec);
/*
      temp = Ni1000LG2temp(rt);
      temp = solve(1000.0, rt, 4.427E-3, 5.172E-6);
*/
/*    } */
  }
  else {
     temp = tVec[nPts - 1] + 1; /* 910.34; */
  }
  return temp;
}

#define DEBUG
#ifdef DEBUG

static int nPt = 30;
static double rVec[] = {
         110, 140, 180, 230, 300, 400, 540, 740,
        1040, 1490, 1800, 1870, 1940, 2020, 2100,
        2180, 2270, 2360, 2460, 2560, 2670, 3290, 4090,
        5120, 6450, 8220, 10500, 13700, 17900, 23800
/*
        23800, 17900, 13700, 10500, 8220, 6450, 5120, 4090, 3290, 2670,
        2560, 2460, 2360, 2270, 2180, 2100, 2020, 1940, 1870, 1800,
        1490, 1040, 740, 540, 400, 300, 230, 180, 140, 110
*/
};

static double tVec[] = {
        120, 110, 100, 90, 80, 70, 60, 50, 40, 30,
        25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
        15, 10, 5, 0, -5, -10, -15, -20, -25, -30
/*
        -30, -25, -20, -15, -10, -5, 0, 5, 10, 15,
        16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
        30, 40, 50, 60, 70, 80, 90, 100, 110, 120
*/
};

main()
{
  double prev, u_in, rw, t, rs, res;
  int ires;
  
  prev = 0;
  u_in = 5000.0;
  rw = 0.0;
  rs = 10000;

  while (1) {  
    printf("res ? "); 
    scanf("%d", &ires);
    
    if (ires == 0) break;
  
  res = ires;
  
  u_in = 10.0*res / (rs+res); 
 
  u_in = (u_in * 1000.0 - 682.072) * 10000.0 / (1666.667 - 682.072);
/* 
  u_in = (682.072 + u_in * ( (1666.667 - 682.072) / 10000)) / 1000.00;
*/

  t = Ni1000LG(prev, u_in, rw);
  printf("%g ohm -> %g mv -> %g grC\n", res, u_in, t);
  
  u_in = 10000.0*res / (rs+res); 
  t = FilterFkn(prev, u_in, rw, rs, nPt, rVec, tVec);
  printf("%g ohm -> %g mv -> %g grC\n", res, u_in, t);
 
  }
  
}
#endif
