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


#ifndef HOLDEM_OpponentFunctions
#define HOLDEM_OpponentFunctions

#define OLD_BROKEN_RISKLOSS_WRONG_SIGN
// Current status:
//  * `#define OLD_BROKEN_RISKLOSS_WRONG_SIGN` fails testRegression_022 when switching to the `(nominalFoldChips + std::numeric_limits<float64>::epsilon() < trueFoldChipsEV)` condition
//     ↑ According to the test, h22 is supposed to bet on the Flop. But when `trueFoldChipsEV` is too close to `nominalFoldChips` it will check instead?
//  * `#undef OLD_BROKEN_RISKLOSS_WRONG_SIGN` fails testRegression_018
//     ↑ ActionBot18 really shouldn't check down the river with trip Aces when there aren't any flushes or straights to be afraid of. Do they really think someone with pockets hit a full house?

//#define REFINED_FACED_ODDS_RAISE_GEOM

//#define DEBUG_EXFDEXF
//#define DEBUG_TRACE_DEXF 2
//#define DEBUG_TRACE_P_RAISE 0
// ^^^ Define if you need to trace through a specific search
//     For example, if you are debugging `PureGainStrategy bot("abc.txt", 2)` you will want to `#define DEBUG_TRACE_DEXF 2`
// ^^^ Disable (i.e. You can explicitly `#undef DEBUG_TRACE_DEXF`) if you don't want `.github/workflows/ci.yml` to test with it


#include "functionbase.h"
#include "inferentials.h"
#include "portability.h"
#include <cmath>
#include <limits>

#define SACRIFICE_COMMITTED

// The probability of being dealt, e.g. two aces: 1 in 221
// HOWEVER, when used as a proxy for callcumulation, it might be too low because there is "same suit as you" as well as "different suits as you" optionality.
// Ideally this would be 50 * 49 or something.
#define RAREST_HAND_CHANCE 221.0

constexpr float64 SEARCH_SPACE_PROBABILITY_QUANTUM = 0.25/RAREST_HAND_CHANCE;
constexpr float64 DISPLAY_PROBABILITY_QUANTUM = 1.0/RAREST_HAND_CHANCE;

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
};

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

struct DBetSizeCache {
  bool b_assume_w_is_constant = false;
  float64 input_n = std::numeric_limits<float64>::signaling_NaN();
  float64 output_d_dbetSize = std::numeric_limits<float64>::signaling_NaN();

  constexpr bool bHasCachedValueFor(const float64 n) const {
    return (b_assume_w_is_constant && (input_n == n) && !std::isnan(output_d_dbetSize));
  }
};

// [!TIP]
// Read `FoldGainModel` instead. As far as I can tell it's the only code that uses this class.
// Essentially, `FoldWaitLengthModel` is only a helper computation used by the broader FoldGainModel below.
//
// [!WARNING]
// This template is instantiated at the bottom of src/callPredictionFunctions.cpp to avoid linker errors
template<typename T1, typename T2>
class FoldWaitLengthModel : public virtual ScalarFunctionModel
{
    private:
    float64 cacheRarity;
    protected:
    float64 getRawPCT(const float64 n);
    float64 d_rawPCT_d_n(const float64 n, const float64 rawPCT);
    float64 d_rawPCT_d_w(const float64 n, float64 rawPCT);


    float64 dRemainingBet_dn();
    float64 grossSacrifice(const float64 n);

    /**
     *  Parameters:
     *    rank:
     *      Your rank
     *  Returns:
     *    Either mean or rank (depending on mode) of the hand with the given rank
     */
    float64 lookup(const float64 rank) const;
    float64 dlookup(const float64 rank, const float64 mean) const;

    struct DBetSizeCache cached_d_dbetSize;

    // Describe the hand they would be folding
    float64 w;                    // Set to RANK if meanConv is null. Set to MEAN_winpct if using *meanConv
    CallCumulationD<T1, T2> (* meanConv); // Set to null if using RANK for payout simulation
public:

    // Describe the situation
    float64 amountSacrificeVoluntary; // you're giving up this much money each time you reach this situation (and fold) again
    float64 amountSacrificeForced; // you're giving up this much money each time you're dealt a new hand
    float64 bankroll;
    float64 opponents;
    float64 betSize; // if you do call, this is the amount you will win from this round
                     // NOTE: The way HoldemArena works, your total bets this round are still part of your money, so you only win the extra bets from other players.
    float64 prevPot; // if you do call, this is the amount you will win from previous rounds
                     // NOTE: The way HoldemArena works, your total bets from previous rounds are not part of your money, so you win the total prevPot including that which you committed.


