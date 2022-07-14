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
#include "csl_usb.h"
#include "csl_usbAux.h"
#include "csl_general.h"

#include "irq.h"
#include "messageProcessor.h"
#include "tick.h"
#include "flash.h"
#include "sd.h"
#include "audioTest.h"
#include "protect.h"
#include "d3std.h"

#define COMMAND_BUFFER_SIZE     (256)
#define FILE_IO_BUFFER_SIZE     (256)
#define CSL_USB_MAX_CURRENT   (50)
#define CSL_USB_WAKEUP_DELAY  (10)

extern CSL_UsbContext     gUsbContext;
extern CSL_UsbRegsOvly    usbRegisters;

pUsbEpHandle          hEpObjArray[CSL_USB_ENDPOINT_COUNT];
CSL_UsbEpObj          usbCtrlOutEpObj;
CSL_UsbEpObj          usbCtrlInEpObj;
CSL_UsbEpObj          usbBulkEp1Obj;
CSL_UsbEpObj          usbBulkEp2Obj;
CSL_UsbEpObj          usbBulkEp3Obj;
CSL_UsbEpObj          usbBulkEp4Obj;

char txpending;
Uint16    commandBuf[COMMAND_BUFFER_SIZE];

#pragma DATA_ALIGN(fileIoBuf, 8);
Uint16 	  fileIoBuf[FILE_IO_BUFFER_SIZE];
Uint16    *deviceDescPtr;
Uint16    *cfgDescPtr;
Uint16    *strDescPtr;
Uint16    bytesRem;
Uint16    devAddr;
Uint16    saveIndex;
Uint16    endpt;
int fsEnable;

Uint32 flashWritePageAddress;

typedef enum {
  EP_CONTROL_OUT 	=  0x00, //Endpoint 0  -  Control Out Endpoint
  EP_CONTROL_IN	=  0x01, //Endpoint 0  -  Control In Endpoint
  EP_COMMAND_IN 	=  0x02, //Endpoint 1  -  Bulk Command In
  EP_COMMAND_OUT	=  0x03, //Endpoint 2  -  Bulk Command Out
  EP_FILE_IN 		=  0x04, //Endpoint 3  -  Bulk File System In
  EP_FILE_OUT		=  0x05  //Endpoint 4  -  Bulk File System Out
} UsbEpArrayNum;


Uint16    deviceDesc[9] = {0x0112, 0x0200, 0x0000, 0x4000, 0x0451,
                           0x5058, 0x0100, 0x0201, 0x0103};

                         
Uint16    cfgDesc[40] = {0x0209, 0x003C, 0x0101, 0x8001, 0x09FA,	// configure descriptor
                         0x0004, 0x0600, 0x0000, 0x0000,		 	// interface descriptor
                         0x0507, 0x0281, 0x0200, 0x0700,			//endpoint 1 IN descriptor 
                         0x0105, 0x0002, 0x0002, 					//endpoint 1 OUT descriptor
                         0x0507, 0x0282, 0x0200, 0x0700,			//endpoint 2 IN descriptor
                         0x0205, 0x0002, 0x0002, 					//endpoint 2 OUT descriptor
                         0x0507, 0x0283, 0x0200, 0x0700, 			//endpoint 3 IN descriptor
                         0x0405, 0x0002, 0x0002						//endpoint 4 OUT descriptor
                         };

Uint16    strDesc[4][20] = {
							// string 0 English-USA
							{0x0304, 0x0409},
							// string 1 "Ocean Instruments"
						    {0x0324, 0x004F, 0x0043, 0x0045, 0x0041, 0x004E, 0x0020,  0x0049, 0x004E,
						     0x0053, 0x0054, 0x0052, 0x0055, 0x004D, 0x0045, 0x004E, 0x0054, 0x0053},
							// string 2 "SUDAR"
						    {0x030C, 0x0053, 0x0055, 0x0044, 0x0041, 0x0052},
							// string 3 "00001"
						    {0x030C, 0x0030, 0x0030, 0x0030, 0x0030, 0x0031}
						    };

