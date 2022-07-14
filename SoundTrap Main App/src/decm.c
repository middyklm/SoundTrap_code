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

/*! \file decm.c
    \brief decimation (down-sampling) data processor.
*/

#include <math.h>
#include <string.h>
#include "d3defs.h"
#include "d3std.h"
#include "protect.h"
#include "board.h"
#include "devdep.h"
#include "misc.h"
#include "modnumbers.h"
#include "error.h"
#include "job.h"
#include "dmem.h"
#include "fmem.h"
#include "data.h"
#include "cfg.h"
#include "info.h"
#include "decm.h"
#include "minmax.h"


 // important: don't change the order of the elements in this structure.
 // the assembly language code accesses elements by offset.
 typedef struct {
		int		boffs ;		// buffer offset
		int		nf ; 		// filter length
		int		nch	;		// number of channels
		int		bsize ;		// size of circular buffer
		int		df ; 		// decimation factor
		int		nnew ;		// number of new samples needed
		unsigned int idf ;	// fractional 1/df used to calculate output samples
		int		temp ;		// memory location used by decmc
		int	*coef ;		// filter coefficients
		int	*buff ;		// buffer start address
		int	*pout ;		// output data pointer
		int		ovld ;		// overload (sticky)
		// ovld is the last element accessed by the assembly routine

		int			nout ;		// number of samples/channel in output buffer
		DATA_Obj	*dout ;		// current output buffer
		JOB_Obj		*jlist ;	// list of jobs attached for receive
		DATA_Timekey	tkey ;	// structure for time calculation
		int 		nmax ;		// max samples/channel in output
		long		fsout ;		// last output sampling rate
		int			id ;		// number assigned to this configuration
		int			src ;		// source id number
		} DECM_Obj ;

 // assembly function to do the maths - it returns the number of output
 // samples calculated
 extern int		decmc(DECM_Obj *f, int *pin, int *pout, int nin) ;

 // error handler
 void	decm_error(int id) ;
 void DECM_reperr(int id);

 // initialization values for the structure
 #define decm_finv(a) 				((1u<<15)/(a))
 #define decm_buffsize(nf,df,nch)	((nch)*((nf)+2*(df)-1))
 #define decm_nnew(df)				(2*(df))


int		DECM_open(int *filt, int nfilt, uns df, int nchs)
{
 DECM_Obj	*p ;
 int		id ;

 if((p=(DECM_Obj *)DMEM_alloc(sizeof(DECM_Obj)))==(DECM_Obj *)NULL)
	 return(NULLID) ;		// allocation error

 p->coef = filt ;
 p->nch = nchs ;
 p->nf = nfilt ;
 p->df = df ;
 p->idf = decm_finv(df) ;
 p->bsize = decm_buffsize(nfilt,df,nchs) ;
 p->boffs = 0 ;
 p->ovld = 0 ;
 p->nnew = decm_nnew(df) ;
 p->nmax = DECM_OUTLEN/nchs ;	// maximum samples/channel in output
 p->nout = p->nmax ;
 p->dout = NULLDATA ;

 // TODO: need to allocate the buffer from a separate DARAM
 if((p->buff=(int *)FMEM_alloc(p->bsize))==(int *)NULL)
	 return(NULLID) ;		// allocation error

 // initialize the filter states
 memset(p->buff,0,p->bsize) ;

 DATA_INITTKEY(&(p->tkey)) ;
 p->jlist = NULLJOB ;
 id = CFG_register((JOB_Fxn)DECM_proc,DECM_attach,p) ;
 p->id = id ;
 p->src = NULLID ;
 return(id) ;
}


int		DECM_close(int id)
{
 DECM_Obj 	*p = (DECM_Obj *)CFG_getstate(id) ;

 // free the output buffer if there is one
 if(p->dout != NULLDATA)
 	DATA_free(p->dout) ;

 // free the filter states and control structure
 //FMEM_free(p->buff) ;
 DMEM_free(p) ;
 return(OK) ;
}


int		DECM_attach(int id, int downstr_id, int nice)
{
 DECM_Obj 	*p = (DECM_Obj *)CFG_getstate(id) ;

 if(nice==SRCNICE) {		// this is a SRC id notification call
	p->src = downstr_id ;
    return(DECM_meta(id)) ;		// post the metadata for this instance
	}

 return(JOB_add(&(p->jlist),CFG_getprocfxn(downstr_id),CFG_getstate(downstr_id),nice)) ;
}


void	DECM_remove(int id, int downstr_id)
{
 DECM_Obj 	*p = (DECM_Obj *)CFG_getstate(id) ;
 JOB_remove(&(p->jlist),CFG_getprocfxn(downstr_id)) ;
}


