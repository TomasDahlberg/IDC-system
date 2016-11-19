/**********************************************************************/
/* File:        DT08IO.H       Type: I  (mainProgram/Module/Include)  */
/* By:          Dan Axelsson                                          */
/* Description:	Adresses and constants for hardware on DT08           */
/* Use:         Included in module with #include <DT08IO.H>           */
/* ---------------------  History  ---------------------------------- */
/*   Date     /   What                                                */
/* .................................................................. */
/* 90-10-24   /   started                                             */
/* 91-07-30   /   New constants for SSEL4-7 ($100 added to old values)*/
/* 91-10-29   /   $40C used to hold address for net status storage    */
/**********************************************************************/
/*
void lcdinit();
void lcdwrite(unsigned char c);
void lcdcontrol(unsigned char);
void lcdpos(int r, int c);
void lcdputs(*char);
void lcdprintf(...);
void bell(int onoff);
void bellt(int duration);
void backlight(int onoff);
*/

/* Addresses to LCD and keyboard on MiniBus */
unsigned char *lcd_ctrl = 0x308000;
unsigned char *lcd_data = 0x308001;
unsigned char *keyboard = 0x308002;
unsigned char *ledreg = 0x308002;
/* bit 7 controls buzzer: 0=ON, bit 6 controls LCD-backlight: 1=ON */
/* bit 5 to 0 controls LEDs: 0=ON */
/* 74xx574 assumed, if 74xx576 is used control is reversed */
unsigned char in_ledreg;   /* copy of ledreg, register is write only */

/* bits in keyboard data */
#define KEYINT 0x80           /* interrupt bit, key pressed since last read */
#define KEYPRESSED 0x40       /* key pressed in this very moment */
/* bits in ledreg    */
#define BELL 0x80
#define BACKLIGHT 0x40
#define LED5 0x20
#define LED4 0x10
#define LED3 0x08
#define LED2 0x04
#define LED1 0x02
#define LED0 0x01

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
#define SSEL4 0x110         /* in op2 */
#define SSEL5 0x120         /* in op2 */
#define SSEL6 0x140         /* in op2 */
#define SSEL7 0x180         /* in op2 */

/* pointers to some memory cells */
unsigned char *staterr= 0x400;/* latest get or setstat command not understood*/
unsigned int *nodeaddr= 0x402; /* node address used by this node */
unsigned long *netstatp=0x40C; /* pointer to net status block */
unsigned long *memload= 0x410;  /* address to load module, used by /mem */
unsigned long *memget= 0x414;   /* address to read data from, used by /mem */
unsigned long *iplcode= 0x440;  /* equals to 0xC0DE if iplstring loaded */
char *iplstring= 0x442; /* startup text executed by shell at startup */
/* iplstring is loaded by IPL-command, max 190 chars */
unsigned char *addrmap= 0x500;  /* bitvector for groupaddresses, 128 bytes */
