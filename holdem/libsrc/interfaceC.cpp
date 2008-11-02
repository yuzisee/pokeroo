/***************************************************************************
 *   Copyright (C) 2008 by Joseph Huang                                    *
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

#include <sstream>


#include "../src/arena.h"
#include "../src/stratCombined.h"
#include "../src/stratPosition.h"

#include "holdemDLL.h"


struct return_money DEFAULT_RETURN_MONEY = { 0.0 , SUCCESS };
struct return_betting_result DEFAULT_RETURN_BETTING_RESULT = { -1, SUCCESS };
struct return_event DEFAULT_RETURN_EVENT = { 0 , SUCCESS };
struct return_player DEFAULT_RETURN_PLAYER = { {'\0',-1} , SUCCESS };
struct return_table DEFAULT_RETURN_TABLE = { {0,0,-1} , SUCCESS };



/*****************************************************************************
	BEGIN
	Betting round accessors
*****************************************************************************/


///Get the amount of money playerNumber has in front of him
C_DLL_FUNCTION
struct return_money GetMoney(void * table_ptr, playernumber_t playerNumber)
{
	struct return_money retval = DEFAULT_RETURN_MONEY;

	if( !table_ptr )
	{
		retval.error_code = NULL_TABLE_PTR;
	}else
	{
		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(table_ptr);

		const Player * ptrP = myTable->ViewPlayer(playerNumber);

		if( !ptrP )
		{
			retval.error_code = PARAMETER_INVALID;
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
enum return_status SetMoney(void * table_ptr, playernumber_t playerNumber, float64 money)
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
			error_code = PARAMETER_DATA_ERROR;
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

			if( ! myTable->OverridePlayerMoney(playerNumber,newMoney) )
			{
				error_code = PARAMETER_INVALID;
			}
		}
	}

	return error_code;
}



///Get the amount of money playerNumber has bet so far this round
C_DLL_FUNCTION
struct return_money GetCurrentRoundBet(void * table_ptr, playernumber_t playerNumber)
{
	struct return_money retval = DEFAULT_RETURN_MONEY;

	if( !table_ptr )
	{
		retval.error_code = NULL_TABLE_PTR;
	}else
	{
		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(table_ptr);

		const Player * ptrP = myTable->ViewPlayer(playerNumber);

		if( !ptrP )
		{
			retval.error_code = PARAMETER_INVALID;
		}else
		{
			const Player & withP = *ptrP;

			float64 playerBet = withP.GetBetSize();

			if( playerBet < 0 )
			{
				playerBet = 0.0;
				retval.error_code = INPUT_CLEANED;
			}
			
			retval.money = playerBet;
		}
	}

	return retval;
}
///resolveActions will make GetBetSize invalid. However, at that point the round is over anyways.



///Get the amount of money playerNumber has bet so far in all previous rounds
C_DLL_FUNCTION
struct return_money GetPrevRoundsBet(void * table_ptr, playernumber_t playerNumber)
{
	struct return_money retval = DEFAULT_RETURN_MONEY;

	if( !table_ptr )
	{
		retval.error_code = NULL_TABLE_PTR;
	}else
	{
		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(table_ptr);

		const Player * ptrP = myTable->ViewPlayer(playerNumber);

		if( !ptrP )
		{
			retval.error_code = PARAMETER_INVALID;
		}else
		{
			const Player & withP = *ptrP;

			float64 playerHandBetTotal = withP.GetContribution();

			if( playerHandBetTotal < 0 )
			{
				playerHandBetTotal = 0.0;
				retval.error_code = INPUT_CLEANED;
			}
			
			retval.money = playerHandBetTotal;
		}
	}

	return retval;
}
///Looking at resolveActions, handBetTotal is not used for any special purpose regarding allIns



///Get the amount of money that is in the pot
C_DLL_FUNCTION
struct return_money GetPotSize(void * table_ptr)
{
	struct return_money retval = DEFAULT_RETURN_MONEY;

	if( !table_ptr )
	{
		retval.error_code = NULL_TABLE_PTR;
	}else
	{
		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(table_ptr);

		retval.money = myTable->GetPotSize();
	}

	return retval;
}


