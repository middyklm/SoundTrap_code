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

/*! \file job.c
    \brief Task queue and scheduler.
*/

#include "d3defs.h"
#include "d3std.h"
#include "protect.h"
#include "board.h"
#include "devdep.h"
#include "misc.h"
#include "modnumbers.h"
#include "error.h"
#include "job.h"

// macros for handling the flags on the nice number
#define		DELETEWHENDONE(j)		j->nice |= 0x4000
#define		ISDELETEWHENDONE(j)	(((j->nice)&0x4000) !=0)

// All of the memory used to store jobs in the job module is locally held.
// Incoming jobs are copied to local versions which are then posted on the
// ready queue. This allows the same job to be posted multiple times (e.g.,
// with different data).

// NJOBS jobs are allocated from a static buffer starting at JOB_QUEUE

static JOB_Obj		JOB_QUEUE[NJOBS] ;

// Note that if sizeof(JOB_Obj) is not even, the C55x compiler will
// put a dummy word between each JOB_Obj so that they start on an 
// even word boundary. 

// pointer to next job to run
static JOB_Obj		*nextrdy = NULLJOB ;

// pointer to next free job
static JOB_Obj		*nextfree = NULLJOB ;

// number of jobs on ready queue
static int			nrdy = 0 ;

int restartOnExit = 1; //default to restart in event of task returning not OK

//******************************************************
// utilities for handling JOB_Obj lists
// these must all be called with interrupts enabled

// allocate a job
static inline JOB_Obj	*job_alloc(void) ;

// free a job
static inline void	job_free(JOB_Obj *j) ;

// add a job to a list
int	job_addj(JOB_List *list, JOB_Fxn f, Ptr state, uns nice, Ptr data) ;

// remove a job from a list
JOB_Obj	*job_removej(JOB_List *list, JOB_Fxn f) ;

// find the job at the end of a list
static inline JOB_Obj	*job_findend(JOB_Obj *list) ;

// count the number of jobs on a list
//static inline int	job_listlen(JOB_Obj *list) ;

//******************************************************


int		JOB_init(void)
{
 // this only needs to run once before the scheduler is first run
 int		 k ;
 JOB_Obj	 *j = JOB_QUEUE ;

 GO_ATOMIC ;

 for(k=0;k<NJOBS-1;k++,j++)	// for each job in the free queue
	j->next = j+1 ;			// link the current job to the next one

 // last job in the free list has a null link pointer
 j->next = NULLJOB ;

 // the ready queue is empty
 nextrdy = NULLJOB ;
 nrdy = 0 ;

 // the free list starts at JOB_QUEUE - from now on jobs can be posted
 // even if the scheduler is not running.
 nextfree = (JOB_Obj *)JOB_QUEUE ;
 END_ATOMIC ;
 return(OK) ;
}


int		JOB_scheduler(void)
// maintain a list of jobs that are ready to run and run each in turn
// freeing completed jobs. The scheduler terminates when there are no 
// more jobs on the list or if a job returns with an error.
{
 int		ret = OK ;
 JOB_Obj	*curr ;

 GO_ATOMIC ;				// disable interrupts while we are messing with the queues
 while(nextrdy != NULLJOB && ret==OK) {	// while there is a job to do
	 
	 curr = nextrdy ;			// get the next job to run

	 if(curr->nice != IDLENICE) {		// if it is an idle job, keep it on the queue
		 nextrdy = curr->next ;			// otherwise pop the current job off the queue
		 --nrdy ;							// decrement the ready job count
		 }

	 END_ATOMIC	;							// re-enable interrupts


	 if(curr->run != (JOB_Fxn)NULL)			// if the function pointer is bona fide
		 ret = (curr->run)(curr->state,curr->data) ;		 // run the current job

	 GO_ATOMIC ;					// disable interrupts while we are messing with the queues
	 if(curr->nice != IDLENICE)		// if the job is not an idle job, free it
		 job_free(curr) ;

	 }

 // free any jobs remaining on the ready queue including idle jobs
 JOB_freelist(nextrdy) ;
 nextrdy = NULLJOB ;
 nrdy = 0 ;						// ready queue should now be empty
 END_ATOMIC	;					// re-enable interrupts
 return(ret) ;
}


