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
#include "hid.h"

Uint32 framesDropped;

#define I2S_PORTS 2

CSL_DMA_Handle i2sDma[MAX_CHANNELS];
CSL_DMA_ChannelObj i2sDmaChan[MAX_CHANNELS];

CSL_DMA_Handle memcpyDma[MAX_CHANNELS];
CSL_DMA_ChannelObj memcpyDmaChan[MAX_CHANNELS];

CSL_I2sHandle i2sPort[I2S_PORTS] = {NULL, NULL};

int channelMap[MAX_CHANNELS];

#pragma	DATA_SECTION(audioBuf,".pingpong")
#pragma DATA_ALIGN(audioBuf, 8);
Uint16 audioBuf[AUDIO_BUFFLEN * 2];

static Ptr AUDIO_H[MAX_CHANNELS];
extern Uint32 sysclk;

int channels = 0;
int nActiveChannels = 0;
int transferSize;

// Bands structure collects information needed to configure a serial port for
// a particular band
typedef struct
{
    int div;        // divider
    I2S_Fsdiv FPER; // number of serial clocks in a sampling (frame sync) period
    uns FWID; // maximum number of bits that can be read from the ADC per sample
    I2S_Clkdiv DIV; // divider to use for serial clock with respect to a cpu clock of DIVCLKREF MHz
} AUDIO_Band;

// The DIV component of the band structure is referred to a SYSCLK of 32768000 * 2 MHz
#define     DIVCLKREF       (32768000) //TODO - support other system clocks
// predefined audio bands - the indices correspond to the band definitions SLAVE,HF,MF, etc
static AUDIO_Band AudioBands[] = {
		{ 64, I2S_FSDIV32, 16, I2S_CLKDIV2 },       // HF1 same as HF2 TODO fix
        { 128, I2S_FSDIV32, 16, I2S_CLKDIV4 },        // HF2
        { 256, I2S_FSDIV32, 16, I2S_CLKDIV8 },        // MF1
        { 512, I2S_FSDIV32, 16, I2S_CLKDIV16 },        // MF2
        { 1024, I2S_FSDIV32, 16, I2S_CLKDIV32 },        // LF1
        { 2048, I2S_FSDIV32, 16, I2S_CLKDIV64 } };      // LF2

#define AUDIO_NBANDS    (sizeof(AudioBands)/sizeof(AUDIO_Band))

Uint32 I2S_ReceiveAddresses[] = {0x2928, 0x292C, 0x2A28, 0x2A2C};
CSL_DMAChanNum dmaI2SChannels[] = {CSL_DMA_CHAN12, CSL_DMA_CHAN13, CSL_DMA_CHAN4, CSL_DMA_CHAN5};
CSL_DMAChanNum dmaMCpyChannels[] = {CSL_DMA_CHAN8, CSL_DMA_CHAN9, CSL_DMA_CHAN10, CSL_DMA_CHAN11};
I2S_Instance i2sInstances[] = {I2S_INSTANCE1, I2S_INSTANCE2};

Uint32 audioBlocksProcessed;

extern void audio_run(Ptr ah);
extern void audio_reperr(int chan, int etype, Ptr ah);

long AUDIO_getfs(uns band)
{
	if (band >= AUDIO_NBANDS)
        return (-1);

    return (sysclk / AudioBands[band].div);
}

//Called by DMA ISR on block read completion. Calls audio_run which in turn calls
//audio_nextbuff which sets up the buffer for the next block
void dmaIsr(int dmaChannel)
{
    audioBlocksProcessed++;
    audio_run(AUDIO_H[0]);
}

