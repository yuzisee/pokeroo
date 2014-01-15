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

#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include "inferentials.h"

//#include <iostream>

#define SMOOTHED_CALLCUMULATION_D
#define ACCELERATE_SEARCH_MINIMUM 64
//const float64 CallCumulation::tiefactor = DEFAULT_TIE_SCALE_FACTOR;


DistrShape DistrShape::newEmptyDistrShape() {
    StatResult dummy;
    dummy.wins = std::numeric_limits<float64>::signaling_NaN();
    dummy.loss = std::numeric_limits<float64>::signaling_NaN();
    dummy.splits = std::numeric_limits<float64>::signaling_NaN();
    dummy.repeated = std::numeric_limits<float64>::signaling_NaN();
    dummy.pct = std::numeric_limits<float64>::signaling_NaN();

    DistrShape result(0.0, dummy, dummy, dummy);
    return result;
}

DistrShape::DistrShape(float64 count, StatResult worstPCT, StatResult meanOverall, StatResult bestPCT)
: n(count)
,
mean(meanOverall), best(bestPCT), worst(worstPCT)
,
avgDev(0), stdDev(0), improve(0), skew(0), kurtosis(0)
{
    for (size_t k=0; k<COARSE_COMMUNITY_NUM_BINS; ++k) {
        coarseHistogram[k].wins = 0.0;
        coarseHistogram[k].splits = 0.0;
        coarseHistogram[k].loss = 0.0;
        coarseHistogram[k].repeated = 0.0;
        coarseHistogram[k].pct = 0.5;
    }
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
    best = o.best;
    for (size_t k=0; k<COARSE_COMMUNITY_NUM_BINS; ++k) {
        coarseHistogram[k] = o.coarseHistogram[k];
    }
    return *this;
}


