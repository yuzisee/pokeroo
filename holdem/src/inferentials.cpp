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

#include <math.h>
#include "inferentials.h"

//#include <iostream>

//#define SMOOTHED_CALLCUMULATION
#define SMOOTHED_CALLCUMULATION_D

//const float64 CallCumulation::tiefactor = DEFAULT_TIE_SCALE_FACTOR;


void DistrShape::AddVal(float64 x, float64 occ)
{
	float64 d = (x - mean);
	float64 d1 = d * occ;
	float64 d2 = d1*d;
	float64 d3 = d2*d;
	float64 d4 = d3*d;

	avgDev += fabs(d1);
	stdDev += d2;
	skew += d3;
	kurtosis += d4;

	if( d > 0 ){
		improve += occ;
	}
	else if( d == 0 ){
		improve += occ/2;
	}
	if( x < worst ) worst = x;
}

void DistrShape::Complete(float64 mag)
{
	normalize(mag);
	Complete();
}

void DistrShape::Complete()
{
	avgDev /= n;
	stdDev /= n;
	stdDev = sqrt(stdDev);
	improve *= 2/n;
	improve -= 1;

	float64 o3 = stdDev*stdDev*stdDev;
	skew /= n*o3;
	kurtosis /= n*o3*stdDev;
	kurtosis -= 3;
}

void DistrShape::normalize(float64 mag)
{
	avgDev /= mag;
	stdDev /= mag*mag;
	skew /= mag*mag*mag;
	kurtosis /= mag*mag*mag*mag;


	worst /= mag;
	mean /= mag;
}
CallCumulation::~CallCumulation()
{
}

///oddsFaced is the "virtual chance to win" that would correspond to the pot odds faced.
///eg. 4:1 pot odds means opponent can bet 1 to win 4, (ie. pay 1 to receive 5 = 20% = 0.2 = oddsFaced)
///The lower oddsFaced is, the better his/her odds are.
///On a side note, if you're looking at heads up, and I bet x/3 of the pot, I'm giving him 4:1 or 0.2
///A quick way to figure out what to put for oddsFaced would be to say:
///If this is a permanently-even bet to make, what would my chance to win be?
float64 CallCumulation::pctWillCall_tiefactor(const float64 oddsFaced, const float64 tiefactor) const
{

/*    if( (oddsFaced - .63 < 0.0001 && oddsFaced - .63 > -0.0001) || (oddsFaced - .48 < 0.0001 && oddsFaced - .48 > -0.0001) )
    {
        searchGap(oddsFaced);
    }*/

    const size_t maxsize = cumulation.size();
	const size_t guess = searchGap(oddsFaced);
	if( guess  == maxsize +1 ) return 0;
	if( guess == maxsize ) return 1;
	float64 curPCT = cumulation[guess].repeated;
	if( curPCT == oddsFaced )
	{
		if( guess == 0 ) return 0;
		return cumulation[guess].repeated * tiefactor + cumulation[guess-1].repeated * (1 - tiefactor);
	}
	#ifdef SMOOTHED_CALLCUMULATION
		if( guess == maxsize -1 || guess == 0 )
        {
            return curPCT;
        }

/*        if( (oddsFaced - .63 < 0.0001 && oddsFaced - .63 > -0.0001) || (oddsFaced - .48 < 0.0001 && oddsFaced - .48 > -0.0001) )
        {
         const StatResult& sn1 = cumulation[guess-1];
         const StatResult& s0 = cumulation[guess];
         const StatResult& s2 = cumulation[guess+1];
         float64 s = (
                cumulation[guess].repeated * (oddsFaced - cumulation[guess+1].pct)
                +
                cumulation[guess+1].repeated * (cumulation[guess].pct - oddsFaced)
              )
              / (cumulation[guess].pct - cumulation[guess+1].pct)
                ;
         float64 h = (cumulation[guess].pct - cumulation[guess+1].pct);
        }
*/
        curPCT =
            (
                cumulation[guess].repeated * (oddsFaced - cumulation[guess+1].pct)
                +
                cumulation[guess+1].repeated * (cumulation[guess].pct - oddsFaced)
            )
            / (cumulation[guess].pct - cumulation[guess+1].pct)
                ;
	#endif
	return curPCT;

}

