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

#include "callSituation.h"
#include "inferentials.h"
#include "math_support.h"



ExpectedCallD::~ExpectedCallD()
{
}

/*
float64 FoldOrCall::foldGain(CallCumulationD* const e)
{
    return foldGain(e,0);
}


float64 FoldOrCall::foldGain(CallCumulationD* const e, float64 * const foldWaitLength_out)
{
    return foldGain(e,0,callBet(),foldWaitLength_out);
}

float64 FoldOrCall::foldGain(CallCumulationD* const e, const float64 extra, const float64 facedBet)
{
    return foldGain(e,extra, facedBet, 0);
}
*/


static playercounts_t suggestPlayerCount(const HoldemArena & table) {
    return table.NumberStartedRound();
}



float64 FoldOrCall::foldGain(MeanOrRank meanOrRank, const float64 extra, const float64 facedBet, float64 * const foldWaitLength_out) const
{
    const Player &p = fPlayer;

    // If we fold now and expect to win in the future we'd have to beat all the players who would have gotten us into this situation.
    // When we come back, you'll still have to beat everyone who got an opportunity to see a hand this time.
    const float64 playerCount = suggestPlayerCount(fTable).inclAllIn();

    const float64 avgBlind = fTable.GetBlindValues().OpportunityPerHand(fTable.NumberAtTable());
    FoldGainModel<PlayerStrategyPerspective, OppositionPerspective> FG(fTable.GetChipDenom()/2);

    // CoreProbabilities & myOdds;
    // If using RANK for payout simulation:
    //   w = rank of current hand <-- from myOdds
    // If using meanConv for payout simulation:
    //   w = MEAN_winpct of current hand <-- from myOdds
    switch (meanOrRank) {
        case MEAN:
            // Since ExactCallD::ed() returns fCore.callcumu
            FG.waitLength.setMeanConv(  &(fCore.callcumu)  ); //  &(fCore.____); // Which *e is this usually called with?
            // One vote for: ea.ed from BluffGainInc's oppRaisedMyFoldGain
            // One vote for: ea.ed from BluffGainInc's "y -= myFoldGain"

            FG.waitLength.setW( fCore.statmean.pct );// meanW; // When called with e, what is the winPct -- how do we get that from fCore?
            // One vote for: core.statmean.pct from statProbability constructor of ExpectedCallD
            break;
        case RANK:
            FG.waitLength.setMeanConv( EMPTY_DISTRIBUTION );
            FG.waitLength.setW( fCore.statRanking().pct ); //fCore.callcumu.Pr_haveWinPCT_orbetter(fCore.statmean.pct); // rankW; // When called with e is 0, what is the rank -- how do we get that from fCore?
            // One vote for: const float64 rarity3 = core.callcumu.Pr_haveWinPCT_orbetter(core.statmean.pct); from StatResultProbabilities::Process_FoldCallMean

            break;
    }


    FG.waitLength.bankroll = p.GetMoney();
    FG.waitLength.amountSacrificeForced = avgBlind;
    FG.waitLength.setAmountSacrificeVoluntary( p.GetBetSize()
#ifdef SACRIFICE_COMMITTED
                 + p.GetVoluntaryContribution()
    #endif
                                    + extra
        - avgBlind
    );
	FG.waitLength.opponents = playerCount - 1;
    FG.waitLength.prevPot = fTable.GetPrevPotSize();

	const float64 totalFG = 1 + ExpectedCallD::betFraction(p,  FG.f((facedBet > FG.waitLength.bankroll) ? (FG.waitLength.bankroll) : facedBet)  );

    if( totalFG < 0 )
    { // You can't lose more than ALL your money
        if( foldWaitLength_out != 0 ) *foldWaitLength_out = 0;
         return 0;
    }else
    {
        if( foldWaitLength_out != 0 ) *foldWaitLength_out = FG.n;
        return totalFG;
    }

}

uint8 ExpectedCallD::OppRaiseOpportunities(int8 oppID) const
{
    return table->RaiseOpportunities(oppID, playerID);
}

float64 ExpectedCallD::oppBet() const
{
    return table->GetRoundBetsTotal();
}

float64 ExpectedCallD::alreadyBet() const
{
    return table->ViewPlayer(playerID)->GetBetSize();
}

float64 ExpectedCallD::callBet() const
{
    return table->GetBetToCall();
}

