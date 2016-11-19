/*
!     Puls_2  filtrering....
!     Frekvens  + filtrering ....
!
! while (1) {
    getTime();
    setUp();
    scanAllIO(); 
  }
!
*/

struct _channel {
  struct _channel *next;
/*
  char *varPtr, *durationPtr, *filterPtr, *lockPtr;
  unsigned char varType, durationType, filterType;
*/
  char *varPtr, *lockPtr;
  unsigned char varType;
  double durValue, filtValue;
  
  unsigned char filterFknType;
  unsigned char channel, flag;
  unsigned short save;
} *channelPtr;  /* size 28 bytes */

#define MAX_MODULE 24
struct _module {        /* size 10 bytes */
  struct _channel *ch;
  unsigned short save;
  unsigned char cardIOtype;
  unsigned char module;       /* may be removed */
  unsigned char flag;
  unsigned char counter;
} modPtr[MAX_MODULE];


#define INT_TYPE 7
#define FLOAT_TYPE 8

#define DIGIN_1   0x01
#define DIGOUT_1  0x02
#define TEMP_1    0x03
#define ANAIN_1   0x04
#define ANAOUT_1  0x05
#define PULS_2    0x06

#define Ni1000LG  0x01
#define Ni1000DIN 0x02
#define Pt1000    0x03
#define Pt100_150 0x04
#define Pt100_500 0x05
#define Staefa_PTC_150  0x06

#define NO_FILTER 0x00
main(argc, argv)
int argc;
char *argv[];
{
  int i, j, q = 0, no = 0, m, typ, nom;
  double f[128];
  
  typ = atoi(argv[1]);
  nom = atoi(argv[2]);
  printf("Type %d, (DIGIN=1,DIGOUT=2,TEMP=3)\n", typ);
  printf("%d modules\n", nom);
  initphyio();
  init();
  for (i = 1; i <= nom; i ++) {
    if (nom > 8 && i == 8)        /* if expander, skip exp-card slot */
      continue;
    if (nom > 16 && i == 16)      /* if another exp, skip its exp-card slot */
      continue;
    for (j = 1; j <= 8; j++, q++) {
      insert(i, j, typ /* TEMP_1 */, Ni1000LG, 
                      FLOAT_TYPE, &f[q], 
                      INT_TYPE, &no, INT_TYPE, &no, &no);
    }
  }
  m = 1;
  for (j = 1; j < 4; j++) {
    long t1, t2;
    m = m * 10;
    t1 = time(0);
    for (i = 0; i < m; i++)
      scanAllModules();
    t2 = time(0);
    printf("%d scan loops took %d seconds\n", m, t2 - t1);
  }
}

update(mptr, module, channel, durValue, filtValue)
char *mptr;
int module, channel;
double durValue, filtValue;
/* mptr not used ... */
{
  struct _channel *channelPtr, *ch;
  
  ch = modPtr[module].ch;    

  while (ch && (ch = (channelPtr = ch)->next, channelPtr)) {
    if (channel == channelPtr->channel) {
      channelPtr->durValue  = durValue;
      channelPtr->filtValue = filtValue;
      break;
    }
  }
}

/*
insert(module, channel, cardIOtype, filterFknType, varType, varPtr, 
          durType, durPtr, filtType, filtPtr,
*/
insert(module, channel, cardIOtype, filterFknType, varType, varPtr, 
          lockPtr)
int module, channel, cardIOtype, varType;
char *varPtr, *lockPtr;          
int filterFknType;
{
  struct _channel *tmp;
  
  tmp = (struct _channel *) malloc(sizeof(struct _channel));
/*
    insert 
*/
  tmp->channel = channel;
  tmp->varType = varType;
/*
  tmp->durationType = durType;
  tmp->filterType = filtType;
*/
  tmp->filterFknType = filterFknType;
  tmp->varPtr = varPtr;
/*
  tmp->durationPtr = durPtr;
  tmp->filterPtr = filtPtr;
*/
  tmp->lockPtr = lockPtr;
  tmp->save = 0;
  tmp->flag = 0;
/*  tmp->next = (struct _channel *) 0;    */

  tmp->next = modPtr[module].ch;

  modPtr[module].ch = tmp;
  modPtr[module].cardIOtype = cardIOtype;  
  modPtr[module].module = module;
  modPtr[module].save = 0;
}

static long cnv2Int(typ, ptr)
int typ;
char *ptr;
{
  if (typ == INT_TYPE)
    return *((int *) ptr);
  else
    return *((double *) ptr);
}

static double cnv2Float(typ, ptr)
int typ;
char *ptr;
{
  if (typ == INT_TYPE)
    return *((int *) ptr);
  else
    return *((double *) ptr);
}

static cnvfromInt(source, typ, ptr)
long source;
int typ;
char *ptr;
{
  if (typ == INT_TYPE)
    *((int *) ptr) = source;
  else
    *((double *) ptr) = (double) source;
}

static cnvfromFloat(source, typ, ptr)
double source;
int typ;
char *ptr;
{
  if (typ == INT_TYPE)
    *((int *) ptr) = (int) source;
  else
    *((double *) ptr) = source;
}

