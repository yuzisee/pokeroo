/***************************************************************************
 *   Copyright (C) 2006 by Joseph Huang                                    *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifndef HOLDEM_AICache
#define HOLDEM_AICache

#include "engine.h"
#include "ai.h"
#include <iostream>  // I/O
#include <fstream>   // file I/O
//#include <iomanip.h>   // format manipulation

/*
One must store:
myChancesEach; (init?)
mytotalChances; (init?)
statCount;
moreCards;
(statGroup;)
(currentCard;)
(myWins;)
*/

/*
CallStats:
cumulation (vector)
*/

/*
WinStats:
myAvg
[PCT,W/(W+L),Pyth]
	AvgDev
	StdDev
	Improve
	Skew
	Kurtosis
*/

class CacheManager
{
private:
	FILE* datafile;
	float64 myChancesEach;
	float64 mytotalChances;
	int32 statCount;
	int16 moreCards;
	int32 statGroup;
	StatResult* myWins;
	
	WinStats* myCachedW;
	CallStats* myCachedC;
	DealRemainder myStatBuilder;
	
public:
	CacheManager()
	{
	}

}
;

#endif
