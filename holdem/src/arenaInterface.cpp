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

 const float64 HoldemArena::BASE_CHIP_COUNT = 100;
 const float64 HoldemArena::FOLDED = -1;
 const float64 HoldemArena::INVALID = -2;

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

}

void HoldemArena::broadcastHand(const Hand& h)
{
	int8 cycleIndex = curIndex;
	incrIndex();

	while(cycleIndex != curIndex)
	{
            #ifdef LEAVE_TABLE_WHEN_LOSING
                if( IsAlive(curIndex) )
            #endif
        (*p[curIndex]).myStrat->SeeOppHand(cycleIndex, h);
		incrIndex();
	}
}

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



	++nextNewPlayer;
	++livePlayers;

	return (nextNewPlayer-1);
}


HoldemArena::~HoldemArena()
{
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

int8 HoldemArena::GetNumberInHand() const
{
	return playersInHand;
}

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

float64 HoldemArena::GetRoundPotSize() const
{
	return myBetSum;
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

float64 HoldemArena::GetMaxShowdown() const
{
	int8 highest = 1;
	int8 secondhighest = 1;


	for(int8 i=0;i<nextNewPlayer;++i)
	{
		if(! HasFolded(i) )
		{
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

