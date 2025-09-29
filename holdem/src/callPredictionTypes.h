/***************************************************************************
 *   Copyright (C) 2025 by Joseph Huang                                    *
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

#ifndef HOLDEM_CallPredictionTypes
#define HOLDEM_CallPredictionTypes

#include "portability.h"
#include <cstddef>
#include <limits>
#include <utility>

typedef std::pair<int32, int32> firstFoldToRaise_t;


struct ChipPositionState
{
    ChipPositionState(float64 stack, float64 tablepot, float64 sofar, float64 commit, float64 pastpot)
    : bankroll(stack), pot(tablepot), alreadyBet(sofar), alreadyContributed(commit), prevPot(pastpot)
    {}

    const float64 bankroll;
    float64 pot;
    const float64 alreadyBet; // What is my bet so far, this round? (As I understand it, this is included in `pot`)
    const float64 alreadyContributed; // This is the same as `alreadyBet` (except it excludes any blinds, until you call or raise)
    float64 prevPot;
}
;

struct SimulateReraiseResponse {
  bool bGoodFold; // You forced an overbet, so go ahead and fold because you will profit from this.
  bool bUnprofitable;

  SimulateReraiseResponse() = delete;
  explicit constexpr SimulateReraiseResponse(bool bG, bool bU) : bGoodFold(bG), bUnprofitable(bU) {};

  // You got caught on an unprofitable hand but now at a higher bet size. We need to fold but would have preferred to not have raised in the first place.
  constexpr bool bBadFold() const {
    return bUnprofitable && !bGoodFold;
  }

  constexpr static SimulateReraiseResponse at_callSteps(firstFoldToRaise_t callSteps, int32 i) {
    return SimulateReraiseResponse {
      callSteps.first <= i,
      callSteps.second <= i
    };
  }

  static constexpr firstFoldToRaise_t all_reraises_considered_bad_folds() {
    static_assert(sizeof(int32) <= sizeof(size_t), "We'll be using int32_t::max() and it needs to fit into both. Thanks!");
    // TODO(from joseph): There is room for very minor further tuning of what to do during FacedOddsRaiseGeom::query and its relationship with ExpectedCallD::RiskLossHeuristic
    return std::pair<int32, int32> { -1, std::numeric_limits<int32>::max() - 1 };
  }

  // This is a strictly invalid state. If you look at how StateModel::firstFoldToRaise_only and/or StateModel::calculate_final_potRaisedWin work, it should be essentially impossible to interpret bIsOverbetAgainstThisRound without bUnprofitable also being true at the same time
  // Use it to represent a value of "uninitialized"
  static constexpr SimulateReraiseResponse construct_uninitialized() {
    return SimulateReraiseResponse {
      true, false
    };
  }
}; // end SimulateReraiseResposne

struct HypotheticalBet {
  const struct ChipPositionState &bettorSituation;
  float64 hypotheticalRaiseTo;
  float64 hypotheticalRaiseAgainst;
  const float64 counterfactualFoldAbandon_raw; // This is the same as `bettorSituation.alreadyContributed` EXCEPT it includes any blinds you are on the hook for as well. In theory it's exactly `bettorSituation.alreadyBet`
  const struct SimulateReraiseResponse bWillGetCalled;

  // if you're _RE-RAISING_ this is the increase compared to the highest bet so far
  constexpr float64 raiseBy() const {
    return hypotheticalRaiseTo - hypotheticalRaiseAgainst;
  }

   // if you're _RE-RAISING_ this is the increase compared to YOUR last bet
  constexpr float64 betIncrease() const {
    return hypotheticalRaiseTo - bettorSituation.alreadyBet;
  }

  constexpr float64 fold_bet() const {
    if( counterfactualFoldAbandon_raw > bettorSituation.bankroll )
    {
      return bettorSituation.bankroll;
    } else {
      return counterfactualFoldAbandon_raw;
    }
  }

  // It seems to be that:
  //  (i) The underlying `accumulateOneOpponentPossibleRaises` and thus `ExactCallD::query` is a search over ExactCallD::query's `betSize`
  //      so when we take the derivative, we want to sample the derivative at the value of `betSize` corresponding to ExactCallD::query
  //  (ii) The FoldGain of a position is based on the bet you're facing (and would consider folding against INSTEAD OF following through on HypotheticalBet)
  //       Well, that bet you're facing is exactly `hypothetical.hypotheticalRaiseAgainst`
  // and… well what do you know? These two (i) and (ii) are the same value.
  constexpr float64 faced_bet() const {
    if( hypotheticalRaiseAgainst > bettorSituation.bankroll )
    {
      return bettorSituation.bankroll;
    } else {
      return hypotheticalRaiseAgainst;
    }
  }

  constexpr bool bEffectivelyAllIn(const float64 chipDenom) const {
    return (bettorSituation.bankroll - chipDenom/2.0 <= hypotheticalRaiseTo);
  }

  constexpr bool bMoreThanAllIn() const {
    return bettorSituation.bankroll < hypotheticalRaiseTo;
  }

  // either because you're first to act OR because the only bet so far is equal to your blind bet
  constexpr bool bCouldHaveChecked() const {
    //If oppBetAlready == betSize AND table->CanRaise(pIndex, playerID), the player must be in the blind. Otherwise,  table->CanRaise(pIndex, playerID) wouldn't hold
    const bool bOppCouldCheck = (hypotheticalRaiseAgainst == 0.0) || /*(betSize == callBet())*/(counterfactualFoldAbandon_raw == hypotheticalRaiseAgainst);//If oppBetAlready == betSize AND table->CanRaise(pIndex, playerID), the player must be in the blind. Otherwise,  table->CanRaise(pIndex, playerID) wouldn't hold
    //The other possibility is that your only chance to raise is in later rounds. This is the main force of bWouldCheck.
    return bOppCouldCheck;
  }
}
;


#endif // HOLDEM_CallPredictionTypes
