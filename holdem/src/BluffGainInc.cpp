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

#include "BluffGainInc.h"
#include "math_support.h"

#include <float.h>

#define RAISED_PWIN



void AutoScalingFunction::query(float64 sliderx, float64 x)
{
float64 yl;
float64 yr;
float64 fd_yl;
float64 fd_yr;

    last_x = x;
    last_sliderx = sliderx;

    if( bNoRange )
    {
        // bNoRange indicates an extreme situation is being explored.
        // If these are all-in situations, we want to be on the pessimistic side (choose right rather than left). Being pessimistic here is crucial.
        // If these are no bet situations, it doesn't matter what we pick because we'll probably just check/fold regardless of the model. Being pessimistic here is harmless.
        yr = right.f(x);
        fd_yr = right.fd(x,yr);

        y = yr; dy = fd_yr;

#ifdef DEBUG_TRACE_SEARCH
        if(bTraceEnable) std::cout << "\t\t\tbNoRange" << std::flush;
#endif

    }else
    {


        const float64 autoSlope = saturate_upto / (saturate_max - saturate_min) ;
        const float64 slider = (sliderx - saturate_min) * autoSlope ;

        //std::cerr << autoSlope << endl;
        //std::cerr << slider << endl;


        if( slider >= 1 )
        {
            yr = right.f(x);
            fd_yr = right.fd(x, yr);

            y = yr; dy = fd_yr;

#ifdef DEBUG_TRACE_SEARCH
            if(bTraceEnable) std::cout << "\t\t\tbMax" << std::flush;
#endif
        }
        else if( slider <= 0 )
        {
            yl = left.f(x);
            fd_yl = left.fd(x, yl);

            y = yl; dy = fd_yl;

#ifdef DEBUG_TRACE_SEARCH
            if(bTraceEnable) std::cout << "\t\t\tbMin" << std::flush;
#endif
        }
        else
        {
            yl = left.f(x);
            yr = right.f(x);

            fd_yl = left.fd(x,yl);
            fd_yr = right.fd(x,yr);


#ifdef TRANSFORMED_AUTOSCALES
            if( AUTOSCALE_TYPE == LOGARITHMIC_AUTOSCALE )
            {

				const float64 rightWeight = log1p(slider)/log(2.0);
                const float64 leftWeight = 1 - rightWeight;

                y = yl*leftWeight+yr*rightWeight;
                //y = yl*log(2-slider)/log(2)+yr*log(1+slider)/log(2);

                const float64 d_rightWeight_d_slider = 1.0/(slider)/log(2.0);

                dy = fd_yl*leftWeight - yl*autoSlope*d_rightWeight_d_slider   +   fd_yr*rightWeight + yr*autoSlope*d_rightWeight_d_slider;
		    }else
#ifdef DEBUGASSERT
                if( AUTOSCALE_TYPE == ALGEBRAIC_AUTOSCALE )
#endif // DEBUGASSERT
                {
#endif
                    y = yl*(1-slider)+yr*slider;
                    dy = fd_yl*(1-slider) - yl*autoSlope   +   fd_yr*slider + yr*autoSlope;

#ifdef DEBUG_TRACE_SEARCH
                    if(bTraceEnable) std::cout << "\t\t\t y(" << x << ") = " << yl << " * " << (1-slider) << " + " <<  yr << " * " << slider << std::endl;
                    if(bTraceEnable) std::cout << "\t\t\t dy = " << fd_yl << " * " << (1-slider) << " - " <<  yl << " * " << autoSlope << " + " <<  fd_yr << " * " << slider << " + " <<  yr << " * " << autoSlope << std::endl;
#endif // DEBUG_TRACE_SEARCH
#ifdef TRANSFORMED_AUTOSCALES
                }
#ifdef DEBUGASSERT
                else{
                    std::cerr << "AutoScale TYPE MUST BE SPECIFIED" << endl;
                    exit(71); // EX_OSERR (?)
                }
#endif // DEBUGASSERT
#endif // TRANSFORMED_AUTOSCALES
        }


    }
}