float64 ExpectedCallD::stagnantPot() const
{
    const float64 roundFolds = table->GetFoldedPotSize() - table->GetPrevFoldedRetroactive();
    return (roundFolds + prevpotChips());
}


float64 ExpectedCallD::minCallFraction(const float64 betSize)
{
    const float64 maxShowdown = table->GetMaxShowdown();
    //Most of the time, (betSize < maxShowdown), so minCall is betSize;
    //Obviously you can't have someone call less than betSize unless everybody else folds.
    const float64 minCall = (betSize < maxShowdown) ? betSize : maxShowdown;
    return betFraction(minCall + stagnantPot());
}


float64 ExpectedCallD::maxBet() const
{
    return table->ViewPlayer(playerID)->GetMoney();
}

float64 ExpectedCallD::maxBetAtTable() const
{
	return table->GetMaxShowdown();
}

float64 ExpectedCallD::maxRaiseAmount() const
{
	const float64 maxShowdown = table->GetMaxShowdown();
	const float64 maxCall = maxBet();
	return ((maxCall < maxShowdown) ? (maxCall) : (maxShowdown) );
}

float64 ExpectedCallD::allChips() const
{
    return table->GetAllChips();
}

float64 ExpectedCallD::chipDenom() const
{
    return table->GetChipDenom();
}

// This is the "established" hand strength requirement of anyone willing to claim they will win this hand.
playernumber_t ExpectedCallD::handStrengthOfRound() const
{   // Same units as ExpectedCallD::handsToBeat(), which is number of opponents.
    return table->NumberAtFirstActionOfRound().inclAllIn()-1;
    // Don't overcomplicate it for now. At the time of the first action the bettor knows who has folded already and
    // isn't considering them when establishing the hand strength we will be playing.
}

playernumber_t ExpectedCallD::handsToOutplay() const
{
    return table->NumberStartedRoundInclAllIn()-1;  // For now, do this to match ro-643-patch2
}

playernumber_t ExpectedCallD::handsToShowdownAgainst() const {
    return table->NumberInHandInclAllIn()-1;  //Number of hands (drawn) *remaining*
}

playernumber_t ExpectedCallD::handsDealt() const
{
    return table->NumberAtTable();  //Number of live players
}

playernumber_t ExpectedCallD::handsIn() const //In general, used for "who can you split with" type requests as handsIn()-1?
{
    return table->NumberInHandInclAllIn();  //Number of live players not folded
}

float64 ExpectedCallD::prevpotChips() const
{
    return (  table->GetPrevPotSize()  );
}

float64 ExpectedCallD::betFraction(const float64 betSize) const
{
    return betFraction(*(ViewPlayer()), betSize);
}

float64 ExpectedCallD::betFraction(const Player & player, const float64 betSize)
{
    return (
            betSize
            /
            ( player.GetMoney() )
            );
}


float64 ExpectedCallD::handBetBase()
{
    return 1;
}

float64 ExpectedCallD::minRaiseTo() const
{
    return table->GetMinRaise() + callBet();
}


bool ExpectedCallD::inBlinds() const
{
    return ( table->GetUnbetBlindsTotal() > 0 );
}

const Player * ExpectedCallD::ViewPlayer() const {
    return table->ViewPlayer(playerID);
}

