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

/*! \file cdet.c
    \brief Matched-filter based click/transient sound detector.
*/


#include <tistdtypes.h>
#include "uart.h"
#include "d3defs.h"
#include "d3std.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "modnumbers.h"
#include "error.h"
#include "dmem.h"
#include "job.h"
#include "data.h"
#include "flsh.h"
#include "timr.h"
#include "filt.h"
#include "cfg.h"
#include "info.h"
#include "cdet_v2.h"
#include "bpwr.h"
#include "blkpwr.h"


typedef	struct {
         Ptr         filtobj ;   // information for the embedded filter
         DATA_Obj    *din ;      // last filter input data
         DATA_Obj    *dout ;     // last filter output data
		 DATA_Timekey	tkey ;	 // structure for time calculation
         JOB_Obj	 *jlist ;    // list of downstream jobs requesting detection outputs
         long        *pwr ;      // location to find the current noise power estimate
         BLKPWR_Obj	 det ;		 // structure for the detector
         CDET_Data   *dv[CDET_MAXDETS] ; // pointers to in-progress detection vectors
         CDET_Params p ;         // detection parameters
         float		 rthr ;		 // threshold multiplier
		 int         blanktogo ; // current blanking countCdetv2down
         int         pause ;     // enable or disable detection
         int         src ;       // upstream data source id
         int         id ;        // id of this instance
         int         fid ;       // id of the embedded filter
		 } CDET_Obj ;
         
// an assembly language function in blkpwr_asm.s5x is used to find the
// first instantaneous power in a vector above a threshold
extern int	find1stpwr(long thr, int *in, int skip, int n) ;

// an assembly language function in blkpwr_asm.s5x is used to find the
// first block power in a vector above a threshold
extern int	find1stblkpwr(BLKPWR_Obj *p, int *in, int n, long thr) ;
extern int	find1stblkchk(BLKPWR_Obj *p, int *in, int n, long thr) ;

// internal processing function called by upstream processors
int      CDET_proc(Ptr s, DATA_Obj *din) ;

// internal error function
void	 CDET_error(int id) ;

// publish metadata
int		 CDET_meta(int id) ;

CDET_Data *CDET_alloc(CDET_Obj *cd, DATA_Obj *din, int k) ;
void	CDET_flush(CDET_Obj *p) ;

inline int     freedata(CDET_Obj *cd, DATA_Obj *din, DATA_Obj *dout) ;
inline int   newdet(CDET_Obj *cd, int k, DATA_Obj *din, DATA_Obj *dout, long np) ;
inline void   updatedet(CDET_Obj *cd, DATA_Obj *din, DATA_Obj *dout) ;
int		add2det(CDET_Data *dv, int k, DATA_Obj *din, DATA_Obj *dout) ;
void 	postdet(CDET_Obj *cd, CDET_Data *dv) ;



// Open an instance of the click detector
int		CDET_open(DATA *filt, int nfilt, long *nl, CDET_Params *p)
{
 CDET_Obj	*cd ;

 if((cd=(CDET_Obj *)DMEM_alloc(sizeof(CDET_Obj)))==(CDET_Obj *)NULL) {
	 err(CDET_MOD,CDET_ALLOCFAIL) ;
	 return(NULLID) ;		// allocation error
	 }

 memset(cd,0,sizeof(CDET_Obj)) ;    // zero states and pointers
 cd->pwr = nl ;
 memcpy((int *)&(cd->p),(int *)p,sizeof(CDET_Params)) ;   // copy the detection parameters
 DATA_INITTKEY(&(cd->tkey)) ;
 cd->fid = FILT_open(filt,nfilt,1) ;
 BLKPWR_init(&(cd->det),p->npwr) ;
 cd->rthr = (float)cd->p.npwr * (float)cd->p.rthresh * pow(2.0,-BPWR_PSHFT) ;   // the relative power threshold
 cd->id = CFG_register((JOB_Fxn)CDET_proc,CDET_attach,cd) ;
 cd->src = NULLID ;
 cd->filtobj = CFG_getstate(cd->fid) ;

 return(cd->id) ;
}


