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
#include "audioTest.h"
#include "dma.h"
#include "ioExpander.h"
#include "d3std.h"
#include "math.h"
#include "hid.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAX_CHANNEL_COUNT 4
#define TX_AUDIO_LENGTH 2304
#define TRIG_PAGES 4 //min of 2 for ping pong
#define AUDIO_PAGE_LENGTH 256 //to math usb transfer
#define AUDIO_BUFFLEN (AUDIO_PAGE_LENGTH * TRIG_PAGES)

enum AUDIO_IO_BITS
{
	IO_BIT_AUDIO_ON,
	IO_BIT_GAIN,
	IO_BIT_CAL_CHAN,
	IO_BIT_HPASS,
	IO_BIT_MUTE,
	IO_BIT_CAL_ON,
	IO_BIT_ID1,
	IO_BIT_ID2
};


extern Uint32 sysclk;


CSL_I2sHandle i2sHandle;
CSL_DMA_Handle i2sDma;
CSL_DMA_ChannelObj i2sDmaChan;

CSL_I2sHandle i2sHandleDac;
CSL_DMA_Handle txDma;
CSL_DMA_ChannelObj txDmaChan;

I2S_Config i2scfg;

#pragma	DATA_SECTION(audioTestBuf,".audioBuf")
#pragma DATA_ALIGN(audioTestBuf, 8);
Uint16 audioTestBuf[AUDIO_BUFFLEN];


#pragma	DATA_SECTION(txAudioBuf,".audioBuf")
#pragma DATA_ALIGN(txAudioBuf, 8);
Uint16 txAudioBuf[TX_AUDIO_LENGTH * 2];

Uint16   l_rnd =  21845;  /* random seed */

Uint16 audioRegVal;
Uint16 audioClkDivider;
Fxn audioHandler;

Uint32 calTonef;

Uint16 pingPong;
Uint16 newData;
int oneShot = 0;
int trigPage = 0;

Uint16 activeChannel = 0;

void dmaIsr(int c)
{
    //pingPong = i2sDma->dmaRegs->DMACH0TCR2 & 0x0002 ?   AUDIO_BUFFLEN : 0;
    pingPong = i2sDma->dmaRegs->DMACH0TCR2 & 0x0002 ?   AUDIO_PAGE_LENGTH : 0;
    newData = 1;
}

void audioTestReadNextBuf(Uint16 **pReadBuffer)
{
	if(!oneShot)  {
		while(!newData) {};
		newData = 0;
		*pReadBuffer = &audioTestBuf[pingPong];
	}
	else {
		*pReadBuffer = &audioTestBuf[trigPage*AUDIO_PAGE_LENGTH];
		if(++trigPage == TRIG_PAGES) trigPage = 0;
	}
}

int audioTestRegSetBit(int bit, int on)
{
	if(on) {
		audioRegVal |= 1 << bit;
		return ioeWrite(AUDIO_IO_EXPANDER_DEVICE_ADDRESS, audioRegVal);
	}
	else {
		audioRegVal &= ~(1 << bit);
		return ioeWrite(AUDIO_IO_EXPANDER_DEVICE_ADDRESS, audioRegVal);
	}
}

int	audioTestSetMute(int on)
{
 	return audioTestRegSetBit(IO_BIT_MUTE, on);
}

int	audioTestEnableCalDAC(int on)
{
 	return audioTestRegSetBit(IO_BIT_CAL_ON, on);
}

int	audioTestSetGain(int on)
{
 	return audioTestRegSetBit(IO_BIT_GAIN, on) ;
}

int	audioTestSetPower(int on)
{
 	return audioTestRegSetBit(IO_BIT_AUDIO_ON, on) ;
}

int	audioTestSetHighPass(int on)
{
	return audioTestRegSetBit(IO_BIT_HPASS, on);
}

void audioTestEnable(int enable)
{
	audioTestSetPower(enable);

	if(enable) {
		I2S_transEnable(i2sHandle, 1);
		DMA_start(i2sDma);
    }
    else {
    	I2S_transEnable(i2sHandle, 0);
        DMA_stop(i2sDma);
    }
}

void audioTestCalDacEnable(int enable)
{
    if (enable) {
    	audioTestEnableCalDAC(1);
        DMA_start(txDma);
    }
    else {
    	audioTestEnableCalDAC(0);
        DMA_stop(txDma);
    }
}

Uint16 random(void) {


	l_rnd = (l_rnd * 31821U) + 13849U;
    return l_rnd;
}

/*
void audioTestSetCalTone()
{
    Uint32 i;
	Uint32 audioClk = sysclk / 32 / pow(2,(audioClkDivider+1));

	if(calTonef == 0)
	{
		for (i = 0; i < TX_AUDIO_LENGTH; i++) {
			txAudioBuf[i] = (Uint16)(random() / 8.0);
			if(hid > ST200) txAudioBuf[i] >>= 1;
		}
	}
	else {
		float v = (float)calTonef * 2.0 * M_PI / (float)audioClk;
		for (i = 0; i < TX_AUDIO_LENGTH; i++) {
			txAudioBuf[i] = ((Uint16) round(( sin( v*i ) * 4096) + 16384));
			if(hid > ST200) txAudioBuf[i] >>= 1;
		}
	}
}
*/