void DistrShape::AddVal(const StatResult &x)
{
    const float64 occ = x.repeated;

    const float64 dx = (best.pct - worst.pct) / COARSE_COMMUNITY_NUM_BINS;

    // Store into a histogram
    if (dx > 0.0) {
        // Which of the COARSE_COMMUNITY_NUM_BINS do we fall into?
        const float64 bin0 = worst.pct + dx/2;
        const float64 bin = round((x.pct - bin0)/dx);

        size_t binIdx;
        if (bin == -1.0) {
            binIdx = 0;
        } else if (bin == COARSE_COMMUNITY_NUM_BINS) {
            binIdx = COARSE_COMMUNITY_NUM_BINS - 1;
        } else if (0.0 <= bin && bin < COARSE_COMMUNITY_NUM_BINS) {
            binIdx = static_cast<size_t>(bin);
        } else {
            binIdx = -1;
            abort(); // CRASH this value is invalid.
        }

        coarseHistogram[binIdx].addByWeight(x);
    }

    // Apply "PCT only" summary statistics
	const float64 d = (x.pct - mean.pct);
	const float64 d1 = d * occ;
	const float64 d2 = d1*d;
	const float64 d3 = d2*d;
	const float64 d4 = d3*d;

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

/**
 * During raw computation, .pct, .loss, etc. aren't percentages between 0.0 and 1.0, they are in fact integer counts.
 * We divide by the total number of occurrences here to bring all of these values back down to pure percentages.
 */
void DistrShape::normalize(float64 mag)
{
	avgDev /= mag;
	stdDev /= mag*mag;
	skew /= mag*mag*mag;
	kurtosis /= mag*mag*mag*mag;

    best.scaleOrdinateOnly(1.0 / mag);
    worst.scaleOrdinateOnly(1.0 / mag);
    mean.scaleOrdinateOnly(1.0 / mag);

    for (size_t k=0; k<COARSE_COMMUNITY_NUM_BINS; ++k) {
        coarseHistogram[k].scaleOrdinateOnly(1.0 / mag);
    }
}
CallCumulation::~CallCumulation()
{
}


void CallCumulation::ReversePerspective()
{
//Originally, the incremental/marginal rank of hand #n is [n].repeated - [n-1].repeated
    std::reverse(cumulation.begin(),cumulation.end());
//After reversing, you must calculate [n].repeated - [n+1].repeated
    vector<StatResult>::iterator target;
    vector<StatResult>::iterator next_target = cumulation.begin();

    (*next_target).pct = 1 - (*next_target).pct;
    (*next_target).wins = 1 - (*next_target).wins - (*next_target).splits;
    (*next_target).loss = 1 - (*next_target).loss - (*next_target).splits;

    while( next_target != cumulation.end() )
    {
        target = next_target;
        ++next_target;

        if( next_target == cumulation.end() )
        {
            (*target).repeated = 1;
            break;
        }else
        {

            (*target).repeated = 1 - (*next_target).repeated;

            (*next_target).pct = 1 - (*next_target).pct;
            (*next_target).wins = 1 - (*next_target).wins - (*next_target).splits;
            (*next_target).loss = 1 - (*next_target).loss - (*next_target).splits;
        }
    }



}

///This function is the derivative of nearest_winPCT_given_rank by drank
float64 CallCumulationD::inverseD(const float64 rank, const float64 mean) const
{
    //f(x) = Pr_haveWinPCT_strictlyBetterThan(x-EPS), where f(x) is rarity, 1-f(x) is rank, x is winPCT
    //nearest_winPCT_given_rank(1 - Pr_haveWinPCT_strictlyBetterThan(x-EPS)) = x
    //nearest_winPCT_given_rank(1 - Pr_haveWinPCT_strictlyBetterThan(x)) = x+EPS
    //nearest_winPCT_given_rank(1 - f(x)) = x+EPS
    //nearest_winPCT_given_rank(1 - x) = f-1(x) + EPS
    //nearest_winPCT_given_rank(x) = f-1(1-x) + EPS
    //nearest_winPCT_given_rank(x)-EPS = f-1(1-x)
    //f(nearest_winPCT_given_rank(x)-EPS) = 1-x
    //Pr_haveWinPCT_strictlyBetterThan(nearest_winPCT_given_rank(rank)-EPS) = rarity = 1 - rank
    //f(nearest_winPCT_given_rank(rank)-EPS) = rarity = 1 - rank
//differentiate with respect to rank
    //f'(nearest_winPCT_given_rank(rank)-EPS) * nearest_winPCT_given_rank'(rank) = -1
    //nearest_winPCT_given_rank'(rank) = -1 / f'(nearest_winPCT_given_rank(rank)-EPS)


    //f(x) = Pr_haveWorsePCT(x-EPS), where f(x) is rarity, 1-f(x) is rank, x is winPCT
    //nearest_winPCT_given_rank(Pr_haveWorsePCT(x)) = x
    //nearest_winPCT_given_rank(f(x)) = x
    //nearest_winPCT_given_rank(x) = f-1(x)
    //nearest_winPCT_given_rank(1-x) = f-1(1-x)
    //f(nearest_winPCT_given_rank(x)) = x
    //Pr_haveWinPCT_strictlyBetterThan(nearest_winPCT_given_rank(rank)) = rank = 1 - rarity
    //f(nearest_winPCT_given_rank(rank)) = rank = 1 - rarity
    //differentiate with respect to rank
    //f'(nearest_winPCT_given_rank(rank)-EPS) * nearest_winPCT_given_rank'(rank) = 1
    //nearest_winPCT_given_rank'(rank) = 1 / f'(nearest_winPCT_given_rank(rank)-EPS)

    //Since mean == nearest_winPCT_given_rank( rank )
    return 1.0 / Pr_haveWorsePCT_continuous( mean ).second;
}

//How this works:
//reverseLookup(0.9) returns A pct, where (1-0.9) of hands are better to have than A pct.
///If you have rank, you would have winPCT
float64 CallCumulation::nearest_winPCT_given_rank(const float64 rank_toHave)
{
    //Search for this .repeated is the rank_toHave
    const size_t maxsize = cumulation.size();
    size_t high_index = maxsize - 1;
    size_t low_index = 0;

    float64 high_rank, low_rank;

    low_rank = cumulation[0].repeated;
    high_rank = cumulation[high_index].repeated;

//Early returns
    if( rank_toHave < 0 )
    {//Closer to rank 0 than the smallest positive rank
        return 0;
    }
    if( rank_toHave < low_rank )
    {
        return cumulation[0].pct; //.pct is toHave -- if you haven't ReversedPerspective then that's the pct of the first hand dealt. See the bottom of CallStats::Analyze()
    }
    if( rank_toHave > cumulation[high_index-1].repeated )
    {
        return cumulation[high_index].pct; //.pct is toHave.
    }
    if( rank_toHave > high_rank ) //Greater than 1??
    {
        return 1;
    }

    bool bFloor = true;
    size_t guess_index;

    //See if the cache hits
    if( cached_high_index > 0 && cached_high_index < maxsize )
    {
        const float64 cache_low_rank = cumulation[cached_high_index-1].repeated;
        const float64 cache_high_rank = cumulation[cached_high_index].repeated;
        //High index should be just above the desired rank
        //Low index should be just below
        if( cache_high_rank > rank_toHave )
        {
            high_rank = cache_high_rank;
			high_index = cached_high_index;
        }

        if( cache_low_rank < rank_toHave )
        {//as if guess_rank < rank_toHave
            low_rank = cache_low_rank;
			low_index = cached_high_index - 1;
        }
    }

    size_t lastWidth = high_index - low_index;

    while( high_index > low_index + 1 )
    {

#ifdef ACCELERATE_SEARCH_MINIMUM
        size_t currentWidth = high_index-low_index;
        if( currentWidth < ACCELERATE_SEARCH_MINIMUM || (currentWidth-1)*2 > lastWidth )
        {
            //We use bisection if the last iteration didn't cut the range in at least half
            //We also start using bisection exclusively after a SEARCH_MINIMUM
#endif
            guess_index = (high_index+low_index)/2;
#ifdef ACCELERATE_SEARCH_MINIMUM
        }else
        {
            float64 false_position = (high_index*(rank_toHave-low_rank) + low_index*(high_rank-rank_toHave))/(high_rank-low_rank);
            false_position = bFloor ? floor(false_position) : ceil(false_position);
            guess_index = static_cast<size_t>(false_position);


            if( guess_index <= low_index + 1 )
            {
                guess_index = low_index + 1;
            }else
            {
                if( !bFloor ) {++guess_index ;}
            }

            if( guess_index >= high_index - 1 )
            {
                guess_index = high_index - 1;
            }else
            {
                if( bFloor ) {--guess_index ;}
            }
        }
#endif
        const float64 guess_rank = cumulation[guess_index].repeated;
        lastWidth = currentWidth;

        if( guess_rank > rank_toHave )
        {
            high_rank = guess_rank;
            high_index = guess_index;
            bFloor = true;
        }
        else if( guess_rank < rank_toHave ) //midpoint rank too low
        {//Higher indices have higher ranks, bump up low_index
            low_rank = guess_rank;
            low_index = guess_index;
            bFloor = false;
        }else
        {//Perfect match. The PCT for a given rank is one index higher
            cached_high_index = guess_index;
            return cumulation[guess_index+1].pct;
        }

    }

//repeated is rank, .pct is pctToHave -- see bottom of CallStats::Analyze()
    high_rank = cumulation[high_index].repeated;
    const float64 high_pct = cumulation[high_index+1].pct;
    low_rank = cumulation[low_index].repeated;
    const float64 low_pct = cumulation[low_index+1].pct;
//Higher indices have higher ranks (to have), bump up low_index

    cached_high_index = high_index;

    return (
                low_pct * (high_rank - rank_toHave)
                +
                high_pct * (rank_toHave - low_rank)
            ) / (high_rank - low_rank);


}

float64 CallCumulationD::linearInterpolate(float64 x1, float64 y1, float64 x2, float64 y2, float64 x) const
{
    return ((x-x1)*y2 + (x2-x)*y1)/(x2-x1);
}

///This function returns the probability of having winPCT_toHave or better
float64 CallCumulation::Pr_haveWinPCT_strictlyBetterThan(const float64 winPCT_toHave) const
{

    //const size_t maxsize = cumulation.size();
	const size_t firstBetterThan = searchWinPCT_strictlyBetterThan(winPCT_toHave);
    // firstBetterThan-1 is the last hand to be as good as or worse, so its .repeated is your result.

	//The absolute best had to have is at maxsize-1, so to take frequency from one step back...
	if(firstBetterThan == 0)
	{//All hands are better.
	    return 1.0;
	}

    const float64 frequency = 1.0 - cumulation[firstBetterThan-1].repeated;//The probability of having better than this hand
    return frequency;

}
///In this function, assume cumulation is all the possible outcomes we could face
//.wins, .loss, .splits are the stats of your outcome
//.pct is YOUR chance to win if you have that outcome (in all cases, .pct is the genPCT() of that StatResult)
//.repeated is the rank of that outcome (actually, of the next outcome better)
//We return the "probability of having a worse hand" or "Pr{PCT < winpct_tohave}" but smoothly interpolated across the histogram.
std::pair<float64, float64> CallCumulationD::Pr_haveWorsePCT_continuous(const float64 winPCT_toHave) const
{//However, we would like to piecewise linear interpolate, so the function is continuous.
    // This helps especially because the derivative is never zero anywhere, which won't confuse a solver.

/*
 Imagine a set of ordered outcomes
 
 a
 a
 B
 B
 B
 B
 B
 c
 c
 c
 c
 D
 D
 e
 e
 .
 .
 .

 each outcome has a fixed ".repeated" contribution but they are grouped.
 Now, for visualization I'll space them out by pct
 
 +a
  a
    B
    B
    B
    B
    B
           c
           c
           c
           c
            D
            D
               e
               e
 
 Making cumulative, we have Pr{PCT < pct} =
  

               |
               |
            |--
            |
           |
           |
           |
           |
    |------
    |
    |
    |
    |
  |-
  |
 +
 
 Here, x-axis is pct and y-axis is Pr{PCT < pct}
 
 Say we applied some kernel smoothing...
 
 Okay, so the interpolation intersects at a horizontal midpoint and a vertical midpoint always.

 */

    const size_t maxsize = cumulation.size();
	const size_t firstBetterThan = searchWinPCT_strictlyBetterThan(winPCT_toHave);
	//firstBetterThan is the first outcome that's strictly better than winPCT_toHave
	//cumulation[maxsize-1] is the strongest outcome in this CDF

    if (maxsize == 1) {
        // All outcomes constant
        if (cumulation[0].pct < winPCT_toHave) {
            return std::pair<float64, float64>(1.0, 0.0);
        } else if (winPCT_toHave < cumulation[0].pct) {
            return std::pair<float64, float64>(0.0, 0.0);
        } else {
            return std::pair<float64, float64>(0.5, 0.0);
        }
    }

    if( firstBetterThan == maxsize )
    {//All hands meet criteria
        return std::pair<float64, float64>(1.0, 0.0);
    }
    if (firstBetterThan == 0) {
        //No hands meet criteria
        return std::pair<float64, float64>(0.0, 0.0);
    }
//These boundaries form a region with start and end.
    // Okay we lie somewhere between firstBetterThan and firstBetterThan-1
    float64 prevKeypointPct;// = std::numeric_limits<float64>::quiet_NaN();
    float64 prevKeypointRepeated;// = std::numeric_limits<float64>::quiet_NaN();
    float64 nextKeypointPct;// = std::numeric_limits<float64>::quiet_NaN();
    float64 nextKeypointRepeated;// = std::numeric_limits<float64>::quiet_NaN();
    // Which side of the horizontal midpoint are we on?
    float64 horizontalMidpointPCT = (cumulation[firstBetterThan-1].pct + cumulation[firstBetterThan].pct)/2.0;
    float64 horizontalMidpointRepeated = cumulation[firstBetterThan-1].repeated;
    if (horizontalMidpointPCT < winPCT_toHave) {
        // The prevKeypoint is on the horizontal.
        //
        //        |
        //        |
        //       /|
        //      / |
        //   --/--
        prevKeypointPct = horizontalMidpointPCT;
        nextKeypointPct = cumulation[firstBetterThan].pct;

        prevKeypointRepeated = horizontalMidpointRepeated;
        if (firstBetterThan == maxsize - 1) {
            nextKeypointRepeated = cumulation[firstBetterThan].repeated;
        } else {
            nextKeypointRepeated = (cumulation[firstBetterThan-1].repeated + cumulation[firstBetterThan].repeated) / 2.0;
        }

    } else if (winPCT_toHave < horizontalMidpointPCT) {
        // The nextKeypoint is on the horizontal
        prevKeypointPct = cumulation[firstBetterThan-1].pct;
        nextKeypointPct = horizontalMidpointPCT;

        if (firstBetterThan-1 == 0) {
            prevKeypointRepeated = 0.0;
        } else {
            prevKeypointRepeated = (cumulation[firstBetterThan-2].repeated + cumulation[firstBetterThan-1].repeated) / 2.0;
        }
        nextKeypointRepeated = horizontalMidpointRepeated;
    } else {
        const float64 prevRepeated = sampleSafe_prevrepeated(firstBetterThan-1); // (firstBetterThan == 1) ? 0.0 : cumulation[firstBetterThan-2].repeated;
        // We're right on the midpoint. Pick any slope in between (it's instantaneous)
            const float64 rise = (cumulation[firstBetterThan].repeated - prevRepeated)/2.0;
            const float64 run = cumulation[firstBetterThan].pct - cumulation[firstBetterThan-1].pct;


        return std::pair<float64, float64> (horizontalMidpointRepeated, rise/run);
    }


    // INVARIANT: At this point, we are interpolating between (x=prevPct, y=prevRepeated) and (x=nextPct, y=nextRepeated)
    const float64 rise = nextKeypointRepeated - prevKeypointRepeated;
    const float64 run = nextKeypointPct - prevKeypointPct;
    const float64 slope = rise/run;

    const float64 result = horizontalMidpointRepeated + slope * (winPCT_toHave - horizontalMidpointPCT);

#ifdef DEBUGASSERT
    if( !(0.0 <= result && result <= 1.0) )
    {
        std::cout << "INVALID result in Pr_haveWorsePCT_continuous! " << result;
        exit(1);
    }
#endif


    return std::pair<float64, float64> (result
                                        ,
                                        slope
                                        );
}


StatResult CallCumulation::worstHandToHave() const
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
    retVal.repeated = 0;
    return retVal;
}



