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

#include <stdio.h>
#include <tistdtypes.h>
#include "d3std.h"
#include "messageProcessor.h"
#include "commands.h"
#include "tick.h"
#include "crc.h"
#include "flash.h"
#include "usb.h"
#include "serialEeprom.h"
#include "msp.h"
#include "sysControl.h"
#include "fs.h"
#include "mspif.h"
#include "config.h"
#include "mspif.h"
#include "gpio.h"
#include "audioTest.h"
#include "sd.h"
#include "hid.h"
#include "uart.h"
#include "i2c.h"
#include "sd.h"
#include "ioExpander.h"
#include "csl_intc.h"
#include "memCardSelect.h"



#define OFFLOADER_SW_VER 139

/* File Revision history
 *
 * 108 - Fixed MSP Prog by moving tick processing to main line.
 * 130 - STV2 support.
 */


extern Uint32 flashWritePageAddress;
extern Uint16 messageBuf[];
extern MSP_Sensors sensorData;
extern Uint16 mainPressure;
extern Uint16 flashState;

typedef struct {
	Uint32 totalSectors;
	Uint32 sectorsUsed;
} TcardData;


int lastCommandStatus;

typedef struct {
	Uint32 resetRunTime;
	Uint32 totalRunTime;
	Uint32 unixTime;
	Uint16 deviceType;      /*******************************************************/
	Uint16 temperature;
	Uint16 audioHardwareId;
	Uint16 commsErrorLog;    /* These feilds are shared */
	Uint16 setupSaved;
	Uint16 nvmInitGood;
	Uint16 swVer;
	Uint16 lastCommandStatus;
	Uint16 battVoltage;
	Uint16 extBattVoltage;
} TstatusData;

typedef struct {
  TmessageHeader header;
  TstatusData statusData;
} TstatusPacket;

void getStatusData( TstatusData *status ) {
  	status->deviceType = (hid  << 8) + ( audioTestGetAudioHardwareId() & 0xff);
  	status->audioHardwareId = audioTestGetAudioHardwareId();
  	status->commsErrorLog = getCommsErrorLog();
  	status->setupSaved = 0;
  	status->resetRunTime = getTickCount() / 1000;
  	status->totalRunTime = 0;
  	status->nvmInitGood = 1;
  	status->swVer = OFFLOADER_SW_VER;
  	status->battVoltage =  sensorData.vb;
  	status->temperature = sensorData.tempr;
	status->unixTime = tickGetUnixTime();
	status->extBattVoltage = sensorData.extvb;
}

void sendStatusData( Uint16 status )
{
	TstatusPacket packet;
	packet.header.sop = SOP;
	packet.header.id = deviceID;
	packet.header.type = STATUS_READ;
	packet.header.dataLength = sizeof( packet.statusData ) * 2;
	getStatusData( &packet.statusData );
	packet.statusData.lastCommandStatus = status;
	packet.header.dataCRC = crc(&packet.statusData, sizeof(packet.statusData) );
	packet.header.headerCRC = crc(&packet.header, sizeof(packet.header) -  sizeof(packet.header.headerCRC));
	txBuffer(&packet, sizeof( packet ) );
}


void sendConfigData()
{
	TmessageHeader header;
	header.sop = SOP;
	header.id = deviceID;
	header.type = GET_CONFIG;
	header.dataLength = sizeof(TConfig)*2;
	header.dataCRC = crc(&systemConfig, sizeof(TConfig));
	header.headerCRC = crc(&header, sizeof(header) -  sizeof(header.headerCRC));
	txBuffer(&header, sizeof( header ) );
	txBuffer(&systemConfig, sizeof(TConfig));
}

void sendBusySignal( Uint16 percentComplete )
{
	TmessageHeader header;
	header.sop = SOP;
	header.id = deviceID;
	header.type = BUSY;
	header.dataLength = 2;
	header.dataCRC = crc(&percentComplete, 1);
	header.headerCRC = crc(&header, sizeof(header) -  sizeof(header.headerCRC));
	txBuffer(&header, sizeof( header ) );
	txBuffer(&percentComplete, 1 );
} 

