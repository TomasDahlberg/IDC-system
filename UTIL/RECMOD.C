/**************************************************************************/
/* File:        RECMOD.C      Type: P  (mainProgram/Module/Include)       */
/* By:          Dan Axelsson                                              */
/* Description:         Program to receive a module in DT-08 through stdin        */
/*              The program is started by PC/SENDMOD and acts like a      */
/*              server. A shell must be running on the communication port */
/* Use:         RECMOD                                                    */
/*                 terminate with a QUIT datablock only,                  */
/*                 or kill command from another terminal                  */
/*                 or reset DT08                                          */
/*                 The kill alternative will leave com parameters as is   */
/* ---------------------  History  -------------------------------------  */
/*   Date     /   What                                                    */
/* ...................................................................... */
/* 91-03-20   /   started                                                 */
/* 91-03-21   /   RECMOD will terminate (timeout) if no characters comes  */
/* 91-04-16   /   increased resolution in timeout, gives faster transfer  */
/*            /   New version Ver 1.1                                     */
/* 91-09-15   /   Sequence number variables now unsigned char (prev int)  */
/*            /   files with more than 256 blocks now works               */
/*            /   New version Ver 1.2                                     */
/* 91-09-15   /   updated                                                 */
/* 92-10-01   /   get_ch() reads all data into internal cache             */
/* 93-02-16       updates memload correct
/**************************************************************************/
@_sysedit: equ 7
@_sysattr: equ $800b

#define DT08

#include <stdio.h>
#include <sgstat.h>
#include <setsys.h>
#include <procid.h>
#include <signal.h>
/*
#include <module.h>
*/
#include "dt08io.h"
#include "dtproto.h"

#define NIL NULL

mod_exec *mexec, *modlink();
long modsize;
char *sendptr;

/* main variables as globals so they not use stack space */
int state, ready, errcnt, length;
int type;
unsigned long laddr;
FILE *mout;
char modname[40];
struct _sgs statbuf;    /* 128 byte status buffer */
struct _sgs bufcopy;
int stat;
char ch;
char version[] = "RECMOD version 1.3";
char signon[80];
char st[82];   /* string for local and temporary usage */
#include "dtproto.c"

int ltoa(lval, pasc)
unsigned long lval;
char *pasc;
  {
  sprintf(pasc, "%ld", lval);
  }

static int more, pek;                     /* added 921001 */
static unsigned char buff[256];

int get_more(p, n)    /* returns no of bytes read, added 921001 */
unsigned char *p;
int n;
{
  int min;

  if (!more)          /* no more in cache */
    return 0;
    
  if (n < more)
    min = n;
  else
    min = more;
  memcpy(p, &buff[pek], min);
  
  more -= min;
  pek += min; 

  return min;
}

int get_ch()
{
  int i, no;
  unsigned char ch;

  for (i=0; i<30000; i++)     /* allow 5 seconds for response */
  {
    if ((no = _gs_rdy(0)) > 0)     /* character at input ? */
    {
        if (read(0,&ch,1) != 1)
	    return(ER_READ);
          else return(ch);
    }
/*  tsleep(1);    /* tick = 1/100 second */
  }
  return(ER_TIMOUT);    /* timeout */
}

/* returns 0 if OK, otherwise ER_WRITE */
int wr_ch(ch)
char ch;
  {
  if (write(1,&ch,1)!= 1)
      return(ER_WRITE);
    else return(0);
  }

/* returns 0 if OK, otherwise ER_WRITE */
int wr_blk(nof)
int nof;
  {
  int stat;
  unsigned char *pblk;
  stat=0;
  pblk= &wblk.start;
  while ((nof-- > 0) && (stat==0))
    {
    stat= wr_ch(*pblk++);
    }
  return(stat);
  }

/* assemble and send block as answer from slave (don't wait for respons) */
/* type: packet type */
/* pstr: pointer to null terminated string */
/* returns error from wr_blk() */
int as_blk(type, pstr)
char type;
char *pstr;
  {
  wblk.type= type;
  strcpy(&wblk.data[0], pstr);
  return(wr_blk(asm_blk(strlen(&wblk.type))));
  }

int send_ack()
{
  wblk.type = PACK;
  wblk.seqnr= outSeqNr();  /* sequence number */
  wblk.length= (unsigned char) 3;        /* length */
  wblk.start= SOH;                       /* start of header code */
  wblk.data[0] = wblk.length + wblk.seqnr + wblk.type;
  write(1, &wblk, 5);
}

int send_block(buf, siz)
unsigned char *buf;
int siz;
{
  int i;
  unsigned char chs;
  wblk.type = PDATA;
  wblk.seqnr= outSeqNr();  /* sequence number */
  wblk.length= (unsigned char) 3 + siz;        /* length */
  wblk.start= SOH;                       /* start of header code */
  memcpy(wblk.data, buf, siz);
  chs = wblk.length + wblk.seqnr + wblk.type;
  for (i = 0; i < siz; i++)
    chs += *buf++;
  wblk.data[siz] = chs;
  write(1, &wblk, siz + 5);
}

int old_send_ack()
  {
  as_blk(PACK, "");
  }

