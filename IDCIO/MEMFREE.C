#include <setsys.h>

main()
{
  struct { int a, s; } m[200];
  int d0, d1, d2, d3, i;
  int tot = 0;
  
  printf("_sysglob = %d\n", _sysglob);
  printf("sysglob = %d\n", sysglob(int, D_FreMem));
  
  printf("_getsys = %d\n", _getsys(D_FreMem, 4));
  
  printf("freemem = %d\n", freemem());
  printf("stacksiz = %d\n", stacksiz());
  
  d0 = gblk(m,800, &d1, &d2, &d3);
  printf("d0 = %d, d1 = %d, d2 = %d, d3 = %d\n", d0, d1, d2, d3);
  for (i = 0; i < d1; i++) {
    printf("%d: %d, %d\n", i, m[i].a, m[i].s);
    tot += m[i].s;
  }
  printf("Tot = %d\n", tot);
  printf("memfree says = %d\n", memfree());
  printf("memfree says = %d\n", memfree() / 1000);
  printf("memfree says = %d\n", memfree() >> 10);
}


#asm
memfree:
  moveq.l   #0,d0
  moveq.l   #0,d1
  OS9       F$GBlkMp
  move.l    d3,d0
  rts
#endasm


#asm
gblk:
  move.l    d0,a0
  moveq.l   #0,d0
  OS9       F$GBlkMp
  movea.l   4(a7),a0
  move.l    d1,(a0)
  movea.l   8(a7),a0
  move.l    d2,(a0)
  movea.l   12(a7),a0
  move.l    d3,(a0)
  rts
 
#endasm

