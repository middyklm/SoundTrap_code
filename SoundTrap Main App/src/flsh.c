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

/*! \file flsh.c
    \brief file-based FLASH memory interface.
    Currently only supports multi-user writes to a single file.
*/

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
#include "data.h"
#include "flsh.h"
#include "cfg.h"
#include "info.h"
#include "fs.h"
#include "crc.h"


#define		REOPEN_INSTR	(0x1000)

// write queue is a linked list of FLSH_Job objects
struct FLSH_JobLink {
		DATA_Obj	*data ;			   // data object currently being read / written
		JOB_Fxn		callback ;		// list of jobs attached for callback
		Ptr			state ;			// state for callback
		struct FLSH_JobLink *next ;	// next link in the list	
		} ;

typedef struct FLSH_JobLink	FLSH_Job ;

// FIFO job queue of data to be written to the flash memory
static FLSH_Job		*jqhead ;	// head of queue is the next job to do
static FLSH_Job		*jqtail ;	// tail of queue is the entry point to the list
static FLSH_Job		*jqcurr ;	// current job being run

static JOB_List		end_jlist = NULLJOB ;		// callbacks for end-of-flash
static JOB_List		nearend_jlist = NULLJOB ;	// callbacks for near-end-of-flash

int		jlist_len(void) ;
FLSH_Job	*pop_jlist(void) ;
int		push_jlist(FLSH_Job *j) ;


#define	WRITE_DONE		(0)
#define	WRITE_DATA		(1)
static int	wstate ;					// state of the writing machine
static long	wordsremaining ;			// number of words to write before a re-open
static long	samplesremaining ;			// number of samples to write before a re-open

// chunk header handlers
#define	FLSH_CHLEN				(10)		// chunk header length in words
static uns	chdr[FLSH_CHLEN] ;			// preparation area for the chunk header
uns		*makechunkhdr(DATA_Obj *d) ;

// configuration chunk (metadata) handlers
int	flsh_postcfgs(void) ;

// overall module initialization lock
static int	flshinit = 0 ;

// write file lock - set to 1 when the file is open and ready to write
static volatile int	wfileopen = 0 ;

FlashCallbackHandler fccb = NULL; //flash closing callback
FlashCallbackHandler nfcb = NULL; //flash new file callback

int	FLSH_init(void)
{
 flshinit = 0 ;

 switch(fsInit()) {
	 case FAIL_FS_FULL: shutdown(); //file system is full, shutdown
 	 case FAIL: return(FAIL) ;
 	 default:break;
 }

 // setup the job queue
 jqhead = jqtail = jqcurr = (FLSH_Job *)NULL ;
 wstate = WRITE_DONE ;
 wfileopen = 0 ;

 flshinit = 1 ;
 return(OK) ;
}


int	FLSH_open(void)
{
 // start a new recording
 if(!flshinit || wfileopen!=0) {
	 err(FLSH_MOD,BADFILESTATE) ;
	 return(FAIL) ;
	 }

 wfileopen = 1 ;
 wordsremaining = FLSH_MAXFILESIZE>>1 ;
 samplesremaining = FLSH_MAXSAMPLES ;

 INFO_event("START","STATE=\"NEW\"",NULL) ;
 // request metadata
 if(nfcb != NULL) nfcb();

 INFO_requestall() ;

 return(OK) ;
}


int	FLSH_reopen(char *reason)
{
 // start a new recording
 if(!flshinit || !wfileopen) {
	 err(FLSH_MOD,BADFILESTATE) ;
	 return(FAIL) ;
	 }

 // reset the size and samples counters - this has to happen
 // before any more posts otherwise FLSH_reopen will be 
 // re-called by FLSH_write
 wordsremaining = FLSH_MAXFILESIZE>>1 ;
 samplesremaining = FLSH_MAXSAMPLES ;

 if(fccb != NULL) fccb();

 // post an end advisory metadata
 INFO_event("END","STATE=\"REOPEN\"",reason) ;
 // post a flash job with the re-open instruction
 FLSH_write(NULLDATA,(JOB_Fxn)NULL,(Ptr)REOPEN_INSTR) ;

 INFO_event("START","STATE=\"REOPEN\"",NULL) ;

 if(nfcb != NULL) nfcb();

 INFO_requestall() ;	// request metadata	

 return(OK) ;
}


