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


#include "../src/arena.h"
#include "../src/stratCombined.h"
#include "../src/stratPosition.h"

#include "holdemDLL.h"

#define NUMBER_OF_BOTS_COMBINED 6

typedef PositionalStrategy * stratPtr;


struct return_money DEFAULT_RETURN_MONEY = { 0.0 , SUCCESS };
struct return_event DEFAULT_RETURN_EVENT = { 0 , SUCCESS };
struct return_player DEFAULT_RETURN_PLAYER = { {0,0,-1} , SUCCESS };
struct return_table DEFAULT_RETURN_TABLE = { {0,0,-1} , SUCCESS };


/*****************************************************************************
	BEGIN
	Betting round accessors
*****************************************************************************/


///Get the amount of money playerNumber has in front of him
C_DLL_FUNCTION
struct return_money GetMoney(void * table_ptr, int8 playerNumber)
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
struct return_money GetCurrentBet(void * table_ptr, int8 playerNumber)
{

}
///TODO: If the player is all in, make sure this returns the correct value



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


//Get the playerNumber of the player who's turn it is
int8 WhoIsNext(void * table_ptr);

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
	Event functions
*****************************************************************************/




///Call this when the betting begins
C_DLL_FUNCTION struct return_event CreateNewBettingRound(void * table_ptr, struct holdem_cardset community )
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


/*
    while(b.bBetState == 'b')
    {
        b.MakeBet(p[curIndex]->myStrat->MakeBet());
    }

    return b.playerCalled;
*/

///Call these functions when playerNumber Raises, Folds, or Calls
C_DLL_FUNCTION enum return_status PlayerCalls(void * table_ptr, int8 playerNumber)
{
	/*
	enum return_status error_code = SUCCESS;

	if( !table_ptr )
	{
		error_code = NULL_TABLE_PTR;
	}else
	{
		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(table_ptr);

		const Player * ptrP = myTable->ViewPlayer(curIndex);

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

			if( myTable->OverridePlayerMoney(playerNumber,newMoney) )
			{
				error_code = INTERNAL_INCONSISTENCY;
			}
		}
	}

	return error_code;
	*/
}
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
struct return_table CreateNewTable(playernumber_t seatsAtTable, float64 chipDenomination)
{
	struct return_table retval = DEFAULT_RETURN_TABLE;

	if( seatsAtTable != SEATS_AT_TABLE )
	{
		retval.error_code = NOT_IMPLEMENTED;
	}else
	{
		bool illustrate = true;
		bool spectate = true;
		bool externalDealer = true;

		retval.table.table_ptr = reinterpret_cast<void *>(new HoldemArena(chipDenomination, std::cout ,illustrate,spectate));
		retval.table.players_array = new struct holdem_player[seatsAtTable];
		retval.table.seat_count = seatsAtTable;

		for(playernumber_t i=0;i<seatsAtTable;++i)
		{
			retval.table.players_array[i].seat_number = -1; //Mark as empty
		}
	}

	return retval;
	
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
C_DLL_FUNCTION
struct holdem_player CreateNewHumanOpponent(void * table_ptr, char * playerName)
{
    HoldemArena * table = reinterpret_cast<HoldemArena *>(table_ptr);
    //table->AddHuman(playerName, money,
    //p[newID]->bSync = true;

}


//Player objects are allocated and freed by the HoldemArena class
//PlayerStrategy objects (only exist for bots) must be allocated and freed here.
C_DLL_FUNCTION
struct return_player CreateNewStrategyBot(struct holdem_table add_to_table, char *playerName, float64 money, char botType)
{
	struct return_player retval = DEFAULT_RETURN_PLAYER;

	if( !add_to_table.table_ptr )
	{
		retval.error_code = NULL_TABLE_PTR;
	}else if( !playerName )
	{
		retval.error_code = PARAMETER_DATA_ERROR;
	}else if( add_to_table.players_array[add_to_table.seat_count-1].seat_number != -1 ) //Already a player in the last seat
	{
		retval.error_code = NOT_IMPLEMENTED;
	}else
	{
		HoldemArena * myTable = reinterpret_cast<HoldemArena *>(add_to_table.table_ptr);

		PlayerStrategy * botStrat = 0;
		PositionalStrategy **children = 0;
		switch( botType )
		{
		case 'A':
			botStrat = new ImproveGainStrategy(2);
			break;
		case 'C':
			botStrat = new DeterredGainStrategy();
			break;
		case 'D':
			botStrat = new DeterredGainStrategy(1);
			break;
		case 'N':
			botStrat = new ImproveGainStrategy(0);
			break;
		case 'S':
			botStrat = new DeterredGainStrategy(2);
			break;
		case 'T':
			botStrat = new ImproveGainStrategy(1);
			break;
		case 'G':
		case 'M':
			children = new stratPtr[NUMBER_OF_BOTS_COMBINED];
			
			children[0] = new ImproveGainStrategy(0); //Norm
			children[1] = new ImproveGainStrategy(1); //Trap
			children[2] = new ImproveGainStrategy(2); //Action
			children[3] = new DeterredGainStrategy(); //Com
			children[4] = new DeterredGainStrategy(1);//Danger
			children[5] = new DeterredGainStrategy(2);//Space

			{
				MultiStrategy * combined = new MultiStrategy(children,NUMBER_OF_BOTS_COMBINED);
				if( botType == 'M' ) combined->bGamble = 0;
				if( botType == 'G' ) combined->bGamble = 1;
				
				botStrat = combined;
			}

			break;
		default:
			botStrat = 0;
			break;
		}

		if( !botStrat )
		{
			retval.error_code = PARAMETER_INVALID;
		}else
		{
			playernumber_t pIndex = myTable->AddPlayer(playerName, money, botStrat);

			retval.player.pstrat_children = reinterpret_cast<void **>(children);

			retval.player.pstrat_ptr = reinterpret_cast<void *>(botStrat);
			retval.player.seat_number = pIndex;
			
			add_to_table.players_array[pIndex] = retval.player;
		}
	}

	return retval;

}

C_DLL_FUNCTION 
enum return_status DeleteTable(struct holdem_table table_to_delete)
{
	enum return_status error_code = SUCCESS;

	if( !(table_to_delete.table_ptr) )
	{
		error_code = NULL_TABLE_PTR;
	}else
	{
		HoldemArena * table = reinterpret_cast<HoldemArena *>(table_to_delete.table_ptr);
		delete table;

		for(playernumber_t i=0;i<table_to_delete.seat_count;++i)
		{
			void ** children = table_to_delete.players_array[i].pstrat_children;
			void * strat = table_to_delete.players_array[i].pstrat_ptr;

			if( strat )
			{
				PlayerStrategy * pStrat = reinterpret_cast<PlayerStrategy *>(strat);
				if( !children )
				{
					delete pStrat;
				}else
				{
					PositionalStrategy ** mChildren = reinterpret_cast<PositionalStrategy **>(children);

					MultiStrategy * mStrat = dynamic_cast<MultiStrategy *>(pStrat);
					delete mStrat;

					for(playernumber_t n=0;n<NUMBER_OF_BOTS_COMBINED;++n)
					{
						delete mChildren[n];
					}

					delete [] mChildren;
				}
			}
			table_to_delete.players_array[i].seat_number = -1;
		}
		delete [] table_to_delete.players_array;
	}

	return error_code;
}

/*****************************************************************************
	Initial setup functions
	END
*****************************************************************************/


