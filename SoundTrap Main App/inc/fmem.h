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

/*! \file fmem.h
    \brief Simple one-short memory allocation for fast memory.
    
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

 // error codes
 #define	FMEM_SIZETOOBIG	(1)	//!<  too larger size requested

/*! \brief Initialize the memory manager.

    The memory manager must be initialized before use by calling this
    function. This should be done when initializing the API in the main routine.
    \return FAIL if there is a problem, OK otherwise.
*/
 extern	int	FMEM_init(void) ;

/*! \brief Allocate memory from the static pool.

    \param sz The number of integers (the smallest addressable unit on a digital signal
    processor) to allocate.
    \return A non-null pointer to a memory block or NULLPTR if there are no more blocks.
*/
 extern	Ptr	FMEM_alloc(uns sz) ;

/*! \brief Print the current status of the memory manager.

    Only provides output when a JTAG debugger is connected and the C stdio printf
    functions are operable.
*/
 extern	int	FMEM_status(void) ;

