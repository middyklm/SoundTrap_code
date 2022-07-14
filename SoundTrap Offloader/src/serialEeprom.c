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

//Interface Module for TI 24AA256 I2C EEPROM

#include <tistdtypes.h>
#include <string.h>
#include "i2c.h"
#include "tick.h"
#include "d3std.h"


#define SLAVE_ADDRESS 0x54 //A2 asserted
#define TIMEOUT 0xFFF0
#define TIMEOUT_MS 5
#define FLASH_PAGE_LENGTH 64


Uint16 serialFlashBuf[FLASH_PAGE_LENGTH+2];

Uint16 serialEepromTestReady()
{
	Uint16 addressBuf[2];
	addressBuf[0] = 0;
	addressBuf[1] = 0;
	return i2cWrite(SLAVE_ADDRESS, addressBuf, 2) == OK;
}


Uint16 serialEepromWaitForReady()
{
	Uint32 startTime;
	startTime = getTickCount();
	while( !serialEepromTestReady() ) {
		if(getTickCount() - startTime > TIMEOUT_MS) {
			return 0;
		} 
	}
	return 1;
}

Uint16 serialEepromWrite(Uint16 address, Uint16 *buf, Uint16 count)
{
	Uint16 i;
	Uint16 *p;
	
	if( serialEepromWaitForReady() ) {
		if(count <= (FLASH_PAGE_LENGTH /2)) {
			serialFlashBuf[0] = address >> 8;
			serialFlashBuf[1] = address & 0x00FF;
			p = &serialFlashBuf[2];
			for(i=0;i<count;i++) {
				p[2*i] = buf[i] & 0x00ff;
				p[(2*i)+1] = (buf[i] >> 8) & 0x00ff;
			}
			return i2cWrite(SLAVE_ADDRESS, serialFlashBuf, (count*2) + 2) == OK;
		}
	}
	return 0;
}


Uint16 serialEepromRead(Uint16 address, Uint16 *buf, Uint16 count)
{
	Uint16 i;
	Uint16 addressBuf[2];

	addressBuf[0] = address>>8;
	addressBuf[1] = address & 0x00FF;
	
	if( serialEepromWaitForReady() ) {
		if(count <= (FLASH_PAGE_LENGTH/2)) {
			if( i2cWrite(SLAVE_ADDRESS, addressBuf, 2) == OK) {
				if( i2cRead(SLAVE_ADDRESS, serialFlashBuf, count*2) == OK ) {
					for(i=0;i<count;i++) {
						buf[i] = (serialFlashBuf[(2*i)+1] << 8 ) + serialFlashBuf[2*i];
					}
					return 1;
				}
			}
		}
	}
	return 0;
}

void serialEepromInit()
{
	
}
