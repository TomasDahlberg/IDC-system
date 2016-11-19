
/*******************************************************************/
/*                                                                 */
/*                        N E T L I B                              */
/*                 COLNET Interface library                        */
/*                                                                 */
/*     This library contains C-functions designed to interface     */
/*     with the COLNET network driver NT82C684. This library       */
/*     requires an edition of 22. or higher for the driver.        */
/*                                                                 */
/*                                                                 */
/*******************************************************************/

#ifndef NETLIB

/* This file should be compiled into a library, but if the user    */
/* #includes it, define this label to make it impossible to        */
/* include it more than once.                                      */

#define NETLIB 2.0

/* Make sure everything we need is included. */

#include <sgstat.h>
#include <errno.h>

#ifndef stdin
#include <stdio.h>
#endif
#ifndef S_IREAD
#include <modes.h>
#endif
#ifndef NETDEVICE
#define NETDEVICE "/nb"
#endif

static int netpath = 0,   /* File descriptor to the network path */
           netnode = -1,   /* The node we are currently talking to */
           netpacketsz;   /* current network packet size */

/*******************************************************************/
/*                                                                 */
/* int netopen()                                                   */
/*                                                                 */
/* The netopen() function is used to establish a connection to the */
/* network device driver using OS9s file system. The path returned */
/* needs to be kept open during the entire session, therefore it   */
/* is saved in a static variable - rather than trust the user to   */
/* pass it at every call to the other functions.                   */
/*                                                                 */
/* The return value is -1 if the network could not be accessed,    */
/* if ok, then the file descriptor is returned.                    */
/*******************************************************************/

int netopen()
{
   if (netpath <= 0)
      if ((netpath = open(NETDEVICE, 0x03)) == -1) return(-1);
   return(netpath);
}

int netSetBps(bps)
int bps;
{
  struct sgbuf buffer;
  
  if (_gs_opt(netpath, &buffer) == -1)
    return (errno) ? errno : -1;
  buffer._sgm._sgs._sgs_baud = bps;
  if (_ss_opt(netpath, &buffer) == -1)
    return (errno) ? errno : -1;
  return 0;
}

/*******************************************************************/
/*                                                                 */
/* void netflush()                                                 */
/*                                                                 */
/* The netflush() function is used anytime it is neccessary to     */
/* be certain that every byte written by the netwrite() function   */
/* is transferred to the selected node. Normally the network uses  */
/* the delayed write method to improve throughput on the network,  */
/* this also means that some data may still be hanging around in   */
/* the masters output buffers. Calling this function guarantees    */
/* that every byte will be received by the selected node.          */
/*                                                                 */
/*******************************************************************/

void netflush()
{
	_ss_dcon(netpath, netnode);
}

/*******************************************************************/
/*                                                                 */
/* void netclose()                                                 */
/*                                                                 */
/* The netclose() function is used to end the current session in   */
/* a controlled manner. Any data hanging around in the drivers     */
/* buffers will be transmitted to the intended destination before  */
/* closing down.                                                   */
/*                                                                 */
/*******************************************************************/

void netclose()
{
   if (netpath > 0)
   {
     netflush();
     close(netpath);
     netpath = 0;
   }
}

/*******************************************************************/
/*                                                                 */
/* int netpoll()                                                   */
/*                                                                 */
/* The netpoll() function is used to get the status for any node   */
/* in the network. This function is similar to _gs_rdy() described */
/* in the OS9 C-library reference manual. The ONLY difference is   */
/* that if the computer calling this function happens to be the    */
/* master node, then the returned value will reflect how many      */
/* data bytes the currently selected node has available. This is   */
/* NOT the same as the number of bytes currently available in the  */
/* masters input data buffer. If the computer using this function  */
/* is a slave, then there is no difference between using this      */
/* function and the _gs_rdy() function.                            */
/*                                                                 */
/* In a slave:                                                     */
/* Returns -1 (E$NotRdy) if no data is available (0 bytes), or     */
/* n if n bytes can be read without blocking.                      */
/*                                                                 */
/* In the master:                                                  */
/* Returns -1 if the selected node doesnt exist.                   */
/* Returns 0 if the selected node has no data available            */
/* Returns n if n bytes can be read from that node.                */
/*                                                                 */
/*******************************************************************/

