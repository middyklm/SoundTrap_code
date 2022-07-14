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

#ifndef CONFIG_H_
#define CONFIG_H_

enum StartTriggerMode {
	startTriggerModeManual,
	startTriggerModeOnDisconnect,
	startTriggerModeSWS,
	startTriggerModePressure,
	startTriggerModeTime
};

enum RunMode {
	runModeContinuous,
	runModeTimed
};

enum SampleMode {
	sampleModeWave,
	sampleModeSpectral
};

#define ACCEL_SENSOR 0x01
#define TEMP_SENSOR 0x02
#define PRESSURE_SENSOR 0x02
#define CONFIG_FLAGS_DISABLE_CAL_ROUTINE 0x0001

typedef struct  {
	Uint32 startTriggerMode;
	Uint32 runMode;
	Uint32 startTime;
	Uint32 onTime;
	Uint32 onTimeScale;
	Uint32 onceEveryTime;
	Uint32 onceEveryTimeScale;
	Uint32 sampleMode;
	Uint32 decimator;
	Uint32 auxSensorEnable;
	Uint32 syncMode;
	Uint32 gain;
	Uint32 compressionMode;
	Uint32 auxSensorInterval;
	Uint32 flags;
	Uint32 highPass;
	Uint16 serialLogMode;
	Uint16 channelEnable;
	Uint32 detect1;
	Uint32 detect2;
	Uint32 detect3;
	Uint32 detect4;
	Uint32 detect5;
	Uint32 detect6;
	Uint32 detect7;
	Uint32 detect8;
	Uint32 detect9;
	Uint32 detect10;
	Uint16 unused10;
	Uint16 crc;
} TConfig;

extern TConfig systemConfig;

int configUpdate(TConfig newConfig);
int configInit();


#endif /*CONFIG_H_*/
