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

#include "d3std.h"
#include "messageProcessor.h"
#include "crc.h"
#include "tick.h"
#include "commands.h"
#include "usb.h"

#define RX_TIME_OUT 2
#define QUEUE_EMPTY( queue ) ( queue.count == 0 )
#define QUEUE_FULL( queue ) ( queue->count == USB_QUEUE_SIZE )
#define MAX_MESSAGE_DATA_SIZE 2048

#pragma DATA_ALIGN(messageBuf, 8); //align 
Uint16 messageBuf[MAX_MESSAGE_DATA_SIZE/2];


TusbQueue usbRxQueue;
TusbQueue usbTxQueue;

Uint16 commsErrorLog;

Uint16 getCommsErrorLog()
{
	return commsErrorLog;
}

void queueInit(TusbQueue *queue)
{
  queue->head = queue->tail = &queue->data[0];
  queue->count = 0;
}


/*-------------------------------------------------------
  Read a byte from the queue
--------------------------------------------------------*/
static Uint16 queueRead(TusbQueue *queue, Uint16 *data, Uint16 count)
{
	
	Uint16 * wp = data;
  	Uint16 result = 0;
  	if(!queue->lock) {
	  	while( (result < count) && (queue->count > 0))
	  	{
	    	*wp++ = *queue->head;
	    	result ++;
	    	if (queue->head != &queue->data[USB_QUEUE_SIZE-1])
	    	{
	      		queue->head++;
	    	}
		    else
		    {
		      queue->head = &queue->data[0];
		    }
		    queue->count--;
	  	}
  	}
  	return result;
}

/*-------------------------------------------------------
  Write a byte to the queue
--------------------------------------------------------*/
/*
static void queueWrite(TusbQueue *queue, Uint16 *data, Uint16 count)
{
	Uint16 * rp = data;
  	Uint16 result = 0;
  		
	if( count > (USB_QUEUE_SIZE - queue->count)) {
	 	//insuficient space
	 	//log buf full error
	 	return;
	}  

	queue->lock = 1;


	  	while( (result < count) && (queue->count < USB_QUEUE_SIZE))
	  	{
	    	*queue->tail = *rp++;
	    	result ++;
	    	if (queue->tail != &queue->data[USB_QUEUE_SIZE-1])
	    	{
	      		queue->tail++;
	    	}
		    else
		    {
		      queue->tail = &queue->data[0];
		    }
		    queue->count++;
	  	}



	queue->lock = 0;
}
*/
static void queueWrite(TusbQueue *queue, Uint16 *data, Uint16 count)
{
	while( count > (USB_QUEUE_SIZE - queue->count)) {
	 	//insuficient space
		parseHeader();
	}  

	queue->lock = 1;

	if((queue->tail + count) <= queue->data + USB_QUEUE_SIZE )
	{
		memcpy((void *)queue->tail, data, count);
		queue->tail += count;
		queue->count+= count;
	}
	else {
		Uint16 countTillEnd = (queue->data + USB_QUEUE_SIZE) - queue->tail; 
		memcpy((void *)queue->tail, data, countTillEnd);
		memcpy((void *)queue->data, data + countTillEnd , count - countTillEnd);
		queue->tail = queue->data + (count - countTillEnd);
		queue->count+= count;
	}

	queue->lock = 0;
}

void txBuffer(void *buf, Uint16 count)
{
	queueWrite(&usbTxQueue, buf, count);
	postUsbData();
}

int readFromTxQueue(void *buf, Uint16 count)
{
	return queueRead(&usbTxQueue, buf, count);
}

//Data from USB enters the queue here (under interrupt)
void writeToRxQueue(void *buf, Uint16 count)
{
	queueWrite(&usbRxQueue, buf, count);
}

Uint32 getSpaceAvailableInRxQueue()
{
	return USB_QUEUE_SIZE - usbRxQueue.count;
}

Uint16 waitForQueue(TusbQueue *queue, void *buf, Uint16 count, Bool wait, DoProcessingCallBack doProcessingCallBack)
{
	Uint32 startTime = getTickCount();
	if(!wait && !queue->count) return 0; // return immediately if no data
	while(queue->count < count) {
		if(doProcessingCallBack != NULL) doProcessingCallBack();
		//UsbServiceDataOut();
		if(getTickCount() - startTime > RX_TIME_OUT) break; 
	}
	return queueRead(queue, buf, count);
}


void parseMessage ( TmessageHeader *header )
{
	int count;
	if ( header->dataLength > 0)
	{
   		if ( header->dataLength > MAX_MESSAGE_DATA_SIZE )
   		{
   			commsErrorLog++;
   			return;
   		}

		//count = rxBuffer( messageBuf, header->dataLength/2, TRUE);
  		count = waitForQueue(&usbRxQueue, messageBuf, header->dataLength/2, 0, &usbServiceDataOut);
    	if(  count != header->dataLength/2)
    	{
      		commsErrorLog++;
      		return;
    	}

    	if( crc( messageBuf, header->dataLength/2 ) != header->dataCRC )
    	{
      		commsErrorLog++;
      		return;
    	}
  	}
  //GotFirstCommand = TRUE;
  executeCommand( header, messageBuf );
}


Bool parseHeader( void ) {

  TmessageHeader header;
  Uint16 count;

  Bool result = 0;	

  result = 1;
  //count = rxBuffer( &header, sizeof( header ), FALSE );
  count = waitForQueue(&usbRxQueue, &header, sizeof( header ), 0, &usbServiceDataOut);
  if( count == sizeof( header ))
  {
    if( (header.sop == SOP) && (header.id == 0 || header.id == deviceID) )
    {
      if( crc(&header, sizeof( header ) - 1 ) == header.headerCRC)
      {
        parseMessage( &header );
      }
      else commsErrorLog++;
    }
    else commsErrorLog++;
  }
  else if( count != sizeof( header )) commsErrorLog=count++;

  return result;
}


void messageProcessorInit()
{
	queueInit(&usbRxQueue);
	queueInit(&usbTxQueue);
}


