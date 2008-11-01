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

#include "callSituation.h"


ExpectedCallD::~ExpectedCallD()
{
}

float64 ExpectedCallD::forfeitChips() const
{
    return ( alreadyBet() + stagnantPot() - table->ViewPlayer(playerID)->GetContribution() );
}

float64 ExpectedCallD::foldGain(CallCumulationD* const e)
{
    return foldGain(e,0);
}

float64 ExpectedCallD::foldGain(CallCumulationD* const e, float64 * const foldWaitLength_out)
{
    return foldGain(e,0,callBet(),foldWaitLength_out);
}

float64 ExpectedCallD::foldGain(CallCumulationD* const e, const float64 extra, const float64 facedBet)
{
    return foldGain(e,extra, facedBet, 0);
}

float64 ExpectedCallD::foldGain(CallCumulationD* const e, const float64 extra, const float64 facedBet, float64 * const foldWaitLength_out)
{
    const float64 playerCount = table->NumberAtTable();



    const float64 bigBlind = table->GetBlindValues().GetBigBlind() ;
    const float64 smallBlind = table->GetBlindValues().GetSmallBlind();
#ifdef SAME_WILL_LOSE_BLIND
    const float64 blindsPow = 1.0 / (playerCount);
#else
    const float64 rawLoseFreq = 1 - (2.0 / playerCount) ;
    const float64 blindsPow = rawLoseFreq / playerCount;
#endif

    const float64 avgBlinds = (bigBlind+smallBlind)*blindsPow;
    FoldGainModel FG(table->GetChipDenom()/2);
    FG.waitLength.meanConv = e;
    FG.waitLength.w = meanW;
    FG.waitLength.bankroll = table->ViewPlayer(playerID)->GetMoney();
    FG.waitLength.amountSacrificeVoluntary = table->ViewPlayer(playerID)->GetBetSize()
    #ifdef SACRIFICE_COMMITTED
                 + table->ViewPlayer(playerID)->GetContribution()
    #endif
                                    + potCommitted + extra;
	FG.waitLength.amountSacrificeForced = avgBlinds;
    FG.waitLength.opponents = playerCount - 1;

	const float64 totalFG = 1 + betFraction(  FG.f((facedBet > FG.waitLength.bankroll) ? (FG.waitLength.bankroll) : facedBet)  );

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

playernumber_t ExpectedCallD::handsToBeat() const
{
    return table->NumberAtRound()-1;  //Number of hands drawn
}

playernumber_t ExpectedCallD::handsDealt() const
{
    return table->NumberAtTable();  //Number of live players
}

playernumber_t ExpectedCallD::handsIn() const //In general, used for "who can you split with" type requests as handsIn()-1?
{
    return table->NumberInHand();  //Number of live players not folded
}

float64 ExpectedCallD::prevpotChips() const
{
    return (  table->GetPrevPotSize()  );
}


float64 ExpectedCallD::betFraction(const float64 betSize) const
{
    return (
            betSize
            /
            ( table->ViewPlayer(playerID)->GetMoney() + potCommitted )
           );
}

float64 ExpectedCallD::handBetBase() const
{
    return 1-betFraction(potCommitted);
}

float64 ExpectedCallD::minRaiseTo() const
{
    return table->GetMinRaise() + callBet();
}


bool ExpectedCallD::inBlinds() const
{
    return ( table->GetUnbetBlindsTotal() > 0 );
}



float64 ExpectedCallD::RiskLoss(float64 rpAlreadyBet, float64 bankroll, float64 opponents, float64 raiseTo,  CallCumulationD * useMean, float64 * out_dPot) const
{

    const int8 N = handsDealt();
    const float64 avgBlind = table->GetBlindValues().OpportunityPerHand(N);

    FoldGainModel FG(table->GetChipDenom()/2);
    FG.waitLength.meanConv = useMean;

    if(useMean == 0)
    {
        FG.waitLength.w = 1.0 - 1.0/N;
    }else
    {
        FG.waitLength.w = useMean->nearest_winPCT_given_rank(1.0 - 1.0/N);
    }
    FG.waitLength.amountSacrificeVoluntary = (table->GetPotSize() - stagnantPot() - rpAlreadyBet)/(handsIn()-1);
	FG.waitLength.amountSacrificeForced = avgBlind;
    FG.waitLength.bankroll = (allChips() - bankroll)/(N-1);
    FG.waitLength.opponents = 1;
    FG.dw_dbet = 0; //Again, we don't need this
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









