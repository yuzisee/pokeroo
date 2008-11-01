/***************************************************************************
 *   Copyright (C) 2005 by Joseph Huang                                    *
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

//#define LEAVE_TABLE_WHEN_LOSING

#include "arena.h"
#include <iostream>
#include <string.h> //for strlen



// const float64 HoldemArena::BASE_CHIP_COUNT = 100;
 const float64 HoldemArena::FOLDED = -1;
 const float64 HoldemArena::INVALID = -2;




void HoldemArena::ToString(const HoldemAction& e, std::ostream& o)
{
    if ( e.IsFold() )
    {
        o << "folds" << endl;
    }
    else if ( e.IsCheck() )
    {
        o << "checks" << endl;
    }else
    {
        if ( e.IsCall() )
        {
            o << "calls $"  << e.incrBet << flush;
        }
        else
        {

            if ( e.bBlind == HoldemAction::BIGBLIND )
            {
                if( e.IsAllIn() ) o << "all-in, " << flush;
                o << "posts BB of $" << flush;
                //SpaceBot posts SB of $0.50
            }
            else if ( e.bBlind == HoldemAction::SMALLBLIND )
            {
                if( e.IsAllIn() ) o << "all-in, " << flush;
                o << "posts SB of $" << flush;
            }
            else if ( e.IsRaise() )
            {
                o << "raises " << flush;
                if( e.IsAllIn() ) o << "all-in " << flush;
                //o << "by " << e.GetRaiseBy() << flush;
                    o << "to $" << flush;
            }
            else
            {
                if( e.IsAllIn() )
                {
                    o << "pushes all-in, $" << flush;
                }else
                {
                    o << "bets $" << flush;
                }
            }
            o << e.GetAmount() ;
        }
        o << " ($" << e.newPotSize << ")" << endl;// Chips left: " << e.chipsLeft << ")" << endl;
    }
}

void HoldemArena::PrintPositions(std::ostream& o)
{
    o << "(" << (int)(NumberInHand()) << " players" << flush;
    int8 tempIndex = curDealer;
    do
    {
        incrIndex(tempIndex);
        if( CanStillBet(tempIndex) )
        {
            o << ", " << p[tempIndex]->GetIdent() << " $" << p[tempIndex]->GetMoney() << flush;
        }
        else if( IsInHand(tempIndex) && !HasFolded(tempIndex) )
        {
            o << ", " << p[tempIndex]->GetIdent() << " all-in" << flush;
        }
    }while( tempIndex != curDealer );

    o << ")" << endl;
}

void HoldemArena::addBets(float64 b)
{
	myBetSum += b;
	myPot += b;
}

void HoldemArena::incrIndex(int8& c) const
{
	++c;
	c %= nextNewPlayer;
}

void HoldemArena::incrIndex()
{
	++curIndex;
	curIndex %= nextNewPlayer;
}

void HoldemArena::broadcastCurrentMove(const int8& playerID, const float64& theBet, const float64 theIncrBet
	, const float64& toCall, const int8 bBlind, const bool& isBlindCheck, const bool& isAllIn)
{
    const float64 moneyRemain = p[playerID]->GetMoney() - theBet;
	const HoldemAction currentMove(myPot + theIncrBet,theIncrBet, moneyRemain ,  playerID, theBet, toCall, bBlind , isBlindCheck, isAllIn);

	//ASSERT ( playerID == curIndex )
	int8 cycleIndex = curIndex;
	incrIndex();

	while(cycleIndex != curIndex)
	{
            #ifdef LEAVE_TABLE_WHEN_LOSING
                if( IsAlive(curIndex) )
            #endif
        (*p[curIndex]).myStrat->SeeAction(currentMove);
    	incrIndex();
	}
	if( bSpectate )
	{
	    gamelog << p[currentMove.GetPlayerID()]->GetIdent() << " " << flush;
	    ToString(currentMove, gamelog);
	}

}

void HoldemArena::broadcastHand(const Hand& h, const int8 broadcaster)
{

	int8 cycleIndex = broadcaster;
	incrIndex(cycleIndex);

	while(cycleIndex != broadcaster)
	{
            #ifdef LEAVE_TABLE_WHEN_LOSING
                if( IsAlive(cycleIndex) )
            #endif
        (*p[cycleIndex]).myStrat->SeeOppHand(broadcaster, h);
		incrIndex(cycleIndex);
	}
	(*p[broadcaster]).myStrat->SeeOppHand(broadcaster, h);
}


void HoldemArena::UnserializeRoundStart(std::ifstream & fileLoadState)
{

            fileLoadState >> handnum ;
            fileLoadState.ignore(1,'n');

	    int16 numericValue;

/*
            blinds->mySmallBlind = HoldemUtil::ReadFloat64( fileLoadState );
            fileLoadState.ignore(1,'=');
            blinds->myBigBlind = HoldemUtil::ReadFloat64( fileLoadState );

        blinds->Reload(blinds->mySmallBlind,blinds->myBigBlind,handnum);

	    fileLoadState.ignore(1,'@');
*/
            fileLoadState >> numericValue;

	    curDealer = static_cast<int8>(numericValue);

	    fileLoadState.ignore(1,'@');
            smallestChip = HoldemUtil::ReadFloat64( fileLoadState );
            fileLoadState.ignore(1,'^');


            for( int8 i=0;i<nextNewPlayer;++i )
            {
                float64 pMoney = HoldemUtil::ReadFloat64( fileLoadState );
                p[i]->myMoney = pMoney;
				if( pMoney <= 0 ){
					--livePlayers;
					p[i]->myMoney = -1;
				}else
				{
					p[i]->myMoney = pMoney;
				}
            }




}


