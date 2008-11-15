/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package holdem;

/**
 *
 * @author Joseph
 */
public class HoldemJNI {
    
    public class HoldemCardsetJNI
    {
        
    }
    
    static native int GetHandnum(byte[] holdemCPtr);
    static native double GetMoney(byte[] holdemCPtr, int playerNumber);
    static native void SetMoney(byte[] holdemCPtr, int playerNumber, double money);
    static native double GetCurrentRoundBet(byte[] holdemCPtr, int playerNumber); //Get the amount of money a player has bet so far this round
    static native double GetPrevRoundsBet(byte[] holdemCPtr, int playerNumber); //Get the amount of money a player has bet so far in all previous rounds
    static native double GetPotSize(byte[] holdemCPtr);
    static native double GetPrevRoundsPotsize(byte[] holdemCPtr);
    static native void RestoreTableState(String stateStr, byte[] holdemCPtr);
    static native void InitializeNewTableState(byte[] holdemCPtr);
    static void BeginNewHands(byte[] holdemCPtr, double smallBlind) { BeginNewHands(holdemCPtr, smallBlind, -1); } 
    static native void BeginNewHands(byte[] holdemCPtr, double smallBlind, int dealerOverride);
    static native void ShowHoleCardsToBot(byte[] holdemCPtr, double smallBlind, int dealerOverride);
    
    
}

/*
 * 
static PyMethodDef HoldemMethods[] = {
	{"get_hand_number",					PyHoldem_GetHandnum, METH_VARARGS			, "s#: Get the hand number."},
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
 */