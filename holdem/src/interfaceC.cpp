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


#include "arena.h"
#include "../inc/holdemDLL.h"

struct return_money DEFAULT_RETURN_VALUE = { 0.0 , SUCCESS };





/*****************************************************************************
	BEGIN
	Betting round accessors
*****************************************************************************/

///Call this to determine if the big blind has changed
C_DLL_FUNCTION
struct return_money GetBigBlind(void * table_ptr)
{
	struct return_money retval = DEFAULT_RETURN_VALUE;

	if( !table_ptr )
	{
		retval.error_code = NULL_TABLE_PTR;
	}else
	{
		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(table_ptr);

		retval.money = myTable->GetBigBlind();
	}

	return retval;
}

///Call this to determine if the small blind has changed
C_DLL_FUNCTION
struct return_money GetSmallBlind(void * table_ptr)
{
	struct return_money retval = DEFAULT_RETURN_VALUE;

	if( !table_ptr )
	{
		retval.error_code = NULL_TABLE_PTR;
	}else
	{
		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(table_ptr);

		retval.money = myTable->GetSmallBlind();
	}

	return retval;
}

///Get the amount of money playerNumber has in front of him
C_DLL_FUNCTION
struct return_money GetMoney(void * table_ptr, int8 playerNumber)
{
	struct return_money retval = DEFAULT_RETURN_VALUE;

	if( !table_ptr )
	{
		retval.error_code = NULL_TABLE_PTR;
	}else
	{
		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(table_ptr);

		const Player * ptrP = myTable->ViewPlayer(playerNumber);

		if( !ptrP )
		{
			retval.error_code = INTERNAL_INCONSISTENCY;
		}else
		{
			const Player & withP = *ptrP;

			float64 playerMoney = withP.GetMoney() - withP.GetBetSize();

			if( playerMoney < 0 )
			{
				playerMoney = 0.0;
				retval.error_code = INPUT_CLEANED;
			}
			
			retval.money = playerMoney;
		}
	}

	return retval;
}
// According to resolveActions, if the player is all in once a round has 
// completed, myMoney becomes -1 at the same time that myBetSize becomes 0.
// But we're fine because if( playerMoney < 0 ) return 0;



//Override the amount of money playerNumber has in front of him
C_DLL_FUNCTION
enum return_status SetMoney(void * table_ptr, int8 playerNumber, float64 money)
{
	enum return_status error_code = SUCCESS;

	if( !table_ptr )
	{
		error_code = NULL_TABLE_PTR;
	}else
	{
		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(table_ptr);

		const Player * ptrP = myTable->ViewPlayer(playerNumber);

		if( !ptrP )
		{
			error_code = INVALID_PARAMETERS;
		}else
		{
			const Player & withP = *ptrP;

			float64 accountedFor = withP.GetBetSize();
			float64 newMoney = money - accountedFor;
	
			if( newMoney < 0 )
			{
				newMoney = 0.0;
				error_code = INPUT_CLEANED;
			}

			if( myTable->OverridePlayerMoney(playerNumber,newMoney) )
			{
				error_code = INTERNAL_INCONSISTENCY;
			}
		}
	}

	return error_code;
}



///Get the amount of money playerNumber has bet so far this round
C_DLL_FUNCTION
struct return_money GetCurrentBet(void * table_ptr, int8 playerNumber);
///TODO: If the player is all in, make sure this returns the correct value



///Get the amount of money that is in the pot
C_DLL_FUNCTION
struct return_money GetPotSize(void * table_ptr);


///Get the amount of money that was in the pot at the BEGINNING of the current betting round
C_DLL_FUNCTION
struct return_money GetLastRoundPotsize(void * table_ptr);


///Get the size of the highest bet so far
struct return_money GetBetToCall(void * table_ptr);


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
C_DLL_FUNCTION enum return_status PlayerCalls(void * table_ptr, int8 playerNumber);
C_DLL_FUNCTION enum return_status PlayerFolds(void * table_ptr, int8 playerNumber);
C_DLL_FUNCTION enum return_status PlayerRaisesTo(void * table_ptr, int8 playerNumber, float64 amount);
C_DLL_FUNCTION enum return_status PlayerRaisesBy(void * table_ptr, int8 playerNumber, float64 amount);
///Question: If a player doesn't call any of these, which is the default action?


///GetBetAmount is useful for asking a bot what to bet
C_DLL_FUNCTION struct return_money GetBetDecision(void * table_ptr, int8 playerNumber);



///Call this when (if) the showdown begins
C_DLL_FUNCTION enum return_status StartShowdown(void * table_ptr);

///Call this for each card playerNumber reveals during the showdown
///See NewCommunityCard for usage of cardValue and cardSuit
C_DLL_FUNCTION enum return_status PlayerShowsCard(void * table_ptr, int8 playerNumber, char cardValue, char cardSuit);

///Call this when playerNumber mucks his/her hand during the showdown.
///Note: If a player doesn't PlayerShowsCard() then a muck is assumed
C_DLL_FUNCTION enum return_status PlayerMucksHand(void * table_ptr, int8 playerNumber);





///Call this when new hands are dealt
C_DLL_FUNCTION enum return_status StartDealNewHands(void * table_ptr);

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

//WARNING ignored for now: seatsAtTable (it's defined as SEATS_AT_TABLE in debug_flags.h)
C_DLL_FUNCTION
void * NewTable(playernumber_t seatsAtTable)
{
    // new SitAndGoBlinds(b.SmallBlind(),b.BigBlind(),blindIncrFreq);
    BlindStructure* b = new GeomPlayerBlinds(1, 2, 1, 1);
    bool illustrate = true;
    bool spectate = true;
    bool externalDealer = true;

    return reinterpret_cast<void *>(new HoldemArena(b, std::cout ,illustrate,spectate));
}

C_DLL_FUNCTION 
enum return_status DeleteTable(void * table_ptr)
{
    HoldemArena * table = reinterpret_cast<HoldemArena *>(table_ptr);
    //table->free_members();
    delete table;
}

///Choose playerNumber to be the dealer for the first hand
C_DLL_FUNCTION 
enum return_status InitChooseDealer(void * table_ptr, int8 playerNumber)
{
    HoldemArena * table = reinterpret_cast<HoldemArena *>(table_ptr);
}

///Set the amount of money that the SMALLEST chip is worth
C_DLL_FUNCTION 
enum return_status InitSmallestChipSize(void * table_ptr, float64 money)
{
    HoldemArena * table = reinterpret_cast<HoldemArena *>(table_ptr);
}

///Call this when the big blind has changed
C_DLL_FUNCTION
enum return_status SetBigBlind(void * table_ptr, float64 money)
{
    HoldemArena * table = reinterpret_cast<HoldemArena *>(table_ptr);
}

///Call this when the small blind has changed
C_DLL_FUNCTION enum 
return_status SetSmallBlind(void * table_ptr, float64 money)
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


