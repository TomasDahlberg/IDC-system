static char *debugPtr = 0x3ffb0;
static char *debugRelease = 0x3ffb1;

/*
  Call frame:
  
          move.l  d1,-(a7)
          move.l  S.X(a7),d1
          movem.l d0-d7/a0-a7,-(a7)
          bsr     debugSkrift
          
  Stack frame is 
          
          d1         72+&b
          d0         68+&b
          d1         64+&b
          .
          .
          d7         40+&b
          a0         36+&b
          .
          .          12
          a7          8+&b
          PCret       4+&b
          b           8(sp)
          a           4(sp)
          
          
*/
debugSkrift(a,b)
long int a,b;
{
  if (*debugPtr) {
    printf("Vec = %d, PC = %x, ", a & 0xfff, b);
    printf("D0=%x, D1=%x\n", *((long *) &b+17), *((long *) &b+18));
    
    *debugRelease = 0;
    while (*debugRelease == 0)
      ;
  }
}
