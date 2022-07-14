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
#include "d3std.h"
#include "timr.h"
#include "dmem.h"
#include "minmax.h"


#ifdef OFFLOADER
#define SLAVE_ADDRESS 0x54 //A2 asserted USB ON
#else
#define SLAVE_ADDRESS 0x50 //A2 not asserted USB OFF
#endif

#define TIMEOUT 0xFFF0
#define TIMEOUT_MS 5
#define FLASH_PAGE_LENGTH 64




//Uint16 *serialFlashBuf;//[FLASH_PAGE_LENGTH+2];
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
	startTime = TIMR_GetTickCount();
	while( !serialEepromTestReady() ) {
		if(TIMR_GetTickCount() - startTime > TIMEOUT_MS) {
			return 0;
		}
	}
	return 1;
}

Uint16 serialEepromWrite(Uint16 address, Uint16 *buf, Uint16 count)
{
	Uint16 i;
	Uint16 *p;
	Uint16 n;
	Uint16 ww = 0;

	count = count * 2; //convert from words to bytes

	while(count) {

		if( !serialEepromWaitForReady() ) {
			return FAIL;
		}

		n = MIN(count, FLASH_PAGE_LENGTH);
		serialFlashBuf[0] = address >> 8;
		serialFlashBuf[1] = address & 0x00FF;
		p = &serialFlashBuf[2];
		for(i=0;i<(n/2);i++) {
			p[2*i] = buf[i + ww] & 0x00ff;
			p[(2*i)+1] = (buf[i + ww] >> 8) & 0x00ff;
		}
		if(i2cWrite(SLAVE_ADDRESS, serialFlashBuf, n + 2) != OK) {
			return FAIL;
		}
		address += n;
		count -= n;
		ww+= n/2;
	}
	return OK;
}


Uint16 serialEepromRead(Uint16 address, Uint16 *buf, Uint16 count)
{
	Uint16 i;
	Uint16 addressBuf[2];
	Uint16 n;
	Uint16 wr = 0;

	count = count * 2; //convert from words to bytes

	if( !serialEepromWaitForReady() ) {
		return FAIL;
	}

	addressBuf[0] = address>>8;
	addressBuf[1] = address & 0x00FF;

	if( !i2cWrite(SLAVE_ADDRESS, addressBuf, 2) == OK) {
		return FAIL;
	}

	while(count) {
		n = MIN(count, FLASH_PAGE_LENGTH);

		if( !i2cRead(SLAVE_ADDRESS, serialFlashBuf, n) == OK ) {
			return FAIL;
		}

		for(i=0; i<(n/2); i++) {
			buf[wr + i] = (serialFlashBuf[(2*i)+1] << 8 ) + serialFlashBuf[2*i];
		}

		count -= n;
		//address += n;
		wr += n/2;
	}
	return OK;
}

int serialEepromInit()
{
	//if((serialFlashBuf=(Uint16 *)DMEM_alloc(FLASH_PAGE_LENGTH+2))==NULL)
		// return(FAIL) ;
	return OK;
}
