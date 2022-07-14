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

/*! \file d3defs.h
    \brief D3 API standard definitions.

    Include this file to access basic board interface commands.
    \todo There are many holes in the documentation that need to be filled.
    A few additional restrictions or jobs to do are collected here.
*/

/*! \mainpage D3 Application Programming Interface
 *
 * \section intro_sec Introduction
 *
 * The D3 API is a simple non-preemptive real-time scheduler together with
 * a number of data generating and signal processing functions useful for
 * building embedded real-time sampling systems. The API is intended
 * for use with Texas Instruments C55x digital signal processors and the
 * Code Composer Studio IDE. Functions are optimized for sampling audio
 * signals and performing down-sampling, filtering and compression before
 * saving data in a FLASH file.
 */

#ifndef _D3DEFS_H_
#define _D3DEFS_H_

#include	<stdio.h>
#include 	<csl_dma.h>
#include 	<csl_gpio.h>
#include 	<csl_i2c.h>
#include 	<csl_pll.h>
#include 	<csl_rtc.h>
/*
#include   "modnumbers.h"
#include	"d3std.h"
#include	"protect.h"
#include   "board.h"
#include   "devdep.h"
#include   "misc.h"
*/

#define	MAX(a,b)	(((a)>(b))? (a):(b))
#define	MIN(a,b)	(((a)<(b))? (a):(b))

#endif

