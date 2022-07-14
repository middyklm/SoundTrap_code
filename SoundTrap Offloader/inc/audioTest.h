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

extern Uint16 audioTestBuf[];
extern Uint16 *pAudioBuf;
extern Uint16 audioBlocksProcessed;

int	audioTestSetMute(int on);
int	audioTestSetGain(int on);

void audioTestCalDacEnable(int enable);
void audioSample(Uint16 noOfBuffers);
void audioTestEnable(int enable);
void audioTestRegisterAudiohandler( void (*handler)(Uint16 *buf) );
void audioTestReadNextBuf(Uint16 **pReadBuffer);
Uint16 audioTestGetAudioHardwareId();
void audioTestSetSampleRate(Uint16 divider);
void audioSetCalTonef(Uint32 calTonefrequency);
int	audioTestSetHighPass(int on);
void audioTestInit();
void audioTestEnableOneShot(int enable);
void audioSetActiveChannel(Uint16 channel);
void audioTestTrigger();


