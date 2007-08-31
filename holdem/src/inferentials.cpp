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
#include <algorithm>
#include "inferentials.h"

//#include <iostream>

#define SMOOTHED_CALLCUMULATION_D
#define ACCELERATE_SEARCH_MINIMUM 256
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


void CallCumulation::ReversePerspective()
{
//Originally, the incremental/marginal rank of hand #n is [n].repeated - [n-1].repeated
    std::reverse(cumulation.begin(),cumulation.end());
//After reversing, you must calculate [n].repeated - [n+1].repeated
    vector<StatResult>::iterator target;
    vector<StatResult>::iterator next_target = cumulation.begin();
    do
    {
        target = next_target;

        (*target).pct = 1 - (*target).pct;
        (*target).wins = 1 - (*target).wins;
        (*target).loss = 1 - (*target).loss;

        ++next_target;

        (*target).repeated = 1 - (*next_target).repeated;

    }while( next_target != cumulation.end() );


    (*next_target).pct = 1 - (*next_target).pct;
    (*next_target).wins = 1 - (*next_target).wins;
    (*next_target).loss = 1 - (*next_target).loss;
    (*next_target).repeated = 1;

}

///This function is the derivative of nearest_winPCT_given_rank by drank
float64 CallCumulationD::inverseD(const float64 rank)
{
    //f(x) = Pr_haveWinPCT_orbetter(x), where f(x) is rarity, 1-f(x) is rank, x is winPCT
    //nearest_winPCT_given_rank(1 - Pr_haveWinPCT_orbetter(x)) = x
    //nearest_winPCT_given_rank(1 - f(x)) = x
    //nearest_winPCT_given_rank(1 - x) = f-1(x)
    //nearest_winPCT_given_rank(x) = f-1(1-x)
    //Pr_haveWinPCT_orbetter(nearest_winPCT_given_rank(rank)) = rarity = 1 - rank
    //f(nearest_winPCT_given_rank(rank)) = rarity = 1 - rank
//differentiate with respect to rank
    //f'(nearest_winPCT_given_rank(rank)) * nearest_winPCT_given_rank'(rank) = -1
    //nearest_winPCT_given_rank'(rank) = -1 / f'(nearest_winPCT_given_rank(rank))

    return - 1 / d_dw_only( nearest_winPCT_given_rank( rank ) );
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
        return 1 - cumulation[0].pct; //1-.pct is the toHave.
    }
    if( rank_toHave > cumulation[high_index-1].repeated )
    {
        return 1 - cumulation[high_index].pct; //1-.pct is the toHave.
    }
    if( rank_toHave > high_rank ) //Greater than 1??
    {
        return 1;
    }

    bool bFloor = true;
    size_t guess_index;

    //See if the cache hits
    if( cached_high_index > 0 )
    {
        low_rank = cumulation[cached_high_index-1].repeated;
        high_rank = cumulation[cached_high_index].repeated;
        //High index should be just above the desired rank
        //Low index should be just below
        if( high_rank > rank_toHave )
        {
            high_index = cached_high_index;
        }

        if( low_rank < rank_toHave )
        {//as if guess_rank < rank_toHave
            low_index = cached_high_index - 1;
        }
    }

    while( high_index > low_index + 1 )
    {

        if( high_index-low_index < ACCELERATE_SEARCH_MINIMUM )
        {
            guess_index = (high_index+low_index)/2;
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

        const float64 guess_rank = cumulation[guess_index].repeated;


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
            return 1 - cumulation[guess_index+1].pct;
        }

    }

//repeated is rank, 1-.pct is pctToHave
    high_rank = cumulation[high_index].repeated;
    const float64 high_pct = 1 - cumulation[high_index+1].pct;
    low_rank = cumulation[low_index].repeated;
    const float64 low_pct = 1 - cumulation[low_index+1].pct;
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
float64 CallCumulation::Pr_haveWinPCT_orbetter(const float64 winPCT_toHave) const
{

    //const size_t maxsize = cumulation.size();
	const size_t firstBetterThan = searchWinPCT_betterThan_toHave(winPCT_toHave);

	//The absolute best had to have is at maxsize-1, so to take frequency from one step back...
	if(firstBetterThan == 0)
	{//All hands are better.
	    return 1;
	}

    const float64 frequency = 1 - cumulation[firstBetterThan-1].repeated;//The probability of having this hand or better
    return frequency;

}

