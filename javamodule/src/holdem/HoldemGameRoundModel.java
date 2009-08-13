/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package holdem;

import com.biotools.meerkat.GameInfo;
import com.biotools.meerkat.PlayerInfo;


/**
 *
 * @author Joseph
 */
public class HoldemGameRoundModel {

   private GameInfo tableState;       // general game information
   private HoldemBotCardset myHoleCards;

   //======================
   //   Native functions
   //======================
   private long ptr_HoldemArena = 0;
   private long ptr_bettingEventPtr = 0;
   private long ptr_showdownEventPtr = 0;

   ///Initialization
   native void CreateNewTable(int seatsAtTable, double chipDenomination, long gameID);

   native int CreateNewHumanOpponent(String playerName, double money);
   native int CreateNewStrategyBot(String playerName, double money, char botType);

   native void InitializeNewTableState();

   native void BeginNewHands(double smallBlind, int dealerOverride);
   native void ShowHoleCardsToBot(int mySeat, HoldemBotCardset myHoleCards);

   ///Betting
   static native long CreateNewBettingRound(long ptr_holdemCPtr, Hand community);
   static native int DeleteFinishBettingRound(long ptr_eventPtr);

   static native double GetBetDecision(long ptr_holdemCPtr, int playerNumber);
   static native void PlayerMakesBetTo(long ptr_eventPtr, int playerNumber, double money);
   static native int WhoIsNext_Betting(long ptr_eventPtr);

   ///Showdown
   static native long CreateNewShowdown(long ptr_holdemCPtr, int calledPlayer, Hand final_community);
   static native void DeleteFinishShowdown(long ptr_holdemCPtr, long ptr_eventPtr);

   static native void PlayerShowsHand(long ptr_eventPtr, int playerNumber, Hand playerHand, Hand community);
   static native void PlayerMucksHand(long ptr_eventPtr, int playerNumber);
   static native int WhoIsNext_Showdown(long ptr_eventPtr);

   //native void FinishHandRefreshPlayers(); //Can be used as an assertion to reconcile money between tables
   native void DeleteTableAndPlayers();

   private void ValidateImplementation()
   {

      if( tableState.getAnte() > 0 ) { throw new RuntimeException("Ante not yet implemented..."); }
      if( tableState.isFixedLimit() || ! tableState.isNoLimit() ) { throw new RuntimeException("Currently only no-limit is implemented..."); }
      if( tableState.isReverseBlinds() ) { throw new RuntimeException("Heads-up reverse blinds not yet implemented (should be easy)..."); }

   }

   public HoldemGameRoundModel(GameInfo gi, char botType, int mySeat, HoldemBotCardset c)
   {
      tableState = gi;
      myHoleCards = c;



      ValidateImplementation();
      /*
       1.  Instantiate a new CardTable
       2a. Add players (add_player_clockwise) as desired and either initialize the state of the table (initialize_table), or restore_state
       2b. Either initialize (initialize_table) the state of the table, or restore (restore_state) it from a previous state
       3.  You may save (save_state) the state of the table at this point, if you want to for some reason.
       *
          4.1 BeginNewHand
          4.2 ShowHoleCards to all bots who need to know their hand
       */


      CreateNewTable(tableState.getNumSeats(), tableState.getSmallBlindSize(), tableState.getGameID());

      for(int seatnum = 1;seatnum < tableState.getNumSeats();++seatnum)
      {
         PlayerInfo seatPlayer = tableState.getPlayer(seatnum);
         double seatMoney = seatPlayer.getBankRoll();
         
         if( seatnum == mySeat ) CreateNewStrategyBot("PokerAI: " + seatPlayer.getName(), seatMoney, botType);
         else                    CreateNewHumanOpponent(seatPlayer.getName(), tableState.getPlayer(seatnum).getBankRoll());
      }

      InitializeNewTableState();

      BeginNewHands(tableState.getSmallBlindSize(), tableState.getButtonSeat());

      ShowHoleCardsToBot(mySeat, myHoleCards);

   }

   @Override
   protected void finalize() throws Throwable {
        try {
            DeleteTableAndPlayers();        // close open files
        } finally {
            super.finalize();
        }
    }

}



    //====================
    //   Initialization
    //====================


    //static native void SetMoney(long ptr_holdemCPtr, int playerNumber, double money);

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

    static native int GetHandnum(long ptr_holdemCPtr);
    */

    //static native void RestoreTableState(String stateStr, long ptr_holdemCPtr);
    //static native String SaveTableState(long ptr_holdemCPtr);
    //static native void ResetDeterministicSeed(long ptr_holdemCPtr);
    //C_DLL_FUNCTION uint32 GetDeterministicSeed(long ptr_holdemCPtr, uint8 small_int);
