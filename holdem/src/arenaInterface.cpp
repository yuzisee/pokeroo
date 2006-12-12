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


 const float64 HoldemArena::BASE_CHIP_COUNT = 100;
 const float64 HoldemArena::FOLDED = -1;
 const float64 HoldemArena::INVALID = -2;

void HoldemArena::ToString(const HoldemAction& e, std::ostream& o)
{
    if ( e.IsFold() )
    {
        o << "folds." << endl;
    }
    else if ( e.IsCheck() )
    {
        o << "checks." << endl;
    }
    else if ( e.IsCall() )
    {
        o << "calls." << endl;
    }
    else
    {
        if ( e.IsRaise() )
        {
            o << "raises by " << e.GetRaiseBy()
                << " to " << flush;
        }
        else
        {
            o << "bets " << flush;
        }
        o << e.GetAmount() << "." << endl;
    }
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

void HoldemArena::broadcastCurrentMove(const int8& playerID, const float64& theBet
	, const float64& toCall, const bool& isBlindCheck, const bool& isAllIn)
{
	const HoldemAction currentMove(playerID, theBet, toCall, isBlindCheck, isAllIn);

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
}

#ifdef DEBUGSAVEGAME
std::istream * HoldemArena::LoadState()
{
    loadFile.open(DEBUGSAVEGAME);
    if( ! (loadFile.is_open()) )
    {
        return 0;
    }
    bLoadGame = true;


#if defined(DEBUGSPECIFIC) || defined(GRAPHMONEY)
            loadFile >> handnum ;
            loadFile.ignore(1,'n');
#endif
            int16 numericValue;

            loadFile >> blinds->mySmallBlind;
            loadFile.ignore(1,'=');
            loadFile >> blinds->myBigBlind;
            loadFile.ignore(1,'@');
            loadFile >> numericValue;

            curDealer = static_cast<int8>(numericValue);
            loadFile.ignore(1,'@');

            for( int8 i=0;i<nextNewPlayer;++i )
            {
                float64 pMoney;
                uint32 *pMoneyU = reinterpret_cast< uint32* >( &pMoney );
                loadFile >> *pMoneyU;
                loadFile.ignore(1,'x');
                loadFile >> *(pMoneyU+1);
                loadFile.ignore(1,':');
                p[i]->myMoney = pMoney;
                if( pMoney == 0 ) --livePlayers;
            }
            #ifdef DEBUGSAVE_EXTRATOKEN
            loadFile.getline(EXTRATOKEN, DEBUGSAVE_EXTRATOKEN);
            #endif

            dealer.Unserialize( loadFile );

            return &loadFile;
}


void HoldemArena::saveState()
{
    if( loadFile.is_open() ) loadFile.close();


    std::ofstream newSaveState(DEBUGSAVEGAME);
    #if defined(DEBUGSPECIFIC) || defined(GRAPHMONEY)
    newSaveState << handnum << "n";
    #endif
    newSaveState << blinds->SmallBlind() << "=" << blinds->BigBlind() << "@" << (int)curDealer << "@" << flush;
    for( int8 i=0;i<nextNewPlayer;++i )
    {
        float64 pMoney =  p[i]->myMoney;
        uint32 *pMoneyU = reinterpret_cast< uint32* >( &pMoney );
        newSaveState << *pMoneyU << "x" << *(pMoneyU+1) << ":" << flush;
    }
#ifdef DEBUGSAVE_EXTRATOKEN
    newSaveState << EXTRATOKEN << endl;
#endif
    newSaveState.close();
}
#endif

int8 HoldemArena::AddPlayer(const char* id, PlayerStrategy* newStrat)
{
	return AddPlayer(id, BASE_CHIP_COUNT, newStrat);
}

int8 HoldemArena::AddPlayer(const char* id, float64 money, PlayerStrategy* newStrat)
{
	if( curIndex != -1 || newStrat->game != 0 || newStrat->me != 0) return -1;

    newStrat->myPositionIndex = nextNewPlayer;
	newStrat->game = this;
	Player* newP = new Player(money, id,newStrat, INVALID);
	newStrat->me = newP;
	newStrat->myHand = &(newP->myHand);
	p.push_back( newP );

    allChips += money;

	++nextNewPlayer;
	++livePlayers;

	return (nextNewPlayer-1);
}

#ifdef GLOBAL_AICACHE_SPEEDUP
void HoldemArena::CachedQueryOffense(CallCumulation& q, const CommunityPlus& withCommunity) const
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
	while(! (p.empty()) )
	{
		delete p.back();
		p.pop_back();
	}
}

int8 HoldemArena::GetCurPlayer() const
{
	return curIndex;
}

float64 HoldemArena::GetAllChips() const
{
	return allChips;
}

int8 HoldemArena::GetNumberInHand() const
{
	return playersInHand;
}

///Number "left" at table
int8 HoldemArena::GetNumberAtTable() const
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

float64 HoldemArena::GetPrevPotSize() const
{
	return prevRoundPot;
}

///This is the sum of bets made this round by people still in the round
float64 HoldemArena::GetRoundBetsTotal() const
{
    return GetRoundPotSize() - blindBetSum;
}


float64 HoldemArena::GetMinRaise() const
{
    return lastRaise;
	//return 0;
}

float64 HoldemArena::GetBigBlind() const
{
	return blinds->BigBlind();
}

float64 HoldemArena::GetSmallBlind() const
{
	return blinds->SmallBlind();
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
    return IsInHand(n) && p[n]->allIn == INVALID;
}

float64 HoldemArena::GetBetToCall() const
{
	return highBet;
}
/*
float64 HoldemArena::GetMaxShowdown() const
{
	int8 highest;
	int8 secondhighest;

    std::cout << p[0]->GetIdent() << " can still bet " << p[0]->GetMoney() - p[0]->GetBetSize() << std::endl;
    std::cout << p[1]->GetIdent() << " can still bet " << p[1]->GetMoney() - p[1]->GetBetSize() << std::endl;

    if( p[1]->GetMoney() - p[1]->GetBetSize()
             >
             p[0]->GetMoney() - p[0]->GetBetSize() )
    {
        highest = 1;
        secondhighest = 0;
    }else
    {
        highest = 0;
        secondhighest = 1;
    }

	for(int8 i=2;i<nextNewPlayer;++i)
	{
		if(! HasFolded(i) )
		{
		    std::cout << p[i]->GetIdent() << " can still bet " << p[i]->GetMoney() - p[i]->GetBetSize() << std::endl;
			if( p[i]->GetMoney() - p[i]->GetBetSize()
						 >
						 p[highest]->GetMoney() - p[highest]->GetBetSize() )
			{
				secondhighest = highest;
				highest = i;
			}
		}
	}

	return p[secondhighest]->GetMoney() - p[secondhighest]->GetBetSize();
}

*/
float64 HoldemArena::GetMaxShowdown() const
{
	int8 highest;
	int8 secondhighest;

    if( p[1]->GetMoney()
             >
             p[0]->GetMoney() )
    {
        highest = 1;
        secondhighest = 0;
    }else
    {
        highest = 0;
        secondhighest = 1;
    }

	for(int8 i=2;i<nextNewPlayer;++i)
	{
		if(! HasFolded(i) )
		{
			if( p[i]->GetMoney()
						 >
						 p[highest]->GetMoney() )
			{
				secondhighest = highest;
				highest = i;
			}else if( p[i]->GetMoney()
						>
						p[secondhighest]->GetMoney() )
			{
				secondhighest = i;
			}
		}
	}

	return p[secondhighest]->GetMoney();
}


