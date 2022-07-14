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
#include "sysControl.h"
#include "csl_intc.h"
#include "csl_general.h"
#include "protect.h"
#include "data.h"
#include "i2c.h"
#include "timr.h"
#include "devdep.h"


#ifndef NULL
	#define NULL ((void*)0)
#endif

#define MOD_CLOCK 8000000

#define ICSTR_AL 0x0001
#define ICSTR_NACK 0x0002
#define ICSTR_ARDY 0x0004
#define ICSTR_ICRRDY 0x0008
#define ICSTR_ICXRDY 0x0010
#define ICSTR_SCD 0x0020
#define ICSTR_BB 0x1000

#define ICMDR_FDF 0x0004
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

#define QUEUE_LENGTH 10

static int transInProgress = 0;
static i2cTrans* transQueue[QUEUE_LENGTH];
static int queueHead = 0;
static int queueTail = 0;
static int queueCount = 0;

i2cTrans *curTrans;

int i2cWaitForBus(Uint16 timeout)
{
	Uint16 i;
	for(i=0; i<timeout; i++)
	{
		if((i2cRegs->ICMDR & ICMDR_MST) == 0x0000) {
			return OK;
		}
		microdelay(100);
	}
	return FAIL;
}


void i2cStart(i2cTrans *trans)
{
    curTrans = trans;
    if( curTrans->lengthOut ) {
		i2cRegs->ICMDR = 0x0000;
		i2cRegs->ICSAR = curTrans->slaveAddress;
		i2cRegs->ICCNT = curTrans->lengthOut;
     	i2cRegs->ICMDR = ICMDR_IRS | ICMDR_MST | ICMDR_TRX | ICMDR_FREE;
     	//if nothing to receive, end with a stop, otherwise we'll do a re-start
     	if(curTrans->lengthIn == 0) i2cRegs->ICMDR |= ICMDR_STP;
    	i2cRegs->ICIMR = CSL_I2C_ICIMR_AL_MASK | CSL_I2C_ICIMR_ICXRDY_MASK  | CSL_I2C_ICIMR_NACK_MASK | CSL_I2C_ICIMR_ICRRDY_MASK | CSL_I2C_ICIMR_SCD_MASK;
    	i2cRegs->ICIVR = 0; // Clear IVR before doing a start
    	i2cRegs->ICMDR |= ICMDR_STT;

    }
    else if(curTrans->lengthIn)
    {
		i2cRegs->ICMDR = 0x0000;
		i2cRegs->ICSAR = curTrans->slaveAddress;
		i2cRegs->ICCNT = curTrans->lengthIn;
		i2cRegs->ICIMR = CSL_I2C_ICIMR_AL_MASK | CSL_I2C_ICIMR_NACK_MASK | CSL_I2C_ICIMR_ICRRDY_MASK | CSL_I2C_ICIMR_SCD_MASK;
		i2cRegs->ICIVR = 0; // Clear IVR before doing a start
		i2cRegs->ICMDR = ICMDR_IRS | ICMDR_MST | ICMDR_STP | ICMDR_STT | ICMDR_FREE;
    }
}

void i2cOnTransComplete(int result)
{
	i2cRegs->ICIMR = 0; //mask all i2c interrupts
	curTrans->result = result;

	if(curTrans->callback!= NULL) {
		curTrans->callback( curTrans->state );
	}

	if(++queueHead == QUEUE_LENGTH)
		queueHead = 0;
	queueCount--;
	if(queueCount)
		i2cStart(transQueue[queueHead]);
	else transInProgress = 0;
}


interrupt void i2c_isr(void)
{
	PROTECT;
	volatile Uint32 intStatus;

	intStatus = i2cRegs->ICIVR;
	while (intStatus != 0)
	{
		switch (intStatus) {
			case CSL_I2C_ICIVR_INTCODE_NONE:
				// No interrupt - Ideally, control should not come here.
				break;

			case CSL_I2C_ICIVR_INTCODE_AL:      // Arbitration loss
				i2cRegs->ICMDR |= ICMDR_STP; //send stop
				i2cRegs->ICSTR = ICSTR_AL;	// clear AL bit
				i2cOnTransComplete(1);
				break;

			case CSL_I2C_ICIVR_INTCODE_NACK:    // NACK interrupt
				i2cRegs->ICMDR |= ICMDR_STP;	// send STP to end transfer
				i2cRegs->ICSTR = ICSTR_NACK;	// clear NACK bit
				i2cOnTransComplete(2);
				break;

			case CSL_I2C_ICIVR_INTCODE_ARDY:
				// Register Access ready - tx is complete, do the rx
				if (curTrans->lengthOut == 0) {
					if(curTrans->lengthIn) {
						// done the tx, now lets rx - send a restart
						i2cRegs->ICMDR = 0x0000;
						i2cRegs->ICSAR = curTrans->slaveAddress;
						i2cRegs->ICCNT = curTrans->lengthIn;
						i2cRegs->ICMDR = ICMDR_IRS | ICMDR_MST | ICMDR_STP | ICMDR_STT | ICMDR_FREE;
					}
				}
				// Disable register access ready interrupt, we only need it the once
				i2cRegs->ICIMR &= ~CSL_I2C_ICIMR_ARDY_MASK;
				break;

			case CSL_I2C_ICIVR_INTCODE_RDR:
				// Receive Data ready
				if (curTrans->lengthIn != 0) {
					*curTrans->dataIn = (Uint16)(i2cRegs->ICDRR);
					curTrans->dataIn++;
					curTrans->lengthIn--;
					if (curTrans->lengthIn == 0) {
						// Receive complete
						// Disable Receive interrupt
						i2cRegs->ICIMR &= ~CSL_I2C_ICIMR_ICRRDY_MASK;
					}
				}
				break;

			case CSL_I2C_ICIVR_INTCODE_TDR:
				// Transmit Data ready
				if (curTrans->lengthOut != 0) {
					i2cRegs->ICDXR = *curTrans->dataOut;
					curTrans->dataOut++;
					curTrans->lengthOut--;
					if(curTrans->lengthOut == 0) {
						// transmit is complete
						// Disable Transmit interrupt
						i2cRegs->ICIMR &= ~CSL_I2C_ICIMR_ICXRDY_MASK;
						// Enable the register access ready interrupt
						i2cRegs->ICIMR |= CSL_I2C_ICIMR_ARDY_MASK;
					}
				}
				break;

			case CSL_I2C_ICIVR_INTCODE_SCD:
				// Stop condition detected
				// Disable this interrupt enable bit
				i2cRegs->ICIMR &= ~CSL_I2C_ICIMR_SCD_MASK;
 				i2cOnTransComplete(OK);
				break;

			default:
				break;
		}
		intStatus = i2cRegs->ICIVR;
	}
	END_PROTECT ;
}

