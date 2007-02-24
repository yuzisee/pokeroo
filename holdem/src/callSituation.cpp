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


#define BLIND_ADJUSTED_FOLD
#define SAME_WILL_LOSE_BLIND
//#define GEOM_COMBO_FOLDPCT


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
	
#ifdef ANTI_PRESSURE_FOLDGAIN
	const float64 handFreq = 1/handRarity;
        const float64 baseFraction = betFraction( table->ViewPlayer(playerID)->GetBetSize() + potCommitted );
#else
        const float64 baseFraction = betFraction( table->ViewPlayer(playerID)->GetBetSize() + potCommitted );
#endif

#ifdef BLIND_ADJUSTED_FOLD
    //const float64 blindPerHandGain = ( ViewTable().GetBigBlind()+ViewTable().GetSmallBlind() ) / myMoney / ViewTable().GetNumberAtTable();
    const float64 bigBlindFraction = betFraction( table->GetBigBlind() );
    const float64 smallBlindFraction = betFraction( table->GetSmallBlind() );

    const float64 blindsGain = (1 - baseFraction - bigBlindFraction)*(1 - baseFraction - smallBlindFraction);
    #ifdef SAME_WILL_LOSE_BLIND
    const float64 blindsPow = 1 / table->GetNumberAtTable();
    #else
    const float64 rawLoseFreq = 1 - (1.0 / table->GetNumberAtTable()) ;
    const float64 blindsPow = rawLoseFreq / table->GetNumberAtTable();
    #endif

    const float64 totalFG = pow(1-baseFraction,1-2*blindsPow)*pow(blindsGain,blindsPow);

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
	const float64 significance = 1/static_cast<float64>( handsDealt()-1 );
    const float64 myexf = betSize;
    const float64 mydexf = 1;

	float64 overexf = 0;
	float64 overdexf = 0;

    totalexf = table->GetPotSize() - table->ViewPlayer(playerID)->GetBetSize()  +  myexf;

    //float64 lastexf = totalexf;


    totaldexf = mydexf;
    //float64 lastdexf = totaldexf;

    int8 pIndex = playerID;
    table->incrIndex(pIndex);
    while( pIndex != playerID )
    {
		float64 nextexf = 0;
        float64 nextdexf = 0;
		const float64 oppBetAlready = table->ViewPlayer(pIndex)->GetBetSize();

        if( table->CanStillBet(pIndex) )
        {///Predict how much the bet will be
            const float64 oppBankRoll = table->ViewPlayer(pIndex)->GetMoney();

            if( betSize < oppBankRoll )
            {

                const float64 oppBetMake = betSize - oppBetAlready;
                //To understand the above, consider that totalexf includes already made bets

                if( oppBetMake <= 0 )
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
                const float64 oldpot = table->GetPrevPotSize();
                const float64 effroundpot = (totalexf - oldpot) * oppBankRoll / betSize;
                const float64 oppBetMake = oppBankRoll - oppBetAlready;


				nextexf = oppBetMake * e->pctWillCall( pow(oppBetMake / (oppBetMake + oldpot + effroundpot),significance) );

				nextdexf = 0;

            }


            //lastexf = nextexf;
            totalexf += nextexf;

            //lastdexf = nextdexf;
            totaldexf += nextdexf;

        }

		const float64 oppInPot = oppBetAlready + nextexf;
		if( oppInPot > betSize )
		{
			overexf += oppInPot - betSize;
			overdexf += nextdexf;
		}

        table->incrIndex(pIndex);
    }

    totalexf = totalexf - myexf - overexf;
    totaldexf = totaldexf - mydexf - overdexf;

	if( totalexf < 0 ) totalexf = 0; //Due to rounding error in overexf?
	if( totaldexf < 0 ) totaldexf = 0; //Due to rounding error in overexf?

}

