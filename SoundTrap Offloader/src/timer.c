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
#include "protect.h"
#include "timer.h"
#include "d3defs.h"

#define NO_OF_IO_TIMERS 3

Timer_Isr Timer_Isrs[NO_OF_IO_TIMERS];

typedef struct  {
    volatile Uint16 TCR;
    volatile Uint16 RSVD0;
    volatile Uint16 TIMPRD1;
    volatile Uint16 TIMPRD2;
    volatile Uint16 TIMCNT1;
    volatile Uint16 TIMCNT2;
} TimRegs;

typedef volatile ioport TimRegs  *TimRegsOvly;

TimRegsOvly timRegs0;
TimRegsOvly timRegs1;
TimRegsOvly timRegs2;

TimRegsOvly timerGetRegsPointer(Uint16 timer)
{
	TimRegsOvly regs;
	switch(timer) {
		case 0: regs = timRegs0; break;
		case 1: regs = timRegs1; break;
		case 2: regs = timRegs2; break;
		default: regs = NULL;
	}
	return regs;
}

interrupt void timerIsr(void)
{
    int i;
	Uint32 tiafrValue;
	PROTECT;

	tiafrValue = *(ioport Uint16 *)0x1C14;
  	*(ioport Uint16 *)0x1C14 = tiafrValue;

	for(i=0;i<3;i++) {
		if((tiafrValue >> i) & 0x0001) {
			if(Timer_Isrs[i] != NULL){
				(Timer_Isrs[i])();
			}
		}
	}
 	END_PROTECT ;
}

void timerInterruptHandlerRegister(Uint16 timer, Timer_Isr isr)
{
	Timer_Isrs[timer] = isr;
}

void timerInterruptHandlerDeRegister(Uint16 timer)
{
	Timer_Isrs[timer] = NULL;
}

void timerStart(Uint16 timer)
{
	TimRegsOvly regs = timerGetRegsPointer(timer);
	if(regs != NULL) {
		regs->TCR |= 0x8001;
	}
}

void timerStop(Uint16 timer)
{
	TimRegsOvly regs = timerGetRegsPointer(timer);
	if(regs != NULL) {
		regs->TCR &= ~0x8001;
	}
}

void timerConfig(Uint16 timer, Timer_Config config)
{
	Uint16 tcr = 0;

	TimRegsOvly regs = timerGetRegsPointer(timer);
	if(regs != NULL) {
		regs->TIMPRD1 = config.prd & 0x0000ffff;
		regs->TIMPRD2 = config.prd >> 16;
		tcr |= config.preScaleDiv << 2;
		if(config.autoLoad)	tcr |= 0x0002;
		regs->TCR = tcr;
	}
}

void timerInit()
{
	timRegs0 = (TimRegsOvly)0x1810;
	timRegs1 = (TimRegsOvly)0x1850;
	timRegs2 = (TimRegsOvly)0x1890;

	irqDisable(TINT_EVENT);
	irqPlug(TINT_EVENT, &timerIsr);
	irqEnable(TINT_EVENT);
    irqClear(TINT_EVENT);
    CSL_SYSCTRL_REGS->TIAFR = 0x0F;
}
