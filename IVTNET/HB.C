#define HBLED 0x04         /* in op2 */
unsigned char *qsop2= 0x34001e;       /* set bit (low) in output port 2 */
unsigned char *qrop2= 0x34001f;       /* reset bit (high) in output port 2 */

    {
      static char prev;
      if (prev)
        *qsop2 = HBLED;
      else
        *qrop2 = HBLED;
      prev ^= 1;
    }