void ExactCallBluffD::query(const float64 betSize)
{
	ExactCallD::query(betSize);

	float64 countMayFold = table->GetNumberInHand() - 1 ;
//    const float64 countToBeat = table->GetNumberAtTable() - 1 ;
//    float64 countToBeat = table->GetNumberAtTable() - table->GetNumberInHand() + 1 ;
//	float64 countMayFold = 1 ;
    const float64 myexf = betSize;
    const float64 mydexf = 1;


	float64 nextFold;
	float64 nextFoldPartial;
	allFoldChance = 1;
	allFoldChanceD = 0;
	const float64 origPot = table->GetPotSize() - table->ViewPlayer(playerID)->GetBetSize()  +  myexf;
	const float64 origPotD = mydexf;


	if( betSize < minRaiseTo() - chipDenom()/4 || callBet() >= table->GetMaxShowdown() )
    {
		allFoldChance = 0;
		allFoldChanceD = 0;
		nextFold = 0;
		nextFoldPartial = 0;
    }

    int8 pIndex = playerID;
    table->incrIndex(pIndex);
    while( pIndex != playerID && allFoldChance > 0)
    {

	const float64 oppBetAlready = table->ViewPlayer(pIndex)->GetBetSize();

    const float64 significanceLinear =/* 1/countToBeat ;
	const float64 significance =*/ 1/countMayFold;

        if( table->CanStillBet(pIndex) )
        {///Predict how much the bet will be
            const float64 oppBankRoll = table->ViewPlayer(pIndex)->GetMoney();

            if( betSize < oppBankRoll )
            {

                const float64 oppBetMake = betSize - oppBetAlready;
                //To understand the above, consider that totalexf includes already made bets



/*

				const float64 nextFoldB = ea->pctWillCall( pow(  oppBetMake / (oppBetMake + origPot)  , significance  ) );
				const float64 nextFoldPartialB = ea->pctWillCallD(   pow(  oppBetMake / (oppBetMake + origPot)  , significance  )  )
												*  pow(  oppBetMake / (oppBetMake + origPot)  , significance - 1 ) * significance
                                                * (origPot - oppBetMake * origPotD)
                                                 /(oppBetMake + origPot) /(oppBetMake + origPot);
*/
//                const float64 nextFoldF = 1 -
				nextFold =
						pow(  oppBetMake / (oppBetMake + origPot)  , significanceLinear  );
//				const float64 nextFoldPartialF = -
				nextFoldPartial =
						pow(  oppBetMake / (oppBetMake + origPot)  , significanceLinear - 1 ) * significanceLinear
                    * (origPot - oppBetMake * origPotD)
                    /(oppBetMake + origPot) /(oppBetMake + origPot);


/*
				#ifndef GEOM_COMBO_FOLDPCT

					nextFold = 1 - (nextFoldB + nextFoldF)/2;
					nextFoldPartial = -(nextFoldPartialB + nextFoldPartialF)/2;

				#else

					if( nextFoldB <= 0 || nextFoldF <= 0 )
					{//If they are not going to call no matter what
						nextFold = 1; //So they will fold no matter what
						nextFoldPartial = 0;
						//Betting more won't improve this player's chance of folding, which is already 100%
					}else
					{
						nextFold = sqrt(nextFoldB*nextFoldF);
						nextFoldPartial = -nextFold*(nextFoldPartialB/nextFoldB + nextFoldPartialF/nextFoldF)/2;
						nextFold = 1 - nextFold;
					}

				#endif
*/

            }else
            {///Opponent would be all-in to call this bet
                const float64 oldpot = table->GetPrevPotSize();
                const float64 effroundpot = (origPot - oldpot) * oppBankRoll / betSize;
                const float64 oppBetMake = oppBankRoll - oppBetAlready;

				if( oppBankRoll < minRaiseTo() - chipDenom()/4 )
				{
					allFoldChance = 0;
					allFoldChanceD = 0;
					nextFold = 0;
				}else
				{
/*
					const float64 nextFoldB = e->pctWillCall( pow(oppBetMake / (oppBetMake + oldpot + effroundpot),significance) );
*/
//					const float64 nextFoldF = 1 -
					nextFold =
						pow(oppBetMake / (oppBetMake + oldpot + effroundpot),significanceLinear );
/*
					#ifndef GEOM_COMBO_FOLDPCT
					nextFold = 1 - (nextFoldB + nextFoldF)/2;
					#else
					nextFold = 1-sqrt(nextFoldB*nextFoldF);
					#endif
*/
				}
				nextFoldPartial = 0;
			}


            countMayFold -= 1;
//            countToBeat  += 1;

			if(
				(allFoldChance == 0 && allFoldChanceD == 0) || (nextFold == 0 && nextFoldPartial == 0)
				)
			{
				allFoldChance = 0;
				allFoldChanceD = 0;
			}else
			{
				allFoldChance *= nextFold;
				allFoldChanceD += nextFoldPartial / nextFold;
			}


        }else
		{//Player can't bet anymore
			if( oppBetAlready > 0 )
			{//Must have gone all-in just now, or at least this round
				allFoldChance = 0;
				allFoldChanceD = 0;
			}
		}


        table->incrIndex(pIndex);
    }

	allFoldChanceD *= allFoldChance;

}

float64 ExactCallBluffD::PushGain()
{
    const float64 baseFraction = betFraction(table->GetPotSize() - alreadyBet());

#ifdef BLIND_ADJUSTED_FOLD
        //const float64 blindPerHandGain = ( ViewTable().GetBigBlind()+ViewTable().GetSmallBlind() ) / myMoney / ViewTable().GetNumberAtTable();
        const float64 bigBlindFraction = betFraction( table->GetBigBlind() );
        const float64 smallBlindFraction = betFraction( table->GetSmallBlind() );

        const float64 blindsGain = (1 + baseFraction + bigBlindFraction)*(1 + baseFraction + smallBlindFraction);
        const float64 rawWinFreq = (1.0 / table->GetNumberAtTable()) ;
        const float64 blindsPow = rawWinFreq*(1.0 - 1.0 / table->GetNumberAtTable());


        const float64 totalFG = pow(1+baseFraction,1-2*blindsPow)*pow(blindsGain,blindsPow);

        return totalFG;
    #else
        return 1 + baseFraction;
	#endif
}

float64 ExactCallBluffD::pWin(const float64 betSize)
{
	if( queryinput != betSize )
    {
        query(betSize);
        queryinput = betSize;
    }
    ///Try pow(,impliedFactor) maybe
	return allFoldChance;//*impliedFactor;
}

float64 ExactCallBluffD::pWinD(const float64 betSize)
{
	if( queryinput != betSize )
    {
        query(betSize);
        queryinput = betSize;
    }
	return allFoldChanceD;//*impliedFactor;
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


