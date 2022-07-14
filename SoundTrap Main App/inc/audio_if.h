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

//	AUDIO module - Low-level audio ADC and calibration DAC interface driver
// audio_if.h v1.0
// Handles the synchronous serial and dma interface to the audio and 
// sensor board.

#ifndef AUDIO_IF_H_
#define AUDIO_IF_H_


void audio_init(void);
int audio_openchans(int channels, int nchan, uns band, Ptr streamHandle);
void audio_close();
void audio_start();
void audio_stop();
void audio_nextbuff(Ptr buf, int bufLength);
void audioStartAllChannels();

#endif
