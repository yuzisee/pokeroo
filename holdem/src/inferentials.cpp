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

float64 CallCumulation::pctWillCall(const float64 oddsFaced) const
{
    return pctWillCall(oddsFaced,DEFAULT_TIE_SCALE_FACTOR);
}

///oddsFaced is the "virtual chance to win" that would correspond to the pot odds faced.
///eg. 4:1 pot odds means opponent can bet 1 to win 4, (ie. pay 1 to receive 5 = 20% = 0.2 = oddsFaced)
///The lower oddsFaced is, the better his/her odds are.
///On a side note, if you're looking at heads up, and I bet x/3 of the pot, I'm giving him 4:1 or 0.2
///A quick way to figure out what to put for oddsFaced would be to say:
///If this is a permanently-even bet to make, what would my chance to win be?
float64 CallCumulation::pctWillCall(const float64 oddsFaced, const float64 tiefactor) const
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

