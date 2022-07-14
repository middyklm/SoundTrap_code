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
 * C5535 SDHC hardware interface. Implements filesystem for writing blocks to SDHC hardware.
 *  
 * Dependant on TI C55 CSL v2.5
 * 
 * Note - does not support standard capacity SD cards. So only to be used with SD cards > 4GB. 
 * 
 * JMJA 
 * 
 */

#include <tistdtypes.h>
#include <csl_dma.h>

#include 		"irq.h"
#include 		"sd.h"
#include 		"soc.h"
#include 		"d3std.h"
#include		"dma.h"
#include		"protect.h"
#include		"gpio.h"
#include		"timr.h"

#define SD_PAGE_LEN 512

/*
 * Command Types Ref 
 * R0  		0x00xx
 * R1/R6  	0x02xx
 * R1b 		0x03xx
 */

//SD Commands
#define CMD_GO_IDLE 0x0000
#define CMD_GET_CID 0x0402
#define CMD_SEND_REL_ADD 0x0203
#define CMD_SELECT_CARD 0x0307
#define CMD_SEND_IF_COND 0x0208
#define CMD_SEND_CSD 0x0409
#define CMD_ERASE_START 0x0220
#define CMD_ERASE_END 0x0221
#define CMD_ERASE 0x0326
#define CMD_SEND_OP_COND 0x0629
#define CMD_APP_CMD 0x0237

#define CMD_SET_BLK_COUNT 0x0417


#define CMD_STOP 0x038C
#define CMD_RD_MULT_BLK 0xA212
#define CMD_RD_BLK 0xA211
#define CMD_WR_MULT_BLK 0x2A19
#define CMD_WR_BLK 0x2A18

//Register Bits
#define MMCST0_DATDNE 	0x0001
#define MMCST0_BSYDNE 	0x0002
#define MMCST0_RSPDNE 	0x0004
#define MMCST0_TOUTRD 	0x0008
#define MMCST0_TOUTRS   0x0010
#define MMCST0_CRCWR   	0x0020
#define MMCST0_CRCRD 	0x0040
#define MMCST0_CRCRS 	0x0080
#define MMCST0_DXRDY 	0x0200
#define MMCST0_DRRDY 	0x0400
#define MMCST0_DATED 	0x0800
#define MMCST0_TRNDNE 	0x1000

#define MMCST1_FIFOEMP 	0x0020
#define MMCST1_BUSY 	0x0001

#define MMCIM_DATDNE	0x0001
#define MMCIM_EBSYDNE	0x0002
#define MMCIM_ERSPDNE	0x0004
#define MMCIM_ETOUTRD	0x0008
#define MMCIM_ETOUTRS	0x0010
#define MMCIM_ECRCWR	0x0020
#define MMCIM_ECRCRD	0x0040
#define MMCIM_ECRCRS	0x0080
#define MMCIM_EDXRDY	0x0200
#define MMCIM_EDRRDY	0x0400
#define MMCIM_EDATED	0x0800
#define MMCIM_ETRNDNE	0x1000

