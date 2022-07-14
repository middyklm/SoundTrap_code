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

#ifndef SD_H_
#define SD_H_

#include <tistdtypes.h>

typedef struct {
	Uint16 csdStruct;          // CSD version; 0 - 1.0, 1 - 2.0            
	Uint16    mmcProt;         // MMC protocol version                     
	Uint16    taac;            // TAAC                                     
	Uint16    nsac;            // NSAC                                     
	Uint16    tranSpeed;       // Max data transmission speed              
	Uint16    ccc;             // Card command classes                     
	Uint16    readBlLen;       // Maximum Read Block Length                
	Uint16    readBlPartial;   // Indicates if partial read blocks allowed 
	Uint16    writeBlkMisalign;// Flag indicates write block misalignment  
	Uint16    readBlkMisalign; // Flag indicates read block misalignment   
	Uint16    dsrImp;          // Flag indicates whether card has DSR reg  
	Uint32    cSize;           // Device size
	Uint16    vddRCurrMin;     // Max. Read Current @ Vdd Min              
	Uint16    vddRCurrMax;     // Max. Read Current @ Vdd Max              
	Uint16    vddWCurrMin;     // Max. Write Current @ Vdd Min             
	Uint16    vddWCurrMax;     // Max. Write Current @ Vdd Max             
	Uint16    cSizeMult;       // Device size multiplier                   
	Uint16    eraseBlkEnable;  // Erase single block enable                
	Uint16    eraseGrpSize;    // Erase sector group size                  
	Uint16    eraseGrpMult;    // Erase group multiplier                   
	Uint16    wpGrpSize;       // Write protect group size                 
	Uint16    wpGrpEnable;     // Write protect enable flag                
	Uint16    defaultEcc;      // Manufacturer Default ECC                 
	Uint16    r2wFactor;       // Stream write factor                      
	Uint16    writeBlLen;      // Maximum write block length               
	Uint16    writeBlPartial;  // Indicates if partial write blocks allowed
	Uint16    contProtApp;     // Content protection application           
	Uint16    fileFmtGrp;      // File format group                        
	Uint16    copyFlag;        // Copy flag                                
	Uint16    permWriteProtect;// Dis/en-able permanent write protection   
	Uint16    tmpWriteProtect; // Dis/en-able temporary write protection   
	Uint16    fileFmt;         // File format                              
	Uint16    ecc;             // ECC code                                 
	Uint16    crc;             // Cyclic redundancy check                  
} SdCsd;

typedef struct {
	Uint16    mfgId;          	// 8 bit Manufacturer ID                     
	Uint16    oemAppId;       	// 16 bit OEM and application ID             
	Uint8     productName[6]; 	// 40 or 48 bit Product name                 
	Uint16    productRev;     	// 8 bit Product Revision Number             
	Uint32    serialNumber;   	// 32 bit Serial Number                      
	Uint16    month;          	//4 bit Manufacturing Date (Month)          
	Uint16    year;         	//4 bit Manufacturing Date (Year) in case of
								//MMC (Year 0 = 1997). SD has an 8 bit year field.(Year 0 = 2000)          
	Uint16    checksum;       	// 7 bit crc                                 
} SdCardId;


typedef struct {
	Uint16 rca;  //Relative card address (RCA) published by the card
	Uint32 cardCapacity; 
	Uint32 blockLength;
	Uint32 totalSectors;
	SdCardId cid; 	//Manufacturers Card ID
	SdCsd csd; 		//Card specific data
} SdCard;

int sdWaitForReadDmaDone(int sendStop);
int sdRead(Uint32 cardAddr, Uint16 noOfBytes, Uint16 *pReadBuffer, int wait);
int sdWrite(Uint32 cardAddr, Uint16 noOfBytes, Uint16 *pWriteBuffer);
int sdSendTransmissionStopCommand();
int sdGetSectorCount(Uint32 *secctorCount);
int sdCheckLastWriteStatus();
int sdInterruptEnable(int enable);
int sdInit(int divider);
int sdSetWriteBlockEraseCount(Uint32 count);
int sdWaitForSdWriteCompletion(Int16 timeout);
int sdSetWriteCompleteHandler( void (*handler)(int status) );
int sdErase(Uint32 startAddress, Uint32 blkCnt);
int sdSetPreEraseSize(Uint32 blocks);
int sdClose();
int sdGetCardInfo(Uint16 *info, int maxLangth);
int sdSendGetStatusCommand();
int sdIdle();

#endif /*SD_H_*/
