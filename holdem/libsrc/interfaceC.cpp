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

#include <sstream>
#include <string.h>

#include "../src/arena.h"
#include "../src/stratCombined.h"
#include "../src/stratPosition.h"

#include "holdemDLL.h"


struct return_money DEFAULT_RETURN_MONEY = { 0.0 , SUCCESS };
struct return_seat DEFAULT_RETURN_SEAT = { -1, SUCCESS };
struct return_event DEFAULT_RETURN_EVENT = { 0 , SUCCESS };
struct return_table DEFAULT_RETURN_TABLE = { {0,-1} , SUCCESS };



/*****************************************************************************
	BEGIN
	Betting round accessors
*****************************************************************************/

C_DLL_FUNCTION handnum_t GetHandnum(void * table_ptr)
{
	if( !table_ptr )
	{
		return 0;
	}else
	{
		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(table_ptr);

		return myTable->handnum;
	}
}

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

///Get the size of the highest bet so far this round
C_DLL_FUNCTION
struct return_money GetMinRaise(void * table_ptr)
{
	struct return_money retval = DEFAULT_RETURN_MONEY;

	if( !table_ptr )
	{
		retval.error_code = NULL_TABLE_PTR;
	}else
	{
		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(table_ptr);

		retval.money = myTable->GetMinRaise();
	}

	return retval;
}




/*****************************************************************************
	Betting round accessors
	END
*****************************************************************************/




/*****************************************************************************
	BEGIN	
	Flow control functions
*****************************************************************************/




C_DLL_FUNCTION
enum return_status ShowHoleCardsToBot(void * table_ptr, playernumber_t playerNumber, struct holdem_cardset holecards)
{

	enum return_status error_code = SUCCESS;

	if( !table_ptr )
	{
		error_code = NULL_TABLE_PTR;
	}else if( !holecards.card_count )
	{
		error_code = PARAMETER_DATA_ERROR;
	}else
	{
		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(table_ptr);

		CommunityPlus * myHand = reinterpret_cast<CommunityPlus *>(holecards.cards_ptr);

		if( !myTable->IsAlive( playerNumber ) )
		{
			error_code = PARAMETER_INVALID;
		}else
		{
			const Player * myPlayer = myTable->ViewPlayer(playerNumber);
			myTable->ShowHoleCards( *myPlayer , *myHand );
		}

	}

	return error_code;
}


//Use overrideDealer == -1 and you won't override the dealer
C_DLL_FUNCTION
enum return_status BeginNewHands(void * table_ptr, float64 smallBlind, playernumber_t overrideDealer)
{

	enum return_status error_code = SUCCESS;

	if( !table_ptr )
	{
		error_code = NULL_TABLE_PTR;
	}else
	{
		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(table_ptr);

		BlindValues b;
		b.SetSmallBigBlind(smallBlind);

		bool bNewBlindValues = false; //bNewBlindValues is always false when calling through DLL because notification of blind changes is redundant.
		if( myTable->IsAlive(overrideDealer) )
		{
			myTable->BeginNewHands(b,bNewBlindValues,overrideDealer);
		}
		else
		{
			if( overrideDealer >= 0 )
			{
				error_code = PARAMETER_INVALID;
			}else
			{
				if( overrideDealer != -1 ) error_code = INPUT_CLEANED;
				
				myTable->BeginNewHands(b,bNewBlindValues);
			}
		}
		
		 
	}

	return error_code;
}

C_DLL_FUNCTION enum return_status FinishHandRefreshPlayers(void * table_ptr)
{
	enum return_status error_code = SUCCESS;

	if( !table_ptr )
	{
		error_code = NULL_TABLE_PTR;
	}else
	{
		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(table_ptr);

		myTable->RefreshPlayers();
	}

	return error_code;
}

//Serializes the table state into a single string.
//Actions performed during betting events and showdown events will not be saved properly.
//The new string is allocated with malloc(), free this buffer when you no longer need it
C_DLL_FUNCTION
struct return_string SaveTableState(void * table_ptr)
{
	
	struct return_string retval;
	retval.error_code = SUCCESS;
	retval.str = 0;

