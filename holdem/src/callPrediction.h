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

#ifndef HOLDEM_CallPredict
#define HOLDEM_CallPredict


#include "math_support.h"
#include "inferentials.h"
#include "callPredictionFunctions.h"
#include "callSituation.h"

#if defined(DEBUG_TRACE_PWIN) || defined(DEBUG_TRACE_DEXF)
#define DEBUG_TRACE_EXACTCALL
#else
#undef DEBUG_TRACE_EXACTCALL
#endif

#ifdef DEBUG_TRACE_EXACTCALL
#include <iostream>
#endif

// All values should be expressed as a fraction of your bankroll (so that Geom and Algb can be compared directly)
// 1.0 is "no change"
class IStateCombiner {
public:
    virtual ~IStateCombiner() {}

    virtual struct AggregatedState createOutcome(float64 value, float64 probability, float64 dValue, float64 dProbability) const = 0;
    virtual struct AggregatedState createBlendedOutcome(const size_t arraySize, const ValueAndSlope * const values, const ValueAndSlope * const probabilities) const = 0;

    virtual struct AggregatedState combinedContributionOf(const struct AggregatedState &a, const struct AggregatedState &b, const struct AggregatedState &c) const = 0;
}
;

class IExf {
public:
    static constexpr float64 UNINITIALIZED_QUERY = -1;

    virtual ~IExf() {}


    virtual float64 exf(const float64 betSize) = 0;
    virtual float64 dexf(const float64 betSize) = 0;

}
;

///==============================
///   Pr{call} and Pr{raiseby}
///==============================
// TODO(from joseph): Rename to "ExactPlayD" to better contrast against `ExactCallBluffD` below
class ExactCallD : public IExf
{//NO ASSIGNMENT OPERATOR
    private:

        float64 totalexf;
        float64 totaldexf;

        void accumulateOneOpponentPossibleRaises(const int8 pIndex, ValueAndSlope * const nextNoRaise_A, const size_t noRaiseArraySize_now, const float64 betSize, const int32 callSteps, float64 * const overexf_out, float64 * const overdexf_out);

        const ExpectedCallD * const tableinfo;

        static constexpr int32 OPPONENTS_ARE_ALWAYS_ENCOURAGED_TO_RAISE = -1;
    protected:
        float64 queryinput;
		int32 querycallSteps;

        float64 nearest; // If you're raising and in exf we assume you have been called, tell us what else would have had to be added to the player most likely to make the smallest call to call you. This ensures that exf is accurate (pessimistic) when raising.
        float64 impliedFactor;



        int32 noRaiseArraySize;
        float64 *noRaiseChance_A;
        float64 *noRaiseChanceD_A;

        template<typename T> float64 facedOdds_call_Geom(const ChipPositionState & cps, float64 humanbet, float64 n,  CallCumulationD<T, OppositionPerspective> * useMean) const;
        template<typename T> float64 dfacedOdds_call_dbetSize_Geom(const ChipPositionState & cps, float64 humanbet, float64 dpot, float64 w, float64 n,  CallCumulationD<T, OppositionPerspective> * useMean) const;

        template<typename T> float64 facedOdds_raise_Geom(const struct HypotheticalBet & hypothetical, float64 startingPoint, float64 n, CallCumulationD<T, OppositionPerspective> * useMean) const;
        template<typename T> float64 dfacedOdds_raise_dfacedBet_GeomDEXF(const struct HypotheticalBet & hypothetical, float64 w, float64 opponents, float64 dexf, CallCumulationD<T, OppositionPerspective> * useMean) const;

        void query(const float64 betSize, const int32 callSteps);
    public:

    // By default, startingPoint == 0.0
    // When using this function for the purposes of nextNoRaise_A, you'll want to start at the previous value to avoid rounding errors.
    template<typename T> static float64 facedOdds_raise_Geom_forTest(float64 startingPoint, float64 denom, float64 riskLoss, float64 avgBlind, const struct HypotheticalBet & hypotheticalRaise, float64 opponents, CallCumulationD<T, OppositionPerspective> * useMean);


    // CallCumulationD &choicecumu = statprob.core.callcumu;
    // CallCumulationD &raisecumu = statprob.core.foldcumu;
    //    ExactCallBluffD myDeterredCall(&tablestate, &choicecumu, &raisecumu);
    struct CoreProbabilities &fCore;
    CommunityStatsCdf * ed() const {
        return &(fCore.callcumu);
    }
#ifdef DEBUG_TRACE_DEXF
		std::ostream * traceOut;
#endif

        ExactCallD(ExpectedCallD * const tbase //, CallCumulationD* data
                   ,
                   CoreProbabilities &core
                   )
        :
        tableinfo(tbase)
        ,
        impliedFactor(1)
        , noRaiseArraySize(0),noRaiseChance_A(0),noRaiseChanceD_A(0)
        ,
        //ed(data)
        fCore(core)
#ifdef DEBUG_TRACE_DEXF
					,traceOut(0)
#endif
            {
                queryinput = UNINITIALIZED_QUERY;
                querycallSteps = OPPONENTS_ARE_ALWAYS_ENCOURAGED_TO_RAISE;
            }

            virtual ~ExactCallD();


             float64 exf(const float64 betSize) override final;
             float64 dexf(const float64 betSize) override final;