float64 AutoScalingFunction::f(const float64 x)
{
    if( last_x != x || last_sliderx != x)
    {
        query(x,x);
    }
    return y;
}


float64 AutoScalingFunction::fd(const float64 x, const float64 y_dummy)
{
    if( last_x != x || last_sliderx != x)
    {
        query(x,x);
    }
    return dy;
}

float64 AutoScalingFunction::f_raised(float64 sliderx, const float64 x)
{
    if(fSliderBehaviour == RAW) {
        sliderx = x;
    }


    if( last_x != x || last_sliderx != sliderx)
    {
        query(sliderx,x);
    }
    return y;
}

float64 AutoScalingFunction::fd_raised(float64 sliderx, const float64 x, const float64 y_dummy)
{
    if(fSliderBehaviour == RAW) {
        sliderx = x;
    }

    if( last_x != x || last_sliderx != sliderx)
    {
        query(sliderx,x);
    }
    return dy;
}



// @param value - bankroll multiplier if you win (i.e. `value - 1.0` is your profit)
// @param probability - the chance that you'll actually get to the showdown
// @return `result.value` is the "expected" worth of trying to reach the showdown, accounting for both: profit of the showdown AND the probability your opponents actually make it to the showdown
struct AggregatedState GeomStateCombiner::createOutcome(float64 value, float64 probability, float64 dValue, float64 dProbability) const {
    struct AggregatedState result;
    result.pr = probability;
    if (probability < EPS_WIN_PCT) {
        // otherwise pow(0.0, 0.0) might be undefined
        result.value = 1.0;
        result.contribution.v = 1.0;

        // y = std::pow(value, probability)
        // log(y) = probability * log(value)
        // d log(y) = d probability * log(value) + probability * d log(value)
        // dy / y = d probability * log(value) + probability * (d value) / value
        // dy = y * (d probability * log(value) + probability * (d value) / value)
        // dy = y * (d probability * log(value) + probability * (d value) / value)

        result.contribution.D_v = result.contribution.v * ( probability*dValue/value + dProbability*log(value) );
    } else {
        result.value = value;
        result.contribution = ValueAndSlope::exponentiate_unsafe(ValueAndSlope{value, dValue}, ValueAndSlope{probability, dProbability});
    }

    return result;
}

static ValueAndSlope geom_state_combiner_aggregated_contribution(const size_t arraySize, const ValueAndSlope * const values, const ValueAndSlope * const probabilities) {
  ValueAndSlope aggregated_contribution = {1.0, 0.0};

  for (size_t i = 0; i < arraySize; ++i) {
      if (probabilities[i].v < EPS_WIN_PCT) {
          //aggregated_contribution.v *= 1.0; // "no change"
          // log(values) ~= 0.0
          // probability ~= 0.0
      } else if (values[i].v < DBL_EPSILON) {
          aggregated_contribution.v = 0.0; // "lose everything"
          // 1.0 / values ~= \infty
          aggregated_contribution.D_v = std::numeric_limits<float64>::infinity();
          // [!WARNING]
          // EARLY RETURN!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
          return aggregated_contribution;
      } else {
          aggregated_contribution.v *= std::pow(values[i].v, probabilities[i].v);
          aggregated_contribution.D_v += probabilities[i].D_v * std::log(values[i].v) + probabilities[i].v * values[i].D_v / values[i].v;
      }

  }

  aggregated_contribution.D_v *= aggregated_contribution.v;

  return aggregated_contribution;
}

struct AggregatedState GeomStateCombiner::createBlendedOutcome(const size_t arraySize, const ValueAndSlope * const values, const ValueAndSlope * const probabilities) const {
    struct AggregatedState result;
    result.pr = 0.0;
    result.contribution = geom_state_combiner_aggregated_contribution(arraySize, values, probabilities);

