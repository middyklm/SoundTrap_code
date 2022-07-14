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
#include "d3defs.h"
#include "d3std.h"
#include "misc.h"

#define SAR_TIMOUT 10 //x100us

typedef struct  {
    volatile Uint16 SARCTRL;
    volatile Uint16 RSVD2;
    volatile Uint16 SARDATA;
    volatile Uint16 RSVD3;
    volatile Uint16 SARCLKCTRL;
    volatile Uint16 RSVD4;
    volatile Uint16 SARPINCTRL;
    volatile Uint16 RSVD5;
    volatile Uint16 SARGPOCTRL;
} SarRegs;

typedef volatile ioport SarRegs  *SarRegsOvly;
SarRegsOvly sarRegs;


int sarDoConversion(Uint16 chan, Uint16 *data)
{
	int timeout = SAR_TIMOUT;
	sarRegs->SARPINCTRL = 0x3100; //PWRUP, SARP, REFVDDDSEL
	sarRegs->SARCTRL = 0x8400 | ( chan <<  12); //start, norm, single
	microdelay(100);
	while(sarRegs->SARDATA & 0x8000) {
		if( timeout-- > SAR_TIMOUT) return FAIL;
		microdelay(100);
	}
	*data = sarRegs->SARDATA & 0x03FF;
	sarRegs->SARPINCTRL = 0x0000; //power down
	return OK;
}



int sarInit()
{
	sarRegs = (SarRegsOvly)	0x7012; //initialise register structure pointer
	sarRegs->SARCLKCTRL = 99; //divide sys clk by 100
	return OK;
}
