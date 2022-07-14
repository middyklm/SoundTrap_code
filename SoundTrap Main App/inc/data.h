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

// DATA module - abstract data buffer handling functions
// data.h v1.0

/*! \file data.h
    \brief Wrapper functions for dynamically allocating data blocks.
    
   Allocate and frees data buffers from the DMEM memory pool. The data 
   buffer is combined with a meta-data structure to create a DATA_Obj. 
   This object should contain all of the information needed to communicate 
   a block of data from one module to another. 
*/


/*! pointer to an undefined DATA_Obj */
#define		NULLDATA	((DATA_Obj *)NULL)

// Error flags to add to the id component of a DATA_Obj
// This feature is not yet used.
#define		TIMING_ERROR	(0x8000u)

// be careful with the order in the following structure - some interrupt
// routines may use memcpy to fill multiple components in a DATA_Obj
/*! Structure of information about a block of data including the pointer
    to the data itself. */

typedef struct {
	Ptr		p ;			//!< where the data is
	Time	rtime ;		//!< receive time to the nearest preceding second
	ulong	mticks ;	//!< microseconds since rtime to the first sample in buffer
	ulong	fs ;		//!< sampling rate in Hz
	uns		size ;		//!< number of 16-bit words in the buffer
	uns		nsamps ;	//!< number of samples per channel in the buffer
	uns		nch ;		//!< number of channels of data
	uns		nbits ;		//!< number of valid bits per sample in the buffer
	uns		id ;		//!< module id that produced the data + tag & error flags
	int		sema ;		//!< semaphore for data sharing
	uns		maxsize ;	//!< maximum number of 16-bit words that fit in the buffer
	} DATA_Obj ;


/*! Structure containing a sub-set of information about a data block.

    This structure can be pre-prepared and copied to a DATA_Obj using memcpy
    for fast initialization of data objects for which the sampling rate, number
    of bits etc, do not change.
*/

 typedef struct {
	// match the order of the following elements to those in DATA_Obj so that
   // memcpy can be used to fill the metadata in new DATA_Obj's from the values here
	ulong		fs ;		//!< sampling rate in Hz
	uns			size ;		//!< number of 16-bit words in the buffer
	uns			nsamps ;	//!< number of samples per channel in the buffer
	uns			nch ;		//!< number of channels of data and channel bit map
	uns			nbits ;		//!< number of valid bits per sample in the buffer
	uns			id ;		//!< configuration id
	int			sema ;		//!< semaphore (normally set this to zero)
	} DATA_Meta ;


/*! Another structure containing a sub-set of information about a data block.

    This structure can be pre-prepared and copied to a DATA_Obj using memcpy
    for fast initialization of data objects for which the timing, sampling 
    rate, number of bits etc, do not change.
*/

 typedef struct {
	// match the order of the following elements to those in DATA_Obj so that
   // memcpy can be used to fill the metadata in new DATA_Obj's from the values here
	Time		rtime ;		//!< receive time to the nearest preceding second
	ulong		mticks ;	//!< microseconds since rtime to the first sample in buffer
	ulong		fs ;		//!< sampling rate in Hz
	uns			size ;		//!< number of 16-bit words in the buffer
	uns			nsamps ;	//!< number of samples per channel in the buffer
	uns			nch ;		//!< number of channels of data and channel bit map
	uns			nbits ;		//!< number of valid bits per sample in the buffer
	uns			id ;		//!< configuration id
	int			sema ;		//!< semaphore (normally set this to zero)
	} DATA_FullMeta ;

/*! Macro to initialize the metadata in a DATA_Obj from another DATA_Obj */
 #define    DATA_CPYALLMETA(dout,din)   (memcpy(&(dout->rtime),&(din->rtime),sizeof(DATA_Obj)-5))

/*! Macro to initialize the metadata in a DATA_Obj from a DATA_Meta structure */
 #define	DATA_SETMETA(dobj,dmeta)	(memcpy(&((dobj)->fs),(dmeta),sizeof(DATA_Meta)))

/*! Macro to initialize the metadata in a DATA_Obj from a DATA_FullMeta structure */
 #define	DATA_SETFULLMETA(dobj,dmeta) (memcpy(&((dobj)->rtime),(dmeta),sizeof(DATA_FullMeta)))

// tools for extracting channel information (currently this feature is unused)
 #define	DATA_NCHANS(a)	((a)&0x0ff)
 #define	DATA_CHANMASK(a)	(((a)>>8)&0x0ff)

/*! Macro to initialize the semaphore in a DATA_Obj */
 #define	DATA_SETSEMA(d,n)		(d->sema = n)

/*! Macro to increment the semaphore in a DATA_Obj, i.e., to assert possesion */
 #define	DATA_INCSEMA(d)			(++(d->sema))

/*! Macro to post a job list with a DATA_Obj and set the semaphore */
 #define	DATA_POST(j,d)		((d)->sema = JOB_post((j),(Ptr)(d)))	

