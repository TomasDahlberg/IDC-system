waitLcd macro
wl\@	
	btst	#$7,$308000
	bne.s	wl\@
 endm

lcdctrl	macro
	move.b	\1,$308000
 endm

lcddata	macro
	move.b	d0,$308001
 endm

READRTC macro
	move.b	$310000,d0
 endm

WRITERTC macro
	move.b	\1,$310000
 endm

WRITELED macro
	move.b	\1,$308002
 endm

lcdtest macro
	move.w	#$bb8,d0
lt2\@	btst	#7,$308000
	beq.s	lt1\@
	subq.w	#1,d0
	bne.s	lt2\@
lt1\@	tst.w	d0
 endm

lcdgo macro
	waitLcd
	lcdctrl	#$1
	waitLcd
	lcdctrl	#$0d
	waitLcd
	lcdctrl	#$38
	waitLcd
	lcdctrl	#$6
 endm

lcdput	macro
	waitLcd
	lcddata	
 endm
