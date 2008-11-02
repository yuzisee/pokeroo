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

//http://www.flipcode.com/archives/Creating_And_Using_DLLs.shtml
//http://www.parashift.com/c++-faq-lite/mixing-c-and-cpp.html
//http://sig9.com/node/35
//http://www.flounder.com/ultimateheaderfile.htm

#include "../src/portability.h"



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

struct holdem_player
{
	void * pstrat_ptr;
	void ** pstrat_children;
	playernumber_t seat_number;
}
;


struct holdem_table
{
	void * table_ptr;
	struct holdem_player * seats_array;
	playernumber_t seat_count;
}
;


enum return_status {

//OK
   SUCCESS,
   INPUT_CLEANED,

//FAIL
   NULL_TABLE_PTR,
   NOT_IMPLEMENTED,
   PARAMETER_INVALID,
   PARAMETER_DATA_ERROR,
   INTERNAL_INCONSISTENCY
};




struct return_money
{
	float64 money;
	enum return_status error_code;
}
;

struct return_event
{
	void * event_ptr;
	enum return_status error_code;
}
;

struct return_player
{
	struct holdem_player player;
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

///Get the amount of money playerNumber has in front of him
C_DLL_FUNCTION
struct return_money GetMoney(void * table_ptr, int8 playerNumber);
//TODO: If the player is all in, make sure this returns the correct value



//Override the amount of money playerNumber has in front of him
C_DLL_FUNCTION
enum return_status SetMoney(void * table_ptr, int8 playerNumber, float64 money);



///Get the amount of money playerNumber has bet so far this round
C_DLL_FUNCTION
struct return_money GetCurrentBet(void * table_ptr, int8 playerNumber);
///TODO: If the player is all in, make sure this returns the correct value



///Get the amount of money that is in the pot
C_DLL_FUNCTION
struct return_money GetPotSize(void * table_ptr);


///Get the amount of money that was in the pot at the BEGINNING of the current betting round
C_DLL_FUNCTION
struct return_money GetPrevRoundsPotsize(void * table_ptr);


///Get the size of the highest bet so far
C_DLL_FUNCTION
struct return_money GetBetToCall(void * table_ptr);

///Get the size of the highest bet so far
C_DLL_FUNCTION
struct return_money GetBetToCall(void * table_ptr);

//Get the playerNumber of the player who's turn it is
C_DLL_FUNCTION int8 WhoIsNext_Betting(void * table_ptr);

C_DLL_FUNCTION int8 WhoIsNext_Showdown(void * table_ptr);

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

C_DLL_FUNCTION struct holdem_cardset CreateNewCardset();

///For example, for the eight of hearts: cardValue = '8' and cardSuit = 'H'
C_DLL_FUNCTION enum return_status AppendCard(struct holdem_cardset * c, char cardValue,char cardSuit);

/*****************************************************************************
	END
	Card functions
*****************************************************************************/



/*****************************************************************************
	BEGIN
	Event functions
*****************************************************************************/





///Call this when the betting begins
C_DLL_FUNCTION enum return_status StartBetting(void * table_ptr, struct holdem_cardset community );

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
C_DLL_FUNCTION
struct return_table CreateNewTable(playernumber_t seatsAtTable, float64 chipDenomination);

C_DLL_FUNCTION
enum return_status DeleteTableAndPlayers(struct holdem_table table_to_delete);

///Choose playerNumber to be the dealer for the first hand
C_DLL_FUNCTION
enum return_status InitChooseDealer(void * table_ptr, int8 playerNumber);

///Set the amount of money that the SMALLEST chip is worth
C_DLL_FUNCTION
enum return_status InitSmallestChipSize(void * table_ptr, float64 money);

///Call this when the big blind has changed
C_DLL_FUNCTION
enum return_status SetBigBlind(void * table_ptr, float64 money);

///Call this when the small blind has changed
C_DLL_FUNCTION
enum return_status SetSmallBlind(void * table_ptr, float64 money);

///Add a player to the table. PLAYERS MUST BE ADDED IN CLOCKWISE ORDER
C_DLL_FUNCTION
struct return_player AddHumanOpponent(void * table_ptr, char * playerName);

///Add a bot to the table. PLAYERS MUST BE ADDED IN CLOCKWISE ORDER.
C_DLL_FUNCTION
struct return_player AddStrategyBot(void * table_ptr, char *playerName, char botType);







#endif // HOLDEM_HeaderDLL


