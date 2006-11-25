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
#include <math.h>

//#define DEBUGWATCHPARMS
#ifdef DEBUGWATCHPARAMS
#include <iostream>
#endif

const float64 ExactCallD::UNITIALIZED_QUERY = -1;

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
    return ( alreadyBet() + prevpotChips() - table->ViewPlayer(playerID)->GetContribution() );
}

float64 ExpectedCallD::foldGain() const
{

        #ifdef DEBUGWATCHPARMS
            const float64 a = 1 - betFraction( table->ViewPlayer(playerID)->GetBetSize() );
        #endif
    return 1 - betFraction( table->ViewPlayer(playerID)->GetBetSize() );
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

float64 ExpectedCallD::prevpotChips() const
{
    return (  table->GetPrevPotSize()  );
}


float64 ExpectedCallD::betFraction(const float64 betSize) const
{
    return (  betSize / table->ViewPlayer(playerID)->GetMoney()  );
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


void ExactCallD::SetImpliedFactor(const float64 bonus)
{
    impliedFactor = bonus;
}

void ExactCallD::query(const float64 betSize)
{
	const float64 significance = 1/static_cast<double>( handsDealt()-1 );
    const float64 myexf = betSize;
    const float64 mydexf = 1;

    totalexf = table->GetPotSize() - table->ViewPlayer(playerID)->GetBetSize()  +  myexf;
    //float64 lastexf = totalexf;

    totaldexf = mydexf;
    //float64 lastdexf = totaldexf;


    int8 pIndex = playerID;
    table->incrIndex(pIndex);
    while( pIndex != playerID )
    {

        if( table->CanStillBet(pIndex) )
        {///Predict how much the bet will be
            float64 oppBankRoll = table->ViewPlayer(pIndex)->GetMoney();
            float64 oppBetAlready = table->ViewPlayer(pIndex)->GetBetSize();
            float64 nextexf;
            float64 nextdexf;


            if( betSize < oppBankRoll )
            {

                float64 oppBetMake = betSize - oppBetAlready;
                //To understand the above, consider that totalexf includes already made bets

                if( oppBetMake == 0 )
                {
                    nextexf = 0;
                    nextdexf = 1;
                }else
                {
                        #ifdef DEBUGWATCHPARMS
                            const float64 vodd = pow(  oppBetMake / (oppBetMake + totalexf), significance);
                            const float64 willCall = e->pctWillCall( pow(  oppBetMake / (oppBetMake + totalexf)  , significance  ) );
                            const float64 willCallD = e->pctWillCallD(   pow(  oppBetMake / (oppBetMake + totalexf)  , significance  )  );

/*
                            std::cout << willCall << " ... " << willCallD << std::endl;
                            std::cout << "significance " << significance << std::endl;
                            std::cout << "(oppBetMake + totalexf) " << (oppBetMake + totalexf) << std::endl;
                            std::cout << "(oppBetMake ) " << (oppBetMake ) << std::endl;
                            std::cout << "(betSize) " << (oppBetMake) << std::endl;
                            std::cout << "(oppBetAlready) " << (oppBetAlready) << std::endl;
*/
                        #endif
                    nextexf = e->pctWillCall( pow(  oppBetMake / (oppBetMake + totalexf)  , significance  ) );
                    nextdexf = nextexf + oppBetMake * e->pctWillCallD(   pow(  oppBetMake / (oppBetMake + totalexf)  , significance  )  )
													*  pow(  oppBetMake / (oppBetMake + totalexf)  , significance - 1 ) * significance
                                                    * (totalexf - oppBetMake * totaldexf)
                                                     /(oppBetMake + totalexf) /(oppBetMake + totalexf);
                    nextexf *= oppBetMake;

                }
            }else
            {///Opponent would be all-in to call this bet
                float64 oldpot = table->GetPrevPotSize();
                float64 effroundpot = (totalexf - oldpot) * oppBankRoll / betSize;
                float64 oppBetMake = oppBankRoll - oppBetAlready;
                nextexf = oppBetMake * e->pctWillCall( pow(oppBetMake / (oppBetMake + oldpot + effroundpot),significance) );

                nextdexf = 0;
            }
            //lastexf = nextexf;
            totalexf += nextexf;

            //lastdexf = nextdexf;
            totaldexf += nextdexf;
        }

        table->incrIndex(pIndex);
    }

    totalexf = totalexf - myexf;
    totaldexf = totaldexf - mydexf;


}

float64 ExactCallD::exf(const float64 betSize)
{
    //if( queryinput == UNINITIALIZED_QUERY )
    if( queryinput != betSize )
    {
        query(betSize);
        queryinput = betSize;
    }
    return totalexf*impliedFactor;
}

float64 ExactCallD::dexf(const float64 betSize)
{
//    if( query == UNINITIALIZED_QUERY )
    if( queryinput != betSize )
    {
        query(betSize);
        queryinput = betSize;
    }
    return totaldexf*impliedFactor;
}




float64 ZeroCallD::exf(float64 betSize)
{///Only money already put into the pot.
//Recall that GetPotSize == GetDeadPotSize + GetRoundPotSize
    return table->GetPotSize();
}


float64 ZeroCallD::dexf(float64 betSize)
{///Expect no-one to call your bet except people who are better than you anyways
    return 0;
}

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


#ifdef DEBUGBETMODEL

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
    blindBetSum = amount;
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


