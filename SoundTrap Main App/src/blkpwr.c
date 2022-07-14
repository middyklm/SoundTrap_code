// D3-API v1.0
// Copyright (C) 2008-2010, Mark Johnson
//
// This file is part of D3, a real-time patch panel scheduler
// for digital signal processors.
//
// D3 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// D3 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with D3. If not, see <http://www.gnu.org/licenses/>.

/*! \file blkpwr.c
    \brief Block power detector support functions.
*/

#include "d3defs.h"
#include "d3std.h"
#include <string.h>
#include "error.h"
#include "dmem.h"
#include "fmem.h"
#include "blkpwr.h"


int	BLKPWR_init(BLKPWR_Obj *b, int n)
{
 if((b->buff=(int *)FMEM_alloc(n))==(int *)NULL)
	return(FAIL) ;			// allocation error

 b->bsize = n ;
 b->acc = 0 ;
 b->bindex = 0 ;
 memset((Ptr)b->buff,0,b->bsize) ;
 return(OK) ;
}


void	BLKPWR_clear(BLKPWR_Obj *b)
{
 b->acc = 0 ;
 b->bindex = 0 ;
 memset((Ptr)b->buff,0,b->bsize) ;
}
