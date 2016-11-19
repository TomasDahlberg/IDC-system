/* pointers to QUART */
unsigned char *qip1= 0x34000d;        /* input port 1 */
unsigned char *qip2= 0x34001d;        /* input port 2 */
unsigned char *qsop1= 0x34000e;       /* set bit (low) in output port 1 */
unsigned char *qrop1= 0x34000f;       /* reset bit (high) in output port 1 */
unsigned char *qsop2= 0x34001e;       /* set bit (low) in output port 2 */
unsigned char *qrop2= 0x34001f;       /* reset bit (high) in output port 2 */

/* bits in QUART regs */

#define RTSA 0x01          /* in op1 */
#define RTSB 0x02          /* in op1 */
#define RTSC 0x01          /* in op2 */
#define RTSD 0x02          /* in op2 */


int rtsA(level)
int level;
{
  if (level)
    *qrop1 = RTSA;
  else
    *qsop1 = RTSA;
}

int rtsB(level)
int level;
{
  if (level)
    *qrop1 = RTSB;
  else
    *qsop1 = RTSB;
}

int rtsC(level)
int level;
{
  if (level)
    *qrop2 = RTSC;
  else
    *qsop2 = RTSC;
}

int rtsD(level)
int level;
{
  if (level)
    *qrop2 = RTSD;
  else
    *qsop2 = RTSD;
}