int send_nak()
  {
  as_blk(PNAK, "");
  }

int set_com_param()
  {
  stat= getstat(0, 0, &statbuf);
/*  if (stat) printf("getstat status= %d\n", stat);
  printf("baud=%02x xoff=%02x xon=%02x tab=%02x\n",
    statbuf._sgs_baud,statbuf._sgs_xoff,statbuf._sgs_xon,statbuf._sgs_tabcr);
*/
  memcpy(&bufcopy, &statbuf, 128);   /* save copy to restore */
  statbuf._sgs_echo= 0;
  statbuf._sgs_pause=0;
  statbuf._sgs_bspch=0;
  statbuf._sgs_dlnch=0;
  statbuf._sgs_rlnch=0;
  statbuf._sgs_dulnch=0;
  statbuf._sgs_kbich=0;
  statbuf._sgs_bsech=0;
  statbuf._sgs_xon=0;
  statbuf._sgs_xoff=0;
  statbuf._sgs_tabcr=0;
  statbuf._sgs_delete= 0;
  statbuf._sgs_eorch=13;
  statbuf._sgs_eofch=0;
  statbuf._sgs_psch=0;
  statbuf._sgs_kbach=0;
  statbuf._sgs_bellch=0;

  setstat(0,0,&statbuf);     /* set new parameters */
  return(stat);
  }

int setBps(bps)	/* returns old os9-bps code if ok, otherwise 0 */
int bps;
{
  int oldBps;
  struct sgbuf buffer;
  
  if (_gs_opt(0, &buffer) == -1) {
    return 0;
  }
  oldBps = buffer._sgm._sgs._sgs_baud;
  buffer._sgm._sgs._sgs_baud = bps;
  
  if (_ss_opt(0, &buffer) == -1) {
    return 0;
  }
  return oldBps;
}

int restore_param()
  {
  stat= setstat(0,0,&bufcopy);
  return(stat);
  }

void init()
  {
  set_com_param();
  }

void pexit()
  {
  restore_param();
  exit(0);
  }

