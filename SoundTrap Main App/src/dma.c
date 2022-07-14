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
 * Implements C5535 DMA hardware interface.
 *
 * Dependant on TI C55 CSL v2.5
 * 
 * JMJA
 * 
 */

#include "d3defs.h"
#include "d3std.h"
#include "protect.h"
#include "board.h"
#include "devdep.h"
#include "misc.h"
#include "modnumbers.h"
#include "error.h"
#include "csl_intc.h"
#include "csl_dma.h"
#include "csl_general.h"
#include "dma.h"

#define NO_OF_DMA_CHANNELS (16)

DMA_Isr DMA_Isrs[NO_OF_DMA_CHANNELS];

int blocksProcessed = 0;

interrupt void dma_isr(void)
{
    int i, ifrValue;
	PROTECT;

    blocksProcessed++;
    
    //IRQ_clear(DMA_EVENT);
  	ifrValue = CSL_SYSCTRL_REGS->DMAIFR;
	CSL_SYSCTRL_REGS->DMAIFR = ifrValue;
	for(i=0;i<16;i++) {
		if((ifrValue >> i) & 0x0001) {
			if(DMA_Isrs[i] != NULL){
				//asm("	BSET XF");
				(DMA_Isrs[i])(i);
			}
		}
	}
 	END_PROTECT ;
}

void dmaInterruptHandlerRegister(int dmaChannel, DMA_Isr isr)
{
	DMA_Isrs[dmaChannel] = isr;
}
void dmaInterruptHandlerDeRegister(int dmaChannel)
{
	DMA_Isrs[dmaChannel] = NULL;
}

void dmaInterruptHandlerInit()
{
	int i;
	for(i=0; i<NO_OF_DMA_CHANNELS; i++) DMA_Isrs[i] = NULL;
	DMA_init();
	IRQ_plug (DMA_EVENT, &dma_isr);
	IRQ_enable(DMA_EVENT);

}