void dmaStart(int channel, Uint32 src_address, Uint32 dest_address)
{
    src_address  = (src_address << CSL_DMA_ADDR_SHIFT) + CSL_DMA_DARAM_ADDR_OFFSET; //pingpong in daram
    dest_address  = (dest_address << CSL_DMA_ADDR_SHIFT) + CSL_DMA_SARAM_ADDR_OFFSET;  //dmem buffers in saram


    //CSL_DMA_Config is too slow so we'll do it ourselves
	switch(memcpyDma[channel]->chanNum % 4)
    {
    	case(0):
    		memcpyDma[channel]->dmaRegs->DMACH0DSAL = dest_address & 0xFFFF;
    		memcpyDma[channel]->dmaRegs->DMACH0DSAU = (Uint16)(dest_address >> 16);
    		memcpyDma[channel]->dmaRegs->DMACH0SSAL = src_address & 0xFFFF;
    		memcpyDma[channel]->dmaRegs->DMACH0SSAU =  (Uint16)(src_address >> 16);
    		memcpyDma[channel]->dmaRegs->DMACH0TCR1 = transferSize*2;
            CSL_FINS(memcpyDma[channel]->dmaRegs->DMACH0TCR2, DMA_DMACH0TCR2_EN, CSL_DMA_CHANNEL_ENABLE);
            break;
		case(1):
			memcpyDma[channel]->dmaRegs->DMACH1DSAL = dest_address & 0xFFFF;
			memcpyDma[channel]->dmaRegs->DMACH1DSAU = (Uint16)(dest_address >> 16);
			memcpyDma[channel]->dmaRegs->DMACH1SSAL = src_address & 0xFFFF;
			memcpyDma[channel]->dmaRegs->DMACH1SSAU =  (Uint16)(src_address >> 16);
			memcpyDma[channel]->dmaRegs->DMACH1TCR1 = transferSize*2;
            CSL_FINS(memcpyDma[channel]->dmaRegs->DMACH1TCR2, DMA_DMACH1TCR2_EN, CSL_DMA_CHANNEL_ENABLE);
    		break;
    	case(2):
			memcpyDma[channel]->dmaRegs->DMACH2DSAL = dest_address & 0xFFFF;
			memcpyDma[channel]->dmaRegs->DMACH2DSAU = (Uint16)(dest_address >> 16);
			memcpyDma[channel]->dmaRegs->DMACH2SSAL = src_address & 0xFFFF;
			memcpyDma[channel]->dmaRegs->DMACH2SSAU =  (Uint16)(src_address >> 16);
			memcpyDma[channel]->dmaRegs->DMACH2TCR1 = transferSize*2;
            CSL_FINS(memcpyDma[channel]->dmaRegs->DMACH2TCR2, DMA_DMACH2TCR2_EN, CSL_DMA_CHANNEL_ENABLE);
    		break;
    	case(3):
			memcpyDma[channel]->dmaRegs->DMACH3DSAL = dest_address & 0xFFFF;
			memcpyDma[channel]->dmaRegs->DMACH3DSAU = (Uint16)(dest_address >> 16);
			memcpyDma[channel]->dmaRegs->DMACH3SSAL = src_address & 0xFFFF;
			memcpyDma[channel]->dmaRegs->DMACH3SSAU =  (Uint16)(src_address >> 16);
			memcpyDma[channel]->dmaRegs->DMACH3TCR1 = transferSize*2;
            CSL_FINS(memcpyDma[channel]->dmaRegs->DMACH3TCR2, DMA_DMACH3TCR2_EN, CSL_DMA_CHANNEL_ENABLE);
    		break;
    }
}

//Configures DMA context for the next buffer
void audio_nextbuff(Ptr buf, int bufLength)
{
	int c;
	Uint16 *s;
	Uint16 *d;
	Uint16 pingPong;
    pingPong = i2sDma[0]->dmaRegs->DMACH0TCR2 & 0x0002 ? transferSize : 0;

    /*
    if (i2sPort[channel/2]->hwRegs->I2SINTFL & 0x0001) {
        //over run detected
        audio_reperr(0, OVERRUN_ERR, AUDIO_H[channel]);
    }
    */

    for(c=0; c<nActiveChannels; c++) {
    	s = &audioBuf[(c * transferSize * 2) + pingPong];
    	d = buf;
    	d+= c * transferSize;
    	dmaStart(channelMap[c], (Uint32) s, (Uint32) d);
    }
}

//Initialization of I2S
int adcInit(int port, AUDIO_Band *b)
{
    I2S_Config cfg;

    i2sPort[port] = I2S_open(i2sInstances[port], DMA_POLLED, I2S_CHAN_MONO);

    if (i2sPort[port] == INV ) {
    	err(AUDIO_MOD, ADCPORTOPENFAIL);
        return FAIL;
    }

    cfg.dataType = I2S_STEREO_ENABLE; //I2S_MONO_ENABLE;
    cfg.loopBackMode = I2S_LOOPBACK_DISABLE;
    cfg.fsPol = I2S_FSPOL_LOW; /**< Left Channel transmission polarity */
    cfg.clkPol = hid == ST200 ? I2S_RISING_EDGE : I2S_FALLING_EDGE; // Clock polarity
    cfg.datadelay = I2S_DATADELAY_ONEBIT; /**< I2S data delay                 */
    cfg.datapack = I2S_DATAPACK_ENABLE; //I2S_DATAPACK_DISABLE;     	/**< Data pack bit                  */
    cfg.signext = I2S_SIGNEXT_DISABLE; /**< sign of the data to be tx/rx   */
    cfg.dataFormat = I2S_DATAFORMAT_LJUST; //I2S_DATAFORMAT_DSP;		/**< Data format                    */
    cfg.FError = I2S_FSERROR_DISABLE; /**< Frame-sync error reporting enable/disable	*/
    cfg.OuError = I2S_OUERROR_DISABLE; /**< Overrun or under-run error reporting enable/disable	*/
    cfg.wordLen = I2S_WORDLEN_16; /**< Number of bits in a word     	*/
    cfg.fsDiv = b->FPER; /**< FSDIV value					*/
    cfg.clkDiv = b->DIV; /**< Clock divisor					*/
    //cfg.i2sMode = port == 0 ? I2S_MASTER : I2S_SLAVE; /**< I2S device operation mode      */
    cfg.i2sMode = I2S_MASTER; /**< I2S device operation mode      */
    I2S_setup(i2sPort[port], &cfg);
    i2sPort[port]->hwRegs->I2SSCTRL &= ~0x0001; //I2S_DATAFORMAT_LJUST
    //I2S_transEnable(i2sPort[port], 1);
    return OK;
}

