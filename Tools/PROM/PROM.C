#include <stdio.h>
#include <module.h>

#define PROM_START            (unsigned short int *) 0x00380000
#define PROM_SIZE_IN_WORDS    0x00040000

struct {
  char name[33];
  char rev;
} arr[512];   /* 34 * 256 = 8704 byte */
int apek = 0;

char *name(m)
struct modhcom *m;
{
  static char n[32];
  strncpy(n, ((char *) m) + m->_mname, 30);
  n[31] = 0;
  return n;
}

int c(m)
struct modhcom *m;
{
  return *(long *) ((char *) m + m->_msize - 4);
}

main(a, b)
int a;
char **b;
{
  unsigned short int *hp, *p = PROM_START;
  int s = PROM_SIZE_IN_WORDS;
  int i, hsum, acc, h, tkn;
  struct modhcom *m;
  
  if (a == 2 && !strcmp(b[1], "-s")) {
    char buff[64];
    strncpy(buff, (char *) 0x3fffd0, 48);
    buff[48] = 0;
    printf("%s\n", buff);
    printf("Calculating crc..."); fflush(stdout);
    acc = -1;
    crc((char *) PROM_START, (unsigned) (PROM_SIZE_IN_WORDS << 1), &acc);
    crc((char *) 0, (unsigned) 0, &acc);
    acc = ~acc;
    printf("[ok]\n");
    printf("24bit CRC for entire EPROM is %06x\n", acc);
    printf("Calculating check sum..."); fflush(stdout);
    p = PROM_START;
    hsum = 0;
    for (i = 0; i < s; i++, p++) {
      hsum += (*p & 255) + (*p >> 8);
    }
    hsum &= 0xffff;
    printf("[ok]\n");
    printf("Check sum is %04x\n", hsum);
    
    printf("Calculating check sum..."); fflush(stdout);
    hsum = checksum(PROM_START, PROM_SIZE_IN_WORDS << 1);
    printf("[ok]\n");
    printf("Check sum is %04x\n", hsum);

    exit(1);
  }
  printf("Address  Size     Rev Ed# CRC     Module name\n");
  printf("=============================================\n");
  for (i = 0; i < s; i++, p++) {
    if (*p != 0x4afc)
      continue;
    for (hp = p, hsum = h = 0; h < 24; h++) {
      hsum ^= *hp++;
    }
    if (hsum != 0xffff) 
      continue;

    m = (struct modhcom *) p;
    acc = -1;
    crc(m, m->_msize, &acc);
    acc &= 0x00ffffff;
    if (acc != CRCCON)
      continue;


/*    p += m->_msize / 2;     /* advance p pointer */
/*    p--;	*/

/* see if same name already exists */

    tkn = ' ';
    for (h = 0; h < apek; h++) {
      if (strcmp(arr[h].name, name(m)))
        continue;
      if (arr[h].rev < (m->_mattrev & 255)) {
        tkn = '+';
        arr[h].rev = m->_mattrev;
      } else if (arr[h].rev > (m->_mattrev & 255))
        tkn = '-';
      else
        tkn = '=';
      break;
    }
    if (h >= apek) {
      strcpy(arr[apek].name, name(m));
      arr[apek++].rev = m->_mattrev & 255;
    } 

    printf("%08x %8d %3d %3d %06x %c%s\n", m, m->_msize, m->_mattrev & 255, 
          m->_medit, c(m), tkn, name(m));
    
  }
}

#asm
*    hsum = checksum(PROM_START, PROM_SIZE_IN_WORDS << 1);
checksum:
	move.l	d0,a0		a0 is pointer to EPROM
	clr.l	d0		d0 is accumulator
	clr.l	d2		d2 is byte contents
loop	move.b	(a0)+,d2
	add.w	d2,d0
	subq.l	#1,d1
	bne.s	loop
	rts
#endasm
