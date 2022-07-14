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

#include "sysControl.h"

#define P_RESET_COUNT 0x0020

typedef struct  {
    volatile Uint16 EBSR;
    volatile Uint16 RSVD0;
    volatile Uint16 PCGCR1;
    volatile Uint16 PCGCR2;
    volatile Uint16 PSRCR;
    volatile Uint16 PRCR;
    volatile Uint16 RSVD1[14];
    volatile Uint16 TIAFR;
    volatile Uint16 RSVD2;
    volatile Uint16 ODSCR;
    volatile Uint16 PDINHIBR1;
    volatile Uint16 PDINHIBR2;
    volatile Uint16 PDINHIBR3;
    volatile Uint16 DMA0CESR1;
    volatile Uint16 DMA0CESR2;
    volatile Uint16 DMA1CESR1;
    volatile Uint16 DMA1CESR2;
    volatile Uint16 SDRAMCCR;
    volatile Uint16 CCR2;
    volatile Uint16 CGCR1;
    volatile Uint16 CGICR;
    volatile Uint16 CGCR2;
    volatile Uint16 CGOCR;
    volatile Uint16 CCSSR;
    volatile Uint16 RSVD3;
    volatile Uint16 ECDR;
    volatile Uint16 RSVD4;
    volatile Uint16 RAMSLPMDCNTLR1;
    volatile Uint16 RSVD5;
    volatile Uint16 RAMSLPMDCNTLR2;
    volatile Uint16 RAMSLPMDCNTLR3;
    volatile Uint16 RAMSLPMDCNTLR4;
    volatile Uint16 RAMSLPMDCNTLR5;
    volatile Uint16 RSVD6[2];
    volatile Uint16 DMAIFR;
    volatile Uint16 DMAIER;
    volatile Uint16 USBSCR;
    volatile Uint16 ESCR;
    volatile Uint16 RSVD7[2];
    volatile Uint16 DMA2CESR1;
    volatile Uint16 DMA2CESR2;
    volatile Uint16 DMA3CESR1;
    volatile Uint16 DMA3CESR2;
    volatile Uint16 CLKSTOP;
    volatile Uint16 RSVD8[5];
    volatile Uint16 DIEIDR0;
    volatile Uint16 DIEIDR1;
    volatile Uint16 DIEIDR2;
    volatile Uint16 DIEIDR3;
    volatile Uint16 DIEIDR4;
    volatile Uint16 DIEIDR5;
    volatile Uint16 DIEIDR6;
    volatile Uint16 DIEIDR7;
} SysRegs;

typedef volatile ioport SysRegs	 *SysRegsOvly;
SysRegsOvly sysCtrlReg;

// Returns 64 bit unique ID
void sysControlGetId(Uint16 *buf)
{
    buf[0] = sysCtrlReg->DIEIDR0;

    buf[1] = sysCtrlReg->DIEIDR1 << 2;
    buf[1] |= sysCtrlReg->DIEIDR4 & 0x0003;

    buf[2] = sysCtrlReg->DIEIDR2;

    buf[3] = sysCtrlReg->DIEIDR3 << 4;
    buf[3] |= (sysCtrlReg->DIEIDR4 & 0x003C) >> 3;

/*
    buf[0] = sysCtrlReg->DIEIDR0;
	buf[1] = sysCtrlReg->DIEIDR1;
	buf[2] = sysCtrlReg->DIEIDR2;
	buf[3] = sysCtrlReg->DIEIDR3;
	buf[4] = sysCtrlReg->DIEIDR4;
	buf[5] = sysCtrlReg->DIEIDR5;
	buf[6] = sysCtrlReg->DIEIDR6;
	buf[7] = sysCtrlReg->DIEIDR7;
*/
}

void sysControlResetPeripheral(Uint16 bit)
{
	Uint16 i;
	if(bit < 16) {
		sysCtrlReg->PRCR |= (0x0001 << bit);
		for(i=0; i<P_RESET_COUNT; i++) {
			asm("\tNOP"); //delay for reset period
		}
	}
}

void sysControlInit()
{
	*(ioport short *)0x0001 = 0x0000; //Idle Config Red (ICR)
	asm("\tIDLE"); //take all ports out of idle state
		
	sysCtrlReg = (SysRegsOvly)	0x1c00; //initialise register structure pointer
 	sysCtrlReg->EBSR = 	0x1400; //0001 0100 0000 0000 PPMODE =001, SP1MODE =01, SP0MODE = 00
 	sysCtrlReg->PSRCR = P_RESET_COUNT; //reset cycles
 	sysCtrlReg->PCGCR1 = 0x0000; //enable all clocks
 	sysCtrlReg->PCGCR2 = 0x0000; //enable all clocks
 	sysCtrlReg->PDINHIBR1 = 0x0000;
 	sysCtrlReg->PDINHIBR2 = 0x0000;
 	sysCtrlReg->PDINHIBR3 = 0x0000; //turn on pull ups/downs
 	sysCtrlReg->PRCR = 0x00BB; 	//reset peripherals
}
