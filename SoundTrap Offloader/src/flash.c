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
#include 		"sd.h"
#include 		"d3std.h"
#include 		"tick.h"
#include 		"gpio.h"
#include 		"fs.h"


#define PAGE_LENGTH_BYTES 512
#define BUF_LENGTH_PAGES 32
#define DMA_READ_TIMEOUT_MS 100

//ping pong buffer
#pragma	DATA_SECTION(flashBuffer,".flashBuffer")
#pragma DATA_ALIGN(flashBuffer, 8);
static Uint16 flashBuffer[2][BUF_LENGTH_PAGES * PAGE_LENGTH_BYTES / 2];

static Uint16 curPage;
static Uint16 pages = 0;
static Uint16 pagesToRead;

static Uint32 flashReadPageAddress;
static Uint32 flashReadPageIncrement;
static int nextBuffer = 0; //ping or pong

Uint32 flashTotalPageCount;
Uint32 flashFirstFreeSector;


int flashGetNoOfPages(Uint32 *pages)
{
	return sdGetSectorCount(pages);
}

int flashErase(Uint32 startAddress, Uint32 pageCount)
{
	return sdErase(startAddress, pageCount);
}

//return ping and start DMA filling pong
int fillBuffer(int buffer, int pages)
{
	int inc = pages > 1 ? pages : flashReadPageIncrement;
	if( sdWaitForReadDmaDone(1) != OK ) { 		//wait for previous read to complete
		sdSendGetStatusCommand();
	}
	//initiate the next DMA read - don't wait for completion
	if( sdRead(flashReadPageAddress, pages* PAGE_LENGTH_BYTES, flashBuffer[buffer], 0) != OK)  {
		return FAIL;
	}
	if( flashReadPageAddress + inc < flashTotalPageCount )	flashReadPageAddress += inc;

	return OK;
}

//Set SD page address, and start filling buffers
int flashSetReadAddress(Uint32 address)
{
	if( address < flashTotalPageCount ) {
		flashReadPageAddress = address;
		if( fillBuffer(0, pagesToRead) == OK) {
			nextBuffer = 0;
			pages = 0;
			return OK;
		}
	}
	return FAIL;
}

int flashSetReadIncrement(Uint32 increment)
{
	flashReadPageIncrement = increment;
	pagesToRead = flashReadPageIncrement > 1 ? 1 : BUF_LENGTH_PAGES;
	return OK;
}

int flashRequestNextBuffer(int *nextBufferIndex)
{
	*nextBufferIndex = nextBuffer;
	nextBuffer = !nextBuffer;
	if( fillBuffer(nextBuffer, pagesToRead) != OK) return FAIL; //start dma filling next buffer - don't wait
	return OK;
}

int flashReadNextPage(Uint16 **pReadBuffer)
{
	static int currentbuffer;
	if(pages == 0) {
		if( flashRequestNextBuffer(&currentbuffer) != OK) return FAIL;
		pages = pagesToRead;
		curPage = 0;
	}
	
	*pReadBuffer = &flashBuffer[currentbuffer][curPage * 256];
	pages--;
	curPage++;
	return OK;
}

int flashGetFirmwareDetails(char *ver, Uint16 maxCount, Uint16 *count)
{
	sdRead(0, PAGE_LENGTH_BYTES, flashBuffer[0], 1);
	if(flashBuffer[0][0] == 0x09AA)
	{
		*count = flashBuffer[0][4];
		if(*count > maxCount) *count = maxCount;
		memcpy(ver, &flashBuffer[0][7], *count);
		return OK;
	}
	*count = 0;
	return OK;
}




Uint16 blockBuff[256];

//Find first empty file boundary using binary search
Int32 flashBinarySearch(Int32 imin, Int32 imax)
{
	Int32 imid; //file address
	Uint32 lb; //logical block address
  	if(imin > imax)
    	return imin;
  	else {
     	imid = (imin + imax) / 2;
     	lb = imid*FS_BLOCKS_PER_FILE;

	  	if(sdRead(lb * FS_PAGES_PER_BLOCK, 512, blockBuff, 1) != 0) return -1;

      	if ((blockBuff[0] == 0xFFFF) || (blockBuff[0] == 0x0000))
        	return flashBinarySearch(imin, imid-1);
      	else
        	return flashBinarySearch(imid+1, imax);
    }
}

//Find first empty file boundary
int	flashFindFirstFreeFile()
{

	Uint32 noOfLogicalBlocks = flashTotalPageCount / FS_PAGES_PER_BLOCK;
	Uint32 noOfFileBoundarys = noOfLogicalBlocks / FS_BLOCKS_PER_FILE;

	Int32 result = flashBinarySearch(0, noOfFileBoundarys-1);
	if(result >= 0) {
		flashFirstFreeSector = result*FS_PAGES_PER_FILE;
		return(OK);
	}
	return(FAIL);
}


int flashInit(void)
{
	flashTotalPageCount = 0;
	flashFirstFreeSector = 0;
	if(sdInit(0) != OK) return FAIL;
	if(flashGetNoOfPages(&flashTotalPageCount) != OK) return FAIL;
	flashFindFirstFreeFile();
	return OK;
}

