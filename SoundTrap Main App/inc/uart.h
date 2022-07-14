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

#ifndef UART_H_
#define UART_H_

typedef  void (*UartRxCallbackHandler)(int data);
typedef  void (*UART_Isr)(void);

int uartEnableTx(Bool enable);
int uartSelectTransceiver(Bool enable485);
int uartInit(Uint32 sysClk, Uint32 baud);
int uartTx(Uint16 *data, Uint16 count);
void uartTx_cha(Uint8 data);    // Middy's code
int uartTx_Str(Uint16 *data);   // Middy's code
int uartRx(Uint16 *data, Uint16 count);
int uartEnableRxInterrupt(int enable);
void uartInterruptHandlerRegister(UART_Isr isr);
Uint16 uartRxBuffer(void *buf, Uint16 count);
int uartClearRx();
void uartRegRxCallbackHandler( UartRxCallbackHandler cb );
void pllInit(Uint32 clk);

#endif /*UART_H_*/
