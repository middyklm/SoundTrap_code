// SoundTrap Software v1.0
//
// Copyright (C) 2011-2014, John Atkins and Mark Johnson
//
// This work is a derivative of the D3-API Copyright (C) 2008-2010, Mark Johnson
//
// This file is part of the SoundTrap software. SoundTrap is an acoustic
// recording system intended for underwater acoustic measurements. This
// component of the SoundTrap project is free software: you can redistribute
// it and/or modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation, either version 3 of the License,
// or any later version.
//
// The SoundTrap software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this code. If not, see <http://www.gnu.org/licenses/>.

#include	<stdio.h>
#include <tistdtypes.h>
#include "soc.h"
#include "csl_pll.h"
#include "csl_intc.h"
#include "csl_general.h"
#include "gpio.h"

extern Uint32	sysclk ;

 #define DTAG3_wio1_input	(GPIO_RSET(IODIR,GPIO_RGET(IODIR)&0xbf))
 #define DTAG3_wio1_get		((GPIO_RGET(IODATA)&0x40)!=0)
 #define DTAG3_wio2_high	(GPIO_RSET(IODATA,GPIO_RGET(IODATA)|0x80))
 #define DTAG3_wio2_low		(GPIO_RSET(IODATA,GPIO_RGET(IODATA)&0x7f))
 #define DTAG3_wio2_input	(GPIO_RSET(IODIR,GPIO_RGET(IODIR)&0x7f))
 #define DTAG3_wio2_output	(GPIO_RSET(IODIR,GPIO_RGET(IODIR)|0x80))

#define	SYNC_TIMEOUT	(1000)
#define	SYNC_RETRIES	(30)
#define	BAUD_INTVL		(104)	// bit period in us
#define	bitdelay		(SWDelayUsec(BAUD_INTVL))
#define	hbitdelay		(SWDelayUsec(BAUD_INTVL>>1))
#define	shortdelay		(SWDelayUsec(10))
#define	longdelay		(SWDelayMsec(5))
#define	turnarounddelay	(SWDelayUsec(2000))
#define	chardelay		(SWDelayUsec(1000))
#define progdelay		(SWDelayUsec(10000))
#define	stimeout		(3000)	// units of 10us
#define	ltimeout		(100000)
#define txlow			(gpioSetVal(GPIO_BIT_WIF2,0))
#define	txhigh			(gpioSetVal(GPIO_BIT_WIF2,1))
#define	rxget			(gpioGetVal(GPIO_BIT_WIF1))
#define	txoutput		(gpioSetDir(GPIO_BIT_WIF2, GPIO_DIR_OUT))
#define	txinput			(gpioSetDir(GPIO_BIT_WIF2, GPIO_DIR_OUT))
#define	rxinput			(gpioSetDir(GPIO_BIT_WIF1, GPIO_DIR_IN))
#define testlow			(gpioSetVal(GPIO_BIT_MSP_PROG,0))
#define	testhigh		(gpioSetVal(GPIO_BIT_MSP_PROG,1))
#define	testoutput		(gpioSetDir(GPIO_BIT_MSP_PROG, GPIO_DIR_OUT))
#define	testinput		(gpioSetDir(GPIO_BIT_MSP_PROG, GPIO_DIR_IN))

int		bitcnt(Uint32 x) ;
void 	SWDelayUsec(Uint16 usec) ;
void	uarttx_mess(char *c, int n) ;
int		uartrx_mess(char *c, int nmax, long timeout) ;
void	uarttx_char(char c) ;
int		uartrx_char(long timeout) ;
char	chksum8(char *c, int n) ;
void	pause(void) ;


void	pause()
{
 //printf(".\n") ;
}


void	msp_prog_mode()
{
 testlow ;
 testoutput ;

 //printf("Push and hold Backdoor button. Press run to continue...\n") ;
 pause() ;
 testhigh ;
 progdelay ;
 testlow ;
 progdelay ;
 testhigh ;
 progdelay ;
 //printf("Release Backdoor button. Press run to continue...\n") ;
 pause() ;
 progdelay ;
 testlow ;
 testinput ;
}