void main(argc, argv)
int argc;
char *argv[];
{
  int i;
  long memPtr;
  int node, err;
  int highSpeed, oldSpeed, doHighSpeed;

  initidcio();  
  setpr( getpid(), (short) 500);

  state = IDLE;
  ready = errcnt= 0;
  stdin->_flag |= _UNBUF;
  stdin->_flag |= _RBF;
  init();
  sprintf(signon, "%s running. Current load address: $%lx", version, *memload);
  as_blk(PACK, signon);    /* start with sign on message */
  do
    {
    stat= get_blk(rblk);
    if (errcnt++ > MAXERR)
        {
        as_blk(PNAK, "Giving up! Please syncronize to my protocol!");
        pexit();
        }
    switch (stat)
      {
      case ER_TIMOUT:
      case ER_READ:
      case ER_CHKSUM:
      case ER_SEQ:
      case ER_LENGTH:
        strcpy(st, "RECMOD: ");
        as_blk(PNAK, strcat(st,perrtab[-stat])); break;
      default:
      length= stat;
      errcnt= 0;
      type= rblk[0]; /* first byte tells messege type */
      switch (state)
        {
        case IDLE:
          switch (type)
            {
            case PCMD:
              switch (rblk[1])
                {
                case 'N':  /* new */
                  sscanf(&rblk[2], "%lx", &node);
                  if (node) {
                    if ((err = netNew(node)) == 0) {
                      sprintf(&wblk.data[0], 
                          "RECMOD: NEW done at node %d", node);
                      as_blk(PACK, &wblk.data[0]);   /* assemble and send block */
                    } else {
                      sprintf(&wblk.data[0], 
                          "RECMOD: NEW error %d from node %d", err, node);
                      as_blk(PNAK, &wblk.data[0]);   /* assemble and send block */
                    }
		  }
                  break;
                case 'S':  /* set load address */
                  sscanf(&rblk[2], "%lx", &laddr);
                  if (laddr>= MIN_LOAD_ADDR)
                      {
                      *memload= laddr;
                      sprintf(&wblk.data[0], "RECMOD: New load address: $%lx", *memload);
                      as_blk(PACK, &wblk.data[0]);   /* assemble and send block */
                      }
                    else as_blk(PNAK, "RECMOD: Address error!");
                  break;
                case 'R':  /* read back load address */
                  laddr= *memload;
                  sprintf(&wblk.data[0], "RECMOD: Current load address: $%lx", *memload);
                  as_blk(PACK, &wblk.data[0]);   /* assemble and send block */
                  break;
                case 'L':  /* load ipl string */
		  *iplcode = 0xc0de;
		  strncpy(iplstring, &rblk[2], length - 1);
		  iplstring[length - 1] = 0x0d;
		  send_ack();
		  break;
		case 'H':  /* high speed transfer (19200) */
/* if we can change the bps send ack at old speed otherwise nak at old speed,
   then we change baudrate
*/
		  highSpeed = 0x0f;  /* 19200; /*atoi(*/
		  if (oldSpeed = setBps(highSpeed)) {
		    doHighSpeed = 1;		/* dont need this ? */
		    setBps(oldSpeed);
		    send_ack();
		    sleep(2);		/* emit bytes at old rate */
		    setBps(highSpeed);

{
  int n;
  unsigned char ch;
  FILE *ff;

  ff = fopen("/t2", "w");
  fprintf(ff, "More = %d\n", more);
  while (1) {
    n = get_ch();
/*    n = read(0,&ch,1);	*/
    fprintf(ff, "(%2d:%3d), ", n, 0); 
  }

}

		  } else {
		    as_blk(PACK, "0");
		  }
		  break;
		case 'I':  /* restart DT08, don't expect life for 5 seconds */
		  os9fork("reboot", 0, 0, 0, 0, 0, 0);
/*                  send_nak();  /* not yet implemented */
		}
	      break;
	    case PSEND:  /* Initiate receive file */
	      if (rblk[1] == 'N')
		sscanf(&rblk[2], "%x", &node);
	      else
		node = 0;

	      {
		char buf[50];
               
                sprintf(buf, "RECMOD: receiving node is %d", node);
                as_blk(PACK, buf);
	      }
/*              send_ack();     */
              state= RECFILE;
              break;
            case PREC:  /* Initiate transmitt file */
              if (rblk[1] == 'N')
                sscanf(&rblk[2], "%x", &node);
              else
                node = 0;

	      {
		char buf[50];

		sprintf(buf, "RECMOD: transmitting node is %d", node);
		as_blk(PACK, buf);
	      }
/*              send_ack();     */
	      state= SENDFILE;
	      break;
	    case PQUIT:
	      send_ack();
	      ready= 1;
	      break;
	    default: send_nak();
	    }
	  break;
	case RECFILE:  /* fileheader */
	  if (rblk[0] != 'F') send_nak();     /* PFILEH */
	    else
	      {
	      strcpy(modname, &rblk[1]);
	      if ((mout= fopen("/mem", "w")) != NIL)
		  {
		  send_ack();
		  state= RECDATA;
		  memPtr = *memload;
		  }
		else as_blk(PNAK, "IVT16 Error opening /mem");
	      }
	  break;
	case SENDFILE:  /* fileheader */
	  if (rblk[0] != 'F')      /* PFILEH */
	    send_nak();
	  else {
	    strcpy(modname, &rblk[1]);
	    if ((mexec = modlink(modname, (short) 0)) == (mod_exec *) -1) {
		as_blk(PNAK, "IVT16 No such module");
	    } else {
		modsize = mexec->_mh._msize;
		sendptr = (char *) mexec;
	        send_ack();
		state= SENDDATA;
		memPtr = *memload;
	    }
          }
	  break;
	case RECDATA:  /* receive data */
	  switch (rblk[0])
	  {
	    case PDATA:
/*          if (fwrite(&rblk[1], 1, length-1, mout)== (length-1))
*/
	      if (node)
		err = netPutMem(node, memPtr, length - 1, &rblk[1]);
	      else {
		memcpy(memPtr, &rblk[1], length - 1);
		err = 0;
	      }
	      if (err == 0) {
		memPtr += (length - 1);
		send_ack();
	      }

	      break;
            case P_CMPR_DATA:
              if (node) {
                int size;
/*
		err = netPutMemCmpr(node, memPtr, length - 1, &rblk[1], &size);
*/
                memPtr += size;
	      } else {
		memPtr += unpack(&rblk[1], length - 1, memPtr);
		err = 0;
	      }
	      if (err == 0) {
		send_ack();
	      }
	      break;
	    case PEOF:
	      fclose(mout);     /* move from after to before ! 930216 */
	      if (!node) {              /* added 921110 */
		*memload= memPtr;
	      }
	      send_ack();
	      state= STOP;
	      break;
	    default:
	      send_nak();
	    }
	  break;
	case SENDDATA:  	/* transmitt data */
	  if (type== PACK) {
	      if (node)
		err = netGetMem(node, memPtr, length - 1, &rblk[1]);
	      else {
		memcpy(&rblk[1], sendptr, 250);
		sendptr += 250;
		modsize -= 250;
		err = 0;
	      }
	      if (err == 0) {
		if (modsize <= 0) {
		  state = SENDEOF;
		  modsize += 250;
		  send_block(&rblk[1], modsize);
		} else
		  send_block(&rblk[1], 250);
	      } else {
		send_nak();
	      }
	  } else {
	    send_block(&rblk[1], 250);
	  }
	  break;
	case SENDEOF:
	  if (type == PACK) {
	    as_blk(PEOF, "");  	/* dummy data block to get into SENDDATA state */
	    state = STOP;
	  } else
	    send_block(&rblk[1], modsize);
	  break;
	case STOP:
	  if (rblk[0] != PQUIT)
	      send_nak();
	    else
	      {
	      as_blk(PACK, "RECMOD: No unrecoverable errors! RECMOD terminating!");
	      ready= 1;
	      }
	}  /* end switch (state) */
      }  /* end switch(stat) */
    } while (!ready);
  sleep(2);  /* wait for message to leave DT08 */
  /* pexit(); */
  restore_param();
 }

