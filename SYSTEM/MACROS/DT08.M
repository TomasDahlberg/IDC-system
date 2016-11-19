* FILE:      DT08.M
* DESCRIPTION:
*   macros for accessing DT/08 hardware
*   lcd-display unit, rtc etc.
*
*   lcdgo   should be used to initialize
*   lcd x   will try to display the ascii byte value x
*
* DATE:  90-11-25/co
*        91-04-03/da  WRITELED added
*
lcdtest macro
 move.w #3000,d0
lg\@ btst #7,$00308000
 beq.s lq\@
 subq.w #1,d0
 bne.s lg\@
lq\@ tst.w d0
 endm
lcdinit macro
lc\@ btst #7,$00308000
 bne lc\@
 move.b #\1,$00308000
 endm
lcd macro
lt\@ btst #7,$00308000
 bne lt\@
 move.b #\1,$00308001
 endm
lcdput macro
lt\@ btst #7,$00308000
 bne lt\@
 move.b d0,$00308001
 endm
lcdgo macro
 lcdinit $01
 lcdinit $0D
 lcdinit $38
 lcdinit $06
 endm

*
* READRTC
*   read from rtc device
*
READRTC macro
 move.b $310000,\1
 endm

*
* WRITERTC
*   write to rtc device
*
WRITERTC macro
 move.b \1,$310000
 endm

*
WRITELED macro
 move.b \1,$308002
 endm
