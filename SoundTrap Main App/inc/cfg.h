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

/*! \file info.h
    \brief Configuration manager
 
    Maintains a list of module instances and provides a layer of abstraction
    in chaining modules to form processing configurations.
*/

#ifndef _CFG_H_
#define _CFG_H_

/*! Return id from CFG_register if there is an error condition */
#define	NULLID			(-1)

/*! The root id is used for metadata. It is not available for use by other modules. */
#define	ROOTID			(0)

/*! Number of instances supported. */
#define	MAXID			(100)

/*! Key for passing chaining information to modules.

    Running an attach function with SRCNICE for a nice number means we are indicating 
    to the downstream function the id number of the upstream function
*/
#define	SRCNICE			(-1)

#define	CFG_BADID		(1)
#define	CFG_BADATTACH	(2)

/*! Prototype for attach functions.
 
    Participating 'attach' functions must have this prototype where the arguments are 
    the upstream id, the downstream processor id, and the nice number of the job.
*/
typedef int	(*CFG_Attach)(int,int,int) ;

/*! \brief Register an instance of a data processor and receive a unique configuration id.

    Used to enroll a module in the abstraction functions implemented by cfg.c. This facilitates
    chaining processing modules using a series of CFG_attach instructions rather than module-
    specific attach instructions. It also provides a convenient handle (id number) for the instance
    to be used in associating metadata to the module and in changing parameters. 
    \param proc The processing function that should be called when data is ready for the instance.
    This is the job that will be posted on the ready queue and must conform to the JOB_Fxn
    prototype. To register a data source, use NULL for the proc function (i.e., there is 
    nothing upstream of a data source).
    \param attach The function to call when a downstream module wants to attach to this
    module. To register a data sink, use NULL for the attach function (i.e., there is nothing 
    downstream of a data sink).
    \param state Pointer to state information to be passed to the processing function whenever
    it is run. Use NULL if no state is required.
    \return A unique identification number for the instance if the operation was performed 
    without problem, NULLID otherwise.
*/

extern int	CFG_register(JOB_Fxn proc, CFG_Attach attach, Ptr state) ;

/*! \brief Chain a pair of processing modules.

    Attach a processing module to another module so that data can be streamed
    between them. Module instances are referred to by their identification 
    number returned when opening the module or from a call to CFG_register.
    \param upstr_id the id number of the upstream or data source instance.
    \param downstr_id the id number of the downstream or data sink instance.
    \param nice the nice number associated with this data connection. A low
    nice number means that the downstream processing job will be run with high
    priority whenever there is data available from the upstream processor.
    \return OK if the operation was performed without problem, FAIL otherwise.
*/

extern int	CFG_attach(int upstr_id, int downstr_id, int nice) ;

/*! \brief Get the state associated with an instance of a processing module.

    The instance must first have been registered using CFG_register.
    \param id The instance identification number returned when opening a module.
    \return A pointer to the state.
*/

extern Ptr	CFG_getstate(int id) ;


/*! \brief Get the processing function of a processor.

    The instance must first have been registered using CFG_register.
    \param id The instance identification number returned when opening a module.
    \return A pointer to the processing function. Returns NULL if the module associated 
    with id is a data source and so does not have a processing function.
*/
 
extern JOB_Fxn	CFG_getprocfxn(int id) ;

/*! \brief Recover the metadata of a processor.

    \param id The instance identification number returned when opening a module.
    \return Pointer to the metadata associated with the processor.
*/

extern Ptr	CFG_getmeta(int id) ;

/*! \brief Associate metadata with a processor instance.

    \param id The instance identification number returned when opening a module.
    \param meta Pointer to the metadata to associate with the id. There are no
    restrictions on the type of metadata.
    \return OK if the operation was performed without problem, FAIL otherwise.
*/

extern int	CFG_setmeta(int id, Ptr meta) ;

/*! \brief Pass data to a processor.

    \param id The instance identification number returned when opening a module.
    \param data A pointer to the data to pass to the processor. The data can be
    of any form for which a pointer can be defined - it is up to you to make
    sure that the receiving processor will know how to interpret the pointer.
    \return FAIL if the module associated with id is a data source module. OK
    otherwise.
*/

extern int	CFG_pass(int id, Ptr data) ;

/*! \brief Get the number of declared module instances.

    Returns the number of module instances declared using CFG_register. 
    \return The number of ids allocated.
*/

extern int	CFG_getidcnt(void) ;

#endif
