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




#ifndef HOLDEM_HeaderDLL
#define HOLDEM_HeaderDLL

#include "../src/portability.h"


//Predefined Macros: What architecture are we compiling for?
//http://predef.sourceforge.net/preos.html
//http://blogs.msdn.com/oldnewthing/archive/2006/09/06/742710.aspx

#if defined(_WIN32)

	//http://www.flipcode.com/archives/Creating_And_Using_DLLs.shtml
	//http://www.parashift.com/c++-faq-lite/mixing-c-and-cpp.html
	//http://sig9.com/node/35
	//http://www.flounder.com/ultimateheaderfile.htm

	#ifdef _WINDLL
	// Microsoft Visual Studio conveniently defines the _WINDLL define when you're in a project that's
	// building a DLL. The __declspec(dllexport) tells the compiler that this function is part of
	// the API exported by the DLL.

		/* DLL export */
		#define DLL_FUNCTION __declspec(dllexport)
	#else
	// In most cases, you will want to use this file as a header file.
	// The __declspec(dllimport) tells the compiler that the code for this function will NOT be
	// linked, and that it should be imported later from a DLL.

		/* EXE import */
		#define DLL_FUNCTION __declspec(dllimport)
	#endif
#elif defined(__linux)
	//In Linux your functions will just be included as any other object file, nothing special.
	#define DLL_FUNCTION
#else
	#error "Not yet implemented on your target operating system?"
#endif




#ifdef __cplusplus
// As part of the C++ specification, all compilers must define __cplusplus if they are compiling
// C++ code. The extern "C" means we are declaring functions callable by C code.

	#define C_FUNCTION extern "C"
#else
// If __cplusplus is not defined, you are including this header file for a C program somewhere.
// Your C compiler will just assume that these function prototypes refer to C functions anyways.
// As long as the functions were defined as extern "C" during the DLL compile, the DLL will
// contain C function prototypes, and this header file contains C function prototypes, and everything
// works out great.
	#define C_FUNCTION 
#endif // __cplusplus


// Sanity check: Our library is a C++ function, so if we're trying to build the DLL, we better be in __cplusplus
#ifdef _WINDLL
	#ifndef __cplusplus
		#error "Sanity check: Why are you trying to build this DLL with a C compiler?"
	#endif
#endif

//======================================================
//   Combine the DLL_FUNCTION and C_FUNCTION defines.
//======================================================
#define C_DLL_FUNCTION C_FUNCTION DLL_FUNCTION




//======================
//   Basic data types
//======================


struct holdem_cardset
{
	void * cards_ptr;
	int card_count;
}
;



struct holdem_table
{
	void * table_ptr;
	playernumber_t seat_count;
}
;


enum return_status {

//OK
   SUCCESS,
   INPUT_CLEANED,

//WARNING
   UNRELIABLE_RESULT,

//FAIL
	OUT_OF_MEMORY,
   NULL_TABLE_PTR,
   NOT_IMPLEMENTED,
   PARAMETER_INVALID,
   PARAMETER_DATA_ERROR,
   INTERNAL_INCONSISTENCY   
};


enum betting_round {
   LATER_BETTING_ROUND,
   FIRST_BETTING_ROUND
};



struct return_money
{
	float64 money;
	enum return_status error_code;
}
;

struct return_seat
{
	playernumber_t seat_number;
	enum return_status error_code;
}
;

struct return_cardset
{
	struct holdem_cardset cardset;
	enum return_status error_code;
}
;

struct return_string
{
	char * str;
	enum return_status error_code;
}
;

struct return_event
{
	void * event_ptr;
	enum return_status error_code;
}
;


struct return_table
{
	struct holdem_table table;
	enum return_status error_code;
}
;

//=========================
//   List your functions
//=========================



/*****************************************************************************
	BEGIN
	Betting round functions
*****************************************************************************/



///Get the amount of money playerNumber has in front of him
C_DLL_FUNCTION handnum_t GetHandnum(void * table_ptr);

///Get the amount of money playerNumber has in front of him
C_DLL_FUNCTION struct return_money GetMoney(void * table_ptr, playernumber_t);




