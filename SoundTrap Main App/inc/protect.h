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

// Interrupt protection definitions
// protect.h v1.0
// Last modified: June 2009

// assembly language function for manipulating interrupt mask
// use with care.

// memory location protectm is used to store a single semaphore
// bit. If protectm.1 is set, interrupts will not be re-enabled
// by END_ATOMIC. protectm is defined in misc.c

#ifndef _PROTECT_H_
#define _PROTECT_H_

extern int protectm;

static inline void protectOn(void)
{
	protectm = 1;
}

static inline void protectOff(void)
{
	protectm = 0;
}

static inline void protectEndAtomic(void)
{
	asm("      .global _protectm") ;
	asm("      BTST #0,*(_protectm),TC1") ;
	asm("      XCC  !TC1") ;
	asm("      BCLR INTM,ST1_55") ;
	asm("      NOP") ;
	asm("      NOP") ;
	asm("      NOP") ;
	asm("      NOP") ;
	asm("      NOP") ;
}

#define	GO_ATOMIC		 (_disable_interrupts())
#define	END_ATOMIC		 (protectEndAtomic())
#define	PROTECT		 	 (protectOn())
#define	END_PROTECT		 (protectOff())

#endif