void HoldemArena::SerializeRoundStart(std::ofstream & fileSaveState)
{
    	fileSaveState << handnum << "n";
/*
    HoldemUtil::WriteFloat64( fileSaveState, blinds->SmallBlind() );
    fileSaveState << "=" << flush;
    HoldemUtil::WriteFloat64( fileSaveState, blinds->BigBlind() );
    fileSaveState << "@" << flush;
*/
    fileSaveState << (int)curDealer << "@" << flush;
    HoldemUtil::WriteFloat64( fileSaveState, smallestChip );
    fileSaveState << "^" << flush;

    for( int8 i=0;i<nextNewPlayer;++i )
    {
        float64 pMoney =  p[i]->myMoney;
	if( pMoney < 0 ) pMoney = 0;
	HoldemUtil::WriteFloat64( fileSaveState, pMoney );
    }

}


playernumber_t HoldemArena::AddPlayer(const char* const id, const float64 money, PlayerStrategy* newStrat)
{

#ifdef DEBUGASSERT
	if( curIndex != -1 || nextNewPlayer >= SEATS_AT_TABLE)
	{
	    std::cerr << "Cannot add more players" << endl;
	    exit(1);
	}
#endif


    Player* newP = new Player(money, id,newStrat, INVALID);

	if( newStrat )
	{
	    newStrat->Link(newP, this, nextNewPlayer);
	}

	p[nextNewPlayer] = newP;  //p.push_back( newP );


    allChips += money;

	++nextNewPlayer;
	++livePlayers;

	return (nextNewPlayer-1);
}

#ifdef GLOBAL_AICACHE_SPEEDUP
void HoldemArena::CachedQueryOffense(CallCumulation& q, const CommunityPlus& community, const CommunityPlus& withCommunity) const
{
    StatsManager::QueryOffense(q,withCommunity,community,cardsInCommunity,&communityBuffer);
}
#endif

HoldemArena::~HoldemArena()
{


#ifdef GLOBAL_AICACHE_SPEEDUP
    if( communityBuffer != 0 )
    {
        delete communityBuffer;
        communityBuffer = 0;
    }
#endif

    for(int8 n=0;n<SEATS_AT_TABLE;++n){ if(p[n]){ delete p[n]; }}

	//delete [] p;
}

int8 HoldemArena::GetCurPlayer() const
{
	return curIndex;
}

int8 HoldemArena::GetDealer() const
{
    return curDealer;
}

float64 HoldemArena::GetAllChips() const
{
	return allChips;
}

int8 HoldemArena::NumberAtRound() const
{
	return roundPlayers;
}


int8 HoldemArena::NumberInHand() const
{
	return playersInHand;
}

///Number "remaining" at table
int8 HoldemArena::NumberAtTable() const
{
	return livePlayers;
}


