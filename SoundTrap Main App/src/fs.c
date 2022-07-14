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
 * Implements simple consecutive block file system. Consumes FLSH_Buff objects and 
 * writes pages to underlying block based file system. 
 *
 * JMJA
 *  
*/
 
#include <string.h>
#include <stdio.h>
#include <tistdtypes.h>

#include "soc.h"
#include "d3defs.h"
#include "d3defs.h"
#include "d3std.h"
#include "protect.h"
#include "board.h"
#include "devdep.h"
#include "misc.h"
#include "modnumbers.h"
#include "error.h"
#include "crc.h"
#include "csl_mmcsd.h"
#include "csl_intc.h"
#include "csl_general.h"
#include "cslr_mmcsd.h"
#include "fs.h"
#include "sd.h"
#include "job.h"
#include "crc.h"
#include "gpio.h"
#include "timr.h"
#include "data.h"
#include "flsh.h"
#include "info.h"
#include "minmax.h"
#include "sysControl.h"
#include "hid.h"
#include "memCardSelect.h"


#define FLASH_WRITE_RETRIES (2)	//no of write retires before flagging a fatal error

volatile enum FS_STATE {
	fsStateIdle,
	fsStateWaitingForWriteComplete
} fsState;

//file system params
typedef struct {
	Uint32 pageSize;
	Uint32 noOfPages;
	Uint32 noOfLogicalBlocks;
	Uint32 pagesPerLogicalBlock;
	Uint32 logicalBlocksPerFileBoundary;
	Uint32 noOfFileBoundarys;
} Tfs;

//block header
typedef struct {
	Uint16 id; //always 0x5abc
	Uint32 fileStartBlock;
	Uint32 blockNumber;
	Uint32 blockLength; //in bytes
	Uint32 time;
	Uint16 serial0;
	Uint16 serial1;
	Uint16 serial2;
	Uint16 serial3;
	Uint16 crc;
} TblockHeader;


// Write Buffer
#pragma DATA_ALIGN(blockBuff, 8); //align for DMA 
#pragma	DATA_SECTION(blockBuff,".sdbuf")
Uint16 blockBuff[FS_PAGES_PER_BLOCK * FS_PAGE_SIZE /2];//write buffer

Tfs fs; //file system table

volatile Uint32 nextLogicalBlockToWrite; //next logical address block to be written
volatile Uint16 bufWritePos;		//write buffer cursor
volatile Bool flashActive;  		//indicates if write process is active
Uint16 flashWriteRetryCount;
Uint32 fileStartBlock;	//start block for current file
int blocksWritten;	//for debug TODO - remove
int reOpenRequested;
Uint32 fsSectorsUsed;
Uint32 fsTotalSectors;

// writefrag function implemented is flsh.c 
extern int	writefrag(FLSH_Buff *buff) ;

//Write a logical block
int fsWriteLogBlock(Uint32 logBlockAddr, Uint16 *pWriteBuffer, Uint16 noOfBytes)
{
	Uint32 cardAddr = logBlockAddr * fs.pagesPerLogicalBlock;
	return sdWrite(cardAddr, noOfBytes, pWriteBuffer);
}

//Read a logical block
int fsReadLogBlock(Uint32 logBlockAddr, Uint16 *pReadBuffer, Uint16 noOfBytes)
{
	Uint32 pageAddr = logBlockAddr * fs.pagesPerLogicalBlock;
	return sdRead(pageAddr, noOfBytes, pReadBuffer, 1);
}

//Find first empty file boundary using binary search 
Int32 fsBinarySearch(Int32 imin, Int32 imax)
{
	Int32 imid; //file address
	Uint32 lb; //logical block address
  	if(imin > imax)
    	return imin;
  	else {
     	imid = (imin + imax) / 2;
     	lb = imid*fs.logicalBlocksPerFileBoundary;
	  	if(fsReadLogBlock(lb, blockBuff, 512) != CSL_SOK) return -1;
      	if ((blockBuff[0] == 0xFFFF) || (blockBuff[0] == 0x0000))
        	return fsBinarySearch(imin, imid-1);
      	else
        	return fsBinarySearch(imid+1, imax);
    }
}

//Find first empty file boundary
int	fsFindFirstFreeFile(Uint32 *firstFreeFile)
{
	Int32 result = fsBinarySearch(0, fs.noOfFileBoundarys-1);
	if(result >= 0) {
		*firstFreeFile = result;
		return(OK);
	}
	return(FAIL);
}

//Calculate space (words) remaining in buffer
inline Uint16 fsCalcWordsRemainingInBlock()
{
	return ( sizeof(blockBuff) - bufWritePos );
}

