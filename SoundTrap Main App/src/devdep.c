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


#include		<stdlib.h>

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
#include "hid.h"

#define GREEN_LED_BIT 13
// local copy of the 8 or 16 bit control register(s) on the audio board
Uint16 audiocreg = 0 ;
Uint16 mcreg = 0;

enum AUDIO_IO_BITS
{
	IO_BIT_AUDIO_ON,
	IO_BIT_GAIN,
	IO_BIT_CAL_CHAN,
	IO_BIT_HIGHPASS,
	IO_BIT_MUTE,
	IO_BIT_CAL_ON,
	IO_BIT_ID1,
	IO_BIT_ID2
};

int	audiocregandor(uns andm, uns orm)
{
	// change the audio control register by and and or masks
	int	ret = OK ;
	Uint16	b, c ;

	c = (audiocreg & ~andm) | orm ;
	b = c^audiocreg ;

	if((b&0x0ff)!=0) {
		ret = ( ioeWrite(AUDIO_IO_EXPANDER_DEVICE_ADDRESS, c) == TRUE ) ? OK : FAIL;
	}
 	audiocreg = c ;
 	return(ret) ;
}

int	audiocregwrite(uns c, int mtype)
{
 	if(mtype)
		return(audiocregandor(0,c)) ;
	else
 		return(audiocregandor(c,0)) ;
}

Uint32 getrtctime(void)
{
	MSP_Time t;
	if(msp_gettime(&t,NULL,NULL)==MSP_FAIL)
		return(0ul);
	return(t.rtime);
}

void reboot(int on)
{
	msp_SendRequest(REBOOT_REQ);
}

int	leds(int which)
{
	gpioSetVal(GPIO_BIT_GREEN_LED, which & GREENLED); 
	//MSP Switch Red LED;	//TODO
	return OK;
}

int	getbattery(void)
{
 	// returns battery voltage in mV
 	int	v;
	MSP_Sensors s;
	if(msp_getsensors(&s)==MSP_FAIL)
		return(-1);
	v = s.vb;
 	return(v);
}

void powerdown(void)
{
	msp_SendRequest(PWROFF_REQ);
 	exit(1);			// should never get here
}

int	ispowered(void)
{
 	return(CHG_USB | CHG_EXT);	// complete lie - need to actually check
}

int	mute(int on)
{
 	return(audiocregwrite(1 << IO_BIT_MUTE, on));
}

int	enableCalDAC(int on)
{
 	return(audiocregwrite(1 << IO_BIT_CAL_ON, on));
}

int	switchCalChan(int ext_on)
{
 	return(audiocregwrite(1 << IO_BIT_CAL_CHAN, ext_on));
}

int	setgain(int channels, int gainOn)
{
	if(channels & 0x0001 == 1) //only chan 1 has gain control
		return(audiocregwrite(1<<IO_BIT_GAIN, gainOn));
	else return OK;
}

int setHPass(int channels, int hpass)
{
	if(channels & 0x0001 == 1) //only chan 1 has hpass control
		return(audiocregwrite(1<<IO_BIT_HIGHPASS, hpass & 0x0001));
	else return OK;
}

int getAudioHardwareId(Uint16 *id)
{
	Uint16 buf;
	if( ioeRead(AUDIO_IO_EXPANDER_DEVICE_ADDRESS, &buf) != TRUE ) return FAIL;
	*id = ( buf & 0xc0 ) >>6;
	return OK;
}

float getgain(int level)
{
	return 1.0; //TODO
}

int	audiochanpwr(int ch, int on)
{
 	return(OK); 
}


int	audiopwr(int channel, int on)
{
	if( channel == 0 )
		return audiocregwrite(1 << IO_BIT_AUDIO_ON, on) ;
	else {
		mcreg |=  1 << (channel - 1);
		return ( ioeWrite(AUDIO_IO_EXPANDER_DEVICE_ADDRESS_MC, mcreg) == TRUE ) ? OK : FAIL;
	}
}



int	isvalidchans(uns chans)
{
	return chans == 0x0001;
}

int	audiosync(int mode, int on)
{
	return(0) ;
}

int	audionchans(void)
{
 	return 1;
}

int	sensorpwr(int on)
{
 	return 1;
}

int	ecgpwr(int on)
{
 	return 1;
}

int	audiocreginit(int channel)
{
/*
	if(channel == 0)
		return ( ioeInit(AUDIO_IO_EXPANDER_DEVICE_ADDRESS, 0x00C0, 0x0000) == TRUE); //1-6 output, 7&8 input
	else if(channel == 1)
		return ( ioeInit(AUDIO_IO_EXPANDER_DEVICE_ADDRESS_MC, 0x00C0, 0x0000) == TRUE); //1-6 output, 7&8 input
	else return OK;
*/
}

int	devdep_init(void)
{
	audiocreg = 0;

	if( ioeInit(AUDIO_IO_EXPANDER_DEVICE_ADDRESS, 0x00C0, 0x0000) != TRUE ) {
		return FAIL;
	}


	if ((hid == ST4300) || (hid == ST500))
	{
		//set outputs
		if( ioeInit(AUDIO_IO_EXPANDER_DEVICE_ADDRESS_MC, 0x00C0, 0x0000) != TRUE ) {
			return FAIL;
		}
	}

	if(hid == ST500) {
		//turn on card pullups
		if( ioeWritePU(AUDIO_IO_EXPANDER_DEVICE_ADDRESS, 0x38) != TRUE) {
			return FAIL;
		}
	}

	//if( audiocreginit(0) != TRUE ) return FAIL;
 	return(OK);
}
