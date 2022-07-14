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
#include "protect.h"
#include "board.h"
#include "devdep.h"
#include "misc.h"
#include "modnumbers.h"
#include "error.h"
#include "config.h"
#include "data.h"
#include "info.h"
#include "mspif.h"
#include "timr.h"
#include "job.h"
#include "flsh.h"

#define SHUTDOWN_DELAY 1 //seconds
#define MIN_OFFTIME 10 //seconds
#define MIN_ONTIME 5 //seconds
#define BOOT_DELAY 2 //seconds

extern Uint32 systemStartTime;

void scheduleHibernate(Ptr a, Ptr b) 
{
	Uint32 nextStartTime;
	Uint32 offTime = systemConfig.onceEveryTime - systemConfig.onTime;
	if( systemConfig.runMode == runModeTimed ) {

		if( offTime < MIN_OFFTIME ) {
			FLSH_reopen("schedule") ;
			TIMR_doin(systemConfig.onTime + offTime,(JOB_Fxn)scheduleHibernate,NULL) ;
			return;
		}

		if(msp_getStartTime( &nextStartTime) != MSP_OK  ) {
			INFO_event("CONFIG", "MSP get start time failed", NULL);
			fatal(SCHED_MOD, 0); //restart
		}

		if(nextStartTime <= (TIMR_getUnixTime() + MIN_OFFTIME)) {
			INFO_event("CONFIG", "Error - bad restart time", NULL);
			fatal(SCHED_MOD, 1); //restart
		}

		if(msp_arm(ARMTIMER) != MSP_OK) {
			INFO_event("CONFIG", "MSP arm failed", NULL);
			fatal(SCHED_MOD, 2); //restart
		}
		TIMR_doin(SHUTDOWN_DELAY, JOB_terminator,NULL);
	}
}


int scheduleInit( int startFlags )
{
	Uint32 lastStartTime;
	Uint32 offTime = systemConfig.onceEveryTime - systemConfig.onTime;

	if( systemConfig.runMode == runModeTimed ) {

		if( offTime < MIN_OFFTIME ) {
			TIMR_doin(systemConfig.onTime + offTime,(JOB_Fxn)scheduleHibernate,NULL) ;
			return OK;
		}

		if(systemConfig.onTime > MIN_ONTIME ) {
			//prepare next restart

			if((msp_getStartTime(&lastStartTime) == MSP_OK) && ( abs(systemStartTime - lastStartTime) < 6)) { //if msp has correct start time
				systemStartTime = lastStartTime; //use last start time as stored on msp
			}
			else systemStartTime -= 2; //else use time recorded at when main() was called and assume it took 2s to boot

			if(msp_setStartTime(systemStartTime + systemConfig.onceEveryTime) != MSP_OK)  return FAIL;
			TIMR_doin(systemConfig.onTime, (JOB_Fxn)scheduleHibernate,NULL) ;
		}
		else {
			INFO_event("CONFIG", "ERROR, on time too short", NULL);
		}
	}
	return OK;
}

