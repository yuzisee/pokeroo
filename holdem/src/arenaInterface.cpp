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
    o << "(" << (int)(GetNumberInHand()) << " players" << flush;
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


            blinds->mySmallBlind = HoldemUtil::ReadFloat64( loadFile );
            loadFile.ignore(1,'=');
            blinds->myBigBlind = HoldemUtil::ReadFloat64( loadFile );
            blinds->Reload(blinds->mySmallBlind,blinds->myBigBlind
            #if defined(DEBUGSPECIFIC) || defined(GRAPHMONEY)
            ,handnum
            #endif
            );
            loadFile.ignore(1,'@');
            loadFile >> numericValue;

            curDealer = static_cast<int8>(numericValue);
            loadFile.ignore(1,'@');

            smallestChip = HoldemUtil::ReadFloat64( loadFile );
            loadFile.ignore(1,'^');


            for( int8 i=0;i<nextNewPlayer;++i )
            {
                float64 pMoney = HoldemUtil::ReadFloat64( loadFile );
                p[i]->myMoney = pMoney;
				if( pMoney <= 0 ){
					--livePlayers;
					p[i]->myMoney = -1;
				}else
				{
					p[i]->myMoney = pMoney;
				}
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

#if defined(DEBUGSAVEGAME_ALL) && (defined(DEBUGSPECIFIC) || defined(GRAPHMONEY))
            char handnumtxt/*[12] = "";
            char namebase*/[23+12] = "./" DEBUGSAVEGAME_ALL "/" DEBUGSAVEGAME "-";
            sprintf(handnumtxt + strlen(handnumtxt) ,"%lu",handnum);

            std::ofstream allSaveState( handnumtxt );
#endif

    std::ofstream newSaveState(DEBUGSAVEGAME);
    #if defined(DEBUGSPECIFIC) || defined(GRAPHMONEY)
    newSaveState << handnum << "n";
        #ifdef DEBUGSAVEGAME_ALL
    allSaveState << handnum << "n";
    HoldemUtil::WriteFloat64( allSaveState, blinds->SmallBlind() );
    allSaveState << "=" << flush;
    HoldemUtil::WriteFloat64( allSaveState, blinds->BigBlind() );
    allSaveState << "@" << (int)curDealer << "@" << flush;
    HoldemUtil::WriteFloat64( allSaveState, smallestChip );
    allSaveState << "^" << flush;
        #endif
    #endif
    HoldemUtil::WriteFloat64( newSaveState, blinds->SmallBlind() );
    newSaveState << "=" << flush;
    HoldemUtil::WriteFloat64( newSaveState, blinds->BigBlind() );
    newSaveState << "@" << (int)curDealer << "@" << flush;
    HoldemUtil::WriteFloat64( newSaveState, smallestChip );
    newSaveState << "^" << flush;


    for( int8 i=0;i<nextNewPlayer;++i )
    {
        float64 pMoney =  p[i]->myMoney;
		if( pMoney < 0 ) pMoney = 0;
		HoldemUtil::WriteFloat64( newSaveState, pMoney );
        #if defined(DEBUGSAVEGAME_ALL) && (defined(DEBUGSPECIFIC) || defined(GRAPHMONEY))
        HoldemUtil::WriteFloat64( allSaveState, pMoney );
        #endif
    }
#ifdef DEBUGSAVE_EXTRATOKEN
    newSaveState << EXTRATOKEN << endl;
    #if defined(DEBUGSAVEGAME_ALL) && (defined(DEBUGSPECIFIC) || defined(GRAPHMONEY))
    allSaveState << EXTRATOKEN << endl;
    #endif
#endif
    newSaveState.close();
    #if defined(DEBUGSAVEGAME_ALL) && (defined(DEBUGSPECIFIC) || defined(GRAPHMONEY))
    allSaveState.close();
    #endif
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

int8 HoldemArena::GetDealer() const
{
    return curDealer;
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
    return IsInHand(n) && p[n]->allIn == INVALID && p[n]->GetMoney() > 0;
}

bool HoldemArena::CanRaise(int8 n) const
{
    return ( (curHighBlind == n) || (highBet > p[n]->myBetSize) || (highBet <= 0) ) && (CanStillBet(n));
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


