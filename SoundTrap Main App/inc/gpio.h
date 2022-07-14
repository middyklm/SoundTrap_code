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

#ifndef GPIO_H
#define GPIO_H

#include <tistdtypes.h>

typedef  void (*GPIO_Isr)(void);

typedef enum  {
	GPIO_BIT_DSPIO = 10,
	GPIO_BIT_GREEN_LED = 13, 
	GPIO_BIT_MSP_PROG = 15,
	GPIO_BIT_WIF1 = 16, 
	GPIO_BIT_WIF2 = 17,
	GPIO_BIT_RS485 = 11,
	GPIO_BIT_SD_EN = 12,
	GPIO_BIT_UART_TX_EN = 14
} gpioBits;

typedef enum {
	GPIO_DIR_IN, 
	GPIO_DIR_OUT
} gpioDir;

void gpioSetDir( gpioBits bit, gpioDir dir ) ;
void gpioSetVal( gpioBits bit, Bool on ); 
Bool gpioGetVal( gpioBits bit ); 
void gpioEnableInterrupt( gpioBits bit, Bool on, Bool risingEdge);
void gpioInterruptHandlerRegister(int bit, GPIO_Isr isr);
void gpioInterruptHandlerDeRegister(int bit);
void gpioInit();


#endif
