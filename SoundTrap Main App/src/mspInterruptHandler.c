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
#include "data.h"
#include "misc.h"
#include "audio.h"
#include "audioout.h"
#include "modnumbers.h"
#include "error.h"
#include "devdep.h"
#include "mspif.h"
#include "timr.h"
#include "job.h"
#include "info.h"
#include "audioOut.h"
#include "job.h"

extern	int muteon(Ptr s, Ptr d) ;
#define	CAL_JOB		((JOB_Fxn)muteon)

extern void	flash(int n);

#define	RESPONSE	(flash(1))
#define	MSP_INT_NICE			(5)


enum IR_COMMANDS {
	IR_COMMAND_STOP = 0x80,
	IR_COMMAND_START = 0x40,
	IR_COMMAND_PLAY = 0xC0, //?
	IR_COMMAND_STATUS = 0x20,
	IR_COMMAND_ARM = 0xA0,
	IR_COMMAND_CAL = 0x60,
	IR_COMMAND_BAT = 0x0E0,
	IR_COMMAND_LIGHT = 0x10,
	IR_COMMAND_F1 = 0x58,
	IR_COMMAND_F2 = 0xD8,
	IR_COMMAND_F3 = 0x38
};

int	mspInterruptJob(Ptr s, Ptr d)
{
	int flags = msp_getflags() ;


	if(flags & USB_PWR_FLAG) {
		restartOnExit = 1;
		INFO_event("SHUTDOWN",NULL,"USB");
		//log event and restart
 		return (FAIL);
	}

	if(flags & SHUTDOWN_FLAG) {
		//shutdown
		//restartOnExit = 0;
#ifdef PLOG
		INFO_event("SHUTDOWN",NULL,"MSP");
		return FAIL;
#else
		INFO_event("SHUTDOWN",NULL,"MSP");
		TIMR_doin(1, JOB_terminator,NULL);
		return (OK);
#endif
	}

	//IR Commands
	if(flags & IR_COMMAND_FLAG) {
	    char s[20];
		int	c = msp_getIrCommand() ;
	    sprintf(s, "DATA=\"%d\"", c);
	    INFO_event("REMOTE", s, NULL);

			switch (c) {
#ifdef PLOG
				case IR_COMMAND_STOP:
			 		return (FAIL);
#endif
				case IR_COMMAND_CAL:
					//JOB_postone(CAL_JOB,NULL,MAXNICE-2,NULL) ;
				break;
				case IR_COMMAND_F1:
					//audioOutStartTxFromSD(1);
				break;
				case IR_COMMAND_F2:
					//while(1) {}; //simulate lockup to trigger watchdog
					//audioOutStartTxFromSD(2);
				break;
				case IR_COMMAND_F3:
					//AUDIO_start_full_cal();
					//audioOutEnableCal(0);
				break;
			}
			RESPONSE ;
		}

	return(OK) ;
}

void mspInterruptHandler(Ptr s, Ptr d)
{
	JOB_postone((JOB_Fxn)mspInterruptJob,NULL,MSP_INT_NICE,NULL);
}

void mspInterryptHandlerInit()
{
	mspifRegisterInterruptHandler((Fxn)mspInterruptHandler);
}
