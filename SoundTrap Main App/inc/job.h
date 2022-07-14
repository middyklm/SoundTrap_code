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

/*! \file job.h
    \brief Job queue and scheduler.
    
    Contains the real-time scheduler, the ready job queue and tools for
    adding and removing jobs to/from the queue.

To use this module, #include "job.h" in the module that will call the JOB 
functions. The job module must be initialized before use using JOB_init(). 
This should be done in the main function.

The D3 uses a simple non-preemptive real-time executive to schedule tasks. 
The process is data driven in that tasks are blocked until new data becomes 
available. When there is new data, each function that is waiting for the 
data is posted to the ready queue. Functions on the ready queue are executed 
in order of priority (i.e., in order of increasing NICE number). Being a 
non-preemptive scheduler, once a task is scheduled for execution, it runs to 
completion and so must not block while waiting for other data or resources. 
When a task is complete, it is removed from the ready queue and will not run 
again until new data becomes available and the task is re-posted on the ready
queue.
*/

#ifndef JOB_H_
#define JOB_H_

/*! Error types
*/
#define		JOBALLOCFAIL		(0)		// error trying to alloc a new job
#define		LASTJOB				(1)		// last job allocated
#define		DMEMALLOCFAIL		(2)
#define		FLASH_WRITE_FAIL	(3)

/*! Define a sentinel so that other modules can test if job scheduling is 
	being included in the build.
*/
#define		_JOB

/*! Maximum number of jobs that can be defined at a time. Use JOB_status
    to see how many are actually being used. The job objects are statically
    defined during initialization to speed up job creation. */
#define		NJOBS		(70)

/*! pointer to an undefined job */
#define		NULLJOB				((JOB_Obj *)NULL)

/*! prototype for job functions to be run from the scheduler, i.e.,
    int	fxn(Ptr state, Ptr data) ; 
    Every job that the scheduler runs must conform to this prototype.
    What the pointers point to is arbitrary. The function returns OK
    if processing can continue or FAIL if the scheduler must terminate */
typedef	int (*JOB_Fxn)(Ptr,Ptr) ;


typedef struct {
				Ptr		state ;		// private information for the job
				JOB_Fxn	f ;			// function to run when the job is scheduled
				uns		nice ;		// nice number used to assign priority
				} JOB_JobSpec ;


struct JOB_Str {
				Ptr		state ;		// private information for the job
				JOB_Fxn	run ;			// function to run when the job is scheduled
				Ptr		data ;		// pointer to data to pass to the job when it runs
				struct JOB_Str	*next ;	// next job in the blocked or ready queue
				uns		nice ;		// nice number used to assign priority
				int		dummy ;		// sizeof(JOB_Obj) should be even!!
				} ;

typedef	struct JOB_Str	JOB_Obj ;
typedef JOB_Obj	*JOB_List ;


/*! The highest priority nice number */
#define		MAXPRI			(0)
/*! The lowest normal priority nice number */
#define		MAXNICE			(126)
/*! The nice number of the idle task. This task is always on the ready queue */
#define		IDLENICE			(127)

// adding a RUNONCE flag to the nice number means that the job will be
// removed from the blocked queue when it is posted to the ready queue.
#define		RUNONCE			(0x4000)
#define		NICEMASK			(0x00ff)

// Which jobs to remove on the ready list: first instance, last instance
// or all instances
#define		KILLNONE		(0)
#define		KILLFIRST		(1)
#define		KILLALL			(2)


extern int restartOnExit;


/*! \brief Initialize the job memory pool and scheduler.

   JOB_init prepares the run list and a static heap of job blanks for
   use by other functions when creating jobs. It should be run only once
   and must be run before any other job-handling functions are called.
*/
extern int		JOB_init(void) ;

/*! \brief The job scheduler.

   JOB_scheduler runs jobs on the ready queue until all jobs are run
   or a job returns an error code FAIL. Once the scheduler is running,
   new jobs are created by interrupt functions or are posted by other
   jobs.
*/
extern int		JOB_scheduler(void) ;

/*! \brief Post a job list to the ready queue

   Called by a data source/processing module to add a list of ready
   jobs to the ready queue. All jobs will share the same data.
*/

extern int		JOB_post(JOB_List *list, Ptr data) ;

/*! \brief Post a single job to the ready queue.
    \param f Function to be run. It must have a prototype matching JOB_Fxn.
    \param state State information to pass to the function. This can be a 
      pointer to an object or NULL if no state is needed by the function.
    \param nice Nice number for the job.
    \param data Data to pass to the job. This can be a pointer to an 
      object or NULL if no data is needed by the function. The DATA_Obj
      structure defined in data.h is the usual format for passing data.
*/

extern int		JOB_postone(JOB_Fxn f, Ptr state, uns nice, Ptr data) ;

/*! \brief Create a job wrapper for a function and attach it to a job list.

   This does not post the job on the ready queue but rather adds the job
   to a list of jobs that could be posted when data is available, i.e.,
   a blocked queue. The job is inserted in the correct nice order in the list. 
   \param list The job list to add the job to. This is a pointer to a
      job list because the job added might be at the head of list, changing
      the value of the job list. If the value of job list is NULLJOB, the new
      job will be the only job on the list. 
   \param f The function to be run. It must have a prototype matching JOB_Fxn.
   \param state Pointer to an object that will be passed to the job
      whenever it is run. Use NULL if not required.
   \param nice Nice number for the job.
*/

extern int		JOB_add(JOB_List *list, JOB_Fxn f, Ptr state, uns nice) ;

/*! \brief Detach a job from a job list and free the job wrapper.

   The job list is not the ready queue. It is any other list of jobs e.g.,
   the blocked queue of a processing function.
   \param list The job list to remove the job from. This is a pointer to a
      job list because the job removed might be at the head of list, changing
      the value of the job list.
   \param f The function to be removed. If multiple instances of the same
      function are on the job list, only the first will be removed. If a
      matching function is not found, no action is taken.
*/

extern void 	JOB_remove(JOB_List *list, JOB_Fxn f) ;

/*! \brief Removes a job or all instances of a job from the ready queue.
   \param f The function to be removed. If a matching function is not found, 
      no action is taken.
   \param which If multiple instances of the same function are on the 
      ready queue, the action taken depends on this value. Options are:
      - KILLFIRST Only the first will be removed.
      - KILLALL All instances are removed.
*/

extern void		JOB_cancel(JOB_Fxn f, int which) ;

/*! \brief Removes all jobs from a job list.

   The job are freed and the list destroyed. No jobs are cancelled from 
   the ready queue.
   \param list The job list to free.
*/

extern void		JOB_freelist(JOB_List list) ;

/*! \brief Report the status of the job module.

   This function only operates when a JTAG debugger is attached to the target
   and the API is compiled with JTAG defined. The report is given to the
   real-time display in the debugger using C stdio (printf).
   \param mode If mode is 1, a full report is given, otherwise a brief report
      is output.
*/

extern int		JOB_status(int mode) ;

/*! \brief Null job that causes the scheduler to end when it is posted on the ready queue. 

   The prototype has two dummy arguments to conform to the JOB_Fxn prototype.
*/

extern int		JOB_terminator(Ptr s, Ptr d) ;


#endif