extern void VECSTART(void);
interrupt void usb_isr(void);
//void CSL_suspendCallBack(CSL_Status    status);
//void CSL_selfWakeupCallBack(CSL_Status    status);
CSL_Status CSL_startTransferCallback(void    *vpContext, void    *vpeps);
CSL_Status CSL_completeTransferCallback(void    *vpContext, void    *vpeps);
void UsbReadDataOut();

void handleEP0()
{
	pUsbEpHandle          hEPx;
	CSL_Status            status;
	CSL_UsbSetupStruct    usbSetup;
	saveIndex = usbRegisters->INDEX_TESTMODE;
	CSL_FINS(usbRegisters->INDEX_TESTMODE, USB_INDEX_TESTMODE_EPSEL, CSL_USB_EP0);

	USB_getSetupPacket(CSL_USB0, &usbSetup, TRUE);
	if((usbRegisters->PERI_CSR0_INDX & CSL_USB_PERI_CSR0_INDX_RXPKTRDY_MASK) == CSL_USB_PERI_CSR0_INDX_RXPKTRDY_MASK)
	{
		/* Service the RXPKTRDY after reading the FIFO */
		CSL_FINS(usbRegisters->PERI_CSR0_INDX, USB_PERI_CSR0_INDX_SERV_RXPKTRDY, TRUE);

		/* GET DESCRIPTOR Req */
		switch(usbSetup.bRequest)
		{
			/* zero data */
			case CSL_USB_SET_FEATURE:
				switch(usbSetup.wValue)
				{
					case CSL_USB_FEATURE_ENDPOINT_STALL:
						/* updated set and clear endpoint stall
						 * to work with logical endpoint num
						 */
						endpt = (usbSetup.wIndex) & 0xFF;
						hEPx = USB_epNumToHandle(CSL_USB0, endpt);
						if(!(USB_getEndptStall(hEPx, &status))) {
							USB_stallEndpt(hEPx);
						}
						break;

					case CSL_USB_FEATURE_REMOTE_WAKEUP:
						if(!(USB_getRemoteWakeupStat(CSL_USB0))) {
							USB_setRemoteWakeup(CSL_USB0, CSL_USB_TRUE);
						}
						break;

					default:
						break;
				}
				CSL_FINS(usbRegisters->PERI_CSR0_INDX, USB_PERI_CSR0_INDX_SERV_RXPKTRDY, TRUE);
				CSL_FINS(usbRegisters->PERI_CSR0_INDX, USB_PERI_CSR0_INDX_DATAEND, TRUE);
				break;

			case CSL_USB_CLEAR_FEATURE:
				switch(usbSetup.wValue)
				{
					case CSL_USB_FEATURE_ENDPOINT_STALL:
						endpt = (usbSetup.wIndex) & 0xFF;
						hEPx = USB_epNumToHandle(CSL_USB0, endpt);
						if(USB_getEndptStall(hEPx, &status)) {
							USB_clearEndptStall(hEPx);
						}
						break;

					case CSL_USB_FEATURE_REMOTE_WAKEUP:
						if(USB_getRemoteWakeupStat(CSL_USB0)) {
							USB_setRemoteWakeup(CSL_USB0,CSL_USB_FALSE);
						}
						break;

					default:
						 break;
				}
				break;

			case CSL_USB_SET_CONFIGURATION :
			case CSL_USB_SET_INTERFACE:
				endpt = (usbSetup.wIndex) & 0xFF;
				hEPx = USB_epNumToHandle(CSL_USB0, endpt);
				USB_postTransaction(hEPx, 0, NULL, CSL_USB_IOFLAG_NONE);
				/* DataEnd + ServicedRxPktRdy */

				CSL_FINS(usbRegisters->INDEX_TESTMODE, USB_INDEX_TESTMODE_EPSEL, CSL_USB_EP2);
				CSL_FINS(usbRegisters->PERI_CSR0_INDX, USB_PERI_TXCSR_FLUSHFIFO, TRUE);
				CSL_FINS(usbRegisters->PERI_CSR0_INDX, USB_PERI_TXCSR_FLUSHFIFO, TRUE);
				CSL_FINS(usbRegisters->PERI_CSR0_INDX, USB_PERI_TXCSR_CLRDATATOG, TRUE);
				CSL_FINS(usbRegisters->PERI_CSR0_INDX, USB_PERI_TXCSR_CLRDATATOG, TRUE);
				CSL_FINS(usbRegisters->INDEX_TESTMODE, USB_INDEX_TESTMODE_EPSEL, CSL_USB_EP3);
				CSL_FINS(usbRegisters->PERI_CSR0_INDX, USB_PERI_TXCSR_FLUSHFIFO, TRUE);
				CSL_FINS(usbRegisters->PERI_CSR0_INDX, USB_PERI_TXCSR_FLUSHFIFO, TRUE);
				CSL_FINS(usbRegisters->PERI_CSR0_INDX, USB_PERI_TXCSR_CLRDATATOG, TRUE);
				CSL_FINS(usbRegisters->PERI_CSR0_INDX, USB_PERI_TXCSR_CLRDATATOG, TRUE);

				fsEnable = 1;

				CSL_FINS(usbRegisters->INDEX_TESTMODE, USB_INDEX_TESTMODE_EPSEL, CSL_USB_EP0);

				CSL_FINS(usbRegisters->PERI_CSR0_INDX, USB_PERI_CSR0_INDX_SERV_RXPKTRDY, TRUE);
				CSL_FINS(usbRegisters->PERI_CSR0_INDX, USB_PERI_CSR0_INDX_DATAEND, TRUE);
				break;

			case CSL_USB_GET_DESCRIPTOR :

				switch(usbSetup.wValue >> 8)
				{
					case CSL_USB_DEVICE_DESCRIPTOR_TYPE:
						deviceDescPtr = (Uint16 *)deviceDesc;
						status = USB_postTransaction(hEpObjArray[1], deviceDesc[0]&0xFF, deviceDescPtr, CSL_USB_IN_TRANSFER);
						break;

					case CSL_USB_CONFIGURATION_DESCRIPTOR_TYPE:
						if(usbSetup.wLength == 0x0009) {
							cfgDescPtr = cfgDesc;
							status = USB_postTransaction(hEpObjArray[1], 9, cfgDescPtr, CSL_USB_IN_TRANSFER);
						}
						else {
							cfgDescPtr = cfgDesc;
							status = USB_postTransaction(hEpObjArray[1], cfgDesc[1], cfgDescPtr, CSL_USB_IN_TRANSFER);
						}

						break;

					case CSL_USB_STRING_DESCRIPTOR_TYPE:
						if((usbSetup.wValue & 0xFF) == 0x00) {
							strDescPtr = (Uint16 *)strDesc[0];
							status = USB_postTransaction(hEpObjArray[1], strDesc[0][0]&0xFF, strDescPtr, CSL_USB_IN_TRANSFER);
						}
						if((usbSetup.wValue & 0xFF) == 0x01) {
							strDescPtr = (Uint16 *)strDesc[1];
							status = USB_postTransaction(hEpObjArray[1], strDesc[1][0]&0xFF, strDescPtr, CSL_USB_IN_TRANSFER);
						}
						if((usbSetup.wValue & 0xFF) == 0x02) {
							strDescPtr = (Uint16 *)strDesc[2];
							status = USB_postTransaction(hEpObjArray[1], strDesc[2][0]&0xFF, strDescPtr, CSL_USB_IN_TRANSFER);
						}
						if((usbSetup.wValue & 0xFF) == 0x03) {
							strDescPtr = (Uint16 *)strDesc[3];
							status = USB_postTransaction(hEpObjArray[1], strDesc[3][0]&0xFF, strDescPtr, CSL_USB_IN_TRANSFER);
						}
						break;

					default:
						break;
				}

				deviceDescPtr = (Uint16 *)deviceDesc;
				cfgDescPtr    = (Uint16 *)cfgDesc;
				strDescPtr    = (Uint16 *)strDesc[0];
				CSL_FINS(usbRegisters->PERI_CSR0_INDX, USB_PERI_CSR0_INDX_TXPKTRDY, TRUE);
				CSL_FINS(usbRegisters->PERI_CSR0_INDX, USB_PERI_CSR0_INDX_DATAEND, TRUE);
				break;

			case CSL_USB_SET_ADDRESS :
				devAddr = usbSetup.wValue;
				CSL_FINS(usbRegisters->PERI_CSR0_INDX, USB_PERI_CSR0_INDX_SERV_RXPKTRDY, TRUE);
				CSL_FINS(usbRegisters->PERI_CSR0_INDX, USB_PERI_CSR0_INDX_DATAEND, TRUE);
				break;

			default:
				break;
		}
	}
	else
	{
		if(usbSetup.bRequest == 0x05) USB_setDevAddr(CSL_USB0, devAddr);
	}
	usbRegisters->INDEX_TESTMODE  = saveIndex;
}