/*! Structure wrapper for a DATA_Obj to use it as a buffer where the data object
    is filled gradually in successive calls of a processor */
 typedef struct {
	int		*p ;		//!< pointer to next sample to process
	int		n ;			//!< number of samples left to process
	int		sema ;	//!< unallocated storage word (this is used anyway for alignment
	DATA_Obj	*d ;			//!< the underlying DATA_Obj being filled/emptied
	} DATA_Buff ;

 /*! macro to return the number of samples left to process in a buffer */
 #define		BUFF_NTODO(a)		((a)->n)
 /*! macro to return the number of samples that can be handled by a pair of buffers */
 #define		BUFF_NMAX(a,b)		(MIN((a)->n,(b)->n))
 /*! macro to move the buffer pointer by n samples */
 #define		BUFF_UPDATE(a,nn)	{((a)->n)-=(nn);((a)->p)+=(nn)*(((a)->d)->nch);}
 /*! macro to initialize a buffer */
 #define		BUFF_INIT(a,dd,nn)	{(a)->d=(dd);(a)->p=(dd)->p;(a)->n=(nn);}
 /*! macro to initializa a buffer with no DATA_Obj */
 #define		BUFF_NULL(a)		{(a)->n=0;(a)->d=NULLDATA;}
 /*! macro to free the DATA_Obj in a buffer */
 #define		BUFF_FREE(a)		DATA_free((a)->d)
 /*! macro to allocate a DATA_Obj and embed it in a buffer */
 #define		BUFF_ALLOC(b,n,nch)		{DATA_Obj *d;d=DATA_alloc((n)*(nch));BUFF_INIT(b,d,n)}
 /*! macro to test if a buffer refers to an allocated DATA_Obj */
 #define		BUFF_ISNULL(a)		((a)->d==NULLDATA)
 /*! macro to test if a buffer is complete (i.e., all data has been handled) */
 #define		BUFF_ISDONE(a)		((a)->n<=0)

/*! Structure for modifying the time elements in a DATA_Obj when a processor is
    buffering or changing the sampling rate */
 typedef struct {
    long	lastfs ;		// last sampling rate
	long	tickfact ;		// microticks per block of 2^exp input points
	int		exp ;			// timebase used with tickfact
	} DATA_Timekey ;

 /*! macro to initialize a DATA_Timekey structure (this structure is used 
     to set the time elements in a DATA_Obj */
 #define		DATA_INITTKEY(t)	((t)->lastfs = 0)

// functions defined in data.c

/*! \brief Allocate a DATA_Obj from the dynamic memory pool.

    \param len Minimum data size required in 16 bit words. This is the size of 
    the underlying data space and does not include the overhead of the DATA_Obj
    wrapper. The actual size of the data block allocated will depend on the
    block sizes available from the DMEM module. The actual size will be returned
    in the maxsize element of the returned DATA_Obj and will always be >= len.
    \return A pointer to the DATA_Obj or NULLDATA if unable to allocate.
*/

 extern DATA_Obj	*DATA_alloc(uns len) ;

/*! \brief Free a previously-allocated DATA_Obj and the underlying data buffer.
*/

 extern void		DATA_free(DATA_Obj *d) ;

/*! \brief Print information about a DATA_Obj

    This function requires that the JTAG debugger is connected and the code is 
    compiled with JTAG defined.
*/

 extern void		DATA_status(DATA_Obj *d) ;

/*! \brief Set the time elements of a DATA_Obj

    The time elements (rtime and mticks) in an 'output' DATA_Obj are set based on the
    time in another 'input' DATA_Obj corrected for a number of samples elapsed. The
    output time will be the input time plus nsamples/fs_out, where fs_out is the output
    sampling rate and nsamples is the number of samples elapsed at the output rate.
    To avoid performing the division by fs_out at every invocation, 1/fs_out is
    stored in the DATA_Timekey structure and is only calculated if the sampling-rate
    changes.
    \param t Pointer to a DATA_Timekey structure initialized using DATA_INITTKEY(t).
    \param out Pointer to the DATA_Obj requiring a time entry.
    \param in Pointer to the reference DATA_Obj.
    \param nsamps The number of samples at the output sampling rate that have
    elapsed between the first sample in 'in' and the first sample in 'out'.
    \return 1 if the sampling rate has changed since the last call, 0 otherwise.
*/

 extern int		DATA_settime(DATA_Timekey *t, DATA_Obj *out, DATA_Obj *in, int nsamps) ;

#ifdef _JOB
/*! \brief create a zero-length DATA_Obj and pass it to the functions in a job list

    \param j A pointer to a job list to pass the data to.
    \param id The configuration id to associate with the data.
*/

 inline void     DATA_passnull(JOB_List *j,int id) 
 {
 DATA_Obj *d = DATA_alloc(0) ;		// make a 0 size output object
 d->nsamps = 0 ;
 d->id = id ;

 // pass the 0-length data to the attached jobs
 DATA_POST(j,d) ;
 }

#endif
