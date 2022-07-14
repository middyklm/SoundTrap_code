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

#include		<string.h>
#include 		<stdio.h>
#include   		<tistdtypes.h>

#include 		"csl_general.h"
#include		"csl_mmcsd.h"

CSL_MmcsdHandle    	  mmcsdHandle;
CSL_MMCControllerObj 	pMmcsdContObj;
CSL_MMCCardObj			mmcCardObj;
CSL_MMCCardIdObj 		cardIdObj;
CSL_MMCCardCsdObj 		cardCsdObj;

extern CSL_Status MMC_sendRca(CSL_MmcsdHandle hMmcsd, CSL_MMCCardObj *pSdCardObj, Uint16 *pRCardAddr);

int flashRead(Uint32 pageAddr, Uint16 *pReadBuffer, Uint16 noOfBytes)
{
	return ( MMC_read(mmcsdHandle, pageAddr, noOfBytes, pReadBuffer) == CSL_SOK );	
}

int flashInit(void)
{
	CSL_Status	    mmcStatus;
	Uint16          rca;

	// Initialize MMCSD module
	if( MMC_init() != CSL_SOK) return FALSE;
	// Open the MMCSD module
    mmcsdHandle = MMC_open(&pMmcsdContObj, CSL_MMCSD0_INST, CSL_MMCSD_OPMODE_POLLED, &mmcStatus);
	if( mmcsdHandle == NULL) return FALSE;
	// Send CMD0 to the card
	if( MMC_sendGoIdle(mmcsdHandle) != CSL_SOK ) return FALSE;
	// Check for the card
    mmcStatus = MMC_selectCard(mmcsdHandle, &mmcCardObj);
	if((mmcStatus == CSL_ESYS_BADHANDLE) || (mmcStatus == CSL_ESYS_INVPARAMS)) return FALSE;
	// Verify we have a HCSD card
	if((mmcCardObj.cardType != CSL_SD_CARD) || (mmcsdHandle->cardObj->sdHcDetected != TRUE)) 	return FALSE;
	// Set the init clock
	if( MMC_sendOpCond(mmcsdHandle, 70) != CSL_SOK ) return FALSE;
	// Send the SD card identification Data
	if( MMC_sendAllCID(mmcsdHandle, &cardIdObj) != CSL_SOK ) return FALSE;
	// Set the SD Relative Card Address
	if( MMC_sendRca(mmcsdHandle, &mmcCardObj, &rca) != CSL_SOK ) return FALSE;
	// Read the SD Card Specific Data
	if( SD_getCardCsd(mmcsdHandle, &cardCsdObj) != CSL_SOK ) return FALSE;
	// Set the card pointer in internal data structures
	if( MMC_setCardPtr(mmcsdHandle, &mmcCardObj) != CSL_SOK) return FALSE;
	// Set the clock for read-write access
	if( MMC_sendOpCond(mmcsdHandle, 1) != CSL_SOK) return(CSL_ESYS_FAIL);
	if( MMC_setEndianMode(mmcsdHandle, CSL_MMCSD_ENDIAN_BIG, CSL_MMCSD_ENDIAN_BIG) != CSL_SOK) return FALSE;

	return TRUE;
}