interrupt void usb_isr(void)
{
	Uint32 startTime; 
	CSL_Status            status;
	pUsbContext           pContext = &gUsbContext;

	/* Read the masked interrupt status register */
	pContext->dwIntSourceL = usbRegisters->INTMASKEDR1;
	pContext->dwIntSourceH = usbRegisters->INTMASKEDR2;

	/* Clear the interrupts */
	if(pContext->dwIntSourceL != FALSE) {
		usbRegisters->INTCLRR1 = pContext->dwIntSourceL;
	}

	if(pContext->dwIntSourceH != FALSE) {
		usbRegisters->INTCLRR2 = pContext->dwIntSourceH;
	}

	if((pContext->dwIntSourceL != FALSE) || (pContext->dwIntSourceH != FALSE)) {
		/* Reset interrupt */
		if(pContext->dwIntSourceH & CSL_USB_GBL_INT_RESET) {
			usbRegisters->INDEX_TESTMODE = usbRegisters->INDEX_TESTMODE & 0x00ff;
			CSL_FINS(usbRegisters->INDEX_TESTMODE, USB_INDEX_TESTMODE_EPSEL, CSL_USB_EP2);
			CSL_FINS(usbRegisters->PERI_CSR0_INDX, USB_PERI_CSR0_INDX_RXPKTRDY, TRUE);
		}

		/* Resume interrupt */
		if(pContext->dwIntSourceH & CSL_USB_GBL_INT_RESUME) {
			USB_setRemoteWakeup(CSL_USB0, CSL_USB_TRUE);
			status = USB_issueRemoteWakeup(CSL_USB0, TRUE);
			/* Give 10 msecs delay before resetting resume bit */
			startTime = getTickCount();
			while (getTickCount() - startTime < 10) {};
			status = USB_issueRemoteWakeup(CSL_USB0, FALSE);
			if(status != CSL_SOK) printf("USB Resume failed\n");
		}

		/* Check end point0 interrupts */
		if(pContext->dwIntSourceL & CSL_USB_TX_RX_INT_EP0) {
			handleEP0();
		}

		if(pContext->dwIntSourceL & CSL_USB_RX_INT_EP2) {
		}

		// Check EP3 Data In Ready 
		if(pContext->dwIntSourceL & CSL_USB_TX_INT_EP3) {

		} 

		// Check EP1 Data In Ready 
		if(pContext->dwIntSourceL & CSL_USB_TX_INT_EP1) {
			int count = usbTxQueue.count;
			if(count > 256) count = 256; 
			readFromTxQueue(commandBuf, count);
			
			if(count > 0) {
				status = USB_postTransaction(hEpObjArray[2], count*2,
			                             commandBuf, CSL_USB_IN_TRANSFER);
			}	else txpending = FALSE;                             

			if(status != CSL_SOK) {
				printf("USB Transaction failed\n");
			}
		}

		/* Connect interrupt */
		if(pContext->dwIntSourceH & CSL_USB_GBL_INT_DEVCONN) {
			status = USB_connectDev(CSL_USB0);
			if(status != CSL_SOK) {
				printf("USB Connect failed\n");
			}
		}

		/* Disconnect interrupt */
		if(pContext->dwIntSourceH & CSL_USB_GBL_INT_DEVDISCONN)
		{
			status = USB_disconnectDev(CSL_USB0);
			if(status != CSL_SOK) {
				printf("USB Disconnect failed\n");
			}
		}

		/* Suspend interrupt */
		if(pContext->dwIntSourceH & CSL_USB_GBL_INT_SUSPEND)
		{
			status = USB_suspendDevice(CSL_USB0);
			if(status != CSL_SOK) {
				printf("USB Suspend failed\n");
			}
		}

		CSL_FINS(usbRegisters->EOIR, USB_EOIR_EOI_VECTOR, CSL_USB_EOIR_RESETVAL);
	}
}