//SD controller register map
typedef struct  {
    volatile Uint16 MMCCTL;
    volatile Uint16 RSVD0[3];
    volatile Uint16 MMCCLK;
    volatile Uint16 RSVD1[3];
    volatile Uint16 MMCST0;
    volatile Uint16 RSVD2[3];
    volatile Uint16 MMCST1;
    volatile Uint16 RSVD3[3];
    volatile Uint16 MMCIM;
    volatile Uint16 RSVD4[3];
    volatile Uint16 MMCTOR;
    volatile Uint16 RSVD5[3];
    volatile Uint16 MMCTOD;
    volatile Uint16 RSVD6[3];
    volatile Uint16 MMCBLEN;
    volatile Uint16 RSVD7[3];
    volatile Uint16 MMCNBLK;
    volatile Uint16 RSVD8[3];
    volatile Uint16 MMCNBLC;
    volatile Uint16 RSVD9[3];
    volatile Uint16 MMCDRR1;
    volatile Uint16 MMCDRR2;
    volatile Uint16 RSVD10[2];
    volatile Uint16 MMCDXR1;
    volatile Uint16 MMCDXR2;
    volatile Uint16 RSVD11[2];
    volatile Uint16 MMCCMD1;
    volatile Uint16 MMCCMD2;
    volatile Uint16 RSVD12[2];
    volatile Uint16 MMCARG1;
    volatile Uint16 MMCARG2;
    volatile Uint16 RSVD13[2];
    volatile Uint16 MMCRSP0;
    volatile Uint16 MMCRSP1;
    volatile Uint16 RSVD14[2];
    volatile Uint16 MMCRSP2;
    volatile Uint16 MMCRSP3;
    volatile Uint16 RSVD15[2];
    volatile Uint16 MMCRSP4;
    volatile Uint16 MMCRSP5;
    volatile Uint16 RSVD16[2];
    volatile Uint16 MMCRSP6;
    volatile Uint16 MMCRSP7;
    volatile Uint16 RSVD17[2];
    volatile Uint16 MMCDRSP;
    volatile Uint16 RSVD18[7];
    volatile Uint16 MMCCIDX;
    volatile Uint16 RSVD19[19];
    volatile Uint16 SDIOCTL;
    volatile Uint16 RSVD20[3];
    volatile Uint16 SDIOST0;
    volatile Uint16 RSVD21[3];
    volatile Uint16 SDIOIEN;
    volatile Uint16 RSVD22[3];
    volatile Uint16 SDIOIST;
    volatile Uint16 RSVD23[3];
    volatile Uint16 MMCFIFOCTL;
} SdRegs;

typedef volatile ioport SdRegs *SdRegsOvly;
SdRegsOvly sdRegs;

#define TIME_OUT_MS 200

//DMA objects
CSL_DMA_Config dmaWriteCfg;
CSL_DMA_Config dmaReadCfg;
CSL_DMA_Handle   	  dmaWrObj;
CSL_DMA_Handle   	  dmaRdObj;

//SD Card object
SdCard sdCard;

Fxn writeCompleteHandler;

//Send a command to the card
int sdSendCmd(Uint16 command, Uint32 arg)
{
	Uint16 status;
	sdRegs->MMCST0 = 0x0000;
    sdRegs->MMCARG1 = arg & 0xffff;
    sdRegs->MMCARG2 = arg >> 16;
	sdRegs->MMCCMD1 = command;
    do {
        status = sdRegs->MMCST0;
		if(status & (MMCST0_TOUTRS | MMCST0_CRCRS)) {
			return(FAIL);
		}
	} while((status & MMCST0_RSPDNE) == 0);
    return (OK);
}

//Wait for the read DMA to complete
int sdWaitForReadDmaDone(int sendStop)
{
	Uint32 start = TIMR_GetTickCount();
	while (dmaRdObj->dmaRegs->DMACH0TCR2 & 0x4000) {
		if(TIMR_GetTickCount() - start > TIME_OUT_MS )
			return FAIL;
	}
	if(sendStop) return sdSendTransmissionStopCommand();
	return OK;
}

//Send transmission stop command. Required post multiple block read/write
int sdSendTransmissionStopCommand()
{
	return sdSendCmd(CMD_STOP, 0x0000);
}

int sdSendGetStatusCommand()
{
	return sdSendCmd(0x020D, 0x0000);
}

