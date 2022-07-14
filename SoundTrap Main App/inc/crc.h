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

#ifndef _CRC_H_
#define _CRC_H_

#include <tistdtypes.h>
#include "d3defs.h"

// CRC module - cyclic redundancy checks
// crc.h v1.0

// Assembly language routines to perform the following type of 16-bit
// CRC from byte-oriented data.
//
// unsigned int crc16(unsigned int *vec, int length)
// {
// unsigned int fcs = CRC_SEED ;
//
//  while(length--) {
//    fcs = (fcs << 8) ^ CRCtab[((fcs>>8) ^ (*vec >> 8)) & 0xff]; 
//    fcs = (fcs << 8) ^ CRCtab[((fcs>>8) ^ *vec++) & 0xff]; 
//    }
//
// return(fcs);
//}



#define  CRC_SEED		(0x0ffff)

// all functions are in crc.s55
extern   void crcunpack1lo(uns *crc, uns *ovec, uns *ivec, int len) ;
extern   void crcunpack1hi(uns *crc, uns *ovec, uns *ivec, int len) ;
extern   void crcunpack2lo(uns *crc, uns *ovec, uns *ivec, int len) ;
extern   void crcunpack2hi(uns *crc, uns *ovec, uns *ivec, int len) ;
extern   void crc2(uns *crc, uns *ivec, int len) ;

Uint16 crc(void *buf, Uint32 len) ;


#endif
