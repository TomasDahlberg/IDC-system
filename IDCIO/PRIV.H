/* priv.h  1992-06-17 TD,  version 1.0 */
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
! priv.h
! Copyright (C) 1992, IVT Electronic AB.
*/

#define PRIV_M_READ_ALARM     0x0001
#define PRIV_M_ENTER          0x0002
#define PRIV_M_ENTER_KURVA    0x0004
#define PRIV_M_ENTER_CALENDAR 0x0008
#define PRIV_M_CONFIRM_ALARM  0x0010
#define PRIV_M_DISABLE_ALARM  0x0020
#define PRIV_M_SET_TIME       0x0040
#define PRIV_M_VAR_LIST       0x8000
#define PRIV_M_ALL            0x7fffffff


#define PRIV_M_ALL_ENTER      PRIV_M_ENTER | PRIV_M_ENTER_KURVA | PRIV_M_ENTER_CALENDAR
#define PRIV_M_ALL_ALARM      PRIV_M_READ_ALARM | PRIV_M_CONFIRM_ALARM | PRIV_M_DISABLE_ALARM

#define PRIV_M_OLD_LEVEL_1  0
#define PRIV_M_OLD_LEVEL_2  PRIV_M_READ_ALARM
#define PRIV_M_OLD_LEVEL_3  PRIV_M_OLD_LEVEL_2 | PRIV_M_ALL_ENTER
#define PRIV_M_OLD_LEVEL_4  PRIV_M_OLD_LEVEL_3 | PRIV_M_ALL_ALARM | PRIV_M_SET_TIME
#define PRIV_M_OLD_LEVEL_5  PRIV_M_ALL


