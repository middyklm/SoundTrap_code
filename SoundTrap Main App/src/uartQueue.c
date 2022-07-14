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
#include "d3std.h"
#include "d3defs.h"
#include "protect.h"
#include "gpio.h"
#include "irq.h"
#include "uart.h"
#include "dma.h"
#include "data.h"
#include "dmem.h"
#include "uart.h"
#include "job.h"
#include "cfg.h"

// write queue is a linked list of FLSH_Job objects
typedef struct uartQueueJob {
	DATA_Obj	*data ;			   // data object currently being read / written
	struct uartQueueJob *next ;	// next link in the list
	struct uartQueueJob *previous ;	// next link in the list
} uartQueueJob;

static uartQueueJob	*uartQueTail = NULL;
static uartQueueJob	*uartQueHead = NULL;
static uartQueueJob	*uartQueCurrent = NULL;

int busy = 0;
int curBufPos = 0;

#define UQ_BUF_SIZE 32

Uint32 uartQueueBuff[(UQ_BUF_SIZE * 3) +2];

const char HEX[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

void uartQueueBuffer(int mode, Uint16 *data, Uint16 count)
{
	Uint16 j=0;
	Uint16  i;
	Uint16 c = 0;
	if(mode == 1) uartQueueBuff[j++] = 'S';
	for(i=0; i<count; i++) {
		uartQueueBuff[j++] = HEX[(data[i] >> 12) & 0x000f];
		uartQueueBuff[j++] = HEX[(data[i] >> 8) & 0x000f];
		uartQueueBuff[j++] = HEX[(data[i] >> 4) & 0x000f];
		uartQueueBuff[j++] = HEX[data[i] & 0x000f];
		uartQueueBuff[j++] = ' ';
	}
	if(mode == 2) uartQueueBuff[j++] = '\n';
	uartTxDma(uartQueueBuff, j);
}

void sendStart()
{
	uartQueueBuff[0] = 'S';
	uartTxDma(uartQueueBuff, 1);
}

void uartQueueRun()
{
	int mode = 0;
	int wordsToSend;
	int wordsRemaining;

	if(uartQueCurrent == NULL) {
		GO_ATOMIC ;
		uartQueCurrent = uartQueHead;
		curBufPos = 0;
		if( uartQueCurrent!=NULL ) {
			uartQueHead = uartQueCurrent->next ;
			if(uartQueHead == NULL)	// if there is no next job
				uartQueTail = NULL ;	// null the queue
		}
		END_ATOMIC ;
	}

	if(uartQueCurrent != NULL)
	{

		wordsRemaining = uartQueCurrent->data->size - curBufPos;
		wordsToSend = MIN(wordsRemaining, UQ_BUF_SIZE/2);

		if(curBufPos== 0) mode = 1;
		if(wordsToSend == wordsRemaining) mode = 2;

		uartQueueBuffer(mode,  (Uint16 *)(uartQueCurrent->data->p) + curBufPos,  wordsToSend );


		curBufPos += wordsToSend;
		wordsRemaining -= wordsToSend;

		if(wordsRemaining == 0)
		{
			DATA_free(uartQueCurrent->data);
			DMEM_free(uartQueCurrent);
			uartQueCurrent = NULL;
		}
		busy = 1;
	}
	busy = 0;
}

void uartQueueOnTxCompleteHandler(int status)
{
	uartQueueRun();
}

int uartQueueProc(Ptr pt, DATA_Obj *d)
{
	uartQueueJob	*j ;

	if( d->nsamps==0 ) {
		return(OK) ;
	}

	// create a uart job
	if((j=(uartQueueJob *)DMEM_alloc(sizeof(uartQueueJob)))==(uartQueueJob *)NULL) {
		//run out of dmem. Will have to drop this packet
		DATA_free(d);
		return(OK);
	}
	j->data = d;
	j->next = NULL;
	GO_ATOMIC;
	if(uartQueHead == NULL) {
		uartQueHead = uartQueTail = j;
	}
	else {
		uartQueTail->next = j;
		uartQueTail = j;
	}
	END_ATOMIC;

	if(!busy) {
		uartQueueRun(); //kick off the tx process
	}

	return(OK);
}


int	uartQueueOpen()
{
	int id;
	uartSetTxCompleteHandler(uartQueueOnTxCompleteHandler);
	id = CFG_register((JOB_Fxn) uartQueueProc, NULL, NULL);
	return id;
}



