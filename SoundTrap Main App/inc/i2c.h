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

#ifndef _I2C_H_
#define _I2C_H_

#include <tistdtypes.h>


typedef	void (*i2cCallback)(Ptr state) ;

typedef struct  {
	Uint16 slaveAddress;
	int *dataOut;
	Uint16 lengthOut;
	int *dataIn;
	Uint16 lengthIn;
	i2cCallback callback;
	Ptr state;
	volatile int result;
} i2cTrans;



int i2cRead(Uint16 slaveAddress, Uint16 *buf, Uint16 count);
int i2cWrite(Uint16 slaveAddress, Uint16 *buf, Uint16 count);
int i2cWriteRead(Uint16 slaveAddress, Uint16 *outBuf, Uint16 outCount, Uint16 *inBuf, Uint16 inCount);
int i2cPostTrans(i2cTrans *trans);
int i2cInit(Uint32 clk); 

#endif