// DEPRECATED: Use OpponentHandOpportunity from CombinedStatResultsPessemistic instead.
// TODO: Let's say riskLoss is a table metric. In that case, if you use mean it's callcumu always.
// At the time of this writing...
//  + src/stratFear.h:ScalarPWinFunction uses ExactCallBluffD but it's inconclusive.
//  + src/stratPosition.h has `DeterredGainStrategy` and `ImproveGainStrategy` which both have varying levels of ExactCallBluffD
//  + src/stratPosition.cpp also creates StateModel objects in all three of ImproveGainStrategy::MakeBet & DeterredGainStrategy::MakeBet & PureGainStrategy::MakeBet
//                          so do they all eventually call into RiskLoss?
ValueAndSlope ExpectedCallD::RiskLoss(const struct HypotheticalBet & hypotheticalRaise, CommunityStatsCdf * foldwait_length_distr) const
{
    const float64 raiseTo = hypotheticalRaise.hypotheticalRaiseTo;
    const int8 N = handsDealt(); // This is the number of people they would have to beat in order to ultimately come back and win the hand on the time they choose to catch you.
                                 // handsDealt() is appropriate here because it suggests how often they'd have the winning hand in the first place.

    const float64 avgBlind = table->GetBlindValues().OpportunityPerHand(N);

    FoldGainModel<PlayerStrategyPerspective, OppositionPerspective> FG(table->GetChipDenom()/2);
    // [!TIP]
    // FoldGainModel needs _whose_ perspective?
    //  * When used by StateModel::query (via `FoldOrCall::foldGain` method) it uses callcumu to answer "should I fold?"
    //  * When used by FacedOddsRaiseGeom/FacedOddsAlgb to answer "Will my opponent call?"/"Will my opponent Raise?" ...it would use foldcumu if your opponent knows your hand, and callcumu if your opponent doesn't know your hand
    //  * When used by RiskLoss to answer "Will my opponent *raise*?" (depending on whether they think _I_ will fold to their raise), we go back to callcumu.
    // and that's why we accept `CommunityStatsCdf` here and not just any `CallCumulationD`
    FG.waitLength.setMeanConv( foldwait_length_distr );

    if(foldwait_length_distr == 0)
    {
        FG.waitLength.setW( 1.0 - 1.0/N ); // i.e. rarity() will be 1.0/N
    }else
    {
        FG.waitLength.setW( foldwait_length_distr->nearest_winPCT_given_rank(1.0 - 1.0/N) );
    }
    FG.waitLength.amountSacrificeForced = avgBlind;

    // This is a weird "generic player" who represents all the other players combined
    // It has, as a "bankroll" that splits the entire rest of the chip stack equally
    // It has, as a "sacrifice" the average amount that everyone else has bet
    //        ↑ it's been that way since https://github.com/yuzisee/pokeroo/blob/6b1eaf1bbaf9e4a9c41476c1200965d32e25fcb7/holdem/src/callPrediction.cpp#L567
    FG.waitLength.setAmountSacrificeVoluntary( (table->GetPotSize() - stagnantPot() - hypotheticalRaise.bettorSituation.alreadyBet)/(handsIn()-1) );
    // ^^^ If amountSacrificeVoluntary were for a single player it would be: that player's current bet size − that player's forced bet (e.g. blinds)

    FG.waitLength.bankroll = (allChips() - hypotheticalRaise.bettorSituation.bankroll)/(N-1);
    FG.waitLength.opponents = 1;

    FG.waitLength.prevPot = table->GetPrevPotSize();
    // We don't need FG.waitLength.betSize because FG.f() will set betSize;

    //FG.dw_dbet = 0; //Again, we don't need this
    // NOMINALLY FG.f( raiseTo ) is to lose `FG.waitLength.amountSacrificeVoluntary + FG.waitLength.amountSacrificeForced` because all else being equal, if you fold you lose the money you put in.
    const float64 nominalFoldChips = -FG.waitLength.amountSacrificeVoluntary - FG.waitLength.amountSacrificeForced;
    const float64 trueFoldChipsEV = FG.f( raiseTo );
    // ^^^ Given the hand strength, how much would someone gain by folding against a bet of `raiseTo`?

    // INVARIANT: If `FG.f( raiseTo )` (i.e. "FoldGain") is positive, it means it is profitable to fold against `raiseTo`

    const float64 riskLoss =
			#ifdef OLD_BROKEN_RISKLOSS_WRONG_SIGN
			(trueFoldChipsEV < nominalFoldChips) ? ( trueFoldChipsEV - nominalFoldChips
      // (nominalFoldChips + std::numeric_limits<float64>::epsilon() < trueFoldChipsEV) ? ( trueFoldChipsEV - nominalFoldChips
			#else
			(std::numeric_limits<float64>::epsilon() < trueFoldChipsEV) ? ( -trueFoldChipsEV
			  // If trueFoldChipsEV is *strictly profitable*, then the player who made `faced_bet` could "win" by folding, meaning it's overly risky for this person (doing HypotheticalBet right now) to raise as high as `hypotheticalRaise.hypotheticalRaiseTo`
        // As such, we need to penalize this `hypotheticalRaise.hypotheticalRaiseTo` by returning a riskLoss quantity that represents this surplus
      #endif
			)
			:
			(
			  0.0
			)
		;

		const float64 dRiskLoss =
		  #ifdef OLD_BROKEN_RISKLOSS_WRONG_SIGN
				(nominalFoldChips < trueFoldChipsEV) ? (
				  (FG.dF_dAmountSacrifice( raiseTo ) / (handsIn()-1) + 1.0 / static_cast<float64>(handsIn()-1))
			#else
      (nominalFoldChips + std::numeric_limits<float64>::epsilon() < trueFoldChipsEV) ? (
       // If trueFoldChipsEV offers any benefit at all, then the player who made `faced_bet` could benefit more by folding, meaning it's not productive this opponent (the person doing HypotheticalBet right now) to raise as high as `hypotheticalRaise.hypotheticalRaiseTo`
       // As such, we need to penalize this `hypotheticalRaise.hypotheticalRaiseTo` by returning a riskLoss quantity that represents this surplus

       // https://github.com/yuzisee/pokeroo/commit/6b1eaf1bbaf9e4a9c41476c1200965d32e25fcb7
         // d_riskLoss/d_pot = d/dpot { FG.f( raiseTo ) }                           + d/dpot { FG.waitLength.amountSacrifice }
         //                                                                             ^^^ see `setAmountSacrificeVoluntary`
         //   d_pot/d_AmountSacrifice { FG.f( raiseTo ) } * d_AmountSacrifice/d_pot + d/dpot { FG.waitLength.amountSacrifice }
         -(FG.dF_dAmountSacrifice( raiseTo ) / (handsIn()-1) + 1.0 / static_cast<float64>(handsIn()-1))
         #endif
         // In this case, `riskLoss.D_v` needs to be ∂{riskLoss.v}/∂pot
         // TODO(from joseph): Do we need a unit test for this? (Is it still used considering it has been deprecated?)
      )
      :
      (
        0.0
      )
    ;

	return (ValueAndSlope { riskLoss, dRiskLoss });

}

MeanOrRank FoldOrCall::suggestMeanOrRank() const {
    if (suggestPlayerCount(fTable).inclAllIn() > 2) {
    // The odds of beating multiple people isn't based on the odds of beating one person.
    // Since it's more complicated than that, just go with rank for now.
        return RANK;
    } else {
        return MEAN;
    }
}

    RaiseRatio::RaiseRatio(float64 quantum, float64 firstBetBy, float64 totalBetBy, int8 rounds)
    : ScalarFunctionModel(quantum),

    fA(firstBetBy)
    ,
    fS(totalBetBy)
    ,
    fK(rounds)
    {}


    // [!TIP]
    // This is just a simple sum of a finite geometric series.
    float64 RaiseRatio::f(const float64 r)  {
        float64 expected = fS; // We would like to reach this target amount at the end
        float64 actual = 0.0;
        float64 next = fA;
        for (int i=1;i<=fK;++i) {
            next *= r;
            actual += next;

        }
        return actual - expected;
    }
    float64 RaiseRatio::fd(const float64 r, const float64 dummy)  {
        float64 dActual = 0.0;
        float64 next = fA;
        for (int i=1;i<=fK;++i) {
            // actual += fA*r^i
            // dActual += fA * i * r^(i-1)
            dActual += i * next;
            next *= r;
        }
        return dActual;
    }

float64 RaiseRatio::FindRatio() {

    const float64 reraisedByFinalRatio = fS / fA;
    // If it's the final betting round this would be the solution.
    //         ^^^ i.e. `fK == 1` ^^^
    // If fK > 1 the required ratio only gets lower.

    return FindZero(1.0, reraisedByFinalRatio, false);
}

float64 FoldOrCall::predictedRaiseToThisRound(float64 actualBetToCall, float64 hypotheticalMyRaiseTo, float64 predictedRaiseToFinal) const {
    const uint8 spreadRaisesOverThisManyBettingRounds = 1 + fTable.FutureRounds();
    const float64 reraisedByFinal = predictedRaiseToFinal - hypotheticalMyRaiseTo;
    const float64 myRaiseBy = (hypotheticalMyRaiseTo - actualBetToCall);
    const float64 minRaiseBy = fTable.GetMinRaise();

    const float64 start = (minRaiseBy < myRaiseBy) ? myRaiseBy : minRaiseBy; // choose the larger of the two

    // The quantum we want is enough to get the first bet size accurate to one chipdenom.
    const float64 quantum = fTable.GetChipDenom() / start;

    RaiseRatio raiseRatio(quantum, start, reraisedByFinal, spreadRaisesOverThisManyBettingRounds);

    const float64 r = raiseRatio.FindRatio();


    // Or, perhaps the total raise can be approximated with an integral:
    //  reraisedByFinal = {\int_a}^b  (hypotheticalMyRaise - actualBetToCall) * exp(x/k)  dx
    // To keep the units correct, let's start with the mean value and multibly by our own interval
    //  reraisedByFinal = spreadRaisesOverThisManyBettingRounds * (1.0/( b- a)) (hypotheticalMyRaise - actualBetToCall) * {\int_a }^b  exp(x/k) dx
    //  reraisedByFinal = spreadRaisesOverThisManyBettingRounds * (1.0/(xB-xA)) (hypotheticalMyRaise - actualBetToCall) * {\int_xA}^xB exp(x/k) dx
    //    f = exp(x/k)
    //    F = k * exp(x/k)
    //  reraisedByFinal = spreadRaisesOverThisManyBettingRounds * (1.0/(xB-xA)) (hypotheticalMyRaise - actualBetToCall) * (k * exp(xB/k) - k * exp(xA/k))
    //  reraisedByFinal = spreadRaisesOverThisManyBettingRounds * (1.0/(xB-xA)) (hypotheticalMyRaise - actualBetToCall) * k * (exp(xB/k) - exp(xA/k))
    //  reraisedByFinal =                                     K * (1.0/(xB-xA)) (hypotheticalMyRaise - actualBetToCall) * k * (exp((K+0.5)/k) - exp(0.5/k))
    //  reraisedByFinal =                                     K * (1.0/(K))     (hypotheticalMyRaise - actualBetToCall) * k * (exp((K+0.5)/k) - exp(0.5/k))
    //  reraisedByFinal = (hypotheticalMyRaise - actualBetToCall) * k * (exp((K+0.5)/k) - exp(0.5/k))
    //  reraisedByFinal = (hypotheticalMyRaise - actualBetToCall) * k * exp(0.5/k) * (exp((K)/k) - 1.0)
    //  reraisedByFinal = (hypotheticalMyRaise - actualBetToCall) * k * exp(0.5/k) * (exp(K/k) - 1.0)

#ifdef DEBUGASSERT
    if(std::isnan(r)) {
        std::cout << "raiseRatio(" << quantum << "," << myRaiseBy << "," << reraisedByFinal << "," << static_cast<int>(spreadRaisesOverThisManyBettingRounds) << ") NaN'ed" << std::endl;
        exit(1);
    }
    if( (r < 1.0) && (start < reraisedByFinal)) {
      std::cout << "We are outside the range? Raising would never cause the pot to SHRINK but... raiseRatio.FindRatio() returned " << r << " during FoldOrCall::predictedRaiseToThisRound" << std::endl;
      exit(1);
    }
    if ((reraisedByFinal + std::numeric_limits<float64>::epsilon()) / (start - std::numeric_limits<float64>::epsilon()) < r - std::numeric_limits<float64>::epsilon()) {
      std::cout << "We want to answer the question of \"if their *target* raise is to reach reraisedByFinal: knowing there are `spreadRaisesOverThisManyBettingRounds` remaining, how much will they raise _this round_?\"  And, yet, raiseRatio.FindRatio() returned " << r
      << " which would exceed their target of " << reraisedByFinal << " if the first raise is " << start << std::endl;
      exit(1);
    }
#endif // DEBUGASSERT

    const float64 reraisedByThisRound = start * r;
    if (predictedRaiseToFinal < reraisedByThisRound) {
        // They're re-raising all-in then?
        return predictedRaiseToFinal;
    } else {
        return reraisedByThisRound;
    }
}

FoldResponse FoldOrCall::myFoldGainAgainstPredictedReraise(MeanOrRank meanOrRank, float64 currentAlreadyBet, float64 actualBetToCall, float64 hypotheticalMyRaiseTo, float64 predictedReraiseToFinal) const {
    struct FoldResponse result;
    result.gain = foldGain(meanOrRank, hypotheticalMyRaiseTo - currentAlreadyBet
                    ,
                    // predictedReraiseToFinal
                    predictedRaiseToThisRound(actualBetToCall, hypotheticalMyRaiseTo, predictedReraiseToFinal)
                    ,
                    &(result.n)
                    );
    return result;
}

float64 FoldOrCall::myFoldGain(MeanOrRank meanOrRank) const {
    return foldGain(meanOrRank, 0, fTable.GetBetToCall(), (float64*)0);
}

std::pair<float64,float64> FoldOrCall::myFoldGainAndWaitlength(MeanOrRank meanOrRank) const {
    std::pair<float64,float64> result;
    result.first = foldGain(meanOrRank, 0, fTable.GetBetToCall(), &(result.second));
    return result;
}