///Get the amount of money that was in the pot at the BEGINNING of the current betting round
C_DLL_FUNCTION
struct return_money GetPrevRoundsPotsize(void * table_ptr)
{
	struct return_money retval = DEFAULT_RETURN_MONEY;

	if( !table_ptr )
	{
		retval.error_code = NULL_TABLE_PTR;
	}else
	{
		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(table_ptr);

		retval.money = myTable->GetPrevPotSize();
	}

	return retval;
}


///Get the size of the highest bet so far this round
C_DLL_FUNCTION
struct return_money GetBetToCall(void * table_ptr)
{
	struct return_money retval = DEFAULT_RETURN_MONEY;

	if( !table_ptr )
	{
		retval.error_code = NULL_TABLE_PTR;
	}else
	{
		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(table_ptr);

		retval.money = myTable->GetBetToCall();
	}

	return retval;
}



/*****************************************************************************
	Betting round accessors
	END
*****************************************************************************/



/*****************************************************************************
	BEGIN
	Card functions
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

C_DLL_FUNCTION
struct holdem_cardset CreateNewCardset()
{
	CommunityPlus * newHand = new CommunityPlus();

	struct holdem_cardset c;
	c.card_count = 0;
	c.cards_ptr = reinterpret_cast<void *>(newHand);

	return c;
}

C_DLL_FUNCTION
enum return_status AppendCard(struct holdem_cardset * c, char cardValue,char cardSuit)
{
	enum return_status error_code = SUCCESS;

	if( !c || !(c->cards_ptr) )
	{
		error_code = PARAMETER_DATA_ERROR;
	}else
	{
		CommunityPlus * myHand = reinterpret_cast<CommunityPlus *>(c->cards_ptr);
		
		int8 newCardIndex = HoldemUtil::ParseCard(cardValue,cardSuit);
		if( newCardIndex < 0 )
		{
			error_code = PARAMETER_INVALID;
		}else
		{
			DeckLocation newCard;
			newCard.SetByIndex(newCardIndex);
			
			myHand->AddToHand( newCard );
			++(c->card_count);
		}
	}

	return error_code;
}

C_DLL_FUNCTION
enum return_status DeleteCardset(struct holdem_cardset c)
{
	enum return_status error_code = SUCCESS;

	if( !(c.cards_ptr) )
	{
		error_code = PARAMETER_DATA_ERROR;
	}else
	{
		CommunityPlus * myHand = reinterpret_cast<CommunityPlus *>(c.cards_ptr);
		delete myHand;
	}

	return error_code;
}

/*****************************************************************************
	END
	Card functions
*****************************************************************************/



/*****************************************************************************
	BEGIN
	Betting round functions
*****************************************************************************/




///Call this when betting begins
C_DLL_FUNCTION
struct return_event CreateNewBettingRound(void * table_ptr, struct holdem_cardset community )
{
	struct return_event retval = DEFAULT_RETURN_EVENT;

	if( !table_ptr )
	{
		retval.error_code = NULL_TABLE_PTR;
	}else if( !community.cards_ptr )
	{
		retval.error_code = PARAMETER_INVALID;
	}else
	{
		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(table_ptr);

		CommunityPlus * myHand = reinterpret_cast<CommunityPlus *>(community.cards_ptr);

		HoldemArenaBetting * bettingEvent = new HoldemArenaBetting( myTable, *myHand, community.card_count );
	}

	return retval;
}


C_DLL_FUNCTION
struct return_betting_result DeleteFinishBettingRound(void * event_ptr)
{
	struct return_betting_result retval = DEFAULT_RETURN_BETTING_RESULT;

	if( !event_ptr )
	{
		retval.error_code = PARAMETER_DATA_ERROR;
	}else
	{
		HoldemArenaBetting * bettingEvent = reinterpret_cast<HoldemArenaBetting *>(event_ptr);

		retval.seat_number = bettingEvent->playerCalled;

		//If we're in the middle of a betting round that hasn't concluded,
		// we will report an error but still delete bettingEvent anyways.
		//It's possible that the user could be quitting the program and needs
		// to delete things in the middle of a betting round.
		if( bettingEvent->bBetState == 'b' ) retval.error_code = UNRELIABLE_RESULT;

		//The 'F' state reports that everybody folded to one player.
		//Make sure that this is a consistent result.
		if( bettingEvent->bBetState == 'F' && retval.seat_number != -1 )
		{
			retval.error_code = INTERNAL_INCONSISTENCY;
		}

		//The 'C' state reports that someone called the high bet.
		//Make sure that this is a consistent result.
		if( bettingEvent->bBetState == 'C' && retval.seat_number == -1 )
		{
			retval.error_code = INTERNAL_INCONSISTENCY;
		}

		delete bettingEvent;
	}