int	DECM_proc(Ptr pt, DATA_Obj *d)
{
 // called when there is data to work with

 DECM_Obj	*p = (DECM_Obj *)pt ;
 int		nin ;
 int		*pin ;

 if(p->jlist == NULLJOB) {		// no downstream jobs attached - nothing to do
	DATA_free(d) ;
	return(OK) ;
	}

 if((nin=d->nsamps)==0) {
	// zero length input data means a timing error in the source module
    decm_error(p->id) ;
	DATA_free(d) ;
	return(OK) ;
	}

 // get input pointer
 pin = (int *)(d->p) ;

 while(nin>0) {
	int		ntogo ;

	if(p->dout==NULLDATA) {			// if there is no output buffer
       DATA_Obj	*dout = p->dout ;	  // recover the output data object
	   // get one
	   if((p->dout=dout=(DATA_Obj *)DATA_alloc(DECM_OUTLEN))==NULLDATA) {
		  p->nout = p->nmax ;
		  DECM_reperr(p->id);
		  err(DECM_MOD,DECM_PROCALLOC) ;
		  //DMEM_status() ;
 		  DATA_free(d);
		  return(OK); 
	   }

	   // set the metadata
	   dout->id = p->id ;

	   if(DATA_settime(&(p->tkey),dout,d,d->nsamps-nin)) {
		  p->fsout = ((long)d->fs) / (p->df) ;
		  }

	   dout->fs = p->fsout ;
	   dout->nch = p->nch ;
	   dout->nbits = 16 ;			// need to work on this

	   // set the counters and pointer for the new output object
	   p->pout = dout->p ;
	   p->nout = 0 ;
	}

	if((ntogo=p->nmax-p->nout)>=2) {
		int		n, ns ;
		// do the actual processing
		n = MIN(nin,ntogo*p->df) ;	// number of input samples/channel for this call
		ns = decmc(p,pin,p->pout,n) ;	// returns the number of output samples created
		// decmc uses all n input samples and produces as many output 
		// samples as possible. Input points not needed to make an
		// output sample are added to the state vector for the next call
		pin += n*p->nch ;		// update input buffer pointer
		nin -= n ;				// remaining input samples/chan
		p->nout += ns ;			// number of output samples/chan done
		ntogo -= ns ;			// number of output samples/chan to do
		p->pout += ns*p->nch ;	// update output buffer pointer
		}

	if(ntogo<2) {				// if an output buffer is ready
       DATA_Obj	*dout = p->dout ;	  // recover the output data object
	   dout->nsamps = p->nout ;		  // set the size and number of samples
	   dout->size = p->nout*p->nch ;
       DATA_POST(&(p->jlist),dout) ;  // post the jobs attached
	   p->dout = NULLDATA ;			// signal to assign a new buffer
	   }
    } // while(nin>0)

 DATA_free(d) ;
 return(OK) ;
}


int		DECM_meta(int id)
{
 char		s[20] ;
 DECM_Obj 	*p = (DECM_Obj *)CFG_getstate(id) ;

  // setup the metadata
 INFO_new(id,NULL,p->src,NULLID) ;
 INFO_add(id,"PROC",NULL,"DECM") ;
 snprintf(s, 20, "%d",p->df) ;
 INFO_add(id,"DF",NULL,s) ;
 snprintf(s, 20, "%d",p->nf) ;
 INFO_add(id,"NF",NULL,s) ;
 return(INFO_end(id)) ;			// post the metadata
}


int		DECM_status(int id)
{
 char		s[20] ;
 DECM_Obj 	*p = (DECM_Obj *)CFG_getstate(id) ;

 if(p->ovld) {
	snprintf(s, 20, "ID=\"%d\" OVL=\"1\"",id) ;
	INFO_event("DECM",s,NULL) ;
	p->ovld = 0 ;				// reset the overload indicator
 }
 return(OK) ;		
}

void DECM_reperr(int id)
{
	DECM_Obj 	*p = (DECM_Obj *)CFG_getstate(id) ;
    char s[40];
    snprintf(s, 40, "WARNING=\"UNKNOWN\"");
    INFO_event("DECM", s, NULL);
}

void	decm_error(int id)
{
// Flush any current partial buffer and send a zero-length buffer downstream
 DATA_Obj	*d ;
 DECM_Obj 	*p = (DECM_Obj *)CFG_getstate(id) ;

 d = p->dout ;		// get the current output object
 if(d == NULLDATA)
	d = DATA_alloc(0) ;

 d->nsamps = d->size = 0 ;
 DATA_POST(&(p->jlist),d) ;		// post the jobs attached

 p->dout = NULLDATA ;
 p->nout = p->nmax ;
 // re-initialize the filter states
 memset(p->buff,0,p->bsize) ;
 p->nnew = decm_nnew(p->df) ;
}