//Read block from card. 
//NoOfBytes should be multiple of 512
int sdRead(Uint32 cardAddr, Uint16 noOfBytes, Uint16 *pReadBuffer, int wait)
{
	Uint32 start;
	Uint16             blkCnt;
	Uint16             readCmd;

	blkCnt = (noOfBytes)/ SD_PAGE_LEN;
	sdRegs->MMCBLEN = SD_PAGE_LEN;
	sdRegs->MMCNBLK = blkCnt; 

	start = TIMR_GetTickCount();
	while (dmaWrObj->dmaRegs-> DMACH1TCR2 & 0x4000) {
		if(TIMR_GetTickCount() - start > TIME_OUT_MS )
			return FAIL;
	}

	start = TIMR_GetTickCount();
    while( (!(sdRegs->MMCST1 & MMCST1_FIFOEMP)) || (sdRegs->MMCST1 & MMCST1_BUSY))
    {
		if(TIMR_GetTickCount() - start > TIME_OUT_MS )
			return FAIL;
    }

	sdRegs->MMCFIFOCTL = 0x0001; //reset FIFO
    sdRegs->MMCFIFOCTL = 0x0004; //config FIFO - read, 4byte width, 256bit level

	readCmd = blkCnt > 1 ? CMD_RD_MULT_BLK : CMD_RD_BLK;

	/* Configure DMA channel */
	dmaReadCfg.dataLen  = noOfBytes;
	dmaReadCfg.destAddr = (Uint32)pReadBuffer;

	if( DMA_config(dmaRdObj, &dmaReadCfg) != CSL_SOK ) return FAIL;
	if( DMA_start(dmaRdObj) != CSL_SOK ) return FAIL;

	if( sdSendCmd(readCmd, cardAddr) != OK ) return FAIL;

	if(wait) {
		sdWaitForReadDmaDone(0);
		if((sdRegs->MMCST0 & (MMCST0_TOUTRD | MMCST0_CRCRD)) != 0) return FAIL;
		if(readCmd == CMD_RD_MULT_BLK) sdSendTransmissionStopCommand();	
	}
	
	return OK;
}


//Write block to card.
//NoOfBytes should be multiple of 512
int sdWrite(Uint32 cardAddr, Uint16 noOfBytes, Uint16 *pWriteBuffer)
{
	Uint16 blkCnt;
	Uint16 writeCmd;
	Uint16 status;
	
	blkCnt = (noOfBytes)/(SD_PAGE_LEN);

    /* Set block length */
	sdRegs->MMCBLEN = SD_PAGE_LEN;
	sdRegs->MMCNBLK = blkCnt; 

    while(( sdRegs->MMCST1 & MMCST1_FIFOEMP)!= MMCST1_FIFOEMP ){
    	status = sdRegs->MMCST0;
    	if((status & MMCST0_DATDNE) == MMCST0_DATDNE) {
    		break;
    	}
    }

	while((sdRegs->MMCST1 & MMCST1_BUSY) == MMCST1_BUSY){};

	sdRegs->MMCFIFOCTL = 0x0001; //reset FIFO
    sdRegs->MMCFIFOCTL = 0x0006; //write, 4byte width, 256bit level

	// Check whether to send single block write or multi block write 
	writeCmd = blkCnt > 1 ? CMD_WR_MULT_BLK : CMD_WR_BLK;

	// Configure DMA channel 
	dmaWriteCfg.dataLen = noOfBytes;
	dmaWriteCfg.srcAddr = (Uint32)pWriteBuffer;
	DMA_config(dmaWrObj, &(dmaWriteCfg));
	DMA_start(dmaWrObj);

//	if( sdSendCmd(writeCmd, cardAddr) != OK ) return FAIL;
	sdRegs->MMCST0 = 0x0000;
    sdRegs->MMCARG1 = cardAddr & 0xffff;
    sdRegs->MMCARG2 = cardAddr >> 16;
	sdRegs->MMCCMD1 = writeCmd;

  	/* trigger first DMA event*/
  	sdRegs->MMCCMD2 |= 0x0001;
  	
	return OK;
}

int sdGetSectorCount(Uint32 *secctorCount)
{
	*secctorCount = sdCard.totalSectors;
	return OK;
}
int sdErase(Uint32 startAddress, Uint32 pageCount)
{
	Uint16    mmcStatus;
    if(pageCount > 0) {
		if( sdSendCmd(CMD_ERASE_START, startAddress) == OK) {
			if( sdSendCmd(CMD_ERASE_END, startAddress + pageCount) == OK) {
				if(sdSendCmd(CMD_ERASE, 0x0000) == OK) {
					do {
						mmcStatus = sdRegs->MMCST1;
					} while(mmcStatus & 0x0001);
					if(sdSendCmd(0x020D, (Uint32)sdCard.rca << 16)  == OK) {
						return sdRegs->MMCRSP7;
					}
				}
			}
		}
	}
	return FAIL;
}

int sdGetRca(Uint16 *rca)
{
	if( sdSendCmd(CMD_SEND_REL_ADD, 0) != OK ) return FAIL; 
	*rca = sdRegs->MMCRSP7;
	return OK;
}