init()
{
  int module;
  for (module = 0; module < MAX_MODULE; module++) {
    modPtr[module].ch = (struct _channel *) 0;
    modPtr[module].save = 0;
    modPtr[module].counter = 0;
    modPtr[module].flag = 0;
  }
}

scanOneModule(module)
int module;
{
  if (modPtr[module - 1].ch)
    scanModule(&modPtr[module - 1]);
}

scanAllModules()
{
  int module;
  struct _module *mptr;
  for (mptr = &modPtr[0], module = 0; module < MAX_MODULE; module++, mptr++)
    if (mptr->ch)
      scanModule(mptr);
}

/*
!   a fast running conversion from Ni1000 resistance to temperature
*/
static double res[] = {
        790.882, 851.151, 913.484, 977.994,
        1044.79, 1113.99, 1185.71, 1260.06,
        1337.15, 1417.09, 1500.01, 1586,
        1675.19, 1767.68, 1863.6, 1963.05
};

double delta[] = { 0.248884, 0.240643, 0.232522, 0.224564,
                   0.216763, 0.209147, 0.201748, 0.194578,
                   0.187641, 0.180897, 0.174439, 0.16818,
                   0.16218, 0.15638, 0.15083
};

#define RES 15

double Ni1000LG2temp(r)
double r;
{
  int i, a, b;
  double T1;
  double q;

  if (r < res[0])
    return -50;
  else if (r > res[15])
    return 175;

  b = 1;
  while (r > res[b])
    b++;
  a = b - 1;
  q = (r - res[a]) * delta[a];
  T1 = -50 + a * RES + q;
  if (q > 2.25 && q < 12.75)
    T1 += 0.06;

  return T1;
}

scanModule(mptr)
struct _module *mptr;
{
  int i, module, channel, ivalue = 0, ivalue2;
  struct _channel *channelPtr, *ch;
  double fvalue, r;
/*
!   scan
*/
  module = mptr->module;
  if (mptr->cardIOtype == DIGIN_1)
    mptr->save = get_di(module);

  ch = mptr->ch;
  while (ch && (ch = (channelPtr = ch)->next, channelPtr)) {
    if (*channelPtr->lockPtr)
      continue;

    channel = channelPtr->channel;
    switch (mptr->cardIOtype) {
      case DIGIN_1:
        if (channel)
          ivalue = (mptr->save & (1 << (channel - 1))) ? 1 : 0;
        else
          ivalue = mptr->save;
        cnvfromInt(ivalue, channelPtr->varType, channelPtr->varPtr);
        break;
      case DIGOUT_1:
        ivalue2 = cnv2Int(channelPtr->varType, channelPtr->varPtr);
        if (channel) {
          if (ivalue2)
            ivalue |= (1 << (channel - 1));
          else
            ivalue &= ~(1 << (channel - 1));
        } else {
          ivalue = ivalue2;
        }
        break;
      case ANAIN_1:
        ivalue = get_ad(module, channel - 1);
        if (ivalue == channelPtr->save)
          ;
        else {
          fvalue = ((float) ivalue) / 1000.0;
          cnvfromFloat(fvalue, channelPtr->varType, channelPtr->varPtr);
          channelPtr->save = ivalue;
	}
        break;
      case ANAOUT_1:
        fvalue = cnv2Float(channelPtr->varType, channelPtr->varPtr);
        if (fvalue >= 0 && fvalue <= 10) {
          ivalue = fvalue * 1000.0;
          if ((ivalue != channelPtr->save) || (channelPtr->flag == 0)) {
            put_da(module, channel - 1, channelPtr->save = ivalue);
            channelPtr->flag = 16;   /* another 16 counts */
          } else
            channelPtr->flag --;
        }
        break;
      case PULS_2:
        ivalue = get_pulse(module, channel - 1) + 
              cnv2Int(channelPtr->varType, channelPtr->varPtr);
        cnvfromInt(ivalue, channelPtr->varType, channelPtr->varPtr);
        break;
      case TEMP_1:
        ivalue = get_ad(module, channel - 1);
        if (ivalue == channelPtr->save)
          ;
        else {
          channelPtr->save = ivalue;      /* if not error */
          switch (channelPtr->filterFknType) {
            case Ni1000LG:
              
              r = 1100.0;
              fvalue = Ni1000LG2temp(r);

              break;
            case Ni1000DIN:
              break;
            case Pt1000:
              break;
            case Pt100_150:
              break;
            case Pt100_500:
              break;
            case Staefa_PTC_150:
              break;
            case NO_FILTER:
              break;
          }   /* end of switch filter function */
        }   /* end of if changed input value */
        break;
      }   /* end of switch */
  } /* end of while any more channel */
  if (mptr->cardIOtype == DIGOUT_1) {
    if ((ivalue != mptr->save) || (mptr->counter == 0)) {
      put_do(module, mptr->save = ivalue);
      mptr->counter = 16; /* another 16 scans */
    } else 
      mptr->counter --;
  }
}