int	FLSH_close(int wait)
{
 // finish writing data to a file and close it 
 if(!flshinit || !wfileopen) {
	 err(FLSH_MOD,BADFILESTATE) ;
	 return(FAIL) ;
 }

 // TODO: signal configurations to flush any data they are holding
 if(fccb != NULL) fccb();

 // post an end advisory metadata
 INFO_event("END","STATE=\"CLOSE\"",NULL) ;

 wfileopen = -1 ;	// set flag to not accept any new data
 if(wait==0)
	return(OK) ;

 fsNewdata();

#ifdef PLOG
 // wait for pending jobs to complete writing
 while(wfileopen!=0) {}; //causing hang on app close
#endif

 fsClose();
 return(OK) ;
}


int	FLSH_write(DATA_Obj *d, JOB_Fxn f, Ptr s)
{
 FLSH_Job	*j ;

 if(!flshinit || wfileopen<=0) {
	#ifdef JTAG
 	 printf("write: %d %d\n",flshinit,wfileopen) ;
	#endif
	err(FLSH_MOD,BADFILESTATE) ;
	return(FAIL) ;
	}

 if(d==NULLDATA && s==NULL)
	return(OK) ;

 // create a flash job
 if((j=(FLSH_Job *)DMEM_alloc(sizeof(FLSH_Job)))==(FLSH_Job *)NULL) {
	err(FLSH_MOD,ALLOCFAIL) ;
	return(FAIL) ;
	}

 if(d!=NULLDATA) {
    DATA_INCSEMA(d) ;
	wordsremaining -= (d->size + FLSH_CHLEN) ;
	samplesremaining -= (d->nsamps * d->nch) ;
	}

 // fill the FLSH_Job elements
 j->data = d ;
 j->callback = f ;
 j->state = s ;
 
 // add the job to the write queue atomically
 // if this is the only job on the queue...
 if(push_jlist(j) && jqcurr==(FLSH_Job *)NULL)
	fsNewdata() ;	// signal that there is new data to write

 // if the file has reached maximum size, force a re-open
 
 if(wordsremaining<=0)
	FLSH_reopen("max size") ;
 else if(samplesremaining<=0)
	FLSH_reopen("max samples") ;
 
 return(OK) ;
}


int	FLSH_status(void)
{
#ifdef JTAG
 if(!flshinit) {
	printf(" flash interface not initialized\n") ;
	return(OK) ;
	}

 flsh_status() ;
 if(!wfileopen) {
    printf(" file not open for writing\n") ; 
	return(OK) ;
	}

 printf(" bytes written %ld\n",FLSH_MAXFILESIZE-(wordsremaining<<1)) ;
 printf(" flsh jobs in queue %d\n", jlist_len()) ;
#endif
 return(OK) ;
}


int		FLSH_attach(int event, JOB_Fxn f, Ptr s, int nice)
{
 JOB_List	*j ;

 j = (event == FLSH_END) ? &end_jlist : &nearend_jlist ;
 return(JOB_add(j,f,s,nice)) ;
}


void	FLSH_remove(int event, JOB_Fxn f)
{
 JOB_List	*j ;

 j = (event == FLSH_END) ? &end_jlist : &nearend_jlist ;
 JOB_remove(j,f) ;
}

//
// Internal functions hereon
//

void	flsh_stop(int signal)
{
 // this is called when the flash is almost or completely written
 // if we are on the last block, post any jobs attached to this callback
 if(signal == FLSH_NEAREND) {
 	INFO_event("ALERT","MESS=\"FLASH FULL\"",NULL) ;
	JOB_post(&nearend_jlist,NULL) ;
	return ;
    }

 wfileopen = 0 ;
 JOB_post(&end_jlist,NULL) ;
}