	return retval;
}



//Get the seat of the player who's turn it is
C_DLL_FUNCTION
struct return_betting_result WhoIsNext_Betting(void * event_ptr)
{
	struct return_betting_result retval = DEFAULT_RETURN_BETTING_RESULT;

	if( !event_ptr )
	{
		retval.error_code = PARAMETER_DATA_ERROR;
	}else
	{
		HoldemArenaBetting * bettingEvent = reinterpret_cast<HoldemArenaBetting *>(event_ptr);

		if( bettingEvent->bBetState != 'b' ) //if the round has already finished,
		{
			retval.error_code = NOT_IMPLEMENTED; 
		}else
		{
			retval.seat_number = bettingEvent->WhoIsNext();
		}
	}

	return retval;
}


///Call these functions when playerNumber Raises, Folds, or Calls
C_DLL_FUNCTION
enum return_status PlayerMakesBetTo(void * event_ptr, playernumber_t playerNumber, float64 money)
{

	enum return_status error_code = SUCCESS;

	if( !event_ptr )
	{
		error_code = PARAMETER_DATA_ERROR;
	}else
	{
		HoldemArenaBetting * bettingEvent = reinterpret_cast<HoldemArenaBetting *>(event_ptr);

		if( bettingEvent->bBetState != 'b' ) //if the round has already finished,
		{
			error_code = NOT_IMPLEMENTED;
		}else if( bettingEvent->WhoIsNext() != playerNumber )
		{
			error_code = PARAMETER_INVALID;
		}else
		{
			bettingEvent->MakeBet(money);
		}
	}

	return error_code;
	
}


///GetBetAmount is useful for asking a bot what to bet
C_DLL_FUNCTION struct return_money GetBetDecision(void * table_ptr, struct holdem_player player)
{
	struct return_money retval = DEFAULT_RETURN_MONEY;

	if( !table_ptr )
	{
		retval.error_code = NULL_TABLE_PTR;
	}else if( player.seat_number == -1 || player.player_type == ' ' )
	{
		retval.error_code = PARAMETER_INVALID;
	}else if( !player.player_type )
	{
		retval.error_code = PARAMETER_DATA_ERROR;
	}else
	{
		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(table_ptr);

		//If you're asking a bot what it would bet even though there are
		// players ahead of it that haven't bet yet.
		//I don't know if the more complicated bots can handle being out of turn.
		if( myTable->GetCurPlayer() != player.seat_number )
		{
			retval.error_code = UNRELIABLE_RESULT;
		}
		else
		{
			retval.money = myTable->GetBetDecision(player.seat_number);
		}
	}

	return retval;
}


/*****************************************************************************
	Betting round functions
	END
*****************************************************************************/




/*****************************************************************************
	BEGIN
	Showdown functions
*****************************************************************************/



///Call this when betting begins
C_DLL_FUNCTION
struct return_event CreateNewShowdown(void * table_ptr, playernumber_t calledPlayer)
{
	struct return_event retval = DEFAULT_RETURN_EVENT;

	if( !table_ptr )
	{
		retval.error_code = NULL_TABLE_PTR;
	}else
	{
		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(table_ptr);

		HoldemArenaShowdown * bettingEvent = new HoldemArenaShowdown( myTable, calledPlayer );
	}

	return retval;
}



C_DLL_FUNCTION
enum return_status DeleteFinishShowdown(void * event_ptr )
{
	enum return_status error_code = SUCCESS;

	if( !event_ptr )
	{
		error_code = PARAMETER_DATA_ERROR;
	}else
	{
		HoldemArenaShowdown * showdownEvent = reinterpret_cast<HoldemArenaShowdown *>(event_ptr);

		//If we're in the middle of a showdown that hasn't concluded,
		// we will report an error but still delete bettingEvent anyways.
		//It's possible that the user could be quitting the program and needs
		// to delete things in the middle of a betting round.
		if( showdownEvent->bRoundState != '!' ) error_code = UNRELIABLE_RESULT;

		delete showdownEvent;
	}

