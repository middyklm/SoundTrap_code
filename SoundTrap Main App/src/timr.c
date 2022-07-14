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

/*! \file timr.c
    \brief Timed or periodic task launching.
*/

#include "csl_gpt.h"
#include "csl_intc.h"

#include "d3defs.h"
#include "d3std.h"
#include "protect.h"
#include "board.h"
#include "devdep.h"
#include "misc.h"
#include "modnumbers.h"
#include "error.h"
#include "dmem.h"
#include "job.h"
#include "timr.h"

interrupt void TIMR_isr(void) ;

typedef	struct {
	Uint32			clk ;	// cpu clk in Hz
	CSL_PreScale	prsc ;	// prescalar for timer (use psc-1)
	Uint32			prd ;	// timer period (use prd-1) to get a
							// timer interrupt every 2 ms
	Uint32			scale ;	// scalar to convert timer ticks to
							// microseconds 
} TIMR_Settings ;


#define	TIMR_NSET	(10)

static TIMR_Settings	TIMR_SET[TIMR_NSET] = {
	{8192000, GPT_PRE_SC_DIV_0,8192,  4},
	{16384000,GPT_PRE_SC_DIV_0,16384, 8},
	{18432000,GPT_PRE_SC_DIV_0,18432, 10},
	{32768000,GPT_PRE_SC_DIV_0,32768, 16},
	{36864000,GPT_PRE_SC_DIV_0,36864, 18},
	{49152000,GPT_PRE_SC_DIV_0,49152, 24},
	{65536000,GPT_PRE_SC_DIV_1,32768, 16},
	{73728000,GPT_PRE_SC_DIV_1,36864, 18},
	{96000000,GPT_PRE_SC_DIV_2,24000, 12},
	{98304000,GPT_PRE_SC_DIV_2,24726, 12},
};


typedef	struct {
	CSL_Handle	h ;
	Uint32		scale ;
	Uint32		prd ;
	int			init ;
} TIMR_Obj ;


typedef	struct {
	Uint32		wait ;	// cpu clk in MHz
	JOB_Fxn	job ;
	Ptr		state ;
	int		nice ;
	int		cnt ;
	Uint32		waitcnt ;
} TIMR_WaitObj ;

// as a temporary hack there are two queues. The normal one is for
// processes that occur on >= second intervals
// The second queue is for one-off delays of multiple milliseconds.

static TIMR_WaitObj		WAITOBJ[TIMR_MAXWAITING] ;
static int				nwaiting = 0 ;
static int				ticks = 0 ;
static volatile ulong	msTicks = 0 ;
CSL_GptObj		gptObj;

#ifdef NOT_NEEDED_YET
inline int	TIMR_irqtest(Uint16 EventId) ;
#endif

Time	SYSTIME = 0 ;

// how many times the timer isr will fire per second
#define	TIMR_ISRPERSEC	(500)
#define	TIMR_MSPERISR	(1000 / TIMR_ISRPERSEC)

// local state for the timer
static TIMR_Obj	TIMR_OBJ = {(CSL_Handle)INV,0,0} ;

int		timr_cfg(Uint32 clk) ;

// isr
interrupt void TIMR_isr(void)
{
	// this interrupt runs every 2 ms
	int k;
	PROTECT ;
	CSL_SYSCTRL_REGS->TIAFR = CSL_IAFR_TIMER_FLAG_0_RESETVAL;

	msTicks += TIMR_MSPERISR;

	// check for any short delay
	if(++ticks>=TIMR_ISRPERSEC) {
		ticks = 0 ;
		++SYSTIME ;
	} // if(++ticks...

	// go through all waiting jobs
	for(k=0;k<nwaiting;k++) {
	   TIMR_WaitObj	*w ;

	   w = &(WAITOBJ[k]) ;	// get the next waiting job

	   // check if job is expired
	   if(w->cnt==0) { // shuffle waiting cues to eliminate the job
		  int	kk ;
		  // TODO: replace with single in-place memcpys that go in correct order
		  // to not overwrite the cues
		  for(kk=k; kk<nwaiting-1; kk++)
			 memcpy(&(WAITOBJ[kk]),&(WAITOBJ[kk+1]),sizeof(TIMR_WaitObj)) ;

		  --nwaiting ;
		  --k ;
		  }

	   // otherwise see if the job is due
	   else if(--(w->waitcnt)<=0) {
		  w->waitcnt = w->wait ;
		  JOB_postone(w->job,w->state,w->nice,(Ptr)&SYSTIME) ;

		  if(w->cnt>0)
			 --(w->cnt) ;
		  }

	   } // for(k=0...


	END_PROTECT ;
}

ulong TIMR_getUnixTime()
{
	return SYSTIME;
}

ulong TIMR_GetTickCount()
{
	return msTicks;
}

int		TIMR_gettime(ulong *secs, ulong *microsecs)
{
	Uint32		ttick ;
	Uint32		m;
	Uint16     intStatus;

 	GO_ATOMIC ;
 	*secs = SYSTIME ;
 	if(microsecs == (Uint32 *)NULL) {
		END_ATOMIC ;
		return(OK) ;
	}

	//ttick = ((Uint32)TIMR_OBJ.h->regs->TIMCNT2 << 16) & TIMR_OBJ.h->regs->TIMCNT1;
	ttick = TIMR_OBJ.h->regs->TIMCNT1;
	intStatus = CSL_CPU_REGS->IFR0 & (0x001 << TINT_EVENT);
	if(ttick < 10) intStatus = 0;
	
 	ttick = TIMR_OBJ.prd - ttick;
 	ttick = ttick / TIMR_OBJ.scale;
 	m = ((Uint32)ticks * TIMR_INTVL) + ttick ;

 	// have to check if TINTx is set - if so, add an extra TIMR_INTVL
 	// to microsecs
 	if( intStatus > 0 )
    	m += TIMR_INTVL;

 	*microsecs = m ;
 	END_ATOMIC ;
 	return(OK) ;
}

