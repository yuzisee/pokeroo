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

#include "Python.h"

typedef double float64;
typedef unsigned char int8;

//#include "portability.h"
//#include <string.h>

//using std::string;


/*****************************************************************************
	BEGIN
	Betting round accessors
*****************************************************************************/


///Get the amount of money playerNumber has in front of him
static PyObject * GetMoney (PyObject *self, PyObject *args) //float64 (int8 playerNumber);
{
    //TODO: If the player is all in, make sure this returns the correct value
}


//Override the amount of money playerNumber has in front of him
static PyObject * SetMoney (PyObject *self, PyObject *args) ////void (int8 playerNumber, float64 money);
{
}



///Get the amount of money playerNumber has bet so far this round
static PyObject * GetBet (PyObject *self, PyObject *args) //float64 (int8 playerNumber);
{
    //TODO: If the player is all in, make sure this returns the correct value
}




///Get the amount of money that is in the pot
static PyObject * GetPotSize (PyObject *self, PyObject *args) //float64 ();
{
}


///Get the amount of money that was in the pot at the BEGINNING of the current betting round
static PyObject * GetLastRoundPotsize (PyObject *self, PyObject *args) //float64 ();
{
}


///Get the size of the highest bet so far
static PyObject * GetBetToCall (PyObject *self, PyObject *args) //float64 ();
{
}



//Get the playerNumber of the player who's turn it is
static PyObject * WhoIsNext (PyObject *self, PyObject *args) //int8 ();
{
}


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
static PyObject * AutoPlayGame (PyObject *self, PyObject *args) //void ();
{
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
static PyObject * NewCommunityCard (PyObject *self, PyObject *args) //void (char cardValue,char cardSuit);
{
}





///Call this when the betting begins
static PyObject * StartBetting (PyObject *self, PyObject *args) //void ();
{
}


///Call these functions when playerNumber Raises, Folds, or Calls
static PyObject * PlayerCalls (PyObject *self, PyObject *args) //void (int8 playerNumber);
{}

static PyObject * PlayerFolds (PyObject *self, PyObject *args) //void (int8 playerNumber);
{}

static PyObject * PlayerRaisesTo (PyObject *self, PyObject *args) //void (int8 playerNumber, float64 amount);
{}

static PyObject * PlayerRaisesBy (PyObject *self, PyObject *args) //void (int8 playerNumber, float64 amount);
{}
///Question: If a player doesn't call any of these, which is the default action?



///GetBetAmount is useful for asking a bot what to bet
static PyObject * GetBetAmount (PyObject *self, PyObject *args) //float64 (int8 playerNumber);
{
}

///GetAction returns a string descfribing what playerNumber wants to do in this situation
///This is particularly useful for bots
static PyObject * GetAction (PyObject *self, PyObject *args) //string (int8 playerNumber);
{
}



///Call this when (if) the showdown begins
static PyObject * StartShowdown (PyObject *self, PyObject *args) //void ();
{
}

///Call this for each card playerNumber reveals during the showdown
///See NewCommunityCard for usage of cardValue and cardSuit
static PyObject * PlayerShowsCard (PyObject *self, PyObject *args) //void (int8 playerNumber, char cardValue, char cardSuit);
{
}

///Call this when playerNumber mucks his/her hand during the showdown.
///Note: If a player doesn't PlayerShowsCard() then a muck is assumed
static PyObject * PlayerMucksHand (PyObject *self, PyObject *args) //void (int8 playerNumber);
{
}



///Call this when new hands are dealt
static PyObject * StartDealNewHands (PyObject *self, PyObject *args) //void ();
{
}

/*****************************************************************************
	Event functions
	END
*****************************************************************************/




/*****************************************************************************
	BEGIN
	Initial setup functions

static PyObject * SetBigBlind (PyObject *self, PyObject *args) //Note: () and SetSmallBlind() can be called between
hands anytime the blind size changes during the game
*****************************************************************************/

///Choose playerNumber to be the dealer for the first hand
static PyObject * InitChooseDealer (PyObject *self, PyObject *args) //void (int8 playerNumber);
{
}

///Set the amount of money that the SMALLEST chip is worth
static PyObject * InitSmallestChipSize (PyObject *self, PyObject *args) //void (float64 money);
{
}

///Call this when the big blind has changed
static PyObject * SetBigBlind (PyObject *self, PyObject *args) //void ();
{
}

///Call this when the small blind has changed
static PyObject * SetSmallBlind (PyObject *self, PyObject *args) //void ();
{
}

///Add a player to the table. PLAYERS MUST BE ADDED IN CLOCKWISE ORDER.
///The function returns a playerNumber to identify this player in your code
static PyObject * AddHumanOpponent (PyObject *self, PyObject *args) //int8 (string playerName, float64 initialMoney);
{}

static PyObject * AddStrategyBot_ConservativeDefence (PyObject *self, PyObject *args) //int8 (string playerName, float64 initialMoney);
{}

static PyObject * AddStrategyBot_GambleDefence (PyObject *self, PyObject *args) //int8 (string playerName, float64 initialMoney);
{}

static PyObject * AddStrategyBot_ConservativeOffence (PyObject *self, PyObject *args) //int8 (string playerName, float64 initialMoney);
{}

static PyObject * AddStrategyBot_GambleOffence (PyObject *self, PyObject *args) //int8 (string playerName, float64 initialMoney);
{}



/*****************************************************************************
	Initial setup functions
	END
*****************************************************************************/