//non-blocking - interrupts must be enabled
int i2cPostTrans(i2cTrans *trans)
{
	if(queueCount < QUEUE_LENGTH)
	{
		GO_ATOMIC;	// disable interrupts while we are messing with the queues
		transQueue[queueTail] = trans;
		transQueue[queueTail]->result = -1;
		if(++queueTail == QUEUE_LENGTH)
			queueTail = 0;
		queueCount++;
		END_ATOMIC;

		if(!transInProgress) {
			transInProgress = 1;
			i2cStart(transQueue[queueHead]);
		}
		return OK;
	}
	return FAIL;
}


i2cTrans blockingTrans;

//blocks
int i2cRead(Uint16 slaveAddress, Uint16 *buf, Uint16 count)
{
	blockingTrans.slaveAddress = slaveAddress;
	blockingTrans.dataIn = (int *)buf;
	blockingTrans.lengthIn = count;
	blockingTrans.lengthOut = 0;
	blockingTrans.dataOut = NULL;
	blockingTrans.callback = NULL;
	blockingTrans.result = -1;
	if( i2cPostTrans(&blockingTrans) ) return FAIL;
	while(blockingTrans.result == -1) {
		microdelay(100);
	}
	return blockingTrans.result;
}

//blocks
int i2cWrite(Uint16 slaveAddress, Uint16 *buf, Uint16 count)
{
	blockingTrans.slaveAddress = slaveAddress;
	blockingTrans.dataOut = (int *)buf;
	blockingTrans.lengthOut = count;
	blockingTrans.lengthIn = 0;
	blockingTrans.dataIn = NULL;
	blockingTrans.callback = NULL;
	blockingTrans.result = -1;
	if( i2cPostTrans(&blockingTrans) ) return FAIL;
	while(blockingTrans.result == -1) {
		microdelay(100);
	}
	return blockingTrans.result;
}

//blocks
int i2cWriteRead(Uint16 slaveAddress, Uint16 *outBuf, Uint16 outCount, Uint16 *inBuf, Uint16 inCount)
{
	blockingTrans.slaveAddress = slaveAddress;
	blockingTrans.dataOut = (int *)outBuf;
	blockingTrans.lengthOut = outCount;
	blockingTrans.dataIn = (int *)inBuf;
	blockingTrans.lengthIn = inCount;
	blockingTrans.callback = NULL;
	blockingTrans.result = -1;
	if( i2cPostTrans(&blockingTrans) ) return FAIL;
	while(blockingTrans.result == -1) {
		microdelay(100);
	}
	return blockingTrans.result;
}

int i2cInit(Uint32 clk)
{
	Int16 div = (clk / MOD_CLOCK) - 1;
	if(div < 0) return FAIL;

	//busy = 0;
	
	i2cRegs = (I2cRegsOvly)0x1A00;
	//sysControlResetPeripheral(0x00); 

	i2cRegs->ICMDR = 0x4000; 	//0100 0000 0000 0000
	i2cRegs->ICOAR = 0x2F;
	i2cRegs->ICIMR = 0x00;
	i2cRegs->ICSTR = 0xFFFF; 	//reset status
	i2cRegs->ICCLKL = 35;
	i2cRegs->ICCLKH = 35;     //100kHz (assuming ICPSC >= 2)
	i2cRegs->ICPSC = div; 
	i2cRegs->ICEMDR = 0x0000;
	i2cRegs->ICMDR |= 0x0020; //Enable (IRS=1)

	//i2cDeJam();

	IRQ_plug (I2C_EVENT, &i2c_isr);
	IRQ_enable(I2C_EVENT);
	return OK;
}
