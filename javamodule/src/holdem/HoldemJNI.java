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

    /*Hole cards for a Player or community cards for a HoldemArena
    You may also add multiple cards at a time: separate your cards with whitespace.
    */
    public class HoldemCardsetJNI
    {
        
        
        
    }

    //==============================
    //   Constructors/Destructors
    //==============================


    static native long CreateNewShowdown(long ptr_holdemCPtr, int calledPlayer, Hand final_community);
    static native void DeleteFinishShowdown(long ptr_holdemCPtr, long ptr_eventPtr);

    
    static native long CreateNewBettingRound(long ptr_holdemCPtr, Hand community);
    static native int DeleteFinishBettingRound(long ptr_eventPtr);

    //=========================
    //   Forwarded Callbacks
    //=========================



    
    static native double GetBetDecision(long ptr_holdemCPtr, int playerNumber);

    static native void FinishHandRefreshPlayers(long ptr_holdemCPtr);

    static native void PlayerMakesBetTo(long ptr_eventPtr, int playerNumber, double money);
    static native void PlayerShowsHand(long ptr_eventPtr, int playerNumber, Hand playerHand, Hand community);
    static native void PlayerMucksHand(long ptr_eventPtr, int playerNumber);



    //====================
    //   Initialization
    //====================


    static native void SetMoney(long ptr_holdemCPtr, int playerNumber, double money);

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
