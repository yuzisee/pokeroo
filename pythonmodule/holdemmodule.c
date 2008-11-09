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
#if 0
^/*[A-Z_]* .* ([^\( ][^\( ]*)\(.*
static PyObject * PyHoldem_\1 (PyObject *self, PyObject *args)\n{\n}
*/
#endif
 
 
// http://docs.python.org/extending/extending.html
// Since Python may define some pre-processor definitions which affect the standard headers on some systems, you must include Python.h before any standard headers are included.
#include "Python.h"


// The import library is currently stored with the dll at ../holdem/holdemdll/Release/* along with its import library.
// In lieu of a definitions file we have used the dllexport directive
#include "../holdem/libsrc/holdemDLL.h"

//sizeof(char) is 1, that's the C standard.
//http://drj11.wordpress.com/2007/04/08/sizeofchar-is-1/

// If you want to create an actual object http://starship.python.net/crew/arcege/extwriting/pyext.html
// But we probably won't be doing that


//http://docs.python.org/c-api/exceptions.html#PyErr_SetString
static PyObject * return_on_success(PyObject * retval, enum return_status error_code)
{

	//If there is an error of any sort, this function should return directly from inside the switch statement.
	switch( error_code )
	{
	case SUCCESS:
		break;
		
	case INPUT_CLEANED:
		break;
		
	case UNRELIABLE_RESULT:
	//http://docs.python.org/c-api/exceptions.html#PyErr_WarnEx
	//http://docs.python.org/library/warnings.html
		PyErr_WarnEx( PyExc_RuntimeWarning, "The returned result will be unreliable; hopefully you are trying to exit your program", 2);
		break;
		

//http://docs.python.org/library/exceptions.html?highlight=exceptions#module-exceptions
		
	case OUT_OF_MEMORY:
		PyErr_SetString(PyExc_MemoryError, "A call to malloc() or new() returned null");
		return NULL;
		
	case NULL_TABLE_PTR:
		PyErr_SetString(PyExc_AttributeError, "Pointer to C++ HoldemArena object is null!");
		return NULL;
		
	case NOT_IMPLEMENTED:
		PyErr_SetString(PyExc_NotImplementedError, "The function called does not apply to the current situation");
		return NULL;
		
	case PARAMETER_INVALID:
		PyErr_SetString(PyExc_LookupError, "A function parameter is out of range");	
		return NULL;
		
	case PARAMETER_DATA_ERROR:
		PyErr_SetString(PyExc_ValueError, "Parameters that were specified have not been initialized properly");	
		return NULL;
		
	case INTERNAL_INCONSISTENCY:
		PyErr_SetString(PyExc_AssertionError, "The internal state of the C++ objects are inconsistent");
		return NULL;
		
	default:
		PyErr_SetString(PyExc_RuntimeError, "The C interface has returned an unknown error_code");
		return NULL;
	}

	
	//If the code reaches this point outside the switch statement, we will be returning retval.
	//However, if the function would prefer to return None, it will not have specified a retval.
	if( retval )
	{
		return retval;
	}else
	{
		Py_RETURN_NONE;
	}
}

static PyObject * return_None_on_success(enum return_status error_code)
{
	return return_on_success(0,error_code);
}

static void * reconstruct_voidptr_address(const char * cards_ptr_char_array, int array_len)
{
	
	if( array_len == sizeof(void *) )
	{
		//A char array starting at the same address as the starting address of the void** will be interpreted by python as a string.
		//chars need one byte, numeric addresses need multiple bytes.
		//&(c.cards_ptr) is a (void**), say the memory location at which that numeric address is stored or a length-1 array of void*'s
		//c.cards_ptr is a (void*), say a numeric address of something.
		void ** cards_ptr_memory_address = (void **)cards_ptr_char_array;
		return (*cards_ptr_memory_address);
	}else
	{
		return 0; //All of the functions within the DLL have null pointer checking and will return an appropriate error
	}
	
}

static struct holdem_cardset reconstruct_holdem_cardset(const char * cards_ptr_char_array, int array_len, int card_count)
{
	struct holdem_cardset c;

	
	c.cards_ptr = reconstruct_voidptr_address(cards_ptr_char_array,array_len);
	c.card_count = card_count;
	return c;
}

static struct holdem_table reconstruct_holdem_table(const char * table_ptr_char_array, int array_len, int seat_count)
{
	struct holdem_table t;

	
	t.table_ptr = reconstruct_voidptr_address(table_ptr_char_array,array_len);
	t.seat_count = seat_count;
	return t;
}

static PyObject * return_tuple_holdem_cardset(struct holdem_cardset c)
{
	//c.cards_ptr is a (void*), say a numeric address of something.
	//&(c.cards_ptr) is a (void**), say the memory location at which that numeric address is stored or a length-1 array of void*'s
	//chars need one byte, numeric addresses need multiple bytes.
	//A char array starting at the same address as the starting address of the void** will be interpreted by python as a string.
	const char * cards_ptr_char_array = (const char *)(&(c.cards_ptr));
	
	return Py_BuildValue("(s#i)", cards_ptr_char_array, sizeof(void *), c.card_count);
}

