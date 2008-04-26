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
#ifdef DEBUGWATCHPARMS
    const float64 a = alreadyBet();
    const float64 r = prevpotChips();
    const float64 hc = table->ViewPlayer(playerID)->GetContribution();
#endif
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


    const float64 bigBlind = table->GetBigBlind() ;
    const float64 smallBlind = table->GetSmallBlind() ;
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
    FG.waitLength.amountSacrifice = table->ViewPlayer(playerID)->GetBetSize()
    #ifdef SACRIFICE_COMMITTED
                 + table->ViewPlayer(playerID)->GetContribution()
    #endif
                                    + potCommitted + extra + avgBlinds;
    FG.waitLength.opponents = playerCount - 1;

    const float64 totalFG = 1 + betFraction(  FG.f(facedBet)  );

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

bool ExpectedCallD::OppCanRaiseMe(int8 oppID) const
{
    return table->CanRaise(oppID, playerID);
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

float64 ExpectedCallD::allChips() const
{
    return table->GetAllChips();
}

float64 ExpectedCallD::chipDenom() const
{
    return table->GetChipDenom();
}

int8 ExpectedCallD::handsToBeat() const
{
    return table->NumberAtRound()-1;  //Number of hands drawn
}

int8 ExpectedCallD::handsDealt() const
{
    return table->NumberAtTable();  //Number of live players
}

int8 ExpectedCallD::handsIn() const //In general, used for "who can you split with" type requests as handsIn()-1?
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



#ifdef DEBUGBETMODEL
void DebugArena::updatePot()
{
    highBet = 0;
    float64 livePot=0;
    if( livePlayers == 0 )
    {
        HoldemArena::myPot = 0;
    }else
    {
        int8 i = 0;
        ///Assume all players are live players
        do{
            float64 tbet = p[i]->GetBetSize();
            livePot += tbet;
            if( tbet > highBet ) highBet = tbet;
            incrIndex(i);
        }while(i > 0);
        HoldemArena::myPot = deadPot + livePot;
    }
}



const float64 DebugArena::PeekCallBet()
{
    return highBet;
}

void DebugArena::InitGame()
{
    playersInHand = livePlayers;
}

void DebugArena::SetBlindPot(float64 amount)
{
    forcedBetSum = amount;
}

void DebugArena::SetDeadPot(float64 amount)
{
    deadPot = amount;
    prevRoundPot = amount;
    updatePot();
}

void DebugArena::SetBet(int8 playerNum, float64 amount)
{
    p[playerNum]->myBetSize = amount;
    updatePot();
}

void DebugArena::GiveCards(int8 playerNum, CommunityPlus h)
{
    p[playerNum]->myHand.SetUnique(h);
}

void DebugArena::SetCommunity(const CommunityPlus h, const int8 cardnum)
{
    community = h;
    cardsInCommunity = cardnum;
}

void DebugArena::AssignHandNum(uint32 n)
{
    handnum = n;
}

#endif


