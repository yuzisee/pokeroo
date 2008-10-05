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

#include "portability.h"

#include "arena.h"


/*****************************************************************************
	BEGIN
	Betting round accessors
*****************************************************************************/

///Call this to determine if the big blind has changed
void GetBigBlind(void * table_ptr)
{}

///Call this to determine if the small blind has changed
void GetSmalllind(void * table_ptr)
{}

///Get the amount of money playerNumber has in front of him
float64 GetMoney(void * table_ptr, int8 playerNumber);
//TODO: If the player is all in, make sure this returns the correct value



//Override the amount of money playerNumber has in front of him
void SetMoney(void * table_ptr, int8 playerNumber, float64 money);



///Get the amount of money playerNumber has bet so far this round
float64 GetCurrentBet(void * table_ptr, int8 playerNumber);
///TODO: If the player is all in, make sure this returns the correct value



///Get the amount of money that is in the pot
float64 GetPotSize(void * table_ptr);


///Get the amount of money that was in the pot at the BEGINNING of the current betting round
float64 GetLastRoundPotsize(void * table_ptr);


///Get the size of the highest bet so far
float64 GetBetToCall(void * table_ptr);


//Get the playerNumber of the player who's turn it is
int8 WhoIsNext(void * table_ptr);

/*****************************************************************************
	Betting round accessors
	END
*****************************************************************************/





/*****************************************************************************
	BEGIN
	Event functions
*****************************************************************************/


///Call NewCommunityCard for each card that is dealt to the table during flop/turn/river
///cardValue and cardSuit are both characters:
/*
cardValue can be any of:
'2' for two
'3' for three
'4' for four
'5' for five
'6' for six
'7' for seven
'8' for eight
'9' for nine
't' for ten (notice lowercase 't')
'J' for Jack
'Q' for Queen
'K' for King
'A' for Ace


cardSuit can be any of:
'S' for Spades
'H' for Hearts
'C' for Clubs
'D' for Diamonds
*/
///For example, for the eight of hearts: cardValue = '8' and cardSuit = 'H'
void NewCommunityCard(void * table_ptr, char cardValue,char cardSuit);





///Call this when the betting begins
void StartBetting(void * table_ptr);

///Call these functions when playerNumber Raises, Folds, or Calls
void PlayerCalls(void * table_ptr, int8 playerNumber);
void PlayerFolds(void * table_ptr, int8 playerNumber);
void PlayerRaisesTo(void * table_ptr, int8 playerNumber, float64 amount);
void PlayerRaisesBy(void * table_ptr, int8 playerNumber, float64 amount);
///Question: If a player doesn't call any of these, which is the default action?


///GetBetAmount is useful for asking a bot what to bet
float64 GetBetDecision(void * table_ptr, int8 playerNumber);



///Call this when (if) the showdown begins
void StartShowdown(void * table_ptr);

///Call this for each card playerNumber reveals during the showdown
///See NewCommunityCard for usage of cardValue and cardSuit
void PlayerShowsCard(void * table_ptr, int8 playerNumber, char cardValue, char cardSuit);

///Call this when playerNumber mucks his/her hand during the showdown.
///Note: If a player doesn't PlayerShowsCard() then a muck is assumed
void PlayerMucksHand(void * table_ptr, int8 playerNumber);





///Call this when new hands are dealt
void StartDealNewHands(void * table_ptr);

/*****************************************************************************
	Event functions
	END
*****************************************************************************/




/*****************************************************************************
	BEGIN
	Initial setup functions

Note: SetBigBlind() and SetSmallBlind() can be called between
hands anytime the blind size changes during the game
*****************************************************************************/

void * NewTable()
{
    // new SitAndGoBlinds(b.SmallBlind(),b.BigBlind(),blindIncrFreq);
    BlindStructure* b = new GeomPlayerBlinds(1, 2, 1, 1);
    bool illustrate = true;
    bool spectate = true;
    bool externalDealer = true;


    return reinterpret_cast<void *>(new HoldemArena(b, std::cout ,illustrate,spectate));
}

void DeleteTable(void * table_ptr)
{
    HoldemArena * table = reinterpret_cast<HoldemArena *>(table_ptr);
    table->free_members();
    delete table;
}

///Choose playerNumber to be the dealer for the first hand
void InitChooseDealer(void * table_ptr, int8 playerNumber)
{
    HoldemArena * table = reinterpret_cast<HoldemArena *>(table_ptr);
}

///Set the amount of money that the SMALLEST chip is worth
void InitSmallestChipSize(void * table_ptr, float64 money)
{
    HoldemArena * table = reinterpret_cast<HoldemArena *>(table_ptr);
}

///Call this when the big blind has changed
void SetBigBlind(void * table_ptr, float64 money)
{
    HoldemArena * table = reinterpret_cast<HoldemArena *>(table_ptr);
}

///Call this when the small blind has changed
void SetSmallBlind(void * table_ptr, float64 money)
{
    HoldemArena * table = reinterpret_cast<HoldemArena *>(table_ptr);
}

///Add a player to the table. PLAYERS MUST BE ADDED IN CLOCKWISE ORDER.
///The function returns a playerNumber to identify this player in your code
int8 AddHumanOpponent(void * table_ptr, char * playerName)
{
    HoldemArena * table = reinterpret_cast<HoldemArena *>(table_ptr);
    //table->AddHuman(playerName, money,
    //p[newID]->bSync = true;

}

int8 AddStrategyBot(void * table_ptr, char *playerName, char botType)
{
    HoldemArena * table = reinterpret_cast<HoldemArena *>(table_ptr);

}


/*****************************************************************************
	Initial setup functions
	END
*****************************************************************************/


