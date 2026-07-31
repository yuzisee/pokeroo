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
#include "callPrediction.h"
#include "math_support.h"

#include <float.h>

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
        if(traceEnable != nullptr) std::cout << "\t\t\tbNoRange" << std::flush;
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
            if(traceEnable != nullptr) std::cout << "\t\t\tbMax" << std::flush;
#endif
        }
        else if( slider <= 0 )
        {
            yl = left.f(x);
            fd_yl = left.fd(x, yl);

            y = yl; dy = fd_yl;

#ifdef DEBUG_TRACE_SEARCH
            if(traceEnable != nullptr) std::cout << "\t\t\tbMin" << std::flush;
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

				const float64 rightWeight = std::log1p(slider)/std::log(2.0);
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
                    if(traceEnable != nullptr) std::cout << "\t\t\t y(" << x << ") = " << yl << " * " << (1-slider) << " + " <<  yr << " * " << slider << std::endl;
                    if(traceEnable != nullptr) std::cout << "\t\t\t dy = " << fd_yl << " * " << (1-slider) << " - " <<  yl << " * " << autoSlope << " + " <<  fd_yr << " * " << slider << " + " <<  yr << " * " << autoSlope << std::endl;
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


ValueAndSlope StateModel::g_raised(float64 raisefrom, const float64 betSize)
{
    const float64 potWin = fp->f_raised(raisefrom, betSize);

    const float64 yval = potWin; // Subtract out foldgain, to get f_raised without computing it again.

    return ValueAndSlope {
      potWin,
      fp->fd_raised(raisefrom, betSize, yval) // There is no derivative of ea.FoldGain() here because it is constant relative to betSize.
    };
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

static constexpr float64 INVISIBLE_PERCENT = EPS_WIN_PCT;

std::pair<firstFoldToRaise_t, FoldOrCall> StateModel::calculate_final_potRaisedWin(const size_t arraySize, ValueAndSlope * potRaisedWin_A, const float64 betSize) {

  size_t found_firstBadFoldToRaise = arraySize;
  size_t found_firstGoodFoldToRaise = arraySize;

  const FoldOrCall myFoldGain(*(table_spec.tableView->table), c.fCore); // My current foldgain with the same units as my CombinedStatResult (for proper comparison with call vs. fold)

	for( size_t i=0;i<arraySize; ++i)
     {
         const float64 sliderx = betSize;
         // TODO(from yuzisee): Explore using
         /* fMyFoldGain.predictedRaiseToThisRound(<#float64 currentBetToCall#>, <#float64 currentAlreadyBet#>, <#float64 predictedRaiseTo#>) */
         // as sliderx?


         const float64 evalX =  ExactCallD::RaiseAmount(*table_spec.tableView, betSize,i);
         // g_raised ultimately calls into either `GainModelGeom` or `GainModelNoRisk`, both of which are IExf of their own
         potRaisedWin_A[i] = g_raised(sliderx,evalX); // as you can see below, this value is only blended in if we are below firstFoldToRaise
         // ^^^ Can't use `.set_value_and_slope` here because we need `potRaisedWin_A[i].v` when computing `potRaisedWin_A[i].D_v`

         const struct SimulateReraiseResponse will_fold_to_reraise = willFoldToReraise(evalX, potRaisedWin_A[i].v, myFoldGain, *(table_spec.tableView), betSize
           #ifdef DEBUG_WILL_FOLD_TO_RERAISE
           , nullptr
           #endif
         );
         if (will_fold_to_reraise.bGoodFold) {
           if( found_firstGoodFoldToRaise == arraySize ) found_firstGoodFoldToRaise = i;
         }
         else if (will_fold_to_reraise.bBadFold())
         {
           if( found_firstBadFoldToRaise == arraySize ) { found_firstBadFoldToRaise = i; }

           const float64 g_raised_by_folding = 1.0 - table_spec.tableView->betFraction(betSize);
           if (potRaisedWin_A[i].v < g_raised_by_folding) {
             //Since g_raised isn't pessimistic based on raiseAmount (especially when we're just calling), don't add additional gain opportunity -- we should instead assume that if we would fold against such a raise that the opponent has us as beat as we are.
             potRaisedWin_A[i].set_value_and_slope(
               // Deduct the bet you make, and fold
               g_raised_by_folding, -1.0
             );
           }
         }

     } // end loop over [i]

	return std::make_pair(std::make_pair(found_firstGoodFoldToRaise, found_firstBadFoldToRaise), myFoldGain);
}
// ↑↓ are these two basically the same thing? TODO(from joseph): Can we combine/simplify?
template< typename T >
firstFoldToRaise_t StateModel::firstFoldToRaise_only(T & m, const float64 displayBet, const FoldOrCall &myFoldGain, const ExpectedCallD & tablestate, const float64 maxShowdown
#ifdef DEBUG_WILL_FOLD_TO_RERAISE
, std::ostream &logF
#endif
) {
  int32 firstGoodFoldToRaise = -1;
  int32 firstBadFoldToRaise = -1;

    int32 raiseStep = 0;
  float64 rAmount;
  for(raiseStep = 0, rAmount = 0.0; rAmount < maxShowdown; ++raiseStep )
  {
      rAmount =  ExactCallD::RaiseAmount(tablestate, displayBet, raiseStep);

            const float64 oppRaisedPlayGain = m.g_raised(displayBet, rAmount).v;
            const struct SimulateReraiseResponse will_fold_to_reraise = StateModel::willFoldToReraise(rAmount, oppRaisedPlayGain, myFoldGain, tablestate, displayBet
              #ifdef DEBUG_WILL_FOLD_TO_RERAISE
              , &logF
              #endif
            );
            if (will_fold_to_reraise.bGoodFold) {
              if (firstGoodFoldToRaise == -1) {
                firstGoodFoldToRaise = raiseStep;
              }
            } else if (will_fold_to_reraise.bBadFold()) {
              if (firstBadFoldToRaise == -1) {
                firstBadFoldToRaise = raiseStep;
              }
            }
            if ((firstBadFoldToRaise != -1) && (firstGoodFoldToRaise != -1))
            { break; /* We'd fold at this point. Stop incrementing */ }
  } // end for rAmount (raiseStep)

  if (firstBadFoldToRaise == -1) {
    firstBadFoldToRaise = raiseStep;
  }
  if (firstGoodFoldToRaise == -1) {
    firstGoodFoldToRaise = raiseStep;
  }

  return std::make_pair(firstGoodFoldToRaise, firstBadFoldToRaise);
}
template firstFoldToRaise_t StateModel::firstFoldToRaise_only(StateModel & m, const float64 displayBet, const FoldOrCall &myFoldGain, const ExpectedCallD & tablestate, const float64 maxShowdown
  #ifdef DEBUG_WILL_FOLD_TO_RERAISE
  , std::ostream &logF
  #endif
);

ValueAndSlope StateModel::calculate_oppRaisedChance(const float64 betSize, const size_t arraySize, ValueAndSlope * const oppRaisedChance_A, const firstFoldToRaise_t firstFoldToRaise, ValueAndSlope * const potRaisedWin_A, const ValueAndSlope &oppFoldChance) const {
      ValueAndSlope lastuptoRaisedChance = {0.0, 0.0};

      const bool bCallerWillPush = (arraySize == 1); //If arraySize is 1, then minRaise goes over maxbet. Anybody who can call will just reraise over the top.

      // TODO(from joseph) ValueAndSlope
      float64 newRaisedChance = 0;
      float64 newRaisedChanceD = 0;
      // Read the values of ea's pRaise below so they can be combined with the values of g_raised above to get an aggregate gain.
      // Note that we read from the top down, in case it's not monotonic (which can occur due to pessimistic bMyWouldCall) and
      // the higher of the raises takes precedent (which is the Fold outcome, which trumps expecting lower bets to make money if you can be pushed)
      for( int32 i=arraySize-1;i>=0; --i)
      {
          if( bCallerWillPush ) {
              //ASSERT: i == 0 && arraySize == 1 && lastUptoRaisedChance == 0 && oppFoldChance and oppFoldChanceD have already been determined
              newRaisedChance = 1 - oppFoldChance.v;
              newRaisedChanceD = - oppFoldChance.D_v;
          }else{
              //Standard calculation
              const ValueAndSlope pr_raiseto_showdown = c.pRaise(betSize,i,firstFoldToRaise);
              // NOTE! Pr{Raise} includes the probability of being raised later in future rounds too
              newRaisedChance = pr_raiseto_showdown.v;
              newRaisedChanceD = pr_raiseto_showdown.D_v;
          }

		if( newRaisedChance - lastuptoRaisedChance.v > INVISIBLE_PERCENT )
		{
			oppRaisedChance_A[i].v = newRaisedChance - lastuptoRaisedChance.v;
			oppRaisedChance_A[i].D_v = newRaisedChanceD - lastuptoRaisedChance.D_v;
			lastuptoRaisedChance.v = newRaisedChance;
			lastuptoRaisedChance.D_v = newRaisedChanceD;
		}
		//if( oppRaisedChance_A[i] < INVISIBLE_PERCENT )
		else
          {
              //raiseAmount_A[i] = 0;
              oppRaisedChance_A[i].clearToZero();
              potRaisedWin_A[i].v = 1; // "no change"
              potRaisedWin_A[i].D_v = 0;
          }

  #ifdef DEBUGASSERT
          if (oppRaisedChance_A[i].any_nan()) {
              std::cerr << "oppRaisedChance_A[" << static_cast<int>(i) << "] should not be NaN" << std::endl;
              std::cerr << oppRaisedChance_A[i].v << " = " << newRaisedChance << " − " << lastuptoRaisedChance.v << " during bCallerWillPush=" << bCallerWillPush << std::endl;
              std::cerr << oppRaisedChance_A[i].D_v << " = " << newRaisedChanceD << " − " << lastuptoRaisedChance.D_v << std::endl;
              exit(1);
          }
  #endif // DEBUGASSERT

  #ifdef DEBUG_TRACE_SEARCH
          if(traceEnable != nullptr) {
              std::cout << "\t\t(oppRaiseChance[" << i << "] , cur, highest) = " << oppRaisedChance_A[i].v  << " , "  << newRaisedChance << " , " << lastuptoRaisedChance.v << std::endl;
          }
  #endif
      }

      return lastuptoRaisedChance;
}

//Count needed array size
int32 StateModel::state_model_array_size_for_blending(float64 betSize) const {
  int32 arraySize = 0;
  while( ExactCallD::RaiseAmount(*table_spec.tableView, betSize,arraySize) < table_spec.tableView->maxRaiseAmount() )
  {
      ++arraySize;
  }
  //This array loops until noRaiseArraySize is the index of the element with RaiseAmount(noRaiseArraySize) == maxBet()
  if(betSize < table_spec.tableView->maxRaiseAmount()) ++arraySize; //Now it's the size of the array (unless you're pushing all-in already)

  return arraySize;
}

// The primary purpose of this query is to return `y` which is our E[x] if betting `betSize`
void StateModel::query( const float64 betSize )
{
    // betSize here is always "my" bet size. The perspective of opponents is already covered in <tt>ea</tt>

    last_x = betSize;

    ///Establish [PushGain] values

	float64 potFoldWin = table_spec.tableView->PushGain();
	const float64 potFoldWinD = 0;
#ifndef DUMP_CSV_PLOTS
  ValueAndSlope
#endif // if ! DUMP_CSV_PLOTS
    oppFoldChance = { ea.pWin(betSize), ea.pWinD(betSize) };

    //#ifdef DEBUGASSERT
	if( potFoldWin < 0 || oppFoldChance.v < INVISIBLE_PERCENT ){
		potFoldWin =  1;
		oppFoldChance.clearToZero();
	}
    //#endif

    ///Establish [Raised] values

    //Create arrays
    const int32 arraySize = state_model_array_size_for_blending(betSize);
    ValueAndSlope * potRaisedWin_A = new ValueAndSlope[arraySize];
    ValueAndSlope * oppRaisedChance_A = new ValueAndSlope[arraySize];
    // TODO(from joseph): Go with `unique_ptr` instead?

    std::pair<firstFoldToRaise_t, FoldOrCall> potRaised_delimiters = calculate_final_potRaisedWin(arraySize, potRaisedWin_A, betSize);
    const FoldOrCall fMyFoldGain = std::move(potRaised_delimiters.second);

    ValueAndSlope lastuptoRaisedChance = calculate_oppRaisedChance(betSize, arraySize, oppRaisedChance_A, potRaised_delimiters.first, potRaisedWin_A, oppFoldChance);

    ValueAndSlope potNormalWin = g_raised(betSize,betSize);

    ///Establish [Play] values
#ifndef DUMP_CSV_PLOTS
	float64
#endif // if ! DUMP_CSV_PLOTS
	playChance = 1 - oppFoldChance.v - lastuptoRaisedChance.v;
	float64 playChanceD = - oppFoldChance.D_v - lastuptoRaisedChance.D_v;
    if( 0.0 < playChance && playChance <= INVISIBLE_PERCENT ) //roundoff, but {playChance == 0} is push-fold for the opponent
    {
        //Correct other odds
        const float64 totalChance = 1.0 - playChance;
        for( int32 i=arraySize-1;i>=0; --i)
        {
            oppRaisedChance_A[i].rescale (1.0 / totalChance);
        }
              oppFoldChance.rescale(1.0 / totalChance);

        //Remove call odds
        playChance = 0;
        playChanceD = 0;
              potNormalWin.set_value_and_slope(1.0, 0.0);
    }


    ///Calculate factors

    outcomeCalled = table_spec.stateCombiner.createOutcome(potNormalWin.v, playChance, potNormalWin.D_v, playChanceD);
    blendedRaises = table_spec.stateCombiner.createBlendedOutcome(arraySize, potRaisedWin_A, oppRaisedChance_A);
    outcomePush = table_spec.stateCombiner.createOutcome(potFoldWin, oppFoldChance.v, potFoldWinD, oppFoldChance.D_v);

     ///Store results
     struct AggregatedState gainCombined = table_spec.stateCombiner.combinedContributionOf(outcomePush, outcomeCalled, blendedRaises);

#ifdef DEBUG_TRACE_SEARCH
    if(traceEnable != nullptr) {
      // https://github.com/yuzisee/pokeroo/commit/ecec2a0e4f8d119a01f310fef9ce4e4652c3ce58
        std::cout << "\t\t (gainWithFold*gainNormal*gainRaised) = " << gainCombined.contribution.v << std::endl;
        std::cout << "\t\t ((gainWithFoldlnD+gainNormallnD+gainRaisedlnD)*y) = " << gainCombined.contribution.D_v << std::endl;
        std::cout << "\t\t fMyFoldGain.myFoldGain(" << (fMyFoldGain.suggestMeanOrRank() == 0 ? "MEAN" : "RANK") << ") = " << fMyFoldGain.myFoldGain(fMyFoldGain.suggestMeanOrRank()) << std::endl;
    }
#endif

    y = gainCombined.contribution.v; // e.g. gainWithFold*gainNormal*gainRaised;

    dy = gainCombined.contribution.D_v; // e.g. (gainWithFoldlnD+gainNormallnD+gainRaisedlnD)*y;

    if (table_spec.tableView->ViewPlayer()->GetBetSize() < table_spec.tableView->table->GetBetToCall()) {
      // Only consider the action of betting `betSize` to be a "positive" action if it's better than folding.
      y -= fMyFoldGain.myFoldGain(fMyFoldGain.suggestMeanOrRank());
    } else {
      y -= 1.0; // When you check, nothing happens. You don't win money and you don't lose money. You stay at 1.0 of your bankroll.
    }

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

    delete [] oppRaisedChance_A;
    delete [] potRaisedWin_A;
} // end StateModel::query

SimulateReraiseResponse StateModel::willFoldToReraise
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
 #ifdef DEBUG_WILL_FOLD_TO_RERAISE
 , std::ostream * trace_statemodel
 #endif
) {
    const float64 betSize = hypotheticalMyRaiseTo;

    // An eventually raised showdown can happen gradually if there are more betting rounds!
    const struct FoldResponse oppRaisedMyFoldGain = fMyFoldGain.myFoldGainAgainstPredictedReraise(fMyFoldGain.suggestMeanOrRank(), //MEAN /* called with ea.ed */,
                                                                                                  myInfo.alreadyBet(), myInfo.callBet(), betSize, raiseAmount); //You would fold the additional (betSize - ea->alreadyBet() )

    const bool bIsOverbetAgainstThisRound = 0 < oppRaisedMyFoldGain.n; // this is an overbet even this round, so I'd just fold to it now and win more later

    // When we can't profit from folding, foldGain will return the concede gain, conveniently.
    const float64 concedeGain = oppRaisedMyFoldGain.gain;

    const bool bIsProfitableCall = concedeGain < playGain; // At least by calling I can win back a little more than losing concedeGain upfront

    if (bIsOverbetAgainstThisRound) {
      // We can profit from folding. This is a way overbet and we wouldn't call here then.
      #ifdef DEBUG_WILL_FOLD_TO_RERAISE
        if (trace_statemodel != nullptr) {
          *trace_statemodel << "\t\tStateModel::willFoldToReraise being re-raised to $" << raiseAmount << " would be a clear overbet,";
          if ((myInfo.alreadyBet() == 0.0) && (myInfo.callBet() == 0.0) && (betSize == 0.0)) {
            *trace_statemodel << " as the first action";
          } else {
            *trace_statemodel << " if I first raised from " << myInfo.alreadyBet() << " to " << betSize << " in the face of " <<  myInfo.callBet();
          }
          *trace_statemodel << " — folding would allow me to improve on my current " << /* fMyFoldGain.fCore.statmean.pct */ myInfo.meanW << " winPCT" << std::endl;
        }
      #endif

    } else if (!bIsProfitableCall) {
      // Not a profitable call either? Then I guess we'll fold to reraise. Return true.
      #ifdef DEBUG_WILL_FOLD_TO_RERAISE
        if (trace_statemodel != nullptr) {
          *trace_statemodel << "\t\tStateModel::willFoldToReraise will fold to $" << raiseAmount << " after we already tried first raising to $" << hypotheticalMyRaiseTo << " based on comparing " << ((playGain - 1.0) * myInfo.ViewPlayer()->GetMoney() ) << "⛀ ≤ " << ((concedeGain - 1.0) * myInfo.ViewPlayer()->GetMoney()) << "⛀" << std::endl;
        }
      #endif

    }

    return SimulateReraiseResponse {
      bIsOverbetAgainstThisRound,
      !bIsProfitableCall
    };
}