    // NOTE: quantum is 1/3rd of a hand. We don't need more precision than that when evaluating f(n).
    FoldWaitLengthModel() : ScalarFunctionModel(1.0/3.0),
    cacheRarity(std::numeric_limits<float64>::signaling_NaN()),
    w(std::numeric_limits<float64>::signaling_NaN()), meanConv(nullptr), amountSacrificeVoluntary(std::numeric_limits<float64>::signaling_NaN()), amountSacrificeForced(std::numeric_limits<float64>::signaling_NaN()), bankroll(std::numeric_limits<float64>::signaling_NaN()), opponents(std::numeric_limits<float64>::signaling_NaN()), betSize(std::numeric_limits<float64>::signaling_NaN()), prevPot(std::numeric_limits<float64>::signaling_NaN())
    {}

    /*
    // NOTE: Although this is the copy constructor, it doesn't copy caches. This lets you clone a configuration and re-evaluate it.
    FoldWaitLengthModel(const FoldWaitLengthModel & o) : ScalarFunctionModel(1.0/3.0),
        cacheRarity(std::numeric_limits<float64>::signaling_NaN()), lastdBetSizeN(std::numeric_limits<float64>::signaling_NaN()), lastRawPCT(std::numeric_limits<float64>::signaling_NaN()), cached_d_dbetSize(std::numeric_limits<float64>::signaling_NaN()),
        w(o.w), meanConv(o.meanConv), amountSacrificeVoluntary(o.amountSacrificeVoluntary), amountSacrificeForced(o.amountSacrificeForced), bankroll(o.bankroll), opponents(o.opponents), betSize(o.betSize), prevPot(o.prevPot)
    {};
    */
    FoldWaitLengthModel(const FoldWaitLengthModel & o) = delete;

    const FoldWaitLengthModel & operator= ( const FoldWaitLengthModel & o ) = delete;
    void resetCaches() {
      this->cached_d_dbetSize.b_assume_w_is_constant = false;
      this->cached_d_dbetSize.input_n = std::numeric_limits<float64>::signaling_NaN();
      this->cached_d_dbetSize.output_d_dbetSize = std::numeric_limits<float64>::signaling_NaN();

      this->cacheRarity = std::numeric_limits<float64>::signaling_NaN();
    }
    void copyFrom_withCaches ( const FoldWaitLengthModel & o ) {
      this->copyFrom_noCaches(o);

      this->cacheRarity = o.cacheRarity;
      this->cached_d_dbetSize = o.cached_d_dbetSize;
    }
    void copyFrom_noCaches ( const FoldWaitLengthModel & o ) {
      this->w = o.w;
      this->meanConv = o.meanConv;
      this->amountSacrificeVoluntary = o.amountSacrificeVoluntary;
      this->amountSacrificeForced = o.amountSacrificeForced;
      this->bankroll = o.bankroll;
      this->opponents = o.opponents;
      this->betSize = o.betSize;
      this->prevPot = o.prevPot;
    }

    bool has_same_inputs ( const FoldWaitLengthModel & o ) const;
    bool has_same_cached_output_values ( const FoldWaitLengthModel & o ) const;
    bool operator== ( const FoldWaitLengthModel & o ) const = delete;

    virtual ~FoldWaitLengthModel();

    /*! @brief Here, n, is the number of HANDS, not the number of folds. */
    virtual float64 f(const float64);
    virtual float64 fd(const float64, const float64);
    virtual float64 d_dbetSize( const float64 n );
    virtual float64 d_dw( const float64 n );
    virtual float64 d_dC( const float64 n );

    virtual float64 FindBestLength();
    float64 rarity();


    void load(const ChipPositionState &cps, float64 avgBlind);
    void setAmountSacrificeVoluntary(float64 amount) {
        amountSacrificeVoluntary = (amount < 0) ? 0.0 : amount;
    }

    void setMeanConv(CallCumulationD<T1, T2> * new_meanConv);
    void setW(float64 neww);
    float64 getW() const;

}
;


