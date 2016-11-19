int overflow;
int zeroDivide;
main()
{
  int x, y, z;
  _setUpOverflowHandler();
  
  x = 17, y = 0;
  z = x / y;
  printf("overflow = %x\n", overflow);
  printf("zeroDivide = %x\n", zeroDivide);
}

static long *our_overflow_pc, *our_zerodivide_pc;

int _setUpOverflowHandler()
{
  our_overflow_pc = &overflow;
  our_zerodivide_pc = &zeroDivide;
  if (our_overflow_pc || our_zerodivide_pc) {
    init();
  }
}

#asm
ExcpTbl   
          dc.w    T_ZerDiv,Divide-*-4
          dc.w    T_TRAPV,Divide-*-4
          dc.w    -1

init     movem.l  a0-a1,-(a7)
         movea.l  #0,a0
         lea      ExcpTbl(pc),a1
         os9      F$STrap
         movem.l  (a7)+,a0-a1
         rts

Divide   andi.l   #$ffff,d7
         cmpi.w   #28,d7
         bne.s    zerod
         move.l   our_overflow_pc(a6),a1
         beq.s    not_set
         bra.s    ok
zerod    move.l   our_zerodivide_pc(a6),a1
         beq.s    not_set
ok       move.l   a0,(a1)                       /* save pc at idc-var place */

*****************         move.w   line(a6),error_line(a6)

not_set  move.l   R$a7(a5),a7
         move.l   R$pc(a5),-(a7)
         move.w   R$sr(a5),-(a7)
         movem.l  (a5),a0-a6/d0-d7
         rtr
#endasm

