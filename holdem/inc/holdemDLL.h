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

#include "portability.h"

#include "arena.h"


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




//=========================
//   List your functions
//=========================


///Call this to determine if the big blind has changed
void GetBigBlind(void * table_ptr)
{}

///Call this to determine if the small blind has changed
void GetSmallBlind(void * table_ptr)
{}


///Get the amount of money playerNumber has in front of him
C_DLL_FUNCTION
float64 GetMoney(void * table_ptr, int8 playerNumber);
//TODO: If the player is all in, make sure this returns the correct value



//Override the amount of money playerNumber has in front of him
C_DLL_FUNCTION
void SetMoney(void * table_ptr, int8 playerNumber, float64 money);



///Get the amount of money playerNumber has bet so far this round
C_DLL_FUNCTION
float64 GetCurrentBet(void * table_ptr, int8 playerNumber);
///TODO: If the player is all in, make sure this returns the correct value



///Get the amount of money that is in the pot
C_DLL_FUNCTION
float64 GetPotSize(void * table_ptr);


///Get the amount of money that was in the pot at the BEGINNING of the current betting round
C_DLL_FUNCTION
float64 GetLastRoundPotsize(void * table_ptr);


///Get the size of the highest bet so far
C_DLL_FUNCTION
float64 GetBetToCall(void * table_ptr);


//Get the playerNumber of the player who's turn it is
C_DLL_FUNCTION int8 WhoIsNext_Betting(void * table_ptr);

C_DLL_FUNCTION int8 WhoIsNext_Showdown(void * table_ptr);

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
C_DLL_FUNCTION
void NewCommunityCard(void * table_ptr, char cardValue,char cardSuit);





///Call this when the betting begins
C_DLL_FUNCTION
void StartBetting(void * table_ptr);

///Call these functions when playerNumber Raises, Folds, or Calls
C_DLL_FUNCTION void PlayerCalls(void * table_ptr, int8 playerNumber);

C_DLL_FUNCTION void PlayerFolds(void * table_ptr, int8 playerNumber);

C_DLL_FUNCTION void PlayerRaisesTo(void * table_ptr, int8 playerNumber, float64 amount);

C_DLL_FUNCTION void PlayerRaisesBy(void * table_ptr, int8 playerNumber, float64 amount);
///Question: If a player doesn't call any of these, which is the default action?


///GetBetAmount is useful for asking a bot what to bet
C_DLL_FUNCTION float64 GetBetDecision(void * table_ptr, int8 playerNumber);



///Call this when (if) the showdown begins
C_DLL_FUNCTION void StartShowdown(void * table_ptr);

///Call this for each card playerNumber reveals during the showdown
///See NewCommunityCard for usage of cardValue and cardSuit
C_DLL_FUNCTION void PlayerShowsCard(void * table_ptr, int8 playerNumber, char cardValue, char cardSuit);

///Call this when playerNumber mucks his/her hand during the showdown.
///Note: If a player doesn't PlayerShowsCard() then a muck is assumed
C_DLL_FUNCTION void PlayerMucksHand(void * table_ptr, int8 playerNumber);





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
C_DLL_FUNCTION
void * NewTable();

C_DLL_FUNCTION
void DeleteTable(void * table_ptr);

///Choose playerNumber to be the dealer for the first hand
C_DLL_FUNCTION
void InitChooseDealer(void * table_ptr, int8 playerNumber);

///Set the amount of money that the SMALLEST chip is worth
C_DLL_FUNCTION
void InitSmallestChipSize(void * table_ptr, float64 money);

///Call this when the big blind has changed
C_DLL_FUNCTION
void SetBigBlind(void * table_ptr, float64 money);

///Call this when the small blind has changed
C_DLL_FUNCTION
void SetSmallBlind(void * table_ptr, float64 money);

///Add a player to the table. PLAYERS MUST BE ADDED IN CLOCKWISE ORDER.
///The function returns a playerNumber to identify this player in your code
C_DLL_FUNCTION
int8 AddHumanOpponent(void * table_ptr, char * playerName);

C_DLL_FUNCTION
int8 AddStrategyBot(void * table_ptr, char *playerName, char botType);







#endif // HOLDEM_HeaderDLL