    for (size_t i = 0; i < arraySize; ++i) {
        result.pr += probabilities[i].v;
    }

    // y = std::pow(value1, probability1) * ... * std::pow(valueN, probabilityN)
    // log(y) = probability1 * log(value1) + ... + probabilityN * log(valueN)
    // d log(y) = d probability1 * log(value1) + probability1 * d log(value1) + ... + d probabilityN * log(valueN) + probabilityN * d log(valueN)
    // dy / y = d probability1 * log(value1) + probability1 * (d value1) / value1 + ... + d probabilityN * log(valueN) + probabilityN * (d valueN) / valueN
    // dy = y * (d probability1 * log(value1) + probability1 * (d value1) / value1 + ... + d probabilityN * log(valueN) + probabilityN * (d valueN) / valueN)

    if (result.pr > 0.0) {

        // result.value is the blended value @ result.pr probability
        result.value = std::pow(result.contribution.v, 1.0 / result.pr);

    } else {
        result.value = 1.0;
    }

    return result;
}

struct AggregatedState GeomStateCombiner::combinedContributionOf(const struct AggregatedState &a, const struct AggregatedState &b, const struct AggregatedState &c) const {
    struct AggregatedState result;
    result.contribution = ValueAndSlope::multiply3(a.contribution, b.contribution, c.contribution);
    result.pr = a.pr + b.pr + c.pr;
    result.value = std::pow(result.contribution.v, 1.0 / result.pr);

    return result;
}

struct AggregatedState AlgbStateCombiner::createOutcome(float64 value, float64 probability, float64 dValue, float64 dProbability) const {
    const float64 profit = value - 1.0;


#ifdef DEBUGASSERT
    if (profit < -1.0) {
        std::cerr << "AlgbStateCombiner given value < 0.0 which should be impossible!" << endl;
        exit(1);
    }
#endif // DEBUGASSERT

    struct AggregatedState result;
    result.pr = probability;
    result.value = value;
    result.contribution = ValueAndSlope::multiply2(ValueAndSlope{profit, dValue}, ValueAndSlope{probability, dProbability});
    result.contribution.v += 1.0;
    // `profit` === `value - 1.0`
    // From https://github.com/yuzisee/pokeroo/commit/d4ee09a348f2a091fd9bc9dfaebf27bcbd0cbe62
    //
    // y = 1.0 + (v             - 1.0) * prb;
    // y = 1.0 +  v * prb         - prb;
    // dy =     dv * prb + v * dprb - dprb;
    // dy =     dv * prb + (v - 1.0) * dprb;

    return result;
}
struct AggregatedState AlgbStateCombiner::createBlendedOutcome(const size_t arraySize, const ValueAndSlope * const values, const ValueAndSlope * const probabilities) const {
    struct AggregatedState result;
    result.pr = 0.0;
    result.contribution.D_v = 0.0;

    float64 blendedProfit = 0.0;
    for (size_t i = 0; i < arraySize; ++i) {
        const float64 profit = values[i].v - 1.0;

#ifdef DEBUGASSERT
        if (profit < -1.0) {
            std::cerr << "AlgbStateCombiner given value < 0.0 which should be impossible!" << endl;
            exit(1);
        }
#endif // DEBUGASSERT

        blendedProfit += profit * probabilities[i].v;

        result.contribution.D_v += values[i].D_v * probabilities[i].v + profit * probabilities[i].D_v;

        result.pr += probabilities[i].v;
    }

    // y = 1.0 + (v1 - 1.0) * prb1          + ... + (vN - 1.0) * prbN;
    // dy = dv1 * prb1 + (v1 - 1.0) * dprb1 + ... + dvN * prbN + (vN - 1.0) * dprbN;



#ifdef DEBUGASSERT
    if (blendedProfit < -1.0) {
        std::cerr << "AlgbStateCombiner resulting contribution < 0.0 due to rounding error?" << endl;
        exit(1);
    }
#endif // DEBUGASSERT

