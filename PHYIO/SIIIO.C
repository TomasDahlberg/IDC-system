/* siiio.c  1993-08-30 TD,  version 1.51 */
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
! siiio.c
! Copyright (C) 1991-1993 IVT Electronic AB.
*/


/**********************************************************************/
/* File:	SIIMOD.C       Type: M  (mainProgram/Module/Include)  */
/* By:		Dan Axelsson                                          */
/* Description:	Functions to read and write modules on SII-bus        */
/* Use:		                                                      */
/* ---------------------  History  ---------------------------------- */
/*   Date     /   What                                                */
/* .................................................................. */
/* 90-10-28   /   started                                             */
/* 90-11-04   /   updated                                             */
/* 90-12-09   /   updated, now handles port 2 also !, by TD           */
/* 91-07-22   /   updated, transparent expander functions added       */
/* 92-02-13   /   TD included and reformed PULSE & SINC functions     */
/**********************************************************************/
/*
  1992-05-21  TD  1.30  The HC589 case in get_di has been moved to after 
                        the selection to bypass the siiexp card
                        
  1992-09-30  TD  1.40  Added functions                       
                          bypassSNPKrelay
                          blockSNPKkeyboard
                          setAlarmSNPK
                          getAlarmSNPK
                          
  1992-10-16  TD  1.41  Bugfix in getCode and getSNPKstatus
                        New edition# is 8
  
  1992-10-21  TD  1.42  Bugfix in setAlarmSNPK, changed %3d -> %03d
                        New edition# is 9

  1993-07-16  TD  1.50  Initial test code for; AU03 with additional 4channels,
                        PT100 card utilizing an LTC1292 with exterior muxes
                        The put_da() uses the new put_dac2().
                        The PT100_Res returns a float (!) resistance value  
                        
  1993-08-30  TD  1.51  Bugfix, put_dac2 switched val1 & val2 -> ch1 is not ch5!
                        PT100_Res ok and returns a double

*/
#include "dt08io.h"

/*
static char *debugPtr = 0x3ffb0;
static char *debugRelease = 0x3ffb1;
*/

/*
#define DASEL SSEL0
#define ADSEL SSEL1
#define DISEL SSEL2
#define DOSEL SSEL3
*/

static int da0val, da1val, da2val, da3val;

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

int rw_spi_bit(sbit)
int sbit;
  {
  int rbit;
  set_sdo(sbit);
  set_sclk(1);
  if ((*qip1 & SDI) != 0 )
      rbit=1;
    else
      rbit=0;
  set_sclk(0);    /* clock low */
  return(rbit);
  }

int rw_spi(select, sdata, port, module)
int select;
int sdata;
int port;
int module;
{
  int i;
  int rdata;
  int tsdata;
  tsdata=sdata;
  rdata= 0;
  set_sclk(0);
  set_ssel(select, 0, port, module);         /* select PI/SINC card */
  for (i=0; i< 8; i++,sdata<<=1)
  {
    rdata= (rdata<<1) | rw_spi_bit(sdata & 0x80);
  }
  set_ssel(select, 1, port, module);         /* unselect PI/SINC card */
  return (rdata);
}

int get_spi_int(select, sendval, port, module)
int select, sendval, port, module;
{
  int i;
  int val;
  val= rw_spi(select, sendval, port, module);      /* get MSByte first */
  val= (val<<8) | rw_spi(select, sendval+1, port, module);
  return (val);
}

int get_pulse(module, ch)     /* read channel 0, 1, 2 or 3 */
int module, ch;
{
  return get_pi(module, 0x20 + ch);
}

int get_pi(module, cmd)              /* send command and get 16-bits of data */
int module, cmd;
{
  int cnt;
  int select, port;
  
  select = module2select(module, &port);
  set_sclk(0);               /* set Clock low first */
  rw_spi(select, cmd, port, module);               /* send command */
  cnt= get_spi_int(select,1, port, module);
  return(cnt);
}

