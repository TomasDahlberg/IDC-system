#include <stdio.h>

extern struct { char *name; int *ids; } scr[];
extern int noOfScreens;
/*
! 	this is part of the driver
*/

/* Win3208:
V�lj duc
L�s in metamod.<ducnr>
L�nka Duc<ducnr>.DLL
loop {
  Kontrollerar vilka vars som anv�nds i aktuell bild
  H�mtar dessa variabler
  id = Dll-anrop(id);
  Uppdatera bilden (text och lysdioder)
  Kontrollera vilka av aktuella variabler som har �ndrats, skicka dessa till duc
}

Entries:

	int DriverGetStartPicId(void);
	int Driver(int PicId);
	int DriverGetVars(int PicId);

*/

#if 0
struct _dcb {
  char picture[2][40];
  int Red, Green, Yellow;	/* 0=off, 1=on, 2=flash */

} *dcb;

id = driver(id)
#endif

dumpScr()
{
  int i, *pek;
  for (i = 0; i < noOfScreens; i++) {
    printf("\nname %s\n", scr[i].name);
    pek = scr[i].ids;
    while (*pek != -1) {
      printf("%d, ", *pek++);
    }
  }
}

/*
!	Vid start skapas en map[].
!	Output.c kan ju ber�tta vilka id anv�nds f�r ett screen-namn
!	men vi beh�ver vilka som anv�nds f�r ett picId.
!	Beh�ver map[picId] -> i     i programmet ovan.
!	Algoritm: 
!		for each entry in screenTable
!			get screen name
!			search for it in scr[i].name
!			if hit
!				map[entry] = i;
!
!	S�kalgoritmen blir d� (i C):
!		int *pek;
!		pek = scr[map[picId]].ids;
!		while (*pek != -1) {
!			printf("%d, ", *pek++);
!		}
!
*/
short int map[1000];	/* 2kB ! */

typedef void (*PFI)();

extern struct {
  short left, right, up, down, help;
  PFI fcn;
  char *name;
} screens[];

int doMap()
{
  int i = 0, j = 0, cmp;
  while (screens[i].name) {
	if (i >= 1000) 
		break;
	map[i] = -1;			/* if it doesn't exist */
	for ( ; j < noOfScreens; j ++) {
	  cmp = strcmp(screens[i].name, scr[j].name);
	  if (cmp == 0) {
		map[i] = j++;
		break;
	  } else if (cmp < 0) 
		break;
	}
	i ++;
  }
}

int printTags(int picId)	/* quite easy now */
{
  int *pek;
  extern char *metaMod;
  extern char *metaName(char *, int);

  if (map[picId] < 0)
    return 0;
  pek = scr[map[picId]].ids;
  while (*pek != -1) {
      printf("%d, %s\n", *pek, metaName(metaMod, *pek));
      pek++;
  }
}