int8 HoldemArena::GetTotalPlayers() const
{
	return nextNewPlayer;
}

bool HoldemArena::IsAlive(int8 n) const //IsAtTable()
{
	if (n < 0 || n >= nextNewPlayer )
	{ return false; }

	return (p[n]->GetMoney() >= 0);
}

bool HoldemArena::IsInHand(int8 n) const
{
	if (n < 0 || n >= nextNewPlayer )
	{ return false; }

	return IsAlive(n) && (p[n]->GetBetSize() >= 0);
}


const Player* HoldemArena::ViewPlayer(int8 n) const
{
	if (n >= 0 && n < nextNewPlayer )
	{
		return p[n];
	}
	return 0;
}


float64 HoldemArena::GetPotSize() const
{
	return myPot;
}

float64 HoldemArena::GetFoldedPotSize() const
{
	return myFoldedPot;
}

float64 HoldemArena::GetUnfoldedPotSize() const
{
	return myPot - myFoldedPot;
}

float64 HoldemArena::GetDeadPotSize() const
{
	return myPot - myBetSum;
}

float64 HoldemArena::GetLivePotSize() const
{
	return myBetSum;
}

float64 HoldemArena::GetRoundPotSize() const
{
	return myPot - prevRoundPot;
}
float64 HoldemArena::GetPrevFoldedRetroactive() const
{
    return prevRoundFoldedPot;
}

float64 HoldemArena::GetPrevPotSize() const
{
	return prevRoundPot;
}

///This is the sum of bets made this round by people still in the round
float64 HoldemArena::GetRoundBetsTotal() const
{
    return GetRoundPotSize() - forcedBetSum;
}

float64 HoldemArena::GetUnbetBlindsTotal() const
{
    return blindOnlySum;
}


float64 HoldemArena::GetMinRaise() const
{
    return lastRaise;
	//return 0;
}

float64 HoldemArena::GetChipDenom() const
{
    return smallestChip;
}

bool HoldemArena::HasFolded(int8 n) const
{
	return p[n]->myBetSize == FOLDED;
}

bool HoldemArena::CanStillBet(int8 n) const
{
    return IsInHand(n) && p[n]->allIn == INVALID && p[n]->GetMoney() > 0;
}

uint8 HoldemArena::FutureRounds() const
{
    return bettingRoundsRemaining;
}

uint8 HoldemArena::RaiseOpportunities(int8 n, int8 nowBettor) const
{
    bool bHasPlayed = (((curDealer < n) && (n < nowBettor)) && (curDealer < nowBettor))//Between curDealer and newBettor when curDealer is before nowBettor
                        ||
                      (((curDealer < n) || (n < nowBettor)) && (nowBettor < curDealer))
					    ||
				      (nowBettor == curDealer)
                        ;
    bool bCanCheck = (highBet <= 0) && (!bHasPlayed);
    if(
	    ( (curHighBlind == n) || (highBet > p[n]->myBetSize) || (bCanCheck) ) && (CanStillBet(n))
    )
    {
	    return FutureRounds() + 1;
    }
    else
    {
	    return FutureRounds();
    }
}

float64 HoldemArena::GetBetToCall() const
{
	return highBet;
}

float64 HoldemArena::GetMaxShowdown(const float64 myMoney) const
{
	float64 highestMoney = -1;
	float64 secondHighestMoney = -1;


	for(int8 i=0;i<nextNewPlayer;++i)
	{
		if(! HasFolded(i) )
		{
		    if( highestMoney <= 0 || p[i]->GetMoney() > highestMoney )
			{
				secondHighestMoney = highestMoney;
				highestMoney = p[i]->GetMoney();
			}else if( secondHighestMoney <= 0 ||
                        p[i]->GetMoney() > secondHighestMoney )
			{
				secondHighestMoney = p[i]->GetMoney();
			}
		}
	}

    if( myMoney > 0 && myMoney < secondHighestMoney )
    {
        return myMoney;
    }else
    {
        return secondHighestMoney;
    }
}



bool HoldemArena::OverridePlayerMoney(playernumber_t n, float64 m)
{
	if( !p[n] ) return false;

	p[n]->myMoney = m;
	return true;
}