int get_pi_byte(module, cmd)         /* send command and get 8-bits of data */
int module, cmd;
{
  int cnt;
  int select, port;
  
  select = module2select(module, &port);
  set_sclk(0);               /* set Clock low first */
  rw_spi(select, cmd, port, module);               /* send command */
  return(rw_spi(select, 1, port, module));
}

void pi_uptime(module, days, hours, min, sec, tics, timestamp)
int module, *days, *hours, *min, *sec, *tics, *timestamp;
{
  int select, port;
  
  select = module2select(module, &port);
  set_sclk(0);               /* set Clock low first */
  rw_spi(select, 0x70, port, module);              /* send command */
  if (days) *days = rw_spi(select, 1, port, module);
  if (hours) *hours = rw_spi(select, 2, port, module);
  if (min) *min = rw_spi(select, 3, port, module);
  if (sec) *sec = rw_spi(select, 4, port, module);
  if (tics) *tics = get_spi_int(select, 5, port, module);
  if (timestamp) *timestamp = get_spi_int(select, 7, port, module);
}


void pi_measure(module, chan, pedge, nedge, pper, nper, hpul, lpul)
int module, chan; /* 0-3 */
int *pedge, *nedge, *pper, *nper, *hpul, *lpul;
{
  int select, port;
  
  select = module2select(module, &port);
  set_sclk(0);               /* set Clock low first */
  rw_spi(select, 0x30+chan, port, module);         /* send command */
  if (pedge) *pedge = get_spi_int(select, 1, port, module);
  if (nedge) *nedge = get_spi_int(select, 3, port, module);
  if (pper) *pper   = get_spi_int(select, 5, port, module);
  if (nper) *nper   = get_spi_int(select, 7, port, module);
  if (hpul) *hpul   = get_spi_int(select, 9, port, module);
  if (lpul) *lpul   = get_spi_int(select, 11, port, module);
#if 0
  printf("Channel %d: pedge=%-4d", chan, get_spi_int(select, 1, port, module));   /* get byte 0,1 */
  printf(" nedge=%-4d", get_spi_int(select, 3, port, module));  /* get byte 2,3 */
  printf(" pper=%-4d", get_spi_int(select, 5, port, module));  /* get byte 4,5 */
  printf(" nper=%-4d", get_spi_int(select, 7, port, module));  /* get byte 6,7 */
  printf(" hpul=%-4d", get_spi_int(select, 9, port, module));  /* get byte 8,9 */
  printf(" lpul=%-4d", get_spi_int(select, 11, port, module));  /* get byte 10,11 */
#endif
}

/*
!   Here comes routines specific for SINC
*/

void sinc_uptime(module, days, hours, min, sec, ms)
int module, *days, *hours, *min, *sec, *ms;
{
  int select, port;
  
  select = module2select(module, &port);
  set_sclk(0);               /* set Clock low first */
/*  printf("SINC UPtime: ");  */
  rw_spi(select, 0x70, port, module);              /* send command */

  if (days) *days   = rw_spi(select, 1, port, module);
  if (hours) *hours = rw_spi(select, 2, port, module);
  if (min) *min     = rw_spi(select, 3, port, module);
  if (sec) *sec     = rw_spi(select, 4, port, module);
  if (ms) *ms       = get_spi_int(select, 5, port, module);
/*
  printf("%d dagar ", rw_spi(select,1, port, module));
  printf("%d timmar ", rw_spi(select,2, port, module));
  printf("%d minuter ", rw_spi(select,3, port, module));
  printf("%d sekunder ", rw_spi(select,4, port, module));
  printf("%d millisek ", get_spi_int(select,5, port, module));
*/
}

unsigned int get_version(module, ver, rev, type)
int module, *ver, *rev, *type;
{
  int i;
  int select, port;
  unsigned int version;     /* nibbles VVRT , Version, Revision, Type */
  
  select = module2select(module, &port);
  set_sclk(0);               /* set Clock low first */
  rw_spi(select, 0x71, port, module);

  version= get_spi_int(select,1,port, module);
  *ver = version >> 8;
  *rev = (version & 0xff) >> 4;
  *type = version & 0x0f;
  return version;
/*
  printf("SINC version: %2x.%1x Type: %1x",
    version>>8, (version&0xff)>>4, version&0x0f);
*/
/*
  printf(" PI01 version %04x", get_spi_int(select, 1, port, module));
  return get_spi_int(select, 1, port, module);
*/
}