float64 CallCumulation::pctWillCall(const float64 oddsFaced) const
{
    return pctWillCall_tiefactor(oddsFaced, DEFAULT_TIE_SCALE_FACTOR);
}

StatResult CallCumulation::strongestOpponent() const
{
    #ifdef DEBUGASSERT
        if( cumulation.size() == 0 )
        {
            std::cout << "EMPTY CALLCUMULATION!";
            exit(1);
        }
    #endif

    StatResult retVal;
    retVal = cumulation[0];
    retVal.pct = 1 - retVal.pct;
    return retVal;
}

StatResult CallCumulation::weakestOpponent() const
{
    #ifdef DEBUGASSERT
        if( cumulation.size() == 0 )
        {
            std::cout << "EMPTY CALLCUMULATION!";
            exit(1);
        }
    #endif
    StatResult retVal;
    retVal = cumulation[cumulation.size()-1];
    retVal.pct = 1 - retVal.pct;
    return retVal;
}


float64 CallCumulationD::slopeof(const size_t x1, const size_t x0) const
{
    const float64& y0 = cumulation[x0].repeated;
    const float64& y1 = cumulation[x1].repeated;
    const float64& p0 = cumulation[x0].pct;
    const float64& p1 = cumulation[x1].pct;
	return (y1 - y0)/(p1 - p0);
}


float64 CallCumulationD::pctWillCallD(const float64 oddsFaced) const
{
	const size_t maxsize = cumulation.size();
	if( maxsize <= 1 ) return 0;
	const size_t guess = searchGap(oddsFaced);


	if( guess >= maxsize )
    {
        return 0; //This is either less than 0 or more than maxsize-1, to return 0 slope
    }
	if( guess == maxsize -1 )
	{
	    return slopeof(maxsize-1,maxsize-2);
	}

	if( guess == 0 ) return slopeof(1,0);
	float64 curPCT = cumulation[guess].repeated;
	if( curPCT == oddsFaced )
	{
		return slopeof(guess+1,guess-1);
	}
	#ifdef SMOOTHED_CALLCUMULATION_D
	if( guess == 1 || guess == maxsize-2 )
	#endif
	{
        return slopeof(guess,guess-1);
	}
	#ifdef SMOOTHED_CALLCUMULATION_D
	if( guess == 2 || guess == maxsize-3 )
	{
	    return slopeof(guess+1,guess-2);
	}
    return slopeof(guess+2,guess-3);
    #endif
}

float64 SlidingPairCallCumulationD::pctWillCall(const float64 oddsFaced) const
{
    return left->pctWillCall(oddsFaced) * (1-slider) + right->pctWillCall(oddsFaced) * (slider);
}

