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
 * Implements KXTI9-1001 accelerometer hardware interface.
 *
 * JMJA
 *
 */

#include "i2c.h"
#include "d3defs.h"
#include "d3std.h"
#include "protect.h"
#include "job.h"
#include "sensor.h"
#include "accelerometer.h"
#include "timr.h"

#define SAD 0x000F
#define LSM303CSAD 0x001D
#define ON_MODE 0xC0 //12 bit mode

static i2cTrans trans;
static TaccelData d;
static sensorCallback accelCallback;
static int accelId;
static int buff[12];
static int address;
static accelRange range = G2;
static int lsm303c = 0;

int accelWakeup()
{
	Uint16 buf[2];
	Uint16 temp;
	//blocks
	buf[0] = 0x0F;
	if( i2cWriteRead(LSM303CSAD, buf, 1, &temp, 1) == OK) {
		if(temp == 0x41) lsm303c = 1;
	}

	buf[0] = lsm303c ? 0x20 : 0x1B;
	buf[1] = lsm303c ? 0x1F : (ON_MODE | (range<<3));
	return i2cWrite(lsm303c ? LSM303CSAD : SAD, buf, 2);
}

int accelSleep()
{
	Uint16 buf[2];
	buf[0] = lsm303c ? 0x20 : 0x1B;
	buf[1] = 0x00;
	return i2cWrite(lsm303c ? LSM303CSAD : SAD, buf, 2);
}

int accelPostResult(Ptr p1, Ptr p2)
{
	char s[60] ;
	int i;
	Uint32 utime;
	if(trans.result == OK) {
		TIMR_gettime(&utime, NULL);
		if( lsm303c ) {
			d.x = (Int16)(buff[0] << 8) + buff[1];
			d.y = (Int16)(buff[2] << 8) + buff[3];
			d.z = (Int16)(buff[4] << 8) + buff[5];
		}
		else {
			d.x = (Int16)(buff[1] << 8) + buff[0];
			d.y = (Int16)(buff[3] << 8) + buff[2];
			d.z = (Int16)(buff[5] << 8) + buff[4];
		}
		i = snprintf(s, 60,"%ld, %+06d,%+06d,%+06d\r\n", utime, d.x, d.y, d.z) ;
		accelCallback(accelId, (Uint16 *)s, i);
	}
	return(OK) ;
}

//called by I2C ISR
void accelOnI2cComplete(Ptr state)
{
	JOB_postone((JOB_Fxn)accelPostResult,NULL,MAXNICE-1,NULL);
}

int accelTrigger( Ptr p1, Ptr p2 )
{
	if(trans.result != -1) {
		address = lsm303c ? 0x29 : 0x06;
		trans.slaveAddress = lsm303c ? LSM303CSAD : SAD;
		trans.dataOut = &address;
		trans.lengthOut = 1;
		trans.dataIn = buff;
		trans.lengthIn = 6;
		trans.callback = accelOnI2cComplete;
		trans.result = -1;
		i2cPostTrans(&trans);
	}
	return OK;
}

int	accelRead(TaccelData *ret)
{
	Uint16 address[1];
	address[0] = lsm303c ? 0x29 : 0x06;

	i2cWrite(lsm303c ? LSM303CSAD : SAD, address, 1);
	i2cRead(lsm303c ? LSM303CSAD : SAD, (Uint16*)buff, 6);

	if(lsm303c) {
		d.x = (Int16)(buff[0] << 8) + buff[1];
		d.y = (Int16)(buff[2] << 8) + buff[3];
		d.z = (Int16)(buff[4] << 8) + buff[5];
	}
	else {
		d.x = (Int16)(buff[1] << 8) + buff[0];
		d.y = (Int16)(buff[3] << 8) + buff[2];
		d.z = (Int16)(buff[5] << 8) + buff[4];
	}
	return OK;
}


void cb(Uint16 id, Uint16 *buf, Uint16 length)
{

}

Ptr accelInit(int id, sensorCallback callback)
{
	trans.result = 0;
	accelId = id;
	accelCallback = callback;
	return NULL;
}