void UsbServiceEpCommandOut() {
	pUsbContext           pContext = &gUsbContext;
	CSL_Status            status;
	Uint16 usb_income_num_bytes;
	Uint16 epNum;
	
	pUsbEpHandle handle = hEpObjArray[EP_COMMAND_OUT];
	epNum = handle->epNum % 8;
	if(USB_isValidDataInFifoOut( &pContext->pEpStatus[ epNum ])) {
		usb_income_num_bytes = USB_getDataCountReadFromFifo( handle );
		if(usb_income_num_bytes) {
			status = USB_postTransaction(handle, usb_income_num_bytes, commandBuf, CSL_USB_OUT_TRANSFER);
			if(status == CSL_SOK){
				writeToRxQueue(commandBuf, usb_income_num_bytes/2);
			}
		}
	}
}


void UsbServiceEpFileOut() {
	Uint16 epNum;
	Uint16 saveIndex = 0;
	CSL_Status            status;
	Uint16 usb_income_num_bytes;
	pUsbEpHandle handle = hEpObjArray[EP_FILE_OUT];
	epNum = handle->epNum % 8;
	saveIndex = usbRegisters->INDEX_TESTMODE;
	usbRegisters->INDEX_TESTMODE &= ~CSL_USB_INDEX_TESTMODE_EPSEL_MASK;
   	usbRegisters->INDEX_TESTMODE |= epNum;

	if(usbRegisters->PERI_RXCSR_INDX & 0x0001) {
		usb_income_num_bytes = usbRegisters->COUNT0_INDX & 0x1FFF;
		if(usb_income_num_bytes) {
			status = USB_postTransaction(handle, usb_income_num_bytes, fileIoBuf, CSL_USB_OUT_TRANSFER);
			if(status == CSL_SOK) {
				if( sdWrite(flashWritePageAddress++ , usb_income_num_bytes, fileIoBuf) != OK) {
					CSL_FINS(usbRegisters->PERI_RXCSR_INDX,   USB_PERI_RXCSR_SENDSTALL, TRUE); //stall endpoint
				}
			}
		}
	}			 
	usbRegisters->INDEX_TESTMODE = saveIndex;
}

