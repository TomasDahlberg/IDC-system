/* rs232.c  1993-09-22 TD,  version 1.00 */
/*
 * This file contains proprietary information of IVT Electronic AB.
 * Copying or reproduction without prior written approval is prohibited.
 *
 * This file is furnished under a license agreement or nondisclosure
 * agreement. The software may be used or copied only in accordance 
 * with the terms of the agreement.
 *
 * In no event will IVT Electronic AB, be liable for any lost revenue or
 * profits or other special, indirect and consequential damages, even if
 * IVT has been advised of the possibility of such damages.
 *
 * IVT Electronic AB
 * Heimdalsgatan 4
 * 113 28 Stockholm
 * Sweden
 */

/*
! rs232.c
! Copyright (C) 1993 IVT Electronic AB.
*/

/*
!   rs232     - rs232 utility functions
!
!   history:
!   date       by  rev  what
!   ---------- --- ---  ------------------------------------
!   1993-09-22 td  1.00 initial coding
!
!
*/

/*
!   ch  1 - port 1
!       2 - port 2
!
!   length, length to assert in us
!
!   typ 0 - no address, normal data
!       1 - no address, month data
!       2 - emit address
!
!   address
*/
/*
rs232_DTR(ch, length, typ, address)
int ch, length, typ, address;
*/
rs232_DTR(data)
int data[4];
{
  int ch, length, typ, address;
  int sr;
  ch = data[0];
  length = data[1];
  typ = data[2];
  address = data[3];

  sr = setSR(0x2700);		/* supervisor mode and int level 7 only */
  assertRTS(ch);
/*  wait_us(typ == 0 ? 10000 : (typ == 1 ? 4000 : 6660));	*/
  wait_us(length); 
  if (typ == 2) {
    sendRTS_address(ch, address);
  }
  negateRTS(ch);
  setSR(sr);
}

#asm
*	d0 time in ms
*	d1 channel 1/2
*	Function: assert RTS on channel 'd1' for 'd0' ms

qsop1	equ	$34000e
qrop1	equ	$34000f
qsop2	equ	$34001e
qrop2	equ	$34001f
RTSb	equ	2			on port 1
RTSc	equ	1			on port 2

assertRTS:
	cmpi.l	#1,d0
	bne.s	ch2
	move.b	#RTSb,qsop1
	rts
ch2	move.b	#RTSc,qsop2
	rts

negateRTS:
	cmpi.l	#1,d0
	bne.s	ch2n
	move.b	#RTSb,qrop1
	rts
ch2n	move.b	#RTSc,qrop2
	rts

sendRTS_address:
        movem.l d2-d3/a0-a1,-(sp)
	cmpi.l	#1,d0
	bne.s	ch2s
	move.b	#RTSb,d2
	movea.l #qsop1,a0
	movea.l #qrop1,a1
	bra.s   send
ch2s    move.b  #RTSc,d2
	movea.l #qsop2,a0
	movea.l #qrop2,a1
*
* 1200baud, startbit, 8address bits, each 833us long
*
send
        move.b  d2,(a1)     send start bit
        move.l  #770,d0     24
        bsr.s   wait_us
        moveq   #7,d3       8
emit_bits
        ror.b   #1,d1       12
        bcc.s   emit_0      18/(12 not taken)
        move.b  d2,(a0)     12  send asserted bit
        move.l  #837,d0     24		824us
        bra.s   ok          18
emit_0  move.b  d2,(a1)     12  send negated bit
        move.l  #770,d0     24		824us
ok
        bsr.s   wait_us     
        dbra    d3,emit_bits  18/(26 not taken)
        movem.l (sp)+,d2-d3/a0-a1
	rts

*
*   hang for 'd0' us
*
wait_us:subq.l  #8,d0             12 t
us_lop  subq.l  #3,d0             12 t
        bcc.s   us_lop            18 t -> 30t, on MC68008/10MHz: 3us
        rts
*
*   10 -> 7 4 1 /1   -> 30+30+30+12+12+32+ (call) 34 = 18us
*
*   10 -> 10.2 us
*   100 ->    30*30 + 102 = 1002 = 100.2 us
*   833 -> 275*30 + 102 = 8352 = 835.2 us
*   1000 -> 330*30 + 102 = 10002 = 1000.2 us
*
* 833 -> 835.2 us -> 1197 baud -> -0.22 % error 

setSR:
	clr.l	d1
	move	sr,d1
	move	d0,sr
	move	d1,d0
	rts
#endasm
