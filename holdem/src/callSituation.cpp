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

float64 ExpectedCallD::foldGain(const float64 extra) const
{


#ifdef DEBUGWATCHPARMS
    const float64 a = 1 - betFraction( table->ViewPlayer(playerID)->GetBetSize() );
#endif

    const float64 baseFraction = betFraction( table->ViewPlayer(playerID)->GetBetSize() + potCommitted + extra);
#ifdef ANTI_PRESSURE_FOLDGAIN
///If extra > 0, you are making them raise you, which allows you to wait out a good spot?
    const float64 handFreq = /*(extra > 0) ? (2-handRarity) : */1/handRarity;
    if( handRarity <= 0 )
    {
        return 0;
    }
#else
    const float64 handFreq = 1;
#endif

#ifdef BLIND_ADJUSTED_FOLD
    //const float64 blindPerHandGain = ( ViewTable().GetBigBlind()+ViewTable().GetSmallBlind() ) / myMoney / ViewTable().GetNumberAtTable();
    const float64 bigBlindFraction = betFraction( table->GetBigBlind() );
    const float64 smallBlindFraction = betFraction( table->GetSmallBlind() );
#ifdef SAME_WILL_LOSE_BLIND
    const float64 blindsPow = 1.0 / table->GetNumberAtTable();
#else
    const float64 rawLoseFreq = 1 - (2.0 / table->GetNumberAtTable()) ;
    const float64 blindsPow = rawLoseFreq / table->GetNumberAtTable();
#endif

#ifdef ANTI_PRESSURE_FOLDGAIN
    #ifdef PURE_BLUFF
        const float64 totalFG = (1 - (baseFraction+(bigBlindFraction+smallBlindFraction)*blindsPow)*handFreq);
    #else
        const float64 totalFG = (1 - baseFraction - ((bigBlindFraction+smallBlindFraction)*blindsPow)*handFreq);
    #endif
#else
    if( 1 < baseFraction + bigBlindFraction*handFreq )
    {
        return 0;
    }else
    {
        const float64 blindsGain = (1 - baseFraction - bigBlindFraction*handFreq)*(1 - baseFraction - smallBlindFraction*handFreq);

        const float64 totalFG = pow(1-baseFraction,1-2*blindsPow)*pow(blindsGain,blindsPow);
    }
#endif
    if( totalFG < 0 ) return 0;
    return totalFG;

#else

    return 1 - baseFraction;
#endif
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

float64 ExpectedCallD::allChips() const
{
    return table->GetAllChips();
}

float64 ExpectedCallD::chipDenom() const
{
    return table->GetChipDenom();
}


int8 ExpectedCallD::handsDealt() const
{
    return table->GetNumberAtTable();  //Number of live players
}

int8 ExpectedCallD::handsIn() const
{
    return table->GetNumberInHand();  //Number of live players not folded
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

#ifdef ASSUMEFOLDS
void ExpectedCallD::callingPlayers(float64 n)
{
    eFold = n;
}

float64 ExpectedCallD::callingPlayers() const
{
    return eFold;
}
#endif

/*
float64 EstimateCallD::exf(float64 betSize)
{
    const float64& x = betFraction(betSize);
    return table->GetNumberInHand() * e->pctWillCall(  x/(2*x+potFraction() )  );
}


float64 EstimateCallD::dexf(float64 betSize)
{
    const float64& f_pot = potFraction();
    const float64& x = betFraction(betSize);
    return table->GetNumberInHand() * e->pctWillCallD(  x/(2*x+f_pot)  ) * f_pot / (2*x+f_pot) /(2*x+f_pot);
}
*/



float64 ZeroCallD::exf(float64 betSize)
{///Only money already put into the pot.
//Recall that GetPotSize == GetDeadPotSize + GetRoundPotSize
    return table->GetPotSize();
}


float64 ZeroCallD::dexf(float64 betSize)
{///Expect no-one to call your bet except people who are better than you anyways
    return 0;
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