//Flush the write buffer 
int fsflush()
{
	while(fsCalcWordsRemainingInBlock() > 0) blockBuff[bufWritePos++] = 0xFFFF;
	fsState = fsStateWaitingForWriteComplete;
	return fsWriteLogBlock(nextLogicalBlockToWrite, blockBuff, sizeof(blockBuff)*2);
}

//Terminate all flash writing services
int	fsEnd(void)
{
 	fsflush();
 	return(OK) ;
}

//Initialise a fresh logical block
void fsPrepareBlockHeader()
{
	TblockHeader *bh = (TblockHeader *)blockBuff;
	bufWritePos = 0;
	bh->id = 0x6abc;
	bh->fileStartBlock = fileStartBlock;
	bh->blockNumber = nextLogicalBlockToWrite;
	bh->blockLength = sizeof(blockBuff)*2; //in bytes
	bh->time = TIMR_getUnixTime();
	bh->serial0 = DeviceId[1];
	bh->serial1 = DeviceId[0];
	bh->serial2 = DeviceId[3];
	bh->serial3 = DeviceId[2];
	bh->crc = crc(bh, sizeof(TblockHeader) - 2);
	bufWritePos+= sizeof(TblockHeader);
}

//Open a new file. Seeks to next file boundary and intialises new logical block
int	fsFileOpen(void)
{
	//seek to next file boundary
	if(nextLogicalBlockToWrite % fs.logicalBlocksPerFileBoundary) {
		nextLogicalBlockToWrite = ((nextLogicalBlockToWrite / fs.logicalBlocksPerFileBoundary) + 1) * fs.logicalBlocksPerFileBoundary;
	}
	fileStartBlock = nextLogicalBlockToWrite;
	return(OK) ;
}

//Disused
void fsStatus(void)
{
}

//Caches data to 'blockBuff' and writes to flash when buffer is full. 
Uint16 fsWrite(void *buf, Uint16 count)
{
	Uint16 result = MIN( fsCalcWordsRemainingInBlock(), count);
	safe_memcpy(&blockBuff[bufWritePos], buf, result);
	bufWritePos+= result;
	if( fsCalcWordsRemainingInBlock() == 0 ) {
		fsState = fsStateWaitingForWriteComplete;
		fsWriteLogBlock(nextLogicalBlockToWrite, blockBuff, sizeof(blockBuff)*2);
		return result;
	}
	return result;
}

//Consumes data packets obtained by calling the flsh.c 'writefrag' function. 
//Passes data to fsWrite where it is buffered before writing to flash
int fsProcessIncommingData(int *unused1, int *unused2)
{
	static FLSH_Buff fb; 
	static Uint16 wordsWritten = 0;
	flashActive = 1;
	do {
		if( fb.type == WRITE_FLUSH ) { //flush file
			reOpenRequested = 1;
			writefrag(&fb); //get next job;
			wordsWritten = 0;
			fsflush();
			return OK; //fs is busy, take a break & wait for interrupt
		}
		wordsWritten += fsWrite(&fb.p[wordsWritten], fb.n-wordsWritten);
		if(fsState == fsStateWaitingForWriteComplete) return OK; //fs is busy, take a break & wait for interrupt
		writefrag(&fb); //get next job
		wordsWritten = 0;
	} while(fb.type != WRITE_NONE);
	flashActive = 0;
	return OK;
}

#define	FS_RETRY_ERR (1)


void fsReportError(int etype)
{	
	char s[32] ; 
 	snprintf(s, 32, "ERROR=\"%d\"",etype);
 	INFO_event("FS",s,NULL) ;
}

//Called by hardware OnDataWritten interrupt. Checks if write was successful, 
//and retries where necessary. On success, initialises new block and resumes data 
//consumption process by calling fsWritePage function.
void fsOnWriteComplete(int success)
{
	if( fsState == fsStateWaitingForWriteComplete ) {
		if( success ) {
			blocksWritten++;
			flashWriteRetryCount = 0;
			nextLogicalBlockToWrite++;
			if(nextLogicalBlockToWrite > (fs.noOfLogicalBlocks - 2 )) {
				if(nextLogicalBlockToWrite == (fs.noOfLogicalBlocks - 1 )) {
					flsh_stop(FLSH_END);
					return;
				}
				else flsh_stop(FLSH_NEAREND);
			}
			bufWritePos = 0;
			if(reOpenRequested) {
				fsFileOpen();
				reOpenRequested = 0;
			}
			fsPrepareBlockHeader();
		}
		else {
			fsReportError(FS_RETRY_ERR);
			if(flashWriteRetryCount++ > FLASH_WRITE_RETRIES) {
				fatal(FLSH_MOD,FLASH_WRITE_FAIL) ;	// this is a fatal error - need to restart
			} //otherwise just wait for the write process to retry
		}
		fsState = fsStateIdle;
#ifdef PLOG
		fsProcessIncommingData(NULL, NULL);
#else
		JOB_postone((JOB_Fxn)fsProcessIncommingData, NULL, 3 | RUNONCE, NULL);
#endif
	}
}