float64 CallCumulationD::d_dw_only(const float64 w_toHave) const
{
    float64 d_dw;
    Pr_haveWinPCT_orbetter_continuous(w_toHave, &d_dw);
    return d_dw;
}

///In this function, assume cumulation is all the possible hands your opponent could have.
//.wins, .loss, .splits are the stats of your opponent's cards
//.pct is YOUR chance to win if opponent has that hand
//.repeated is the rank of your opponents cards (actually, of the next hand better for your opponent to have)
///Let's say the opponent could read you, and knew what cards you had.
///It would know your PCT as a function of the hand it had.
float64 CallCumulationD::Pr_haveWinPCT_orbetter_continuous(const float64 winPCT_toHave, float64 *out_d_dw) const
{//However, we would like to piecewise linear interpolate, so the function is continuous.

    const size_t maxsize = cumulation.size();
	const size_t firstBetterThan = searchWinPCT_betterThan_toHave(winPCT_toHave);
	//firstBetterThan is the first hand where your opponent can beat you with odds more than winPCT_toHave
	//cumulation[maxsize-1] is the best hand your opponent can have

    float64 midpointRarity;
//Without interpolation, we would return cumulation[firstBetterThan-1].repeated as the chance of getting a better hand next time (the rarity)
//winPCT_toHave is between cumulation[firstBetterThan-1].pct and cumulation[firstBetterThan].pct
    if( firstBetterThan == maxsize )
    {//No hands meet criteria
        if( out_d_dw != 0 ){*out_d_dw = 0;}
        return 1; //same is:  {return midpointRarity;}
    }
//These boundaries form a region with midpoint:
    size_t nextBestHandToHave = firstBetterThan + 1;
    size_t barelyWorseHandToHave = firstBetterThan - 1;
    size_t prebarelyWorseHandToHave = barelyWorseHandToHave - 1;
    float64 midpointPCT_toHave;
    if( firstBetterThan == 0 )
    {//Noting that cumulation[0].pct as high as possible, and also cumulation[0].repeated is not zero
        midpointPCT_toHave = 1 - (1 + cumulation[firstBetterThan].pct)/2;
        midpointRarity = 1;
        //There is no hand worse for your opponent to have
        barelyWorseHandToHave = maxsize+1;
        prebarelyWorseHandToHave = maxsize+1;
        ///SPECIAL CASE handling:
         if( winPCT_toHave < midpointPCT_toHave )
         {
             if( out_d_dw != 0 ){*out_d_dw = 0;}
             return 1; //Obviously the frequency of having a better hand than a hand equal to or even worse than the worst is 100%
         }

    }else
    {
        midpointPCT_toHave = 1 - (cumulation[barelyWorseHandToHave].pct + cumulation[firstBetterThan].pct)/2;
        midpointRarity = 1 - cumulation[barelyWorseHandToHave].repeated;
    }

    float64 nextBestPCT_toHave;
    const float64 nextBestRarity = 1 - cumulation[firstBetterThan].repeated; //Frequency of having better than firstBetterThan
    if( firstBetterThan == maxsize - 1 )
    {
        nextBestPCT_toHave = 1 - cumulation[firstBetterThan].pct;
    }else
    {
        nextBestPCT_toHave = 1 - (cumulation[firstBetterThan].pct + cumulation[firstBetterThan+1].pct)/2;
    }


    float64 barelyWorsePCT_toHave,barelyWorseRarity;

    if( firstBetterThan == 1 )
    {
        prebarelyWorseHandToHave = maxsize+1;
        barelyWorsePCT_toHave = 1 - (1 + cumulation[firstBetterThan].pct)/2;
        barelyWorseRarity = 1;
    }else
    {
        barelyWorsePCT_toHave = 1 - ( sampleInBounds_pct(prebarelyWorseHandToHave) + sampleInBounds_pct(barelyWorseHandToHave))/2;
        barelyWorseRarity = 1 - sampleInBounds_repeated(prebarelyWorseHandToHave);
    }



    if( winPCT_toHave < midpointPCT_toHave )
    {
        if( out_d_dw != 0 ){*out_d_dw = slopeof(prebarelyWorseHandToHave,barelyWorseHandToHave,barelyWorseHandToHave,firstBetterThan,prebarelyWorseHandToHave,barelyWorseHandToHave);}
        return linearInterpolate(barelyWorsePCT_toHave,barelyWorseRarity,midpointPCT_toHave,midpointRarity,winPCT_toHave);
    }
    else if( winPCT_toHave > midpointPCT_toHave )
    {//Highest PCT_toHave is at the high indices
        if( out_d_dw != 0 ){*out_d_dw = slopeof(barelyWorseHandToHave,firstBetterThan,firstBetterThan,nextBestHandToHave,barelyWorseHandToHave,firstBetterThan);}
        return linearInterpolate(midpointPCT_toHave,midpointRarity,nextBestPCT_toHave,nextBestRarity,winPCT_toHave);
    }
    else//( winPCT_toHave == midpointPCT_toHave )
    {
        if( out_d_dw != 0 ){*out_d_dw = slopeof(prebarelyWorseHandToHave,barelyWorseHandToHave,firstBetterThan,nextBestHandToHave,prebarelyWorseHandToHave,firstBetterThan);}
        return midpointRarity;
    }

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
    retVal.pct = 1 - retVal.pct;
    retVal.repeated = 0;
    return retVal;
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
    retVal.pct = 1 - retVal.pct;
    retVal.repeated = cumulation[cumulation.size()-2].repeated;
    return retVal;
}

