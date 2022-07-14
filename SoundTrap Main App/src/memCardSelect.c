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

#include "hid.h"
#include "gpio.h"
#include "ioExpander.h"


void memCardSelect(int cardNo)
{
	//ioeInit(0x0027, 0x00C0, 0x00FF);
	ioeWritePU(AUDIO_IO_EXPANDER_DEVICE_ADDRESS_MC, 0x00ff);

	switch(cardNo) {
		case 1:
			gpioSetVal(GPIO_BIT_SD_EN, FALSE);
			ioeSet(AUDIO_IO_EXPANDER_DEVICE_ADDRESS_MC, 0x0038, 0x0008);
			break;
		case 2:
			gpioSetVal(GPIO_BIT_SD_EN, FALSE);
			ioeSet(AUDIO_IO_EXPANDER_DEVICE_ADDRESS_MC, 0x0038, 0x0010);
			break;
		case 3:
			gpioSetVal(GPIO_BIT_SD_EN, FALSE);
			ioeSet(AUDIO_IO_EXPANDER_DEVICE_ADDRESS_MC, 0x0038, 0x0020);

			break;
		default: //main card
			gpioSetVal(GPIO_BIT_SD_EN, TRUE);
			ioeSet(AUDIO_IO_EXPANDER_DEVICE_ADDRESS_MC, 0x0038, 0x0000);
			break;

	}
}




