/* phyio.h  1991-08-03 TD,  version 1.1 */
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
! phyio.h 
! Copyright (C) 1991, IVT Electronic AB.
*/


/*
!   names of each key
*/
#define KEY_PLUS_MINUS	10
#define KEY_POINT 	11
#define KEY_UP  	12
#define KEY_DOWN	13
#define KEY_HELP	14
#define KEY_ENTER	15
#define KEY_RIGHT	16
#define KEY_LEFT	17
#define KEY_CHANGE 	18
#define KEY_ALARM	19

#define NO_KEY          -1
#define TIMEOUT         -2

int lcdwr();
void lcdwrite();
void lcdpositRel();
void lcdputs();
void lcdinit();
void lcdprintf();
void lcdhome();
void lcdpos();
void led();
void flashLed();
int keyDown();
int key();
int getKey();
void lcddefine();
int lcdcld();
int lcdcld_through_cache();
int lcdsetWrap();
int lcdsetCache();
int lcdsetCacheCursor();
void lcdcursorOn();
void lcdcursorOff();
void updateLed();
int get_ad();
void put_da();
int get_di();
void put_do();

