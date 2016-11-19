/* idcio.h  1992-09-16 TD,  version 1.2 */
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
! idcio.h 
! Copyright (C) 1991,1992 IVT Electronic AB.
*/

/*
!   history:
!   date       by  rev  ed#  what
!   ---------- --- ---  ---  ------------------------------------
!   1991-??-?? td  1.00   1  initial coding
!
!     91-08-05 td  1.10      
!
!     92-09-16 td  1.20   x  changed the size of struct _pid, 
!                            removed obsolete fields
*/


struct _dtime {
  long time, date, tick;
};


/* pid definitions */

/*
struct _pid 
{
  struct _dtime previous;
  double e_n_2, e_n_1, e_sum, oldout; 
  long e_n_2_dms, e_n_1_dms;
};
*/

struct _pid                 /* changed 920916 */
{
  struct {
    long date, time;
  } previous;
  double e_n_2, e_n_1;
};    /* 24 * 32 = 768 byte */

struct _glitch {
/*
  time_t abs;
  long rel;
  int  day;
  int  includeFlag;
*/

  time_t prevTime;
  int prevState;
};

#define _GLITCH_INCLUDED_ABS  1
#define _GLITCH_INCLUDED_REL  2
#define _GLITCH_INCLUDED_DAY  4

/* definition of meta module */
/*#define _REMOTE_MASK 0x8000 */
#define _REMOTE_MASK 0xFF80
#define _ALIAS_MASK 0x0040

struct _remoteS {
  long timeStamp;
};
struct _metaEntry {
  unsigned short nameOffset,
        size,
        offset,
        lockOffset,
        type;
};

/* from pid.c */
double pid1();
double pid2();
double pid3();
double pid4();
double pid5();
double pid6();
double pid7();
double pid8();

double pid();

/* from runtime_scan.c */
double Ni1000();
double ANAIN_1();
double ANAOUT_1();
int DIGIN_1();
int DIGOUT_1();
int color();
int checkLevel();
int checkGlitchRaise();
int checkGlitchFall();
int checkCalendarRaise();
int checkCalendarFall();

double Pt1000();
double Pt100_150();
double Pt100_500();
int count();

double Ni1000LG();
double Ni1000DIN();
double TEMP_1();
double Staefa_PTC_150();
double filterFkn();

int PULS_1();

int battLow();
double kurva();
int tvpump();
int metaSize();

/* from mstime.c */
long deltatime();

/* from redday.c */
int colour_of_day();

/* from datamodule.c */
char *linkDataModule();
char *createDataModule();

/* from findmeta.c */

char *metaValue();
int metaId();
int metaType();
char *metaLock();
char *metaName();

/* from trap_screen.c */

int display();
int oldEnter();
int editVariable();
int edit();
int showVariables();

int /* char * */ swdctime();                 /* hmmm, change to int ? */
int xxxswdctime();
int readAlarm();
int alarm_active();

/* from counter.c */
long COUNTER_1();

/* from enter.c */

double enter();
int enter_kurva();
int set_time();

/* from timer.c */

long getRelTime();
int  setTime();
int  account();
int  isTimerInit();
int  timerCancel();
int  timerInit();
int  timerReady();
int  getTime();
int  firstFreeTimer();

#define MIN_LEVEL_ENTER             3
#define MIN_LEVEL_SET_TIME          4
#define MIN_LEVEL_ENTER_CALENDAR    3
#define MIN_LEVEL_ENTER_KURVA       3

#define MIN_LEVEL_READ_ALARM        2
#define MIN_LEVEL_CONFIRM_ALARM     4
#define MIN_LEVEL_DISABLE_ALARM     4


/*
#define NAME_OF_VAR_MODULE    "VARS"
#define NAME_OF_META_MODULE   "METAVAR"
*/