send_snpk(select, node, strp, port, module)
int select;
int node;
char *strp;
int port, module;
{
  rw_spi(select, (strlen(strp)+1) | 0x20, port, module);        /* command to SINC 0x2n */
  rw_spi(select, node, port, module);
  while (*strp)
    rw_spi(select,*strp++, port, module);
}

int read_snpk(select, node, dvec, port, module)
int select;
int node;
unsigned char *dvec; /* vector to put data in, max 16 bytes, length returned */
int port, module;
{
  int i, retry, len;
  int loopcnt;
  for (retry=0; retry<3; retry++)
  {
    send_snpk(select, node, "69", port, module);     /* command to node to send its buffer */
    loopcnt= 0;
    do
    {
      if(++loopcnt >= 10)      /* count loops, timeout if too many */
         break;
      rw_spi(select, 0x40, port, module);                    /* send status request to SINC */
    } while ((len=rw_spi(select, 0, port, module)) == 0);  /* wait for data arrival from SNPK */
    if (len!=0) break;
  }
  if (len==0) return(-1);
  len &= 0x0f;
  for (i=0; i<len; i++)
    dvec[i]= rw_spi(select, i+1, port, module);            /* i+1:  tell next byte number */
  return(len);
}

setCode(module, node, codeNo, code)
int module;
int node;
int codeNo, code;
{
  int select, port;
  char str[12];

  select = module2select(module, &port);
  sprintf(str, "10%03d%04d\0", codeNo, code);
/*  strcpy(str, "10");
  strcat(str, strp); */
  send_snpk(select, node, str, port, module);
}

setSafeCode(module, node, codeNo, code)
int module;
int node;
int codeNo, code;
{
  int select, port, r, tries;
  char str[12];

#define MAX_TRIES 10
  for (tries = 1; tries <= MAX_TRIES; tries++) {
    if ((r = getCode(module, node, codeNo)) == code)
      return 0;                                   /* code already set ! */
    if (r >= 0)
      break;
  }
  select = module2select(module, &port);
  for (tries = 1; tries <= MAX_TRIES; tries++) {
    sprintf(str, "10%03d%04d\0", codeNo, code);
    send_snpk(select, node, str, port, module);
    if (getCode(module, node, codeNo) == code)
      break;
  }
  return (tries <= MAX_TRIES) ? tries : -1;
}

getCode(module, node, codeNo)
int module;
int node;
int codeNo;
{
  int select, port, datalen, tmp;
  char str[12];
  unsigned char data[20];

  select = module2select(module, &port);
  sprintf(str, "96%03d\0", codeNo);
  send_snpk(select, node, str, port, module);
  ldelay(1000L);
  datalen = read_snpk(select, node, data, port, module);
  if (datalen == -1)
    return -1;
  if (data[0] == 0xff)
    return -2;
#if 0
  return data[0] << 8 | data[1]  ;
#else
  tmp = data[0] << 8 | data[1]  ;     /* 4386 */
  sprintf(data, "%04x\0", tmp);         /* 1122 */
  return atoi(data);
#endif
}

getSNPKStatus(module, node)
int module;
int node;
{
  int select, port, datalen;
  char str[12];
  unsigned char data[20];

  select = module2select(module, &port);
  sprintf(str, "61\0");
  send_snpk(select, node, str, port, module);
  ldelay(1000L);
  datalen = read_snpk(select, node, data, port, module);
  if (datalen == -1)
    return -1;
  if (data[0] == 0xff)
    return -2;
  return data[0] - 0x20;
}

/*activateSNPK(module, node, relay, tenths) */