float64 SlidingPairCallCumulationD::pctWillCallD(const float64 oddsFaced) const
{
    return left->pctWillCallD(oddsFaced) * (1-slider) + right->pctWillCallD(oddsFaced) * (slider);
}
/*
float64 CallCumulation::pctWillCallDEBUG(const float64 oddsFaced, const float64 tiefactor) const
{
	//binary search
	size_t first=0;
	size_t last=cumulation.size()-1;
	size_t guess;
	float64 curPCT;


    ///Binary search for oddsFaced
    ///Post-analysis of the algorithm defines a DECREASING pct in cumulation
	while(last > first)
	{
		guess = (last+first)/2;
		curPCT = cumulation[guess].pct;

		if (curPCT < oddsFaced)
		{

			if (guess == first)
			{ //A. guess == first implies last == first + 1
				if (guess == 0) return 0;
				return cumulation[guess-1].repeated;
			}
			last = guess - 1;
		}
		else if (curPCT > oddsFaced)
		{
			if (guess + 1 == last)
			{ //B. first  + 1 == last - 1
				if ( cumulation[last].pct > oddsFaced )
				{
					++last;
					if (last == cumulation.size() ) return 1;
					return cumulation[last].repeated;
				}
				else if (cumulation[last].pct < oddsFaced)
				{
					return cumulation[guess].repeated;
				}
				else
				{// cumulation[last].pct == oddsFaced
					return cumulation[last].repeated;
				}
			}
			first = guess + 1;
		}
		else
		{///The odds faced are EXACTLY the chance to win. How many people would take this bet?
            ///Let's scale this by the scalefactor
            if( guess == 0 ) return cumulation[guess].repeated;
			return cumulation[guess].repeated * tiefactor + cumulation[guess-1].repeated * (1 - tiefactor);
		}
	}
	curPCT = cumulation[last].pct;
	if(curPCT < oddsFaced)
	{
		if (last == 0)
		{
			return 0;
		}
		return cumulation[last-1].repeated;
	}
	else
	{
		return cumulation[last].repeated;
	}

}
*/
//Search for the first element with a lower .pct and oddsFaced from a descending set in a binary manner
//The last element of cumulation represents my weakest hand. This also represents the opponent's strongest hand.
//As such, the largest "call number" will occur at the bottom.
//So for any given odds, we want to find the .repeated of the NEXT ELEMENT DOWN (we list front to back : 0 to size()-1 : top to bottom)
//because that is the cumulative sum of all the "repeated" that are better (below)
size_t CallCumulation::searchGap(const float64 oddsFaced) const
{
	//binary search
	size_t first=0;
	size_t last=cumulation.size()-1;
	size_t guess;
	float64 curPCT;


    ///Binary search for oddsFaced
    ///Post-analysis of the algorithm defines a DECREASING pct in cumulation
	while(last > first)
	{
		guess = (last+first)/2;
		curPCT = cumulation[guess].pct;

		if (curPCT < oddsFaced)
		{

			if (guess == first)
			{ //A. guess == first implies last == first + 1
				if (guess == 0) return cumulation.size()+1;
				return guess-1;
			}
			last = guess;
		}
		else if (curPCT > oddsFaced)
		{
			if (guess + 1 == last)
			{ //B. first  + 1 == last - 1
				if ( cumulation[last].pct > oddsFaced )
				{
					++last;
					if (last == cumulation.size() ) return cumulation.size();
					return last;
				}
				else if (cumulation[last].pct < oddsFaced)
				{
					return guess;
				}
				else
				{// cumulation[last].pct == oddsFaced
					return last;
				}
			}
			first = guess;
		}
		else
		{///The odds faced are EXACTLY the chance to win. How many people would take this bet?
            ///Let's scale this by the scalefactor
            //if( guess == 0 ) return cumulation[guess].repeated;
			return guess; //Please check if(cumulation[searchGap(oddsFaced)].pct == oddsFaced) then SCALEFACTOR
			//return cumulation[guess].repeated * tiefactor + cumulation[guess-1].repeated * (1 - tiefactor);
		}
	}
	curPCT = cumulation[last].pct;
	if(curPCT < oddsFaced)
	{
		if (last == 0)
		{
			return cumulation.size()+1;
		}
		return last-1;
	}
	else
	{
		return last;
	}

}

const CallCumulation & CallCumulation::operator=(const CallCumulation& o)
{
    cumulation = o.cumulation;
    return *this;
}

const DistrShape & DistrShape::operator=(const DistrShape& o)
{
    avgDev = o.avgDev;
    improve = o.improve;
    kurtosis = o.kurtosis;
    mean = o.mean;
    n = o.n;
    skew = o.skew;
    stdDev= o.stdDev;
    worst = o.worst;
    return *this;
}

