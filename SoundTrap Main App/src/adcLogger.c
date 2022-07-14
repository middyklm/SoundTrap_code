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
 * Implements MCP3424 ADC hardware interface.
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
#include "timr.h"
#include "dmem.h"
#include "cfg.h"

#define MCP3424_ADD 0x0068

#define ADC_0_ADD 0x0068
#define ADC_1_ADD 0x0069




#define MAX_CHANNELS 4

static float scale = 1;

typedef struct
{
	int id;
	int adcAdddress;
	int deviceNo;
	int buff[4];
	int channel;
	int lastChannel;
	float readings[MAX_CHANNELS];
	sensorCallback adcLoggerCallback;
	int command;
	i2cTrans trans;
} adcLogConfig;

int adcLoggerLoggerPostResult(Ptr p1, Ptr p2)
{
	char s[60] ;
	int i;
	//Int16 v;
	Int32 v;



	adcLogConfig *c = (adcLogConfig *)p1;
	float *readings = c->readings;

	Uint32 utime;
	if(c->trans.result == OK) {
		TIMR_gettime(&utime, NULL);

		//v =  (Int16)(buff[0] << 8) + buff[1]; //16 bit

		v = c->buff[0];
		v <<= 8;
		v += c->buff[1];
		v <<= 8;
		v += c->buff[2];
		v <<= 8;
		//v /= 16384;

		//readings[c->lastChannel] = scale * v;
		readings[c->lastChannel] = *((Int32 *) &v);
		readings[c->lastChannel] /= 256;
		readings[c->lastChannel] *= scale;

		if(c->lastChannel == 3) {
			i = snprintf(s, 60,"%ld, %4.2f, %4.2f, %4.2f, %4.2f\r\n", utime, readings[0], readings[1], readings[2], readings[3]) ;
			c->adcLoggerCallback(c->id, (Uint16 *)s, i);
		}

	}
	return(OK) ;
}

//called by I2C ISR
void adcLoggerOnI2cComplete(Ptr state)
{
	JOB_postone((JOB_Fxn)adcLoggerLoggerPostResult, state, MAXNICE-1, NULL);
}

int adcLoggerTrigger( Ptr p1, Ptr p2 )
{
	adcLogConfig* c = (adcLogConfig *)p1;

	if(c->trans.result != -1) {
		c->lastChannel = c->channel;
		if(++c->channel == 4) c->channel = 0;
		//command = 0x008B;//  1000 1011 16 bit
		c->command = ((c->channel << 5) & 0x60)  | 0x008F;//  1000 1111 18 bit
		c->trans.slaveAddress = c->deviceNo ? ADC_1_ADD : ADC_0_ADD;
		c->trans.dataOut = &c->command;
		c->trans.lengthOut = 1;
		c->trans.dataIn = c->buff;
		c->trans.lengthIn = 4;
		c->trans.callback = adcLoggerOnI2cComplete;
		c->trans.state = c;
		c->trans.result = -1;
		i2cPostTrans(&c->trans);
	}
	return OK;
}

int adcLoggerSetScale(float scaleParam)
{
	scale = scaleParam;
	return OK;
}


Ptr adcLoggerInit(int id, sensorCallback callback)
{
	static int deviceNo = 0;
	adcLogConfig* c;

    // allocate a structure for the audio object
    if ((c = (adcLogConfig *) DMEM_alloc(sizeof(adcLogConfig))) == NULL) {
        //err(AUDIO_MOD, OPENALLOCFAIL);
        return (NULL);
    }

    c->id = id;
	c->adcLoggerCallback = callback;
	c->deviceNo = deviceNo;
	c->trans.result = 0;
	c->channel = 0;
    deviceNo++;

    return (Ptr)c;
}