int	JOB_post(JOB_List *list, Ptr data)
{
 // add a list of jobs to the ready queue. Each job will be run 
 // only once and then freed. The new jobs are added in the 
 // correct nice order. If a job is flagged RUNONCE,
 // it is deleted from the job list. Returns the number of jobs
 // posted

 int		ret=0 ;
 JOB_Obj	*j, *jlast, *jj ;

 if(list == NULL)
	return(ret) ;

 GO_ATOMIC ;
 j = *list ;
 jlast = NULLJOB ;

 while(j!=NULLJOB) {
	if(job_addj(&nextrdy, j->run, j->state, j->nice & NICEMASK, data)!=OK)
	   break ;

	++ret ;
	if(j->nice & RUNONCE) {
	   jj = j ;
	   j = j->next ;
	   if(jlast == NULLJOB)
		  *list = j ;
	   else
	      jlast->next = j ;

	   job_free(jj) ;
	   }

	else {
 	   jlast = j ;
	   j = j->next ;
	   }
	}

 nrdy += ret ;
 END_ATOMIC ;
 return(ret) ;
}


int	JOB_postone(JOB_Fxn f, Ptr state, uns nice, Ptr data)
{
 // add a job to the ready queue. The job will be run only once and then
 // freed. The new job is added in the correct nice order.
 int	ret ;

 // if there is no new job just return
 if(f == (JOB_Fxn)NULL)
	 return(OK) ;

 GO_ATOMIC ;
 ++nrdy ;			// increment the ready job count
 ret = job_addj(&nextrdy, f, state, nice&NICEMASK, data) ;
 END_ATOMIC ;
 return(ret) ;
}


int	JOB_add(JOB_List *list, JOB_Fxn f, Ptr state, uns nice)
{
 // add a job to a job list in the correct nice order.
 int	ret=OK ;

 // if there is no new job just return
 if(f == (JOB_Fxn)NULL)
	 return(OK) ;

 GO_ATOMIC ;
 ret = job_addj(list, f, state, nice, NULL) ;
 END_ATOMIC ;
 return(ret) ;
}


void	JOB_remove(JOB_List *list, JOB_Fxn f)
{

 // Remove a job from a list and free the job. This is used to 
 // detach a data processing module from a data source module.
 // Return a pointer to the starting job on the list which may have
 // changed if the job removed was the first on the list.
 JOB_Obj	*j ;

 GO_ATOMIC ;
 j = job_removej(list, f) ;
 if(j != NULLJOB)
    job_free(j) ;

 END_ATOMIC ;
}


void	JOB_cancel(JOB_Fxn f, int which)
{

 // remove a job or all instances of a job from the ready queue. 
 // This is used to prevent execution of a job that was previously posted on 
 // the ready queue. In case the same function appears in multiple jobs, 
 // argument 'which' selects between KILLFIRST, or KILLALL.
 JOB_Obj	*j ;

 GO_ATOMIC ;
 while((j = job_removej(&nextrdy, f))!=NULLJOB) {
    job_free(j) ;
	if(which == KILLFIRST)
	   break ;
	}

 END_ATOMIC ;
}


void	JOB_freelist(JOB_Obj *list)
{
 // JOB_freelist frees all jobs from a job list

 JOB_Obj *endj ;

 if(list == NULLJOB)
	 return ;

 GO_ATOMIC ;
 endj = job_findend(list) ;	// find the last job in the list
 endj->next = nextfree ;		// chain the last job to the start of the free list
 nextfree = list ;				// the free list now starts with the first job
 END_ATOMIC ;
}


int		JOB_status(int mode)
{
 // report the status of the job list and free job dump
 #ifdef JTAG
  int		nfree ;

  GO_ATOMIC ;
  nfree = job_listlen(nextfree) ;
  printf("Ready jobs: %d, Free jobs: %d\n", nrdy, nfree) ;
  if(mode || nrdy>10) {
	 // give full report
 	 int		k ;
	 JOB_Obj	*j ;
	 j = nextrdy ;
 	 for(k=1;j!=NULLJOB;k++) {
		printf(" Job %d: fxn 0x%lx, nice %d\n",k,(long)j->run,j->nice) ;
	 	j = j->next ;
		}
	 }

  END_ATOMIC ;
 #endif

 return(OK) ;
}


