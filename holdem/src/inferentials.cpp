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

float64 CallCumulation::pct(const float64 oddsFaced) const
{
	//binary search
	size_t first=0;
	size_t last=cumulation.size()-1;
	size_t guess;
	float64 curPCT;



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
		{
			return cumulation[guess].repeated;
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