int		CDET_close(int id)
{
 CDET_Obj 	*p = (CDET_Obj *)CFG_getstate(id) ;
 
 FILT_close(p->fid) ;   // free the underlying filter
 CDET_flush(p) ;
 DMEM_free(p) ;         // free the control structure
 return(OK) ;
}


void     CDET_pause(int id, int on)
{
 // prevent or enable detection
 CDET_Obj 	*p = (CDET_Obj *)CFG_getstate(id) ;
 p->pause = on ;
 if(!on) {
 	p->blanktogo = 0 ;
 	CDET_flush(p) ;
 	BLKPWR_clear(&(p->det)) ;
 	}
}


CDET_Data *CDET_alloc(CDET_Obj *cd, DATA_Obj *din, int k)
{
 // Create an object for a detection output. This contains:
 //    1. a control structure of type CDET_Data
 //    2. one or two buffers of length len

 CDET_Data	*d ;
 DATA_Obj	*p1, *p2 ;
 int		n ;

 // allocate the control structure
 if((d=(CDET_Data *)DMEM_alloc(sizeof(CDET_Data)))==NULLPTR)
	return(NULL) ;

 n = cd->p.npre + cd->p.npost ; 	// size of the output vectors will be npre+npost.

 // allocate DATA Objects within the CDET_Data structure
 if((p1=DATA_alloc(n))==NULLDATA) {
	DMEM_free(d) ;
	return(NULL) ;
	}

 // setup the metadata
 p1->fs = din->fs ;
 p1->nch = 1 ;
 p1->id = cd->id ;
 p1->nsamps = p1->size = n ;
 DATA_settime(&(cd->tkey),p1,din,k) ;
 d->din = p1 ;
 d->dout = NULLDATA ;
 d->q = n ;         	// number of samples yet to fill in the DATA_Objs
 d->offset = 0 ;    	// current offset in the DATA_Objs
 d->sema = 0 ;

 if(cd->p.storeout) {
 	if((p2=DATA_alloc(n))==NULLDATA) {
 	   DATA_free(p1) ;
	   DMEM_free(d) ;
	   return(NULL) ;
	   }

	DATA_CPYALLMETA(p2,p1) ;
 	d->dout = p2 ;
 	}

 return(d) ;
}


void     CDET_free(CDET_Data *d)
{
 DATA_free(d->din) ;
 DATA_free(d->dout) ;
 if(--(d->sema) <= 0)
    DMEM_free(d) ;
}


void	CDET_flush(CDET_Obj *p)
{
 int	k ;
 for(k=0;k<CDET_MAXDETS;k++)
    if(p->dv[k] != NULL)      	// free any pending detection vectors
       CDET_free(p->dv[k]) ;
}


int		CDET_attach(int id, int downstr_id, int nice)
{
 CDET_Obj 	*p = (CDET_Obj *)CFG_getstate(id) ;

 if(nice==SRCNICE) {			// this is a SRC id notification call
	p->src = downstr_id ;
    return(CDET_meta(id)) ;		// post the metadata for this instance
	}

 return(JOB_add(&(p->jlist),CFG_getprocfxn(downstr_id),CFG_getstate(downstr_id),nice)) ;
}


void	   CDET_remove(int id, int downstr_id)
{
 CDET_Obj 	*p = (CDET_Obj *)CFG_getstate(id) ;
 JOB_remove(&(p->jlist),CFG_getprocfxn(downstr_id)) ;
}


