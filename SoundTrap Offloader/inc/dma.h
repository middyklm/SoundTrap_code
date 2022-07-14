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

#ifndef DMAINTERRUPTHANDLER_H_
#define DMAINTERRUPTHANDLER_H_

#include <tistdtypes.h>

typedef  void (*DMA_Isr)(void);

#define SDCARD_DMA_READ_CHANNEL CSL_DMA_CHAN0
#define SDCARD_DMA_WRITE_CHANNEL CSL_DMA_CHAN1
#define ADC_DMA_CHANNEL CSL_DMA_CHAN13

void dmaInterruptHandlerInit();
void dmaInterruptHandlerRegister(int dmaChannel, DMA_Isr isr);
void dmaInterruptHandlerDeRegister(int dmaChannel);

#endif /*DMAINTERRUPTHANDLER_H_*/
