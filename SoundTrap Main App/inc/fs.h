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

#ifndef FS_H_
#define FS_H_

#include   		<tistdtypes.h>

#define FS_PAGE_SIZE 512 //bytes
#define FS_PAGES_PER_BLOCK 32 //16kB
#define FS_BLOCKS_PER_FILE 256 //4MB
#define FS_PAGES_PER_FILE (FS_PAGES_PER_BLOCK * FS_BLOCKS_PER_FILE)

#define FS_PAGE_ADDRESS_FIRMWARE 0
#define FS_PAGE_ADDRESS_CONFIG ((FS_BLOCKS_PER_FILE - 1) * FS_PAGES_PER_BLOCK)

#define FAIL_FS_FULL 2

int	fsInit(void) ;
void fsNewdata(void) ;
void fsStatus(void) ;
int	fsIsbusy(void) ;
int fsWriteLogBlock(Uint32 logBlockAddr, Uint16 *pWriteBuffer, Uint16 noOfBytes);
int fsReadLogBlock(Uint32 logBlockAddr, Uint16 *pReadBuffer, Uint16 noOfBytes);
int fsflush();
int	fsClose(void);

#endif /*FS_H_*/
