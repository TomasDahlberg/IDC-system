
main()
{
  int x, prevSample;
  
 
  while (1) {
    x = DIGIN_1(x, 1, 1, 5, &prevSample);
    printf("                  x = %d\n", x);
    sleep(1);
  }
}

int DIGIN_1(dummy, module, channel, duration, prevSample)
int dummy;
int module, channel, duration;
long *prevSample;
{
  int tmp, bit, prevBit;
  static struct { int value, bitmask; } port[10];

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
    long now;
    prevBit = (*prevSample & (1 << 31)) ? 1 : 0;    /* get previous value */

  printf("bit = %d, prevBit = %d\n", bit, prevBit);    
    if (prevBit == bit) {
      (*prevSample) &= 0x80000000;      /*       stop timer     */
      return bit;
    }
    now = time(0);
    if (!((*prevSample) & 0x7fffffff))
      (*prevSample) |= now;
      
    if ((now - ((*prevSample) & 0x7fffffff)) >= duration) {  /* duration expired */      
      *prevSample = 0;                  /* stop timer and clear bit */
      (*prevSample) |= (bit << 31);     /*      previous bit = bit */
      return bit;
    }
    return prevBit;
  }
  return bit;
}

get_di(mod)
int mod;
{
  static int x, q, p;
  int t;
  

 t = time(0) & 31;
 printf("t = %d\n", t);
  if (t < 15) {
    printf("digin = %d\n", x);
    q = 1;
    return x++;
  } else {
    if (q) {
      q = 0;
      p ++;
    }
  }
  printf("digin %d\n", p);
  return p;
}