    if (result.pr > 0.0) {

        result.contribution.v = 1.0 + blendedProfit;

        // result.value is the blended value @ result.pr probability
        result.value = 1.0 + blendedProfit / result.pr;

    } else {
        result.contribution.v = 1.0;
        result.value = 1.0;
    }

#ifdef DEBUGASSERT
    if (result.value < 0.0) {
        std::cerr << "Can't lose more than everything -- please debug" << endl;
        exit(1);
    }
#endif // DEBUGASSERT

    return result;

}

struct AggregatedState AlgbStateCombiner::combinedContributionOf(const struct AggregatedState &a, const struct AggregatedState &b, const struct AggregatedState &c) const {
    struct AggregatedState result;
    result.pr = a.pr + b.pr + c.pr;
    result.contribution = ValueAndSlope::sum3(a.contribution, b.contribution, c.contribution);
    result.contribution.v -= 2.0;
    // y = a + b + c - 2.0
    // dy = da + db + dc
    result.value = 1.0 + (result.contribution.v - 1.0) / result.pr;

    return result;
}


StateModel::~StateModel()
{
    if( bSingle && fp != 0 )
    {
        delete fp;
    }
}


float64 StateModel::g_raised(float64 raisefrom, const float64 betSize)
{
    const float64 potWin = fp->f_raised(raisefrom, betSize);
    return potWin;
}


float64 StateModel::gd_raised(float64 raisefrom, const float64 betSize, const float64 yval)
{
    const float64 fp_f_raised = yval; // Subtract out foldgain, to get f_raised without computing it again.
    return fp->fd_raised(raisefrom, betSize, fp_f_raised); // There is no derivative of ea.FoldGain() here because it is constant relative to betSize.
}


float64 StateModel::f(const float64 betSize)
{
    if( last_x != betSize )
    {

        query(betSize);
    }
    return y;
}


float64 StateModel::fd(const float64 betSize, const float64 yval)
{
    if( last_x != betSize )
    {
        query(betSize);
    }
    return dy;
}



