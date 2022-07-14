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

/*! \file dmem.c
    \brief Simple one-shot memory allocation for fast memory.

    Fixed size memory blocks are allocated on a first-come-first served
    basis from a large static buffer. There is no provision for freeing
    and reallocating this memory.

    buffers are allocated starting at FMEM_START defined in the d3.cmd file
    total buffer size will be less than or equal to FMEM_LENGTH defined in
    fmem.c. FMEM_START must have an even address.
*/

#include "d3defs.h"
#include "d3std.h"
#include "protect.h"
#include "board.h"
#include "devdep.h"
#include "misc.h"
#include "modnumbers.h"
#include "error.h"
#include "minmax.h"
#include "fmem.h"

// definitions to enable profiling:
// define DMEM_REPORT to make a .csv file with an entry for each alloc and free
// define DMEM_PROFILE to keep track of minimum number of blocks in each group

#define FMEM_LENGTH     ((Uint32)10000)

// the memory block for allocation by the fmem module is in
// a special section (.fmem)
#pragma DATA_ALIGN(FMEM_START, 8);
#pragma	DATA_SECTION(FMEM_START,".fmem")
int	FMEM_START[FMEM_LENGTH] ;

static	int	*fmem_next, fmem_togo=0 ;


int		FMEM_init(void)
{
 fmem_next = FMEM_START ;
 fmem_togo = FMEM_LENGTH ;
 return(OK) ;
}


Ptr	FMEM_alloc(uns sz)
{
 Ptr			p ;

 sz = (sz+1)&(~1) ;
 if(sz>fmem_togo) {
	err(FMEM_MOD,FMEM_SIZETOOBIG) ;
    return(NULL) ;
    }

 GO_ATOMIC ;			// disable interrupts
 p = (Ptr)fmem_next ;
 fmem_next += sz ;
 fmem_togo -= sz ;
 END_ATOMIC ;			// re-enable interrupts
 return(p) ;
}


int	FMEM_status(void)
{
 // report how many free blocks there are in each group
 #ifdef JTAG
  printf("FMEM: used %ld, available %ld\n",FMEM_LENGTH-fmem_togo, fmem_togo) ;
 #endif
 return(OK) ;
}
