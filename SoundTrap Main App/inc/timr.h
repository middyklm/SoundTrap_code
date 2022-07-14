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

// TIMR module - timed or periodic task launching
// timr.h v1.0

#include "job.h"

// error codes
#define		TIMRALREADYOPEN		(1)
#define		TIMROPENFAIL		(2)
#define		TIMRBADSYSCLK		(3)
#define		TIMRJOBNOTFOUND		(4)
#define		TIMRTOOMANY			(5)

// maximum number of timer jobs allowed
#define	TIMR_MAXWAITING		(30)
#define	TIMR_MAXMSWAITING	   (10)

// nice number for jobs posted using TIMR_doevery and TIMR_doin
#define	TIMR_NICE	(64)

#define	NULLTIMR	((TIMR_Obj *)NULL)

// timer interrupt interval in microseconds
#define	TIMR_INTVL	(2000)

#define	TIMR_doevery(secs,f,state)	TIMR_schedule((secs) * 1000,-1,f,state,TIMR_NICE)
#define	TIMR_doin(secs,f,state)		TIMR_schedule((secs) * 1000, 1,f,state,TIMR_NICE-1)
#define	TIMR_DO_IN_MS(ms,f,state)		TIMR_schedule(ms, 1,f,state,TIMR_NICE-1)
#define	TIMR_DO_EVERY_MS(ms,f,state)	TIMR_schedule(ms,-1,f,state,TIMR_NICE)

extern int		TIMR_open(void) ;
extern void		TIMR_close(void) ;
extern int		TIMR_attach(JOB_Fxn f, Ptr s, int nice) ;
extern int		TIMR_gettime(ulong *secs, ulong *microsecs) ;
extern int		TIMR_schedule(Uint32 seconds, int cnt, JOB_Fxn f, Ptr s, int nice) ;
extern int		TIMR_cancel(Uint32 seconds, JOB_Fxn f) ;
extern void		TIMR_tout(int on) ;
extern ulong 	TIMR_getUnixTime();
extern ulong 	TIMR_GetTickCount();
void delay_ms(Uint32 duration); // Middy's code