static PyObject * return_tuple_holdem_table(struct holdem_table t)
{
	//t.cards_ptr is a (void*), say a numeric address of something.
	//&(t.cards_ptr) is a (void**), say the memory location at which that numeric address is stored or a length-1 array of void*'s
	//chars need one byte, numeric addresses need multiple bytes.
	//A char array starting at the same address as the starting address of the void** will be interpreted by python as a string.
	const char * table_ptr_char_array = (const char *)(&(t.table_ptr));
	
	return Py_BuildValue("(s#i)", table_ptr_char_array, sizeof(void *), t.seat_count);
}

static PyObject * return_voidptr(void * ptr)
{
	//ptr is a (void*), say a numeric address of something.
	//&(ptr) is a (void**), say the memory location at which that numeric address is stored or a length-1 array of void*'s
	//chars need one byte, numeric addresses need multiple bytes.
	//A char array starting at the same address as the starting address of the void** will be interpreted by python as a string.
	const char * ptr_char_array = (const char *)(&(ptr));
	
	return Py_BuildValue("s#", ptr_char_array, sizeof(void *));
}

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



//C_DLL_FUNCTION struct holdem_cardset CreateNewCardset();
static PyObject * PyHoldem_CreateNewCardset (PyObject *self, PyObject *args)
{
	struct holdem_cardset c = CreateNewCardset();	
	
	if( c.cards_ptr )
	{
		return return_tuple_holdem_cardset(c);
	}
	else
	{
		return return_None_on_success(OUT_OF_MEMORY);
	}
	
}



///For example, for the eight of hearts: cardValue = '8' and cardSuit = 'H'
//C_DLL_FUNCTION struct return_cardset AppendCard(struct holdem_cardset c, char cardValue,char cardSuit);
static PyObject * PyHoldem_AppendCard (PyObject *self, PyObject *args)
{
	const char *c_chars;
	int c_chars_len;
    char cardValue;
	char cardSuit;

	struct holdem_cardset c;
	
    if (!PyArg_ParseTuple(args, "(s#i)cc", &c_chars, &c_chars_len, &(c.card_count), &cardValue, &cardSuit))
        return NULL;

	c = reconstruct_holdem_cardset(c_chars,c_chars_len,c.card_count);
	
	struct return_cardset retval = AppendCard(c,cardValue,cardSuit);
	
	return return_on_success( return_tuple_holdem_cardset(retval.cardset) , retval.error_code );
}

//C_DLL_FUNCTION enum return_status DeleteCardset(struct holdem_cardset c);
static PyObject * PyHoldem_DeleteCardset (PyObject *self, PyObject *args)
{
	const char *c_chars;
	int c_chars_len;

	struct holdem_cardset c;

	if (!PyArg_ParseTuple(args, "(s#i)", &c_chars, &c_chars_len, &(c.card_count)))
        return NULL;
	
	c = reconstruct_holdem_cardset(c_chars,c_chars_len,c.card_count);
	
	enum return_status error_code = DeleteCardset(c);
	
	return return_None_on_success(error_code);
}

/*****************************************************************************
	END
	Card functions
*****************************************************************************/


/*****************************************************************************
	BEGIN
	Money functions
*****************************************************************************/

// Use this: http://www.python.org/doc/2.5.2/api/arg-parsing.html (also http://docs.python.org/c-api/arg.html#arg-parsing)
// and this: http://www.python.org/dev/peps/pep-0008/

///Get the amount of money playerNumber has in front of him
//C_DLL_FUNCTION struct return_money GetMoney(void * table_ptr, playernumber_t);
static PyObject * PyHoldem_GetMoney (PyObject *self, PyObject *args) 
{	
	const char *c_chars;
	int c_chars_len;
	int playernumber;

	if (!PyArg_ParseTuple(args, "(s#i)", &c_chars, &c_chars_len, &playernumber))
        return NULL;
	
	void * table_ptr = reconstruct_voidptr_address(c_chars,c_chars_len);
	
	struct return_money retval = GetMoney(table_ptr, playernumber);
	
	return return_on_success(Py_BuildValue("d", retval.money), retval.error_code);

}


///Override the amount of money playerNumber has in front of him
//C_DLL_FUNCTION enum return_status SetMoney(void * table_ptr, playernumber_t, float64);
static PyObject * PyHoldem_SetMoney (PyObject *self, PyObject *args) 
{
	const char *c_chars;
	int c_chars_len;
	int playernumber;
	double money;

	if (!PyArg_ParseTuple(args, "(s#i)d", &c_chars, &c_chars_len, &playernumber, &money))
        return NULL;
	
	void * table_ptr = reconstruct_voidptr_address(c_chars,c_chars_len);
	
	enum return_status error_code = SetMoney(table_ptr, playernumber, money);
	
	return return_None_on_success(error_code);
}