makeSNPKrelay(module, node, relay, tenths)
int module;
int node;
int relay;
int tenths;
{
  int select, port;
  char str[12];

  select = module2select(module, &port);
  sprintf(str, "30%1d%03d\0", relay, tenths);
  send_snpk(select, node, str, port, module);
}

/* new functions as of 920930 */

bypassSNPKrelay(module, node, relay, open)
int module;
int node;
int relay;
int open;
{
  int select, port;
  char str[12];

  select = module2select(module, &port);
  sprintf(str, "34%1d%1d\0", relay, open);
  send_snpk(select, node, str, port, module);
}

blockSNPKkeyboard(module, node, block)
int module;
int node;
int block;
{
  int select, port;
  char str[12];

  select = module2select(module, &port);
  sprintf(str, "35%1d\0", block);
  send_snpk(select, node, str, port, module);
}

setAlarmSNPK(module, node, no)
int module;
int node;
int no;
{
  int select, port;
  char str[12];

  select = module2select(module, &port);
  sprintf(str, "36%03d\0", no);           /* changed %3d -> %03d, 921021 */
  send_snpk(select, node, str, port, module);
}

/*
!   -1      Node not responding
!   0       Illegal datalength
!   > 0     Ok
!
!  HEX(ret >> 16)             = last entered code
!  ((ret >> 8) & 0xff) - 0x20 = no of pass
!  (ret & 0xff) - 0x20        = no of failed tries
!  printf("SNPK%d senaste kod: %02x%02x, antal pass: %d, felaktiga koder: %d",
!    node, data[0], data[1], data[2]-0x20, data[3]-0x20);
!
*/
int getAlarmSNPK(module, node)
int module, node;
{
  int datalen;
  unsigned char data[20];
  int select, port, result;
    
  select = module2select(module, &port);
  ldelay(100L);
  send_snpk(select, node, "97", port, module);              /* tell node to prepare pass status */
  ldelay(100L);
  datalen= read_snpk(select, node, data, port, module);     /* read node buffer */
  if (datalen != 2)
  {
    if (datalen ==-1)
      return -1;    /* printf("PasReq: Node %d not responding", node);*/
    else
      return 0; /*
          printf("PasReq: Node %d datalength is %d, should be 4, data[0]=$%02x",
           node, datalen, data[0]);
           */
  }
/*    data[1] kodnummer satt av set_alarm_no() */
  return data[0] - ' ';       /* antal passager */
}

clearCode(module, node, codeNo)
int module;
int node;
int codeNo;
{
  int select, port;
  char str[12];

  select = module2select(module, &port);
  sprintf(str, "11%03d\0", codeNo);
  send_snpk(select, node, str, port, module);
}

clearAllCodes(module, node)
int module;
int node;
{
  int select, port;
  char str[12];

  select = module2select(module, &port);
  strcpy(str, "191919");
  send_snpk(select, node, str, port, module);
}

unsigned int get_avail_nodes(module)
int module;
{
  int node, datalen;
  int select, port;
  unsigned int result;
  unsigned char data[20];

  select = module2select(module, &port);
  for (node=result=0; node<16; node++) {
    datalen= read_snpk(select, node, data, port, module);
    if (datalen==1 && data[0]==0x10)    /* PTYPALIVE = 0x10 */
      result |= (1 << node);
  }
  return result;
}
     
/*
!   -1      Node not responding
!   0       Illegal datalength
!   1       Ok
*/
int snpk_uptime(module, node, days, hours, min, sec)
int module, node, *days, *hours, *min, *sec;
{
  int datalen;
  unsigned char data[20];
  int select, port;
  
  select = module2select(module, &port);
  send_snpk(select, node, "70", port, module);              /* tell node to prepare uptime */
  ldelay(1000L);
  datalen= read_snpk(select, node, data, port, module);     /* read node buffer */
  if (datalen != 4)
  {
    if (datalen ==-1)
      return -1;  /*      printf("TimReq: Node %d not responding", node); */
    else
      return 0;   /*     printf("TimReq: Node %d datalength is %d, should be 4, data[0]=$%02x",
                          node, datalen, data[0]);  */
  }
  if (days) *days = data[0];
  if (hours) *hours = data[1];
  if (min) *min   = data[2];
  if (sec) *sec   = data[3];
  return 1;
/*  
  printf("SNPK%d UPtime: ", node);
  printf("%d dagar ", data[0]);
  printf("%d timmar ", data[1]);
  printf("%d minuter ", data[2]);
  printf("%d sekunder ", data[3]);
*/  
}