Uint16 UsbReadAudioBuffers = 0;


//FileSystem Read. Must go faster!!!
void UsbServiceEpFileIn()
{
	int i;
	Uint16 epNum;
	Uint16 saveIndex = 0;
	Uint16* nextBuf; 

	pUsbEpHandle handle = hEpObjArray[EP_FILE_IN];
	epNum = handle->epNum % 8;
	saveIndex = usbRegisters->INDEX_TESTMODE;
	usbRegisters->INDEX_TESTMODE &= ~CSL_USB_INDEX_TESTMODE_EPSEL_MASK;
   	usbRegisters->INDEX_TESTMODE |= epNum;

	if(fsEnable) {
		if(!(usbRegisters->PERI_CSR0_INDX & 0x0001)) {
			if(UsbReadAudioBuffers) {
				audioTestReadNextBuf(&nextBuf);
			}
			else {
				if( flashReadNextPage(&nextBuf) != OK) { //512 bytes
					CSL_FINS(usbRegisters->PERI_RXCSR_INDX,   USB_PERI_RXCSR_SENDSTALL, TRUE); //stall endpoint
				}
			}
			for(i = 0; i < 256; i++) {
				usbRegisters->FIFO3R1 = nextBuf[i];
			}
			usbRegisters->PERI_CSR0_INDX |= 0x0001;
		}
	}
	usbRegisters->INDEX_TESTMODE = saveIndex;
}

