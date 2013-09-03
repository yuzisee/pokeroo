/***************************************************************************
 *   Copyright (C) 2009 by Joseph Huang                                    *
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

    #ifdef DUMP_CSV_PLOTS
        #include <iostream>
    #endif





#define COMPENSATE_FOR_BAD_ROUNDING


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

    /**
     * "Add in" the StatResult given, according to its .repeated value
     * This StatResult will be modified to become the weighted average of the two StatResults (weighted by both .repeated values) and
     * our .repeated itself will be replaced with the sum of the two .repeated to represent that both outcomes have been added into one aggregate.
     *
     * Examples:
     *  if a.repeated is 0.0, this is unchanged.
     *  if this->repeated is 0.0, effectively this := a
     *
     *  NOTE: behaviour is undefined if both a.repeated and this->repeated are zero, or either are negative.
     */
    const StatResult addByWeight(const StatResult &a) {
        const float64 netRepeated = this->repeated + a.repeated;

        this->wins   = (  this->wins * this->repeated +   a.wins * a.repeated) / netRepeated;
        this->splits = (this->splits * this->repeated + a.splits * a.repeated) / netRepeated;
        this->loss   = (  this->loss * this->repeated +   a.loss * a.repeated) / netRepeated;
        this->pct    = (   this->pct * this->repeated +    a.pct * a.repeated) / netRepeated;

        this->repeated = netRepeated;
    }

	float64 wins;
	float64 splits;
	float64 loss;
	float64 repeated; // weight this result (e.g. for combining two StatResult objects)

	float64 pct;

	//float64 wlr;
	//float64 pyth;

	StatResult ReversedPerspective() const
	{
		StatResult temp = *this;
		temp.wins = 1 - splits - wins;
		temp.loss = 1 - splits - loss;
		temp.genPCT();

		return (temp);
	}

	void genPCT()
	{

		#ifdef COMPENSATE_FOR_BAD_ROUNDING
		bool bRenorm = false;
		if( wins < 0 ){ wins = 0; bRenorm = true; }
		if( loss < 0 ){ loss = 0; bRenorm = true; }
		if( splits < 0 ){ splits = 0; bRenorm = true; }

		if( bRenorm )
		{
		    float64 nt = wins + loss + splits;
		    wins /= nt;
		    loss /= nt;
		    splits /= nt;
		}
		#endif

		pct = wins + splits/2;

	}

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



    void deterBy(float64 w)
    {
        this->wins -= w;
        this->loss += w;
        this->pct -= w;
        //It is possible for zero crossings
        if( this->pct < 0 || this->wins < 0 )
        {
            this->wins = 0;
            this->loss = 1 - this->splits;
            genPCT();
        }
    }

    void boostBy(float64 w)
    {
        this->wins += w;
        this->loss -= w;
        this->pct += w;

        if( this->pct > 1 || this->wins > 1-(this->splits) )
        {
            this->wins = 1 - this->splits;
            this->loss = 0;
			genPCT();
        }
    }

    void forceRenormalize()
    {
        const float64 scaleTotal = wins + splits + loss;
        wins /= scaleTotal;
        splits /= scaleTotal;
        loss /= scaleTotal;
        genPCT();
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
private:
    size_t cached_high_index;
protected:
	size_t searchWinPCT_betterThan_toHave(const float64 winPCT_toHave) const;

    float64 sampleInBounds_pct(size_t x) const;
    float64 sampleInBounds_repeated(size_t x) const;

public:
    CallCumulation(const CallCumulation& o)
    {
        *this = o;
    }
    CallCumulation() : cached_high_index(1) {}
    virtual ~CallCumulation();
    const CallCumulation & operator=(const CallCumulation& o);

    vector<StatResult> cumulation;

    virtual void ReversePerspective();


	virtual float64 Pr_haveWinPCT_orbetter(const float64 w_toHave) const;
	virtual float64 nearest_winPCT_given_rank(const float64 rank);
	virtual StatResult bestHandToHave() const; // best hand to have against me
	virtual StatResult worstHandToHave() const; // worst hand to have against me
	virtual StatResult oddsAgainstBestHand() const;
	virtual StatResult oddsAgainstBestXHands(float64 X) const; // Here, X is the fraction of hands we care about. If X === 0.0, this returns oddsAgainstBestHand(). If X === 1.0, this means just include all hands and effectively returns statmean.

	#ifdef DUMP_CSV_PLOTS
        static void dump_csv_plots(std::ostream &targetoutput, const CallCumulation& calc)
        {
            targetoutput << std::endl << "=============Reduced=============" << std::endl;
            targetoutput.precision(4);
            size_t vectorLast = calc.cumulation.size();
            targetoutput << std::endl << "x, hand";
            for(size_t i=0;i<vectorLast;i++)
            {
                targetoutput << std::endl << calc.cumulation[i].repeated  << ", "<< calc.cumulation[i].pct<< std::flush;
                //targetoutput << std::endl << "{" << i << "}" << calc.cumulation[i].loss << " l +\t"
                //        << calc.cumulation[i].splits << " s +\t" << calc.cumulation[i].wins << " w =\t" <<
                //        calc.cumulation[i].pct
                //        << " pct\tx;"<< calc.cumulation[i].repeated << std::flush;
            }
        }
    #endif

}
;

class CallCumulationD : public virtual CallCumulation
{
private:
    float64 linearInterpolate(float64 x1, float64 y1, float64 x2, float64 y2, float64 x) const;
	virtual float64 slopeof(size_t x10, size_t x11, size_t x20, size_t x21, size_t, size_t) const;
public:
    float64 d_dw_only(const float64 w_toHave) const;
	virtual float64 Pr_haveWinPCT_orbetter_continuous(const float64 w_toHave, float64 *out_d_dw = 0) const;
	virtual float64 inverseD(const float64, const float64 mean);

/*
        #ifdef DUMP_CSV_PLOTS
            void dump_csv_plots(float64 points, std::ostream& target)
            {


                target << "midpoint,exf,dexf,mandexf" << std::endl;
                for( size_t elementNum=1;elementNum<cumulation.size();++elementNum)
                {
                    float64 midpoint = (cumulation[elementNum-1].pct + cumulation[elementNum].pct)/2;
                    float64 exf = pctWillCall(midpoint);
                    float64 dexf = pctWillCallD(midpoint);
                    float64 mandexf = slopeof(elementNum-1,elementNum);
                    target << midpoint << "," << exf << "," << dexf << "," << mandexf <<// "," << exf << "," << dexf <<
                     std::endl;

                }


            }
        #endif
*/

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

    #ifdef DUMP_CSV_PLOTS
        static void dump_csv_plots(std::ostream& targetoutput, const DistrShape& myDistrPCT)
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
