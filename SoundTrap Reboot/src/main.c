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
#include <stdio.h>
#include <tistdtypes.h>

#include "csl_intc.h"
#include "flash.h"
#include "sysControl.h"
#include "gpio.h"
#include "fs.h"

#define BOOTMAGIC		(0x09AA)
#define GREEN_LED_BIT 	(13)
 
#pragma DATA_ALIGN(sdBuf, 8);
Uint16 sdBuf[256];

int pageAddress;
int bufAddress;

#define FLASH_DELAY 0x00010000 // cycles between led error flashes
#define ERROR_FLASH_COUNT 4 //error flash count excluding code

void flashLed(Uint16 count, Uint32 cycles)
{
	volatile Uint32 j; 
	Uint16 i;
	for(i=0;i<count;i++) {
		gpioSetVal(GPIO_BIT_GREEN_LED, FALSE);
		for(j=0; j<cycles; j++) {};				
		gpioSetVal(GPIO_BIT_GREEN_LED, TRUE);
		for(j=0; j<cycles; j++) {};
	}				
}

//All fatal errors lead here. Indicate error code by flashing led. Then reset the CPU
void fatal(int code)
{
	flashLed(ERROR_FLASH_COUNT + code,FLASH_DELAY * 2); //slow flash
	gpioSetVal(GPIO_BIT_GREEN_LED, FALSE);
	asm("      reset"); //cpu reset 
	while(1) {};
}

//Read 16 bit word fram flash buffer. Refresh buffer when necessary.
unsigned int sdRead16() 
{
	unsigned int val;
	if(bufAddress > (sizeof(sdBuf)-1)) {
		flashRead(++pageAddress, sdBuf, 512);
		bufAddress = 0;
	}  
	val = sdBuf[bufAddress++];
	return val;
}

//Read 32 bit word fram flash buffer. Refresh buffer when necessary.
unsigned long int sdRead32() 
{
	unsigned long val;
	val = sdRead16();
	val = val << 16;
	val += sdRead16();
	return val;
}

int main(void) {
	Uint16 i;
	Uint32 startAddr;
	Uint16 secSize;
	Uint16 pad;
	Uint32 secAddress;

	pageAddress = 0;//FLASH_PAGE_ADDRESS_MAIN_APP_BIN;
	bufAddress = 0;
	
	IRQ_globalDisable();
	sysControlInit();	

	gpioInit();
	gpioSetVal(GPIO_BIT_GREEN_LED, TRUE);

	if( !flashInit() ) fatal(0); //initialise SD flash
	if( !flashRead(pageAddress, sdBuf, 512) ) fatal(1); //Read the first page

	if( sdRead16() != BOOTMAGIC ) fatal(2); //check we have a valid file header

	startAddr = sdRead32(); 
	if(sdRead16() != 0) fatal(3) ; //number of register configurations should = 0

	while(1) //read binary file - see TI doc SPRABL7A
	{
 		pad = 0;
 		secSize = sdRead16();
		if(secSize == 0) 
			break; 		//end of file
 		while((secSize+pad+2) % 4) pad++; //strange file format is padded to 4 byte boundary
		secAddress = sdRead32();
		for(i=0; i<secSize; i++){
			*((int *)secAddress) = sdRead16();
			secAddress++;
		} 
		while(pad-- > 0) { 				//throw away padding
			if( sdRead16() != 0x2020) { //bad words should = 0x2020
				fatal(4);
			}
		}
	}
	flashLed(2, FLASH_DELAY ); 		//2 short happy flashes to indicate success
	((void (*)(void))startAddr)(); 	//jump to start address
}
