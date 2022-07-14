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


#include "csl_types.h"
#include "csl_i2s.h"
#include "csl_dma.h"
#include "csl_intc.h"
#include "assert.h"
#include "d3defs.h"
#include "d3std.h"
#include "protect.h"
#include "board.h"
#include "devdep.h"
#include "misc.h"
#include "modnumbers.h"
#include "error.h"
#include "dmem.h"
#include "audio.h"
#include "audio_if.h"
#include "dma.h"
#include "timr.h"
#include "fs.h"
#include "sd.h"
#include "math.h"
#include "crc.h"
#include "hid.h"


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define TX_AUDIO_LENGTH 288 // = 576 words for ping + pong

#pragma	DATA_SECTION(txAudioBuf,".audout")
#pragma DATA_ALIGN(txAudioBuf, 8);
Uint16 txAudioBuf[TX_AUDIO_LENGTH * 2];

CSL_DMA_Handle txDma;
CSL_DMA_ChannelObj txDmaChan;

Uint32 audioOutfs;
Uint16 txEnable = 0;
Uint32 txFirstPage;
Uint32 txLastPage;
Uint32 txReadPageAddr;
Uint32 calFrequency;


int txBusy = 0;
int audioOutLoadFromSD(Ptr s, Ptr d)
{
	txBusy = 1;
	Uint16 pingPong = txDma->dmaRegs->DMACH2TCR2 & 0x0002 ? TX_AUDIO_LENGTH : 0;
	sdRead(txReadPageAddr, TX_AUDIO_LENGTH * 2, &txAudioBuf[pingPong], 1);



	if(txReadPageAddr < txLastPage) {
		txReadPageAddr++;
	}
	else {
		txReadPageAddr=txFirstPage;
	}
	txBusy = 0;
	return(OK);
}

void audioOutdmaIsr(int i)
{
    if(!txBusy && txEnable) {
    	JOB_postone((JOB_Fxn)audioOutLoadFromSD, NULL, 1, NULL);
    }
}

#define PLAY_BACK_MAJIC_NUMBER 0x8888
typedef struct {
	Uint32 majic;
	Uint32 fileLength;
	Uint16 crc;
} playBackHeader;



/*
 * audioOutStartTxFromSD
 *
 * Plays a file stored on the SD card. First pages of file
 * contains a header indicating length of file. Subsequent pages
 * contain raw audio data. Sample rate is shared with record
 * function. File size should be multiple of FS_PAGE_SIZE (512) bytes.
 *
 */

int audioOutStartTxFromSD(Uint16 fileNo)
{
	Uint32 infoPage;
    CSL_DMA_Config cfg;
    playBackHeader *ph;

	DMA_stop(txDma);

	infoPage = (Uint32)FS_BLOCKS_PER_FILE * FS_PAGES_PER_BLOCK * fileNo;
	sdRead(infoPage, TX_AUDIO_LENGTH * 2, &txAudioBuf[0], 0);
	ph = (playBackHeader *)txAudioBuf;

	if(crc(ph, sizeof(playBackHeader) == 0)) {
		if(ph->majic == PLAY_BACK_MAJIC_NUMBER) {
			AUDIO_mute(1);
			enableCalDAC(1);

			txFirstPage = infoPage + 1;
			txLastPage = txFirstPage + ceil(ph->fileLength/FS_PAGE_SIZE);
			txReadPageAddr = txFirstPage;
			txEnable = 1;

			DMA_reset(txDma);
			cfg.dmaEvt = CSL_DMA_EVT_I2S1_TX;
			cfg.dmaInt = CSL_DMA_INTERRUPT_ENABLE;
			cfg.autoMode = CSL_DMA_AUTORELOAD_ENABLE;
			cfg.burstLen = CSL_DMA_TXBURST_1WORD;
			cfg.chanDir = CSL_DMA_WRITE;
			cfg.dataLen = TX_AUDIO_LENGTH * 2 * 2; //in bytes
			cfg.pingPongMode = CSL_DMA_PING_PONG_ENABLE;
			cfg.trfType = CSL_DMA_TRANSFER_IO_MEMORY;
			cfg.trigger = CSL_DMA_EVENT_TRIGGER;
			cfg.srcAddr = (Uint32) txAudioBuf;
			cfg.destAddr = (Uint32) (0x2908);

			DMA_config(txDma, &cfg);
			DMA_start(txDma);
			return OK;
		}
	}
	return FAIL;
}


