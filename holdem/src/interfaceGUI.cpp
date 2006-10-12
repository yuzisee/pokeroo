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
float64 GetMoney(int8 playerNumber);
//TODO: If the player is all in, make sure this returns the correct value



//Override the amount of money playerNumber has in front of him
//void SetMoney(int8 playerNumber, float64 money);



///Get the amount of money playerNumber has bet so far this round
float64 GetBet(int8 playerNumber);
///TODO: If the player is all in, make sure this returns the correct value



///Get the amount of money that is in the pot
float64 GetPotSize();


///Get the amount of money that was in the pot at the BEGINNING of the current betting round 
float64 GetLastRoundPotsize();


///Get the size of the highest bet so far
float64 GetBetToCall();


//Get the playerNumber of the player who's turn it is
int8 WhoIsNext();

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
void AutoPlayGame();


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
void NewCommunityCard(char cardValue,char cardSuit);





///Call this when the betting begins
void StartBetting();

///Call these functions when playerNumber Raises, Folds, or Calls
void PlayerCalls(int8 playerNumber);
void PlayerFolds(int8 playerNumber);
void PlayerRaisesTo(int8 playerNumber, float64 amount);
void PlayerRaisesBy(int8 playerNumber, float64 amount);
///Question: If a player doesn't call any of these, which is the default action?


///GetBetAmount is useful for asking a bot what to bet
float64 GetBetAmount(int8 playerNumber);

///GetAction returns a string descfribing what playerNumber wants to do in this situation
///This is particularly useful for bots
string GetAction(int8 playerNumber);





///Call this when (if) the showdown begins
void StartShowdown();

///Call this for each card playerNumber reveals during the showdown
///See NewCommunityCard for usage of cardValue and cardSuit
void PlayerShowsCard(int8 playerNumber, char cardValue, char cardSuit);

///Call this when playerNumber mucks his/her hand during the showdown.
///Note: If a player doesn't PlayerShowsCard() then a muck is assumed
void PlayerMucksHand(int8 playerNumber);





///Call this when new hands are dealt
void StartDealNewHands();

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
void InitChooseDealer(int8 playerNumber);

///Set the amount of money that the SMALLEST chip is worth
void InitSmallestChipSize(float64 money);

///Call this when the big blind has changed
void SetBigBlind();

///Call this when the small blind has changed
void SetSmallBlind();

///Add a player to the table. PLAYERS MUST BE ADDED IN CLOCKWISE ORDER.
///The function returns a playerNumber to identify this player in your code
int8 AddHumanOpponent(string playerName, float64 initialMoney);
int8 AddStrategyBot_ConservativeDefence(string playerName, float64 initialMoney);
int8 AddStrategyBot_GambleDefence(string playerName, float64 initialMoney);
int8 AddStrategyBot_ConservativeOffence(string playerName, float64 initialMoney);
int8 AddStrategyBot_GambleOffence(string playerName, float64 initialMoney);


/*****************************************************************************
	Initial setup functions
	END
*****************************************************************************/


