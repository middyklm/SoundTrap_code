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
#include "d3defs.h"
#include "protect.h"
#include "gpio.h"
#include "irq.h"
#include "uart.h"

PLL_Obj pllObj_uart;


typedef struct  {
    volatile Uint16 RBR;
    volatile Uint16 RSVD0;
    volatile Uint16 IER;
    volatile Uint16 RSVD1;
    volatile Uint16 IIR;
    volatile Uint16 RSVD2;
    volatile Uint16 LCR;
    volatile Uint16 RSVD3;
    volatile Uint16 MCR;
    volatile Uint16 RSVD4;
    volatile Uint16 LSR;
    volatile Uint16 RSVD5;
    volatile Uint16 MSR;
    volatile Uint16 RSVD6;
    volatile Uint16 SCR;
    volatile Uint16 RSVD7;
    volatile Uint16 DLL;
    volatile Uint16 RSVD8;
    volatile Uint16 DLH;
    volatile Uint16 RSVD9;
    volatile Uint16 PID1;
    volatile Uint16 PID2;
    volatile Uint16 RSVD10[2];
    volatile Uint16 PWREMU_MGMT;
    volatile Uint16 RSVD11;
    volatile Uint16 MDR;
} UartRegs;

volatile ioport UartRegs * uartRegs;

UartRxCallbackHandler urxcb = NULL;



int uartTx(Uint16 *data, Uint16 count)
{
	Uint16 i = 0;
	while(i < count) {
		while(!(uartRegs->LSR & 0x40)) {};
		uartRegs->RBR = data[i++];
	}
	return 0;
}


void uartTx_cha(Uint8 data){
    while(!(uartRegs->LSR & 0x40)) {};
    uartRegs->RBR = data;
}

int uartTx_Str(Uint16 *Str_data)
{
    while(*Str_data != 0x00) {
        uartTx_cha(*Str_data);
        Str_data++;
    }
    return 0;
}


int uartRx(Uint16 *data, Uint16 count)
{
	Uint16 i = 0;
	while(i < count) {
		while(!(uartRegs->LSR & 0x01)) {};
		data[i++] = uartRegs->RBR;
	}
	return 0;
}

int uartEnableTx(Bool enable)
{
	gpioSetVal(GPIO_BIT_UART_TX_EN, enable);
	return 0;
}

int uartSelectTransceiver(Bool enable485)
{
	gpioSetVal(GPIO_BIT_RS485, enable485);
	return 0;
}

int uartClearRx()
{
	int temp =  uartRegs->RBR;
	return 0;
}

int uartEnableRxInterrupt(int enable)
{
	if(enable) {
		uartRegs->IER |= 0x01;
	}
	else {
		uartRegs->IER &= ~0x01;
	}
	return 0;
}

interrupt void uartIsr(void)
{
	Uint16 data;
	//Uint16 iirValue, source;
	PROTECT;
	//iirValue = uartRegs->IIR;
	//source = (iirValue & 0x0E ) > 1;
	data = uartRegs->RBR;
	if(urxcb != NULL) urxcb(data);
	END_PROTECT ;
}

void uartRegRxCallbackHandler( UartRxCallbackHandler cb )
{
	urxcb = cb;
}

int uartInit(Uint32 sysClk, Uint32 baud)
{
	Uint16 divider;
	
	uartRegs = (volatile ioport UartRegs *)0x1B00;
	
	gpioSetDir(GPIO_BIT_RS485, GPIO_DIR_OUT);
	gpioSetDir(GPIO_BIT_UART_TX_EN, GPIO_DIR_OUT);
	
	gpioSetVal(GPIO_BIT_RS485, FALSE);
	gpioSetVal(GPIO_BIT_UART_TX_EN, FALSE);
	
	divider = sysClk / 16 / baud;
	
	uartRegs->PWREMU_MGMT = 0x6000; //enable
	uartRegs->DLH = divider >> 8; 
	uartRegs->DLL = divider & 0x00ff; 
	uartRegs->IER = 0x0001; //enable rx interrupt
	uartRegs->IIR = 0x0000; //disable fifo mode (IIR & FCR reg share same address)
	uartRegs->LCR = 0x0003; //8-bit

	irqClear(UART_EVENT);
	irqPlug (UART_EVENT, &uartIsr);
	irqEnable(UART_EVENT);

	return 0;
}


void pllInit(Uint32 clk)
{
    PLL_Config configInfo;
    Uint16 d;
    Uint16 m;

    //asm(" BCLR CLKOFF,ST3_55"); //enable clk out

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
        case 18432000:
            m = 2249;
            d = 4;
            break;
        case 100000000:
            m = 3052;
            d = 1;
            break;
    }

    PLL_init(&pllObj_uart,0);

    configInfo.PLLCNTL1 = (m-4) & 0x0FFF; //CGCR1
    configInfo.PLLINCNTL = 0x8000;  //CGCR2
    configInfo.PLLOUTCNTL = d > 1 ? 0x0200 | (d-1) : 0x0000; //CRCR4
    configInfo.PLLCNTL2 = 0x0806; //CGCR3
    PLL_bypass(&pllObj_uart);
    PLL_reset(&pllObj_uart);
    PLL_config(&pllObj_uart, &configInfo);
    PLL_enable(&pllObj_uart);
}

