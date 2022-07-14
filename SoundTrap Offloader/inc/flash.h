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

#ifndef FLASH_H
#define FLASH_H

#include <tistdtypes.h>

extern Uint32 flashTotalPageCount;
extern Uint32 flashFirstFreeSector;



int flashGetNoOfPages(Uint32 *pages);
int flashSetReadAddress(Uint32 address);
int	flashInit(void);
int flashErase(Uint32 startAddress, Uint32 blkCnt);
int flashRead(Uint32 pageAddr, Uint16 *pReadBuffer, Uint16 noOfBytes);
int flashWrite(Uint32 pageAddr, Uint16 *pReadBuffer, Uint16 noOfBytes);
int flashReadNextPage(Uint16 **pReadBuffer);
int flashSetReadIncrement(Uint32 increment);
int flashGetFirmwareDetails(char *ver, Uint16 maxCount, Uint16 *count);
int flashErase(Uint32 startAddress, Uint32 pageCount);


#endif
