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

#define SPI_CLK 250000

typedef struct  {
    volatile Uint16 SPICDR;
    volatile Uint16 SPICCR;
    volatile Uint16 SPIDCR1;
    volatile Uint16 SPIDCR2;
    volatile Uint16 SPICMD1;
    volatile Uint16 SPICMD2;
    volatile Uint16 SPISTAT1;
    volatile Uint16 SPISTAT2;
    volatile Uint16 SPIDAT1;
    volatile Uint16 SPIDAT2;
} SpiRegStruct;


typedef volatile ioport SpiRegStruct *SpiRegsOvly;
SpiRegsOvly spiRegs;

#define FAIL_BAD_PARAM 1
#define OK 0

#define WRITE_CMD 0x02
#define READ_CMD 0x01

#define CHAR_LENGTH_16 15

int spiWrite(Uint16 deviceNo, Uint16 *data, Uint16 count)
{
	Uint16 i;
	Uint16 command;
	if(deviceNo > 3) return FAIL_BAD_PARAM;
	command = (deviceNo << 12) | (CHAR_LENGTH_16 << 3) | WRITE_CMD; //device, wordlength & command
	spiRegs->SPICMD1 = (count-1) & 0x0fff;
	for(i=0; i<count; i++) {
		while( spiRegs->SPISTAT1 & 0x01) {}; // Character complete bit
		spiRegs->SPIDAT2 = data[i];
		spiRegs->SPICMD2 = command;
	}
	return OK;
}

int spiRead(Uint16 deviceNo, Uint16 *data, Uint16 count)
{
	Uint16 i;
	Uint16 command;
	if(deviceNo > 3) return FAIL_BAD_PARAM;
	command = (deviceNo << 12) | (CHAR_LENGTH_16 << 3) | READ_CMD; //device, wordlength & command
	while( spiRegs->SPISTAT1 & 0x01) {}; // Character complete bit
	spiRegs->SPICMD1 = count & 0x0fff;
	for(i=0; i<count; i++) {
		spiRegs->SPICMD2 = command;
		while( spiRegs->SPISTAT1 & 0x01) {}; // Character complete bit
		data[i] = spiRegs->SPIDAT1;
	}
	return OK;
}


int spiInit(Uint32 sysClk)
{
	Uint16 div = (sysClk / SPI_CLK) + 1; 
	if(div < 3) return FAIL_BAD_PARAM;
	
	spiRegs = (SpiRegsOvly)	0x3000; //initialise register structure pointer
	spiRegs->SPICCR = 0; //disable clk
	spiRegs->SPICDR = div;
	spiRegs->SPIDCR1 = 0;
	spiRegs->SPIDCR2 = 0x0500;
	spiRegs->SPICCR = 0x8000; //enable clk
	return OK;	
}
