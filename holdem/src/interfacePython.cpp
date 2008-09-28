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
#include <string>

using std::string;


/*****************************************************************************
	BEGIN
	Betting round accessors
*****************************************************************************/


///Get the amount of money playerNumber has in front of him
extern "C" float64 GetMoney(int8 playerNumber);
//TODO: If the player is all in, make sure this returns the correct value



//Override the amount of money playerNumber has in front of him
//void SetMoney(int8 playerNumber, float64 money);



///Get the amount of money playerNumber has bet so far this round
extern "C" float64 GetBet(int8 playerNumber);
///TODO: If the player is all in, make sure this returns the correct value



///Get the amount of money that is in the pot
extern "C" float64 GetPotSize();


///Get the amount of money that was in the pot at the BEGINNING of the current betting round
extern "C" float64 GetLastRoundPotsize();


///Get the size of the highest bet so far
extern "C" float64 GetBetToCall();


//Get the playerNumber of the player who's turn it is
extern "C" int8 WhoIsNext();

/*****************************************************************************
	Betting round accessors
	END
*****************************************************************************/





/*****************************************************************************
	BEGIN
	Event functions

Note: If AutoPlayGame() is called, bAutoMode is set to 1, you never
need to use any of the Start______() functions.
*****************************************************************************/
int8 bAutoMode = 0;

///Use this to start the game and set bAutoMode = 1; See the note above.
extern "C" void AutoPlayGame();


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
extern "C" void NewCommunityCard(char cardValue,char cardSuit);





///Call this when the betting begins
extern "C" void StartBetting();

///Call these functions when playerNumber Raises, Folds, or Calls
extern "C" void PlayerCalls(int8 playerNumber);
extern "C" void PlayerFolds(int8 playerNumber);
extern "C" void PlayerRaisesTo(int8 playerNumber, float64 amount);
extern "C" void PlayerRaisesBy(int8 playerNumber, float64 amount);
///Question: If a player doesn't call any of these, which is the default action?


///GetBetAmount is useful for asking a bot what to bet
extern "C" float64 GetBetAmount(int8 playerNumber);

///GetAction returns a string descfribing what playerNumber wants to do in this situation
///This is particularly useful for bots
extern "C" string GetAction(int8 playerNumber);





///Call this when (if) the showdown begins
extern "C" void StartShowdown();

///Call this for each card playerNumber reveals during the showdown
///See NewCommunityCard for usage of cardValue and cardSuit
extern "C" void PlayerShowsCard(int8 playerNumber, char cardValue, char cardSuit);

///Call this when playerNumber mucks his/her hand during the showdown.
///Note: If a player doesn't PlayerShowsCard() then a muck is assumed
extern "C" void PlayerMucksHand(int8 playerNumber);





///Call this when new hands are dealt
extern "C" void StartDealNewHands();

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

///Choose playerNumber to be the dealer for the first hand
extern "C" void InitChooseDealer(int8 playerNumber);

///Set the amount of money that the SMALLEST chip is worth
extern "C" void InitSmallestChipSize(float64 money);

///Call this when the big blind has changed
extern "C" void SetBigBlind();

///Call this when the small blind has changed
extern "C" void SetSmallBlind();

///Add a player to the table. PLAYERS MUST BE ADDED IN CLOCKWISE ORDER.
///The function returns a playerNumber to identify this player in your code
extern "C" int8 AddHumanOpponent(string playerName);
extern "C" int8 AddStrategyBot(string playerName, char botType);


/*****************************************************************************
	Initial setup functions
	END
*****************************************************************************/