int netpoll(node)
int node;
{
  if (node != netnode) {
    _ss_dcon(netpath, node);
    netnode = node;
  }
  return(netpacketsz = _gs_rdy(netpath));
}

#define InSize 1024 /* input buffer size */
#define OutSize 1024 /* output buffer size */
#define FrmSize 32 /* frame size */

#define OFF 0
#define ON 1

typedef unsigned char * PTR;
typedef unsigned char BYTE;

long oldstat, oldnowr;

/* Device static storage */
typedef struct {
 PTR Shadow; /* ds.l 1 pointer to shadow register */
 PTR Handler; /* ds.l 1 current input interrupt handler */
 PTR InRead; /* ds.l 1 input-buffer read pointer */
 PTR InWrite; /* ds.l 1 input-buffer write pointer */
 PTR InEnd; /* ds.l 1 input-buffer end pointer */
 PTR OutRead; /* ds.l 1 output-buffer read pointer */
 PTR OutWrite; /* ds.l 1 output-buffer write pointer */
 PTR OutEnd; /* ds.l 1 output-buffer end pointer */
 PTR DevEven; /* ds.l 1 pointer to even device half */
 WORD InCount; /* ds.w 1 input-buffer level */
 WORD OutCount; /* ds.w 1 output-buffer level */
 WORD IRQMask; /* ds.w 1 device irq level mask */
 WORD Station; /* ds.w 1 netnode station id code */
 WORD MsbReg; /* ds.w 1 current address modifier */
 WORD LastAccs; /* ds.w 1 last accessed node */
 WORD CmdAccs; /* ds.w 1 node accessed during this command transfer */
 WORD NextAccs; /* ds.w 1 the next node to access */
 WORD PkSize; /* ds.w 1 current blocking size */
 WORD PkRdy; /* ds.w 1 current ready size */
 WORD PkIn; /* ds.w 1 current input block size */
 WORD PkOut; /* ds.w 1 current output block size */
 BYTE PkCmd; /* ds.b 1 the last transmitted command byte */
 BYTE PkMode; /* ds.b 1 current packet transfer mode */
 BYTE PkType; /* ds.b 1 current packet type (0= data, $80= commands) */
 BYTE PkFlag; /* ds.b 1 remote station accept flag */
 BYTE PkError; /* ds.b 1 accumulated errors */
 BYTE Select; /* ds.b 1 node select (0 = not selected) */
 BYTE MsgByte; /* ds.b 1 the last message byte sent to the other node */
 BYTE Parity; /* ds.b 1 parity code value */
 BYTE BaudRate; /* ds.b 1 baudrate code value */
 BYTE IrqFlags; /* ds.b 1 enabled irqs for this device */
 BYTE RtsBit; /* ds.b 1 enable for RTS output */
 BYTE InBuf[InSize]; /* ds.b InSize input data buffer */
 BYTE OutBuf[OutSize]; /* ds.b OutSize output data buffer */
 long FIX; /* testvalue */
 long NrofReads; /* ds.l 1 nr of good blocks read */
 long NrofWrites; /* ds.l 1 nr of good blocks written */
 long NrofErrors; /* ds.l 1 nr of errors */
 long NrofResets; /* ds.l 1 nr of resets */
 long TxStatus; /* ds.l 1 transmit status */
 WORD SigReady[3]; /* send signal on data ready */
 WORD SigPoll[3];  /* send signal on poll event */
 long Nrofpolls;
 } netstatic;

static netstatic *netstat;

#define VERSION_920911
#ifdef VERSION_920911
int netCheck()
{
  static int prevOutCount, prevPolls;
/*  
static int slowIt;
  slowIt++;
  if (slowIt >= 0 && slowIt < 10)
    return 1;  
  slowIt = 0;
*/
  netstat = (netstatic *) *((long *) 0x40c);

/*  changed 930202 
  if (netstat->OutCount && prevOutCount && (netstat->Nrofpolls == prevPolls)) {
*/
  if (netstat->Nrofpolls == prevPolls) {
      netClean();
      prevOutCount = netstat->OutCount;
      prevPolls = netstat->Nrofpolls;       /* polls added 921112 */
      return 0;   /* no, just did a reset of net */
  }
  prevOutCount = netstat->OutCount;
  prevPolls = netstat->Nrofpolls;
  return 1;   /* ok ! */
}