std::pair<StatResult, float64> CallCumulation::bestXHands(float64 X) const
{
    #ifdef DEBUGASSERT
        if( cumulation.size() == 0 )
        {
            std::cout << "EMPTY CALLCUMULATION!";
            exit(1);
        }
    #endif

    size_t idx = cumulation.size() - 1; // Start with the best hand

    std::pair<StatResult, float64> result;
    float64 & d_pct_dX = result.second;
    d_pct_dX = 0.0;

    StatResult & runningAverage = result.first;
    runningAverage.repeated = 0.0;

    while (true) {

        ///Retrieve stats and reverse the perspective.

        StatResult val = cumulation[idx]; // Get the next outcome
        // retVal.repeated := cumulation[idx].repeated - cumulation[idx-1].repeated;
        val.repeated -= sampleSafe_prevrepeated(idx);

        if (X <= val.repeated + runningAverage.repeated) {
            const float64 thispct = runningAverage.pct;
            const float64 thisrepeated = runningAverage.repeated;
            // Interpolate the edge entry
            // Add only the remaining weight to reach X.
            val.repeated = X - runningAverage.repeated;
            runningAverage.addByWeight(val);

            // Now, compute the derivative.
            // this->pct    := (   this->pct * this->repeated +    a.pct * a.repeated) / netRepeated;
            // this->pct    := (   this->pct * this->repeated +    a.pct * (X - this->repeated)) / X;
            // this->pct    :=     this->pct * this->repeated / X + a.pct - a.pct * this->repeated / X;
            // this->pct    := a.pct + (this->pct - a.pct) * this->repeated / X;

            // d_dX{this->pct} := d_dX(   this->pct * this->repeated / X + a.pct - a.pct * this->repeated / X );
            // d_dX{this->pct} := d_dX(   a.pct + (this->pct - a.pct) * this->repeated / X; );
            // d_dX{this->pct} :=          d_dX(  (this->pct - a.pct) * this->repeated / X; );
            // d_dX{this->pct} :=          (this->pct - a.pct) * this->repeated  * d_dX(  1.0 / X; );
            // d_dX{this->pct} :=          (this->pct - a.pct) * this->repeated  *     (  - 1.0 / X^2; );
            // d_dX{this->pct} :=          (a.pct - this->pct) * this->repeated  / X^2

            d_pct_dX = (val.pct - thispct) * thisrepeated / X / X;
            return result;
        } else
        {
            runningAverage.addByWeight(val);
        }

        if (idx == 0) {
            // TODO(from yuzisee): Assert that X is very close to 1.0? Assert that runningAverage.repeated is very close to 1.0?

            // Perhaps due to rounding error runningAverage.repeated is very close to 1.0 but didn't reach it.
            runningAverage.repeated = X;
            d_pct_dX = 0.0;
            return result;
        }

        // INVARIANT: retVal.repeated is the sum of the probability of having all the hands between idx .. size()-1, inclusive.
        --idx;

    }
}