//Initialization of DMA
int dmaInit(int channel, int chanIndex)
{
	CSL_Status status;
    CSL_DMA_Config cfg;

	//first DMA is used to move date from I2s to pingpong buffer
    i2sDma[channel] = DMA_open(dmaI2SChannels[channel], &i2sDmaChan[channel], &status);
	if (status != CSL_SOK) {
		err(AUDIO_MOD, DMAFAILOPEN);
		return FAIL;
	}
	DMA_reset(i2sDma[channel]);
	cfg.dmaInt = CSL_DMA_INTERRUPT_ENABLE;
	cfg.autoMode = CSL_DMA_AUTORELOAD_ENABLE;
	cfg.burstLen = CSL_DMA_TXBURST_1WORD;
	cfg.chanDir = CSL_DMA_READ;
	cfg.dataLen = (transferSize) * 2 * 2; //in bytes ping + pong
	cfg.dmaEvt = channel < 2 ? CSL_DMA_EVT_I2S1_RX : CSL_DMA_EVT_I2S2_RX;
	cfg.pingPongMode = CSL_DMA_PING_PONG_ENABLE;
	cfg.srcAddr = I2S_ReceiveAddresses[channel];
	cfg.trfType = CSL_DMA_TRANSFER_IO_MEMORY;
	cfg.trigger = CSL_DMA_EVENT_TRIGGER;
	cfg.destAddr = (Uint32) &audioBuf[chanIndex * 2 * transferSize];
	DMA_config(i2sDma[channel], &cfg);
	if(chanIndex == 0) dmaInterruptHandlerRegister(dmaI2SChannels[channel], dmaIsr);

	//Second DMA is used to move data from pingpong buffer to dmem buffers
	memcpyDma[channel] = DMA_open(dmaMCpyChannels[channel], &memcpyDmaChan[channel], &status);

	DMA_reset(memcpyDma[channel]);
    cfg.dmaInt = CSL_DMA_INTERRUPT_DISABLE;
    cfg.autoMode = CSL_DMA_AUTORELOAD_DISABLE;
    cfg.burstLen = CSL_DMA_TXBURST_1WORD;
    cfg.chanDir = CSL_DMA_READ;
    cfg.dataLen = transferSize * 2; //in bytes
    cfg.pingPongMode = CSL_DMA_PING_PONG_DISABLE;
    cfg.trfType = CSL_DMA_TRANSFER_MEMORY;
    cfg.trigger = CSL_DMA_SOFTWARE_TRIGGER;
    cfg.srcAddr = 0;
    cfg.destAddr = 0;
    DMA_config(memcpyDma[channel], &cfg);

	if (status != CSL_SOK) {
		err(AUDIO_MOD, DMAFAILOPEN);
		return FAIL;
	}
	return OK;
}

int audio_openchans(int channels, int nchan, uns band, Ptr streamHandle)
{
	int port, chan = 0;
	nActiveChannels = 0;

	transferSize = AUDIO_BUFFLEN / nchan;
	transferSize &=  ~0x0001; //must be even

	for(chan=0; chan < MAX_CHANNELS; chan++)
	{
		if(channels & (0x0001 << chan)) {
			channelMap[nActiveChannels] = chan;
			port = chan/2;
		    // check that the band is valid
		    if (band >= AUDIO_NBANDS) {
		        err(AUDIO_MOD, BANDINVALID);
		        return (FAIL);
		    }

		    AUDIO_H[chan] = streamHandle;
		    if(i2sPort[port] == NULL ) {
		    	if( adcInit(port, &AudioBands[band]) != OK) {
		            return (FAIL);
		    	}
		    }
		    audiocreginit(chan);
		    dmaInit(chan, nActiveChannels);
			nActiveChannels ++;
		}
		else i2sDma[chan] = NULL;
	}
	if(nActiveChannels != nchan) return FAIL;
	return OK;
}

void audio_close()
{
	int i;
	for(i=0; i<MAX_CHANNELS; i++){
		if(i2sDma[i] != NULL ) DMA_stop(i2sDma[i]);
	}
}

void audio_start()
{
	int i;
	for(i=0; i<MAX_CHANNELS; i++){
		if(i2sDma[i] != NULL ) DMA_start(i2sDma[i]);
	}

	if(i2sPort[0] != NULL) I2S_transEnable(i2sPort[0], 1);
    if(i2sPort[1] != NULL) I2S_transEnable(i2sPort[1], 1);
}

void audio_stop()
{
	int i;
    if(i2sPort[0] != NULL) I2S_transEnable(i2sPort[0], 0);
    if(i2sPort[1] != NULL) I2S_transEnable(i2sPort[1], 0);

    for(i=0; i<MAX_CHANNELS; i++){
		if(i2sDma[i] != NULL ) DMA_stop(i2sDma[i]);
	}
}

void audio_init()
{
    audioBlocksProcessed = 0;
    framesDropped = 0;
    i2sPort[0] = NULL;
    i2sPort[1] = NULL;
}
