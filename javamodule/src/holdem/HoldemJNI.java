/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package holdem;

import com.biotools.meerkat.Hand;

/**
 *
 * @author Joseph
 */
public class HoldemJNI {

    /*Hole cards for a Player or community cards for a HoldemArena
    You may also add multiple cards at a time: separate your cards with whitespace.
    */
    public class HoldemCardsetJNI
    {
        
        
        
    }

    //==============================
    //   Constructors/Destructors
    //==============================
    static native long CreateNewTable(int seatsAtTable, double chipDenomination);
    static native void DeleteTableAndPlayers(long ptr_holdemCPtr);

    static native long CreateNewShowdown(long ptr_holdemCPtr, int calledPlayer, Hand final_community);
    static native void DeleteFinishShowdown(long ptr_holdemCPtr, long ptr_eventPtr);

    static native long NewCardset(Hand c); //C_DLL_FUNCTION struct holdem_cardset CreateNewCardset();
    //C_DLL_FUNCTION struct return_cardset AppendCard(struct holdem_cardset c, char cardValue,char cardSuit);
    static native void DeleteCardset(long ptr_holdemCardset);

    static native long CreateNewBettingRound(long ptr_holdemCPtr, Hand community);
    static native int DeleteFinishBettingRound(long ptr_eventPtr);

    //=========================
    //   Forwarded Callbacks
    //=========================



    static void BeginNewHands(long ptr_holdemCPtr, double smallBlind) { BeginNewHands(ptr_holdemCPtr, smallBlind, -1); }
    static native void BeginNewHands(long ptr_holdemCPtr, double smallBlind, int dealerOverride);

    static native void ShowHoleCardsToBot(long ptr_holdemCPtr, double smallBlind, int dealerOverride);
    static native double GetBetDecision(long ptr_holdemCPtr, int playerNumber);

    static native void FinishHandRefreshPlayers(long ptr_holdemCPtr);

    static native void PlayerMakesBetTo(long ptr_eventPtr, int playerNumber, double money);
    static native void PlayerShowsHand(long ptr_eventPtr, int playerNumber, Hand playerHand, Hand community);
    static native void PlayerMucksHand(long ptr_eventPtr, int playerNumber);



    //====================
    //   Initialization
    //====================


    static native void SetMoney(long ptr_holdemCPtr, int playerNumber, double money);
    static native void InitializeNewTableState(long ptr_holdemCPtr);

    static native int CreateNewHumanOpponent(long ptr_holdemCPtr, String playerName, double money);
    static native int CreateNewStrategyBot(long ptr_holdemCPtr, String playerName, double money, char botType);



    //==========================
    //   Assertions/Debugging
    //==========================

    /*
    static native double GetMoney(long ptr_holdemCPtr, int playerNumber);
    static native double GetCurrentRoundBet(long ptr_holdemCPtr, int playerNumber); //Get the amount of money a player has bet so far this round
    static native double GetPrevRoundsBet(long ptr_holdemCPtr, int playerNumber); //Get the amount of money a player has bet so far in all previous rounds
    static native double GetPotSize(long ptr_holdemCPtr);
    static native double GetPrevRoundsPotsize(long ptr_holdemCPtr);
     
    static native double GetBetToCall(long ptr_holdemCPtr);
    static native int WhoIsNext_Betting(long ptr_eventPtr);
    static native int WhoIsNext_Showdown(long ptr_eventPtr);
    
    static native int GetHandnum(long ptr_holdemCPtr);
    */

    //static native void RestoreTableState(String stateStr, long ptr_holdemCPtr);
    //static native String SaveTableState(long ptr_holdemCPtr);
    //static native void ResetDeterministicSeed(long ptr_holdemCPtr);
    //C_DLL_FUNCTION uint32 GetDeterministicSeed(long ptr_holdemCPtr, uint8 small_int);


}