float64 CallCumulationD::sampleInBounds_pct(size_t x) const
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


float64 CallCumulationD::sampleInBounds_repeated(size_t x) const
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

/*
float64 SlidingPairCallCumulationD::pctWillCall(const float64 oddsFaced) const
{
    return left->pctWillCall(oddsFaced) * (1-slider) + right->pctWillCall(oddsFaced) * (slider);
}

float64 SlidingPairCallCumulationD::pctWillCallD(const float64 oddsFaced) const
{
    return left->pctWillCallD(oddsFaced) * (1-slider) + right->pctWillCallD(oddsFaced) * (slider);
}
*/
//First, "pct to have" is  1-(.pct)
//If you have winPCT_toHave, then the opponent has winPCT_toBeAgainst
//If you have a pct better than winPCT_toHave, then your opponent has a worse pct than winPCT_toBeAgainst
//Therefore, we find the best (.pct) that is less than winPCT_toBeAgainst
//Here, "to be against" means your odds of winning without knowing your cards, but knowing your opponents'

//This function returns the index of the worst winPCT_toHave which is still better than winPCT_toHave
//After calling this function, you are interested in all the elements from [returnedIndex to size()-1]
//The frequency of this set of hands can be determined by taking (1 - cumulation[returnedIndex-1].repeated)
size_t CallCumulation::searchWinPCT_betterThan_toHave(const float64 winPCT_toHave) const
{
    const float64 winPCT_toBeAgainst = 1-winPCT_toHave;

	//binary search
	size_t first=0;
	size_t last=cumulation.size()-1;
	size_t guess;
	float64 curPCT;


	curPCT = cumulation[last].pct;
	//First, we check a special case
	//Could NO hands be better? We must check if cumulation[last] is also unacceptable
	if( curPCT > winPCT_toBeAgainst )
	{
	    //Even the worst hand to be against puts you in a better position than the requested position
	    //ie. Even the best hand to have isn't as good as winPCT_toHave
	    return cumulation.size();
	}


    ///Binary search for oddsFaced
    ///The lowest .pct is at last
    ///We want the pct FURTHEST from last, that doesn't exceed winPCT_toBeAgainst
	while(last > first)
	{
		guess = (last+first)/2;
		curPCT = cumulation[guess].pct;

		if (curPCT < winPCT_toBeAgainst) //curPCT doesn't exceed, so we can go higher in PCT, lower in index (drop the large indices)
		{//Still too close to .size(), pct was too low

            if( guess == last )
            {
                //This should be impossible, since guess would round down always.
                if( cumulation[first].pct < winPCT_toBeAgainst )
                {
                    //still doesn't exceed, we're good
                    last = first;
                }else
                {
                    first = last;
                }
            }else
            {
                last = guess;
                //We can't go last = guess - 1 because that might be too high of a curPCT
            }

		}
		else if (curPCT > winPCT_toBeAgainst)
		{//we went too far (up, if 0 is at the top) and we exceeded winPCT_toBeAgainst!
		 //first is no good.

            if( guess == last )
            {
                first = guess; //This should be impossible, since guess would round down always.
            }else
            {
                first = guess+1;
            }
		}
		else
		{
		    last = guess;
		    first = guess;
		}
	}


	//INVARIANT: last==first
	//last should be the first winPCT to have that is better than the winPCT_toHave
	//For exmaple, if( last == 0 ) ALL hands are better!
	return last;

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

