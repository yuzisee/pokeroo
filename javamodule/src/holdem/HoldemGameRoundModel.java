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
   private long ptr_HoldemArena;

   native void CreateNewTable(int seatsAtTable, double chipDenomination, long gameID);

   native int CreateNewHumanOpponent(String playerName, double money);
   native int CreateNewStrategyBot(String playerName, double money, char botType);

   native void InitializeNewTableState();

   native void BeginNewHands(double smallBlind, int dealerOverride);
   native void ShowHoleCardsToBot(int mySeat, HoldemBotCardset myHoleCards);

   native void DeleteTableAndPlayers();


   public HoldemGameRoundModel(GameInfo gi, char botType, int mySeat, HoldemBotCardset c)
   {
      tableState = gi;
      myHoleCards = c;

      /*
       1.  Instantiate a new CardTable
       2a. Add players (add_player_clockwise) as desired and either initialize the state of the table (initialize_table), or restore_state
       2b. Either initialize (initialize_table) the state of the table, or restore (restore_state) it from a previous state
       3.  You may save (save_state) the state of the table at this point, if you want to for some reason.
       *
          4.1 BeginNewHand
          4.2 ShowHoleCards to all bots who need to know their hand
       */

      if( tableState.getAnte() > 0 ) { throw new RuntimeException("Ante not yet implemented..."); }
      if( tableState.isFixedLimit() || ! tableState.isNoLimit() ) { throw new RuntimeException("Currently only no-limit is implemented..."); }
      if( tableState.isReverseBlinds() ) { throw new RuntimeException("Heads-up reverse blinds not yet implemented (should be easy)..."); }


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
