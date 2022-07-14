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

// The SoundTrap software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this code. If not, see <http://www.gnu.org/licenses/>.

/*! \file misc.c
    \brief Miscellaneous support functions.
*/

//#define JTAG
#include <stdlib.h>
#include "d3defs.h"
#include "d3std.h"
#include "protect.h"
#include "board.h"
#include "devdep.h"
#include "misc.h"
#include "modnumbers.h"
#include "error.h"
#include "mspif.h"


 // place to store a user function to call when there is an error
 static Fxn	error_task = NULL ;

 // error counter
 static int err_cnt = 0 ;


void err(uns mod, uns errno)
{
 ++err_cnt ;
 //msp_setEcode(mod, errno); //store err code in MSP mem
 #ifdef JTAG
  printf(" ERROR %04xh from module %04xh\n", errno, mod) ;
 #endif
 //if(error_task!=NULL)
 //	(error_task)(mod,errno) ;
}

void fatal(uns mod, uns errno)
{
  err(mod, errno);
  hibernate(1); //restart
  exit(1);
}

//Shutdown without flushing buffers. Called by fs when flash is full
void shutdown()
{
  hibernate(0); //dont restart
  exit(1);
}



void onerror(Fxn f)
{
 error_task = f ;
}


int		geterrorcnt(void)
{
 int	e = err_cnt ;
 err_cnt = 0 ;
 return(e) ;
}


uchar	 *bunpack(uchar *b, uns *d, int n)
{
 int	k ;

 for(k=0;k<n;k++) {
	 *b++ = (*d>>8)&0xff ;
	 *b++ = (*d++)&0xff ;
	 }

 return(b) ;
}


uchar	 *bunpackhi(uchar *b, uns *d, int n)
{
 int	k ;

 for(k=0;k<n;k++) {
	 *b++ = *d & 0xff00 ;
	 *b++ = (*d++)<<8 ;
	 }

 return(b) ;
}


uchar *bpack(uns *d, uchar *b, int n)
{
 int	k ;
 uns	dd ;

 for(k=0;k<n;k++) {
	 dd = *b++ << 8 ;
	 *d++ = dd | *b++ ;
	 }

 return(b) ;
}


long	byte2long(int *p)
{
 // accumulate 4 bytes from p into a signed long
 // The byte order is MS first and the first byte
 // contains the sign information. Bytes are right
 // justified in buffer p.
 int	r = (*p<<8) | *(p+1) ;		// do this to sign extend
 return(((long)r<<16) | ((uns)(*(p+2))<<8) | *(p+3)) ; 
}


void	microdelay(uns n)
{
 // approx n micro-second delay
 long	k, nn ;
 Uint32 mhz = getsysclk() / 1000000;

 nn = ((long)n*mhz)>>4 ;
 for(k=0;k<nn;k++) {
	asm("      NOP") ;
	asm("      NOP") ;
	asm("      NOP") ;
	asm("      NOP") ;
	}
}

void safe_memcpy(Uint16 *dst, Uint16 *src, Uint16 n)
{
 // DMA safe version of memcpy. This guarantees DMA access to the memory block of source
 // or destination at the expense of a 2x slower operation
 Uint16 k ;
	for(k=n;k>0;k--) {
		*dst++ = *src++ ;
		asm("     NOP") ;      // need this to force compilation as a rptblk instead of rpt
	}
}

/*
void	DMA_setsrcbuff(DMA_Handle h, int *p)
{
 // configure the source pointer of a dma

 volatile ioport uns *dp ;
 ulong pp = (ulong)p<<1 ;		// convert word address to byte address

 DMA_RSETH(h,DMACSSAL,(uns)(pp&0x0ffffu)) ;		// set the destination start address
 DMA_RSETH(h,DMACSSAU,(uns)((pp>>16)&0x0ffffu)) ;

 // need to check if the buffer is in SARAM or DARAM to configure the source
 // destination parameter register
 dp = (volatile ioport uns *)DMA_ADDRH(h,DMACSDP) ;									// write the buffer size
 if(pp<(ulong)0x10000)		// DARAM is at addresses 0-7fff in data space
	 *dp |= 4 ;
 else
	 *dp &= ~4 ;
}


void	DMA_setdestbuff(DMA_Handle h, int *p)
{
 // configure the destination pointer of a dma

 volatile ioport uns *dp ;
 ulong pp = (ulong)p<<1 ;		// convert word address to byte address

 DMA_RSETH(h,DMACDSAL,(uns)(pp&0x0ffffu)) ;		// set the destination start address
 DMA_RSETH(h,DMACDSAU,(uns)((pp>>16)&0x0ffffu)) ;

 // need to check if the buffer is in SARAM or DARAM to configure the source
 // destination parameter register
 dp = (volatile ioport uns *)DMA_ADDRH(h,DMACSDP) ;									// write the buffer size
 if(pp<(ulong)0x10000)		// DARAM is at addresses 0-7fff in data space
	 *dp |= 0x200 ;
 else
	 *dp &= ~0x200 ;
}
*/
