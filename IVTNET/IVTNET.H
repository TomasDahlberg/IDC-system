/* ivtnet.h  1992-09-23 TD,  version 1.1 */
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
! ivtnet.h
! Copyright (C) 1991, IVT Electronic AB.
*/

/*
!     ivtnet.h is part of the IVTnet product
!
!     File: ivtnet.h
!     
!     Contains definitions for messages sent on IVTnet
!
!     History     
!     Date        Revision Who  What
!     
!      3-sep-1991     1.0  TD   Start of coding
!     23-sep-1992     1.1  TD   Added memory manipulation support functions
!
*/

/*
*   defines messages types
*/

#define GET_VAR             0x01
#define PUT_VAR             0x02
#define ALARM_NO            0x03
#define ALARM_TEXT_1        0x04        /* get alarm text */
#define ALARM_TEXT_2        0x05        /* ack received alarm text */
#define CONFIRM_ALARM       0x06
#define SET_TIME            0x07
#define SET_HOST            0x08
#define KEY_DISPLAY         0x09
#define GETIDX_VAR_1        0x0a        /* index from PC */
#define GET_UPDATED_1       0x0b
#define SET_REMOTE          0x0c
#define REMOTE_DATA         0x0d
#define GET_VARIDX          0x0e
#define GET_CAL             0x0f
#define SET_CAL             0x10
#define GETNIDX_VAR_1       0x11        /* up to 10 GETIDX_VAR requests ! */
#define VERSION             0x12
#define GETIDX_VAR_2        0x13        /* index from PC */
#define GET_UPDATED_2       0x14
#define GETNIDX_VAR_2       0x15        /* up to 10 GETIDX_VAR requests ! */
#define CLEAR_ALARM         0x16
#define GET_STAT_VAR        0x17
#define ALLOCATE_MEM        0x18
#define GET_MEM             0x19
#define PUT_MEM             0x1a
#define LOAD_PROGRAM        0x1b        /* does F$Verify */
#define REBOOT              0x1c
#define NEW                 0x1d

#define REGISTRATE          0x40
#define REPLY_MASK          0x00
#define REQUEST_MASK        0x80

/*
!   error types
*/

#define NET_NOERROR         0x00
#define NET_NOSUCHVAR       0x01
#define NET_ILLEGALTYPE     0x02
#define NET_CANCEL          0x03        /* i.e. stop spawned screen proc */
#define NET_NOBINDING       0x04        /* cannot bind for vars/alarm dm */
#define NET_NOALARMS        0x05
#define NET_BUSY            0x06        /* cannot spawn, one shell only ! */
#define NET_SPAWN_ERROR     0x07        /* some error !! */
#define NET_TIMEOUT         0x08
#define NET_LOGOUT          0x09        /* connection terminated ok */
#define NET_CTRL_C          0x0a        /* send ctrl-c */
#define NET_NOSUCHALARM     0x0b
#define NET_NOMOREALARMS    0x0c        /* just a warning for no more requests*/
#define NET_REGISTRATE      0x0d
#define NET_NOSUCHNODE      0x0e
#define NET_WRONG_CRC       0x0f

struct _message
{
  unsigned char message_type;    /* type of request */
  unsigned char error;
  union {
    struct _var
    {
/* name, zero, [type, value] */

      char varName[50];           /* zero string ! */
      double varValue;                /* var holding long int or double */
      char valueType;                 /* from meta.h */
    } varRequest;
    struct _alarmNo                   /* used for ALARM_NO & SEND_ALARM */
    {
      long noOfAlarms;
      unsigned long activeABCD;       /* no of none confirmed active/noactive */
      unsigned long confirmedABCD;    /* no of active and confirmed */
      unsigned char pcSum;
    } alarmNoRequest;
    struct _alarmText
    {
      unsigned short int alarmNo, serialNo, status;
      long dtime;
      char text[80];
    } alarmTextRequest;               /* 90 byte */
    struct _ackAlarm
    {
      unsigned char PCno, abcdMask[4];
    } ackAlarm;
    struct _confirm
    {
      unsigned short int alarmNo, serialNo, status;
    } confirmAlarm;
    struct _setTime
    {
      struct sgtbuf tm;
      unsigned char abcdMask[4];
    } setTime;            /* 10 bytes */
    struct _sethost
    {
      char ok;                        /* just for reply */
    } setHost;
    union _keyDisplay
    {
      struct _display
      {
        unsigned char buf[82];
/*
        unsigned char map[10];
        unsigned char buf[40];
        unsigned char flashLed, currentLed, status, x, y;
*/
      } display;
      struct _key
      {
        unsigned char keyCode, keyDown, keyWasDown;
      } key;
    } keyDisplay;
    struct _getNIdxVar {
      unsigned char n;
      char buff[340];
    } getNIdxVar;
    
    struct _getIdxVar
    {
/* PCidx, name      or   PCidx type value(4/8) */
      short int PCidx;
/*
      char buf[32];
*/    
  
      char name[32];
      
/*      double varValue;     */           /* var holding long int or double */
/*      char valueType;      */           /* from meta.h */
    } getIdxVar;
    struct _getUpdated
    {                               /* small messages for now ! */
      unsigned char buf[5]; 
/*
     BUG!! long must be word-align -> paded with extra char !!!
*/
/*      unsigned char PCidx_and_type; bit 7 holds 1 if the 'value' is float */ 
/*      long int value;                   */
    } getUpdated[11];
    struct _remoteData
    {
      unsigned char size;
      unsigned char buf[50];
    } remoteData;
    struct _getVarIdx
    {
      unsigned char name[32];      /* name in request, meta idx in reply */
    } getVarIdx;
    union {
      struct _bits      /* for GET_CAL and SET_CAL */
      {
        unsigned char bitPack[50];   /* 8*50 = 400 bits */
      } bits;
      char name[32];
    } cal;
    char version[5];     /* Version 1.664   3ffd8   5tkn */
    struct _stat {
      unsigned char buffS, item;
      char varName[32];     /* on return, type and value 1+4 / 1+8 bytes */
    } stat;
    struct _mem {
      long address;
      unsigned char size;
      char buff[256];
    } mem;
  } mix;
};