void StateModel::query( const float64 betSize )
{
    // betSize here is always "my" bet size. The perspective of opponents is already covered in <tt>ea</tt>

    last_x = betSize;
    const float64 invisiblePercent = EPS_WIN_PCT;// quantum / ea.tableinfo->allChips();

    const FoldOrCall fMyFoldGain(*(ea.tableinfo->table), ea.fCore); // My current foldgain with the same units as my CombinedStatResult (for proper comparison with call vs. fold)

    ///Establish [PushGain] values

	float64 potFoldWin = ea.tableinfo->PushGain();
	const float64 potFoldWinD = 0;
#ifndef DUMP_CSV_PLOTS
	float64
#endif // if ! DUMP_CSV_PLOTS
    oppFoldChance = ea.pWin(betSize);

    float64 oppFoldChanceD = ea.pWinD(betSize);

    //#ifdef DEBUGASSERT
	if( potFoldWin < 0 || oppFoldChance < invisiblePercent ){
		potFoldWin =  1;
		oppFoldChance = 0;
		oppFoldChanceD = 0;
	}
    //#endif

    ///Establish [Raised] values

    //Count needed array size
    int32 arraySize = 0;
    while( ea.RaiseAmount(betSize,arraySize) < ea.tableinfo->maxRaiseAmount() )
    {
        ++arraySize;
    }
    //This array loops until noRaiseArraySize is the index of the element with RaiseAmount(noRaiseArraySize) == maxBet()
    if(betSize < ea.tableinfo->maxRaiseAmount()) ++arraySize; //Now it's the size of the array (unless you're pushing all-in already)

    const bool bCallerWillPush = (arraySize == 1); //If arraySize is 1, then minRaise goes over maxbet. Anybody who can call will just reraise over the top.

    //Create arrays
    float64 * raiseAmount_A = new float64[arraySize];

    ValueAndSlope * oppRaisedChance_A = new ValueAndSlope[arraySize];
    ValueAndSlope * potRaisedWin_A = new ValueAndSlope[arraySize];

    float64 lastuptoRaisedChance = 0;
    float64 lastuptoRaisedChanceD = 0;
    float64 newRaisedChance = 0;
    float64 newRaisedChanceD = 0;

	firstFoldToRaise = arraySize;

	for( int32 i=0;i<arraySize; ++i)
    {
#ifdef RAISED_PWIN
        raiseAmount_A[i] = ea.RaiseAmount(betSize,i);

        const float64 sliderx = betSize;
        // TODO(from yuzisee): Explore using
        /* fMyFoldGain.predictedRaiseToThisRound(<#float64 currentBetToCall#>, <#float64 currentAlreadyBet#>, <#float64 predictedRaiseTo#>) */
        // as sliderx?


        const float64 evalX = raiseAmount_A[i];
        potRaisedWin_A[i].v = g_raised(sliderx,evalX); // as you can see below, this value is only blended in if we are below firstFoldToRaise
        potRaisedWin_A[i].D_v = gd_raised(sliderx,evalX,potRaisedWin_A[i].v);
        // ^^^ Can't use `.set_value_and_slope` here because we need `potRaisedWin_A[i].v` when computing `potRaisedWin_A[i].D_v`


        if (willFoldToReraise(raiseAmount_A[i], potRaisedWin_A[i].v, fMyFoldGain, *(ea.tableinfo), betSize))
        {
          if( firstFoldToRaise == arraySize ) firstFoldToRaise = i;

            //Since g_raised isn't pessimistic based on raiseAmount (especially when we're just calling), don't add additional gain opportunity -- we should instead assume that if we would fold against such a raise that the opponent has us as beat as we are.
            // Deduct the bet you make and fold
            potRaisedWin_A[i].set_value_and_slope(
              1.0 - ea.tableinfo->betFraction(betSize)
              , -1.0
            );
          }

    }

    // Read the values of ea's pRaise below so they can be combined with the values of g_raised above to get an aggregate gain.
    // Note that we read from the top down, in case it's not monotonic (which can occur due to pessimistic bWouldCall) and
    // the higher of the raises takes precedent (which is the Fold outcome, which trumps expecting lower bets to make money if you can be pushed)
    for( int32 i=arraySize-1;i>=0; --i)
    {

        if( bCallerWillPush )
        {
            //ASSERT: i == 0 && arraySize == 1 && lastUptoRaisedChance == 0 && oppFoldChance and oppFoldChanceD have already been determined
            newRaisedChance = 1 - oppFoldChance;
            newRaisedChanceD = - oppFoldChanceD;
        }else{
            //Standard calculation
            // NOTE! Pr{Raise} includes the probability of being raised later in future rounds too
            newRaisedChance = ea.pRaise(betSize,i,firstFoldToRaise);
            newRaisedChanceD = ea.pRaiseD(betSize,i,firstFoldToRaise);
        }

		if( newRaisedChance - lastuptoRaisedChance > invisiblePercent )
		{
			oppRaisedChance_A[i].v = newRaisedChance - lastuptoRaisedChance;
			oppRaisedChance_A[i].D_v = newRaisedChanceD - lastuptoRaisedChanceD;
			lastuptoRaisedChance = newRaisedChance;
			lastuptoRaisedChanceD = newRaisedChanceD;
		}
		//if( oppRaisedChance_A[i] < invisiblePercent )
		else
#endif
        {
            //raiseAmount_A[i] = 0;
            oppRaisedChance_A[i].clearToZero();
            potRaisedWin_A[i].v = 1; // "no change"
            potRaisedWin_A[i].D_v = 0;
        }

#ifdef DEBUGASSERT
        if (oppRaisedChance_A[i].any_nan()) {
            std::cerr << "oppRaisedChance_A[i] should not be NaN" << std::endl;
            exit(1);
        }
#endif // DEBUGASSERT

#ifdef DEBUG_TRACE_SEARCH
        if(bTraceEnable)
        {
            std::cout << "\t\t(oppRaiseChance[" << i << "] , cur, highest) = " << oppRaisedChance_A[i].v  << " , "  << newRaisedChance << " , " << lastuptoRaisedChance << std::endl;
        }
#endif

    }



    ///Establish [Play] values
#ifndef DUMP_CSV_PLOTS
	float64
#endif // if ! DUMP_CSV_PLOTS
	playChance = 1 - oppFoldChance - lastuptoRaisedChance;
	float64 playChanceD = - oppFoldChanceD - lastuptoRaisedChanceD;
    /*
     float64 playChance = 1 - oppFoldChance;
     float64 playChanceD = - oppFoldChanceD;
     for( int32 i=0;i<arraySize;++i )
     {
     playChance -= oppRaisedChance_A[i];
     playChanceD -= oppRaisedChanceD_A[i];
     }*/


    float64 potNormalWin = g_raised(betSize,betSize);
    float64 potNormalWinD = gd_raised(betSize,betSize,potNormalWin);

    if( 0.0 < playChance && playChance <= invisiblePercent ) //roundoff, but {playChance == 0} is push-fold for the opponent
    {
        //Correct other odds
        const float64 totalChance = 1.0 - playChance;
        for( int32 i=arraySize-1;i>=0; --i)
        {
            oppRaisedChance_A[i].rescale (1.0 / totalChance);
        }
        oppFoldChance /= totalChance;
        oppFoldChanceD /= totalChance;

        //Remove call odds
        playChance = 0;
        playChanceD = 0;
        potNormalWin = 1;
        potNormalWinD = 0;
    }





    ///Calculate factors


    outcomePush = table_spec.stateCombiner.createOutcome(potFoldWin, oppFoldChance, potFoldWinD, oppFoldChanceD);
    outcomeCalled = table_spec.stateCombiner.createOutcome(potNormalWin, playChance, potNormalWinD, playChanceD);

    blendedRaises = table_spec.stateCombiner.createBlendedOutcome(arraySize, potRaisedWin_A, oppRaisedChance_A);

    /*
     STATEMODEL_ACCESS gainRaised = 1;
     float64 gainRaisedlnD = 0;


     for( int32 i=0;i<arraySize;++i )
     {
     gainRaised *= (potRaisedWin_A[i] < DBL_EPSILON) ? 0 : pow( potRaisedWin_A[i],oppRaisedChance_A[i]);

     if( oppRaisedChance_A[i] >= invisiblePercent )
     {
     #ifdef DEBUG_TRACE_SEARCH
     if(bTraceEnable)
     {
     std::cout << "\t\t\t(potRaisedWinD_A[" << i << "] , oppRaisedChanceD_A[" << i << "] , log...) = " << potRaisedWinD_A[i] << " , " << oppRaisedChanceD_A[i] << " , " <<  log( g_raised(betSize,raiseAmount_A[i]-quantum/2) ) << std::endl;
     }
     #endif

     if( raiseAmount_A[i] >= ea.tableinfo->maxBet()-quantum/2 )
     {
     gainRaisedlnD += oppRaisedChance_A[i]*potRaisedWinD_A[i]/ g_raised(betSize,raiseAmount_A[i]-quantum/2) + oppRaisedChanceD_A[i]*log( g_raised(betSize,raiseAmount_A[i]-quantum/2) );
     }else
     {
     gainRaisedlnD += oppRaisedChance_A[i]*potRaisedWinD_A[i]/potRaisedWin_A[i] + oppRaisedChanceD_A[i]*log(potRaisedWin_A[i]);
     }
     }
     }


     if( betSize >= myInfo->maxBet() )
     {
     gainNormallnD = playChance*potNormalWinD/g_raised(betSize,betSize-quantum/2) + playChanceD*log(g_raised(betSize,betSize-quantum/2));
     }
     */

     ///Store results
     struct AggregatedState gainCombined = table_spec.stateCombiner.combinedContributionOf(outcomePush, outcomeCalled, blendedRaises);

#ifdef DEBUG_TRACE_SEARCH
    if(bTraceEnable)
    {
      // https://github.com/yuzisee/pokeroo/commit/ecec2a0e4f8d119a01f310fef9ce4e4652c3ce58
        std::cout << "\t\t (gainWithFold*gainNormal*gainRaised) = " << gainCombined.contribution.v << std::endl;
        std::cout << "\t\t ((gainWithFoldlnD+gainNormallnD+gainRaisedlnD)*y) = " << gainCombined.contribution.D_v << std::endl;
        std::cout << "\t\t fMyFoldGain.myFoldGain(" << (fMyFoldGain.suggestMeanOrRank() == 0 ? "MEAN" : "RANK") << ") = " << fMyFoldGain.myFoldGain(fMyFoldGain.suggestMeanOrRank()) << std::endl;
    }
#endif

    y = gainCombined.contribution.v; // e.g. gainWithFold*gainNormal*gainRaised;

    dy = gainCombined.contribution.D_v; // e.g. (gainWithFoldlnD+gainNormallnD+gainRaisedlnD)*y;

    y -= fMyFoldGain.myFoldGain(fMyFoldGain.suggestMeanOrRank());
    /* called with ea.ed */
#ifdef DEBUGASSERT
    if(std::isnan(y))
    {
        std::cout << "StateModel returning NaN is not okay" << std::endl;
        exit(1);
    }

    if (fMyFoldGain.getPlayerId() != estat->playerID) {
        //estat->ViewPlayer()

        std::cerr << "myFoldGain initialized with a player other than that which we intended to call myFoldGain on!" << std::endl;
        std::cerr << (int)(fMyFoldGain.getPlayerId()) << " != " << (int)(estat->playerID) << std::endl;
        exit(1);
    }
    //);
#endif // DEBUGASSERT


    delete [] raiseAmount_A;

    delete [] oppRaisedChance_A;


    delete [] potRaisedWin_A;


}