//Called from flsh.c to test if the filesyetm is busy. 
Bool fsIsbusy(void) 
{
	return(flashActive);
}

//Called from flsh.c to intitiate the filesystem data processing routine. 
void fsNewdata(void)
{
	if(!fsIsbusy()) {
   		fsProcessIncommingData(NULL, NULL);
	}
}

int	fsClose(void)
{
	return sdClose();
}
/*
void fsSetCard(int cardNo)
{
	//ioeInit(0x0027, 0x00C0, 0x00FF);
	switch(cardNo) {
		case 1:
			gpioSetVal(GPIO_BIT_SD_EN, FALSE);
			ioeWritePU(AUDIO_IO_EXPANDER_DEVICE_ADDRESS_MC, 0x0008);
			ioeWrite(AUDIO_IO_EXPANDER_DEVICE_ADDRESS_MC, 0x0008);
			break;
		case 2:
			gpioSetVal(GPIO_BIT_SD_EN, FALSE);
			ioeWritePU(AUDIO_IO_EXPANDER_DEVICE_ADDRESS_MC, 0x0010);
			ioeWrite(AUDIO_IO_EXPANDER_DEVICE_ADDRESS_MC, 0x0010);
			break;
		case 3:
			gpioSetVal(GPIO_BIT_SD_EN, FALSE);
			ioeWritePU(AUDIO_IO_EXPANDER_DEVICE_ADDRESS_MC, 0x0020);
			ioeWrite(AUDIO_IO_EXPANDER_DEVICE_ADDRESS_MC, 0x0020);
			break;
		default: //main card
			gpioSetVal(GPIO_BIT_SD_EN, TRUE);
			ioeWritePU(AUDIO_IO_EXPANDER_DEVICE_ADDRESS_MC, 0x0000);
			ioeWrite(AUDIO_IO_EXPANDER_DEVICE_ADDRESS_MC, 0x0000);
			break;

	}

	microdelay(0xffff);
}
*/


int	fsInitCard(void)
{
	Uint32 firstFreeFile;
	flashActive = 0;
	blocksWritten = 0;
	bufWritePos = 0;
	flashWriteRetryCount = 0;
	fsState = fsStateIdle;
	reOpenRequested = 0;

	if(sdInit(0) != OK) {
		microdelay(0xffff);
		if(sdInit(0) != OK) {
			microdelay(0xffff);
			if(sdInit(0) != OK) {
				return FAIL;
			}
		}
	}

	sdInterruptEnable(FALSE);
	
	sdSetWriteCompleteHandler(fsOnWriteComplete);	
	
	if( sdGetSectorCount(&fs.noOfPages) != OK) return FALSE;

	fs.pageSize = FS_PAGE_SIZE;
	fs.pagesPerLogicalBlock = FS_PAGES_PER_BLOCK;
	fs.noOfLogicalBlocks = fs.noOfPages / fs.pagesPerLogicalBlock;
	fs.logicalBlocksPerFileBoundary = FS_BLOCKS_PER_FILE;
	fs.noOfFileBoundarys = fs.noOfLogicalBlocks / fs.logicalBlocksPerFileBoundary;


	if( fsFindFirstFreeFile(&firstFreeFile) != OK) {
		return FAIL;
	}

	if( firstFreeFile >= fs.noOfFileBoundarys ) {
		return FAIL_FS_FULL;
	}

	nextLogicalBlockToWrite = firstFreeFile * fs.logicalBlocksPerFileBoundary;
	fsFileOpen(); //TODO - move back to flsh.c
	fsPrepareBlockHeader();
	
	sdInterruptEnable(TRUE);
 	return(OK) ;
}

//Find a good memeory card and initialize the filesystem
int	fsInit(void)
{
	Uint16 activeCard = hidMemCardCount-1;
	int noOfCards = hidMemCardCount;

	int retries = 0;
	do { //check each card for space, loop twice
		memCardSelect(activeCard);
		microdelay(0xffff);
		if( fsInitCard() == OK )
			break;
		else
		{
			//sdIdle();
			if(activeCard ) activeCard--; else activeCard = hidMemCardCount-1;
		}
	} while (retries++ < noOfCards); //scan through the cards

	if( retries >= noOfCards) {
		return FAIL_FS_FULL; //all cards are either full or absent
	}

	return OK;
}

