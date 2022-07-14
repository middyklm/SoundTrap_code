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
#include "soc.h"
#include "csl_pll.h"
#include "csl_intc.h"
#include "csl_general.h"
#include "csl_pll.h"

#include "d3std.h"
#include "flash.h"
#include "usb.h"
#include "messageProcessor.h"
#include "tick.h"
#include "i2c.h"
#include "sysControl.h"
#include "gpio.h"
#include "csl_dma.h"
#include "crc.h"
#include "mspif.h"
#include "config.h"
#include "spi.h"
#include "psense.h"
#include "uart.h"
#include "timer.h"
#include "audioTest.h"
#include "dma.h"
#include "commands.h"
#include "sar.h"
#include "hid.h"
#include "ioExpander.h"

PLL_Obj pllObj;
const Uint32  sysclk = 73728000;

volatile int protectm = 1 ;
extern void VECSTART(void);
MSP_Sensors sensorData;
Uint16 flashState = 10;

Uint32 getRtcTime()
{
	MSP_Time t;
	if(msp_gettime(&t,NULL,NULL)==MSP_FAIL)
		return(0ul);
	return(t.rtime);
}

void pllInit(Uint32 clk)
{
    PLL_Config configInfo;
    Uint16 d;
    Uint16 m;

	//asm("	BCLR CLKOFF,ST3_55"); //enable clk out

    switch(clk) {
		case 25000000:
			m = 1526;
			d = 2;
			break;
		case 50000000:
			m = 1526;
			d = 1;
			break;
		case 73728000:
			m = 2249; //due to hardware bug 2249 gives multiple of 2250 - see TI forum  'C5535 PLL multiplier anomaly' 20 Aug '14
			d = 1;
			break;
		case 100000000:
			m = 3052;
			d = 1;
			break;
	}

	PLL_init(&pllObj,0);
    
    configInfo.PLLCNTL1 = (m-4) & 0x0FFF; //CGCR1
    configInfo.PLLINCNTL = 0x8000;	//CGCR2
    configInfo.PLLOUTCNTL = d > 1 ? 0x0200 | (d-1) : 0x0000; //CRCR4
    configInfo.PLLCNTL2 = 0x0806; //CGCR3
    PLL_bypass(&pllObj);
    PLL_reset(&pllObj);
    PLL_config(&pllObj, &configInfo);
 	PLL_enable(&pllObj);
}

void ledOff()
{
	gpioSetVal( GPIO_BIT_GREEN_LED, 0);
}

void blink()
{
	flashState--;
	if(flashState > 10) {
		gpioSetVal( GPIO_BIT_GREEN_LED, 1);
		tickRunOnce(&ledOff, 50);
	}
	else if(!flashState) {
		flashState = 10;
		gpioSetVal( GPIO_BIT_GREEN_LED, 1);
		tickRunOnce(&ledOff, 50);
	}

}

void sensorScan()
{
	msp_getSensorData(&sensorData);
}

Uint16 mainPressure;
void readPressure()
{
	psenseRead(&mainPressure);
}

#define ADC_CHANS 4
#define ADC_DEVS 2
Uint16 chan = 0;
Int32 adcData[ADC_CHANS*ADC_DEVS];
int adcLogEnable = 0;

void readAdc()
{
	int i;
	Int32 v = 0;
	Uint16 inbuf[4];
	Uint16 outbuf[1];
	Uint16 nextChan = chan + 1;

	if(adcLogEnable) {
		if(nextChan >= ADC_CHANS) nextChan = 0;
		outbuf[0] = ((nextChan << 5) & 0x60)  | 0x008F;//  1000 1111 18 bit

		for(i=0; i<ADC_DEVS; i++) {
			i2cWriteRead(0x0068 + i, outbuf, 1, inbuf, 4);
			v = inbuf[0];
			v <<= 8;
			v += inbuf[1];
			v <<= 8;
			v += inbuf[2];
			v <<= 8;
			adcData[chan + (i*ADC_CHANS)] = v;
		}
		chan = nextChan;
	}
}




void main(void)
{
	pllInit(sysclk);

	IRQ_globalDisable();
	IRQ_clearAll();
	IRQ_disableAll();
	IRQ_setVecs((Uint32)(&VECSTART));

	sysControlInit();

	spiInit(sysclk);
	psenseInit();

	gpioInit();
	gpioSetVal(GPIO_BIT_GREEN_LED, TRUE);
	timerInit();
	tickInit(sysclk);


	uartInit(sysclk, 115200);
	uartEnableTx(FALSE);
	uartSelectTransceiver(FALSE);

	i2cInit(sysclk);

	sarInit();
	hidInit();

	ioeInit(AUDIO_IO_EXPANDER_DEVICE_ADDRESS, 0xFFC0, 0x0000);

	if ((hid == ST4300) || (hid == ST500)) {
		ioeInit(AUDIO_IO_EXPANDER_DEVICE_ADDRESS_MC, 0x00C0, 0x0000);
	}

	if(hid == ST500) {
		ioeWritePU(AUDIO_IO_EXPANDER_DEVICE_ADDRESS, 0x38);
	}

	tickSetUnixTime(getRtcTime());

	dmaInterruptHandlerInit();
	audioTestInit();

	flashInit();
	
	messageProcessorInit();
	usbInit();


	tickRunEvery(&blink, 20);

	msp_getFirmwareVersion();
	tickRunEvery(&sensorScan, 30000);
	tickRunEvery(&readAdc, 500);

	IRQ_globalEnable();

	configInit();

	sensorScan();

	while(1) {
		parseHeader();
		usbServiceDataOut();
	    tickProcessCallbacks();
	}
}
