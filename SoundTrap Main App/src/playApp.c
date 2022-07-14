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
#include "data.h"
#include "audio.h"
#include "flsh.h"
#include "timr.h"
#include "decm.h"
#include "cfg.h"
#include "info.h"
#include "decfilt.h"
#include "config.h"
#include "mspif.h"
#include "audioOut.h"

static int	flashstate = 0 ;

// select one of the two following modes defined below

#define	USECLK	(32768000)	// 32 MHz system clock

int	flashnext(Ptr s, Ptr d)
{
	if((--flashstate)&1) {
		leds(GREENLED);
		TIMR_doinms(80,(JOB_Fxn)flashnext,NULL,MAXNICE-1);
	}
	else {
		leds(NOLED);
		if(flashstate>0) TIMR_doinms(50,(JOB_Fxn)flashnext,NULL,MAXNICE-1);
	}
	return(OK) ;
}

void flash(int n)
{
	flashstate = n<<1 ;
	flashnext(NULL,NULL) ;
}


int APP_close(void)
{
    return (OK);
}

int APP_init(void)
{
    int err;
    setsysclk(USECLK);
    int audid;

    msp_requestSensorScan();

    audid = AUDIO_open(AUDIO1, AUD_DIV_128);
    audioOutInit(AUD_DIV_128);
	AUDIO_start(audid);

    // start timed jobs
    err = TIMR_doevery(1,(JOB_Fxn)flash,NULL);// flash leds to indicate that all is well

    return (err);
}