///Get the amount of money playerNumber has bet so far this round
//C_DLL_FUNCTION struct return_money GetCurrentRoundBet(void * table_ptr, playernumber_t playerNumber);
static PyObject * PyHoldem_GetCurrentRoundBet (PyObject *self, PyObject *args) 
{
	const char *c_chars;
	int c_chars_len;
	int playernumber;

	if (!PyArg_ParseTuple(args, "(s#i)", &c_chars, &c_chars_len, &playernumber))
        return NULL;
	
	void * table_ptr = reconstruct_voidptr_address(c_chars,c_chars_len);
	
	struct return_money retval = GetCurrentRoundBet(table_ptr, playernumber);
	
	return return_on_success(Py_BuildValue("d", retval.money), retval.error_code);
}


//C_DLL_FUNCTION struct return_money GetPrevRoundsBet(void * table_ptr, playernumber_t playerNumber);
static PyObject * PyHoldem_GetPrevRoundsBet (PyObject *self, PyObject *args)
{
	const char *c_chars;
	int c_chars_len;
	int playernumber;

	if (!PyArg_ParseTuple(args, "(s#i)", &c_chars, &c_chars_len, &playernumber))
        return NULL;
	
	void * table_ptr = reconstruct_voidptr_address(c_chars,c_chars_len);
	
	struct return_money retval = GetPrevRoundsBet(table_ptr, playernumber);
	
	return return_on_success(Py_BuildValue("d", retval.money), retval.error_code);
}

///Get the amount of money that is in the pot
//C_DLL_FUNCTION struct return_money GetPotSize(void * table_ptr);
static PyObject * PyHoldem_GetPotSize (PyObject *self, PyObject *args) 
{
    const char *c_chars;
	int c_chars_len;

	if (!PyArg_ParseTuple(args, "s#", &c_chars, &c_chars_len))
        return NULL;
	
	void * table_ptr = reconstruct_voidptr_address(c_chars,c_chars_len);
	
	struct return_money retval = GetPotSize(table_ptr);
	
	return return_on_success(Py_BuildValue("d", retval.money), retval.error_code);
}


///Get the amount of money that was in the pot at the BEGINNING of the current betting round
//C_DLL_FUNCTION struct return_money GetPrevRoundsPotsize(void * table_ptr);
static PyObject * PyHoldem_GetPrevRoundsPotsize (PyObject *self, PyObject *args)
{
    const char *c_chars;
	int c_chars_len;

	if (!PyArg_ParseTuple(args, "s#", &c_chars, &c_chars_len))
        return NULL;
	
	void * table_ptr = reconstruct_voidptr_address(c_chars,c_chars_len);
	
	struct return_money retval = GetPrevRoundsPotsize(table_ptr);
	
	return return_on_success(Py_BuildValue("d", retval.money), retval.error_code);
}



/*****************************************************************************
	Money functions
	END
*****************************************************************************/



/*****************************************************************************
	BEGIN
	Game Initialization functions
*****************************************************************************/
//C_DLL_FUNCTION enum return_status RestoreTableState(const char * state_str, void * table_ptr);
static PyObject * PyHoldem_RestoreTableState (PyObject *self, PyObject *args)
{
	const char *state_str;
	const char *c_chars;
	int c_chars_len;

	if (!PyArg_ParseTuple(args, "ss#", &state_str, &c_chars, &c_chars_len))
        return NULL;
	
	void * table_ptr = reconstruct_voidptr_address(c_chars,c_chars_len);
	
	enum return_status error_code = RestoreTableState(state_str, table_ptr);
	
	return return_None_on_success(error_code);
}
//C_DLL_FUNCTION enum return_status InitializeNewTableState(void * table_ptr);
static PyObject * PyHoldem_InitializeNewTableState (PyObject *self, PyObject *args)
{
	const char *c_chars;
	int c_chars_len;

	if (!PyArg_ParseTuple(args, "s#", &c_chars, &c_chars_len))
        return NULL;
	
	void * table_ptr = reconstruct_voidptr_address(c_chars,c_chars_len);
	
	enum return_status error_code = InitializeNewTableState(table_ptr);
	
	return return_None_on_success(error_code);
}

//C_DLL_FUNCTION struct return_string SaveTableState(void * table_ptr);
static PyObject * PyHoldem_SaveTableState (PyObject *self, PyObject *args)
{
	const char *c_chars;
	int c_chars_len;

	if (!PyArg_ParseTuple(args, "s#", &c_chars, &c_chars_len))
        return NULL;
	
	void * table_ptr = reconstruct_voidptr_address(c_chars,c_chars_len);
	
	struct return_string retval = SaveTableState(table_ptr);
	
	PyObject * Py_retval = Py_BuildValue("s", retval.str);
	
	if( retval.error_code == SUCCESS ) free(retval.str);
	
	return return_on_success(Py_retval, retval.error_code);
}
/*****************************************************************************
	Game Initialization functions
	END
*****************************************************************************/


/*****************************************************************************
	BEGIN
	Hand Initialization functions
*****************************************************************************/

