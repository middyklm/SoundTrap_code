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

// DECFILT module - decimation filters to use with the decm module
// decfilt.c v1.0

#include	"d3defs.h"
#include	"decfilt.h"

// decimation filters for df=2,3,4,...

// length 36 decimate by 2 symmetric FIR
static int	DECFILT2[] = {-2,-18,3,80,31,-183,-149,298,400,
			-361,-830,272,1507,157,-2649,-1526,5833,13522,
			13522,5833,-1526,-2649,157,1507,272,-830,
			-361,400,298,-149,-183,31,80,3,-18,-2} ;

static int	DECFILT3[] = {19,-21,-57,-54,16,125,168,39,-227,
			-406,-236,296,803,717,-199,-1431,-1851,-437,2855,
			6794,9470,9470,6794,2855,-437,-1851,-1431,-199,
			717,803,296,-236,-406,-227,39,168,125,16,-54,-57,-21,19} ;

static int	DECFILT4[] = {-7,20,47,60,41,-22,-115,-190,-178,
			-36,213,456,529,299,-237,-889,-1309,-1119,-83,
			1742,3970,5994,7197,7197,5994,3970,1742,-83,
			-1119,-1309,-889,-237,299,529,456,213,-36,-178,
			-190,-115,-22,41,60,47,20,-7} ;

// length 36 decimate by 5 symmetric FIR
static int	DECFILT5[] = {-22,5,45,98,146,149,64,-134,-415,
			-684,-795,-588,47,1117,2492,3929,5125,5804,5804,
			5125,3929,2492,1117,47,-588,-795,-684,-415,-134,
			64,149,146,98,45,5,-22} ;

static int	DECFILT6[] = {-33,-38,-37,-26,0,44,101,158,193,
			179,97,-60,-276,-508,-689,-741,-591,-192,466,1337,
			2331,3320,4161,4726,4924,4726,4161,3320,2331,1337,
			466,-192,-591,-741,-689,-508,-276,-60,97,179,193,
			158,101,44,0,-26,-37,-38,-33} ;

//h = round(32768*fir1(42,0.9/7));
static int	DECFILT7[] = {32,43,54,61,54,24,-39,-135,-256,-376,
			-461,-467,-352,-83,354,943,1645,2391,3099,3684,4068,
			4203,4068,3684,3099,2391,1645,943,354,-83,-352,-467,
			-461,-376,-256,-135,-39,24,54,61,54,43,32};

//h = round(32768*fir1(42,0.9/8));
static int	DECFILT8[] = {36,31,23,6,-27,-77,-146,-224,-295,-338,
			-324,-229,-29,284,708, 1222,1789,2362,2886,3307,3580,3675,
			3580,3307,2886,2362,1789, 1222,708,284,-29,-229,-324,-338,
			-295,-224,-146,-77,-27,6,23,31,36 };

static DecFilt	DECFILTS[] = {
					{DECFILT2,sizeof(DECFILT2)},
					{DECFILT3,sizeof(DECFILT3)},
					{DECFILT4,sizeof(DECFILT4)},
					{DECFILT5,sizeof(DECFILT5)},
					{DECFILT6,sizeof(DECFILT6)},
					{DECFILT7,sizeof(DECFILT7)},
					{DECFILT8,sizeof(DECFILT8)}};

#define	MAXDF	(1+sizeof(DECFILTS)/sizeof(DecFilt))


Uint16	getMaxDecFilt(){
	return MAXDF;
}


DecFilt		*getdecfilt(int df)
{
 if(df<2 || df>MAXDF)
	return((DecFilt *)NULL) ;

 return(&(DECFILTS[df-2])) ;
}
