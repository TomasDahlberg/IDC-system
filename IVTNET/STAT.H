
/* new version of stat handle */
#ifdef NEW_STAT_AS_OF_931021
static struct _metaStatBuff {
  long int /* short changed 920330 ! */ intervall;   /* 2*30*8 = 480byte extra*/
  unsigned long sample;
  char buff, duc, tries, dummy;
  char var[32];
};

static struct _metaStatS {
  char name[32];
  struct _metaStatBuff rows[1 /* ITEM_MAX */ ];
};    /* BUFF_MAX */

static struct _metaStatModule {
  unsigned char noOfBuffs;			/* used to be 8 */
  unsigned char noOfItemsPerBuff;		/* used to be 30 */
  unsigned char metaStatBuff[1];
};
#else
/* old */
#define BUFF_MAX 8
#define ITEM_MAX 30
static struct _metaStatBuff {
  long int /* short changed 920330 ! */ intervall;   /* 2*30*8 = 480byte extra*/
  unsigned long sample;
  char buff, duc;
  char var[32];
};

static struct _metaStatS {
  char name[32];
  struct _metaStatBuff rows[ITEM_MAX];
};    /* BUFF_MAX */
#endif

static struct _statBuff {                 /* size = 10 bytes */
  unsigned char bufItm;
  unsigned char Itm;
  unsigned long sampleTime;
  float value;
};

#define METASTAT_ADDRESS  (struct _metaStatS *) 0x003c000
#define SAVE_STAT_ADDRESS (struct _statBuff *) 0x0040000
#define SAVE_IDX_ADDRESS  (short int *) 0x003ffc0
#define READ_IDX_ADDRESS  (short int *) 0x003ffc2

#define READ_IDX_ADDRESS_PC0  (short int *) 0x003ffc2
#define READ_IDX_ADDRESS_PC1  (short int *) 0x003ffc4
/*
short int *readIdx_PC0 = READ_IDX_ADDRESS_PC0;
short int *readIdx_PC1 = READ_IDX_ADDRESS_PC1;
*/

