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


// The import library is currently stored with the dll at ../holdem/holdemdll/Release/* along with its import library.
// In lieu of a definitions file we have used the dllexport directive
#include "../holdem/libsrc/holdemDLL.h"



// If you want to create an actual object http://starship.python.net/crew/arcege/extwriting/pyext.html
// But we probably won't be doing that


/*****************************************************************************
	BEGIN
	Betting round accessors
*****************************************************************************/

// Use this: http://www.python.org/doc/2.5.2/api/arg-parsing.html (also http://docs.python.org/c-api/arg.html#arg-parsing)
// and this: http://www.python.org/dev/peps/pep-0008/

///Get the amount of money playerNumber has in front of him
static PyObject * PyHoldem_GetMoney (PyObject *self, PyObject *args) //float64 (int8 playerNumber);
{
    return Py_BuildValue("d", 10.0);
    //TODO: If the player is all in, make sure this returns the correct value
}


//Override the amount of money playerNumber has in front of him
static PyObject * PyHoldem_SetMoney (PyObject *self, PyObject *args) ////void (int8 playerNumber, float64 money);
{
    Py_RETURN_NONE;
}

#if 0

///Get the amount of money playerNumber has bet so far this round
static PyObject * GetBetThisRound (PyObject *self, PyObject *args) //float64 (int8 playerNumber);
{
    //TODO: If the player is all in, make sure this returns the correct value
    return Py_BuildValue("d", 1.0);
}




///Get the amount of money that is in the pot
static PyObject * GetPotSize (PyObject *self, PyObject *args) //float64 ();
{
    return Py_BuildValue("d", 4.75);
}


///Get the amount of money that was in the pot at the BEGINNING of the current betting round
static PyObject * GetLastRoundPotsize (PyObject *self, PyObject *args) //float64 ();
{
    return Py_BuildValue("d", 0.5);
}


///Get the size of the highest bet so far
static PyObject * GetBetToCall (PyObject *self, PyObject *args) //float64 ();
{
    return Py_BuildValue("d", 2.5);
}