void sendReply( Uint16 type, Uint16* buf, Uint16 count)
{
	TmessageHeader header;
	header.sop = SOP;
	header.id = deviceID;
	header.type = type;
	header.dataLength = count * 2; //in bytes
	header.dataCRC = crc(buf, count);
	header.headerCRC = crc(&header, sizeof(header) -  sizeof(header.headerCRC));
	txBuffer(&header, sizeof( header ) );
	txBuffer(buf, count );
} 

void processSerialEepromReadCommand(address, count)
{
	Uint16 buf[64];
	if(count>64) count = 64;
	serialEepromRead(address, buf, count);
	sendReply(0x0000, buf, 64);
}

void processGetFlashCardInfoCommand()
{
	Uint16 buf[6];
	int len = sdGetCardInfo(buf, 6);
	sendReply(FLASH_GET_CARD_INFO, buf, len);
}


void processGetIdCommand()
{
	Uint16 buf[8];
	sysControlGetId(buf);
	sendReply(GET_ID, buf, 8);
}

void processGetFsParamsCommand()
{
	Uint32 buf[4];
	buf[0] = FS_PAGE_SIZE;
	buf[1] = FS_PAGES_PER_BLOCK;
	buf[2] = FS_BLOCKS_PER_FILE;
	lastCommandStatus = flashGetNoOfPages(&buf[3]) == OK;
	sendReply(GET_FS_PARAMS, (void *)&buf, sizeof(buf));
}

void processSetRtcTimeCommand(Uint32 newTime)
{
	Bool result;
	MSP_Time t;
	t.rtime = newTime;
	result = ( msp_settime(&t, NULL) == MSP_OK);
	if(result) {
		tickSetUnixTime(newTime);
	}
	sendReply(SET_RTC_TIME, (void *)&result, sizeof(result));
}

void processGetSensorsCommand()
{
	lastCommandStatus = (msp_getsensors(&sensorData) == MSP_OK);
	if(lastCommandStatus) {
		sendReply(GET_SENSORS, (void *)&sensorData, sizeof(sensorData));
	}
    else {
    	sendStatusData(lastCommandStatus);
    }
}

void processReadMspRegCommand()
{
	int data[N_APP_PARAMS*2];
	int dataPacked[N_APP_PARAMS];
	
	lastCommandStatus = (msp_get(0, data, N_APP_PARAMS) == MSP_OK);
	if(lastCommandStatus) {
		msp_brpack((uns *)dataPacked, data, N_APP_PARAMS);
		sendReply(GET_MSP_REG, (void *)dataPacked, N_APP_PARAMS);
	}
    else {
    	sendStatusData(lastCommandStatus);
    }
}

void processGetSwVerCommand()
{
	Uint16 count;
	char swVer [40];
	lastCommandStatus = flashGetFirmwareDetails(swVer, 40, &count) == OK;
	if( lastCommandStatus ) {
		sendReply(GET_SW_VER, (void *)&swVer, count);
	}
	else {
    	sendStatusData(lastCommandStatus);
	}
}

void processMspSendCommand(Uint16 n, Uint16 *in)
{
	Uint16 rx;
	Uint16 r;
	r = msp_send(in, n, &rx, 1);
	lastCommandStatus = (r == 1) && (rx == 0x90);
	sendStatusData(lastCommandStatus);
}

extern Int32 adcData[];
extern int adcLogEnable;

void processAdcI2cReadCommand()
{
	adcLogEnable = 1;
	sendReply(ADC_I2C_READ, (void *)&adcData[messageBuf[0]], 2);
}


#define CRC_SPEED_TEST_DATA_LENGTH 1000
#define CRC_SPEED_TEST_CYCLES 100

Uint16 testdata[CRC_SPEED_TEST_DATA_LENGTH];

typedef struct {
	Uint32 et;
	Uint16 crc;
} CrcSpeedTestResult;

void processCrcSpeedTestCommand()
{
	Uint32 startTime;
	Uint16 i;
	CrcSpeedTestResult tr;

	startTime = getTickCount();
	for(i=0;i<CRC_SPEED_TEST_DATA_LENGTH; i++) testdata[i] = i;

	for(i=0;i<CRC_SPEED_TEST_CYCLES; i++) {
		tr.crc = crc(testdata, CRC_SPEED_TEST_DATA_LENGTH);
	}

	tr.et =  getTickCount() - startTime;
	sendReply(CRC_SPEED_TEST, (void *)&tr, sizeof(tr));
}

