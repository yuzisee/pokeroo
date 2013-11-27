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

//Cosmetic effect of leaving the table is you don't receive broadcasts anymore
//#define LEAVE_TABLE_WHEN_LOSING

#include "arena.h"

#include <iostream>
#include <string.h> //for strlen



// const float64 HoldemArena::BASE_CHIP_COUNT = 100;
 const float64 HoldemArena::FOLDED = -1;
 const float64 HoldemArena::INVALID = -2;


 void HoldemArena::FileNumberString(handnum_t value, char * str)
 {
	sprintf(str,"%u",value);
 }



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
    o << "(" << (int)(NumberInHandInclAllIn()) << " players)" << endl;
    int8 tempIndex = curDealer;
    do
    {
        incrIndex(tempIndex);
        if( CanStillBet(tempIndex) )
        {
            o << "\t[" << p[tempIndex]->GetIdent() << " $" << p[tempIndex]->GetMoney() << "]" << endl;
        }
        else if( IsInHand(tempIndex) && !HasFolded(tempIndex) )
        {
            o << "\t[" << p[tempIndex]->GetIdent() << " all-in]" << endl;
        }
    }while( tempIndex != curDealer );

}

void HoldemArena::addBets(float64 b)
{
	myBetSum += b;
	myPot += b;
}

void HoldemArena::incrIndex(playernumber_t& c) const
{
	++c;
	c %= nextNewPlayer;
}

void HoldemArena::decrIndex(playernumber_t& c) const
{
	if( c == 0 ) c = nextNewPlayer;
	--c;
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
	    Player & broadcastP = (*p[curIndex]);
	    if( broadcastP.IsBot() )
	    {
            #ifdef LEAVE_TABLE_WHEN_LOSING
                if( IsAlive(curIndex) )
            #endif
            broadcastP.myStrat->SeeAction(currentMove);
	    }
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
	    Player & cycleP = (*p[cycleIndex]);
	    if( cycleP.IsBot() )
	    {
            #ifdef LEAVE_TABLE_WHEN_LOSING
                if( IsAlive(cycleIndex) )
            #endif
            cycleP.myStrat->SeeOppHand(broadcaster, h);
	    }
		incrIndex(cycleIndex);
	}

	Player & broadcasterP = (*p[broadcaster]);
	if( broadcasterP.IsBot() )
	{
        broadcasterP.myStrat->SeeOppHand(broadcaster, h);
	}
}



#ifdef GLOBAL_AICACHE_SPEEDUP
void HoldemArena::CachedQueryOffense(CallCumulation& q, const CommunityPlus& community, const CommunityPlus& withCommunity) const
{
    StatsManager::QueryOffense(q,withCommunity,community,cardsInCommunity,&communityBuffer);
}
#endif

playernumber_t HoldemArena::GetCurPlayer() const
{
	return curIndex;
}

playernumber_t HoldemArena::GetDealer() const
{
    return curDealer;
}

float64 HoldemArena::GetAllChips() const
{
	return allChips;
}


playercounts_t const & HoldemArena::NumberAtFirstActionOfRound() const
{
	return playersActiveDuringFirstBetOfRound;
}

playercounts_t const & HoldemArena::NumberStartedRound() const
{
	return startRoundPlayers;
}

playercounts_t const & HoldemArena::NumberInHand() const
{
	return playersInHand;
}


///Number "remaining" at table
playernumber_t HoldemArena::NumberAtTable() const
{
	return livePlayers;
}


playernumber_t HoldemArena::GetTotalPlayers() const
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


const Player* HoldemArena::ViewPlayer(playernumber_t n) const
{
	if (n >= 0 && n < nextNewPlayer )
	{
		return p[n];
	}
	return 0;
}

float64 HoldemArena::GetBetDecision(playernumber_t n)
{
	if (n >= 0 && n < nextNewPlayer )
	{
		if( p[n]->IsBot() )
		{
			return p[n]->myStrat->MakeBet();
		}
	}
	return 0;
}

char HoldemArena::GetPlayerBotType(playernumber_t n) const
{
	if (n >= 0 && n < nextNewPlayer )
	{
		return pTypes[n];
	}
	return 0;
}


uint32 HoldemArena::jenkins_one_at_a_time_hash(const char *key_null_termninated)
{ //http://en.wikipedia.org/wiki/Jenkins_hash_function
	uint32 hash = 0;
	size_t key_len = strlen(key_null_termninated);
    size_t i;

    for (i = 0; i < key_len; i++) {
		unsigned char key = key_null_termninated[i];
        hash += key;
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
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
