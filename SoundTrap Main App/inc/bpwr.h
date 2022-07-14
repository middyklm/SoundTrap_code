// D3-API v1.0
// Copyright (C) 2008-2010, Mark Johnson / WHOI
//
// This file is part of D3, a real-time patch panel scheduler
// for digital signal processors.
//
// D3 is free software: you can redistribute it 
// and/or modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation, either version 3 of 
// the License, or any later version.
//
// D3 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with D3. If not, see <http://www.gnu.org/licenses/>.

/*! \file bpwr.h
    \brief Block-oriented power estimator
 
    Fast two-speed power estimator for click detectors. Produces a fast moving-
    average power estimate and a slow, outlier-constrained AR power estimate.
    Power estimates are written to memory locations provided at open.
*/


// error messages	
#define		BPWR_ALLOCFAIL	(1)

#define		BPWR_PSHFT		(6)

// Open an instance of the power estimator
// tcshift is the log-base-2 of the slow averager time constant.
// It should be less than or equal to BPWR_PSHFT
extern int		BPWR_open(int tcshift, long *fast, long *slow) ;

extern int		BPWR_close(int id) ;
