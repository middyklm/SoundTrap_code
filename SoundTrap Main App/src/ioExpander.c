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
 * Implements interface for MCP23009 8 bit I/O Expander 
 * 
 * JMJA
 *  
 */

#include <tistdtypes.h>
#include "ioExpander.h"
#include  "d3defs.h"
#include "i2c.h"
#include "d3std.h"
#include "gpio.h"



enum REG_ADDRESS {
	REG_ADDRESS_IODIR,
	REG_ADDRESS_IPOL,
	REG_ADDRESS_GPINTEN,
	REG_ADDRESS_DEFCAL,
	REG_ADDRESS_INTCON,
	REG_ADDRESS_IOCON,
	REG_ADDRESS_GPPU,
	REG_ADDRESS_INTF,
	REG_ADDRESS_INTCAP,
	REG_ADDRESS_GPIO,
	REG_ADDRESS_OLAT 
};

Bool ioeRead(Uint16 deviceAddress, Uint16 *portVal)
{
	Uint16 regAddress = REG_ADDRESS_GPIO;
	if( i2cWrite(deviceAddress, &regAddress, 1) != OK) return FALSE;
	if( i2cRead(deviceAddress, portVal, 1) != OK) return FALSE;
	return TRUE;
}

Bool ioeWritePU(Uint16 deviceAddress, Uint16 portVal)
{
	Uint16 buf[2];
	buf[0] = REG_ADDRESS_GPPU;
	buf[1] = portVal & 0x00FF;
	if( i2cWrite(deviceAddress, buf, 2) != OK) return FALSE;
	return TRUE;
}


Bool ioeWrite(Uint16 deviceAddress, Uint16 portVal)
{
	Uint16 buf[2];
	buf[0] = REG_ADDRESS_OLAT;
	buf[1] = portVal & 0x00FF;
	if( i2cWrite(deviceAddress, buf, 2) != OK) return FALSE;
	return TRUE;	
}

Bool ioeSet(Uint16 deviceAddress, Uint16 bitMask, Uint16 portVal)
{
	Uint16 temp;
	//read, modify, write
	if( !ioeRead(deviceAddress, &temp) ) return FALSE;
	temp &= ~bitMask;
	temp |= portVal;
	if( !ioeWrite(deviceAddress, temp) ) return FALSE;
	return TRUE;
}

Bool ioeInit(Uint16 deviceAddress, Uint16 portDir, Uint16 portVal)
{
	Uint16 buf[2];

	// Set sequential operation disabled
	//buf[0] = REG_ADDRESS_IOCON;
	//buf[1] = 0x20; //0010 0000
	//if( i2cWrite(deviceAddress, buf, 2, 100) != OK) return FALSE;

	//Set port value bits
	buf[0] = REG_ADDRESS_GPIO;
	buf[1] = portVal & 0x00FF;
	if( i2cWrite(deviceAddress, buf, 2) != OK ) return FALSE;

	// Set port direction bits
	buf[0] = REG_ADDRESS_IODIR;
	buf[1] = portDir & 0x00FF;
	if( i2cWrite(deviceAddress, buf, 2) != OK ) return FALSE;

	return TRUE;	
}