void processUartRxCommand()
{
	Uint16 i;
	Uint16 rx[10];
	i = 0;//uartRxBuffer(rx, 10);
	sendReply( UART_RX_TEST, rx, i);
}


void processAccelTestCommand()
{
	Uint16 address[] = {0x06};
	Uint16 buff[6];

	i2cWrite(0x000F, address, 1);
	lastCommandStatus  = i2cRead(0x000F, (Uint16*)buff, 6) == OK;
	sendStatusData(lastCommandStatus);
}



void processAudiosampleCommand()
{
	TmessageHeader header;
	header.sop = SOP;
	header.id = deviceID;
	header.type = AUDIO_TEST_SAMPLE;
	header.dataLength = 0;
	header.dataCRC = 0;//crc(&systemConfig, sizeof(TConfig));
	header.headerCRC = crc(&header, sizeof(header) -  sizeof(header.headerCRC));
	txBuffer(&header, sizeof( header ) );
	//txBuffer(buf, AUDIO_BUFFLEN);
}

void audioTriggerUartHandler(int data)
{
	uartClearRx();
	audioTestTrigger();
}

void processEnableAudioTriggerCommand(int enable)
{
	if(enable) {
		uartRegRxCallbackHandler(audioTriggerUartHandler);
		uartClearRx();
	}
	audioTestEnableOneShot(enable);
	uartEnableRxInterrupt(enable);
}

void processSdScanCardsCommand(int cardNo)
{
	int i;
	TcardData dat[4];
	for(i=0; i<4; i++) {
		memCardSelect(i);
		microdelay(0xffff);
		flashInit();
		IRQ_globalEnable();

		dat[i].totalSectors = flashTotalPageCount;
		dat[i].sectorsUsed = flashFirstFreeSector;
	}
	memCardSelect(cardNo);
	microdelay(0xffff);
	flashInit();
	IRQ_globalEnable();

	sendReply(SD_SCAN_CARDS, (void *)&dat, sizeof(dat));
}


void processSdSetCardCommand(int cardNo)
{
	memCardSelect(cardNo);
	microdelay(0xffff);
	flashInit();
	IRQ_globalEnable();
	sendStatusData(1);
}


extern Uint16 UsbReadAudioBuffers;

