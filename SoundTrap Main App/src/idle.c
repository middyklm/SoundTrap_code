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

/*! \file idle.c
    \brief Defines the idle task which runs when there is no other task
    scheduled.
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
#include "job.h"

// state to keep count of how many times idle is run
static ulong		idlecnt[1] ;

int		idlejob(ulong *s, Ptr d) ;


void	IDLE_init(void)
{
	// post the idle job
	// and must have a nice number equal to IDLENICE
	*idlecnt = (ulong)0 ;
	JOB_postone((JOB_Fxn)idlejob,(Ptr)idlecnt,IDLENICE,(Ptr)NULL) ;
}


int		idlejob(ulong *s, Ptr d)
{
	++(*s);
	//asm("	BSET XF,ST1_55");
	*(ioport short *)0x0001 = 0x006f; 	//Idle Config Red (ICR)
	asm("\tIDLE");                      // idle the cpu until the next interrupt
	//asm("	BCLR XF,ST1_55");
	return(OK);
}


int		IDLE_status(void)
{
	#ifdef JTAG
	printf("idle count %ld\n", *idlecnt) ;
	#endif

	*idlecnt = (ulong)0 ;
	return(OK) ;
}