int		msp_send(Uint16 *in, int n, Uint16 *out, int nout)
{
 // send a message to the MSP430 processor and return the response.
 // This function is only applicable to DTAG-3 devices.
 int	k, nr, ret=0 ;

 txhigh ;
 txoutput ;		// prepare pin function for output
 rxinput ;

 // do sync
 for(k=0;k<SYNC_RETRIES && ret!=0x90;k++) {
    uarttx_char(0x80) ;
	ret = uartrx_char(SYNC_TIMEOUT) ;
	}

 if(ret!=0x90) {
	//printf("Timeout on synchronizing\n");
	return(0) ;
	}

 turnarounddelay ;

 // send message
 uarttx_mess((char *)in,n) ;

 // wait for reply
 nr = uartrx_mess((char *)out,nout,ltimeout) ;
 
 txinput ;
 turnarounddelay ;
 return(nr) ;
}


void	uarttx_char(char c)
{
 int	k ;
 
 bitdelay ;		// delay to make stop bit from previous character

 if(bitcnt((long)c)&1)
	c |= 0x100 ;		// add a parity bit at the MS end of the char

 txlow ;		// start bit
 bitdelay ;
 for(k=0;k<9;k++) {		// 8 character bits plus 1 parity bit
 	if(c&1)
 	   txhigh ;
 	else
 	   txlow ;
 	bitdelay ;
 	c >>= 1 ;
    }
    
 txhigh ;  		// stop bit
}


int		uartrx_char(long timeout)
{
 int	c=0 ;
 long	k ;

 if(rxget == 0)				// check that rx line is idling
	return(-1) ;

 for(k=0;k<timeout;k++) {	// wait for start bit edge
 	if(rxget == 0) {
       hbitdelay ;			// wait for half a bit time
       if(rxget == 0)		// make sure start bit is still low
	      break ;
	   }

	shortdelay ;
	}

 if(k==timeout)				// check for a timeout
	return(-2) ;

 bitdelay ;					// wait for start bit to pass
 for(k=0;k<8;k++) {			// otherwise, read in the character
 	if(rxget==1)			// lsb first
	   c |= 0x100 ;
 	bitdelay ;		
 	c >>= 1 ;
    }
    
 bitdelay ;					// wait for the parity 
 hbitdelay ;				// and stop bit
 return(c) ;
}


void	uarttx_mess(char *c, int n)
{
 int	k ;
 char	chk1, chk2 ;

 chk1 = chksum8(c,n>>1) ;	// make the checksums
 chk2 = chksum8(c+1,n>>1) ;

 // send the message, character by character
 for(k=0;k<n;k++) {
 	uarttx_char(*c) ;
 	chardelay ;
 	c++ ;
    }

 uarttx_char(chk1) ;	// send the two checksums
 chardelay ;
 uarttx_char(chk2) ;
}


int		uartrx_mess(char *c, int nmax, long timeout)
{
 int	k, r ;

 // receive the message, character by character
 for(k=0;k<nmax;k++) {
 	if((r=uartrx_char(timeout))<0) {
	   //printf("rx err %d on char %d of %d\n",r,k,nmax) ;
 	   break ;
	   }

 	*c++  = (char)r ;
    }

 //*c = '\0' ;
 return(k) ;
}


char	chksum8(char *c, int n)
{
 int	k ;
 char	chk = 0 ;

 for(k=0;k<n;k++,c+=2)
	chk ^= *c ;

 return(chk^0xff) ;
}


int		bitcnt(Uint32 x)
{
 int	k, n=0 ;

 for(k=0;k<32;k++,x>>=1)
	n += x&1 ;

 return(n) ;
}


void SWDelayUsec(Uint16 usec)
{
 // software delay, calibrated for 25 MHz cpu clock
 // but should scale correctly to other frequencies.
 // Note: delay is correct with optimization level 3.
 // It will change if the optimization is changed.
    unsigned int i, j, loopsperusec;

    //loopsperusec = (sysclk>>20) - 6 ;
    loopsperusec = (sysclk>>20) - 6 ;
    for(i=0;i<usec;i++) {
        for(j = 0; j < loopsperusec; j++)
	 		asm("      NOP") ;
		}
}
