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

#include <tistdtypes.h>
#include "irq.h"
#include "cpu.h"

Uint16 irqGlobalDisable()
{
	Uint16 val = (CPU_REGS->ST1_55 & CPU_ST1_55_INTM) > 0;
	asm("	NOP");
    asm("	BSET INTM");
	asm("	NOP");
	asm("	NOP");
	asm("	NOP");
	asm("	NOP");
	asm("	NOP");
    return val;
}

void irqGlobalRestore(Uint16 enable)
{
	if( enable ) {
		asm("	NOP");
		asm("	BCLR INTM");
		asm("	NOP");
	}
}

void irqGlobalEnable()
{
	asm("	NOP");
	asm("	BCLR INTM");
}

void irqEnable( Uint16 eventId )
{
	Uint16 irqState;
	irqState = irqGlobalDisable();
	if( eventId < 16 ) {
		CPU_REGS->IER0 |= (1 << eventId);
	}
	else {
		CPU_REGS->IER1 |= (1 << (eventId-16));
	}
	irqGlobalRestore(irqState);
}

void irqDisable( Uint16 eventId )
{
	Uint16 irqState;
	irqState = irqGlobalDisable();
	if( eventId < 16 ) {
		CPU_REGS->IER0 &= ~(1 << eventId);
	}
	else {
		CPU_REGS->IER1 &= ~(1 << (eventId-16));
	}
	irqGlobalRestore(irqState);
}

void irqClear( Uint16 eventId )
{
	Uint16 irqState;
	irqState = irqGlobalDisable();
	if( eventId < 16 ) {
		CPU_REGS->IFR0 &= ~(1 << eventId);
	}
	else {
		CPU_REGS->IFR1 &= ~(1 << (eventId-16));
	}
	irqGlobalRestore(irqState);
}

void irqPlug(Uint16 eventId, irqIsr isr)
{
	Uint32 vecAddress;
	Uint16 irqState = irqGlobalDisable();

	vecAddress = (Uint32)(*(Uint16 *)0x0049);
	vecAddress = vecAddress << 8;
	vecAddress += eventId * 8;
	vecAddress /= 2; //word address

	*(Uint32 *)vecAddress = (Uint32)isr;
	irqGlobalRestore(irqState);
}

