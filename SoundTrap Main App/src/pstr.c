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

/*! \file pstr.c
    \brief Packed string handler.

	Packed strings use both bytes in the native 16-bit word of the
	DSP to be more memory efficient.
*/

#include 	<string.h>

#include "d3defs.h"
#include "d3std.h"
#include "protect.h"
#include "board.h"
#include "devdep.h"
#include "misc.h"
#include "modnumbers.h"
#include "error.h"
#include "dmem.h"
#include "pstr.h"


void	pstr_init(PSTRING *p, uns *buff, int len)
{
 // initialize the packed string structure elements
 p->ntogo = len<<1 ;	// number of bytes available in the buffer
 p->b = buff ;			// pointer to the start of the buffer
 p->nby = 0 ;			// number of bytes used
}


int	    pstr_cat(PSTRING *p, char *s)
{
 int	n, k, odd ;
 uns	*b ;

 n = strlen(s) ;
 if(n > p->ntogo)
	return(FAIL) ;

 odd = (p->nby)&1 ;
 b = p->b + ((p->nby)>>1) ;

 for(k=n;k>0;k--) {
	if(odd) {
	   *b |= (*s++) & 0x0ff ;
	   ++b ;
	   odd = 0 ;
	   }
	else {
	   *b = *s++ << 8 ;
	   odd = 1 ;
	   }
	}

 p->nby += n ;
 p->ntogo -= n ;
 return(OK) ;
}


int	    pstr_pncat(PSTRING *p, uns *s, int n)
{
 // cat upto n packed characters to p but stop if a null
 // character is found

 int	k, podd, sodd ;
 char	c ;
 uns	*b ;

 if(n > p->ntogo)
	return(FAIL) ;

 b = p->b + ((p->nby)>>1) ;
 podd = (p->nby)&1 ;
 sodd = 0 ;

 for(k=0;k<n;k++) {
	c = sodd ? *s++ : (*s>>8) ;
	c &= 0x0ff ;
	sodd = (sodd==0) ;
	if(c==0)
	   break ;

	if(podd)
	   *b++ |= c ;
	else
	   *b = c << 8 ;

	podd = (podd==0) ;
	}

 p->nby += k ;
 p->ntogo -= k ;
 return(OK) ;
}


int	pstr_cpy(PSTRING *dest, PSTRING *src)
{
 int	nd ;

 nd = dest->ntogo + dest->nby ;		// size of dest buffer in bytes
 if(nd < src->nby)
	 return(FAIL) ;

 memcpy(dest->b,src->b,pstr_size(src)) ;
 dest->nby = src->nby ;
 dest->ntogo = nd - dest->nby ;
 return(OK) ;
}


int	pstr_add(PSTRING *p, char *s)
{
 // cat a string to a packed string 
 // re-allocate the packed string buffer if necessary
 int	nw ;
 uns	*newb ;

 if(pstr_cat(p,s)==OK)
	 return(OK) ;

 nw = (strlen(s)+p->nby+1)>>1 ;
 if((newb = (uns *)DMEM_alloc(nw+PSTR_LENINC))==NULL)
	 return(FAIL) ;

 memcpy(newb,p->b,pstr_size(p)) ;
 DMEM_free(p->b) ;
 p->b = newb ;
 p->ntogo = ((nw+PSTR_LENINC)<<1) - p->nby ;
 return(pstr_cat(p,s)) ;
}


void	pstr_print(PSTRING *p)
{
 int k;
 uns *pp;

 for(k=0,pp=p->b;k<(p->nby>>1);k++,pp++)
    printf("%c%c",(char)(*pp>>8),(char)(*pp&0x0ff));

 printf("\n");
}
