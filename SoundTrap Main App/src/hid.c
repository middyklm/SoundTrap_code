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


#include <stdlib.h>
#include "d3defs.h"
#include "d3std.h"
#include "protect.h"
#include "board.h"
#include "devdep.h"
#include "misc.h"
#include "modnumbers.h"
#include "error.h"
#include "mspif.h"
#include "gpio.h"
#include "ioExpander.h"
#include "sar.h"
#include "hid.h"
#include "ioExpander.h"


ehid hid = ST200;
int hidMemCardCount = 1;

char* hidGetString()
{
	switch(hid) {
		case ST200: return "ST200";
		case ST300: return "ST300";
		case ST4300: return "ST4300";
		case ST500: return "ST500";
		default: return "unknown";
	}
}
int hidInit()
{
	Uint16 adc;
	Uint16 buf;

	if ( sarDoConversion(SAR_CHAN_BOARD_ID, &adc)  != OK ) {
 		return FAIL;
 	}

 	if(adc > 768)
 		hid = ST300;
 	else
 		hid = ST200;

	if (ioeInit(AUDIO_IO_EXPANDER_DEVICE_ADDRESS_MC, 0x00FF, 0x00FF) == TRUE)
	{
		hid = ST4300;
		if( ioeRead(AUDIO_IO_EXPANDER_DEVICE_ADDRESS_MC, &buf) == TRUE ) {
 			if((buf & 0xC0) == 0x80){ //GP6-7 = 1
 				hid = ST500;
 			}
 		}
	}

	if(hid == ST500) hidMemCardCount = 4;

 	return(OK);
}