	return error_code;
}



//Get the seat of the player who's turn it is.
//Players who aren't all-in first reveal/muck in order.
//Then players who are all-in show their cards starting with the largest stack.
C_DLL_FUNCTION
struct return_betting_result WhoIsNext_Showdown(void * event_ptr)
{
	struct return_betting_result retval = DEFAULT_RETURN_BETTING_RESULT;

	if( !event_ptr )
	{
		retval.error_code = PARAMETER_DATA_ERROR;
	}else
	{
		HoldemArenaShowdown * showdownEvent = reinterpret_cast<HoldemArenaShowdown *>(event_ptr);

		if( showdownEvent->bRoundState == '!' ) //if the round has already finished,
		{
			retval.error_code = NOT_IMPLEMENTED; 
		}else
		{
			retval.seat_number = showdownEvent->WhoIsNext();
		}
	}

	return retval;
}


///Call this for each card playerNumber reveals during the showdown
///See NewCommunityCard for usage of cardValue and cardSuit
C_DLL_FUNCTION enum return_status PlayerShowsHand(void * event_ptr, int8 playerNumber, struct holdem_cardset playerHand, struct holdem_cardset community)
{
	enum return_status error_code = SUCCESS;

	if( !event_ptr )
	{
		error_code = PARAMETER_DATA_ERROR;
	}else
	{
		HoldemArenaShowdown * showdownEvent = reinterpret_cast<HoldemArenaShowdown *>(event_ptr);

		if( showdownEvent->bRoundState == '!' ) //if the round has already finished,
		{
			error_code = NOT_IMPLEMENTED;
		}else if( showdownEvent->WhoIsNext() != playerNumber )
		{
			error_code = PARAMETER_INVALID;
		}else if( !playerHand.card_count || !playerHand.cards_ptr || !community.card_count || !community.cards_ptr )
		{
			error_code = PARAMETER_DATA_ERROR;
		}else
		{
			CommunityPlus * dealtHand = reinterpret_cast<CommunityPlus *>(playerHand.cards_ptr);
			CommunityPlus * dealtCommunity = reinterpret_cast<CommunityPlus *>(community.cards_ptr);

			showdownEvent->RevealHand(*dealtHand,*dealtCommunity);
		}
	}

	return error_code;
}

///Call this when playerNumber mucks his/her hand during the showdown.
///Note: If a player doesn't PlayerShowsCard() then a muck is assumed
C_DLL_FUNCTION enum return_status PlayerMucksHand(void * event_ptr, int8 playerNumber)
{
	enum return_status error_code = SUCCESS;

	if( !event_ptr )
	{
		error_code = PARAMETER_DATA_ERROR;
	}else
	{
		HoldemArenaShowdown * showdownEvent = reinterpret_cast<HoldemArenaShowdown *>(event_ptr);

		if( showdownEvent->bRoundState == '!' ) //if the round has already finished,
		{
			error_code = NOT_IMPLEMENTED;
		}else if( showdownEvent->WhoIsNext() != playerNumber )
		{
			error_code = PARAMETER_INVALID;
		}else
		{
			showdownEvent->MuckHand();
		}
	}

	return error_code;
}


/*****************************************************************************
	Showdown functions
	END
*****************************************************************************/




/*****************************************************************************
	BEGIN	
	Flow control functions
*****************************************************************************/




///Call this when new hands are dealt
C_DLL_FUNCTION enum return_status StartDealNewHands(void * table_ptr);



/*****************************************************************************
	Flow control functions
	END
*****************************************************************************/




/*****************************************************************************
	BEGIN
	Initial setup and final destructor functions
*****************************************************************************/


C_DLL_FUNCTION
struct return_table CreateNewTable(playernumber_t seatsAtTable, float64 chipDenomination)
{
	struct return_table retval = DEFAULT_RETURN_TABLE;

