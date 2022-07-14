// D3-API v1.0
// Copyright (C) 2008-2010, Mark Johnson / WHOI
//
// This file is part of D3, a real-time patch panel scheduler
// for digital signal processors.
//
// D3 is free software: you can redistribute it 
// and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation, either version 3 of 
// the License, or any later version.
//
// D3 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with D3. If not, see <http://www.gnu.org/licenses/>.

// FILT module - general purpose FIR filter
// filt.c v1.0
// Last modified: June 2009

// Testing:
// generate a filter in Matlab using:
// H=round(32767*fir1(15,0.25));fprintf('%d,',H)
// Copy the coefficients into a global vector:
// DATA	FILTER[] = {-42,-177,-406,-352,669,2961,5846,7885,
//			7885,5846,2961,669,-352,-406,-177,-42} ;
// FILT should then produce the same output as:
// round(filter(H,1,x)/2^16)

#include	"d3defs.h"
#include	"d3std.h"
#include	<string.h>
#include 	"modnumbers.h"
#include	"job.h"
#include	"error.h"
#include	"dmem.h"
#include 	"fmem.h"
#include	"data.h"
#include	"cfg.h"
#include	"info.h"
#include	"filt.h"


typedef struct {
		// don't change the order here - the assembly language
		// routine expects things in certain places
		DATA	*coef ;			// filter coefficients
		DATA	*buff ;			// buffer start address
		int		boffs ;			// buffer offset
		int		nf ; 			// filter length
		int		nch	;			// number of channels
		int		bsize ;			// size of circular buffer
		int		ovl ;			// overload indicator
		JOB_Obj	*jlist ;		// list of downstream jobs attached
		int		id ;			// number assigned to this configuration
		int		src ;			// source id number
		DATA	dbuff ;			// first word of the filter data buffer
		} FILT_Obj ;


// assembly language function filtmc in filtmc_asm.s55 does the actual maths
// odd buffer sizes and multiple channels are supported.
extern int		filtmc(int *ip,int *op,FILT_Obj *f,int ns) ;

// the assembly code uses a buffer of size fltbuffsize. This should be
// allocated in a different daram space than the filter coefficients
#define	fltbuffsize(nf,nc)		((nc)*((nf)-1)+2)


int		FILT_open(DATA *filt, int nfilt, int nchs)
{
 FILT_Obj	*p ;
 int		id, n ;

 if(filt == (DATA *)NULL) {
	 err(FILT_MOD,FILT_NOFILT) ;
	 return(NULLID) ;
	 }

 n = fltbuffsize(nfilt,nchs) ;		// how big must the buffer be

 if((p=(FILT_Obj *)FMEM_alloc(sizeof(FILT_Obj)+n-1))==(FILT_Obj *)NULL) {
	 err(FILT_MOD,FILT_NOFILT) ;
	 return(NULLID) ;		// allocation error
	 }

 // initialize the filter states
 memset(&(p->dbuff),0,n) ;
 p->coef = filt ;
 p->nch = nchs ;
 p->nf = nfilt ;
 p->bsize = n ;
 p->buff = &(p->dbuff) ;
 p->boffs = 0 ;
 p->ovl = 0 ;

 p->jlist = NULLJOB ;
 id = CFG_register((JOB_Fxn)FILT_proc,FILT_attach,p) ;
 p->id = id ;
 p->src = NULLID ;
 return(id) ;
}


int		FILT_close(int id)
{
 // free the control structure
// FMEM_free(CFG_getstate(id)) ;
 return(OK) ;
}


int		FILT_attach(int id, int downstr_id, int nice)
{
 FILT_Obj 	*p = (FILT_Obj *)CFG_getstate(id) ;

 if(nice==SRCNICE) {		// this is a SRC id notification call
	 p->src = downstr_id ;
    return(FILT_meta(id)) ;		// post the metadata for this instance
	 }

 return(JOB_add(&(p->jlist),CFG_getprocfxn(downstr_id),CFG_getstate(downstr_id),nice)) ;
}