bool StateModel::willFoldToReraise
(
 const float64 raiseAmount // their hypothetical re-raise amount
 ,
 const float64 playGain
 ,
 const FoldOrCall & fMyFoldGain
 ,
 const ExpectedCallD & myInfo
 ,
 const float64 hypotheticalMyRaiseTo
) {
    const float64 betSize = hypotheticalMyRaiseTo;

    // An eventually raised showdown can happen gradually if there are more betting rounds!
    const struct FoldResponse oppRaisedMyFoldGain = fMyFoldGain.myFoldGainAgainstPredictedReraise(fMyFoldGain.suggestMeanOrRank(), //MEAN /* called with ea.ed */,
                                                                                                  myInfo.alreadyBet(), myInfo.callBet(), betSize, raiseAmount); //You would fold the additional (betSize - ea->alreadyBet() )

    const bool bIsOverbetAgainstThisRound = 0 < oppRaisedMyFoldGain.n; // this is an overbet even this round, so I'd just fold to it now and win more later


    if (bIsOverbetAgainstThisRound) {
        // We can profit from folding. This is a way overbet and we wouldn't call here then.
        return true;
    }

    // When we can't profit from folding, foldGain will return the concede gain, conveniently.
    const float64 concedeGain = oppRaisedMyFoldGain.gain;

    const bool bIsProfitableCall = concedeGain < playGain; // At least by calling I can win back a little more than losing concedeGain upfront

    if (!bIsProfitableCall) {
        // Not a profitable call either? Then I guess we'll fold to reraise. Return true.
        return true;
    }

    // ...



    return false;
}
