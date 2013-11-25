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

float64 FoldOrCall::foldGain(MeanOrRank meanOrRank, const float64 extra, const float64 facedBet, float64 * const foldWaitLength_out)
{
    const Player &p = fPlayer;

    // If we fold now and expect to win in the future we'd have to beat all the players who would have gotten us into this situation.
    // When we come back, you'll still have to beat everyone who got an opportunity to see a hand this time.
    const float64 playerCount = fTable.NumberStartedRound().inclAllIn();

    const float64 avgBlinds = fTable.GetAvgBlindPerHand();
    FoldGainModel FG(fTable.GetChipDenom()/2);

    // CoreProbabilities & myOdds;
    // If using RANK for payout simulation:
    //   w = rank of current hand <-- from myOdds
    // If using meanConv for payout simulation:
    //   w = MEAN_winpct of current hand <-- from myOdds
    switch (meanOrRank) {
        case MEAN:
            // Since ExactCallD::ed() returns fCore.callcumu
            FG.waitLength.meanConv = &(fCore.callcumu); //  &(fCore.____); // Which *e is this usually called with?
            // One vote for: ea.ed from BluffGainInc's oppRaisedMyFoldGain
            // One vote for: ea.ed from BluffGainInc's "y -= myFoldGain"

            FG.waitLength.setW( fCore.statmean.pct );// meanW; // When called with e, what is the winPct -- how do we get that from fCore?
            // One vote for: core.statmean.pct from statProbability constructor of ExpectedCallD
            break;
        case RANK:
            FG.waitLength.meanConv = 0;
            FG.waitLength.setW( fCore.statRanking().pct ); //fCore.callcumu.Pr_haveWinPCT_orbetter(fCore.statmean.pct); // rankW; // When called with e is 0, what is the rank -- how do we get that from fCore?
            // One vote for: const float64 rarity3 = core.callcumu.Pr_haveWinPCT_orbetter(core.statmean.pct); from StatResultProbabilities::Process_FoldCallMean

            break;
    }


    FG.waitLength.bankroll = p.GetMoney();
    FG.waitLength.amountSacrificeForced = avgBlinds;
    FG.waitLength.setAmountSacrificeVoluntary( p.GetBetSize()
#ifdef SACRIFICE_COMMITTED
                 + p.GetVoluntaryContribution()
    #endif
                                    + extra
        - avgBlinds
    );
	FG.waitLength.opponents = playerCount - 1;
    FG.waitLength.prevPot = fTable.GetPrevPotSize();

	const float64 totalFG = 1 + ExpectedCallD::betFraction(p,  FG.f((facedBet > FG.waitLength.bankroll) ? (FG.waitLength.bankroll) : facedBet)  );

    if( totalFG < 0 )
    {
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


float64 ExpectedCallD::handBetBase() const
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

// DEPRECATED: Use OpponentHandOpportunity and CombinedStatResultsPessemistic instead.
// TODO: Let's say riskLoss is a table metric. In that case, if you use mean it's callcumu always.
float64 ExpectedCallD::RiskLoss(float64 rpAlreadyBet, float64 bankroll, float64 opponents, float64 raiseTo,  CallCumulationD * useMean, float64 * out_dPot) const
{
    const int8 N = handsDealt(); // This is the number of people they would have to beat in order to ultimately come back and win the hand on the time they choose to catch you.
                                 // handsDealt() is appropriate here because it suggests how often they'd have the winning hand in the first place.
    
    const float64 avgBlind = table->GetBlindValues().OpportunityPerHand(N);

    FoldGainModel FG(table->GetChipDenom()/2);
    FG.waitLength.meanConv = useMean;

    if(useMean == 0)
    {
        FG.waitLength.setW( 1.0 - 1.0/N );
    }else
    {
        FG.waitLength.setW( useMean->nearest_winPCT_given_rank(1.0 - 1.0/N) );
    }
	FG.waitLength.amountSacrificeForced = avgBlind;
    FG.waitLength.setAmountSacrificeVoluntary( (table->GetPotSize() - stagnantPot() - rpAlreadyBet)/(handsIn()-1) );

    FG.waitLength.bankroll = (allChips() - bankroll)/(N-1);
    FG.waitLength.opponents = 1;
    FG.waitLength.prevPot = table->GetPrevPotSize();
    // We don't need FG.waitLength.betSize because FG.f() will set betSize;

    //FG.dw_dbet = 0; //Again, we don't need this
	float64 riskLoss = FG.f( raiseTo ) + FG.waitLength.amountSacrificeVoluntary + FG.waitLength.amountSacrificeForced;
	float64 drisk;

	if( riskLoss < 0 )
	{//If riskLoss < 0, then expect the opponent to reraise you, since facing it will hurt you
		drisk = FG.dF_dAmountSacrifice( raiseTo ) / (handsIn()-1) + 1 / (handsIn()-1);
	}else
    {//If riskLoss > 0, then the opponent loses by raising, and therefore doesn't.
		riskLoss = 0;
		drisk = 0;
	}

	if(out_dPot != 0)
	{
		*out_dPot = drisk;
	}

	return riskLoss;
}

float64 FoldOrCall::myFoldGainAgainstPredictedRaise(MeanOrRank meanOrRank, float64 currentBetToCall, float64 currentAlreadyBet, float64 predictedRaiseTo) {
    return foldGain(meanOrRank, currentBetToCall - currentAlreadyBet, predictedRaiseTo, (float64*) 0);
}

float64 FoldOrCall::myFoldGain(MeanOrRank meanOrRank) {
    return foldGain(meanOrRank, 0, fTable.GetBetToCall(), (float64*)0);
}

std::pair<float64,float64> FoldOrCall::myFoldGainAndWaitlength(MeanOrRank meanOrRank) {
    std::pair<float64,float64> result;
    result.first = foldGain(meanOrRank, 0, fTable.GetBetToCall(), &(result.second));
    return result;
}







