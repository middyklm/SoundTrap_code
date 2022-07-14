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

#include <tistdtypes.h>
#include "d3defs.h"
#include "d3std.h"
#include "protect.h"
#include "board.h"
#include "devdep.h"
#include "misc.h"
#include "modnumbers.h"
#include "error.h"

#include "config.h"
#include "crc.h"
#include "fs.h"
#include "data.h"
#include "info.h"
#include "sd.h"
#include "timr.h"
#include "audio.h"
#include "serialEeprom.h"


#define CONFIG_SFLASH_ADDRESS 32000//page 500
#define CONFIG_LOGICAL_BLOCK 8

TConfig systemConfig;

int configCheck(TConfig *config)
{
	return ( config->crc == crc(config, sizeof(TConfig)-1));
}

void configLoadDefaults(TConfig *config)
{
	config->startTriggerMode = startTriggerModeOnDisconnect;
	config->runMode = runModeContinuous;
	config->startTime = 0;
	config->onTime = 0;
	config->onceEveryTime = 0;
	config->onceEveryTimeScale = 0;
	config->onTimeScale = 0;
	config->sampleMode = sampleModeWave;
	config->decimator = 3;
	config->auxSensorEnable = 0;
	config->auxSensorInterval = 1;
	config->gain = HIGHGAIN;
	config->compressionMode = 0;
	config->flags = 0;
	config->highPass = 0;
	config->serialLogMode = 0;
	config->detect1 = 0;
	config->channelEnable = 0xffff;
	config->crc = crc(config, sizeof(TConfig)-1);
}

int configWrite(TConfig *config)
{
	return (serialEepromWrite(CONFIG_SFLASH_ADDRESS, (Uint16 *)config, sizeof(TConfig)) == OK);
}

int configRead(TConfig *config)
{
	return (serialEepromRead(CONFIG_SFLASH_ADDRESS, (Uint16 *)config, sizeof(TConfig)) == OK ) &&
			configCheck(config);
}

int configUpdate(TConfig newConfig)
{
	if(configCheck(&newConfig)) {
		systemConfig = newConfig;
		return configWrite(&systemConfig);
	}
	return FALSE;
}

int configInit()
{ 
	if(!configRead(&systemConfig)) {
		configLoadDefaults(&systemConfig);
		return FALSE;
	} 
	return TRUE;
}