void usbServiceDataOut() 
{
	UsbServiceEpCommandOut();
	UsbServiceEpFileIn();
	UsbServiceEpFileOut();
}

void postUsbData()
{
	int count; 
	if(txpending) return;
	txpending = TRUE;
	count = usbTxQueue.count;
	if(count > 256) count = 256; 
	readFromTxQueue(commandBuf, count);
	USB_postTransaction(hEpObjArray[EP_COMMAND_IN], count*2, commandBuf, CSL_USB_IN_TRANSFER);
}

CSL_Status CSL_startTransferCallback(void    *vpContext, void    *vpeps)
{
	pUsbContext      pContext;
	pUsbEpStatus     peps;
	pUsbTransfer     pTransfer;
	CSL_Status       status;

	status = CSL_SOK;

	pContext  = (pUsbContext)vpContext;
	peps      = (pUsbEpStatus)vpeps;

    if((pContext == NULL) || (peps == NULL) || (!pContext->fMUSBIsReady) || (!peps->fInitialized) ) {
        return(CSL_ESYS_INVPARAMS);
	}

    pTransfer = peps->pTransfer;
    pTransfer->fComplete=FALSE;

	if(pTransfer->dwFlags == CSL_USB_OUT_TRANSFER) {
		if(peps->dwEndpoint == CSL_USB_EP0) {
			status = USB_processEP0Out(pContext);
		}
		else {
			status = USB_handleRx(pContext, peps->dwEndpoint);
		}
	}
	else if(pTransfer->dwFlags == CSL_USB_IN_TRANSFER) {
		if(peps->dwEndpoint == CSL_USB_EP0 ) {
			status = USB_processEP0In(pContext);
		}
		else {
			status = USB_handleTx(pContext, peps->dwEndpoint);
		}
	}
	else {
		status = CSL_ESYS_INVPARAMS;
	}
	return(status);
}


CSL_Status CSL_completeTransferCallback(void    *vpContext, void    *vpeps)
{
	return(CSL_SOK);
}

void CSL_suspendCallBack(CSL_Status    status)
{
	printf("\nUSB SUSPEND Callback\n");
}

void CSL_selfWakeupCallBack(CSL_Status    status)
{
	printf("\nUSB Self Wakeup CallBack\n");
}

void resetUsb()
{
	Uint16 epNum;
	pUsbEpHandle handle = hEpObjArray[EP_FILE_IN];
	epNum = handle->epNum % 8;
	usbRegisters->INDEX_TESTMODE &= ~CSL_USB_INDEX_TESTMODE_EPSEL_MASK;
	usbRegisters->INDEX_TESTMODE |= epNum;
	usbRegisters->PERI_CSR0_INDX &= ~0x0004;
	usbRegisters->PERI_CSR0_INDX |= 0x01C0;
}