            // pRaise() is the probability of being raised by RaiseAmount().
            // betSize is the bet you're considering, step is an iterator (because RasieAmount is a gradient but still discrete, so we only have hard pRaise() for each particular RaiseAmount().
            // If you want you could interpolate in between, but we typically just average the outcomes, since we're taking an expectation over all raise amounts that we might face.
            // callSteps is an index that indicates: "all iterator values (of step) starting from this one and higher, are raises that I would fold against)
            // In other worst, callSteps it the smallest RaiseAmount where we know we would just fold to it.
            static float64 RaiseAmount(const ExpectedCallD &tableinfo, const float64 betSize, int32 step);
			virtual float64 pRaise(const float64 betSize, const int32 step, const int32 callSteps );
			virtual float64 pRaiseD(const float64 betSize, const int32 step, const int32 callSteps );

            virtual void SetImpliedFactor(const float64 bonus);

  friend struct FacedOdds;
}
;

///======================
///   Pr{opponentFold}
///======================
class ExactCallBluffD
{//NO ASSIGNMENT OPERATOR
    private:

    template<typename T> float64 facedOdds_Algb(const ChipPositionState & cps, float64 bet,float64 opponents,  CallCumulationD<T, OppositionPerspective> * useMean) const;
    float64 facedOddsND_Algb(const ChipPositionState & cps, float64 bet, float64 dpot, float64 w, float64 n) const;

        //topTwoOfThree returns the average of the top two values {a,b,c} through the 7th parameter.
        //The average of the corresponding values of {a_d, b_d, c_d} are returned by the function.
        /*
        float64 topTwoOfThree(float64 a, float64 b, float64 c, float64 a_d, float64 b_d, float64 c_d, float64 & r) const;
        float64 bottomTwoOfThree(float64 a, float64 b, float64 c, float64 a_d, float64 b_d, float64 c_d, float64 & r) const;
        float64 topThreeOfFour(float64 a, float64 b, float64 c, float64 d, float64 a_d, float64 b_d, float64 c_d, float64 d_d, float64 & r) const;
        /*/
        // ^^^ TODO(from joseph): Unit test these if you want (see unittests/main.cpp), but for now I'm pretty sure they aren't used
        float64 bottomThreeOfFour(float64 a, float64 b, float64 c, float64 d, float64 a_d, float64 b_d, float64 c_d, float64 d_d, float64 & r) const;

        const ExpectedCallD * const tableinfo;

        FoldStatsCdf &fFoldCumu; // when they know your hand
        CommunityStatsCdf &fCallCumu; // when they _don't_ know your hand
    protected:

        float64 allFoldChance;
        float64 allFoldChanceD;

        float64 queryinputbluff;
        void query(const float64 betSize);

    public:
#ifdef DEBUG_TRACE_EXACTCALL
		std::ostream * traceOut;
#endif

        float64 insuranceDeterrent;

        ExactCallBluffD( ExpectedCallD * const tbase, struct CoreProbabilities &core )
    :
    tableinfo(tbase)
    ,
    fFoldCumu(core.foldcumu), fCallCumu(core.callcumu)
    #ifdef DEBUG_TRACE_EXACTCALL
					,traceOut(0)
    #endif
    ,
    insuranceDeterrent(0)
                            {
                                queryinputbluff = IExf::UNINITIALIZED_QUERY;
                            }



                            /**
                             *  pWin()
                             *
                             *   Parameters:
                             *     betSize:
                             *       The size of bet you would make
                             *
                             *   Return value:
                             *     The probability that everyone folds to your bet.
                             */
                            virtual float64 pWin(const float64 betSize);

                            virtual float64 pWinD(const float64 betSize);

            static float64 RiskPrice(const ExpectedCallD &tableinfo, FoldStatsCdf * foldcumu_caching);

            ~ExactCallBluffD();


}
;

// ExactCallD & pr_opponentcallraise;
// ExactCallBluffD & ea;
// const IStateCombiner & fStateCombiner;
// const ExpectedCallD * const myInfo;
struct TableSpec {
  ExpectedCallD * const tableView;
  const IStateCombiner & stateCombiner;
};

// Based on the bet size I am making, how many times can the opponent afford to fold?
// We use this value to determine our expected hand strength assuming the opponent knows what we have.
// By directly establishing the opponent's hand strength this way (as a function of our bet size), there is no need to offet "RiskLoss" anymore. As such, this obsoletes ExpectedCallD::RiskLoss
class OpponentHandOpportunity {
public:
    // facedHands: What does the opponent think his/her win percentage is based on rarity?
    // I guess if we are being pessimistic (which is the point of this exercise: assume that by betting we reveal what we have) they would know what we have.
    // So this should be foldcumu.
    OpponentHandOpportunity(playernumber_t myIdx, const HoldemArena& table, CoreProbabilities &core)
    :
    fTable(table)
    ,
    fLastBetSize(std::numeric_limits<float64>::signaling_NaN())
    ,
    fHandsToBeat(std::numeric_limits<float64>::signaling_NaN()),f_d_HandsToBeat_dbetSize(std::numeric_limits<float64>::signaling_NaN())
    ,
    //fCore(core)
    //,
    fIdx(myIdx)
    {}

    ~OpponentHandOpportunity() {}

    void query(const float64 betSize);
    float64 handsToBeat() { return fHandsToBeat; }
    float64 d_HandsToBeat_dbetSize() { return f_d_HandsToBeat_dbetSize; }

    const HoldemArena &fTable;
private:
    // query inputs
    float64 fLastBetSize;

    // query outputs
    float64 fHandsToBeat;
    float64 f_d_HandsToBeat_dbetSize;


    // Used strictly with FoldGain: What would your opponent be willing to lay down, knowing what you have.
    //CoreProbabilities &fCore;
    const playernumber_t fIdx;

}
;


#endif