// This is the processing engine of the click detector
int      CDET_proc(Ptr s, DATA_Obj *din)
{
 // clickdet is implemented as a wrapper function to an embedded FILT
 // instance doing the matched filtering.
 // This function checks if the filter output is above a threshold
 // and, if so, assembles a vector of unfiltered samples around 
 // the threshold crossing to pass to downstream functions. It only
 // posts the downstream jobs when it has a detection to report and a block
 // of input samples around the detection point for analysis.

 int     *dp, nsamps, k = 0 ;
 long    np, thr ;
 DATA_Obj	*dout ;
 CDET_Obj	*cd = (CDET_Obj *)s ;



 // run the embedded filter - do this irrespective of whether the detector is paused or not
 // to keep the filter state and complete any current detections
 if((dout=FILT_embedproc(cd->filtobj,din))==NULLDATA)
    return(FAIL) ;                      // in case of a memory allocation fail

 updatedet(cd,din,dout) ;        // pass the input and filtered vectors to the extractor
                                 // in case there are any unfinished detections
 if(cd->pause) {                  // if detection is paused
 	freedata(cd,din,dout) ;
    return(OK) ;                 // just return
 	}

 // work out what detection threshold to use - it is the current noise power estimate
 // times the relative threshold.
 
 np = MAX(*(cd->pwr),(long)cd->p.minnp) ;     // get the current noise power estimate
 thr = (long)((float)np * cd->rthr) ;   // the absolute power threshold
 dp = (int *)dout->p ;      			// where to start looking for a detection
 nsamps = dout->nsamps ;

 do {
    if(cd->blanktogo > 0) {     // are we still within the blanking time of a detection?
       int m = MIN(nsamps-k,cd->blanktogo) ;      // if so, adjust the pointer to the end of the blanking time
       cd->blanktogo -= m ;                     // or the end of the block, whichever is first
       k += m ;
       }

	if(k<nsamps) {    		// blanking time is over - look for a detection
       #ifdef THIS_IS_STILL_TO_DO
        if(cd->type == CDET_TYPE_MF) {
           k += find1stpk(cd->det,dp+k,nsamps-k,thr) ;}          // find the first signal level above thr
        else
       #endif
       k += find1stblkpwr(&(cd->det),dp+k,nsamps-k,thr) ;      // find the first block-summed power above thr
     //BLKPWR_clear(&(cd->det)) ;             // re-initialize the block power detector
     //find1stblkpwr(&(cd->det),dp,1000,thr) ;      // find the first block-summed power above thr
     //   k=nsamps;
       }

	// k=nsamps if there is no detection in the current block

	if(k<nsamps) {                                    // if there is a detection in the current block...
	   cd->blanktogo = cd->p.nblank ;      // set the blanking countCdetv2down
       BLKPWR_clear(&(cd->det)) ;             // re-initialize the block power detector
 	   #ifdef JTAG
	 //   printf("det %d %ld %d %d\n",repn,thr,*(dp+k),k);
	   #endif
       newdet(cd,k,din,dout,np) ;           // start extracting the detection.
       }

    } while(k<nsamps) ;


 freedata(cd,din,dout) ;
 return(OK) ;
}


inline int     freedata(CDET_Obj *cd, DATA_Obj *din, DATA_Obj *dout)
{
 // update the stored data buffers
 if(cd->din != NULL)
    DATA_free(cd->din) ;     // free the oldest input data vector
 if(cd->dout != NULL)
    DATA_free(cd->dout) ;    // free the oldest output data vector
    
 cd->din = din ;          // store the pointer to the newest input data
 if(cd->p.storeout)
    cd->dout = dout ;        // store the pointer to the newest output data
 else {
 	cd->dout = NULLDATA ;
 	DATA_free(dout) ;
 	}

 return(OK) ;
}


