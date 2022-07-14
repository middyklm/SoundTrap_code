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
 * Implements I2C hardware interface.
 * 
 * JMJA
 * 
 */


#include <tistdtypes.h>
#include "d3defs.h"
#include "d3std.h"
#include "misc.h"
#include "error.h"
#include "i2c.h"
#include "sysControl.h"


#ifndef NULL
	#define NULL ((void*)0)
#endif

#define I2C_TIMEOUT 10000 // 1s

#define MOD_CLOCK 8000000

#define ICSTR_NACK 0x0002
#define ICSTR_ARDY 0x0004
#define ICSTR_ICRRDY 0x0008
#define ICSTR_ICXRDY 0x0010
#define ICSTR_SCD 0x0020
#define ICSTR_BB 0x1000

#define ICMDR_FREE 0x4000
#define ICMDR_IRS 0x0020
#define ICMDR_TRX 0x0200
#define ICMDR_MST 0x0400
#define ICMDR_STP 0x0800
#define ICMDR_STT 0x2000

typedef struct  {
    volatile Uint16 ICOAR;
    volatile Uint16 RSVD0[3];
    volatile Uint16 ICIMR;
    volatile Uint16 RSVD1[3];
    volatile Uint16 ICSTR;
    volatile Uint16 RSVD2[3];
    volatile Uint16 ICCLKL;
    volatile Uint16 RSVD3[3];
    volatile Uint16 ICCLKH;
    volatile Uint16 RSVD4[3];
    volatile Uint16 ICCNT;
    volatile Uint16 RSVD5[3];
    volatile Uint16 ICDRR;
    volatile Uint16 RSVD6[3];
    volatile Uint16 ICSAR;
    volatile Uint16 RSVD7[3];
    volatile Uint16 ICDXR;
    volatile Uint16 RSVD8[3];
    volatile Uint16 ICMDR;
    volatile Uint16 RSVD9[3];
    volatile Uint16 ICIVR;
    volatile Uint16 RSVD10[3];
    volatile Uint16 ICEMDR;
    volatile Uint16 RSVD11[3];
    volatile Uint16 ICPSC;
    volatile Uint16 RSVD12[3];
    volatile Uint16 ICPID1;
    volatile Uint16 RSVD13[3];
    volatile Uint16 ICPID2;
} I2cRegs;

typedef volatile ioport I2cRegs *I2cRegsOvly;
I2cRegsOvly i2cRegs;

Bool i2cWaitForBus()
{
	Uint16 i;
	for(i=0; i<I2C_TIMEOUT; i++)
	{
	    //if((!(i2cRegs->ICSTR & ICSTR_BB)) && (i2cRegs->ICMDR & ICMDR_STP == 0x0000) ) {
	    if((i2cRegs->ICMDR & ICMDR_MST) == 0x0000) {
			return OK;
		}
		microdelay(100);
	}
	return FAIL;
}
	
//blocking - doesn't use interrupts
int i2cRead(Uint16 slaveAddress, Uint16 *buf, Uint16 count)
{
	Uint16 i;
	if((buf != NULL) && (count != 0))
	{
		if( i2cWaitForBus() == OK) {
			i2cRegs->ICMDR = 0x0000;
			i2cRegs->ICSAR = slaveAddress;
			i2cRegs->ICCNT = count;
			i2cRegs->ICMDR = ICMDR_IRS | ICMDR_MST | ICMDR_STP | ICMDR_STT | ICMDR_FREE;

			for(i=0; i<count; i++)
			{
				while(!(i2cRegs->ICSTR & (ICSTR_ICRRDY | ICSTR_ARDY)));
				if ( i2cRegs->ICSTR & ICSTR_NACK )
				{
					i2cRegs->ICMDR = ICMDR_STP; //send stop
					i2cRegs->ICSTR = ICSTR_NACK; //clear NACK
					return FAIL;
				}
				*buf++ = i2cRegs->ICDRR;
			}
			return OK;
		}
	}
	return FAIL;
}

