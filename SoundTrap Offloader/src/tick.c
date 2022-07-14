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
#include <stdio.h>
#include <csl_general.h>
#include "timer.h"
#include "csl_pll.h"
#include "tick.h"

#define CSL_PLL_CLOCKIN    (32768u)
#define NO_OF_TIMER_CALLBACKS (10)

typedef struct 
{
	timerCallback callback;
	Uint32 msPeriod;
	Uint32 msLast;
	Uint16 repeat;
	Uint16 active;
} timerCallbackContext;

Uint16 timerCallbackCount;
timerCallbackContext timerCallbacks[NO_OF_TIMER_CALLBACKS];

Uint32 SysClk; 
Uint32 tickCount = 0;
Uint16 msTickCount = 0;
Uint32 unixTickCount = 0;
Uint16 tickProcessed;


Uint32 TIMR_GetTickCount()
{
	return tickCount;
}

Uint32 getTickCount()
{
	return tickCount;
}

void tickSetUnixTime(Uint32 time)
{
	msTickCount = 0;
	unixTickCount = time;
}

Uint32 tickGetUnixTime()
{
	return unixTickCount;
}

Uint32 TIMR_getUnixTime() 
{
	return tickGetUnixTime();
}	

void tickProcessCallbacks()
{
	Uint16 i;
	if(!tickProcessed)
	{
		for(i=0; i<NO_OF_TIMER_CALLBACKS; i++) {
			if(timerCallbacks[i].active == 1) {
				if( tickCount - timerCallbacks[i].msLast >= timerCallbacks[i].msPeriod) {
					timerCallbacks[i].msLast = tickCount;
					(timerCallbacks[i].callback)();
					if(!timerCallbacks[i].repeat == 1) {
						timerCallbacks[i].active = 0;
					}
				}
			}
		}
	}
	tickProcessed = 1;
}

void gpt0Isr(void)
{
	tickCount++;
	if(msTickCount++ == 1000) {
		msTickCount = 0;
		unixTickCount ++;
	}
	tickProcessed = 0;
}

void tickCancel(Uint16 handle)
{
	if(handle < NO_OF_TIMER_CALLBACKS) {
		timerCallbacks[handle].active = 0;
	}
}

Uint16 tickRunEvery(timerCallback callback, Uint32 ms)
{
	Uint16 i;
	for(i=0;i<NO_OF_TIMER_CALLBACKS;i++) {
		if(timerCallbacks[i].active == 0) {
			timerCallbacks[i].callback = callback;
			timerCallbacks[i].msPeriod = ms;
			timerCallbacks[i].msLast = tickCount;
			timerCallbacks[i].repeat = 1;
			timerCallbacks[i].active = 1;
			return i;
		}
	}
	return -1;
}

void tickRunOnce(timerCallback callback, Uint32 ms)
{
	Uint16 i;
	for(i=0;i<NO_OF_TIMER_CALLBACKS;i++) {
		if(timerCallbacks[i].active == 0) {
			timerCallbacks[i].callback = callback;
			timerCallbacks[i].msPeriod = ms;
			timerCallbacks[i].msLast = tickCount;
			timerCallbacks[i].repeat = 0;
			timerCallbacks[i].active = 1;
			return;
		}
	}
}


void tickInit(Uint32 sysClk)
{
	Timer_Config config;
		
	tickCount = 0;
	timerCallbackCount = 0;
	
	SysClk = sysClk;
	
	config.autoLoad = 1;
	config.prd = ( sysClk / 2 / 1000 ); //ms period;
	config.preScaleDiv = 0;

	timerConfig(0, config);
	timerInterruptHandlerRegister(0, gpt0Isr);
	timerStart(0);
}

void microdelay(Uint16 n)
{
	// approx n micro-second delay
	Uint32	k, nn ;
	Uint32 mhz = SysClk  / 1000000;
	nn = ((long)n*mhz)>>4 ;
	for(k=0;k<nn;k++) {
		asm("      NOP") ;
		asm("      NOP") ;
		asm("      NOP") ;
		asm("      NOP") ;
	}
}
