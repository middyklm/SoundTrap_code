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

/*
 * Implements MSP temperature logger interface.
 *
 * JMJA
 *
 */

#include "i2c.h"
#include "d3defs.h"
#include "d3std.h"
#include "protect.h"
#include "timr.h"
#include "mspif.h"
#include "job.h"
#include "sensor.h"

#define TEMP_LOG_TIMEOUT 100
#define	BYTE2UNS(r)		((*((uns *)(r)+1)<<8)|(*((uns *)(r))))

static int tempId;
static i2cTrans trans;
static int address;
static int buff[4];
static sensorCallback tempCallback;

int postResult(Ptr p1, Ptr p2)
{
	char s[20];
	Int16 t;
	Uint16 i;
	Uint32 utime;
	if(trans.result == OK) {
		TIMR_gettime(&utime, NULL);
		t = msp_countToDegC(BYTE2UNS(buff));
		i = snprintf(s, 20,"%ld, %+5.1f\r", utime, (float)t/100.0) ;
		tempCallback(tempId, (Uint16 *)s, i);
	}
	return(OK) ;
}

//called by I2C ISR
void onI2cComplete(Ptr state)
{
	JOB_postone((JOB_Fxn)postResult,NULL,MAXNICE-1,NULL);
}

int tempLogTrigger(Ptr p1, Ptr p2)
{
	if(trans.result != -1) {
		address = TEMPR_ADDR;
		trans.slaveAddress = MSP_SLAVE_ADDR;
		trans.dataOut = &address;
		trans.lengthOut = 1;
		trans.dataIn = buff;
		trans.lengthIn = 2;
		trans.callback = onI2cComplete;
		trans.result = -1;
		i2cPostTrans(&trans);
	}
	return OK;
}

Ptr tempLogInit(int id, sensorCallback callback)
{
	tempId = id;
	trans.result = 0;
	tempCallback = callback;
	return NULL;
}