StatResult CallCumulation::bestHandToHave() const
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
    retVal.repeated = cumulation[cumulation.size()-2].repeated; // because cumulation[cumulation.size()-1].repeated = 1.0
    return retVal;
}

/*

float64 CallCumulation::sampleInBounds_pct(size_t x) const
{
    if( x == cumulation.size() + 1 )
    {
        return 1;
    }else if( x == cumulation.size() )
    {
        return 0;
    }

    return cumulation[x].pct;
}


float64 CallCumulation::sampleInBounds_repeated(size_t x) const
{
    if( x == cumulation.size() + 1 )
    {
        return 0;
    }else if( x == cumulation.size() )
    {
        return 1;
    }

    return cumulation[x].repeated;
}
*/

float64 CallCumulation::sampleSafe_prevrepeated(size_t x) const
{
    if( x == 0 )
    {
        return 0.0;
    }else
    {
        return cumulation[x-1].repeated;
    }
}

/*
float64 CallCumulationD::slopeof(size_t x10, size_t x11, size_t x20, size_t x21,size_t y_index0, size_t y_index1) const
{
    #ifdef SMOOTHED_CALLCUMULATION_D
    if( x10 < cumulation.size() )
    {
        for( int8 extraSmooth=0;extraSmooth<2;++extraSmooth)
        {
            if( x10 <= 0 || x21 >= cumulation.size() - 1)
            {
                break;
            }
            --y_index0;
            --x10;
            --x11;
            ++y_index1;
            ++x20;
            ++x21;
        }
    }
    #endif

    const float64 p10 = sampleInBounds_pct(x10);
    const float64 p11 = sampleInBounds_pct(x11);

    const float64 p20 = sampleInBounds_pct(x20);
    const float64 p21 = sampleInBounds_pct(x21);

    const float64 y0 = (1-sampleInBounds_repeated(y_index0));
    const float64 y1 = (1-sampleInBounds_repeated(y_index1));
    const float64 p0 = 1-(p10 + p11)/2;
    const float64 p1 = 1-(p20 + p21)/2;
	return (y1 - y0)/(p1 - p0);
}


float64 SlidingPairCallCumulationD::pctWillCall(const float64 oddsFaced) const
{
    return left->pctWillCall(oddsFaced) * (1-slider) + right->pctWillCall(oddsFaced) * (slider);
}

float64 SlidingPairCallCumulationD::pctWillCallD(const float64 oddsFaced) const
{
    return left->pctWillCallD(oddsFaced) * (1-slider) + right->pctWillCallD(oddsFaced) * (slider);
}
*/
//This function returns the index of the worst StatResult which is still better than winPCT
//After calling this function, you are interested in all the elements from [returnedIndex to size()-1]
//The frequency of this set of hands can be determined by taking (1 - cumulation[returnedIndex-1].repeated)
size_t CallCumulation::searchWinPCT_strictlyBetterThan(const float64 winPCT) const
{
	//binary search
	size_t first=0;
	size_t last=cumulation.size()-1;
	size_t guess;
	float64 curPCT;


	curPCT = cumulation[last].pct;
	//First, we check a special case
	//Could NO hands be strictly better? We must check if cumulation[last] is also unacceptable
	if( curPCT <= winPCT )
	{
	    //Even the best hand is in a worse position than the requested position
	    //ie. Even the best hand to have isn't as good as winPCT
	    return cumulation.size();
	}


    ///Binary search for oddsFaced
    ///The lowest .pct is first
    // The lowest .repeated is first
    ///We want the pct FURTHEST from last, that doesn't drop to winPCT
	while(last > first)
	{
		guess = (last+first)/2;
		curPCT = cumulation[guess].pct;

		if (winPCT < curPCT) //curPCT doesn't drop to winPCT, so we can go lower in PCT, lower in index (drop the large indices)
		{//Still too close to .size(), curPCT was too high

            if( guess == last )
            {//This should be impossible, since guess would round down always.

                if( winPCT < cumulation[first].pct )
                {
                    //even first still doesn't touch (but we know first-1 must have)
                    last = first;
                    // we're good. last === (first) === guess
                }else
                {
                    // first touches winPCT so we return: (last) === first === guess
                    first = last;
                }
            }else
            {
                last = guess;
                //We can't go last = guess - 1 because that might be too low of a curPCT
                // all we know for sure is that gyess is still a valid candidate.
                // if we're close, guess-1 may not be a valid candidate for returning!
            }

		}
		else if (curPCT < winPCT)
		{//we went too low (index-wise) and we dropped way below winPCT!
		 //first is no good for sure
         //guess isn't even good

            if( guess == last )
            {
                first = guess; //This should be impossible, since guess would round down always.
            }else
            {
                first = guess+1;
                //ok, first is now a potential candidate -- until we check (or higher) it eventually
            }
		}
		else
		{
            // so curPCT == winPCT?
            // well I guess
		    last = guess+1;
		    first = guess+1;
		}
	}


	//INVARIANT: last==first
	//last should be the first StatResult to have that is strictly better than winPCT
	//For example, if( last == 0 ) ALL hands are better!
	return last;

}

const CallCumulation & CallCumulation::operator=(const CallCumulation& o)
{
    cumulation = o.cumulation;
    return *this;
}

