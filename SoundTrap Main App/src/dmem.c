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

/*! \file dmem.c
    \brief Simple dynamic memory allocation with fixed latency.

    Fixed size memory blocks are allocated dynamically from a large 
    static buffer. The static buffer is divided into groups of blocks
    with different sizes. A block with a size larger or equal to the
    requested size will be allocated by DMEM_alloc. The
	size and number of blocks are determined at compile time by
	the contents of the DMEM_NBLK and DMEM_SIZE arrays.

	Groups of like-sized blocks form a linked list. The address of
    the first member of each group is maintained in the DMEM_NEXT list.
    Blocks are allocated off the front of the list and returned to the 
    front of the list when freed. The word at ptr-1 where ptr is the
    pointer to a block of memory is used for the group number of the 
    block to simplify freeing. Each block extends from the pointer to
    pointer+size-1 where size is the requested size.

    buffers are allocated starting at DMEM_START defined in the d3.cmd file
    total buffer size will be less than or equal to DMEM_LENGTH defined in 
    d3defs.h. DMEM_START must have an even address.
*/

#include "d3defs.h"
#include "d3std.h"
#include "protect.h"
#include "board.h"
#include "devdep.h"
#include "misc.h"
#include "modnumbers.h"
#include "error.h"
#include "minmax.h"
#include "dmem.h"

// definitions to enable profiling:
// define DMEM_REPORT to make a .csv file with an entry for each alloc and free
// define DMEM_PROFILE to keep track of minimum number of blocks in each group

#define DMEM_LENGTH     ((Uint32)49000)

#define	DMEM_MAGIC		(0x5A3C)	// magic number to detect initialization
#define	DMEM_PROFILE
// the memory block for allocation by the dmem module is in
// a special section (.dmem)
#pragma DATA_ALIGN(DMEM_START, 8);
#pragma	DATA_SECTION(DMEM_START,".dmem")
int	DMEM_START[DMEM_LENGTH] ;


// The DMEM_Grp structure describes a group of like-sized buffers. It 
// contains a linked list pointer, the size of each buffers in words and 
// the buffer count

typedef struct {
			Ptr	next ;	// pointer to the next free buffer
			uns	size ;	// size of buffer in words
			int	n ;		// number of buffers available
			#ifdef DMEM_PROFILE
			int nmin ;  // minimum number of buffers for diagnostics
			uns maxsz ; // largest request allocated to this group
			long nreqs ;	// number of requests from this group
			#endif
			} DMEM_Grp ;

#ifdef DMEM_PROFILE
 #define DGINIT		0,0,0l
#else
 #define DGINIT
#endif

// DMEM_GRP is a static array of DMEM_Grp structures initialized to
// indicate how many buffers and of what size to pre-allocate.
// sizes must be in order from smallest to largest.
// actual block sizes will be odd so may be 1 larger than the sizes
// requested. Each block is preceded by an integer group number but 
// blocks should start on even boundaries to avoid alignment issues 
// in structures containing pointers or long words.


static DMEM_Grp	DMEM_GRP[] = {
			{NULLPTR,8,50,DGINIT}, //8*50=400
			{NULLPTR,20,60,DGINIT},//20*60=1200
			{NULLPTR,40,10,DGINIT},//40*10=400
			{NULLPTR,100,30,DGINIT},//100*30=3000
			{NULLPTR,256,7,DGINIT},//256*7=1792
			{NULLPTR,BIGBUFFSIZE,40,DGINIT}//1024*40= 40960 total = 47752 + 2 x nblocks
			} ;


#define	DMEM_NGRPS		(sizeof(DMEM_GRP)/sizeof(DMEM_Grp))
#define AUDIO_BUF_INDEX 5 //index of audio buffer group

#ifdef DMEM_REPORT
 #define	DMEM_PROFILEFILE	"dmem_profile.csv"
 static FILE *dmem_fp ;
#endif


int		DMEM_init(void)
{
 // initialize DMEM_GRP array and setup linked lists of free blocks
 DMEM_Grp	*g = DMEM_GRP ;
 int			k, kk, *lastp ;
 int			*p=DMEM_START ;	// p points to the start of the first block
 uns			sz ;

 // leave a bad magic number in case initialization doesn't terminate
 *(uns *)p++ = ~DMEM_MAGIC ;
 GO_ATOMIC ;

 // initialize each block of each group
 for(k=0;k<DMEM_NGRPS;k++,g++) {		// initialize each group of blocks

	lastp = (int *)NULL ;			// end of linked list indicator
	sz = g->size | 1 ;				// get the block size for this group

	// actual size may be 1 bigger than specified to keep blocks on even boundaries
	g->size = sz ;						// save the adjusted size

	for(kk=0;kk<g->n;kk++) {	// initialize each block in the group

	   // check that there is enough room in the big buffer to allocate this one
	   
	   if(p+sz >= DMEM_START + DMEM_LENGTH-1 ) {
	 	   END_ATOMIC ;
		   err(DMEM_MOD,DMEM_INITTOOBIG) ;
		   return(FAIL) ;
		   }

	   *p++ = k ;						// set the group number
	   *(Ptr *)p = lastp ;			// setup the linked list: pointer to the previous buffer
	   lastp = p ;						// keep the current pointer for the next iteration
	   p += sz ;						// increment the buffer pointer
	   }

	// the head of the linked list points to the first buffer in the group
	g->next = (Ptr)lastp ;		
	#ifdef DMEM_PROFILE
     g->nmin = g->n ;
    #endif
	}

 // leave a good magic number to show that initialization is done
 *(uns *)DMEM_START = DMEM_MAGIC ;
// END_ATOMIC ;

 #ifdef DMEM_REPORT
   dmem_fp = fopen(DMEM_PROFILEFILE,"wt") ;
   if(dmem_fp==NULL) {
	  err(DMEM_MOD,DMEM_FAILEDOPENINGFILE) ;
	  return(FAIL) ;
	  } 
   fprintf(dmem_fp,"\"reqsz\",\"offsz\",\"dir\",\"nrem\"\n") ;
 #endif

 return(OK) ;
}