extern Uint16 sine (int *x, int *r, Uint16 nx);

void audioTestSetCalTone()
{
    Uint32 i;
    Uint16 v;
    int x;
    int * txAudioBufp = (int *)txAudioBuf;
	Uint32 audioClk = sysclk / 32 / pow(2,(audioClkDivider+1));

	if(calTonef == 0)
	{
		for (i = 0; i < TX_AUDIO_LENGTH; i++) {
			txAudioBuf[i] = (Uint16)(random() / 8.0);
			if(hid > ST200) txAudioBuf[i] >>= 1;
		}
	}
	else {
		//v = (float)calTonef * 2.0 * 32768.0 / (float)audioClk;
		v = (double)calTonef * 65536 / audioClk;
		x = 0;
		for (i = 0; i < TX_AUDIO_LENGTH; i++) {
			txAudioBufp[i] = x;
			x += v;
		}

		sine(txAudioBufp,txAudioBufp, TX_AUDIO_LENGTH);

		for (i = 0; i < TX_AUDIO_LENGTH; i++) {
			txAudioBuf[i] = (txAudioBufp[i] >> 3) + 16384;
			if(hid > ST200) txAudioBuf[i] >>= 1;
		}
	}
}


Uint16 audioHardwareId = 0xffff;
Uint16 audioTestGetAudioHardwareId()
{
	Uint16 portVal;
	if(audioHardwareId == 0xffff) {
		if( ioeRead(AUDIO_IO_EXPANDER_DEVICE_ADDRESS, &portVal) == TRUE ) {
			audioHardwareId = (portVal & 0xC0) >> 6;
		} else audioHardwareId = 0xffff -1;
	}
	return audioHardwareId;
}

void audioInitI2s()
{
	I2S_close(i2sHandle);
    i2sHandle = I2S_open(I2S_INSTANCE1, DMA_POLLED, I2S_CHAN_MONO);
    I2S_reset(i2sHandle);
    i2scfg.dataType = I2S_STEREO_ENABLE; //I2S_MONO_ENABLE;
    i2scfg.loopBackMode = I2S_LOOPBACK_DISABLE;
    i2scfg.fsPol = I2S_FSPOL_LOW; /**< Left Channel transmission polarity */
    i2scfg.clkPol = hid == ST200 ? I2S_RISING_EDGE : I2S_FALLING_EDGE; /**< Clock polarity					*/
    i2scfg.datadelay = I2S_DATADELAY_ONEBIT; /**< I2S data delay                 */
    i2scfg.datapack = I2S_DATAPACK_ENABLE; //I2S_DATAPACK_DISABLE;     	/**< Data pack bit                  */
    i2scfg.signext = I2S_SIGNEXT_DISABLE; /**< sign of the data to be tx/rx   */
    i2scfg.dataFormat = I2S_DATAFORMAT_LJUST; //I2S_DATAFORMAT_DSP;		/**< Data format                    */
    i2scfg.FError = I2S_FSERROR_DISABLE; /**< Frame-sync error reporting enable/disable	*/
    i2scfg.OuError = I2S_OUERROR_DISABLE; /**< Overrun or under-run error reporting enable/disable	*/
    i2scfg.wordLen = I2S_WORDLEN_16; /**< Number of bits in a word     	*/
    i2scfg.fsDiv = I2S_FSDIV32; /**< FSDIV value					*/
    i2scfg.clkDiv = (I2S_Clkdiv)audioClkDivider;  /**< Clock divisor					*/
    i2scfg.i2sMode = I2S_MASTER; /**< I2S device operation mode      */
    I2S_setup(i2sHandle, &i2scfg);
    i2sHandle->hwRegs->I2SSCTRL &= ~0x0001; //I2S_DATAFORMAT_LJUST
	I2S_transEnable(i2sHandle, 1);

	i2sHandle = I2S_open(I2S_INSTANCE2, DMA_POLLED, I2S_CHAN_MONO);
    I2S_reset(i2sHandle);
    i2scfg.dataType = I2S_STEREO_ENABLE; //I2S_MONO_ENABLE;
    i2scfg.loopBackMode = I2S_LOOPBACK_DISABLE;
    i2scfg.fsPol = I2S_FSPOL_LOW; /**< Left Channel transmission polarity */
    i2scfg.clkPol = I2S_FALLING_EDGE; /**< Clock polarity					*/
    i2scfg.datadelay = I2S_DATADELAY_ONEBIT; /**< I2S data delay                 */
    i2scfg.datapack = I2S_DATAPACK_ENABLE; //I2S_DATAPACK_DISABLE;     	/**< Data pack bit                  */
    i2scfg.signext = I2S_SIGNEXT_DISABLE; /**< sign of the data to be tx/rx   */
    i2scfg.dataFormat = I2S_DATAFORMAT_LJUST; //I2S_DATAFORMAT_DSP;		/**< Data format                    */
    i2scfg.FError = I2S_FSERROR_DISABLE; /**< Frame-sync error reporting enable/disable	*/
    i2scfg.OuError = I2S_OUERROR_DISABLE; /**< Overrun or under-run error reporting enable/disable	*/
    i2scfg.wordLen = I2S_WORDLEN_16; /**< Number of bits in a word     	*/
    i2scfg.fsDiv = I2S_FSDIV32; /**< FSDIV value					*/
    i2scfg.clkDiv = (I2S_Clkdiv)audioClkDivider;  /**< Clock divisor					*/
    i2scfg.i2sMode = I2S_MASTER; /**< I2S device operation mode      */
    I2S_setup(i2sHandle, &i2scfg);
    i2sHandle->hwRegs->I2SSCTRL &= ~0x0001; //I2S_DATAFORMAT_LJUST
	I2S_transEnable(i2sHandle, 1);

	audioTestSetCalTone();
}

