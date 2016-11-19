/*
!   server specific information
*/
struct _node
{
  unsigned char failCount;
  unsigned char full;               /*  added 920306 */  
  short size;
  unsigned short int rCount, sCount;
  unsigned short noOfAlarms;       /* come, gone, disable, confirm -> 4*100 */

  unsigned char pollCount;        /* incremented for each poll */
  unsigned char failFlag;
/*  added 920306 */
/*  unsigned char A,B,C,D,cA,cB,cC,cD;  */
  unsigned long activeABCD;
  unsigned long confirmedABCD;
  unsigned short version;
  
  struct {
    char command;   /* last command */
    char fromNode;
    char error;     /* last error */
  } rec;            /* information that node received */
  struct {
    char command;   /* last command */
    char toNode;
    char error;     /* last error */
  } trans;          /* information that node transmittet */
  long recTid, transTid;
};    /* size = 26 * MAX_NODES */

struct _ivtnet {
  char rBuf[256], sBuf[512];  /* changed temp. 920130 256->512 */
  struct _node nodeMap[256];     /* 24 * 256 = 6144 */
  unsigned char map2Node[256];
  int nextFree_inMap2Node;
  int outerMostCounter;
  int outerCounter;
};    /* total 256+512+6656+256+12= 7692 ($1e0c) byte */ 