/*
!   -1      Node not responding
!   0       Illegal datalength
!   > 0     Version#, Ok
!
!  Returned version value is: 
!    printf("SINC version: %2x.%1x Type: %1x",
!                version>>8, (version&0xff)>>4, version&0x0f);
*/
int snpk_version(module, node, ver, rev, type)
int module, node, *ver, *rev, *type;
{
  int datalen;
  int version;     
  unsigned char data[20];
  int select, port;

  select = module2select(module, &port);
  send_snpk(select, node, "71", port, module);              /* tell node to prepare version */
  ldelay(1000L);
  datalen= read_snpk(select, node, data, port, module);     /* read node buffer */
  if (datalen != 2)
  {
    if (datalen ==-1)
      return -1;  /*   printf("VerReq: Node %d not responding", node); */
    else
      return 0;   /* 
        printf("VerReq: Node %d datalength is %d, should be 2, data[0]=$%02x",
           node, datalen, data[0]);
           */
  }
  version = data[0] << 8;
  if (ver) *ver = data[0];

  version |= data[1];
  if (rev) *rev = data[1] >> 4;
  if (type) *type = data[1] & 0x0f;
  return version;
/*
  printf("SNPK%d version: %2x.%1x Type: %1x", node,
    data[0], data[1]>>4, data[1]&0x0f);  nibbles VVRT , Ver, Rev, Type
*/    
}

/*
!   -1      Node not responding
!   0       Illegal datalength
!   > 0     Ok
!
!  HEX(ret >> 16)             = last entered code
!  ((ret >> 8) & 0xff) - 0x20 = no of pass
!  (ret & 0xff) - 0x20        = no of failed tries
!  printf("SNPK%d senaste kod: %02x%02x, antal pass: %d, felaktiga koder: %d",
!    node, data[0], data[1], data[2]-0x20, data[3]-0x20);
!
*/
int get_snpk_pass(module, node, code, pass, fail)
int module, node, *code, *pass, *fail;
{
  int datalen;
  unsigned char data[20];
  int select, port, result;
  
  select = module2select(module, &port);
  ldelay(100L);
  send_snpk(select, node, "95", port, module);              /* tell node to prepare pass status */
  ldelay(100L);
  datalen= read_snpk(select, node, data, port, module);     /* read node buffer */
  if (datalen != 4)
  {
    if (datalen ==-1)
      return -1;    /* printf("PasReq: Node %d not responding", node);*/
    else
      return 0; /*
          printf("PasReq: Node %d datalength is %d, should be 4, data[0]=$%02x",
           node, datalen, data[0]);
           */
  }
  if (code) {
    char hexbuf[10];
    sprintf(hexbuf, "%04x\0", data[0] << 8 | data[1]);
    *code = atoi(hexbuf);
  }
  if (pass) *pass = data[2] - ' ';
  if (fail) *fail = data[3] - ' ';
  memcpy(&result, data, 4);
  return result;
/*  
  printf("SNPK%d senaste kod: %02x%02x, antal pass: %d, felaktiga koder: %d",
    node, data[0], data[1], data[2]-0x20, data[3]-0x20);
*/
}

/*****************************************************************/
/* function : get_siid(nbits)                                    */
/* description: read data clock high and low nbits times         */
/*              shift bits to right in return value              */
static int get_siid(nbits)
int nbits;
{
  int i;
  int val;
  val = 0;
/*  set_sdo(0);   */
  for (i=0; i<nbits; i++)
  {
    if ((*qip1 & SDI) != 0 )
        val= (val<< 1) | 1;
    else
        val= (val<< 1) ;
/* tell word length for ADC-module, other modules don`t care */
/*    if (i==7) set_sdo(1);   */
    set_sclk(1);
    set_sclk(0);    /* clock low */
  }
  return (val);
}

