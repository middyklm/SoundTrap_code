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
    \brief Metdata manager.

    Information about each module in the processing chain is stored in the FLASH
    file-system with each recording. This metadata is used both to document the
    recording method and to create a decodor for the data in d3read on the PC.
    Metadata entries describe the settings of each processing module, which other
    modules it is connected to and what to do when data from that module is encountered
    in the FLASH recording. Most processing modules automatically configure their own
    metadata during open or attachment. The user must ensure that all data passed to
    the FLASH memory using FLSH_post has metadata associated with it. See the example
    processing module in apps/templ.c for an example of how this is done.
*/ 


// supported file types
#define	FTYPE_XML		"xml"   //!< XML filetype
#define	FTYPE_WAV		"wav"   //!< Microsoft WAV format filetype
#define	FTYPE_TXT		"txt"   //!< plain text filetype
#define	FTYPE_BIN		"bin"   //!< block-oriented packed binary filetype
#define	FTYPE_CSV		"csv"   //!< csv filetype

// error codes
// error definitions
#define	BADFTYPE		(1)
#define	BADID			(2)
#define	BADFIELDSIZE	(3)
#define	INFO_ALLOCFAIL	(4)

/*! Metadata for a WAV format file used in the call to INFO_wavmeta
*/
typedef struct {
      long  fs ;        //!< fs The sampling rate of the data in Hz.
      int   nbits ;     //!< The number of bits in each sample word.
      int   nch ;       //!< The number of channels.
      char  *suffix ;   /*! String containing the suffix to attach to each file name. 
                        This allows multiple WAV format files to be produced during a recording 
                        e.g., containing low-frequency and high-frequency sound. */
      int   exp ;       //!< Sampling rate exponent e.g., -3 means that the sampling rate is in mHz. 
      int   timechk ;   //!< disable time checking in d3read if 0
      int   blklen ;    //!< number of samples per channel in a 'block' of contiguous samples
      } INFO_WavMeta ;


/*! \brief Initialize metadata for a configuration.

    Creates a metadata field in the FLASH memory for an instance of a module.
    If the module is producing data for output to a file, the metadata field
    will define what filetype to associate with the data and the parameters of
    the file. If no file or codec is associated with the data source, the metadata
    is informational.
    \param id The instance identification number returned when opening a module.
    \param ftype The file type associated with the data source. Must be one of the
    defined filetypes in info.h (e.g., FTYPE_WAV). use NULL if no file will be needed
    (i.e., if no data with this id will ever be put into the FLASH memory).
    \param src The identification number of the upstream source of the module. This
    creates a chaining entry in the metadata field so that the entire processing chain
    can be reconstructed from metadata entries. Use NULLID if there is no upstream
    data source.
    \param codecid The identification number of a data compressor used with the data
    stream. A matching decompressor will be needed to unpack the data on the host PC and
    this metadata field connects a compressing module and its metadata with the current
    module.
    \return OK if the operation was performed without problem, FAIL otherwise.
*/

extern int	INFO_new(int id, char *ftype, int src, int codecid) ;

/*! \brief Add an entry to the metadata associated with an id.

    Creates a metadata entry of the form <field attr> value <\field> in the metadata
    associated with id. The metadata entry is not parsed by the processor and it is up
    to the user to ensure that it is syntactically correct.
    \param id The instance identification number returned when opening a module.
    \param field String containing the field name. A field name must be given.
    \param attr String containing any attributes. Attributes must be formatted as e.g.:
    "MODE=\"4\" RATE=\"5.6\"". There are no keywords or restrictions on what attributes 
    can be added to a field except that the value of each attribute must be in double quotes.
    If no attributes are required, use NULL.
    \param value String containing any value information. There are no restrictions on the
    contents of the value string. If no value is required, use NULL.
    \return OK if the operation was performed without problem, FAIL otherwise.
*/

extern int	INFO_add(int id, char *field, char *attr, char *value) ;

/*! \brief Close and post the metadata associated with an id.

    Use this instruction after a series of INFO_new, INFO_add, INFO_add, ...
    calls to complete the metadata for a module instance.
    \param id The instance identification number returned when opening a module.
    \return OK if the operation was performed without problem, FAIL otherwise.
*/

extern int	INFO_end(int id) ;


/*! \brief Request metadata associated with an id.

    Returns the metadata prepared for a module instance as a DATA_Obj containing a
    packed string. See pstr.h for functions to work with packed strings.
    \param id The instance identification number returned when opening a module.
    \return Packed string of metadata in a DATA_Obj.
*/

extern DATA_Obj *INFO_request(int id) ;


/*! \brief Post all current metadata to the FLASH memory.

    The metadata associated with each module instance will be sent to the FLASH memory.
    This function is called automatically at the start of each new recording in the FLASH.
    \return OK if the operation was performed without problem, FAIL otherwise.
*/

extern int	INFO_requestall(void) ;


/*! \brief Add an arbitrary field to the metadata.

    Add a self-contained metadata entry describing an event not associated with any 
    particular module instance. There are no restrictions on the contects of the entry 
    and the entry is not parsed on the PC.
    \param field String containing the field name. A field name must be given.
    \param attr String containing any attributes. Attributes must be formatted as e.g.:
    "MODE=\"4\" RATE=\"5.6\"". There are no keywords or restrictions on what attributes 
    can be added to a field except that the value of each attribute must be in double quotes.
    If no attributes are required, use NULL.
    \param value String containing any value information. There are no restrictions on the
    contents of the value string. If no value is required, use NULL.
    \return OK if the operation was performed without problem, FAIL otherwise.
*/

extern int	INFO_event(char *field, char *attr, char *value) ;


/*! \brief Add metadata for a WAV format file.

    Helper function to format metadata for a WAV filetype. This can be done with a series
    of INFO_add instructions but this function ensures that the metadata will be
    parsed correctly on the PC. Additional metadata can also be added with INFO_add 
    instructions if required. INFO_new and INFO_end instructions must bracket this 
    instruction (and any other INFO_add instructions for the same module).
    \param id The instance identification number returned when opening a module.
    \param w Pointer to a INFO_WavMeta structure containing metadata
    \return OK if the operation was performed without problem, FAIL otherwise.
*/

extern int		INFO_addwavmeta(int id, INFO_WavMeta *w) ;
// was this: extern int	INFO_addwavmeta(int id, long fs, int nbits, int nchs, char *suffix, int exponent) ;

#ifdef _PSTRING
 extern PSTRING	*INFO_addopen(int id, char *field, char *attr, int vals) ;
 extern void	INFO_addvalue(PSTRING *p, char *value) ;
 extern void	INFO_endfield(PSTRING *p, char *field) ;
#endif
