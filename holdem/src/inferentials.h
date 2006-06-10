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


#ifndef HOLDEM_STATMODELS
#define HOLDEM_STATMODELS

#include <vector>
#include "portability.h"
using std::vector;

class StatResult
{
	public:
		StatResult() : wins(0), splits(0), loss(0), repeated(1), pct(0) {}
	bool operator> (const StatResult& x) const
	{	return pct > x.pct;	}
	bool operator< (const StatResult& x) const
	{	return pct < x.pct;	}
	bool operator== (const StatResult& x) const
	{	return pct == x.pct;	}
	bool bIdenticalTo (const StatResult& x) const
	{
		return	wins == x.wins
				&&
				splits == x.splits
				&&
				loss == x.loss
				&&
				pct == x.pct;
	}

	const StatResult & operator=(const StatResult& a)
	{
		wins = a.wins;
		splits = a.splits;
		loss = a.loss;
		repeated = a.repeated;
		pct = a.pct;

		return *this;
	}

	float64 wins;
	float64 splits;
	float64 loss;
	float64 repeated;

	float64 pct;

	//float64 wlr;
	//float64 pyth;

	void genPCT()
	{
		pct = wins + splits/2;
	}
	/*
	void genPeripheral()
	{
		if( loss == 0 && wins == 0 )
		{ //All split maybe?
			wlr = 0;
			pyth = 0;
		}else
		{
			wlr = wins*wins; //temporary storage!!
			pyth = wlr / ( wlr + loss * loss);
			wlr = wins / (wins + loss);
		}
		wlr *= (wins+loss+splits);
		pyth *= (wins+loss+splits);
	}
	*/
	float64 genPeripheral()
	{
		if( loss == 0 && wins == 0 )
		{ //All split maybe?
			return 0;
		}else
		{
			return wins / (wins + loss) * (wins + loss + splits);
		}
	}
}
;
/*
class StatResultZero : public StatResult
{
	StatResultZero() : StatResult()
	{
		StatResult::repeated = 0;
	}
}
;
*/


class CallCumulation
{
public:
	vector<StatResult> cumulation;
	float64 pct(const float64) const;
}
;

class DistrShape
{
protected:
	void normalize(float64);
public:
	float64 n;
	float64 mean;

	float64 worst;

	float64 avgDev;
	float64 stdDev;
	float64 improve;  //above or below 1.0
	float64 skew;     //positive or negative ("Distributions with positive skew have larger means than medians.")
	float64 kurtosis; //risk-reward magnifier (high k is high risk high reward, long tail)

	DistrShape(float64 u) { DistrShape(0,u); }
	DistrShape(float64 count, float64 u) : n(count), mean(u), worst(u), avgDev(0), stdDev(0), improve(0), skew(0), kurtosis(0) {}

	void AddVal(float64, float64);
	void AddCount(float64 x, float64 occ){ n += occ; AddVal(x, occ); }
	void Complete();
	void Complete(float64);
}
;

#endif