void audioSetCalTonef(Uint32 calTonefrequency)
{
	calTonef = calTonefrequency;
    audioTestSetCalTone();
}

void audioTestSetSampleRate(Uint16 divider)
{
	I2S_transEnable(i2sHandle, 0);
	audioClkDivider = divider;
	audioInitI2s();
	I2S_transEnable(i2sHandle, 1);
}


void audioTestinitDma(int oneShotEnable)
{
    CSL_Status status;
    CSL_DMA_Config cfg;
    CSL_DMA_Config cfg2;

   	DMA_stop(i2sDma);
    i2sDma = DMA_open(activeChannel < 2 ? CSL_DMA_CHAN12 : CSL_DMA_CHAN4, &i2sDmaChan, &status);
    DMA_reset(i2sDma);

    cfg.dmaInt = CSL_DMA_INTERRUPT_ENABLE;
    cfg.autoMode = oneShotEnable ? CSL_DMA_AUTORELOAD_DISABLE : CSL_DMA_AUTORELOAD_ENABLE;
    cfg.burstLen = CSL_DMA_TXBURST_1WORD;
    cfg.chanDir = CSL_DMA_READ;
    cfg.dataLen = oneShotEnable ? AUDIO_PAGE_LENGTH * 2 * TRIG_PAGES : AUDIO_PAGE_LENGTH * 2 * 2; //in bytes
    cfg.dmaEvt = activeChannel < 2 ? CSL_DMA_EVT_I2S1_RX : CSL_DMA_EVT_I2S2_RX;
    cfg.pingPongMode = oneShotEnable ? CSL_DMA_PING_PONG_DISABLE : CSL_DMA_PING_PONG_ENABLE;

    switch(activeChannel) {
    	case 0:  cfg.srcAddr = (Uint32) (0x2928); break;
    	case 1:  cfg.srcAddr = (Uint32) (0x292C); break;
    	case 2:  cfg.srcAddr = (Uint32) (0x2A28); break;
    	case 3:  cfg.srcAddr = (Uint32) (0x2A2C); break;
    }

    cfg.trfType = CSL_DMA_TRANSFER_IO_MEMORY;
    cfg.trigger = CSL_DMA_EVENT_TRIGGER;
    cfg.destAddr = (Uint32) audioTestBuf;
    DMA_config(i2sDma, &cfg);

   	DMA_stop(txDma);
	txDma = DMA_open(AUDIO_TX_DMA_CHANNEL, &txDmaChan, &status);
	DMA_reset(txDma);
    cfg2.dmaEvt = CSL_DMA_EVT_I2S1_TX;
    cfg2.dmaInt = CSL_DMA_INTERRUPT_DISABLE;
    cfg2.autoMode = CSL_DMA_AUTORELOAD_ENABLE;
    cfg2.burstLen = CSL_DMA_TXBURST_1WORD;
    cfg2.chanDir = CSL_DMA_WRITE;
    cfg2.dataLen = TX_AUDIO_LENGTH * 2; //in bytes
    cfg2.pingPongMode = CSL_DMA_PING_PONG_DISABLE;
    cfg2.trfType = CSL_DMA_TRANSFER_IO_MEMORY;
    cfg2.trigger = CSL_DMA_EVENT_TRIGGER;
    cfg2.srcAddr = (Uint32) txAudioBuf;
    cfg2.destAddr = (Uint32) (0x2908);
    DMA_config(txDma, &cfg2);

    DMA_start(i2sDma);
    DMA_start(txDma);
}

void audioTestTrigger()
{
	trigPage = 0;
	audioTestinitDma(1);
}

void audioTestEnableOneShot(int enable)
{
	oneShot = enable;
}

void audioSetActiveChannel(Uint16 channel)
{
	if(channel < (MAX_CHANNEL_COUNT)) {
		activeChannel = channel;
		audioTestinitDma(0);
	}
}

void audioTestInit()
{
    audioClkDivider = (Uint16)I2S_CLKDIV8;
    calTonef = 1000;

    audioInitI2s();

    audioRegVal = 0;

    dmaInterruptHandlerRegister(CSL_DMA_CHAN12, dmaIsr);
    dmaInterruptHandlerRegister(CSL_DMA_CHAN4, dmaIsr);

    audioSetActiveChannel(0);
}





