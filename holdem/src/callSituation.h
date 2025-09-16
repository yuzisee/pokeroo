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

#ifndef HOLDEM_ArenaSituations
#define HOLDEM_ArenaSituations

#include "arena.h"
#include "callPredictionFunctions.h"

//#define DEBUG_EXFDEXF

#define ANTI_PRESSURE_FOLDGAIN
#define CONSISTENT_AGG
//#define PURE_BLUFF


#define BLIND_ADJUSTED_FOLD
//#define GEOM_COMBO_FOLDPCT





class ExpectedCallD
{//NO ASSIGNMENT OPERATOR


public:
    const int8 playerID;

#ifdef ANTI_PRESSURE_FOLDGAIN
    const float64 handRarity;
    const float64 meanW;
#endif

    const HoldemArena* const table;
    ExpectedCallD(const int8 id, const HoldemArena* base
#ifdef ANTI_PRESSURE_FOLDGAIN
            , const float64 rankPCT, const float64 meanPCT
#endif
            )
    :
    playerID(id)
    ,
    #ifdef ANTI_PRESSURE_FOLDGAIN
    handRarity(1-rankPCT), meanW(meanPCT)
    #endif
    ,
    table(base)
    {}

    virtual ~ExpectedCallD();

    /**
     * The "opportunity cost of folding" formula can also be applied from the perspective of the raiser,
        rather than the folder, in a sort of "opportunity cost of raising" heuristic.
     * The reverse application of the formula is implemented in a function called ``RiskLoss``.
     * This ``RiskLoss`` heuristic reports a loss (negative value) if your bet is large enough for the average opponent to profit (opportunistically) by folding and waiting for a better hand.
     * Since making a small bet does not allow an average opponent to profit via his/her opportunity cost of folding, your ``RiskLoss`` remains zero as long as your bet is suffciently small compared to an average opponent's opportunity.
     * The value returned by the RiskLoss function is used as a deterrent for raising too high.
     */
    virtual float64 RiskLoss(const ChipPositionState & cps, float64 opponents, float64 raiseTo, CallCumulationD * useMean, float64 * out_dPot = 0) const;
    virtual float64 PushGain();

    virtual uint8 OppRaiseOpportunities(int8 oppID) const;

    virtual float64 oppBet() const;
    virtual float64 alreadyBet() const;
    virtual float64 callBet() const;
    virtual float64 minCallFraction(const float64 betSize);
    virtual float64 chipDenom() const;
    virtual float64 allChips() const;
    virtual float64 maxBet() const;
	virtual float64 maxBetAtTable() const;
	virtual float64 maxRaiseAmount() const;

    virtual playernumber_t handsToOutplay() const; // Number of opponents that started this round with you
    virtual playernumber_t handStrengthOfRound() const; // Same units as handsToOutplay, handsToShowdown, handsToBeat() -- it's the number of opponents, not the number in the hand
    virtual playernumber_t handsToShowdownAgainst() const; // Number of opponents who haven't folded yet


    virtual playernumber_t handsDealt() const;
    virtual playernumber_t handsIn() const;
    virtual float64 prevpotChips() const;
    virtual float64 stagnantPot() const; // All the folded chips from this round, and all the chips from previous rounds. These chips are always avalable to you and will not go away for sure.

    float64 betFraction(const float64 betSize) const;
    static float64 betFraction(const Player &, const float64 betSize);

    static float64 handBetBase(); //The B (bankroll) in calculations
	virtual float64 minRaiseTo() const;
	virtual bool inBlinds() const;

    const Player * ViewPlayer() const;


        #ifdef DEBUG_EXFDEXF
            void breakdown(float64 dist, std::ostream& target)
            {



                target << "vodd,exf.pct,dexf" << std::endl;
                for( float64 i=0;i<=1;i+=dist)
                {

                    target << i << "," << e->pctWillCall(i) << "," << e->pctWillCallD(i) << std::endl;

                }

            }
        #endif

}
;


// Assume the bet sequence is:
// x[0]: (hypotheticalMyRaise - actualBetToCall) * exp(0.0)   <-- not included in reraisedByFinal because it's already included in hypotheticalMyRaise
// x[1]: (hypotheticalMyRaise - actualBetToCall) * exp(1.0/k) <-- this happens this round, and then assume we call.
// x[2]: (hypotheticalMyRaise - actualBetToCall) * exp(2.0/k) <-- this raise happens in the next round and again assume we call.
// x[spreadRaisesOverThisManyBettingRounds]: ...
class RaiseRatio : public ScalarFunctionModel {
    const float64 fA;
    const float64 fS;
    const int8 fK;
public:

    RaiseRatio(float64 quantum, float64 firstBetBy, float64 totalBetBy, int8 rounds);

    float64 f(const float64 r) override final ;

    float64 fd(const float64 r, const float64 dummy) override final;

    float64 FindRatio();
}
;

struct FoldResponse {
    float64 gain; // 1.0 is "no change", 0.0 is "lose all money"
    float64 n; // number of hands we can fold if it's profitable to fold
};

/**
 * For comparing whether we should fold or call, provide an offset that would set callGain to zero if it were just as valuable as folding.
 */
class FoldOrCall {
private:

     //float64 foldGain(CallCumulationD* const e);
     //float64 foldGain(CallCumulationD* const e, float64 * const foldWaitLength_out);
     //float64 foldGain(CallCumulationD* const e, const float64 extra, const float64 facedBet);

    /**
     * foldGain()
     *
     *  Parameters:
     *    extra:
     *      How much more is it to call the current bet to call?
     *      i.e. currentBetToCall - currentAlreadyBet
     *      We know we're a bot, so we calculate your gain assuming that if you call now, you'll call every time this exact situation arises.
     *      `extra` is assumed to be lost on top of your current commitment every time you fold (while waiting for a better hand).
     *    facedBet:
     *      How much will you win if you have the better hand?
     *
     *
     *  Return Value:
     *    If the returned value is greater than callGain, you should fold instead of call.
     *    If the returned value is less than callGain, you should at least call.
     *
     *   This extends to bets (raises) as well.
     */
     float64 foldGain(MeanOrRank meanOrRank, const float64 extra, const float64 facedBet, float64 * const foldWaitLength_out);

    const HoldemArena & fTable;
    const Player & fPlayer;
    struct CoreProbabilities & fCore;
public:
    /**
     *
     *  Parameters:
     *    table:
     *      A view of the current table.
     *    p:
     *      The player deciding whether to fold.
     *    core:
     *      The odds of winning (now) of player p, as well as the rarity of player p's current hand.
     */
    FoldOrCall(const HoldemArena & table, struct CoreProbabilities & core)
    :
    fTable(table)
    ,
    fPlayer(*(table.ViewPlayer(core.playerID)))
    ,
    fCore(core)
    {}

    int8 getPlayerId() const {
        return fCore.playerID;
    }

    MeanOrRank suggestMeanOrRank() const;
    float64 predictedRaiseToThisRound(float64 actualBetToCall, float64 hypotheticalMyRaiseTo, float64 predictedRaiseTo) const;

    struct FoldResponse myFoldGainAgainstPredictedReraise(MeanOrRank meanOrRank, float64 currentAlreadyBet, float64 actualBetToCall, float64 hypotheticalMyRaise, float64 predictedReraiseToFinal);
    float64 myFoldGain(MeanOrRank meanOrRank);
    std::pair<float64,float64> myFoldGainAndWaitlength(MeanOrRank meanOrRank);

}
;



#endif // HOLDEM_ArenaSituations
