/**************************************************************************/
/* File:        DTPROTO.C      Type: I  (mainProgram/Module/Include)      */
/* By:          Dan Axelsson                                              */
/* Description:	Protocol routines common to PC program and DT08 program   */
/*              SENDMOD and RECMOD respectively                           */
/*              conditional compiling, DT08 must be defined for DT08      */
/* Use:         #include "dtproto.h"                                      */
/*              #include "dtproto.c"                                      */
/* ---------------------  History  -------------------------------------  */
/*   Date     /   What                                                    */
/* ...................................................................... */
/* 91-03-16   /   started                                                 */
/* 91-09-15   /   updated                                                 */
/**************************************************************************/

/* returns length of message if no error, <0 if error */
/*   ER_TIMOUT: timeout, ER_READ: error read, ER_CHKSUM: error checksum */
/*   ER_SEQ: error sequence number, ER_LENGTH: header not found */
/* first character in rblk tells TYPE and the remaining part is data */
int get_blk(rblk)
unsigned char *rblk;
  {
  int i, chi, state, length, retlength;
  int sync, chksum, check, seqnr, ready;
  sync= check= ready= 0;
  for (i=0; i<270; i++)   /* read at most 120 bytes for one block */
    {
    chi= get_ch();
    switch (chi)
      {
      case ER_TIMOUT:  break;
      case ER_READ:  break;
      default:
      switch (sync)
	{
	case 0:  /* look for start of header */
	  if (chi== SOH) sync++; break;
	case 1:  /* number of bytes in packet excluding SOH and length byte */
	  i= 0; length= retlength= chi; sync++; chksum= chi; break;
	case 2:  /* sequence number of packet */
	  seqnr= chi;
	  if (seqnr != ++in_seq_nr)
          chi= ER_SEQ;
	  sync++; chksum+= chi; length--; break;
	case 3:  /* TYPE of message */
	  *rblk++= chi; sync++; chksum+= chi; length--; break;
	case 4:  /* actual data, maybe nothing in some datatypes */
	  if (length> 1)
	      {
	      chksum+= chi;
	      *rblk++= chi;
	      length--;
	      }
	    else  /* last byte in block, the checksum */
	      {
	      ready= 1;
	      length--;
	      chksum&=0xff;
	      if (chksum == chi)
		  check= 1;
		else
		  {
		  chi= ER_CHKSUM;
#ifndef DT08
		  printf("checksumma: ber=$%02x,  inl=$%02x\n",
			  chksum, chi);
		  printf("rblk: %s\n", rblk);
#endif
		  }
	      }
	  break;
	}
      }
    if ((chi < 0) || ready) break;
    }
  if (chi< 0) return(chi);
  *rblk= '\0'; /* put end of string, useful if text otherwise dosn't matter */
  if (check) return(retlength-2);   /* subtract length of seqnr and chksum */
      else return(ER_LENGTH);
  }

/* assemble a packet to transmit */
/* packet (wblk) must be filled with type and data on entrance */
/* nof: number of bytes incl TYPE */
/* returns total length to send in bytes, or 0 if error */
int asm_blk(nof)
int nof;
  {
  static out_seq_nr;
  unsigned char *p1;
  int chksum, i;
  if (nof> BLK_DATA_MAX+1) return(0);
  chksum= 0;
  wblk.seqnr= ++out_seq_nr;              /* sequence number */
  wblk.length= (unsigned char) (nof+2);  /* length */
  wblk.start= SOH;                       /* start of header code */
  p1= &wblk.length;
  for (i=0; i< (nof+2); i++)             /* calculate checksum */
    chksum+= *p1++;
  *p1++= (unsigned char) chksum;
  return((int)(p1-&wblk.start));
  }

/* send an assembled block, wait for an acknowledge packet */
/* returns number of received bytes data, data in global rblk */
/* returns <0 if error */
int send_blk(nof)
int nof;
  {
  int stat;
  if ((stat=wr_blk(nof))== 0)
      return(get_blk(rblk));
    else return(stat);
  }

/* create error message to string, space must be allocated */
/* returns pointer to string */
/* table of errormessages declared in DTPROTO.H */
char * errmsgs(errno, pstr)
int errno;
char *pstr;
  {
  abs(errno);
  strcpy(pstr, perrtab[errno]);
  return(pstr);
  }