	if( !table_ptr )
	{
		retval.error_code = NULL_TABLE_PTR;
	}else
	{
		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(table_ptr);

		std::ostringstream strBufState;

		myTable->SerializeRoundStart(strBufState);

		retval.str = (char *)(malloc(strBufState.str().length()+1));
		if( retval.str )
		{
			strcpy( retval.str , strBufState.str().c_str() );
		}
		else
		{
			retval.error_code = OUT_OF_MEMORY;
		}
	}

	return retval;
}



//Unserializes the table state from a single string.
//Players and PlayerStrategies will be automatically added back into their correct seats.
//Actions performed during betting events and showdown events will not be restored properly.
C_DLL_FUNCTION
enum return_status RestoreTableState(const char * state_str, void * table_ptr)
{
	
	enum return_status error_code = SUCCESS;

	if( !table_ptr )
	{
		error_code = NULL_TABLE_PTR;
	}else if( !state_str )
	{
		error_code = PARAMETER_DATA_ERROR;
	}else
	{
		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(table_ptr);

		std::string strState(state_str);
		std::istringstream strBufState(strState);

		myTable->UnserializeRoundStart(strBufState);

		//InitGameLoop calls:
		//empty function, after all... myTable->LoadBeginInitialState(); //opens some logfile handles
		myTable->ResetDRseed();
	}

	return error_code;
}

C_DLL_FUNCTION
enum return_status InitializeNewTableState(void * table_ptr, handnum_t game_id)
{

	enum return_status error_code = SUCCESS;

	if( !table_ptr )
	{
		error_code = NULL_TABLE_PTR;
	}else
	{
		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(table_ptr);

		//InitGameLoop calls:
		myTable->BeginInitialState(game_id); //sets handnum=1, opens logfile handles,
		myTable->ResetDRseed();
	}

	return error_code;
}

/*****************************************************************************
	Flow control functions
	END
*****************************************************************************/




/*****************************************************************************
	BEGIN
	Card functions
*****************************************************************************/

C_DLL_FUNCTION enum return_status ResetDeterministicSeed(void * table_ptr)
{
	enum return_status error_code = SUCCESS;

	if( !table_ptr )
	{
		error_code = NULL_TABLE_PTR;
	}else
	{
		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(table_ptr);

		myTable->ResetDRseed();
	}

	return error_code;
}

C_DLL_FUNCTION uint32 GetDeterministicSeed(void * table_ptr, uint8 small_int)
{
	if( !table_ptr )
	{
		//retval.error_code = NULL_TABLE_PTR;
		return 0;
	}else
	{
		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(table_ptr);

		//retval.money = myTable->GetDRseed();
		
		return RandomDeck::Float64ToUint32Seed(small_int,myTable->GetDRseed());
	}

	//return retval;

}

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
struct return_cardset AppendCard(struct holdem_cardset c, char cardValue,char cardSuit)
{
	struct return_cardset retval;
	retval.cardset = c;
	retval.error_code = SUCCESS;

	if( !(c.cards_ptr) )
	{
		retval.error_code = PARAMETER_DATA_ERROR;
	}else
	{
		CommunityPlus * myHand = reinterpret_cast<CommunityPlus *>(c.cards_ptr);
		
		int8 newCardIndex = HoldemUtil::ParseCard(cardValue,cardSuit);
		if( newCardIndex < 0 )
		{
			retval.error_code = PARAMETER_INVALID;
		}else
		{
			DeckLocation newCard;
			newCard.SetByIndex(newCardIndex);
			
			myHand->AddToHand( newCard );
			retval.cardset.card_count = c.card_count + 1;
		}
	}

	return retval;
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
struct return_event CreateNewBettingRound(void * table_ptr, struct holdem_cardset community, enum betting_round bFirstBettingRound , uint8 numFutureBettingRounds )
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

		myTable->PrepBettingRound(bFirstBettingRound == FIRST_BETTING_ROUND, numFutureBettingRounds);

		HoldemArenaBetting * bettingEvent = new HoldemArenaBetting( myTable, *myHand, community.card_count );

