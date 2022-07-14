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
 * Logs GPS data received on the serial port, including (optional) PPS for global time
 * reference.
 *
 */

#include <tistdtypes.h>
#include <string.h>
#include "d3std.h"
#include "d3defs.h"
#include "uart.h"
#include "logg.h"
#include "data.h"
#include "job.h"
#include "info.h"
#include "board.h"
#include "audio.h"
#include "cfg.h"

#define BUFF_LENGTH  100 //maximum characters in each GPS log file line
#define GPS_LOG_HDR  "\"rtime\",\"mticks\",\"data\""
#define SERIAL_BAUD_RATE 9600

static int gpsLogggerLogFileId;
static int Audid;
static int Decimator;

static char charBufferRx[BUFF_LENGTH];
static char charBufferRxCopy[BUFF_LENGTH];
static ulong audioSampleCount = 0;

int gpsLoggerPostResult(Ptr p1, Ptr p2)
{
	int n;
	char b[12];
	n = strlen(charBufferRxCopy);
	snprintf(b,12, ",%lu\r", audioSampleCount);
	audioSampleCount = 0;
	strncat(charBufferRxCopy, b, BUFF_LENGTH - n); //add the audio sample to end of output line
	LOG_diary(gpsLogggerLogFileId, charBufferRxCopy);
	return OK;
}

void gpsLoggerRx(int data)
{
	static int rxIdx = 0;

	if( data == 0x80 ){ // PPS timing mark = 0x80 character for ST GPS hardware
		audioSampleCount = GetSampleCount(CFG_getstate(Audid))/ Decimator; //record the audio sample count
	}

	if(data == 0x0A) { 	// line feed = end of line
		charBufferRx[rxIdx-1] = '\0';	//replace the CR with NULL
		strncpy(charBufferRxCopy, charBufferRx, BUFF_LENGTH);	//take a copy for the log function
		rxIdx = 0;	//reset the line pointer
		JOB_postone((JOB_Fxn)gpsLoggerPostResult,NULL,MAXNICE-1,NULL); //initiate the log process
		return;
	}

	charBufferRx[rxIdx] = data; //store each character as it arrives
	if(rxIdx < BUFF_LENGTH-1) rxIdx++;
}

void gpsLoggerFlush()
{
	if(gpsLogggerLogFileId) {
		LOG_flush(gpsLogggerLogFileId);
		LOG_add(gpsLogggerLogFileId,0,0,GPS_LOG_HDR);
	}
}

int gpsLoggerInit(int audioId, int decimator)
{
    char s[30];

    Audid = audioId;
    Decimator = decimator;
    uartInit( getsysclk(), SERIAL_BAUD_RATE);	//set baud rate
	uartSelectTransceiver(1); 				// enable GPS transceiver
	uartEnableTx(0);						//disable transceiver tx

	gpsLogggerLogFileId = LOG_open("gps.csv",NOQUOTE); //open an output file
	LOG_add(gpsLogggerLogFileId,0,0,GPS_LOG_HDR);
	uartRegRxCallbackHandler(gpsLoggerRx);

	snprintf(s, 30, "%d", SERIAL_BAUD_RATE);
	INFO_event("GPS_RX_BAUD", NULL, s);

	return OK;
}