int	DMEM_blocks_remaining()
{
	 return DMEM_GRP[AUDIO_BUF_INDEX].n; //TODO fix this fudge
}


Ptr	DMEM_alloc(uns sz)
{
 DMEM_Grp	*g = DMEM_GRP ;
 int			k ;
 Ptr			p ;

#ifdef SAFE
 // check that we are initialized
 if(*(uns *)DMEM_START != DMEM_MAGIC) {
	err(DMEM_MOD,DMEM_NOINIT) ;
	return(NULLPTR) ;
	}
#endif

 if(sz==0)
    return(NULLPTR) ;

 // find a group with block size >= sz
 for(k=0; k<DMEM_NGRPS && g->size<sz; k++, g++) ;

 if(k>=DMEM_NGRPS) {
	err(DMEM_MOD,DMEM_SIZETOOBIG) ;
	return(NULLPTR) ;
	}

 // g is the desired group. the block must come from this group
 // or one bigger.

 GO_ATOMIC ;			// disable interrupts

 // check if there is a free buffer in this group
 if(g->next == NULLPTR) {		// if not...
	++g ;										// check the next group just in case

	if(k+1>=DMEM_NGRPS || g->next==NULLPTR) {	// if there is no next group or no room
		#ifdef JTAG
	     DMEM_status() ;
		 printf("memory allocation fail\n");
		 exit(1) ;
		#else
		  //fatal(DMEM_MOD,666) ;		// make a noise and hibernate
		#endif
		return(NULLPTR) ;			
		}
	}

 p = g->next ;				// get next free buffer
 g->next = *(Ptr *)p ;	// the following buffer will be the link from this buffer

 // decrement the free buffer count to keep track of allocations
 --(g->n) ;

 #ifdef DMEM_PROFILE
  g->nmin = MIN(g->n,g->nmin) ;
  g->maxsz = MAX(sz,g->maxsz) ;
  ++(g->nreqs) ;
 #endif

 #ifdef DMEM_REPORT
  fprintf(dmem_fp,"%d,%d,1,%d\n",sz,g->size,g->n) ;
 #endif

 END_ATOMIC ;			// re-enable interrupts
 return(p) ;
}


int	DMEM_free(Ptr p)
{
 DMEM_Grp	*g ;
 int		gn ;

 if(p==NULLPTR)
	 return(OK) ;

#ifdef SAFE
 // check that we are initialized
 if(*(uns *)DMEM_START != DMEM_MAGIC) {
	err(DMEM_MOD,DMEM_NOINIT) ;
	return(FAIL) ;
	}

 // check the pointer is reasonable
 if((int *)p<DMEM_START+1 || (int *)p>=DMEM_START+DMEM_LENGTH) {
	 err(DMEM_MOD,DMEM_BADPTR) ;	// otherwise, complain
	 failmess("bad pointer in DMEM_free") ;
	 return(FAIL) ;
	 }
#endif

 gn = *((int *)(p)-1) ;			// get the group number
 if(gn<0 || gn>=DMEM_NGRPS) {	// if not a valid group number...
	 //bad group in DMEM_free
	 fatal(DMEM_MOD,DMEM_BADGRPN);// fail, log error
	 return(FAIL) ;
	 }

 g = &DMEM_GRP[gn] ;
 GO_ATOMIC ;					// disable interrupts

 *(Ptr *)p = g->next ;		// this block now points to the old next free
 g->next = p ;					// and this block becomes the next free
 ++(g->n) ;						// increment count of free buffers

 #ifdef DMEM_REPORT
  fprintf(dmem_fp,"-1,%d,0,%d\n",g->size,g->n) ;
 #endif

 END_ATOMIC ;					// re-enable interrupts
 return(OK) ;
}


int	DMEM_status(void)
{
 // report how many free blocks there are in each group
 #ifdef DMEM_PROFILE
 int	k ;
 char	s[70] ;

 // check that we are initialized
 if(*(uns *)DMEM_START != DMEM_MAGIC) {
    err(DMEM_MOD,DMEM_NOINIT) ;
    return(FAIL) ;
    }

 for(k=0;k<DMEM_NGRPS;k++) {
	snprintf(s, 70, "GRP=\"%d\" SZ=\"%d\" MAXSZ=\"%d\" NMIN=\"%d\" NREM=\"%d\" NREQS=\"%ld\"",k,
			DMEM_GRP[k].size,DMEM_GRP[k].maxsz,DMEM_GRP[k].nmin,DMEM_GRP[k].n,DMEM_GRP[k].nreqs) ;
    #ifdef JTAG
	 printf("%s\n",s) ;
    #endif
   // INFO_event("DMEM",s,NULL) ;
	// reset the minimum number and request count catchers
	DMEM_GRP[k].nmin = DMEM_GRP[k].n ;
	DMEM_GRP[k].nreqs = 0l ;
	}

 #endif
 return(OK) ;
}
