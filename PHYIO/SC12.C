#include "dt08io.h"

static int phyioError;

int getPhyioError()
{
  int tmp;
  tmp = phyioError;
  phyioError = 0;
  return tmp;
}

static int module2select(module, port)
int module, *port;
{
  static char list[] = 
    {SSEL0, SSEL1, SSEL2, SSEL3, SSEL4, SSEL5, SSEL6, SSEL7, SSEL8};

  if (/* module == 9 ||*/  module < 5)
    *port = 1;          /* port 1 */
  else
    *port = 2;          /* port 2 */
  if (module >= 1 && module <= 8 /* 9 */)
    return list[module - 1];
  if (module > 8) {
    *port = 2;
    return SSEL7;     /* use expander ! */
  }
  return SSEL0;
}

static void ldelay(delay)
int delay;
{
  int i;
  for (i=0; i<delay; i++)
    ;
}

static void set_sclk(hilo)
int hilo;
{
  if (hilo)
    *qrop1 = SCLK;  /* set clock line high */
  else
    *qsop1 = SCLK;  /* set clock line low */
}

static void set_sdo(hilo)
int hilo; /* value 1 or 0 */
{
  if (hilo)
      *qrop1 = SDO;   /* set data out high */
    else
      *qsop1 = SDO;   /* set data out low */
}

static void clock_data();

static int emit_expander_sel(module)
int module;
{
  int sel;

  sel = (module > 8) ? 8 : module;
  sel --;
  set_sclk(0);                /* just in case of put_dac... */
  clock_data(sel & 4);
  clock_data(sel & 2);
#ifdef OK_FOR_AU_IN_EXPANDER
  set_sdo(sel & 1);
  set_sclk(1);               /* leave clock high for MAX500 */
/*  set_sclk(0);    */
#else
  clock_data(sel & 1);
#endif
  if (module > 8) {
    emit_expander_sel(module - 8);
  }
}

static void set_ssel(selnr, hilo, port, module)
int selnr;   /* select line 0-7, codes SSEL0-SSEL7 must be used */
int hilo;    /* value 1 or 0 */
int port;    /* 0 for port 1,  1 for port 2 */
int module;  /* module #, 1 - ...     greater than 8 when using expander ! */
{
  if (port == 1)
  {
    if (hilo)
      *qrop1 = selnr;  /* set select line high */
    else
      *qsop1 = selnr;  /* set select line low */
  } else
  {
    if (hilo)
      *qrop2 = selnr;  /* set select line high */ /* exp automatic. unselected*/
    else {
      *qsop2 = selnr;  /* set select line low */
      if (module > 8)
        emit_expander_sel(module - 8);
    }
  }
}

static void clock_data(hilo)
int hilo;
{
/*  set_sdo(hilo);  */
/*  set_sclk(1); */
/*  set_sclk(0); */
  if (hilo)
      *qrop1 = SDO;   /* set data out high */
    else
      *qsop1 = SDO;   /* set data out low */

    *qrop1 = SCLK;  /* set clock line high */
    *qsop1 = SCLK;  /* set clock line low */
}

/*****************************************************************/
/* function : get_siid(nbits)                                    */
/* description: read data clock high and low nbits times         */
/*              shift bits to right in return value              */
static int get_siid(nbits, out)
int nbits, out;
{
  int i;
  int val;
  val = 0;

  set_sclk(1);		
  set_sclk(0);

  set_sclk(1);		
  set_sclk(0);

/*  set_sdo(0);   */
  for (i=0; i<nbits; i++)
  {
    if ((*qip1 & SDI) != 0 )
        val= (val<< 1) | 1;
    else
        val= (val<< 1) ;
    if (i >= nbits - 5) {
      set_sdo(out & 1);
      out >>= 1;
    }
    set_sclk(1);
    set_sclk(0);    /* clock low */
  }
  return (val);
}

static void put_siido(val,bits)
unsigned int val, bits;
{
  int i;
  val <<= (16-bits);
  for (i=0; i< bits; i++,val<<=1)
    if (val & 0x8000)
	clock_data(1);
      else clock_data(0);
}

int get_ad(module, chn)
int module, chn;
{
  int bval, select, port;
  long bvolt;
  float Ia, Ib, Rg, Ug;

  select = module2select(module, &port);  
  set_sclk(0);               /* set Clock low first */
  set_ssel(select,0, port, module);         /* select ADC-board */
  bval= (get_siid(12, chn) & 0xFFF);		/* and set up for Ib measure */
  set_ssel(select,1, port, module);         /* select ADC-board */
  return bval;
}

main(a,b)
int a;
char **b;
{
  int c[12], i, m, mx, t1, t2, def[4], start = 1;
  double i1, i2, ug, r;
  unsigned short int antal[40][4]; 	/* 2*40*4 = 320 byte */

  double min[4], max[4], sum[4];

  if (a >= 2) mx = atoi(b[1]); else mx = 1;
  t1 = time(0);

  min[0] = min[1] = min[2] = min[3] = 9999;
  max[0] = max[1] = max[2] = max[3] = 0;
  sum[0] = sum[1] = sum[2] = sum[3] = 0;

  for (m=0;m<40;m++) for (i = 0; i < 4; i++) antal[m][i] = 0;
  for (m = 0; m < mx; m++) {
   for (i = 0; i < 4; i++) {
    get_ad(1, i);  /* set up for ch 0 */
    tsleep(1);
    c[i] = get_ad(1, i + 8);	/* read prev and set up for I2 */
    tsleep(1);
    c[i+4] = get_ad(1, i + 16);
    tsleep(1);
    c[i+8] = get_ad(1, 4);	/* if i==3, then shut off */
    tsleep(2);
   }


   if (start) {
     for (i = 0; i < 4; i++)
       def[i] = c[i];
     start = 0;
   }

  for (i = 0; i < 4; i++) {
    i1 = c[i+8];  i1 /= 4095.0; i1*=2.802; i1 /= 0.100;  /* mA */
    i2 = c[i+4];  i2 /= 4095.0; i2*=2.802; i2 /= 0.100;  /* mA */

    if (a > 2) i1 = 10.0;

    ug = c[i]; ug /= 4095.0; ug*=2.802;
    r = ug / i1;
    r = r*1000.0;
    printf("%6.2f %4d  ", r, c[i] - def[i]);

    antal[20 + (c[i] - def[i])][i] ++;

    if (r < min[i]) min[i] = r;
    if (r > max[i]) max[i] = r;
    sum[i] += r;
  }
  printf("\n");


  }
  
  t2 = time(0);
  if (mx > 1) 
    printf("%d sec for %d loops\n", t2 - t1, mx);

  for (i = 0; i < 4; i++) {
    printf("Ch%d: min = %6.2f, max = %6.2f, avg = %6.2f\n", i, min[i], max[i],
		sum[i] / mx);
  }
  if (a > 3) {
    for (m = 39; m >= 0; m--) {
      for (i = 0; i < 4; i++) {
	if (antal[m][i]) 
	  break;
      }
      if (i < 4)
	break;
    }
    mx = m;
    for (m = 0; m < 40; m++) {
      for (i = 0; i < 4; i++) {
	if (antal[m][i])
	  break;
      }
      if (i < 4)
	break;
    }


    printf("      Ch0  Ch1  Ch2  Ch3\n");
    for (; m <= mx; m++) {
      printf("%4d: ", m - 20);
      for (i = 0; i < 4; i++) {
        if (antal[m][i]) 
  	  printf("%3d  ", antal[m][i]);
        else
  	  printf("     ");
      }
      printf("\n");
    }

  }
  
}


