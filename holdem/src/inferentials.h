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

#define DEFAULT_TIE_SCALE_FACTOR 0.5

//#define DEBUGLOGINFERENTIALS
    #ifdef DEBUGLOGINFERENTIALS
        #include <iostream>
    #endif
//#define DEBUG_DEXF
//#include <iostream>

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

	const StatResult operator+(const StatResult& b) const
	{
	    StatResult temp;
		temp.wins = wins + b.wins;
		temp.splits = splits + b.splits;
		temp.loss = loss + b.loss;
		temp.repeated = repeated + b.repeated;
		temp.pct = pct + b.pct;

		return (temp);
	}

	const StatResult operator*(const float64& fx) const
	{
	    StatResult temp;
		temp.wins = wins * fx;
		temp.splits = splits * fx;
		temp.loss = loss * fx;
		temp.repeated = repeated * fx;
		temp.pct = pct * fx;

		return (temp);
	}

    const StatResult operator/(const float64& fx) const
	{
	    StatResult temp;
		temp.wins = wins / fx;
		temp.splits = splits / fx;
		temp.loss = loss / fx;
		temp.repeated = repeated / fx;
		temp.pct = pct / fx;

		return (temp);
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
	float64 genPeripheral() const
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


//In CallCumulation[n], .pct represents the opponents PCT if YOU had hand #n
//.wins, .splits, .loss represent the YOUR rating with hand #n
//.repeated is rank (1.0 for the best hand to have, and slightly above 0 for the worst hand to have)
//1 - .repeated  is rarity (chance of having this or better)
//INVARIANT: 84.9% is the win percentage of AA preflop
//(http://seoblackhat.com/texas-hold-em-poker-statistics/)
//AA is cumulation[168]
//AA has .wins = 84.9%
//AA has .splits = 0.54%
//AA has .loss = 14.6%
//AA has .repeated = 1
//AA has .pct = 0.148

//32o is probably cumulation[0]
//32o has .wins = 29.2%
//32o has .splits = 6.12%
//32o has .loss = 64.6%
//32o has .repeated = 0.00980
//32o has .pct = 0.677

class CallCumulation
{

protected:
	size_t searchWinPCT_betterThan_toHave(const float64 winPCT_toHave) const;

public:
    CallCumulation(const CallCumulation& o)
    {
        *this = o;
    }
    CallCumulation(){}
    virtual ~CallCumulation();
    const CallCumulation & operator=(const CallCumulation& o);

	vector<StatResult> cumulation;
	virtual float64 Pr_haveWinPCT_orbetter(const float64 w_toHave) const;
	virtual float64 nearest_winPCT_given_rank(const float64 rank) const;
	virtual StatResult bestHandToHave() const;
	virtual StatResult worstHandToHave() const;

	#ifdef DEBUGLOGINFERENTIALS
        static void displayCallCumulation(std::ostream &targetoutput, const CallCumulation& calc)
        {
            targetoutput << std::endl << "=============Reduced=============" << std::endl;
            targetoutput.precision(4);
            size_t vectorLast = calc.cumulation.size();
            for(size_t i=0;i<vectorLast;i++)
            {
                targetoutput << std::endl << "{" << i << "}" << calc.cumulation[i].loss << " l +\t"
                        << calc.cumulation[i].splits << " s +\t" << calc.cumulation[i].wins << " w =\t" <<
                        calc.cumulation[i].pct
                        << " pct\tx;"<< calc.cumulation[i].repeated << std::flush;
            }
        }
    #endif

}
;

class CallCumulationD : public virtual CallCumulation
{
private:
    float64 sampleInBounds_pct(size_t x) const;
    float64 sampleInBounds_repeated(size_t x) const;
    float64 linearInterpolate(float64 x1, float64 y1, float64 x2, float64 y2, float64 x) const;
	virtual float64 slopeof(size_t x10, size_t x11, size_t x20, size_t x21, size_t, size_t) const;
public:
    float64 d_dw_only(const float64 w_toHave) const;
	virtual float64 Pr_haveWinPCT_orbetter_continuous(const float64 w_toHave, float64 *out_d_dw = 0) const;
	virtual float64 inverseD(const float64) const;

        #ifdef DEBUG_DEXF
            void breakdown(float64 points, std::ostream& target)
            {


                target << "midpoint,exf,dexf,mandexf" << std::endl;
                for( size_t elementNum=1;elementNum<cumulation.size();++elementNum)
                {
                    float64 midpoint = (cumulation[elementNum-1].pct + cumulation[elementNum].pct)/2;
                    float64 exf = pctWillCall(midpoint);
                    float64 dexf = pctWillCallD(midpoint);
                    float64 mandexf = slopeof(elementNum-1,elementNum);
                    target << midpoint << "," << exf << "," << dexf << "," << mandexf <</* "," << exf << "," << dexf <<*/ std::endl;

                }


            }
        #endif
}
;

class CallCumulationZero : public virtual CallCumulationD
{
    virtual float64 pctWillCall(const float64 oddsFaced) const
    {return 0;}
    virtual float64 pctWillCallD(const float64 oddsFaced) const
    {return 0;}
}
;

class CallCumulationFlat : public virtual CallCumulationD
{
    virtual float64 pctWillCall(const float64 oddsFaced) const
    {return 1-oddsFaced;}
    virtual float64 pctWillCallD(const float64 oddsFaced) const
    {return -1;}
}
;
/*
class SlidingPairCallCumulationD : public virtual CallCumulationD
{
    protected:
        float64 slider;
        const CallCumulationD *left;
        const CallCumulationD *right;
    public:
        SlidingPairCallCumulationD( const CallCumulationD* e_left, const CallCumulationD* e_right, const float64 scale )
        : slider( scale ), left(e_left), right(e_right)
        {}
        virtual float64 pctWillCall(const float64 oddsFaced) const;
        virtual float64 pctWillCallD(const float64 oddsFaced) const;
}
;
*/

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

        DistrShape(const DistrShape &o)
        {
            *this = o;
        }

	DistrShape(float64 u) { DistrShape(0,u); }
	DistrShape(float64 count, float64 u) : n(count), mean(u), worst(u), avgDev(0), stdDev(0), improve(0), skew(0), kurtosis(0) {}

	void AddVal(float64, float64);
	void AddCount(float64 x, float64 occ){ n += occ; AddVal(x, occ); }
	void Complete();
	void Complete(float64);

    const DistrShape & operator=(const DistrShape& o);

    #ifdef DEBUGLOGINFERENTIALS
        static void displayDistr(std::ostream& targetoutput, const DistrShape& myDistrPCT)
        {

            //targetoutput << "myAvg.genPCT " << myWins.pct << "!"  << endl;
            targetoutput << "(Mean) " << myDistrPCT.mean * 100 << "%"  << std::endl;
            targetoutput << std::endl << "Adjusted improve? " << myDistrPCT.improve * 100 << "%"  << std::endl;
            targetoutput << "Worst:" << myDistrPCT.worst *100 << "%" << std::endl;
            targetoutput << "Standard Deviations:" << myDistrPCT.stdDev*100 << "%" << std::endl;
            targetoutput << "Average Absolute Fluctuation:" << myDistrPCT.avgDev*100 << "%" << std::endl;
            targetoutput << "Skew:" << myDistrPCT.skew*100 << "%" << std::endl;
            targetoutput << "Kurtosis:" << (myDistrPCT.kurtosis)*100 << "%" << std::endl;

            targetoutput << std::endl;
        }

    #endif
}
;

#endif