//blocking - doesn't use interrupts
int i2cWrite(Uint16 slaveAddress, Uint16 *buf, Uint16 count)
{
	Uint16 i;
	if((buf != NULL))// && (count != 0))
	{
		if( i2cWaitForBus( ) ==  OK) {
			while(!(i2cRegs->ICSTR & (ICSTR_ICXRDY | ICSTR_ARDY)));
			i2cRegs->ICMDR = 0x0000;
			i2cRegs->ICSAR = slaveAddress;
			i2cRegs->ICCNT = count;
			i2cRegs->ICMDR = ICMDR_IRS | ICMDR_MST | ICMDR_TRX | ICMDR_STP | ICMDR_STT | ICMDR_FREE;
			for(i=0; i<count; i++)
			{
				while(!(i2cRegs->ICSTR & (ICSTR_ICXRDY | ICSTR_ARDY)));
				// If a NACK occurred then SCL is held low and STP bit cleared
				if ( i2cRegs->ICSTR & ICSTR_NACK )
				{
					i2cRegs->ICMDR = ICMDR_STP; //send stop
					i2cRegs->ICSTR = ICSTR_NACK; //clear NACK
					return FAIL;
				}			
				i2cRegs->ICDXR = *buf++;
			}
			return OK;
		}
	}
	return FAIL;
}

// blocking - doesn't use interrupts
int i2cWriteRead(Uint16 slaveAddress, Uint16 *outBuf, Uint16 outCount, Uint16 *inBuf, Uint16 inCount)
{
	Uint16 i;
	if(outBuf != NULL)
	{
		if( i2cWaitForBus() ==  OK) {
			while(!(i2cRegs->ICSTR & (ICSTR_ICXRDY | ICSTR_ARDY)));
			i2cRegs->ICMDR = 0x0000;
			i2cRegs->ICSAR = slaveAddress;
			i2cRegs->ICCNT = outCount;
			i2cRegs->ICMDR = ICMDR_IRS | ICMDR_MST | ICMDR_TRX | ICMDR_STT | ICMDR_FREE;
			for(i=0; i<outCount; i++)
			{
				while(!(i2cRegs->ICSTR & (ICSTR_ICXRDY | ICSTR_ARDY)));
				// If a NACK occurred then SCL is held low and STP bit cleared
				if ( i2cRegs->ICSTR & ICSTR_NACK )
				{
					i2cRegs->ICMDR = ICMDR_STP; //send stop
					i2cRegs->ICSTR = ICSTR_NACK; //clear NACK
					return FAIL;
				}
				i2cRegs->ICDXR = *outBuf++;
			}

			while(!(i2cRegs->ICSTR & ICSTR_ARDY));

			// issue restart

			i2cRegs->ICMDR = 0x0000;
			i2cRegs->ICSAR = slaveAddress;
			i2cRegs->ICCNT = inCount;
			i2cRegs->ICMDR = ICMDR_IRS | ICMDR_MST | ICMDR_STP | ICMDR_STT | ICMDR_FREE;

			for(i=0; i<inCount; i++)
			{
				while(!(i2cRegs->ICSTR & (ICSTR_ICRRDY | ICSTR_ARDY)));
				if ( i2cRegs->ICSTR & ICSTR_NACK )
				{
					i2cRegs->ICMDR = ICMDR_STP; //send stop
					i2cRegs->ICSTR = ICSTR_NACK; //clear NACK
					return FAIL;
				}
				*inBuf++ = i2cRegs->ICDRR;
			}
			return OK;

		}
	}
	return FAIL;
}


int i2cInit(Uint32 clk)
{
	Int16 div = (clk / MOD_CLOCK) - 1;
	if(div < 0) return FAIL;
	
	i2cRegs = (I2cRegsOvly)0x1A00;
	//sysControlResetPeripheral(0x00); 

	i2cRegs->ICMDR = 0x4000; 	//0100 0000 0000 0000
	i2cRegs->ICOAR = 0x2F;
	i2cRegs->ICIMR = 0x00;
	i2cRegs->ICSTR = 0xFFFF; 	//reset status
	i2cRegs->ICCLKL = 11;
	i2cRegs->ICCLKH = 11;     //250kHz (assuming ICPSC >= 2)
	i2cRegs->ICPSC = div; 
	i2cRegs->ICEMDR = 0x0000;
	i2cRegs->ICMDR |= 0x0020; //Enable (IRS=1)
	return OK;
}