///Override the amount of money playerNumber has in front of him
C_DLL_FUNCTION enum return_status SetMoney(void * table_ptr, playernumber_t, float64);



///Get the amount of money playerNumber has bet so far this round
C_DLL_FUNCTION struct return_money GetCurrentRoundBet(void * table_ptr, playernumber_t playerNumber);

///Get the amount of money playerNumber has bet so far in all previous rounds
C_DLL_FUNCTION struct return_money GetPrevRoundsBet(void * table_ptr, playernumber_t playerNumber);



///Get the amount of money that is in the pot
C_DLL_FUNCTION struct return_money GetPotSize(void * table_ptr);


///Get the amount of money that was in the pot at the BEGINNING of the current betting round
C_DLL_FUNCTION struct return_money GetPrevRoundsPotsize(void * table_ptr);


/*****************************************************************************
	Betting round functions
	END
*****************************************************************************/

/*****************************************************************************
	BEGIN	
	Flow control functions
*****************************************************************************/

/*

1. Create a New Table
2. Add Players and InitializeNew the TableState, or RestoreTableState
3. You may save the state of the table at this point, if you really want to.
4. -----
	4.1 BeginNewHand
	4.2 ShowHoleCards to all bots who need to know their hand
	4.3 If you would like to use the deterministic random seed generator, reset it here: ResetDeterministicSeed().
	4.4 -----
		 4.4.1a CreateNewCardset() a blank hand to represent the pre-flop and CreateNewBettingRound() with it
		 4.4.1b Make bets for WhoIsNext_Betting() with PlayerMakesBetTo(), while querying your bots through GetBetDecision()
		 4.4.1c DeleteFinishBettingRound() and it will report if the high bet was called.

		 4.4.2a If the preflop high bet was called, AppendCard() your flop to the holdem_cardset and then CreateNewBettingRound() with it
		 4.4.2b Make bets ...
		 4.4.2c DeleteFinishBettingRound() ...

		 4.4.3 ... If the postflop high-bet was called, play the Turn ... and it will report if the high bet is called.

		 4.4.4 ... If the high-bet was called after the turn, play the River ... and it will report if the high bet is called.

		 4.4.5a If the high-bet was called after the river, CreateNewShowdown()
		 4.4.5b Each WhoIsNext_Showdown can PlayerShowsHand() or PlayerMucksHand()
		 4.4.5c DeleteFinishShowdown() will calculate side pots and move money from the pot to the winners. Compare GetMoney() before and after to determine winners.

	4.5 Call FinishHandRefreshPlayers() to complete any final bookkeeping that needs to take place to prepare data structures for the next hand
	4.6 Here is a good time to shuffle the deck. Retrieve a deterministic seed with GetDeterministicSeed() if you would like.
	4.7 If you would like to save the game, now is the best time to do so. Call SaveTableState()
	4.8 [Go back to step 4.1]

*/


C_DLL_FUNCTION enum return_status RestoreTableState(const char * state_str, void * table_ptr);
C_DLL_FUNCTION enum return_status InitializeNewTableState(void * table_ptr);

C_DLL_FUNCTION enum return_status BeginNewHands(void * table_ptr, float64 smallBlind, playernumber_t overrideDealer);
C_DLL_FUNCTION enum return_status ShowHoleCardsToBot(void * table_ptr, playernumber_t , struct holdem_cardset );

C_DLL_FUNCTION enum return_status FinishHandRefreshPlayers(void * table_ptr);


//The new string is allocated with malloc(), free this buffer when you no longer need it
C_DLL_FUNCTION struct return_string SaveTableState(void * table_ptr);


C_DLL_FUNCTION enum return_status ResetDeterministicSeed(void * table_ptr);
C_DLL_FUNCTION uint32 GetDeterministicSeed(void * table_ptr, uint8 small_int); //small_int works pretty well as a number between 0 and 51


