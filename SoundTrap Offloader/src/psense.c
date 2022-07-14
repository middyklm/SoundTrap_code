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
#include "spi.h"
#include "tick.h"

#define CS 3
#define OK 0


int psenseStartConversion()
{
	Uint16 data[2] = {0xFF08, 0x200A};
	return spiWrite(CS, data, 2);
}

int psenseRead(Uint16 *result)
{
	Uint16 data[1] = {0xFF58};
	spiWrite(CS, data, 1);
	return spiRead(CS, result, 1);
}

int psenseInit()
{
	Uint16 data[2] = {0xffff, 0xffff}; 

	spiWrite(CS, data, 2); //reset
	microdelay(500);

	//data[0] = 0x2800;
	//spiWrite(CS, data, 1);

	data[0] = 0xff10; //config reg
	data[1] = 0x0401; //gain=16, use external ref, input channel=AIN2
	spiWrite(CS, data, 2);

	data[0] = 0x2802; //io reg, current excitation enable 210 uA x 2
	return spiWrite(CS, data, 1);


}
