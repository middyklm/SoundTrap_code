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

/*! \file data.c
    \brief data buffer handling functions.
*/

#include "d3defs.h"
#include "d3std.h"
#include "protect.h"
#include "board.h"
#include "devdep.h"
#include "misc.h"
#include "modnumbers.h"
#include "error.h"
#include "dmem.h"
#include "data.h"
#include <math.h>


DATA_Obj	*DATA_alloc(uns len)
{
 DATA_Obj	*d ;

 if((d=(DATA_Obj *)DMEM_alloc(sizeof(DATA_Obj)))==NULLPTR)
	return(NULLDATA) ;

 if(len == 0)
    d->p = NULLPTR ;

 else if((d->p=DMEM_alloc(len))==NULLPTR) {
	DMEM_free(d) ;
	return(NULLDATA) ;
	}

 d->sema = 0 ;
 d->size = 0 ;
 d->maxsize = len ;
 return(d) ;
}


void DATA_free(DATA_Obj *d)
{
 if(d==NULLDATA)
    return ;
    
 if(--(d->sema) > 0)
    return ;

 DMEM_free(d->p) ;
 DMEM_free(d) ;
}


void DATA_status(DATA_Obj *d)
{
 #ifdef JTAG
  printf(" DATA OBJECT: from source %x\n",d->id) ;
  printf("   fs = %ld, size = %d, nsamps = %d\n",d->fs,d->size,d->nsamps) ;
  printf("   time = %ld : %ld, sema = %d, nbits = %d\n",d->rtime,d->mticks,d->sema,d->nbits) ;
 #endif
}


int	DATA_settime(DATA_Timekey *t, DATA_Obj *out, DATA_Obj *in, int nsamps)
{
 int	newfs ;

 newfs = (t->lastfs) != (long)(in->fs) ;
 out->rtime = in->rtime ;		// initialize the output time fields
 out->mticks = in->mticks ;

 if(nsamps == 0)
	return(newfs) ;		// fixed bug here 31oct09 mj

 // check if we already know the inverse of the input sampling rate
 if(newfs) {
	 t->lastfs = in->fs ;
	 t->tickfact = 16000000l/(long)(in->fs) ;
	 t->exp = 4 ;		// TODO: make this responsive to size of in->fs
	 }

 // adjust output mticks to account for the number of samples offset
 out->mticks += (nsamps*t->tickfact)>>(t->exp) ;
 return(newfs) ;
}

