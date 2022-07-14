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

#include <tistdtypes.h>
#include "d3defs.h"
#include "d3std.h"
#include "misc.h"
#include "error.h"
#include "i2c.h"

#define    PSENSOR_ADDR    (0x68)            // iic address for MCP3424
#define    PSENSOR_CONV    (0.000119209)        // 15.625uV/8/2^14 where 8 is the PGA gain, 2^14 is the shift on v to fix the sign
#define    PSENSOR_SCALE    (100/32.0)            // approx. centimeters/uV for a PA3L 200bar
                                                    // multiplication by this converts ADC units into uV

int psenseRead(long *v)
{
 // returns the pressure reading in centimeters of water
 Uint16    buff[3] ;

 // make sure XF is on - it is by default so nothing to do.
 // read the last conversion
 if(i2cRead(PSENSOR_ADDR, buff, 3, 1000) != OK)
    return(FAIL) ;

 // buff0-2 encode the measurement in an 18 bit 2's complement number
 // concatenate and sign extend
 *v = ((long)((buff[0]<<8)+buff[1]))<<8 ;
 *v = (*v+buff[2])<<14 ;
 *v = *v / 16384;
 //*v = (long)((PSENSOR_CONV * PSENSOR_SCALE) * (float)(*v)) ;

 // request the next pressure conversion
 buff[0] = 0x8f ;  // 18 bit (3.75 SPS)
 //buff[0] = 0x8B ;    // 16 bit (15 SPS)
 return(i2cWrite(PSENSOR_ADDR, buff, 1, 1000)) ;
}

int psenseInit()
{
	return OK;
}