//C_DLL_FUNCTION enum return_status BeginNewHands(void * table_ptr, float64 smallBlind, playernumber_t overrideDealer);
static PyObject * PyHoldem_BeginNewHands (PyObject *self, PyObject *args)
{
	const char *c_chars;
	int c_chars_len;
	double smallBlind;
	int overrideDealer = -1;

	if (!PyArg_ParseTuple(args, "s#di", &c_chars, &c_chars_len, &smallBlind, &overrideDealer))
        return NULL;
	
	void * table_ptr = reconstruct_voidptr_address(c_chars,c_chars_len);
	
	enum return_status error_code = BeginNewHands(table_ptr, smallBlind, overrideDealer);
	
	return return_None_on_success(error_code);
}


//C_DLL_FUNCTION enum return_status ShowHoleCardsToBot(void * table_ptr, playernumber_t , struct holdem_cardset );
static PyObject * PyHoldem_ShowHoleCardsToBot (PyObject *self, PyObject *args)
{
	const char *table_chars;
	int table_chars_len;
	
	const char *cardset_chars;
	int cardset_chars_len;
	
	int playernumber;
	struct holdem_cardset c;
	
	if (!PyArg_ParseTuple(args, "s#i(s#i)", &table_chars, &table_chars_len, &playernumber, &cardset_chars, &cardset_chars_len, &(c.card_count)))
        return NULL;

	c = reconstruct_holdem_cardset(cardset_chars,cardset_chars_len,c.card_count);
		
	void * table_ptr = reconstruct_voidptr_address(table_chars,table_chars_len);
	
	enum return_status error_code = ShowHoleCardsToBot(table_ptr, playernumber, c);
	
	return return_None_on_success(error_code);
}

//C_DLL_FUNCTION enum return_status FinishHandRefreshPlayers(void * table_ptr);
static PyObject * PyHoldem_FinishHandRefreshPlayers (PyObject *self, PyObject *args)
{
	const char *table_chars;
	int table_chars_len;

	if (!PyArg_ParseTuple(args, "s#", &table_chars, &table_chars_len))
        return NULL;
	
	void * table_ptr = reconstruct_voidptr_address(table_chars,table_chars_len);
	
	enum return_status error_code = FinishHandRefreshPlayers(table_ptr);
	
	return return_None_on_success(error_code);
}

/*****************************************************************************
	Hand Initialization functions
	END
*****************************************************************************/



/*****************************************************************************
	BEGIN
	Betting round functions
*****************************************************************************/

//C_DLL_FUNCTION struct return_event CreateNewBettingRound(void * table_ptr, struct holdem_cardset community );
static PyObject * PyHoldem_CreateNewBettingRound (PyObject *self, PyObject *args)
{
	const char *table_chars;
	int table_chars_len;
	
	const char *cardset_chars;
	int cardset_chars_len;

	struct holdem_cardset c;
	
	int total_betting_rounds;
	int future_betting_rounds;
	
	if (!PyArg_ParseTuple(args, "s#(s#i)ii", &table_chars, &table_chars_len, &cardset_chars, &cardset_chars_len, &(c.card_count), &total_betting_rounds, &future_betting_rounds))
        return NULL;

	c = reconstruct_holdem_cardset(cardset_chars,cardset_chars_len,c.card_count);
		
	void * table_ptr = reconstruct_voidptr_address(table_chars,table_chars_len);
	
	
	
	if( future_betting_rounds >= total_betting_rounds )
	{
		return return_None_on_success( PARAMETER_INVALID );
	}else
	{
		enum betting_round is_first_betting_round = LATER_BETTING_ROUND;
		if( total_betting_rounds == future_betting_rounds + 1 )
		{
			is_first_betting_round = FIRST_BETTING_ROUND;
		}
		struct return_event retval = CreateNewBettingRound(table_ptr, c, is_first_betting_round, future_betting_rounds);
		
		return return_on_success( return_voidptr(retval.event_ptr) , retval.error_code );
	}
	
}

//C_DLL_FUNCTION struct return_seat DeleteFinishBettingRound(void * event_ptr);
static PyObject * PyHoldem_DeleteFinishBettingRound (PyObject *self, PyObject *args)
{
    const char *event_chars;
	int event_chars_len;

	if (!PyArg_ParseTuple(args, "s#", &event_chars, &event_chars_len))
        return NULL;
	
	void * event_ptr = reconstruct_voidptr_address(event_chars,event_chars_len);
	
	struct return_seat retval = DeleteFinishBettingRound(event_ptr);
	
	return return_on_success(Py_BuildValue("i", retval.seat_number), retval.error_code);
}


//C_DLL_FUNCTION enum return_status PlayerMakesBetTo(void * event_ptr, playernumber_t playerNumber, float64 money);
static PyObject * PyHoldem_PlayerMakesBetTo (PyObject *self, PyObject *args)
{
    const char *event_chars;
	int event_chars_len;
	int playernumber;
	double money;

	if (!PyArg_ParseTuple(args, "s#id", &event_chars, &event_chars_len, &playernumber, &money))
        return NULL;
	
	void * event_ptr = reconstruct_voidptr_address(event_chars,event_chars_len);
	
	enum return_status error_code = PlayerMakesBetTo(event_ptr, playernumber, money);
	
	return return_None_on_success(error_code);
}



