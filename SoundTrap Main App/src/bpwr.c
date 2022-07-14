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

/*! \file bpwr.c
    \brief Block-oriented power averager.
*/

#include "d3defs.h"
#include "d3std.h"
#include "modnumbers.h"
#include "error.h"
#include "dmem.h"
#include "job.h"
#include "data.h"
#include "flsh.h"
#include "timr.h"
#include "cfg.h"
#include "info.h"
#include "bpwr.h"

#define	BPWR_BIG	(0x7fffffff)
#define BPWR_STARTUP	(4)

typedef	struct {
		 long		 *fast ;	// place to write MA power estimate
		 long		 *slow ;	// place to write AR power estimate
		 long		 pacc ;		// extended resolution accumulator for slow power estimator
		 long		 med[2] ;	// buffer for length-3 median filter
		 int		 tcshft ;	// log2 of exponential window coefficient for slow average
		 int		 startup ;	// countdown for the slow estimator to skip the startup transient of the fast power estimator
		 int		 id ;		// the id of this instance
		 int		 src ;		// the id of the upstream module
		 int		 invn ; 	// inverse of buffer size
		 int		 invshft ;	// shift factor of buffer inverse
		 int		 lastn ;	// last buffer size in samples
		 } BPWR_Obj ;

extern long	pwr_sum(int *in, int n, int shft) ;
extern long	pwr_sumsc(int *in, int n, int shft, int *scf) ;

int		BPWR_meta(int id) ;
int		BPWR_proc(BPWR_Obj *p, DATA_Obj *d) ;
int		BPWR_attach(int id, int downstr_id, int nice) ;
long    med3long(long a, long *p) ;


// Open an instance of the power estimater
int		BPWR_open(int tcshft, long *fast, long *slow)
{
 BPWR_Obj	*cd ;

 if((cd=(BPWR_Obj *)DMEM_alloc(sizeof(BPWR_Obj)))==(BPWR_Obj *)NULL) {
	 err(BPWR_MOD,BPWR_ALLOCFAIL) ;
	 return(NULLID) ;		// allocation error
	 }

 *fast = *slow = BPWR_BIG ;
 cd->fast = fast ;
 cd->slow = slow ;
 cd->pacc = 0 ;
 cd->med[0] = cd->med[1] = BPWR_BIG ;
 cd->tcshft = tcshft ;
 cd->startup = BPWR_STARTUP ;
 cd->lastn = 0 ;
 cd->id = CFG_register((JOB_Fxn)BPWR_proc,BPWR_attach,cd) ;
 cd->src = NULLID ;
 return(cd->id) ;
}


int		BPWR_close(int id)
{
 DMEM_free(CFG_getstate(id)) ;         // free the control structure
 return(OK) ;
}


int		BPWR_attach(int id, int downstr_id, int nice)
{
 BPWR_Obj 	*p = (BPWR_Obj *)CFG_getstate(id) ;

 if(nice!=SRCNICE)		// only act if this is a SRC id notification call
    return(OK) ;

 p->src = downstr_id ;
 return(BPWR_meta(id)) ;		// post the metadata for this instance
}


int		BPWR_proc(BPWR_Obj *p, DATA_Obj *d)
{
 long	pwr ;

 // check for an upstream error
 if(d->size == 0) {
	DATA_free(d) ;	// no need to signal an error - it has already been done
	return(OK) ;
	}

 // check if we already know the inverse of the block size
 if(d->nsamps!=p->lastn) {
	 p->lastn = d->nsamps ;
	 // Amount to shift d->nsamps to make it between 2 and 4.
	 // This is used as a scalar to maintain accuracy in the division.
	 p->invshft = _norm(d->nsamps)-13+BPWR_PSHFT ;
	 p->invn = (int)((1l<<(16+BPWR_PSHFT-p->invshft))/(d->nsamps)) ;
	 }

 // sum power in this block and scale by the block size
 pwr = pwr_sumsc((int *)d->p,d->nsamps,p->invshft,&(p->invn)) ;
 if(pwr<=0) {					// catch bad values
	if(pwr==0)
	   ++pwr ;					// keep pwr greater than zero
	else
	   pwr = BPWR_BIG ;		// full-scale positive for a long
	}

 pwr = med3long(pwr,p->med) ;	 // apply 3-length median filter



 if(p->fast != NULL)		// is a fast power estimate requested?
    *(p->fast) = pwr ;			// if so, store the current result

 if(p->slow != NULL) {
	if(p->startup>0) {
	   --p->startup ;
	   *(p->slow) = pwr ;
	   }

	else {
       // update slow power average - use extended precision for the averaging to capture changes when power is low
       long  pinc, acc = p->pacc ;

	   if((pwr>>1) > *(p->slow))			// constrain maximum upward change in power estimate to 6 dB per update
          acc += *(p->slow) ;			// acc contains the allowable increment in instantaneous power
       else
          acc += pwr-*(p->slow) ;

	   pinc = acc>>(p->tcshft) ;	// the slow power estimator will be incremented by acc>>tcshft
       acc -= pinc<<(p->tcshft) ;	// keep track of bits below the lowest resolution of the slow power estimator
       p->pacc = acc ;				// remember the low-order bits
       pinc += *(p->slow) ;			// now do the slow power update

       if(pinc<0)
       	  pinc = BPWR_BIG ;			// catch 2's complement overflow in the addition

       *(p->slow) = pinc ;
	   }
    }

 DATA_free(d) ;
 return(OK) ;
}


long        med3long(long a, long *p)
{
 // length-3 median filter for iterative calls
 // state is stored in p with the most recent
 // point in *p and the oldest in *(p+1). The
 // new point is in a.
 long       m ;
 
 m = *p ;
 if(*p < *(p+1)) {
    m = *(p+1) ;    // m has the max of the previous inputs
    *(p+1) = *p ;   // *(p+1) has the min
    }

 if(a < m)          // if the new input is less than the max
    m = (a>*(p+1)) ? a : *(p+1) ; // select the largest of a or *(p+1)
                    // otherwise m has the median
 *(p+1) = *p ;      // shuffle the states to eliminate the oldest
 *p = a ;           // and add the new input
 return(m) ;
}


int		BPWR_meta(int id)
{
 char		s[10] ;
 BPWR_Obj 	   *p = (BPWR_Obj *)CFG_getstate(id) ;

 INFO_new(id,NULL,p->src,NULLID) ;      // setup the metadata
 INFO_add(id,"PROC",NULL,"BPWR") ;
 sprintf(s,"%d",p->tcshft) ;
 INFO_add(id,"LOG-TC","UNIT=\"bits\"",s) ;
 sprintf(s,"%d",BPWR_PSHFT) ;
 INFO_add(id,"SHIFT","UNIT=\"bits\"",s) ;
 return(INFO_end(id)) ;			// post the metadata
}