interrupt void sdOnWriteComplete(void)
{
	Uint16 status;
	PROTECT;
	status = sdRegs->MMCST0;
	if( status & MMCIM_ETRNDNE ) {
		sdSendTransmissionStopCommand();
	}
	if(writeCompleteHandler != NULL) {
		writeCompleteHandler(!(status & (MMCIM_ECRCWR | MMCIM_ETOUTRS)));
	}
	END_PROTECT;
}

int sdInterruptEnable(Bool enable)
{
	irqClear(PROG0_EVENT);
	if(enable)
		irqEnable(PROG0_EVENT);
	else
		irqDisable(PROG0_EVENT);
	return OK;
}

int sdGetCardCsd(Uint32 rca)
{
	int csizeMult;
	if( sdSendCmd(CMD_SEND_CSD, (Uint32)rca << 16) != OK) return(FAIL);
	if( sdSendCmd(CMD_SELECT_CARD, (Uint32)rca << 16) != OK) return(FAIL); 

    /* Update the CSD structure */
	sdCard.csd.csdStruct 		= (sdRegs->MMCRSP7 >> 14) & 0x3;
	/* CSD version is 2.0 - SD spec Version 2.00/High Capacity */
	sdCard.csd.crc              = (sdRegs->MMCRSP0 >> 1) & 0x7F;
	sdCard.csd.ecc              = (sdRegs->MMCRSP0 >> 8) & 0x3;
	sdCard.csd.fileFmt          = (sdRegs->MMCRSP0 >> 10) & 0x3;
	sdCard.csd.tmpWriteProtect  = (sdRegs->MMCRSP0 >> 12) & 0x1;
	sdCard.csd.permWriteProtect = (sdRegs->MMCRSP0 >> 13) & 0x1;
	sdCard.csd.copyFlag         = (sdRegs->MMCRSP0 >> 14) & 0x1;
	sdCard.csd.fileFmtGrp       = (sdRegs->MMCRSP0 >> 15) & 0x1;
	sdCard.csd.writeBlPartial   = (sdRegs->MMCRSP1 >> 5) & 0x1;
	sdCard.csd.writeBlLen       = (sdRegs->MMCRSP1 >> 6) & 0xF;
	sdCard.csd.r2wFactor        = (sdRegs->MMCRSP1 >> 10) & 0x7;
	sdCard.csd.wpGrpEnable      = (sdRegs->MMCRSP1 >> 15) & 0x1;
	sdCard.csd.wpGrpSize        = (sdRegs->MMCRSP2) & 0x7F;
	sdCard.csd.eraseGrpSize     = (sdRegs->MMCRSP2 >> 7) & 0x7F;
	sdCard.csd.eraseBlkEnable   = (sdRegs->MMCRSP2 >> 14) & 0x1;
	sdCard.csd.cSize            = ((Uint32)(sdRegs->MMCRSP4 & 0x003F) << 16) | sdRegs->MMCRSP3;
	sdCard.csd.dsrImp           = (sdRegs->MMCRSP4 >> 12) & 0x1;
	sdCard.csd.readBlkMisalign  = (sdRegs->MMCRSP4 >> 13) &0x1;
	sdCard.csd.writeBlkMisalign = (sdRegs->MMCRSP4 >> 14) &0x1;
	sdCard.csd.readBlPartial    = (sdRegs->MMCRSP4 >> 15) &0x1;
	sdCard.csd.readBlLen        = (sdRegs->MMCRSP5) & 0xF;
	sdCard.csd.ccc              = (sdRegs->MMCRSP5 >> 4) & 0xFFF;
	sdCard.csd.tranSpeed        = (sdRegs->MMCRSP6) & 0xFF;
	sdCard.csd.nsac             = (sdRegs->MMCRSP6 >> 8) & 0xFF;
	sdCard.csd.taac             = (sdRegs->MMCRSP7) & 0xFF;
	
	csizeMult = ((sdRegs->MMCRSP3 & 0x0003) << 2) | ((sdRegs->MMCRSP2 >> 15) & 0x0001);
	csizeMult = 1 << (csizeMult +2);
	sdCard.blockLength    = 1 << sdCard.csd.readBlLen;
	sdCard.cardCapacity   = ((Uint32)(sdCard.csd.cSize + 1) * 512);
	sdCard.totalSectors   = (sdCard.cardCapacity * 2);
	return (OK);
}