///GetBetAmount is useful for asking a bot what to bet
//C_DLL_FUNCTION struct return_money GetBetDecision(void * table_ptr, playernumber_t playerNumber);
static PyObject * PyHoldem_GetBetDecision (PyObject *self, PyObject *args)
{
    const char *table_chars;
	int table_chars_len;
	int playernumber;

	if (!PyArg_ParseTuple(args, "s#i", &table_chars, &table_chars_len, &playernumber))
        return NULL;
	
	void * table_ptr = reconstruct_voidptr_address(table_chars,table_chars_len);
	
	struct return_money retval = GetBetDecision(table_ptr, playernumber);
	
	return return_on_success(Py_BuildValue("d", retval.money), retval.error_code);
}



//Get the playerNumber of the player who's turn it is
//C_DLL_FUNCTION struct return_seat WhoIsNext_Betting(void * event_ptr);
static PyObject * PyHoldem_WhoIsNext_Betting (PyObject *self, PyObject *args)
{
    const char *event_chars;
	int event_chars_len;

	if (!PyArg_ParseTuple(args, "s#", &event_chars, &event_chars_len))
        return NULL;
	
	void * event_ptr = reconstruct_voidptr_address(event_chars,event_chars_len);
	
	struct return_seat retval = WhoIsNext_Betting(event_ptr);
	
	if( retval.seat_number == -1 )
	{
		return return_None_on_success(retval.error_code);
	}
	else
	{
		return return_on_success(Py_BuildValue("i", retval.seat_number), retval.error_code);
	}
}



///Get the size of the highest bet so far
//C_DLL_FUNCTION struct return_money GetBetToCall(void * table_ptr);
static PyObject * PyHoldem_GetBetToCall (PyObject *self, PyObject *args) 
{
    const char *c_chars;
	int c_chars_len;

	if (!PyArg_ParseTuple(args, "s#", &c_chars, &c_chars_len))
        return NULL;
	
	void * table_ptr = reconstruct_voidptr_address(c_chars,c_chars_len);
	
	struct return_money retval = GetBetToCall(table_ptr);
	
	return return_on_success(Py_BuildValue("d", retval.money), retval.error_code);
}



//C_DLL_FUNCTION struct return_money GetMinRaise(void * table_ptr);
static PyObject * PyHoldem_GetMinRaise (PyObject *self, PyObject *args) 
{
    const char *c_chars;
	int c_chars_len;

	if (!PyArg_ParseTuple(args, "s#", &c_chars, &c_chars_len))
        return NULL;
	
	void * table_ptr = reconstruct_voidptr_address(c_chars,c_chars_len);
	
	struct return_money retval = GetMinRaise(table_ptr);
	
	return return_on_success(Py_BuildValue("d", retval.money), retval.error_code);
}

/*****************************************************************************
	Betting round functions
	END
*****************************************************************************/


/*****************************************************************************
	BEGIN
	Showdown functions
*****************************************************************************/

///Call this when (if) the showdown begins
//C_DLL_FUNCTION struct return_event CreateNewShowdown(void * table_ptr, playernumber_t calledPlayer, struct holdem_cardset final_community);
static PyObject * PyHoldem_CreateNewShowdown (PyObject *self, PyObject *args)
{
	const char *table_chars;
	int table_chars_len;
	
	const char *cardset_chars;
	int cardset_chars_len;
	
	int calledPlayer;
	struct holdem_cardset c;
	
	if (!PyArg_ParseTuple(args, "s#i(s#i)", &table_chars, &table_chars_len, &calledPlayer, &cardset_chars, &cardset_chars_len, &(c.card_count)))
        return NULL;

	c = reconstruct_holdem_cardset(cardset_chars,cardset_chars_len,c.card_count);
		
	void * table_ptr = reconstruct_voidptr_address(table_chars,table_chars_len);
	
	struct return_event retval = CreateNewShowdown(table_ptr, calledPlayer, c);
	
	return return_on_success( return_voidptr(retval.event_ptr) , retval.error_code );
}

//Get the playerNumber of the player who's turn it is
//C_DLL_FUNCTION struct return_seat WhoIsNext_Showdown(void * event_ptr);
static PyObject * PyHoldem_WhoIsNext_Showdown (PyObject *self, PyObject *args)
{
    const char *event_chars;
	int event_chars_len;

	if (!PyArg_ParseTuple(args, "s#", &event_chars, &event_chars_len))
        return NULL;
	
	void * event_ptr = reconstruct_voidptr_address(event_chars,event_chars_len);
	
	struct return_seat retval = WhoIsNext_Showdown(event_ptr);
	
	if( retval.seat_number == -1 )
	{
		return return_None_on_success(retval.error_code);
	}
	else
	{
		return return_on_success(Py_BuildValue("i", retval.seat_number), retval.error_code);
	}
}




