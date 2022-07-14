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

/*! \file blkpwr.h
    \brief Block power detector support functions.
*/

 typedef struct {
		long	acc ;
		int		*buff ;
		int		bindex ;
		int		bsize ;
 } BLKPWR_Obj ;

extern	int	find1stblkpwr(BLKPWR_Obj *b, int *x, int ns, long thr) ;

int	   BLKPWR_init(BLKPWR_Obj *b, int n) ;
void   BLKPWR_clear(BLKPWR_Obj *b) ;