int sdGetCardInfo(Uint16 *info, int maxLangth)
{
	memcpy(sdCard.cid.productName, info, 6);
	return 6;
}


int sdGetCID()
{
	if( sdSendCmd(CMD_GET_CID, 0x0000) != OK) return(FAIL); //CMD2 
    // Update the CID structure 
    sdCard.cid.mfgId       = ((sdRegs->MMCRSP7 & 0xFF00) >> 8) & 0xFF;
    sdCard.cid.oemAppId    = ((sdRegs->MMCRSP7  & 0x00FF) << 8) | ((sdRegs->MMCRSP6  & 0xFF00) >> 8);
    sdCard.cid.productName[5]  = '\0';
    sdCard.cid.productName[4]  = ((sdRegs->MMCRSP4) & 0x00FF);
    sdCard.cid.productName[3]  = ((sdRegs->MMCRSP4) >> 8);
    sdCard.cid.productName[2]  = ((sdRegs->MMCRSP5) & 0x00FF);
    sdCard.cid.productName[1]  = ((sdRegs->MMCRSP5) >> 8);
    sdCard.cid.productName[0]  = ((sdRegs->MMCRSP6) & 0x00FF);
    sdCard.cid.serialNumber  = ((Uint32)(sdRegs->MMCRSP3 & 0x00FF) << 16) | ((Uint32)sdRegs->MMCRSP2 << 8) | ((Uint32)(sdRegs->MMCRSP1 >> 8));
    sdCard.cid.month = ((sdRegs->MMCRSP0 >> 8) & 0x0F);
    sdCard.cid.year  = ((sdRegs->MMCRSP0 >> 12) | ((sdRegs->MMCRSP1 & 0x000F) << 4) + 2000);
    sdCard.cid.checksum = (sdRegs->MMCRSP0 & 0x00FE) >> 1;
	return (OK);
}

int sdSetWriteCompleteHandler( void (*handler)(int status) )
{
	writeCompleteHandler = handler;
	return OK;
}

int sdSetPreEraseSize(Uint32 blocks)
{
	sdSendCmd(CMD_APP_CMD, sdCard.rca);
	sdSendCmd(0x0217, blocks);
	return OK;
}

int sdClose()
{
	//sdSendTransmissionStopCommand();
	//sdSendCmd(CMD_GO_IDLE, 0);
	return OK;//
}

int sdIdle()
{
	//sdSendTransmissionStopCommand();
	//sdSendCmd(CMD_GO_IDLE, 0);
	sdSendCmd(CMD_GO_IDLE, 0);
	return OK;//
}