void	   FILT_remove(int id, int downstr_id)
{
 FILT_Obj 	*p = (FILT_Obj *)CFG_getstate(id) ;
 JOB_remove(&(p->jlist),CFG_getprocfxn(downstr_id)) ;
}


int		FILT_proc(Ptr pt, DATA_Obj *d)
{
 // called when there is data to work with
 // the pointer must point to an initialized FILT_Obj

 FILT_Obj *p = (FILT_Obj *)pt ;
 DATA_Obj *dout ;
 int	nin ;

 if(p->jlist==NULLJOB) {		// no downstream jobs attached - nothing to do
    DATA_free(d) ;
    return(OK) ;
    }

 if((nin=d->nsamps)==0) {
    // zero length input data means a timing error in the source module
    FILT_error(p->id) ;
    DATA_free(d) ;
    return(OK) ;
    }

 // allocate an output data object of the same size as the input data
 if((dout=(DATA_Obj *)DATA_alloc(nin*p->nch))==NULLDATA) {
	 err(FILT_MOD,FILT_ALLOCFAIL) ;
	 return(FAIL) ;
	 }

 // fill out the metadata in the DATA Obj - it is all the same except
 // for the id
 //memcpy(&(dout->rtime),&(d->rtime),sizeof(DATA_Obj)-5) ;
 DATA_CPYALLMETA(dout,d) ;
 dout->id = p->id ;

 // do the filtering
 p->ovl = filtmc((int *)(d->p),(int *)(dout->p),p,nin) ;

 // post the results and free the input data structure
 DATA_POST(&(p->jlist),dout) ;		// post the jobs attached
 DATA_free(d) ;
 return(OK) ;
}


DATA_Obj	*FILT_embedproc(Ptr pt, DATA_Obj *din)
{
 // called when there is data to work with
 // the pointer must point to an initialized FILT_Obj

 DATA_Obj *dout ;
 FILT_Obj *p = (FILT_Obj *)pt ;
 int	  nin = din->nsamps ;

 // allocate an output data object of the same size as the input data
 if((dout=(DATA_Obj *)DATA_alloc(nin*p->nch))==NULLDATA) {
	 err(FILT_MOD,FILT_ALLOCFAIL) ;
	 return(NULLDATA) ;
	 }

 // fill out the metadata in the DATA Obj - it is all the same except
 // for the id
 DATA_CPYALLMETA(dout,din) ;
 dout->id = p->id ;

 // do the filtering
 p->ovl = filtmc((int *)(din->p),(int *)(dout->p),p,nin) ;
 return(dout) ;
}


int		FILT_meta(int id)
{
 char		s[20] ;
 FILT_Obj 	*p = (FILT_Obj *)CFG_getstate(id) ;

 // setup the metadata
 INFO_new(id,NULL,p->src,NULLID) ;
 INFO_add(id,"PROC",NULL,"FILT") ;
 sprintf(s,"%d",p->nf) ;
 INFO_add(id,"NFILT",NULL,s) ;
 return(INFO_end(id)) ;			// post the metadata
}


int		FILT_status(int id)
{
 char		s[20] ;
 FILT_Obj 	*p = (FILT_Obj *)CFG_getstate(id) ;

 if(p->ovl) {
	sprintf(s,"ID=\"%d\" OVL=\"1\"",id) ;
	INFO_event("FILT",s,NULL) ;
	p->ovl = 0 ;				// reset the overload indicator
	}

 return(OK) ;		
}


void	    FILT_error(int id)
{
 // Flush the filter states and send a zero-length buffer downstream
 FILT_Obj 	*p = (FILT_Obj *)CFG_getstate(id) ;

 DATA_passnull(&(p->jlist),id) ; // pass 0-length data to the downstream jobs
 memset(&(p->buff),0,p->bsize) ; // re-initialize the filter states
}