inline int   newdet(CDET_Obj *cd, int k, DATA_Obj *din, DATA_Obj *dout, long np)
{
 // Create new data object for the detection and fill it with data up to the
 // end of the current data set.
 // If the detection object is complete, pass it to the downstream modules.
 // The output vectors will contain:
 // npre integer samples prior to the detection
 // npost integer samples after and including the detection
 // The time value in the metadata will be for the first sample in
 // the detection vector, not for the detection moment which happens npre 
 // samples later (this is given by the offset element of the CDET_Data structure).
 
 CDET_Data   *dv ;
 int		 kk, stk ;

 for(kk=0;kk<CDET_MAXDETS && (dv=cd->dv[kk])!=NULL; kk++) ;
 if(dv != NULL)
    return(FAIL) ;           // no free detection slot - skip this detection

 // create detection vector structure
 if((dv=CDET_alloc(cd,din,k))==NULL) {   // allocate a detection data object
	 err(CDET_MOD,CDET_ALLOCFAIL) ;
	 return(FAIL) ;
	 }

 dv->nl = np ;
 stk = k - cd->p.npre ;         // first sample to copy into the detection vector

 // start copying the pre-detection data to the detection vectors
 if(stk<0) {				// check if the start is in the previous buffer
    stk += cd->din->nsamps ;
    add2det(dv,stk,cd->din,cd->dout) ;
	stk = 0 ;
	}
 
 if(add2det(dv,stk,din,dout)<=0) {    // if the detection vector is now full
 	postdet(cd,dv) ;
    dv = NULL ;                      // free the detection vector
    }

 cd->dv[kk] = dv ;                   // put the detection vector in the slot
 return(OK) ;
}


inline void   updatedet(CDET_Obj *cd, DATA_Obj *din, DATA_Obj *dout)
{
 // add new data to any incomplete detection vectors
 int  k ;
 CDET_Data    *dv ;
 
 for(k=0;k<CDET_MAXDETS;k++)
    if(((dv = cd->dv[k])!= NULL) && (add2det(dv,0,din,dout)<=0)) {
       // if the detection vector is full - post any attached jobs
 	   postdet(cd,dv) ;
       cd->dv[k] = NULL ;
       }
}


void 	postdet(CDET_Obj *cd, CDET_Data *dv)
{
 int	n ;
 dv->q  = -1 ;                      // restore the q and offset data
 dv->offset = cd->p.npre ;
 n = JOB_post(&(cd->jlist),(Ptr)dv) ; // post any attached jobs
 dv->sema = n ;
 DATA_SETSEMA(dv->din,n) ;
 if(dv->dout != NULL)
    DATA_SETSEMA(dv->dout,n) ;
}


int   add2det(CDET_Data *dv, int k, DATA_Obj *din, DATA_Obj *dout)
{
 int n = MIN(din->nsamps-k,dv->q) ;          // number of samples to copy

 memcpy((int *)(dv->din->p)+dv->offset,(int *)din->p+k,n) ;
 memcpy((int *)(dv->dout->p)+dv->offset,(int *)dout->p+k,n) ;
 dv->offset += n ;
 dv->q -= n ;
 return(dv->q) ;
}

       
int		CDET_meta(int id)
{
 char		s[20] ;
 CDET_Obj 	*cd = (CDET_Obj *)CFG_getstate(id) ;

 INFO_new(id,NULL,cd->src,NULLID) ;      // setup the metadata
 INFO_add(id,"PROC",NULL,"CDET") ;
 sprintf(s,"%d",cd->p.rthresh) ;
 INFO_add(id,"DETTHR","TYPE=\"relative\"",s) ;
 sprintf(s,"%d",cd->p.nblank) ;
 INFO_add(id,"BLANKING","UNIT=\"samples\"",s) ;
 sprintf(s,"%d",cd->p.npre) ;
 INFO_add(id,"PREDET","UNIT=\"samples\"",s) ;
 sprintf(s,"%d",cd->p.npost) ;
 INFO_add(id,"POSTDET","UNIT=\"samples\"",s) ;
 sprintf(s,"%d",cd->p.npwr) ;
 INFO_add(id,"LEN","UNIT=\"samples\"",s) ;
 sprintf(s,"ID=\"%d\"",cd->fid) ;
 INFO_add(id,"USING",s,NULL) ;
 INFO_end(id) ;			// post the metadata
 return(FILT_meta(cd->fid)) ;
}


void	    CDET_error(int id)
{
 CDET_Obj 	*p = (CDET_Obj *)CFG_getstate(id) ;
 FILT_error(p->fid) ;
}