void executeCommand(TmessageHeader *header, Uint16 *messageBuf )
{
  	switch( header->type )
  	{
    	case STATUS_READ:
      		sendStatusData(lastCommandStatus);
      	break;
    	case FLASH_FORMAT:
    		lastCommandStatus = flashErase(*(Uint32 *)messageBuf, *(Uint32 *)&messageBuf[2]) == OK;
    		sendStatusData(lastCommandStatus);
    	break;
    	case FLASH_WRITE:
    	break; 

		case MSP_SEND_COMMAND:
			//msp_send(&messageBuf[1], messageBuf[0], tempBuf, 10);
		break;

    	case FLASH_SET_READ_PAGE_ADDRESS:
			UsbReadAudioBuffers = 0;
    		lastCommandStatus = flashSetReadAddress(*(Uint32 *)messageBuf) == OK;
    		sendStatusData(lastCommandStatus);
    	break;

    	case FLASH_SET_READ_PAGE_INCREMENT:
    		lastCommandStatus = flashSetReadIncrement(*(Uint32 *)messageBuf) == OK;
    		sendStatusData(lastCommandStatus);
    	break;
    	

    	case SET_FLASH_WRITE_PAGE_ADDRESS:
    		flashWritePageAddress = *(Uint32 *)messageBuf;
    		lastCommandStatus = TRUE;
    		sendStatusData(lastCommandStatus);
    	break;
    	
    	case SERIAL_FLASH_WRITE:
			lastCommandStatus = serialEepromWrite(messageBuf[0], &messageBuf[2], messageBuf[1]/2);
    		sendStatusData(lastCommandStatus);
		break;
    	case SERIAL_FLASH_READ:
    		processSerialEepromReadCommand(messageBuf[0], messageBuf[1]/2);
    	break;
    	
    	case GET_ID:
    		processGetIdCommand();
    	break;
    	
    	case GET_FS_PARAMS:
    		processGetFsParamsCommand( );
    	break;
    
    	case SET_RTC_TIME:
    		processSetRtcTimeCommand(*(Uint32 *)messageBuf);
    	break;

    	case SET_CONFIG:
    		lastCommandStatus = configUpdate(*(TConfig *)messageBuf);
    		lastCommandStatus &= mspPrepRestart();
    		sendStatusData(lastCommandStatus);
    	break;

    	case GET_CONFIG:
    		sendConfigData();
    	break;

    	case GET_SENSORS:
			processGetSensorsCommand();    	
    	break;
    	
		case GET_MSP_REG:
			processReadMspRegCommand();
		break;
		case REBOOT:
			msp_hibernate(1);
		break;
		
		case GET_SW_VER:
			processGetSwVerCommand();
		break;
		
		case CRC_SPEED_TEST:
			//msp_setsws(600);
			//msp_arm(0x02);
			//processCrcSpeedTestCommand();
		break;

		case MSP_SEND:
			processMspSendCommand(messageBuf[0], &messageBuf[1]);
		break;

		case MSP_SET_TEST_VAL:
			gpioSetVal(GPIO_BIT_MSP_PROG, (gpioDir)messageBuf[0]);
    		sendStatusData(1);
		break;

		case MSP_SET_TEST_DIR:
			gpioSetDir(GPIO_BIT_MSP_PROG, (gpioDir)messageBuf[0]);
    		sendStatusData(1);
		break;

		case AUDIO_TEST_ENABLE:
			audioTestEnable(messageBuf[0]);
    		sendStatusData(1);
		break;

		case AUDIO_TEST_SAMPLE:
			UsbReadAudioBuffers = 1;
			processAudiosampleCommand();
			//audioSample(messageBuf[0]);
		break;

		case AUDIO_SET_GAIN:
			audioTestSetGain(messageBuf[0]);
			sendStatusData(1);
		break;

		case AUDIO_SET_MUTE:
			audioTestSetMute(messageBuf[0]);
			sendStatusData(1);
		break;

		case AUDIO_SET_HPASS:
			audioTestSetHighPass(messageBuf[0]);
			sendStatusData(1);
		break;

		case AUDIO_ENABLE_CAL_SIG:
			audioTestCalDacEnable(messageBuf[0]);
			sendStatusData(1);
		break;

		case AUDIO_SET_DIVIDER:
			audioTestSetSampleRate(messageBuf[0]);
			sendStatusData(1);
		break;

		case AUDIO_SET_CAL_TONE_F:
			audioSetCalTonef(*(Uint32 *)messageBuf);
			sendStatusData(1);
		break;

		case FLASH_GET_CARD_INFO:
			processGetFlashCardInfoCommand();
		break;

		case LED_FLASH:
			flashState = 30;
		break;

		case UART_TX_TEST:
			uartTx((Uint16 *)"test\r", 5);
		break;

		case UART_RX_TEST:
			processUartRxCommand();
		break;

		case UART_SELECT_TRANS:
			uartSelectTransceiver(messageBuf[0]);
		break;

		case UART_ENABLE_TX:
			uartEnableTx(messageBuf[0]);
		break;

		case ACCEL_TEST:
			processAccelTestCommand();
		break;

		case AUDIO_ENABLE_TRIGGER:
			processEnableAudioTriggerCommand(messageBuf[0]);
			sendStatusData(1);
		break;

		case AUDIO_SET_ACTIVE_CHANNEL:
			audioSetActiveChannel(messageBuf[0]);
			sendStatusData(1);
		break;

		case ADC_I2C_READ:
			processAdcI2cReadCommand();
		break;

		case SD_SET_CARD:
			processSdSetCardCommand(messageBuf[0]);
		break;

		case SD_SCAN_CARDS:
			processSdScanCardsCommand(messageBuf[0]);
		break;

  	}


}
