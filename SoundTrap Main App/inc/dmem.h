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

/*! \file dmem.h
    \brief Simple dynamic memory allocation with fixed latency.
    
    Blocks of data are allocated dynamically from pre-allocated groups
    of same-sized blocks. The block sizes available and the number of
    blocks in each group size are defined in dmem.c. The underlying memory
    is drawn from a single statically allocated block starting at DMEM_START
    and of length DMEM_LENGTH defined in the application .cmd file. 
    DMEM_START must have an even address. Requests for memory are satisfied
    with the smallest block available that is greater than or equal to the
    requested size. A pointer to the memory block is returned. The size of
    the memory block is stored in (int *)(pointer)-1 and must not be changed.
*/


/*! Size in integer words of the normal buffer size to use in exchanging data.
    This should coincide with a block size of which a large number of blocks
    are pre-allocated
*/
 #define	BIGBUFFSIZE		(1024)

/*! Return value when allocation fails */
 #define	NULLPTR	((Ptr)NULL)

 // error codes
 #define	DMEM_NOINIT		(1)	//!<  memory allocator not yet initialized
 #define	DMEM_SIZETOOBIG	(2)	//!<  too larger size requested
 #define	DMEM_BADPTR		(3)	//!<  block to be freed has a bad pointer
 #define	DMEM_BADGRPN	(4)	//!<  group number is corrupt in block to be freed
 #define	DMEM_INITTOOBIG	(5)	//!<  requested blocks too large: increase DMEM_LENGTH
 #define	DMEM_NOFREE		(6)	//!<  no memory available to allocate
 #define 	DMEM_FAILEDOPENINGFILE	(7)	//!<  couldn't open profile output file

/*! \brief Initialize the memory manager.

    The memory manager must be initialized before use by calling this
    function. This should be done when initializing the API in the main routine.
    \return FAIL if there is a problem, OK otherwise.
*/
 extern	int	DMEM_init(void) ;

/*! \brief Allocate memory from the static pool.

    \param sz The number of integers (the smallest addressable unit on a digital signal
    processor) to allocate.
    \return A non-null pointer to a memory block or NULLPTR if there are no more blocks.
*/
 extern	Ptr	DMEM_alloc(uns sz) ;

/*! \brief Free a memory block and return it to the static pool.

    \param p Pointer to the memory block to be returned.
    \return OK if successful, FAIL otherwise.
*/
 extern	int	DMEM_free(Ptr p) ;

/*! \brief Print the current status of the memory manager.

    Only provides output when a JTAG debugger is connected and the C stdio printf
    functions are operable.
*/
 extern	int	DMEM_status(void) ;


 int DMEM_blocks_remaining() ;