///Call this for each card playerNumber reveals during the showdown
//C_DLL_FUNCTION enum return_status PlayerShowsHand(void * event_ptr, playernumber_t playerNumber, struct holdem_cardset playerHand, struct holdem_cardset community);
static PyObject * PyHoldem_PlayerShowsHand (PyObject *self, PyObject *args)
{
	const char *event_chars;
	int event_chars_len;

	const char *playerHand_chars;
	int playerHand_chars_len;
	
	const char *community_chars;
	int community_chars_len;
	
	int playernumber;
	struct holdem_cardset playerHand;
	struct holdem_cardset community;
	
	if (!PyArg_ParseTuple(args, "s#i(s#i)(s#i)", &event_chars, &event_chars_len
											, &playernumber
											, &playerHand_chars, &playerHand_chars_len, &(playerHand.card_count)
											, &community_chars, &community_chars_len, &(community.card_count) )
	   )
	{
        return NULL;
	}

	playerHand = reconstruct_holdem_cardset(playerHand_chars,playerHand_chars_len,playerHand.card_count);
	community = reconstruct_holdem_cardset(community_chars,community_chars_len,community.card_count);
		
	void * event_ptr = reconstruct_voidptr_address(event_chars,event_chars_len);
	
	enum return_status error_code = PlayerShowsHand(event_ptr, playernumber, playerHand, community);
	
	return return_None_on_success(error_code);
}

///Call this when playerNumber mucks his/her hand during the showdow
//C_DLL_FUNCTION enum return_status PlayerMucksHand(void * event_ptr, playernumber_t playerNumber);
static PyObject * PyHoldem_PlayerMucksHand (PyObject *self, PyObject *args)
{
    const char *event_chars;
	int event_chars_len;
	int playernumber;

	if (!PyArg_ParseTuple(args, "s#i", &event_chars, &event_chars_len, &playernumber))
        return NULL;
	
	void * event_ptr = reconstruct_voidptr_address(event_chars,event_chars_len);
	
	enum return_status error_code = PlayerMucksHand(event_ptr, playernumber);
	
	return return_None_on_success(error_code);
}

//C_DLL_FUNCTION enum return_status DeleteFinishShowdown(void * table_ptr, void * event_ptr);
static PyObject * PyHoldem_DeleteFinishShowdown (PyObject *self, PyObject *args)
{
	const char *table_chars;
	int table_chars_len;
	
	const char *event_chars;
	int event_chars_len;

	if (!PyArg_ParseTuple(args, "s#s#", &table_chars, &table_chars_len, &event_chars, &event_chars_len))
        return NULL;
	
	void * table_ptr = reconstruct_voidptr_address(table_chars,table_chars_len);
	void * event_ptr = reconstruct_voidptr_address(event_chars,event_chars_len);
	
	enum return_status error_code = DeleteFinishShowdown(table_ptr, event_ptr);
	
	return return_None_on_success(error_code);
}


/*****************************************************************************
	Showdown functions
	END
*****************************************************************************/




/*****************************************************************************
	BEGIN
	Main Constructor/Destructors
*****************************************************************************/

//C_DLL_FUNCTION struct return_table CreateNewTable(playernumber_t seatsAtTable, float64 chipDenomination);
static PyObject * PyHoldem_CreateNewTable (PyObject *self, PyObject *args)
{
    int seatsAtTable;
	double chipDenomination;
	
    if (!PyArg_ParseTuple(args, "id", &seatsAtTable, &chipDenomination))
        return NULL;

	struct return_table retval = CreateNewTable(seatsAtTable,chipDenomination);
	
	return return_on_success( return_tuple_holdem_table(retval.table) , retval.error_code );
}

//C_DLL_FUNCTION enum return_status DeleteTableAndPlayers(struct holdem_table table_to_delete);
static PyObject * PyHoldem_DeleteTableAndPlayers (PyObject *self, PyObject *args)
{
	const char *table_chars;
	int table_chars_len;

	struct holdem_table t;

	if (!PyArg_ParseTuple(args, "(s#i)", &table_chars, &table_chars_len, &(t.seat_count)))
        return NULL;
	
	t = reconstruct_holdem_table(table_chars,table_chars_len,t.seat_count);
	
	enum return_status error_code = DeleteTableAndPlayers(t);
	
	return return_None_on_success(error_code);
}

///Add a player to the table. PLAYERS MUST BE ADDED IN CLOCKWISE ORDER.
///These functions returns a playerNumber to identify this player in your code

//C_DLL_FUNCTION struct return_seat CreateNewHumanOpponent(struct holdem_table add_to_table, const char * playerName, float64 money);
static PyObject * PyHoldem_CreateNewHumanOpponent (PyObject *self, PyObject *args)
{
    const char *table_chars;
	int table_chars_len;

	struct holdem_table t;
	const char * playerName;
	double money;

	if (!PyArg_ParseTuple(args, "(s#i)sd", &table_chars, &table_chars_len, &(t.seat_count), &playerName, &money))
        return NULL;
	
	t = reconstruct_holdem_table(table_chars,table_chars_len,t.seat_count);
	
	struct return_seat retval = CreateNewHumanOpponent(t, playerName, money);
	
	return return_on_success(Py_BuildValue("i", retval.seat_number), retval.error_code);
}
//C_DLL_FUNCTION struct return_seat CreateNewStrategyBot(struct holdem_table add_to_table, const char *playerName, float64 money, char botType);
static PyObject * PyHoldem_CreateNewStrategyBot (PyObject *self, PyObject *args)
{
    const char *table_chars;
	int table_chars_len;

	struct holdem_table t;
	const char * playerName;
	double money;
	char botType;

	if (!PyArg_ParseTuple(args, "(s#i)sdc", &table_chars, &table_chars_len, &(t.seat_count), &playerName, &money, &botType))
        return NULL;
	
	t = reconstruct_holdem_table(table_chars,table_chars_len,t.seat_count);
	
	struct return_seat retval = CreateNewStrategyBot(t, playerName, money, botType);
	
	return return_on_success(Py_BuildValue("i", retval.seat_number), retval.error_code);
}