/*****************************************************************/
/* function : PT100_get_siid(nbits)                                    */
/* description: read data clock high and low nbits times         */
/*              shift bits to right in return value              */
static int PT100_get_siid(nbits, out)
int nbits, out;
{
  int i;
  int val;
  val = 0;

  set_sclk(1);		
  set_sclk(0);

  set_sclk(1);		
  set_sclk(0);

  for (i=0; i<nbits; i++)
  {
    if ((*qip1 & SDI) != 0 )
        val= (val<< 1) ;
    else
        val= (val<< 1) | 1;
    if (i >= nbits - 4) {
      set_sdo(out & 1);
      out >>= 1;
    }
    set_sclk(1);
    set_sclk(0);    /* clock low */
  }
  return (val);
}

static int AD_get_siid(nbits)
int nbits;
{
  int i;
  int val;
  val = 0;
  set_sdo(0);
  for (i=0; i<nbits; i++)
  {
    if ((*qip1 & SDI) != 0 )
        val= (val<< 1) | 1;
    else
        val= (val<< 1) ;
/* tell word length for ADC-module, other modules don`t care */
    if (i==7) set_sdo(1);
    set_sclk(1);
    set_sclk(0);    /* clock low */
  }
  return (val);
}

static void clock_data(hilo)
int hilo;
{
  set_sdo(hilo);
  set_sclk(1);
  set_sclk(0);
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

#define DA 1
#define AD 2
#define DO 3
#define DI 4

int seltest(module, code)
int module;
int code;
{
  /* do some error display */
  
  if ((*qip1 & SINT) != 0) { 
    phyioError = module;  /* code; */
    return 0;
  } else
    return 1;
}

/* defines for LTC1090 (ADC) protocol */
#define DIFF 0
#define SGL 1
#define BIP 0
#define UNI 1
#define LSBF 0
#define MSBF 1

#define LTC1290_also

#ifdef LTC1290_also
#define WL1 1         /* select 12 bits !! (LTC1290 and LTC1090) */
#define WL0 0
#else
#define WL1 0
#define WL0 1
#endif

#define OFFERT_930707

#ifndef OFFERT_930707
static void put_dac(module, select, chn, val, port)
int module;
int select; /* select line for DAC */
int chn;  /* channel number 0-3 */
int val;  /* binary value to put into DAC 0-255 */
int port; /* port 0 (1) or 1 (2) */
{
  set_ssel(select, 1, port, module);
  set_sdo(1);                 /* start condition, data high */
  set_sclk(1);                /* set clock high */
  set_sdo(0);                 /* set data low */
  set_sclk(0);                /* set clock low */
  clock_data(chn & 0x02);     /* put address A1 */
  clock_data(chn & 0x01);     /* put LSB address A0 */
  put_siido(val,8);           /* put all data bits with clock pulses */
  set_sdo(0);                 /* set data low */
  set_sclk(1);                /* stop condition, set Clock high */
  set_sdo(1);                 /* set data high */
  set_ssel(select, 0, port, module);          /* do the load to DACS */
  seltest(module, DA);
  set_ssel(select, 1, port, module);
}
#endif

static void put_dac2(module, select, chn, val1, val2, port)
int module;
int select; /* select line for DAC */
int chn;  /* channel number 0-3 */
int val1;  /* binary value to put into DAC1 0-255 */
int val2;  /* binary value to put into DAC2 0-255 */
int port; /* port 0 (1) or 1 (2) */
{
  set_ssel(select, 1, port, module);
/*  set_sdo(1);                 /* start condition, data high */
/*  set_sclk(1);                /* set clock high */
  set_sdo(0);                 /* set data low */
  set_sclk(0);                /* set clock low */
  clock_data(chn & 0x02);     /* put address A1 */
  clock_data(chn & 0x01);     /* put LSB address A0 */
  put_siido(val2, 8);           /* put all data bits with clock pulses */

  clock_data(chn & 0x02);     /* put address A1 */
  clock_data(chn & 0x01);     /* put LSB address A0 */
  put_siido(val1, 8);           /* put all data bits with clock pulses */

/*  set_sdo(0);                 /* set data low */
/*  set_sclk(1);                /* stop condition, set Clock high */
/*  set_sdo(1);                 /* set data high */

  set_ssel(select, 0, port, module);          /* do the load to DACS */
  seltest(module, DA);
  set_ssel(select, 1, port, module);
}

#if 0
static void send_dac(module, select, port)
int module;
int select;
int port;
{
  put_dac(module, select, 0, da0val, port);
  put_dac(module, select, 1, da1val, port);
  put_dac(module, select, 2, da2val, port);
  put_dac(module, select, 3, da3val, port);
}

static void put_da_bin(module, select, chn, val, port)
int module;
int select; /* select line for DAC */
int chn;  /* channel number 0-3 */
int val;  /* binary value to put into DAC 0-255 */
int port; /* port 1 or 2, holds value 0 or 1 */
{
  switch (chn)
  {
    case 0: da0val= val; break;
    case 1: da1val= val; break;
    case 2: da2val= val; break;
    case 3: da3val= val; break;
  }
  send_dac(module, select, port);
}
#endif

/*
static int da_value[10][4], currentModule, currentChn, currentSel, currentPort;
this occupied 44*4 = 176byte !
*/

static unsigned char da_value[16][8];   /* this takes only 128 byte ! */
char dummies[48];     /* so total is the same !!! */

int get_ad(module, chn)
int module, chn;
{
  int bval, select, port;
  long bvolt;

  select = module2select(module, &port);  
  set_sclk(0);               /* set Clock low first */
  set_ssel(select,0, port, module);         /* select ADC-board */
  seltest(module, AD);
  clock_data(SGL);           /* select single ended input */
  clock_data(chn & 0x01);    /* put LSB address A0 */
  clock_data(chn & 0x04);    /* put MSB address A2 */
  clock_data(chn & 0x02);    /* put address A1 */
  clock_data(UNI);           /* set unipolar input */
  clock_data(MSBF);          /* set MSB first of data */

  clock_data(WL1);           /* select word length bit1 */
  clock_data(WL0);           /* select word length bit0 */
  clock_data(0);             
  clock_data(0);             

#ifdef LTC1290_also
  clock_data(0);
  clock_data(0);
#endif

  set_ssel(select,1, port, module);
  ldelay(100);                /* wait for conversion ready */
  set_ssel(select,0, port, module);
#ifdef LTC1290_also
  bval= (AD_get_siid(12) & 0xFFF);
  bvolt = (long) bval * 10000L / 4095L;
#else
  bval= (AD_get_siid(10) & 0x3FF);
  bvolt = (long) bval * 10000L / 1023L;
#endif
  set_ssel(select,1, port, module);

  return((int)bvolt);
}

double PT100_Res(module, chn)   /* returns resistance 0 - 280 ohm */
int module, chn;
{
  int bval, select, port;
  long bvolt;
  float Ug;

  select = module2select(module, &port);  
  set_sclk(0);               /* set Clock low first */
  set_ssel(select,0, port, module);         /* select ADC-board */
  PT100_get_siid(12,                        /* and set up for measure */
            (((chn - 1) & 3) << 2) | 1);    /* channel + enable */
  set_ssel(select,1, port, module);         /* unselect ADC-board */

  tsleep(3);                                /* stabilize AD inputs */

  set_sclk(0);                              /* set Clock low first */
  set_ssel(select,0, port, module);         /* select ADC-board */
  bval= (PT100_get_siid(12, 0) & 0xFFF);    /* and disable */
  set_ssel(select,1, port, module);         /* unselect ADC-board */

  Ug = bval; Ug /= 4095.0;
  return Ug * 280.5755396;      /* Rg */    /* 10 * 39 / 139 */
}

void put_da(module, chn, mvolt)
int module;
int chn;  /* channel number 0-3 */
int mvolt;  /* mV value to put into DAC 0-10000 */
{
  int val, select, port;

  select = module2select(module, &port);
  val= mvolt*255L/10000L;     /* calculate binary value to put into DAC */
  
/*
  da_value[module - 1][chn] = val;
  put_da_bin(module, select, chn, val, port);
*/

#ifdef OFFERT_930707
  da_value[(module - 1) & 15][chn & 7] = val;
  if (chn < 4)
    put_dac2(module, select, chn, val, 
          da_value[(module - 1) & 15][chn + 4], port);
  else
    put_dac2(module, select, chn, da_value[(module - 1) & 15][chn & 3], 
          val, port);
#else
  put_dac(module, select, chn, val, port);
#endif
}

int get_di(module)
int module;
{
  int dival, bit8, select, port;

  select = module2select(module, &port);
  set_sdo(0);
  set_sclk(0);               /* set Clock low first */

/* the following two lines were moved up here for prom version 1.71 */
/* since otherwise the three lines for HC589 would not propagate */
/* through the expander */

  set_ssel(select,0, port, module);         /* select DI module */
  seltest(module, DI);

#define HC589
#ifdef HC589
  set_sdo(1);           /* latch data into 'HC589 */
  set_sdo(0);           /* parallell load */
  set_sdo(1);           /* shift mode */
#endif
 
  bit8= ((*qip1 & SDI) != 0 ); /* save value of bit 8 */
  dival= (get_siid(9) & 0xFF);
  set_sdo(1);
  set_ssel(select,1, port, module);         /* unselect DI module */
  set_sdo(0);
  if (bit8) dival |= 0x100;  /* set 9th bit */

/*
  if (*debugPtr) {
    printf("get_di: module %d, value = $%x\n", module, dival);
    *debugRelease = 0;
    while (*debugRelease == 0)
      ;
  }
*/
  return(dival);
}

int get_di_SR(module)
int module;
{
  int dival, bit8, select, port;

  select = module2select(module, &port);
  set_sdo(0);
  set_sclk(0);               /* set Clock low first */

  set_ssel(select,0, port, module);         /* select DI module */
  seltest(module, DI);
  bit8= ((*qip1 & SDI) != 0 ); /* save value of bit 8 */
  dival= (get_siid(9) & 0xFF);
  set_sdo(1);                               /* reset S-R latch */
  set_ssel(select,1, port, module);         /* unselect DI module */
  set_sdo(0);
  if (bit8) dival |= 0x100;  /* set 9th bit */
  return(dival);
}

void put_do(module, doval)
int module, doval;
{
  int select, port;

/*
  if (*debugPtr) {
    printf("put_do: module %d, value = $%x\n", module, doval);
    *debugRelease = 0;
    while (*debugRelease == 0)
      ;
  }
*/
  select = module2select(module, &port);
  set_sclk(0);               /* set Clock low first */
  set_ssel(select,0, port, module);         /* select DO module */
  seltest(module, DO);
  put_siido(doval,8);
  set_ssel(select,1, port, module);         /* unselect DI module */
}


int get_batt_voltage()
{
  int bval, port;
  long bvolt;
  port = 0;
  *qsop1= SCLK;    /* set Clock low first */
  *qsop1= SSEL8;   /* SSEL8 low, select onboard ADC */
#if 1
  *qrop1 = SDO;   /* set data out high */
  bval= (get_siid(10) & 0xFF);   /* to dummy bits first then 8 data bits */
#else
  bval= (AD_get_siid(10) & 0xFF);   /* to dummy bits first then 8 data bits */
#endif
  *qrop1= SSEL8;   /* SSEL8 high */
  bvolt = (long) bval * 5000L / 255L; /* millivolt */
  return((int)bvolt);
}

int opto_power()
{
  return !(*qip2 & OVCCOK);         /* 1 == Opto power connected */
}