CSL_Status usbInit(void)
{
	CSL_Status            status;
	CSL_UsbConfig         usbConfig;
	Uint16            eventMask;

	txpending = FALSE;
	fsEnable = 0;
	
	usbConfig.opMode             = CSL_USB_OPMODE_POLLED;
    usbConfig.devNum             = CSL_USB0;
	usbConfig.maxCurrent         = CSL_USB_MAX_CURRENT;
	usbConfig.appSuspendCallBack = (CSL_USB_APP_CALLBACK)CSL_suspendCallBack;
	usbConfig.appWakeupCallBack  = (CSL_USB_APP_CALLBACK)CSL_selfWakeupCallBack;
	usbConfig.startTransferCallback  = CSL_startTransferCallback;
	usbConfig.completeTransferCallback = CSL_completeTransferCallback;

	hEpObjArray[0] = &usbCtrlOutEpObj;
	hEpObjArray[1] = &usbCtrlInEpObj;
	hEpObjArray[2] = &usbBulkEp1Obj;
	hEpObjArray[3] = &usbBulkEp2Obj;
	hEpObjArray[4] = &usbBulkEp3Obj;
	hEpObjArray[5] = &usbBulkEp4Obj;


	// Plug the USB Isr into vector table 
	irqPlug(USB_EVENT, &usb_isr);

	// Enable USB Interrupts 
	irqEnable(USB_EVENT);
	// Enable CPU Interrupts 
	//irqGlobalEnable();

	// Initialize the USB module 
	status = USB_init(&usbConfig);
	if(status != CSL_SOK) {
		return(CSL_ESYS_FAIL);
	}

	// Reset the USB device 
	if(CSL_SOK != USB_resetDev(CSL_USB0)) {
		return(CSL_ESYS_FAIL);
	}

	// Initialize the Control Endpoint OUT 0 
	eventMask = (CSL_USB_EVENT_RESET | CSL_USB_EVENT_SETUP |
				 CSL_USB_EVENT_SUSPEND | CSL_USB_EVENT_RESUME |
				 CSL_USB_EVENT_RESET | CSL_USB_EVENT_EOT);

	if(CSL_SOK != USB_initEndptObj(CSL_USB0, hEpObjArray[EP_CONTROL_OUT], CSL_USB_OUT_EP0, CSL_USB_CTRL, CSL_USB_EP0_PACKET_SIZE, eventMask, NULL))
	{
		return(CSL_ESYS_FAIL);
	}

	// Initialize the Control Endpoint IN 0 
	if (CSL_SOK != USB_initEndptObj(CSL_USB0, hEpObjArray[EP_CONTROL_IN], CSL_USB_IN_EP0, CSL_USB_CTRL, CSL_USB_EP0_PACKET_SIZE, CSL_USB_EVENT_EOT, NULL)) 
	{
		return(CSL_ESYS_FAIL);
	}

	// Initialize the Bulk Endpoint IN 1 
	eventMask = (CSL_USB_EVENT_RESET | CSL_USB_EVENT_EOT);
	if (CSL_SOK != USB_initEndptObj(CSL_USB0, hEpObjArray[EP_COMMAND_IN], CSL_USB_IN_EP1, CSL_USB_BULK, CSL_USB_EP_PACKET_SIZE_HS, eventMask, NULL)) {
		return(CSL_ESYS_FAIL);
	}

	// Initialize the Bulk Endpoint OUT 2 
	if (CSL_SOK != USB_initEndptObj(CSL_USB0, hEpObjArray[EP_COMMAND_OUT], CSL_USB_OUT_EP2, CSL_USB_BULK, CSL_USB_EP_PACKET_SIZE_HS, CSL_USB_EVENT_EOT, NULL))
	{
		return(CSL_ESYS_FAIL);
	}

	// Initialize the Bulk Endpoint IN 3 
	if (CSL_SOK != USB_initEndptObj(CSL_USB0, hEpObjArray[EP_FILE_IN], CSL_USB_IN_EP3, CSL_USB_BULK, CSL_USB_EP_PACKET_SIZE_HS, eventMask, NULL)) {
		return(CSL_ESYS_FAIL);
	}

	// Initialize the Bulk Endpoint OUT 4 
	if (CSL_SOK != USB_initEndptObj(CSL_USB0, hEpObjArray[EP_FILE_OUT], CSL_USB_OUT_EP4, CSL_USB_BULK, CSL_USB_EP_PACKET_SIZE_HS, CSL_USB_EVENT_EOT, NULL)) {
		return(CSL_ESYS_FAIL);
	}

	// Set the parameters 
	if(CSL_SOK != USB_setParams(CSL_USB0, hEpObjArray, FALSE)) {
		return(CSL_ESYS_FAIL);
	}

	// Connect the USB device 
	if(CSL_SOK != USB_connectDev(CSL_USB0)) {
		return(CSL_ESYS_FAIL);
	}

	deviceDescPtr = (Uint16 *)deviceDesc;
	cfgDescPtr    = (Uint16 *)cfgDesc;
	strDescPtr    = (Uint16 *)strDesc;

	USB_connectDev(CSL_USB0);

	return(CSL_SOK);
}