static int netClean()
{
  _ss_dcoff(netpath, -2);     /* changed -1 -> -2, BIGreset, 921112 */
}
#endif /* VERSION_920911 */

int netOutBufSize()
{
  netstat = (netstatic *) *((long *) 0x40c);
  return netstat->OutCount;
}

int netrdy(node)
int node;
{
  if (node != netnode) {
    _ss_dcon(netpath, node);
    netnode = node;
  }
  if ((netpacketsz = _gs_rdy(netpath)) < 0)
    return 0;
  netstat = (netstatic *) *((long *) 0x40c);
  return netstat->PkFlag & 0x80;
}

/*******************************************************************/
/*                                                                 */
/* int netwrite(node, cptr, sz)                                    */
/* int node;                                                       */
/* char *cptr;                                                     */
/* int sz;                                                         */ 
/*                                                                 */
/* The netwrite() function will write any sized block of data      */
/* to any node on the network. Large blocks (>1 KByte) can block   */
/* the master if the selected node is trying to keep up processing */
/* it. However there is no practical limit to how big the block    */
/* can be.                                                         */
/*                                                                 */
/* NOTE: like any write call this function may not transmit every  */
/* byte. The return value reflects the status.                     */
/*                                                                 */
/* -1 means that the selected node could not be reached (probably  */
/*    powered down or nonexistent)                                 */
/* n < sz  this can happen (see the C-library). If this happens,   */
/*    call this function again with the bytes that did not get     */
/*    written.                                                     */
/* sz All bytes were successfully written.                         */
/*                                                                 */
/*******************************************************************/

int netwrite(node, cptr, sz)
int node;
char *cptr;
int sz;
{
	int n;
	
	/* Continued write to the same node? If so we dont */
	/* need to flush the buffers, simply continue writing */
	if (node == netnode)
	   return(write(netpath, cptr, sz));
   
        /* Write to another node, select the new node and flush */
        /* the buffers */
	_ss_dcon(netpath, node);
	netnode = node;
	
	/* Write data to the the new node */
        return(write(netpath, cptr, sz));
}

/*******************************************************************/
/*                                                                 */
/* int netread(node, cptr, sz)                                     */
/* int node;                                                       */
/* char *cptr;                                                     */
/* int sz;                                                         */ 
/*                                                                 */
/* The netread() function will read any sized block of data        */
/* from any node on the network. Large blocks (>1 KByte) can block */
/* the master if the selected node doesnt have the requested data  */
/* available yet, if so the master will wait until the requested   */
/* data becomes available. The preferred method is to use the      */
/* netpoll() function to check what is available, then read        */
/* that many bytes.                                                */
/*                                                                 */
/* NOTE: like any read call this function may not receive every    */
/* byte. The return value reflects the status.                     */
/*                                                                 */
/* -1 means that the selected node could not be reached (probably  */
/*    powered down or nonexistent)                                 */
/* n < sz  this can happen (see the C-library). If this happens,   */
/*    call this function again with the bytes that was not read.   */
/* sz All bytes were successfully read.                            */
/*                                                                 */
/*******************************************************************/
	
int netread(node, cptr, sz)
int node;
char *cptr;
int sz;
{
/*
  printf("Netread: node = %d, netnode = %d, sz = %d\n", node, netnode, sz);
*/
  
  if (node != netnode) {
    _ss_dcon(netpath, node);
    netnode = node;
  }
/*   printf("netpath = %d, cptr = %d\n", netpath, cptr);        */
  return(read(netpath, cptr, sz));
}