// Q: You don't use FoldGainModel or FoldWaitLengthModel with handcumu, I don't think??
// Scenario: "Will my opponent *raise*?"
//   >>> ExpectedCallD::RiskLoss
//   If the opponent's raise decision depends on whether they think *you* will fold against their raise, your only choice is to use callcumu
// Scenario: "Will my opponent call?"
//   >>> ExactCallD::facedOdds_call_Geom
//   (requirements are same as "Will my opponent fold?" below)
// Scenario: "Will my opponent fold?"
//   >>> ExactCallBluffD::pWin (via FacedOddsAlgb)
//   If it's an opponent against you that knows your hand, they use foldcumu; e.g. "opponent folds until they get a hand that can catch you"
//   If it's an opponent against you that *doesn't* know your hand, they use callcumu.
// Scenario: "Should I fold?"
//   >>> FoldOrCall::foldGain
//   If you're heads-up, I… SUPPOSE you could use handcumu, e.g. "fold until their hand gets worse"
//      ^^^ but this doesn't really make sense because in order to reach the same situation again their hand would have to be just as good
//   If it's an opponent that doesn't know your hand, you use callcumu, e.g. "fold until my hand gets better"
//   OR use rank (nullptr a.k.a. EMPTY_DISTRIBUTION) i.e. "fold until my hand gets better AND their hand gets worse"
template<typename T1, typename T2>
class FoldGainModel : public virtual ScalarFunctionModel
{
    protected:
    FoldWaitLengthModel<T1, T2> lastWaitLength; // cached version of `FoldWaitLengthModel waitLength` below
    float64 lastBetSize;
    float64 last_dw_dbet;
    float64 lastf;
    float64 lastfd;
    float64 lastFA; // df_dw
    float64 lastFB;
    float64 lastFC; // d_dAmountSacrificePerHand

    void query(const float64 betSize);

    public:
    float64 n;
    // float64 dw_dbet; // input for lastFA



    FoldWaitLengthModel<T1, T2> waitLength;

    FoldGainModel(float64 myQuantum) : ScalarFunctionModel(myQuantum)
            , lastWaitLength(), lastBetSize(-1), last_dw_dbet(0) //Cache variables
            //, dw_dbet(0) // optional input variables
            {};

    virtual ~FoldGainModel();

    virtual float64 f(const float64 betSize);
    virtual float64 fd(const float64 betSize, const float64 gain);
    virtual float64 F_a(const float64 betSize);
    virtual float64 F_b(const float64 betSize);

    // "AmountSacrifice" is the number of chips you are effectively losing every time you fold
    // See also `FoldWaitLengthModel::d_dC`
    virtual float64 dF_dAmountSacrifice(const float64 betSize);
}
;
template class FoldGainModel<void, void>;

//How much call can you pick up to your bet?
template<typename T>
class FacedOddsCallGeom : public virtual ScalarFunctionModel
{
    protected:
    float64 lastW;
    float64 lastF;
    float64 lastFD;
    void query( const float64 w );
    public:
    float64 B;
    float64 pot;
//    float64 alreadyBet;
    float64 outsidebet;
    float64 opponents;


    FoldGainModel<T, OppositionPerspective> FG;
    FacedOddsCallGeom(float64 myQuantum) : ScalarFunctionModel(SEARCH_SPACE_PROBABILITY_QUANTUM), lastW(-1), FG(myQuantum/2) {}
    virtual float64 f(const float64 w);
    virtual float64 fd(const float64 w, const float64 U);
}
;
template class FacedOddsCallGeom<PlayerStrategyPerspective>;

//Will everybody fold consecutively to your bet?
template<typename T>
class FacedOddsAlgb : public virtual ScalarFunctionModel
{
    protected:
    float64 lastW;
    float64 lastF;
    float64 lastFD;
    void query( const float64 w );
    public:
    float64 pot;
    //float64 alreadyBet;
    float64 betSize;


    FoldGainModel<T, OppositionPerspective> FG;
    FacedOddsAlgb(float64 myQuantum) : ScalarFunctionModel(SEARCH_SPACE_PROBABILITY_QUANTUM), lastW(-1), FG(myQuantum/2) {}
    virtual float64 f(const float64 w);
    virtual float64 fd(const float64 w, const float64 U);
}
;
template class FacedOddsAlgb<PlayerStrategyPerspective>;
template class FacedOddsAlgb<void>;

// The key factors that influence "RiskLoss"
struct RiskLoss {
  float64 comparisonCutoff;
  float64 nominalFoldChips;
  float64 trueFoldChipsEV;
  float64 d_trueFoldChipsEV_dpot;

  constexpr bool any_nan() const {
    return std::isnan(comparisonCutoff) || std::isnan(nominalFoldChips) || std::isnan(trueFoldChipsEV) || std::isnan(d_trueFoldChipsEV_dpot);
  }

