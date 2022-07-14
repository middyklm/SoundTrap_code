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

#ifndef MESSAGE_PROCESSOR_H
#define MESSAGE_PROCESSOR_H

#include <tistdtypes.h>

#define SOP 0x1234
#define DEVICE_TYPE 0
#define deviceID 0

typedef struct {
  Uint16 sop;
  Uint16 id;
  Uint16 type;
  Uint16 dataLength;
  Uint16 dataCRC;
  Uint16 padding[26]; //pad out to usb FIFI size of 64 bytes
  Uint16 headerCRC;
} TmessageHeader;

#define USB_QUEUE_SIZE 1056//1088

typedef struct {
  volatile Uint16 *head;
  volatile Uint16 *tail;
  volatile Uint16 count;
  char volatile lock;
  Uint16 data[USB_QUEUE_SIZE];
} TusbQueue;

extern TusbQueue usbRxQueue; //buffer for usb out packets
extern TusbQueue usbTxQueue; //buffer for usb in packets

extern Uint16 messageBuf[];

typedef void (*DoProcessingCallBack)();
Uint16 waitForQueue(TusbQueue *queue, void *buf, Uint16 count, Bool wait, DoProcessingCallBack doProcessingCallBack);

Uint16 getCommsErrorLog();
void messageProcessorInit();
void txBuffer(void *buf, Uint16 count);
Bool parseHeader( void );
void writeToRxQueue(void *buf, Uint16 count);
int readFromTxQueue(void *buf, Uint16 count);
Uint32 getSpaceAvailableInRxQueue();


#endif