void audioOutStartTxFromSDold(Uint16 fileNo)
{
    CSL_DMA_Config cfg;

    AUDIO_mute(1);
	enableCalDAC(1);

	txFirstPage = (Uint32)FS_BLOCKS_PER_FILE * FS_PAGES_PER_BLOCK * fileNo;
	txLastPage = txFirstPage + ((Uint32) FS_PAGES_PER_BLOCK * FS_BLOCKS_PER_FILE);
	txReadPageAddr = txFirstPage;
	txEnable = 1;

    //DMA_stop(txDma);
    DMA_reset(txDma);
    cfg.dmaEvt = CSL_DMA_EVT_I2S1_TX;
    cfg.dmaInt = CSL_DMA_INTERRUPT_ENABLE;
    cfg.autoMode = CSL_DMA_AUTORELOAD_ENABLE;
    cfg.burstLen = CSL_DMA_TXBURST_1WORD;
    cfg.chanDir = CSL_DMA_WRITE;
    cfg.dataLen = TX_AUDIO_LENGTH * 2 * 2; //in bytes
    cfg.pingPongMode = CSL_DMA_PING_PONG_ENABLE;
    cfg.trfType = CSL_DMA_TRANSFER_IO_MEMORY;
    cfg.trigger = CSL_DMA_EVENT_TRIGGER;
    cfg.srcAddr = (Uint32) txAudioBuf;
    cfg.destAddr = (Uint32) (0x2908);

    DMA_config(txDma, &cfg);
    DMA_start( txDma);
}


int audioOutconfigTxDma(Uint32 samples)
{
    CSL_DMA_Config cfg;
	txEnable = 0;
	DMA_reset(txDma);
    cfg.dmaEvt = CSL_DMA_EVT_I2S1_TX;
    cfg.dmaInt = CSL_DMA_INTERRUPT_DISABLE;
    cfg.autoMode = CSL_DMA_AUTORELOAD_ENABLE;
    cfg.burstLen = CSL_DMA_TXBURST_1WORD;
    cfg.chanDir = CSL_DMA_WRITE;
    cfg.dataLen = samples * 2; //in bytes
    cfg.pingPongMode = CSL_DMA_PING_PONG_DISABLE;
    cfg.trfType = CSL_DMA_TRANSFER_IO_MEMORY;
    cfg.trigger = CSL_DMA_EVENT_TRIGGER;
    cfg.srcAddr = (Uint32) txAudioBuf;
    cfg.destAddr = (Uint32) (0x2908);
    DMA_config(txDma, &cfg);
    return OK;
}

void audioOutEnableCal(int enable)
{
    if (enable) {
        enableCalDAC(1);
        DMA_start(txDma);
    }
    else {
        enableCalDAC(0);
        DMA_stop(txDma);
    }
}

/*
int audioOutsweep(double f1, double f2)
{
	double value;
	double T = TX_AUDIO_LENGTH/(double)audioOutfs;
	double K  = 2*M_PI*f1 * T / (log(f2/f1));
	double L = T/log(f2/f1);
	Uint16 t;
	double t2;

	for(t=0;t<TX_AUDIO_LENGTH;t++)
	{
		t2=(double)t/(double)audioOutfs;
		value = exp(t2/L) - 1;
		value = sin(K*value);
		txAudioBuf[t] = (value * 2048.0) + 3072.0;
	}
	return OK;
}
*/
extern Uint16 sine (int *x, int *r, Uint16 nx);


int audioOutsetCalTone(Uint32 frequency)
{
    Uint32 i;
    int v;
    int x;
    int samples;
    int * txAudioBufp = (int *)txAudioBuf;

	v = 2.0 * 32768.0 * frequency / (float)audioOutfs;
	//maximise number of full cycles that will fit in buffer
	samples = ((TX_AUDIO_LENGTH *2) / (audioOutfs / frequency)) * (audioOutfs / frequency);
	if(samples == 0) samples = TX_AUDIO_LENGTH * 2; //can't fit a full cycle, proceed anyway
	x = 0;

	for (i = 0; i < samples; i++) {
		txAudioBufp[i] = x;
		x += v;
	}

	sine(txAudioBufp,txAudioBufp, samples);

	for (i = 0; i < samples; i++) {
		txAudioBuf[i] = (txAudioBufp[i] >> 3) + 16384;
		if(hid > ST200) txAudioBuf[i] >>= 1;
	}

	calFrequency = frequency;
	return audioOutconfigTxDma(samples);
}

int audioOutInit(int band)
{
    CSL_Status status;
    audioOutfs = AUDIO_getfs(band);
    txDma = DMA_open(AUDIO_TX_DMA_CHANNEL, &txDmaChan, &status);
    if (status != CSL_SOK) {
        err(AUDIO_MOD, DMAFAILOPEN);
        return FAIL;
    }
    dmaInterruptHandlerRegister(AUDIO_TX_DMA_CHANNEL, audioOutdmaIsr);
    return OK;
}