	//WARNING seatsAtTable is ignored for now
	//it's defined as SEATS_AT_TABLE in debug_flags.h, and can only have one value
	if( seatsAtTable != SEATS_AT_TABLE )
	{
		retval.error_code = NOT_IMPLEMENTED;
	}else
	{
		bool illustrate = true;
		bool spectate = true;
		bool externalDealer = true;

		retval.table.table_ptr = reinterpret_cast<void *>(new HoldemArena(chipDenomination, std::cout ,illustrate,spectate));
		retval.table.seats_array = new struct holdem_player[seatsAtTable];
		retval.table.seat_count = seatsAtTable;

		for(playernumber_t i=0;i<seatsAtTable;++i)
		{
			retval.table.seats_array[i].seat_number = -1; //Mark as empty. This is the only way we can tell if a seat has no player (refB)
		}
	}

	return retval;
	
}




C_DLL_FUNCTION
struct return_player CreateNewHumanOpponent(struct holdem_table add_to_table, char * playerName, float64 money)
{
	//A human opponent is treated the same as a bot, except it has no strat (and therefore no children)
	struct return_player retval = DEFAULT_RETURN_PLAYER;

	if( !add_to_table.table_ptr )
	{
		retval.error_code = NULL_TABLE_PTR;
	}else if( !playerName || add_to_table.seat_count <= 0 )
	{
		retval.error_code = PARAMETER_DATA_ERROR;
	}else
	{
		playernumber_t final_seat = add_to_table.seat_count - 1;
		if( add_to_table.seats_array[final_seat].seat_number != -1 ) //Already a player in the last seat, the table is full
		{
			retval.error_code = NOT_IMPLEMENTED;
		}else
		{
			//====================
			//  Create a Player
			//====================

			HoldemArena * myTable = reinterpret_cast<HoldemArena *>(add_to_table.table_ptr);

			playernumber_t pIndex = myTable->AddHumanOpponent(playerName, money);

			retval.player.player_type = ' ';
			retval.player.seat_number = pIndex;
			//The only field that is always populated for a valid player is seat_number (refB)
			add_to_table.seats_array[pIndex] = retval.player;		
		}
	}

	return retval;
}


C_DLL_FUNCTION
struct return_player CreateNewStrategyBot(struct holdem_table add_to_table, char *playerName, float64 money, char botType)
{
	struct return_player retval = DEFAULT_RETURN_PLAYER;

	if( !add_to_table.table_ptr )
	{
		retval.error_code = NULL_TABLE_PTR;
	}else if( !playerName || add_to_table.seat_count <= 0 )
	{
		retval.error_code = PARAMETER_DATA_ERROR;
	}else
	{
		playernumber_t final_seat = add_to_table.seat_count - 1;
		if( add_to_table.seats_array[final_seat].seat_number != -1 ) //Already a player in the last seat, the table is full
		{
			retval.error_code = NOT_IMPLEMENTED;
		}else
		{
			//==================
			//  Create a Bot
			//==================

			HoldemArena * myTable = reinterpret_cast<HoldemArena *>(add_to_table.table_ptr);

			playernumber_t pIndex = myTable->AddStrategyBot(playerName, money, botType);

			if( pIndex == -1 )
			{
				retval.error_code = PARAMETER_INVALID;
			}else
			{
				retval.player.player_type = botType;
				retval.player.seat_number = pIndex;
				
				add_to_table.seats_array[pIndex] = retval.player;
			}
		}
	}

	return retval;

}



//
//Things we need to clean up here:
//  +  table_to_delete has a pointer to the table and a list of players
// (1) We need to delete the actual table
//  +  For each player in the list HoldemArena will delete the player and any playerstrategy it might have
// (2) We need to delete[] the list of players
//
//Note: If not every seat of the table is occupied, not all of the (2) will be required
//
C_DLL_FUNCTION 
enum return_status DeleteTableAndPlayers(struct holdem_table table_to_delete)
{
	enum return_status error_code = SUCCESS;

	if( !(table_to_delete.table_ptr) )
	{
		error_code = NULL_TABLE_PTR;
	}else
	{
		HoldemArena * table = reinterpret_cast<HoldemArena *>(table_to_delete.table_ptr);
		delete table; //(1) deleted

		delete [] table_to_delete.seats_array; //(2) deleted
	}

	return error_code;
}

/*****************************************************************************
	Initial setup and final destructor functions
	END
*****************************************************************************/