int	writefrag(FLSH_Buff *buff)
{
 // Second-tier interrupt service routine called by functions in flsh_if module.
 // It is called when new data is needed to write to the flash. The address and
 // amount of data is returned in the buff structure. Any amount of data can be
 // returned. This routine keeps track of what is the next piece of data to offer.
 // going in order from chunk header to chunk data. The routine returns the number
 // of words in the buffer. If it returns 0, the flash writing interface will idle
 // until it is signalled by a call to flsh_newdata() that new data is available
 // or by a signal to flsh_flush() to purge any partial buffer.

 DATA_Obj	*d ;
 int		n ;

 // there are only two states:
 // if wstate == WRITE_DONE at entry, a job has just been finished or new data
 //	has arrived. The previous job is freed and the new job is promoted to
 //   current. The chunk header is returned for writing.
 // if wstate == WRITE_DATA at entry, the chunk header of the current job has
 //   been written and the data segment must now be passed to the buffer
  
 if(wstate == WRITE_DONE) {		// job has been finished or new data has arrived

	if(jqcurr != NULL) {		// a job has been finished
	   // free the last job and its data
	   if(jqcurr->data != NULLDATA) {
		  // data object is done: call the callback, if any, and free the job
		  if(jqcurr->callback != NULL)
			 JOB_postone(jqcurr->callback,jqcurr->state,FLSH_NICE,(Ptr)NULL) ;
		  DATA_free(jqcurr->data) ;	// free the underlying data object
		  }

	   // catch a REOPEN job - the flush has been carried out
	   // now we need to reinitialize the block header
 	   //else if(jqcurr->state != NULL)	// a REOPEN job has a non-null state
 	   //	  flsh_filereopen() ;

	   DMEM_free(jqcurr) ;		// free the job carrier
	   }

	// pop the next job off the list
	if((jqcurr = pop_jlist())==NULL) {	// check if there is new data to write
	   if(wfileopen<0) {				// if no more data, check if there is a file close signal	
		  buff->type = WRITE_FLUSH ;	// if so, request a flush
		  wfileopen = 0 ;				// signal that the last data has been taken
	   }
	   else
		  buff->type = WRITE_NONE ;

	   buff->n = 0 ;		 
	   return(0) ;				// if not signal an idle state 
	   }

	if(jqcurr->data == NULLDATA) {		// catch a REOPEN job
 	   buff->type = (jqcurr->state == NULL) ? WRITE_NONE : WRITE_FLUSH ;
	   buff->n = 0 ;
	   return(0) ;
	   }

	 n = FLSH_CHLEN ;				// first part to write is the chunk header
	 d = jqcurr->data ;
	 buff->p = makechunkhdr(d) ;
	 buff->type = WRITE_CH ;		// signal that this is header data
	 // on the next call, write the chunk data if there is any
	 wstate = (d->size>0) ? WRITE_DATA : WRITE_DONE ;
	 }

 else {								// the chunk header has been written
	 d = jqcurr->data ;				// the next thing to write is the chunk data
	 buff->p = (uns *)(d->p) ;
	 buff->type = WRITE_CD ;		// signal that this is chunk data
	 n = d->size ;					// size of the data in words
	 wstate = WRITE_DONE ;			// the job will be done on the next call
	 }

 buff->n = n ;
 return(n) ;
}


uns	*makechunkhdr(DATA_Obj *d)
{
 chdr[0] = CHUNK_MAGIC ;
 chdr[1] = d->id ;
 chdr[2] = d->size << 1 ;
 chdr[3] = d->nsamps ;
 chdr[4] = d->rtime >> 16 ;
 chdr[5] = d->rtime & 0x0ffff ;
 chdr[6] = d->mticks >> 16 ;
 chdr[7] = d->mticks & 0x0ffff ;
 chdr[8] = crc(d->p , d->size);
 chdr[9] = crc(chdr, 9);
 return(chdr) ;
}



FLSH_Job	*pop_jlist(void)
{
 FLSH_Job	*j ;

 GO_ATOMIC ;
 // pop the next job off the FIFO
 j = jqhead ;
 if(j!=(FLSH_Job *)NULL) {	// if there is another job
	// extract it and point the head of queue to the next job
    jqhead = j->next ;				
	if(jqhead == (FLSH_Job *)NULL)	// if there is no next job
	   jqtail = (FLSH_Job *)NULL ;	// null the queue
	}

 END_ATOMIC ;
 return(j) ;
}


int		push_jlist(FLSH_Job *j)
{
 // add a job to the flash job list
 // return 1 if this is the only job on the queue
 GO_ATOMIC ;
 j->next = (FLSH_Job *)NULL ;

 if(jqhead == (FLSH_Job *)NULL) {	// if there are no jobs waiting
	jqhead = jqtail = j ;
	END_ATOMIC ;
	return(1) ;
	}

 jqtail->next = j ;
 jqtail = j ;
 END_ATOMIC ;
 return(0) ;
}


int		jlist_len(void)
{
 int	n=0 ;
 FLSH_Job	*j ;

 GO_ATOMIC ;
 for(j=jqhead;j!=(FLSH_Job *)NULL && n<100;n++)
	j = j->next ;

 END_ATOMIC ;
 return(n) ;
}


int	flsh_postcfgs(void)
{
 DATA_Obj	*d ;
 int	k, n, err=0 ;

 n = CFG_getidcnt() ;
 for(k=1;k<=n;k++) {
	d = INFO_request(k) ;
	if(d != NULLDATA)
	   err |= FLSH_save(d) ;
	}

 return(err) ;
}

void flsh_regNewFileCallbackHandler( FlashCallbackHandler cb )
{
	nfcb = cb;
}


void flsh_regFileClosingCallbackHandler( FlashCallbackHandler cb )
{
	fccb = cb;
}
