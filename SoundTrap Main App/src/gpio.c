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
 * Implements C5535 GPIO hardware interface
 * 
 * JMJA
 * 
 */

#include <tistdtypes.h>

#include "irq.h"
#include "gpio.h"
#include "d3defs.h"
#include "protect.h"


#define NO_OF_IO_PORTS (32)

typedef struct  {
    volatile Uint16 IODIR1;
    volatile Uint16 IODIR2;
    volatile Uint16 IOINDATA1;
    volatile Uint16 IOINDATA2;
    volatile Uint16 IOOUTDATA1;
    volatile Uint16 IOOUTDATA2;
    volatile Uint16 IOINTEDG1;
    volatile Uint16 IOINTEDG2;
    volatile Uint16 IOINTEN1;
    volatile Uint16 IOINTEN2;
    volatile Uint16 IOINTFLG1;
    volatile Uint16 IOINTFLG2;
} GpioRegs;

volatile ioport GpioRegs * gpioRegs;

GPIO_Isr GPIO_Isrs[NO_OF_IO_PORTS];

void gpioSetDir( gpioBits bit, gpioDir dir ) 
{
	if(bit < 16) {
		if(dir == GPIO_DIR_OUT)
		{
			gpioRegs->IODIR1 |= (0x0001 << bit);
		}
		else {
			gpioRegs->IODIR1 &= ~(0x0001 << bit);
		}
	}
	else if(bit < 32) {
		bit -=16;
		if(dir == GPIO_DIR_OUT)
		{
			gpioRegs->IODIR2 |= (0x0001 << bit);
		}
		else {
			gpioRegs->IODIR2 &= ~(0x0001 << bit);
		}
	}
}

void gpioSetVal( gpioBits bit, Bool on ) 
{
	if(bit < 16) {
		if(on)
		{
			gpioRegs->IOOUTDATA1 |= (0x0001 << bit);
		}
		else {
			gpioRegs->IOOUTDATA1 &= ~(0x0001 << bit);
		}
	}
	else if (bit < 32) {
		bit -=16;
		if(on)
		{
			gpioRegs->IOOUTDATA2 |= (0x0001 << bit);
		}
		else {
			gpioRegs->IOOUTDATA2 &= ~(0x0001 << bit);
		}
	}
}

void gpioEnableInterrupt( gpioBits bit, Bool on, Bool risingEdge  )
{
	if(bit < 16) {
		if(on) {
			gpioRegs->IOINTEN1 |= (0x0001 << bit);
		}
		else {
			gpioRegs->IOINTEN1 &= ~(0x0001 << bit);
		}
		if(risingEdge) {
			gpioRegs->IOINTEDG1 |= (0x0001 << bit);
		}
		else {
			gpioRegs->IOINTEDG1 &= ~(0x0001 << bit);
		}
	}
	else if (bit < 32) {
		bit -=16;
		if(on)
		{
			gpioRegs->IOINTEN2 |= (0x0001 << bit);
		}
		else {
			gpioRegs->IOINTEN2 &= ~(0x0001 << bit);
		}
		if(risingEdge) {
			gpioRegs->IOINTEDG2 |= (0x0001 << bit);
		}
		else {
			gpioRegs->IOINTEDG2 &= ~(0x0001 << bit);
		}
	}
} 

Bool gpioGetVal( gpioBits bit ) 
{
	if(bit < 16) {
		return ( gpioRegs->IOINDATA1 & (0x0001 << bit) > 0);
	}
	else if (bit < 32) {
		bit -=16;
		return ( gpioRegs->IOINDATA2 & (0x0001 << bit) > 0);
	}
	else return 0;
}

interrupt void gpioIsr(void)
{
    int i;
	Uint32 ifrValue;
	PROTECT;
  	ifrValue = ((Uint32)gpioRegs->IOINTFLG2 << 16) | gpioRegs->IOINTFLG1;
  	
  	gpioRegs->IOINTFLG1 = 0xffff;
  	gpioRegs->IOINTFLG2 = 0xffff;
  	
	for(i=0;i<32;i++) {
		if((ifrValue >> i) & 0x0001) {
			if(GPIO_Isrs[i] != NULL){
				(GPIO_Isrs[i])();
			}
		}
	}
 	END_PROTECT ;
}

void gpioInterruptHandlerRegister(int bit, GPIO_Isr isr)
{
	GPIO_Isrs[bit] = isr;
}

void gpioInterruptHandlerDeRegister(int bit)
{
	GPIO_Isrs[bit] = NULL;
}

void gpioInit(int enableIrq)
{
	gpioRegs = (volatile ioport GpioRegs *)0x1C06;
 	gpioSetDir(GPIO_BIT_GREEN_LED, GPIO_DIR_OUT);
	gpioSetVal(GPIO_BIT_GREEN_LED, FALSE);
 	gpioSetDir(GPIO_BIT_SD_EN, GPIO_DIR_OUT);
	gpioSetVal(GPIO_BIT_SD_EN, TRUE);

#ifndef BOOTLOADER
	irqClear(GPIO_EVENT);
	irqPlug (GPIO_EVENT, &gpioIsr);
	irqEnable(GPIO_EVENT);
#endif
}