		if( bettingEvent )
		{
			retval.event_ptr = reinterpret_cast<void *>(bettingEvent);
		}
		else
		{
			retval.error_code = OUT_OF_MEMORY;
		}
		
		
	}

	return retval;
}


C_DLL_FUNCTION
struct return_seat DeleteFinishBettingRound(void * event_ptr)
{
	struct return_seat retval = DEFAULT_RETURN_SEAT;

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
//Returns -1 once the round has ended
C_DLL_FUNCTION
struct return_seat WhoIsNext_Betting(void * event_ptr)
{
	struct return_seat retval = DEFAULT_RETURN_SEAT;

	if( !event_ptr )
	{
		retval.error_code = PARAMETER_DATA_ERROR;
	}else
	{
		HoldemArenaBetting * bettingEvent = reinterpret_cast<HoldemArenaBetting *>(event_ptr);

		if( bettingEvent->bBetState != 'b' ) //if the round has already finished,
		{
			retval.seat_number = -1; 
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
C_DLL_FUNCTION struct return_money GetBetDecision(void * table_ptr, playernumber_t playerNumber)
{
	struct return_money retval = DEFAULT_RETURN_MONEY;

	if( !table_ptr )
	{
		retval.error_code = NULL_TABLE_PTR;
	}else
	{
		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(table_ptr);
		char t = myTable->GetPlayerBotType(playerNumber);
		if( !t )
		{
			retval.error_code = PARAMETER_DATA_ERROR;
		}else if( t == '~' || t == ' ' )
		{
			retval.error_code = PARAMETER_INVALID;
		}else
		{
			//If you're asking a bot what it would bet even though there are
			// players ahead of it that haven't bet yet.
			//I don't know if the more complicated bots can handle being out of turn.
			if( myTable->GetCurPlayer() != playerNumber )
			{
				retval.error_code = UNRELIABLE_RESULT;
			}
			else
			{
				retval.money = myTable->GetBetDecision(playerNumber);
			}
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



///Call this when the showdown begins
C_DLL_FUNCTION
struct return_event CreateNewShowdown(void * table_ptr, playernumber_t calledPlayer, struct holdem_cardset final_community)
{
	struct return_event retval = DEFAULT_RETURN_EVENT;

	if( !table_ptr )
	{
		retval.error_code = NULL_TABLE_PTR;
	}else if( !final_community.cards_ptr )
	{
		retval.error_code = PARAMETER_DATA_ERROR;
	}else
	{
		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(table_ptr);

		CommunityPlus * myCommunity = reinterpret_cast<CommunityPlus *>(final_community.cards_ptr);

		myTable->PrepShowdownRound(*myCommunity);

		HoldemArenaShowdown * bettingEvent = new HoldemArenaShowdown( myTable, calledPlayer );

		if( bettingEvent )
		{
			retval.event_ptr = reinterpret_cast<void *>(bettingEvent);
		}
		else
		{
			retval.error_code = OUT_OF_MEMORY;
		}
		
	}

	return retval;
}



C_DLL_FUNCTION
enum return_status DeleteFinishShowdown(void * table_ptr, void * event_ptr)
{
	enum return_status error_code = SUCCESS;

	if( !table_ptr )
	{
		error_code = NULL_TABLE_PTR;
	}else if( !event_ptr )
	{
		error_code = PARAMETER_DATA_ERROR;
	}else
	{
		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(table_ptr);

		HoldemArenaShowdown * showdownEvent = reinterpret_cast<HoldemArenaShowdown *>(event_ptr);

		//If we're in the middle of a showdown that hasn't concluded,
		// we will report an error but still delete bettingEvent anyways.
		//It's possible that the user could be quitting the program and needs
		// to delete things in the middle of a betting round.
		if( showdownEvent->bRoundState != '!' ) error_code = UNRELIABLE_RESULT;

		myTable->ProcessShowdownResults(showdownEvent->winners);

		delete showdownEvent;
	}

	return error_code;
}



//Get the seat of the player who's turn it is.
//Players who aren't all-in first reveal/muck in order.
//Then players who are all-in show their cards starting with the largest stack.
//Returns -1 once the round has ended
C_DLL_FUNCTION
struct return_seat WhoIsNext_Showdown(void * event_ptr)
{
	struct return_seat retval = DEFAULT_RETURN_SEAT;

	if( !event_ptr )
	{
		retval.error_code = PARAMETER_DATA_ERROR;
	}else
	{
		HoldemArenaShowdown * showdownEvent = reinterpret_cast<HoldemArenaShowdown *>(event_ptr);

		if( showdownEvent->bRoundState == '!' ) //if the round has already finished,
		{
			retval.seat_number = -1; 
		}else
		{
			retval.seat_number = showdownEvent->WhoIsNext();
		}
	}

	return retval;
}


///Call this for each card playerNumber reveals during the showdown
///See NewCommunityCard for usage of cardValue and cardSuit
C_DLL_FUNCTION
enum return_status PlayerShowsHand(void * event_ptr, playernumber_t playerNumber, struct holdem_cardset playerHand, struct holdem_cardset community)
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
C_DLL_FUNCTION
enum return_status PlayerMucksHand(void * event_ptr, playernumber_t playerNumber)
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

		HoldemArena * newTable = new HoldemArena(chipDenomination, std::cout ,illustrate,spectate);
		
		if( newTable )
		{
			retval.table.table_ptr = reinterpret_cast<void *>(newTable);
			retval.table.seat_count = seatsAtTable;
		}
		else
		{
			retval.error_code = OUT_OF_MEMORY;
		}
	}

	return retval;
	
}


C_DLL_FUNCTION
struct return_seat CreateNewHumanOpponent(struct holdem_table add_to_table, const char * playerName, float64 money)
{
	//A human opponent is treated the same as a bot, except it has no strat (and therefore no children)
	struct return_seat retval = DEFAULT_RETURN_SEAT;

	if( !add_to_table.table_ptr )
	{
		retval.error_code = NULL_TABLE_PTR;
	}else if( !playerName || add_to_table.seat_count <= 0 )
	{
		retval.error_code = PARAMETER_DATA_ERROR;
	}else
	{
		
		//====================
		//  Create a Player
		//====================

		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(add_to_table.table_ptr);

		playernumber_t pIndex = myTable->AddHumanOpponent(playerName, money);

		if( pIndex == -1 )
		{
			retval.error_code = NOT_IMPLEMENTED;
		}else
		{
			retval.seat_number = pIndex;
		}
		
	}

	return retval;
}


C_DLL_FUNCTION
struct return_seat CreateNewStrategyBot(struct holdem_table add_to_table, const char *playerName, float64 money, char botType)
{
	struct return_seat retval = DEFAULT_RETURN_SEAT;

	if( !add_to_table.table_ptr )
	{
		retval.error_code = NULL_TABLE_PTR;
	}else if( !playerName || add_to_table.seat_count <= 0 )
	{
		retval.error_code = PARAMETER_DATA_ERROR;
	}else
	{
		//==================
		//  Create a Bot
		//==================

		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(add_to_table.table_ptr);

		playernumber_t pIndex = myTable->AddStrategyBot(playerName, money, botType);

		if( pIndex == -1 )
		{
			retval.error_code = NOT_IMPLEMENTED;
		}else
		{
			retval.seat_number = pIndex;
		}
	}

	return retval;

}



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
		delete table;
	}

	return error_code;
}

/*****************************************************************************
	Initial setup and final destructor functions
	END
*****************************************************************************/


/*
        float64 GetFoldedPotSize() const;
        float64 GetUnfoldedPotSize() const;
		float64 GetDeadPotSize() const; //That's pot - betSum;
		float64 GetLivePotSize() const;
		//float64 GetRoundPotSize() const; //ThisRound pot size

		float64 GetPrevFoldedRetroactive() const;
   		float64 GetRoundBetsTotal() const; //Bets made this round by players still in hand, excludes blind bets
   		
		//float64 GetMaxShowdown(const float64 myMoney = -1) const;
		
		float64 GetUnbetBlindsTotal() const; //blindOnlySum
		
*/