int sdInit(int divider)
{
	static CSL_DMA_ChannelObj    dmaWrChanObj;
	static CSL_DMA_ChannelObj    dmaRdChanObj;
	
	Uint16 retries;
	Uint32 resp;
	CSL_Status	status;

	sdRegs = (SdRegsOvly)0x3A00; //Using SD Controller 0
	writeCompleteHandler = NULL;

	// Assign values for DMA config structures 
	dmaWriteCfg.pingPongMode = CSL_DMA_PING_PONG_DISABLE;
	dmaWriteCfg.autoMode     = CSL_DMA_AUTORELOAD_DISABLE;
	dmaWriteCfg.burstLen     = CSL_DMA_TXBURST_8WORD;
	dmaWriteCfg.chanDir      = CSL_DMA_WRITE;
	dmaWriteCfg.dmaInt       = CSL_DMA_INTERRUPT_DISABLE;
	dmaWriteCfg.trfType      = CSL_DMA_TRANSFER_IO_MEMORY;
	dmaWriteCfg.trigger      = CSL_DMA_EVENT_TRIGGER;
	dmaWriteCfg.srcAddr      = (Uint32)NULL;
	dmaWriteCfg.dataLen      = 0;
	dmaWriteCfg.destAddr     = (Uint32)&sdRegs->MMCDXR1;

	dmaReadCfg.pingPongMode = CSL_DMA_PING_PONG_DISABLE;
	dmaReadCfg.autoMode     = CSL_DMA_AUTORELOAD_DISABLE;
	dmaReadCfg.burstLen     = CSL_DMA_TXBURST_8WORD;
	dmaReadCfg.chanDir      = CSL_DMA_READ;
	dmaReadCfg.dmaInt       = CSL_DMA_INTERRUPT_DISABLE;
	dmaReadCfg.trfType      = CSL_DMA_TRANSFER_IO_MEMORY;
	dmaReadCfg.trigger      = CSL_DMA_EVENT_TRIGGER;
	dmaReadCfg.destAddr     = (Uint32)NULL;
	dmaReadCfg.dataLen      = 0;
	dmaReadCfg.srcAddr      = (Uint32)&sdRegs->MMCDRR1;

	dmaWriteCfg.dmaEvt = CSL_DMA_EVT_MMC_SD0_TX;
	dmaReadCfg.dmaEvt  = CSL_DMA_EVT_MMC_SD0_RX;
	
	//Configure interrupt mask
	sdRegs->MMCIM = ( MMCIM_ECRCWR | MMCIM_ETRNDNE | MMCIM_ETOUTRS );
	//0x00e9;
	
	// Open Dma channel for MMCSD write
	dmaWrObj = DMA_open(SDCARD_DMA_WRITE_CHANNEL, &dmaWrChanObj, &status);
    if(status != CSL_SOK) return(FAIL);
	DMA_reset(dmaWrObj);

	// Open Dma channel for MMCSD read
	dmaRdObj = DMA_open(SDCARD_DMA_READ_CHANNEL, &dmaRdChanObj, &status);
    if(status != CSL_SOK) return(FAIL);
	DMA_reset(dmaRdObj);
	
	// Configure time-out registers 
    sdRegs->MMCTOR = 0xffff; //set to maximum
    sdRegs->MMCTOD = 0xffff; //set to maximum;

	// Place the MMCSD controller in RESET state 
	sdRegs->MMCCTL |= 0x0003;
	// Configure the clock 
	sdRegs->MMCCLK = 128; //Divide by 128
	// Take the MMCSD controller out of RESET state 
	sdRegs->MMCCTL &= ~0x0003;
	// Send CMD0 to the card - Go Idle
	if( sdSendCmd(CMD_GO_IDLE, 0) != OK) return(FAIL);
	// Send CMD8 to the card - Send IF Cond
	if( sdSendCmd(CMD_SEND_IF_COND, 0x01AA) != OK) {
		if( sdSendCmd(CMD_SEND_IF_COND, 0x01AA) != OK) { //try again
			if( sdSendCmd(CMD_SEND_IF_COND, 0x01AA) != OK) { //try again
				return(FAIL);
			}
		}
	}

	// Send CMD55 & CMD41 to the card and wait until ready
	retries = 0;
	do {
		if( sdSendCmd(CMD_APP_CMD, 0) != OK) return FAIL;// CMD55 - APP CMD
		if( sdSendCmd(CMD_SEND_OP_COND, 0x40ff8000) != OK) return FAIL; //CMD41 - SD SEND OP COND
		resp = sdRegs->MMCRSP7; 
		resp = (resp << 16) | sdRegs->MMCRSP6;
		if( retries++ > 2000) return FAIL;
	} while (!(resp & 0x80000000));

	//Read CID data 
	if( sdGetCID() != OK ) return FAIL;

	//Request RCA from card
	if( sdGetRca( &sdCard.rca ) != OK) return FAIL;

	// Read the Card Specific Data
	if(sdGetCardCsd(sdCard.rca) != OK) return FAIL;

    // Set bus width 
	if( sdSendCmd(0x0237, (Uint32)sdCard.rca << 16) != OK) return(FAIL); // CMD55
	if( sdSendCmd(0x0206, 0x0002) != OK) return(FAIL); // ACMD 6 
    sdRegs->MMCCTL |= 0x0004;	// switch to using 4 data lines 

	sdRegs->MMCCLK = divider; //set clk to max
	sdRegs->MMCCTL  |= 0x0600; //big endian

	irqDisable(PROG0_EVENT);
	irqPlug(PROG0_EVENT, &sdOnWriteComplete);

	return OK;
}