//Get the playerNumber of the player who's turn it is
static PyObject * WhoIsNext (PyObject *self, PyObject *args) //int8 ();
{
    return Py_BuildValue("i", 0);
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

#endif // 0

static PyObject * PyHoldem_CreateNewCardset (PyObject *self, PyObject *args)
{
	struct holdem_cardset n = CreateNewCardset();
	return Py_BuildValue("i", n.card_count);
}

#if 0

///For example, for the eight of hearts: cardValue = '8' and cardSuit = 'H'
enum return_status AppendCard(struct holdem_cardset * c, char cardValue,char cardSuit);

enum return_status DeleteCardset(struct holdem_cardset c);

/*****************************************************************************
	END
	Card functions
*****************************************************************************/



/*****************************************************************************
	BEGIN
	Event functions

Note: If AutoPlayGame() is called, bAutoMode is set to 1, you never
need to use any of the Start______() functions.
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
static PyObject * NewCommunityCard (PyObject *self, PyObject *args) //void (char cardValue,char cardSuit);
{
    Py_RETURN_NONE ;
}





///Call this when the betting begins
static PyObject * StartBetting (PyObject *self, PyObject *args) //void ();
{
    Py_RETURN_NONE ;
}


///Call these functions when playerNumber Raises, Folds, or Calls
static PyObject * PlayerCalls (PyObject *self, PyObject *args) //void (int8 playerNumber);
{Py_RETURN_NONE;}

static PyObject * PlayerFolds (PyObject *self, PyObject *args) //void (int8 playerNumber);
{Py_RETURN_NONE;}

static PyObject * PlayerRaisesTo (PyObject *self, PyObject *args) //void (int8 playerNumber, float64 amount);
{Py_RETURN_NONE;}

static PyObject * PlayerRaisesBy (PyObject *self, PyObject *args) //void (int8 playerNumber, float64 amount);
{Py_RETURN_NONE;}
///Question: If a player doesn't call any of these, which is the default action?



///GetBetAmount is useful for asking a bot what to bet
static PyObject * DecideBetAmount (PyObject *self, PyObject *args) //float64 (int8 playerNumber);
{
    return Py_BuildValue("d", 6.0);
}



///Call this when (if) the showdown begins
static PyObject * StartShowdown (PyObject *self, PyObject *args) //void ();
{Py_RETURN_NONE;}

///Call this for each card playerNumber reveals during the showdown
///See NewCommunityCard for usage of cardValue and cardSuit
static PyObject * PlayerShowsCard (PyObject *self, PyObject *args) //void (int8 playerNumber, char cardValue, char cardSuit);
{Py_RETURN_NONE;}

///Call this when playerNumber mucks his/her hand during the showdown.
///Note: If a player doesn't PlayerShowsCard() then a muck is assumed
static PyObject * PlayerMucksHand (PyObject *self, PyObject *args) //void (int8 playerNumber);
{Py_RETURN_NONE;}



///Call this when new hands are dealt
static PyObject * StartDealNewHands (PyObject *self, PyObject *args) //void ();
{Py_RETURN_NONE;}

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
{Py_RETURN_NONE;}

///Set the amount of money that the SMALLEST chip is worth
static PyObject * InitSmallestChipSize (PyObject *self, PyObject *args) //void (float64 money);
{Py_RETURN_NONE;}

///Call this when the big blind has changed
static PyObject * SetBigBlind (PyObject *self, PyObject *args) //void ();
{Py_RETURN_NONE;}

///Call this when the small blind has changed
static PyObject * SetSmallBlind (PyObject *self, PyObject *args) //void ();
{Py_RETURN_NONE;}

///Add a player to the table. PLAYERS MUST BE ADDED IN CLOCKWISE ORDER.
///The function returns a playerNumber to identify this player in your code
static PyObject * AddHuman (PyObject *self, PyObject *args) //int8 (string playerName);
{
    return Py_BuildValue("i", 0);
}

static PyObject * AddBot (PyObject *self, PyObject *args) //int8 (string playerName, char botType);
{
    return Py_BuildValue("i", 1);
}


/*****************************************************************************
	Initial setup functions
	END
*****************************************************************************/

#endif //0



/*****************************************************************************
	BEGIN
	Python-specific initializers
*****************************************************************************/

static PyMethodDef HoldemMethods[] = {
    {"get_money",  PyHoldem_GetMoney, METH_VARARGS, "Get the amount of money a player has in front of him/her."},
    {"SetMoney",  PyHoldem_SetMoney, METH_VARARGS, "Override the amount of money a player has in front of him/her."},
	{"CC",  PyHoldem_CreateNewCardset, METH_VARARGS, "Create a new set of cards to form a hand. Delete this cardset when you are done with it."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

/*
C_DLL_FUNCTION struct return_money GetMoney(void * table_ptr, playernumber_t);
C_DLL_FUNCTION enum return_status SetMoney(void * table_ptr, playernumber_t, float64);
C_DLL_FUNCTION struct return_money GetCurrentRoundBet(void * table_ptr, playernumber_t playerNumber);
C_DLL_FUNCTION struct return_money GetPrevRoundsBet(void * table_ptr, playernumber_t playerNumber);
C_DLL_FUNCTION struct return_money GetPotSize(void * table_ptr);
C_DLL_FUNCTION struct return_money GetPrevRoundsPotsize(void * table_ptr);
C_DLL_FUNCTION enum return_status RestoreTableState(char * state_str, void * table_ptr);
C_DLL_FUNCTION enum return_status InitializeNewTableState(void * table_ptr);
C_DLL_FUNCTION enum return_status BeginNewHands(void * table_ptr, float64 smallBlind);
C_DLL_FUNCTION enum return_status ShowHoleCards(void * table_ptr, playernumber_t , struct holdem_cardset );
C_DLL_FUNCTION enum return_status SaveTableState(char * state_str, void * table_ptr);
C_DLL_FUNCTION struct holdem_cardset CreateNewCardset();
C_DLL_FUNCTION enum return_status AppendCard(struct holdem_cardset * c, char cardValue,char cardSuit);
C_DLL_FUNCTION enum return_status DeleteCardset(struct holdem_cardset c);
C_DLL_FUNCTION struct return_event CreateNewBettingRound(void * table_ptr, struct holdem_cardset community );
C_DLL_FUNCTION struct return_seat DeleteFinishBettingRound(void * event_ptr);
C_DLL_FUNCTION enum return_status PlayerMakesBetTo(void * event_ptr, playernumber_t playerNumber, float64 money);
C_DLL_FUNCTION struct return_money GetBetDecision(void * table_ptr, playernumber_t playerNumber);
C_DLL_FUNCTION struct return_money GetBetToCall(void * table_ptr);
C_DLL_FUNCTION struct return_seat WhoIsNext_Betting(void * event_ptr);
C_DLL_FUNCTION struct return_seat WhoIsNext_Showdown(void * event_ptr);
C_DLL_FUNCTION enum return_status PlayerShowsCard(void * event_ptr, playernumber_t playerNumber, struct holdem_cardset playerHand, struct holdem_cardset community);
C_DLL_FUNCTION enum return_status PlayerMucksHand(void * event_ptr, playernumber_t playerNumber);
C_DLL_FUNCTION struct return_event CreateNewShowdown(void * table_ptr, playernumber_t calledPlayer, struct holdem_cardset final_community);
C_DLL_FUNCTION enum return_status DeleteFinishShowdown(void * table_ptr, void * event_ptr);
C_DLL_FUNCTION struct return_table CreateNewTable(playernumber_t seatsAtTable, float64 chipDenomination);
C_DLL_FUNCTION enum return_status DeleteTableAndPlayers(struct holdem_table table_to_delete);
C_DLL_FUNCTION struct return_seat CreateNewHumanOpponent(struct holdem_table add_to_table, char * playerName, float64 money);
C_DLL_FUNCTION struct return_seat CreateNewStrategyBot(struct holdem_table add_to_table, char *playerName, float64 money, char botType);

*/


PyMODINIT_FUNC
init_holdem(void)
{
    (void) Py_InitModule("_holdem", HoldemMethods);
}

/*****************************************************************************
	Python-specific initializers
	END
*****************************************************************************/

