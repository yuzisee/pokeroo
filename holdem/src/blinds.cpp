/***************************************************************************
 *   Copyright (C) 2005 by Joseph Huang                                    *
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

#include "blinds.h"
#include <math.h>

BlindStructure::~BlindStructure()
{
}

const double BlindStructure::SmallBlind()
{
	return mySmallBlind;
}

const double BlindStructure::BigBlind()
{
	return myBigBlind;
}

void BlindStructure::Reload(const float64 small,const float64 big,const uint32 handnum)
{
    mySmallBlind = small;
    myBigBlind = big;
}

bool GeomPlayerBlinds::PlayerEliminated()
{
	myBigBlind *= bigRatio;
	mySmallBlind *= smallRatio;
    return true;
}


bool StackPlayerBlinds::PlayerEliminated()
{
	--playerNum;
	myBigBlind = (totalChips/playerNum)*blindRatio;
	mySmallBlind = myBigBlind/2.0;
    return true;
}

bool AlgbHandBlinds::HandPlayed(float64 timepassed)
{
	++since;
	if( since >= freq )
	{
		since = 0;
		myBigBlind += bigPlus;
		mySmallBlind += smallPlus;
        return true;
	}else
    {
        return false;
    }
}

void SitAndGoBlinds::Reload(const float64 small,const float64 big,const uint32 handnum)
{
    const float64 rat = small/big;
        hist[0] = small*rat*rat;
        hist[1] = small*rat;
        hist[2] = hist[0]+hist[1];

    handCount = handPeriod - (handnum % handPeriod);
    if( handCount == 0 ) handCount = handPeriod;

}

float64 SitAndGoBlinds::fibIncr(float64 a, float64 b)
{
    const float64 roundBy = b-a;
    return floor((a+b)/roundBy)*roundBy;
}

bool SitAndGoBlinds::HandPlayed(float64 timepassed)
{
	--handCount;
    if( handCount == 0 )
    {
        handCount = handPeriod;
        mySmallBlind = hist[0] + hist[1] + hist[2];
        myBigBlind = mySmallBlind*2;
        const float64 newHist = fibIncr(hist[2],hist[1]);
        hist[0] = hist[1];
        hist[1] = hist[2];
        hist[2] = newHist;
        return true;
    }else
    {
        return false;
    }
}