/*******************************************************************/
/*                                                                 */
/* int netget(node, cptr, sz, maxblocked)                          */
/* int node;                                                       */
/* char *cptr;                                                     */
/* int sz;                                                         */
/* int maxblocked;                                                 */
/*                                                                 */
/* The netget() function is used to get a guaranteed delivery      */
/* of bytes to the selected node. Guaranteed means that every      */
/* byte requested will if the other node is online be recevied.    */
/* This is accomplished by calling the netread() function          */
/* until all bytes have been transferred. A number of attempts     */
/* are made until finally giving up. The fourth paramter           */
/* maxblocked should be set to the number of retries to be         */
/* attempted. IF the call is unsuccessful after maxblocked         */
/* attempts, then the number of bytes still not read is            */
/* returned.                                                       */
/*                                                                 */
/* returns 0 if all bytes were received.                           */
/* returns n if n bytes remains to be read.                        */
/*                                                                 */
/*******************************************************************/

int netget(node, cptr, sz, maxblocked)
int node;
char *cptr;
int sz;
int maxblocked;
{
	int n, blocked;
	blocked = 0;
	do {
        	n = netread(node, cptr, sz);
	        if (n > 0) 
	        {
        	    cptr += n;
	            sz -= n;
        	}
	        else
        	{
	        	if (++blocked > maxblocked) return(sz);
	        }
        } while (sz);  
        return(sz);
}

/*******************************************************************/
/*                                                                 */
/* int netput(node, cptr, sz, maxblocked)                          */
/* int node;                                                       */
/* char *cptr;                                                     */
/* int sz;                                                         */
/* int maxblocked;                                                 */
/*                                                                 */
/* The netput() function is used to get a guaranteed delivery      */
/* of bytes to the selected node. Guaranteed means that every      */
/* byte written will if the destination is online be recevied.     */
/* This is accomplished by calling the netwrite() function         */
/* until all bytes have been transferred. A number of attempts     */
/* are made until finally giving up. The fourth paramter           */
/* maxblocked should be set to the number of retries to be         */
/* attempted. IF the call is unsuccessful after maxblocked         */
/* attempts, then the number of bytes still not written is         */
/* returned.                                                       */
/*                                                                 */
/* returns 0 if all bytes were transmitted.                        */
/* returns n if n bytes remains to be written.                     */
/*                                                                 */
/*******************************************************************/

int netput(node, cptr, sz, maxblocked)
int node;
char *cptr;
int sz;
int maxblocked;
{
	int n, blocked;
	blocked = 0;
	do {
        	n = netwrite(node, cptr, sz);
	        if (n > 0) 
	        {
        	    cptr += n;
	            sz -= n;
        	}
	        else
        	{
	        	if (++blocked > maxblocked) return(sz);
	        }
        } while (sz);  
        return(sz);
}

/*******************************************************************/
/*                                                                 */
/* int netcmd(node, cptr, sz)                                      */
/* int node;                                                       */
/* char *cptr;                                                     */
/* int sz;                                                         */ 
/*                                                                 */
/* The netcmd() function will write any sized block of data as a   */
/* command to any node on the network. Commands are defined by the */
/* first byte written and by any subsequent parameter bytes.       */
/* The size of a command is not important, superfluous bytes are   */
/* ignored as well as undefined commands. A command will be        */
/* received by the destination node, but it will be filtered out   */
/* from the data stream. Any command block larger than 512 bytes   */
/* will be truncated to 512 bytes.                                 */
/*                                                                 */
/* NOTE: like any write call this function may not transmit every  */
/* byte. The return value reflects the status.                     */
/*                                                                 */
/* -1 means that the selected node could not be reached (probably  */
/*    powered down or nonexistent)                                 */
/* sz All bytes were successfully written.                         */
/*                                                                 */
/*******************************************************************/

int netcmd(node, cptr, sz)
int node;
char *cptr;
int sz;
{
	int n;
	
	/* Select a new node in command mode (data carrier off) */
	_ss_dcoff(netpath, node);
        netnode = node;
	
	/* Write command packet to the the new node */
        n = write(netpath, cptr, sz);

	/* Did we send anything? */
	if (n > 0)
	{
		/* If the packet was a multiple of 120 bytes long we */
		/* must pad it with a dummy byte */
		if ((n % 120) == 0)
			write(netpath, "", 1);
	}
	
	/* Make sure all command bytes were sent by returning to data mode */
	_ss_dcon(netpath, node);
	
	/* Report nr of bytes written (or error) */
	return(n);
}
#endif