int		TIMR_open(void)
{
 	CSL_Handle h ;
	CSL_Status 	status;

 	if(TIMR_OBJ.h != INV) {
		err(TIMR_MOD,TIMRALREADYOPEN) ;
		return(FAIL) ;
	}

 	if((h=GPT_open(GPT_0, &gptObj, &status))==INV) {
		err(TIMR_MOD,TIMROPENFAIL) ;
		return(FAIL) ;
	}
	
 	TIMR_OBJ.h = h ;
 	TIMR_OBJ.init = 0 ;

 	if(timr_cfg(getsysclk())==FAIL) {
		GPT_close(h) ;
		TIMR_OBJ.h = INV ;
		return(FAIL) ;
	}

 	nwaiting = 0 ;
 	ticks = 0 ;

 	// initialize SYSTIME from the RTC

 	SYSTIME = getrtctime();

 	if(SYSTIME == 0ul) {
 		return FAIL; //get RTC time failed
 	}

 	if( abs( getrtctime() - SYSTIME ) > 1 ) { //get RTC time again, to verify
 		return FAIL;	//times don't match
 	}

 	GO_ATOMIC ;
 	IRQ_plug(TINT_EVENT,&TIMR_isr);
 	IRQ_clear(TINT_EVENT);
 	IRQ_enable(TINT_EVENT);
 	END_ATOMIC ;
 	onclkchange((CLK_Fxn)timr_cfg) ;
 	GPT_start(h) ;
 	return(OK) ;
}


void	TIMR_close(void)
{
	CSL_Handle	h ;

	h = TIMR_OBJ.h ;
	if(h!=INV) {
		IRQ_disable(TINT_EVENT);
		GPT_close(h) ;
	}
}


int		TIMR_schedule(Uint32 ms, int cnt, JOB_Fxn f, Ptr s, int nice)
{
	TIMR_WaitObj	*w ;

	if(nwaiting >= TIMR_MAXWAITING) {
		err(TIMR_MOD,TIMRTOOMANY) ;
		return(FAIL) ;
	}

	GO_ATOMIC ;
	w = &(WAITOBJ[nwaiting]) ;
	w->job = f ;
	w->state = s ;
	w->nice = nice ;
	w->wait = ms / TIMR_MSPERISR;
	w->waitcnt = w->wait;
	w->cnt = cnt ;
	++nwaiting ;
	END_ATOMIC ;
	return(OK) ;
}


int		TIMR_cancel(Uint32 seconds, JOB_Fxn f)
{
	int			k ;
	TIMR_WaitObj	*w ;

	GO_ATOMIC ;
	for(k=0; k<nwaiting; k++) {
		w = &(WAITOBJ[k]) ;
		if(w->wait == (seconds * TIMR_ISRPERSEC ) && w->job == f)
			w->cnt = 0 ;
	}

	END_ATOMIC ;
	return(OK) ;
}


int		timr_cfg(Uint32 hz)
{
	CSL_Status status;
	CSL_Config tcfg;
 	Uint16		  k ;

 	// find appropriate settings given the sysclk frequency
 	for(k=0;k<TIMR_NSET && hz!=TIMR_SET[k].clk;k++) ;

 	if(k==TIMR_NSET) {
 		err(TIMR_MOD,TIMRBADSYSCLK) ;
		return(FAIL) ;
	}

 	TIMR_OBJ.prd = TIMR_SET[k].prd ;
 	TIMR_OBJ.scale = TIMR_SET[k].scale ;

	tcfg.autoLoad 		= GPT_AUTO_ENABLE;
	tcfg.ctrlTim 		= GPT_TIMER_ENABLE;
	tcfg.preScaleDiv 	= TIMR_SET[k].prsc;
	tcfg.prdLow 		= TIMR_SET[k].prd & 0x0000FFFF;
	tcfg.prdHigh 		= TIMR_SET[k].prd >> 16;

 	GO_ATOMIC ;
 	GPT_stop(TIMR_OBJ.h);
 	GPT_reset(TIMR_OBJ.h);
	status = GPT_config(TIMR_OBJ.h, &tcfg);

	if(status == CSL_SOK)
    	GPT_start(TIMR_OBJ.h) ;		// only restart the timer if it was already running

 	TIMR_OBJ.init = 1 ;
 	END_ATOMIC ;
 	return(OK) ;
}


#ifdef NOT_NEEDED_YET
inline int	TIMR_irqtest(Uint16 EventId)
{
	Uint16 bit,reg,mask;

	bit= EventId & 0xfu ;
	reg= (EventId & 0x10u) >> 4 ;
	mask= 1<<bit ;
	bit = (reg?(_IFR1 & mask): (_IFR0 & mask)) != 0 ;
	return(bit) ;
}
#endif

// --------- Middy's code -----------
/*
This delay function is not highly accurate.
f = 32768000 Hz, t = 1/f (s)
When tick = 32768000, t = 1s.
When tick = 32768, t = 1ms.
Apparently, according to MAXCLKCHANGEFXNS = 8 in board.c, clock is divided by 8. So when tick = 32768/8 = 4096, t = 1ms.
Use 2 while loops get time delay more accurate than 1 loop.
*/

void delay_ms(Uint32 duration)
{
    Uint16 tick = 0, ofw = 0;

    while(duration--)
    {
        tick = 0, ofw = 0;

        while(ofw < 2048){ // 1ms count
            tick++;
            if(tick > 2){
                tick = 0;
                ofw++;
            }
        }

    }
}

// --------end Middy's code ---------
