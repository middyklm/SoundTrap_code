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

#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include <tistdtypes.h>

typedef enum {
	STATUS_READ = 4,
	FLASH_FORMAT = 5,
	FLASH_WRITE = 6,
	FLASH_SET_READ_PAGE_ADDRESS = 7,
	SET_FLASH_WRITE_PAGE_ADDRESS = 8,
	SERIAL_FLASH_WRITE = 9,
	SERIAL_FLASH_READ = 10,
	MSP_SEND_COMMAND = 11,
	BUSY = 12,
	GET_ID = 13,
	GET_FS_PARAMS = 14,
	SET_RTC_TIME = 15,
	FLASH_SET_READ_PAGE_INCREMENT = 16,
	SET_CONFIG = 17,
	GET_CONFIG = 18,
	GET_SENSORS = 19,
	GET_MSP_REG = 20,
	REBOOT = 21,
	GET_SW_VER = 22,
	CRC_SPEED_TEST = 23,
	MSP_SEND = 24,
	MSP_SET_TEST_VAL = 25,
	MSP_SET_TEST_DIR = 26,
	AUDIO_TEST_ENABLE = 27,
	AUDIO_TEST_SAMPLE = 28,
	AUDIO_SET_GAIN = 29,
	AUDIO_SET_MUTE = 30,
	AUDIO_ENABLE_CAL_SIG = 31,
	AUDIO_READ_HARDWARE_ID = 32,
	AUDIO_SET_DIVIDER = 33,
	AUDIO_SET_CAL_TONE_F = 44,
	FLASH_GET_CARD_INFO = 45,
	AUDIO_ENABLE_TRIGGER = 46,
	LED_FLASH = 48,
	AUDIO_SET_HPASS = 49,
	UART_TX_TEST = 50,
	UART_RX_TEST = 51,
	UART_SELECT_TRANS = 52,
	UART_ENABLE_TX = 53,
	ACCEL_TEST = 54,
	AUDIO_SET_ACTIVE_CHANNEL = 55,
	ADC_I2C_READ = 56,
	SD_SET_CARD = 57,
	SD_SCAN_CARDS = 58

} TPacketType;

void executeCommand(TmessageHeader *header, Uint16 *messageBuf );

#endif