/*****************************************************************************
	Main Constructor/Destructors
	END
*****************************************************************************/





/*****************************************************************************
	BEGIN
	Python-specific initializers
*****************************************************************************/

static PyMethodDef HoldemMethods[] = {
    {"get_money",  						PyHoldem_GetMoney, METH_VARARGS				, "s#i: Get the amount of money a player has in front of him/her."},
    {"set_money",  						PyHoldem_SetMoney, METH_VARARGS				, "s#id: Override the amount of money a player has in front of him/her."},
	{"get_current_round_bet",  			PyHoldem_GetCurrentRoundBet, METH_VARARGS	, "s#i: Get the amount of money a player has bet so far this round"},
	{"get_previous_rounds_bet",  		PyHoldem_GetPrevRoundsBet, METH_VARARGS		, "s#i: Get the amount of money a player has bet so far in all previous rounds"},
	{"get_pot_size",  					PyHoldem_GetPotSize, METH_VARARGS			, "s#: Get the amount of money that is in the pot"},
	{"get_previous_rounds_pot_size",  	PyHoldem_GetPrevRoundsPotsize, METH_VARARGS	, "s#: Get the amount of money that was in the pot at the BEGINNING of the current betting round"},
	{"restore_table_state",  			PyHoldem_RestoreTableState, METH_VARARGS	, "ss#: Restore a table from a state saved with save_table_state instead of initializing a new table state"},
	{"initialize_new_table_state",  	PyHoldem_InitializeNewTableState, METH_VARARGS, "s#: Initialize the state of a newly created table instead of restoring from a saved state"},
	{"begin_new_hands",  				PyHoldem_BeginNewHands, METH_VARARGS		, "s#di: Call this when it is time to begin dealing new hands to all of the players"},
	{"bot_receives_hole_cards",  		PyHoldem_ShowHoleCardsToBot, METH_VARARGS	, "s#i(s#i): Notify a bot that it has received hole cards"},
	{"finish_hand_refresh_players",  	PyHoldem_FinishHandRefreshPlayers, METH_VARARGS, "s#: Complete any final bookkeeping that needs to take place to prepare data structures for the next hand"},
	{"save_table_state",  				PyHoldem_SaveTableState, METH_VARARGS		, "s#: After finish_hand_refresh_players has been called, use this function to save the state of the table"},
//	{"reset_deterministic_seed",  ResetDeterministicSeed, METH_VARARGS, ""},
//	{"get_deterministic_seed",  GetDeterministicSeed, METH_VARARGS, ""},
	{"create_new_cardset",  			PyHoldem_CreateNewCardset, METH_VARARGS		, "Create a container for cards"},
	{"append_card_to_cardset",  		PyHoldem_AppendCard, METH_VARARGS			, "(s#i)cc: Add a single card to a cardset container"},
	{"delete_cardset",  				PyHoldem_DeleteCardset, METH_VARARGS		, "(s#i): Free a cardset that was created with create_new_cardset to release the memory"},
	{"create_new_betting_round",  		PyHoldem_CreateNewBettingRound, METH_VARARGS, "s#(s#i)ii: A betting round object moderates the bets made at the table for a given set of community cards"},
	{"delete_finish_betting_round",  	PyHoldem_DeleteFinishBettingRound, METH_VARARGS, "s#: When a betting round completes, free the betting round object and retrieve the called player's seat number"},
	{"player_makes_bet",  				PyHoldem_PlayerMakesBetTo, METH_VARARGS		, "s#id: Indicate to a betting round object that a specific player has made a certain bet"},
	{"get_bot_bet_decision",  			PyHoldem_GetBetDecision, METH_VARARGS		, "s#i: Ask a bot what bet it would like to make"},
	{"get_bet_to_call",  				PyHoldem_GetBetToCall, METH_VARARGS			, "s#: Get the amount of the largest bet so far this round"},
	{"get_minimum_raise_by",  			PyHoldem_GetMinRaise, METH_VARARGS			, "s#: Get the amount of the smallest allowable raise"},
	{"who_is_next_to_bet",  			PyHoldem_WhoIsNext_Betting, METH_VARARGS	, "s#: Get the seat number of the player that is next to act in a betting round"},
	{"who_is_next_in_showdown",  		PyHoldem_WhoIsNext_Showdown, METH_VARARGS	, "s#: Get the seat number of the player that is next to act in a showdown"},
	{"player_shows_hand",  				PyHoldem_PlayerShowsHand, METH_VARARGS		, "s#i(s#i)(s#i): Indicate to a showdown object that a specific player has revealed its hand"},
	{"player_mucks_hand",  				PyHoldem_PlayerMucksHand, METH_VARARGS		, "s#i: Indicate to a showdown object that a specific player has mucked its hand"},
	{"create_new_showdown",  			PyHoldem_CreateNewShowdown, METH_VARARGS	, "s#i(s#i): A showdown object moderates the revealing of hands performed at the table, starting with the called player"},
	{"delete_finish_showdown",  		PyHoldem_DeleteFinishShowdown, METH_VARARGS	, "s#s#: What a showdown round completes, free the showdown object"},
	{"create_new_table", 				PyHoldem_CreateNewTable, METH_VARARGS		, "id: Create a table: This is the main constructor"},
	{"delete_table_and_players",  		PyHoldem_DeleteTableAndPlayers, METH_VARARGS, "(s#i): Delete the table and all added players: This is the main destructor"},
	{"add_a_human_opponent",  			PyHoldem_CreateNewHumanOpponent, METH_VARARGS, "(s#i)sd: Indicate that a non-bot player will be sitting at the table in the next seat (clockwise)"},
	{"add_a_strategy_bot",  			PyHoldem_CreateNewStrategyBot, METH_VARARGS	, "(s#i)sdc: Indicate that a bot will be sitting at the table in the next seat (clockwise)"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};
//http://www.python.org/dev/peps/pep-0008/
//Function names should be lowercase, with words separated by underscores as necessary to improve readability.
//Use one leading underscore only for non-public methods and instance variables
//you should use the suffix "Error" on your exception names (if the exception actually is an error).

/*

C_DLL_FUNCTION struct return_money GetMoney(void * table_ptr, playernumber_t);
C_DLL_FUNCTION enum return_status SetMoney(void * table_ptr, playernumber_t, float64);
C_DLL_FUNCTION struct return_money GetCurrentRoundBet(void * table_ptr, playernumber_t playerNumber);
C_DLL_FUNCTION struct return_money GetPrevRoundsBet(void * table_ptr, playernumber_t playerNumber);
C_DLL_FUNCTION struct return_money GetPotSize(void * table_ptr);
C_DLL_FUNCTION struct return_money GetPrevRoundsPotsize(void * table_ptr);
C_DLL_FUNCTION enum return_status RestoreTableState(const char * state_str, void * table_ptr);
C_DLL_FUNCTION enum return_status InitializeNewTableState(void * table_ptr);
C_DLL_FUNCTION enum return_status BeginNewHands(void * table_ptr, float64 smallBlind, playernumber_t overrideDealer);
C_DLL_FUNCTION enum return_status ShowHoleCardsToBot(void * table_ptr, playernumber_t , struct holdem_cardset );
C_DLL_FUNCTION enum return_status FinishHandRefreshPlayers(void * table_ptr);
C_DLL_FUNCTION struct return_string SaveTableState(void * table_ptr);
C_DLL_FUNCTION struct holdem_cardset CreateNewCardset(void);
C_DLL_FUNCTION struct return_cardset AppendCard(struct holdem_cardset c, char cardValue,char cardSuit);
C_DLL_FUNCTION enum return_status DeleteCardset(struct holdem_cardset c);
C_DLL_FUNCTION struct return_event CreateNewBettingRound(void * table_ptr, struct holdem_cardset community );
C_DLL_FUNCTION struct return_seat DeleteFinishBettingRound(void * event_ptr);
C_DLL_FUNCTION enum return_status PlayerMakesBetTo(void * event_ptr, playernumber_t playerNumber, float64 money);
C_DLL_FUNCTION struct return_money GetBetDecision(void * table_ptr, playernumber_t playerNumber);
C_DLL_FUNCTION struct return_money GetBetToCall(void * table_ptr);
C_DLL_FUNCTION struct return_seat WhoIsNext_Betting(void * event_ptr);
C_DLL_FUNCTION struct return_seat WhoIsNext_Showdown(void * event_ptr);
C_DLL_FUNCTION enum return_status PlayerShowsHand(void * event_ptr, playernumber_t playerNumber, struct holdem_cardset playerHand, struct holdem_cardset community);
C_DLL_FUNCTION enum return_status PlayerMucksHand(void * event_ptr, playernumber_t playerNumber);
C_DLL_FUNCTION struct return_event CreateNewShowdown(void * table_ptr, playernumber_t calledPlayer, struct holdem_cardset final_community);
C_DLL_FUNCTION enum return_status DeleteFinishShowdown(void * table_ptr, void * event_ptr);
C_DLL_FUNCTION struct return_table CreateNewTable(playernumber_t seatsAtTable, float64 chipDenomination);
C_DLL_FUNCTION enum return_status DeleteTableAndPlayers(struct holdem_table table_to_delete);
C_DLL_FUNCTION struct return_seat CreateNewHumanOpponent(struct holdem_table add_to_table, const char * playerName, float64 money);
C_DLL_FUNCTION struct return_seat CreateNewStrategyBot(struct holdem_table add_to_table, const char *playerName, float64 money, char botType);

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