/*
Reference PlayGameInner is:
	if( my.PlayRound_BeginHand() == -1 ) return;

	CommunityPlus myFlop;
	my.RequestCards(tableDealer,3,myFlop, "Please enter the flop (no whitespace): ");
    if( my.PlayRound_Flop(myFlop) == -1 ) return;


	DeckLocation myTurn = my.RequestCard(tableDealer);
    if( my.PlayRound_Turn(myFlop,myTurn) == -1 ) return;

	DeckLocation myRiver = my.RequestCard(tableDealer);
    int8 playerToReveal = my.PlayRound_River(myFlop,myTurn,myRiver);
    if( playerToReveal == -1 ) return;


	CommunityPlus finalCommunity;
	finalCommunity.SetUnique(myFlop);
	finalCommunity.AddToHand(myTurn);
	finalCommunity.AddToHand(myRiver);


	my.PlayShowdown(finalCommunity,playerToReveal);
*/


/*****************************************************************************
	Flow control functions
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

C_DLL_FUNCTION struct holdem_cardset CreateNewCardset(void);

///For example, for the eight of hearts: cardValue = '8' and cardSuit = 'H'
C_DLL_FUNCTION struct return_cardset AppendCard(struct holdem_cardset c, char cardValue,char cardSuit);

C_DLL_FUNCTION enum return_status DeleteCardset(struct holdem_cardset c);

/*****************************************************************************
	END
	Card functions
*****************************************************************************/



/*****************************************************************************
	BEGIN
	Event functions
*****************************************************************************/

C_DLL_FUNCTION struct return_event CreateNewBettingRound(void * table_ptr, struct holdem_cardset, enum betting_round , uint8 );

//This function reports who made first high bet that was called
//If nobody called the high bet, then you will get -1 here.
//CreateNewShowdown will need this value to determine who
//is going to act first in the showdown.
C_DLL_FUNCTION struct return_seat DeleteFinishBettingRound(void * event_ptr);


///Call this function when playerNumber Raises, Folds, or Calls
C_DLL_FUNCTION enum return_status PlayerMakesBetTo(void * event_ptr, playernumber_t playerNumber, float64 money);



///GetBetAmount is useful for asking a bot what to bet
C_DLL_FUNCTION struct return_money GetBetDecision(void * table_ptr, playernumber_t playerNumber);




///Get the size of the highest bet so far
C_DLL_FUNCTION struct return_money GetBetToCall(void * table_ptr);
C_DLL_FUNCTION struct return_money GetMinRaise(void * table_ptr);



//Get the playerNumber of the player who's turn it is
C_DLL_FUNCTION struct return_seat WhoIsNext_Betting(void * event_ptr);

C_DLL_FUNCTION struct return_seat WhoIsNext_Showdown(void * event_ptr);


///Call this for each card playerNumber reveals during the showdown
///See NewCommunityCard for usage of cardValue and cardSuit
C_DLL_FUNCTION enum return_status PlayerShowsHand(void * event_ptr, playernumber_t playerNumber, struct holdem_cardset playerHand, struct holdem_cardset community);

///Call this when playerNumber mucks his/her hand during the showdown.
C_DLL_FUNCTION enum return_status PlayerMucksHand(void * event_ptr, playernumber_t playerNumber);

C_DLL_FUNCTION struct return_event CreateNewShowdown(void * table_ptr, playernumber_t calledPlayer, struct holdem_cardset final_community);
C_DLL_FUNCTION enum return_status DeleteFinishShowdown(void * table_ptr, void * event_ptr);


/*****************************************************************************
	Event functions
	END
*****************************************************************************/




/*****************************************************************************
	BEGIN
	Initial setup and final destructor functions
*****************************************************************************/
C_DLL_FUNCTION struct return_table CreateNewTable(playernumber_t seatsAtTable, float64 chipDenomination);

C_DLL_FUNCTION enum return_status DeleteTableAndPlayers(struct holdem_table table_to_delete);


///Add a player/bot to the table. PLAYERS MUST BE ADDED IN CLOCKWISE ORDER
//The first player added to the table (seat #0) will have the button in the first hand
C_DLL_FUNCTION struct return_seat CreateNewHumanOpponent(struct holdem_table add_to_table, const char * playerName, float64 money);
C_DLL_FUNCTION struct return_seat CreateNewStrategyBot(struct holdem_table add_to_table, const char *playerName, float64 money, char botType);


/*****************************************************************************
	Initial setup and final destructor functions
	END
*****************************************************************************/






#endif // HOLDEM_HeaderDLL