  // When `trueFoldChipsEV` is sufficiently low, there is no benefit to folding so they might as well make their stand
  constexpr bool b_raise_will_be_called() const {
  // We move comparisonCutoff to the nominalFoldChips side because
  //   `trueFoldChipsEV + comparisonCutoff < nominalFoldChips`
  // will never happen, considering FoldGain would simply set `FG.n == 0` in that case
    return trueFoldChipsEV <= nominalFoldChips + comparisonCutoff;
    // If trueFoldChipsEV offers any benefit at all, then the player who made `faced_bet` could benefit more by folding, meaning it's not productive this opponent (the person doing HypotheticalBet right now) to raise as high as `hypotheticalRaise.hypotheticalRaiseTo`
    // As such, we need to penalize this `hypotheticalRaise.hypotheticalRaiseTo` by returning a riskLoss quantity that represents this surplus
  }

  // If trueFoldChipsEV is *strictly profitable*, then the player who made `faced_bet` could "win" by folding, meaning it's overly risky for this person (doing HypotheticalBet right now) to raise as high as `hypotheticalRaise.hypotheticalRaiseTo`
  constexpr bool b_raise_is_too_dangerous() const {
    return comparisonCutoff < trueFoldChipsEV;
  }

  constexpr ValueAndSlope riskLoss_adjustment_for_raising_too_much() const {
    ValueAndSlope riskLossAdjustment_by_pot = {
      b_raise_is_too_dangerous() ? ( -trueFoldChipsEV ) : ( 0.0 ) ,

			(!b_raise_will_be_called()) ? (
          -d_trueFoldChipsEV_dpot
      )
      : ( 0.0 )
    };

    return riskLossAdjustment_by_pot;
  }

  constexpr ValueAndSlope old_broken_riskloss_wrong_sign() const {
    #ifdef OLD_BROKEN_RISKLOSS_WRONG_SIGN
      if (b_raise_will_be_called()) {
    #else
      if (!b_raise_will_be_called()) {
    #endif
        ValueAndSlope riskLoss_by_pot = {
          trueFoldChipsEV - nominalFoldChips, d_trueFoldChipsEV_dpot
        };

        return riskLoss_by_pot;
      }
    return ValueAndSlope{0.0, 0.0};
  }
}
;

// How much/likely would they raise or reraise?
//
// [!WARNING]
// This template is instantiated at the bottom of src/callPredictionFunctions.cpp to avoid linker errors
template<typename T>
class FacedOddsRaiseGeom : public virtual ScalarFunctionModel
{
    protected:
    float64 lastW;
    ValueAndSlope lastF_by_w;
    void query( const float64 w );

    public:
    #ifdef DEBUG_TRACE_P_RAISE
      std::ostream * traceOut_pRaise;
    #endif

    float64 raisedPot;
    float64 callPot;
    float64 raiseTo;
    float64 fold_bet; // if I "fold" instead of `raiseTo`, what bet can we get back to just by waiting?
    float64 faced_bet; // if I "call", I increase my bet **to** `faced_bet` (i.e. increase my bet **by** `faced_bet - fold_bet`)
    struct RiskLoss riskLoss;
	  struct SimulateReraiseResponse bRaiseWouldBeCalled; // If this is `true`, at least *one* person will cause the raise (you won't get an all-fold situation)
    bool bCheckPossible;

    FoldGainModel<T, OppositionPerspective> FG;
    FacedOddsRaiseGeom(float64 myQuantum) : ScalarFunctionModel(SEARCH_SPACE_PROBABILITY_QUANTUM),
      lastW(-1),
    #ifdef DEBUG_TRACE_P_RAISE
      traceOut_pRaise(nullptr),
    #endif
      riskLoss(RiskLoss{std::numeric_limits<float64>::signaling_NaN(), std::numeric_limits<float64>::signaling_NaN(), std::numeric_limits<float64>::signaling_NaN(), std::numeric_limits<float64>::signaling_NaN()}),
      FG(myQuantum/2)
    {}

    virtual float64 f(const float64 w);
    virtual float64 fd(const float64 w, const float64 U);

    // You MUST populate `this->FG.waitLength` first, before calling this. We will populate everything else here.
    static void configure_with(FacedOddsRaiseGeom &a, const HypotheticalBet &hypotheticalRaise, const struct RiskLoss &currentRiskLoss);
}
;

#endif
