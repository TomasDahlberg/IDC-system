/* dt08io.h  1991-08-03 TD,  version 1.1 */
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
 * Box 996
 * 191 29 Sollentuna
 * Sweden
 */

/*
! dt08io.h
! Copyright (C) 1991, IVT Electronic AB.
*/


/**********************************************************************/
/* File:	DT08IO.H       Type: I  (mainProgram/Module/Include)  */
/* By:		Dan Axelsson                                          */
/* Description:	Adresses and constants for hardware on DT08           */
/* Use:		Included in module with #include <DT08IO.H>           */
/* ---------------------  History  ---------------------------------- */
/*   Date     /   What                                                */
/* .................................................................. */
/* 90-10-24   /   started                                             */
/* 90-10-28   /   updated                                             */
/**********************************************************************/
/*
void lcdinit();
void lcdwrite(unsigned char c);
void lcdcontrol(unsigned char);
void lcdpos(int r, int c);
*/

/* Adresses to LCD and keyboard on MiniBus */
unsigned char *lcd_ctrl = 0x308000;
unsigned char *lcd_data = 0x308001;
unsigned char *keyboard = 0x308002;

/* bits in keyboard data */
#define KEYINT 0x80           /* interrupt bit, key pressed since last read */
#define KEYPRESSED 0x40       /* key pressed in this very moment */

/* pointer to G64/IO-bus base */
unsigned char *g64base = 0x300000;    /* 1024 bytes of I/O on this bus */

/* pointers to QUART */
unsigned char *qip1= 0x34000d;        /* input port 1 */
unsigned char *qip2= 0x34001d;        /* input port 2 */
unsigned char *qsop1= 0x34000e;       /* set bit (low) in output port 1 */
unsigned char *qrop1= 0x34000f;       /* reset bit (high) in output port 1 */
unsigned char *qsop2= 0x34001e;       /* set bit (low) in output port 2 */
unsigned char *qrop2= 0x34001f;       /* reset bit (high) in output port 2 */

/* bits in QUART regs */
#define SDI 0x04           /* in ip1 */
#define SINT 0x08          /* in ip1 */
#define DIP1 0x10          /* in ip1 */
#define DIP10 0x02         /* in ip2 */
#define OVCCOK 0x04        /* in ip2 */
#define SCLK 0x04          /* in op1 */
#define SDO 0x08           /* in op1 */
#define SSEL8 0x01         /* in op1 */
#define SSEL0 0x10         /* in op1 */
#define SSEL1 0x20         /* in op1 */
#define SSEL2 0x40         /* in op1 */
#define SSEL3 0x80         /* in op1 */
#define HBLED 0x04         /* in op2 */
#define BATTLED 0x08       /* in op2 */
#define SSEL4 0x10         /* in op2 */
#define SSEL5 0x20         /* in op2 */
#define SSEL6 0x40         /* in op2 */
#define SSEL7 0x80         /* in op2 */

/* pointers to some memory cells */
unsigned char *staterr= 0x400;/* latest get or setstat command not understood*/
unsigned short *nodeaddr= 0x402; /* node address used by this node */
unsigned long *iplcode= 0x400;  /* equals to 0xC0DE if iplstring loaded */
char *iplstring= 0x442; /* startup text executed by shell at startup */
/* iplstring is loaded by IPL-command, max 190 chars */
