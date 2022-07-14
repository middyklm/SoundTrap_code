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

/* Revision history
 * 1.0.0.19 Enabled X3 compression
 * 1.0.0.23 Jan 15. Pull downs on IO
 * 1.0.2.5 July 15. STV2 support, I2C bug fix
 * 1.0.2.6 August 15. schedule time bug fix
 * 1.0.2.7 August 15. schedule time bug fix 2
 * 1.0.2.8 September 8
 * 1.0.2.9 September 9. allow schedule off time = 0, fixed neg temp bug, sensor flush, sens timestamps
 * 1.1.0.0 November 15. Added detection
 * 2.0.0.0 October 16. Added four channel
 * 2.0.0.1 December 16. Changed Bat V scaling in MSP, sample count calc to exclude skipped frames
 * 2.0.0.2 December 22. Reinstated cal tones, dma changes, changes to audio thottling, click count reset, dma config speed up
 * 2.0.0.3 December 22. No change
 * 2.0.0.5 23 Feb 2017. Fixed decm=0 = no sampling
 * 2.0.0.6 31 May 2017. Fixed missing meta data bug
 * 3.0.0.0 12 March 2018. Support for ST500
 * 3.1.0.0 Support for ST500
 * 3.1.0.1 Support for ST500 multicard. Support for ST4300HF
 * 3.1.0.2 Support for new accelerometer
 */

#pragma	DATA_SECTION(swVer,".swver")
#ifdef PLOG
const char swVer[] = "PL.1.0.0.0";
#else
//const char swVer[] = "3.1.0.2";
const char swVer[] = "MK.0.0.0.1";
#endif
#pragma	DATA_SECTION(swDate,".swver")
const char swDate[] = __DATE__;
#pragma	DATA_SECTION(swTime,".swver")
const char swTime[] = __TIME__;