int		JOB_terminator(Ptr s, Ptr d)
{
 // put the terminator job on the JOB list to end the scheduler
 #ifdef JTAG
  printf("terminator job\n") ;
 #endif
 restartOnExit = 0;
 return(FAIL) ;
}


//*********************************************************
// utilities for handling JOB_Obj lists
// these must all be called with interrupts enabled

// action to take when there are no more jobs to allocate
void	job_allocfail(void) ;

// find the job at the end of a list
static inline JOB_Obj	*job_findend(JOB_Obj *list)
{
 while(list->next!=NULLJOB)
	 list = list->next ;

 return(list) ;
}

/*
// count the number of jobs on a list
static inline int job_listlen(JOB_Obj *list)
{
 int		k ;

 if(list == NULLJOB)
	 return(0) ;

 for(k=1;list->next!=NULLJOB;k++)
	 list = list->next ;

 return(k) ;
}
*/

static inline void	job_free(JOB_Obj *j)
{
 j->next = nextfree ;
 nextfree = j ;		// chain the job to the start of the free list
}


static inline JOB_Obj	*job_alloc(void)
{
 JOB_Obj *j ;

 // if there are no jobs or one job left
 if(nextfree == NULLJOB || nextfree->next == NULLJOB) {
	job_allocfail() ;
	return(NULLJOB) ;
	}

 j = nextfree ;
 nextfree = j->next ;
 return(j) ;
}


void	job_allocfail(void)
{
	fatal(JOB_MOD,JOBALLOCFAIL) ;	// this is a fatal error - need to restart
}


int		job_addj(JOB_List *list, JOB_Fxn f, Ptr state, uns nice, Ptr data)
{
 // add a job to a list. This is used to attach data processing
 // modules to a data source module. The new job is added in the correct
 // position to maintain increasing NICE number in the blocked list.
 // Returns a pointer to the head of the job list which may change from the
 // input argument if the new job is added at the start of the list.

 JOB_Obj	*lastj=NULLJOB, *nextj, *newj ;
 uns		newnice ;

 // get a job blank and make sure the allocation worked
 if((newj = job_alloc())==NULLJOB)
	 return(FAIL) ;

 // initialize the job structure
 newj->run = f ;
 newj->state = state ;
 newj->nice = nice ;
 newj->data = data ;
 newnice = nice & NICEMASK ;

 // setup pointers to work down the job list
 nextj = *list ;

 while(1) {			 // find where the new job fits in the list

	 // check if the new job fits at the current location - i.e., after lastj
	 // and before nextj. This will be the case if there is no next job or the
	 // new job has a lower NICE number than the job at nextj.
	 if((nextj == NULLJOB) || (newnice < nextj->nice)) {

		 if(lastj==NULLJOB)			// if we are still at the start of the queue
			 *list = newj ;				// the new job goes at the head of the queue
		 else
			 // add the new job at the current location
			 lastj->next = newj ;	// add the new job after the current one

		 //lastj = newj ;				// step down the run list
		 //lastj->next = nextj ;		// finish fitting the new job in the linked list
		 newj->next = nextj ;		// finish fitting the new job in the linked list
		 break ;
		 }

	 // if a job doesn't fit here, step down the list
	 else {
		 lastj = nextj ;
		 nextj = nextj->next ;
		 }
	 }

 return(OK) ;
}


JOB_Obj	*job_removej(JOB_List *list, JOB_Fxn f)
{

 // Remove a job from a list and return a pointer to the job.
 // Returns a null pointer if a matching job is not found 

 JOB_Obj	*nextj, *lastj=NULLJOB ;

 // use a pair of pointers to scan through the list to find a matching job.
 // nextj points to the next job. lastj points to the previous one.
 nextj = *list;

 while(nextj != NULLJOB) {		// while there are still jobs to check...

	 // does the next job on the list match the search function?
	 if(nextj->run == f) {

		 // if so, remove the job from the list
		 if(lastj == NULLJOB)		// if job to be deleted is first on the list
			 *list = nextj->next ;	// adjust the list start pointer

		 else						// otherwise, just remove the job and re-make the link
			 lastj->next = nextj->next ;

		 break ;
		 }

	 // if the job is not a match, step down the list
	 else {
		 lastj = nextj ;
		 nextj = nextj->next ;
		 }
	 }

 return(nextj) ;
}

