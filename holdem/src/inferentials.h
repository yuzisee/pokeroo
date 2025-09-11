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
#include <utility>
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
		StatResult(const StatResult& a) : wins(a.wins), splits(a.splits), loss(a.loss), repeated(a.repeated), pct(a.pct) {}
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
    void scaleOrdinateOnly(float64 scale) {
        wins *= scale;
        splits *= scale;
        loss *= scale;
        // Doesn't scale .repeated (the Abscissa)
        pct *= scale;
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

        return *this;
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


//In CallCumulation[n]
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
//AA has .pct = 0.832

//32o is probably cumulation[low]
//32o has .wins = 29.2%
//32o has .splits = 6.12%
//32o has .loss = 64.6%
//32o has .repeated = 0.00980
//32o has .pct = 0.323

// example CallCumulation
/*
 [0] = {
 wins = 0.03636363636363636
 splits = 0.1303030303030303
 loss = 0.8333333333333334
 repeated = 0.002775208140610546
 pct = 0.1015151515151515
 }
 [1] = {
 wins = 0.04545454545454546
 splits = 0.1212121212121212
 loss = 0.8333333333333334
 repeated = 0.01110083256244218
 pct = 0.1060606060606061
 }
 .
 .
 .
 [91] = {
 wins = 0.8383838383838383
 splits = 0.05252525252525252
 loss = 0.1090909090909091
 repeated = 0.9740980573543015
 pct = 0.8646464646464647
 }
 [92] = {
 wins = 0.8464646464646465
 splits = 0.05252525252525252
 loss = 0.101010101010101
 repeated = 1
 pct = 0.8727272727272727
 }

 */

class CallCumulation
{
private:
    size_t cached_high_index; // This is just an extra first guess we will make.
protected:
	size_t searchWinPCT_strictlyBetterThan(const float64 winPCT) const;

    float64 sampleSafe_prevrepeated(size_t x) const;
    //float64 sampleInBounds_pct(size_t x) const;
    //float64 sampleInBounds_repeated(size_t x) const;

public:
    CallCumulation(const CallCumulation& o)
    : cached_high_index(1)
    {
        *this = o;
    }
    CallCumulation() : cached_high_index(1) {}
    virtual ~CallCumulation();
    const CallCumulation & operator=(const CallCumulation& o);

    vector<StatResult> cumulation;

    virtual void ReversePerspective();


	virtual float64 Pr_haveWinPCT_strictlyBetterThan(const float64 w_toHave) const;
	virtual float64 nearest_winPCT_given_rank(const float64 rank);
	virtual StatResult bestHandToHave() const;
	virtual StatResult worstHandToHave() const;

    // Here, X is the fraction of hands we care about. If X === 0.0, this returns oddsAgainstBestHand(). If X === 1.0, this means just include all hands and effectively returns statmean.
    // Returns both the StatResult at X, as well as d_{StatResult.pct}_dX
    // The two return values are returned as an std::pair
	virtual std::pair<StatResult, float64> bestXHands(float64 X) const;

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
	//virtual float64 slopeof(size_t x10, size_t x11, size_t x20, size_t x21, size_t, size_t) const;
public:

    // Actual and Derivative of {1.0-Pr_haveWinPct_strictlyBetterThan}
    virtual std::pair<float64, float64> Pr_haveWorsePCT_continuous(const float64 w_toHave) const;

    // derivative of nearestWinPctGivenRank
	virtual float64 inverseD(const float64, const float64 mean) const;

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


#define COARSE_COMMUNITY_NUM_BINS 7

class DistrShape
{
protected:
	void normalize(float64);
public:
	float64 n;
	StatResult mean;

    StatResult best;
	StatResult worst;

    StatResult coarseHistogram[COARSE_COMMUNITY_NUM_BINS];

    // Stats about PCT
	float64 avgDev;
	float64 stdDev;   //raw moment (i.e. divided by n not n-1)
	float64 improve_numerator;
	float64 skew;     //positive or negative ("Distributions with positive skew have larger means than medians.")

	//risk-reward magnifier (high k is high risk high reward, long tail)
	float64 pearson_kurtosis_numerator;
	float64 pearson_kurtosis_denominator;
	inline constexpr float64 kurtosis() const {
	  return pearson_kurtosis_numerator / pearson_kurtosis_denominator - 3.0;
  }

  // Return the fraction of outcomes that cause your gain function f(x) to be above vs. below 1.0
  // ...but instead of [0.0..1.0] we rescale to [-1.0..+1.0]
  inline constexpr float64 improve() const {
    // Rescale from
    return
      (improve_numerator / n  // [0.0..1.0]
      * 2.0)  -               // into
                 1.0;         // [-1.0..+1.0]
  }

 // Return the fraction of outcomes that cause your gain function f(x) to be above vs. below 1.0
  // ...but instead of [0.0..1.0] we rescale to [-1.0..+1.0]
  inline constexpr float64 improve() const {
    // Rescale from
    return
      (improve_numerator / n  // [0.0..1.0]
      * 2.0)  -               // into
                 1.0;         // [-1.0..+1.0]
  }

        DistrShape(const DistrShape &o)
        {
            *this = o;
        }

	static DistrShape newEmptyDistrShape();

	DistrShape(float64 count, StatResult worstPCT, StatResult meanOverall, StatResult bestPCT);

    const DistrShape & operator=(const DistrShape& o);

	void AddVal(const StatResult &);

	void Complete(float64);
private:
	void Complete();

    #ifdef DUMP_CSV_PLOTS
        static void dump_csv_plots(std::ostream& targetoutput, const DistrShape& myDistrPCT)
        {

            //targetoutput << "myAvg.genPCT " << myWins.pct << "!"  << endl;
            targetoutput << "(Mean) " << myDistrPCT.mean * 100 << "%"  << std::endl;
            targetoutput << std::endl << "Adjusted improve? " << myDistrPCT.improve() * 100 << "%"  << std::endl;
            targetoutput << "Worst:" << myDistrPCT.worst *100 << "%" << std::endl;
            targetoutput << "Standard Deviations:" << myDistrPCT.stdDev*100 << "%" << std::endl;
            targetoutput << "Average Absolute Fluctuation:" << myDistrPCT.avgDev*100 << "%" << std::endl;
            targetoutput << "Skew:" << myDistrPCT.skew*100 << "%" << std::endl;
            targetoutput << "Kurtosis:" << (myDistrPCT.kurtosis())*100 << "%" << std::endl;

            targetoutput << std::endl;
        }

    #endif
} // DistrShape
;

// TODO(from yuzisee): For now, this is used to denote whether FoldGain reads the MEAN or RANK from CoreProbabilities
// We should make this automatic based on the number of players for which our win probability is being evaluated.
// Extra discussion: With large folds, if you call is it less likely for others to call?
//   On the one hand, if it's better to be pessimistic here, we can assume no which also ensures that FoldGain is always MEAN and not RANK.
//   On the other hand, if you do call, the payout improves for the third player (although their odds decrease.)
enum MeanOrRank {
    MEAN,
    RANK
};

#define EPS_WIN_PCT (1.0 / 2118760.0 / 990.0)

struct CoreProbabilities {
    int8 playerID;

    CallCumulationD handcumu; // CallStats (your odds of winning against each possible opponent)
    CallCumulationD foldcumu; // CallStats, REVERSED PERSPECTIVE (each opposing hand's chance to win)
    CallCumulationD callcumu; // CommunityCallStats (each hole cards' inherent probability of winning)
	StatResult statmean; // (Your hole cards' current inherent probability of winning)

    CoreProbabilities() : playerID(-1) {}

    // Against what fraction of opponents will you have the better hand?
    // This is high for good all-in hands.
    StatResult statRelation() {
        StatResult statrelation;
        const float64 strictlyLosingOutcomes = foldcumu.Pr_haveWinPCT_strictlyBetterThan(0.5); // how often does your opponent have better than 50% against you
        const float64 strictlyWinningOutcomes = handcumu.Pr_haveWinPCT_strictlyBetterThan(0.5); // how often you have better than 50% against your opponent

        //You can tie in rank if and only if you tie in mean
        statrelation.wins = strictlyWinningOutcomes;
        statrelation.loss = strictlyLosingOutcomes;
        statrelation.splits = 1.0 - statrelation.wins - statrelation.loss;
        statrelation.forceRenormalize();
        return statrelation;
    }

    // How often do you get a hand this good? (Here, "good" = raw percentage chance to win)
    // This is high for strong playing hands.
    StatResult statRanking() {
        StatResult statranking;
        const float64 handsThatCanDoBetterThanYou = callcumu.Pr_haveWinPCT_strictlyBetterThan(statmean.pct);
        const float64 handsThatYouCanDoBetterThan = 1.0 - callcumu.Pr_haveWinPCT_strictlyBetterThan(statmean.pct - EPS_WIN_PCT);

        // TODO(from yuzisee): Should this be pct preserving instead? If so, what if pct < splits?
        // TODO(from yuzisee): Perhaps win should be the probability of having strictly better hand and lose should be probability of having strictly worse.
        //                     Then we can be ratio preserving. Right now ranking is biased high.
        //                     The problem is, when you get to the showdown, outcomes are known, so hands only have W L or S.
        //                     Then, "strictly better" might be small compared to splits. Maybe that's okay.
        // TODO(from yuzisee): Maybe split can be probability of having the same hand?
        statranking.loss = handsThatCanDoBetterThanYou;
        statranking.wins = handsThatYouCanDoBetterThan;
        statranking.splits = 1.0 - statranking.loss - statranking.wins;
        statranking.forceRenormalize();
        return statranking;
    }

    double getFractionOfOpponentsWhereMyWinrateIsAtLeast(double winrate) {
        return handcumu.Pr_haveWinPCT_strictlyBetterThan(winrate);
    }

    // TODO(from joseph_huang): Refactor this (or where it is used) to supply mean vs. rank when dealing with heads-up vs. multi-hand.
}
;


#endif
