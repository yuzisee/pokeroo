//
//  main.cpp
//  UnitTests
//
//  Created by Joseph Huang on 2013-06-22.
//  Copyright (c) 2013 Joseph Huang. All rights reserved.
//

#include <ctime>
#include <cassert>


#include "aiCache.h"
namespace NamedTriviaDeckTests {
    
    void testNamePockets() {
        
        CommunityPlus onlyCommunity(CommunityPlus::EMPTY_COMPLUS);
        CommunityPlus withCommunity(CommunityPlus::EMPTY_COMPLUS);
        
        // [0] [1] [2] [3] [4] [5] ...
        //  2S  2H  2C  2D  3S  3H ...
        DeckLocation dealt;
        
        // [18] == 6C
        dealt.Value	= 32;
        dealt.Suit = 2;
        dealt.Rank = 5;
        withCommunity.AddToHand(dealt);
        
        // [11] == 4D
        dealt.Value	= 8;
        dealt.Suit = 3;
        dealt.Rank = 3;
        withCommunity.AddToHand(dealt);
        
        
        NamedTriviaDeck o;
        o.OmitCards(withCommunity);
        o.DiffHand(onlyCommunity);
        o.sortSuits();
        
        
        string actual = o.NamePockets();
        string expected = "64x";
        
        assert(expected == actual);
    }
}

#include "arena.h"
#include "arenaEvents.h"
#include "stratPosition.h"
namespace RegressionTests {

    // Deal a fixed, pre-determined set of hands (and community)
    class FixedHands : public GameDeck {
    public:
        FixedHands(const DeckLocation *cards, size_t n) : cards(cards), n(n), i(0) {}

        virtual void ShuffleDeck() {assert(false);};
        virtual void ShuffleDeck(uint32) {assert(false);};

        virtual float64 DealCard(Hand& h) {
            assert(i < n);
            h.AddToHand(cards[i]);
            ++i;
            return 1.0;
        }
    private:
        const DeckLocation *cards; // a list of predetermined cards to deal
        const size_t n; // the size of the array this->cards
        size_t i; // the index of the next bet to make
    }
    ;
 

    // Make a fixed, pre-determined series of bets.
    class FixedReplayPlayerStrategy : public PlayerStrategy
    {
	public:

		FixedReplayPlayerStrategy(const std::vector<float64> bets) : bets(bets), i(0) {}
		virtual ~FixedReplayPlayerStrategy(){};

        virtual void SeeCommunity(const Hand&, const int8) {};


        virtual float64 MakeBet() {
            assert(i < bets.size());
            const float64 myBet = bets[i];
            ++i;
            if (myBet == myBet) {
                return myBet;
            } else {
                // it's nan, which means CALL
                return ViewTable().GetBetToCall();
            }
        }

        virtual void SeeOppHand(const int8, const Hand&) {};
        
        virtual void SeeAction(const HoldemAction&) {};
        
        virtual void FinishHand() {};

    private:
        const std::vector<float64> bets; // a list of predetermined bets
        size_t i; // the index of the next bet to make
    }
    ;

    void assertDerivative(IFunctionDifferentiable &f, float64 xa, float64 xb, float64 eps_rel) {
        const float64 ya = f.f(xa);
        const float64 yb = f.f(xb);
        const float64 xd = (xa + xb)/2.0;
        const float64 yd = f.f(xd);
        const float64 actual = f.fd(xd, yd);
        const float64 expected = (yb - ya) / (xb - xa);
        assert(fabs(expected - actual) < fabs(expected) * eps_rel);
    }



    // You'll fold Ks Kd pre-flop after raising to $705 yourself only because you got reraised an extra ~$100?
    void testRegression_014a() {

        /*

         Next Dealer is GearBotV
         ================================================================
         ============================New Hand #15========================
         BEGIN


         Preflop
         (Pot: $0)
         (9 players)
         [ActionBotV $810]
         [NormalBotV $1630]
         [TrapBotV $1485]
         [Ali $1945]
         [DangerBotV $1590]
         [MultiBotV $1485]
         [ConservativeBotV $1585]
         [SpaceBotV $1500]
         [GearBotV $1470]

         */


        struct BlindValues b;
        b.SetSmallBigBlind(5.0);

        HoldemArena myTable(b.GetSmallBlind(), std::cout, true, true);
        myTable.setSmallestChip(5.0);

        const std::vector<float64> foldOnly({0});
        const std::vector<float64> nA({365, 1045});
        FixedReplayPlayerStrategy gS(foldOnly);
        FixedReplayPlayerStrategy tS(foldOnly);
        FixedReplayPlayerStrategy dS(foldOnly);
        FixedReplayPlayerStrategy pS(foldOnly);
        FixedReplayPlayerStrategy nS(nA);
        FixedReplayPlayerStrategy mS(foldOnly);
        FixedReplayPlayerStrategy cS(foldOnly);
        FixedReplayPlayerStrategy sS(foldOnly);


        PlayerStrategy * const botToTest = new PureGainStrategy(2); 

        myTable.ManuallyAddPlayer("ActionBot14", 810.0, botToTest);
        myTable.ManuallyAddPlayer("NormalBotV", 1630.0, &nS);
        myTable.ManuallyAddPlayer("TrapBotV", 1485.0, &tS);
        myTable.ManuallyAddPlayer("Ali", 1945.0, &pS); // Ali is the dealer, since DangerBot is the small blind
        myTable.ManuallyAddPlayer("DangerBotV", 1590.0, &dS);
        myTable.ManuallyAddPlayer("MultiBotV", 1485.0, &mS);
        myTable.ManuallyAddPlayer("ConservativeBotV", 1585.0, &cS);
        myTable.ManuallyAddPlayer("SpaceBotV", 1500.0, &sS);
        myTable.ManuallyAddPlayer("GearBotV", 1470.0, &gS);

        const playernumber_t dealer = 8;


        DeckLocation card;

        {
            CommunityPlus handToTest; // Ks Kd

            card.SetByIndex(44);
            handToTest.AddToHand(card);

            card.SetByIndex(47);
            handToTest.AddToHand(card);

            botToTest->StoreDealtHand(handToTest);
        }


        myTable.BeginInitialState(14);
        myTable.BeginNewHands(b, false, dealer);


        /*

         ActionBotV posts SB of $5 ($5)
         NormalBotV posts BB of $10 ($15)
         TrapBotV folds
         Ali folds
         DangerBotV folds
         MultiBotV folds
         ConservativeBotV folds
         SpaceBotV folds
         GearBotV folds
         ActionBotV raises to $55 ($65)
         NormalBotV raises to $365 ($420)
         ActionBotV raises to $705 ($1070)
         NormalBotV raises to $1045 ($1750)
         ActionBotV folds
         
         All fold! NormalBotV wins $705
         
         
         */
        assert (myTable.PlayRound_BeginHand() != -1);
        
    }

    // You're pushing this hard pre-flop with Jc Qd? Why
    void testRegression_013a() {

/*

 Next Dealer is GearBotV
 ================================================================
 ============================New Hand #15========================
 BEGIN


 Preflop
 (Pot: $0)
 (9 players)
 [ActionBotV $810]
 [NormalBotV $1630]
 [TrapBotV $1485]
 [Ali $1945]
 [DangerBotV $1590]
 [MultiBotV $1485]
 [ConservativeBotV $1585]
 [SpaceBotV $1500]
 [GearBotV $1470]
 
 */


        struct BlindValues b;
        b.SetSmallBigBlind(5.0);

        HoldemArena myTable(b.GetSmallBlind(), std::cout, true, true);
        myTable.setSmallestChip(5.0);

        const std::vector<float64> foldOnly({0});
        const std::vector<float64> aA({55, 705, std::nan("")});
        FixedReplayPlayerStrategy gS(foldOnly);
        FixedReplayPlayerStrategy tS(foldOnly);
        FixedReplayPlayerStrategy dS(foldOnly);
        FixedReplayPlayerStrategy pS(foldOnly);
        FixedReplayPlayerStrategy aS(aA);
        FixedReplayPlayerStrategy mS(foldOnly);
        FixedReplayPlayerStrategy cS(foldOnly);
        FixedReplayPlayerStrategy sS(foldOnly);


        //PlayerStrategy * const botToTest = new DeterredGainStrategy(0);
        PlayerStrategy * const botToTest = new PureGainStrategy(3); // originally ImproveGainStrategy(2);

        myTable.ManuallyAddPlayer("ActionBotV", 810.0, &aS);
        myTable.ManuallyAddPlayer("NormalBot13", 1630.0, botToTest);
        myTable.ManuallyAddPlayer("TrapBotV", 1485.0, &tS);
        myTable.ManuallyAddPlayer("Ali", 1945.0, &pS); // Ali is the dealer, since DangerBot is the small blind
        myTable.ManuallyAddPlayer("DangerBotV", 1590.0, &dS);
        myTable.ManuallyAddPlayer("MultiBotV", 1485.0, &mS);
        myTable.ManuallyAddPlayer("ConservativeBotV", 1585.0, &cS);
        myTable.ManuallyAddPlayer("SpaceBotV", 1500.0, &sS);
        myTable.ManuallyAddPlayer("GearBotV", 1470.0, &gS);

        const playernumber_t dealer = 8;


        DeckLocation card;

        {
            CommunityPlus handToTest; // Qd Jc

            card.SetByIndex(38);
            handToTest.AddToHand(card);

            card.SetByIndex(43);
            handToTest.AddToHand(card);

            botToTest->StoreDealtHand(handToTest);
        }
        
        
        myTable.BeginInitialState(13);
        myTable.BeginNewHands(b, false, dealer);
        

        /*

 ActionBotV posts SB of $5 ($5)
 NormalBotV posts BB of $10 ($15)
 TrapBotV folds
 Ali folds
 DangerBotV folds
 MultiBotV folds
 ConservativeBotV folds
 SpaceBotV folds
 GearBotV folds
 ActionBotV raises to $55 ($65)
 NormalBotV raises to $365 ($420)
 ActionBotV raises to $705 ($1070)
 NormalBotV raises to $1045 ($1750)
 ActionBotV folds

 All fold! NormalBotV wins $705


 */
        if (myTable.PlayRound_BeginHand() == -1) {
            assert(myTable.GetPotSize() < 100);
        } else {
            assert(myTable.GetPotSize() < 200);
        }

    }

    // When you have top pair on the river and there are no flush threats and not a significant straight threat, shouldn't you bet?
    void testRegression_012() {

        /*
        Next Dealer is Ali
        ================================================================
        ============================New Hand #1========================
        BEGIN


        Preflop
        (Pot: $0)
        (9 players)
        [DangerBotV $1500]
        [MultiBotV $1500]
        [ConservativeBotV $1500]
        [SpaceBotV $1500]
        [GearBotV $1500]
        [ActionBotV $1500]
        [NormalBotV $1500]
        [TrapBotV $1500]
        [Ali $1500]
*/

        struct BlindValues b;
        b.SetSmallBigBlind(5.0);

        HoldemArena myTable(b.GetSmallBlind(), std::cout, true, true);
        myTable.setSmallestChip(5.0);

        const std::vector<float64> foldOnly({0});
        const std::vector<float64> mA({std::nan(""), std::nan(""), std::nan(""), std::nan(""), std::nan(""), std::nan("")});
        FixedReplayPlayerStrategy gS(foldOnly);
        FixedReplayPlayerStrategy tS(foldOnly);
        FixedReplayPlayerStrategy dS(foldOnly);
        FixedReplayPlayerStrategy pS(foldOnly);
        FixedReplayPlayerStrategy aS(foldOnly);
        FixedReplayPlayerStrategy mS(mA);
        FixedReplayPlayerStrategy cS(foldOnly);
        FixedReplayPlayerStrategy sS(foldOnly);


        PlayerStrategy * const botToTest = new PureGainStrategy(4);

        myTable.ManuallyAddPlayer("GearBotV", 1500.0, &gS);
        myTable.ManuallyAddPlayer("ActionBotV", 1500.0, &aS);
        myTable.ManuallyAddPlayer("NormalBot12", 1500.0, botToTest);
        myTable.ManuallyAddPlayer("TrapBotV", 1500.0, &tS);
        myTable.ManuallyAddPlayer("Ali", 1500.0, &pS); // Ali is the dealer, since DangerBot is the small blind
        myTable.ManuallyAddPlayer("DangerBotV", 1500.0, &dS);
        myTable.ManuallyAddPlayer("MultiBotV", 1500.0, &mS);
        myTable.ManuallyAddPlayer("ConservativeBotV", 1500, &cS);
        myTable.ManuallyAddPlayer("SpaceBotV", 1500.0, &sS);

        const playernumber_t dealer = 4;


        DeckLocation card;

        {
            CommunityPlus handToTest; // Ts Ah

            card.SetByIndex(32);
            handToTest.AddToHand(card);

            card.SetByIndex(49);
            handToTest.AddToHand(card);

            botToTest->StoreDealtHand(handToTest);
        }

        
        myTable.BeginInitialState(11);
        myTable.BeginNewHands(b, false, dealer);
        

         /*

        DangerBotV posts SB of $5 ($5)
        MultiBotV posts BB of $10 ($15)
        ConservativeBotV folds
        SpaceBotV folds
        GearBotV folds
        ActionBotV folds
        NormalBotV calls $10 ($25)
        TrapBotV folds
        Ali folds
        DangerBotV folds
        MultiBotV checks
          */

        assert(myTable.PlayRound_BeginHand() != -1);

        /*
    Flop:	4d 7h 9d    (Pot: $25)
        (2 players)
        [MultiBotV $1490]
        [NormalBotV $1490]
         */


        CommunityPlus myFlop;

        card.SetByIndex(11);
        myFlop.AddToHand(card);

        card.SetByIndex(21);
        myFlop.AddToHand(card);

        card.SetByIndex(31);
        myFlop.AddToHand(card);
        

        /*

        MultiBotV checks
        NormalBotV checks
*/
        assert(myTable.PlayRound_Flop(myFlop) != -1);
        /*
    Turn:	4d 7h 9d Ad   (Pot: $25)
        (2 players)
        [MultiBotV $1490]
        [NormalBotV $1490]
         */

        DeckLocation myTurn; // Ad
        myTurn.SetByIndex(51);

        /*

        MultiBotV checks
        NormalBotV checks
         */
        assert(myTable.PlayRound_Turn(myFlop, myTurn) != -1);

/*
    River:	4d 7h 9d Ad 8h  (Pot: $25)
        (2 players)
        [MultiBotV $1490]
        [NormalBotV $1490]
*/

            DeckLocation myRiver; // 8h
            myRiver.SetByIndex(25);

 /*

        MultiBotV checks
        NormalBotV checks
*/
            assert(myTable.PlayRound_River(myFlop, myTurn, myRiver) != -1);
        //assert(30 < myTable.GetPotSize());

        /*
        ----------
        |Showdown|
        ----------
        Final Community Cards:
        4d 7h 8h 9d Ad 
        
        
        
        MultiBotV reveals: 3c 7s 
        Making,
        007		Pair of Sevens
        7 7 8 3 4 9 A 	AKQJT98765432
        s h h c d d d 	1----11------
        
        NormalBotV reveals: Ts Ah 
        Making,
        014		Pair of Aces
        T 7 8 A 4 9 A 	AKQJT98765432
        s h h h d d d 	----111------
        
        NormalBotV can win 15 or less	(controls 25 of 25)
        * * *Comparing hands* * *
        NormalBotV takes 25 from the pot, earning 15 this round
        
*/
    }

    // DangerBot bets a lot 3-handed
    // Why doesn't Pessimistic trigger at such bet sizes?
    //  --> Why doesn't OpponentHandOpportunity return extra hands at this bet size?
    //  HYPOTHESIS: It's because if your win percentage is tied to mean and there are multiple players, your profit can't make it worthwhile to wait.
    //              In practice, we can either switch this to RANK when {1 < opponents} or we can make it like Pessimistic where they only need to want to beat you. 
    void testRegression_011() {
/*
        Preflop
        (Pot: $0)
        (9 players)
        [DangerBotV $1500]
        [MultiBotV $1500]
        [ConservativeBotV $1500]
        [SpaceBotV $1500]
        [GearBotV $1500]
        [ActionBotV $1500]
        [NormalBotV $1500]
        [TrapBotV $1500]
        [Ali $1500]

        DangerBotV posts SB of $5 ($5)
        MultiBotV posts BB of $10 ($15)
 */

        struct BlindValues b;
        b.SetSmallBigBlind(5.0);

        HoldemArena myTable(b.GetSmallBlind(), std::cout, true, true);
        myTable.setSmallestChip(5.0);

        const std::vector<float64> foldOnly({0});
        const std::vector<float64> pA({std::nan(""), std::nan("")});
        FixedReplayPlayerStrategy gS(foldOnly);
        FixedReplayPlayerStrategy tS(foldOnly);
        FixedReplayPlayerStrategy nS(foldOnly);
        FixedReplayPlayerStrategy pS(pA);
        FixedReplayPlayerStrategy aS(foldOnly);
        FixedReplayPlayerStrategy mS(foldOnly);
        FixedReplayPlayerStrategy cS(foldOnly);
        FixedReplayPlayerStrategy sS(foldOnly);


        //PlayerStrategy * const botToTest = new DeterredGainStrategy(0);
        PlayerStrategy * const botToTest = new PureGainStrategy(2); // originally ImproveGainStrategy(2);

        myTable.ManuallyAddPlayer("GearBotV", 1500.0, &gS);
        myTable.ManuallyAddPlayer("ActionBotV", 1500.0, &aS);
        myTable.ManuallyAddPlayer("NormalBotV", 1500.0, &nS);
        myTable.ManuallyAddPlayer("TrapBotV", 1500.0, &tS);
        myTable.ManuallyAddPlayer("Ali", 1500.0, &pS); // Ali is the dealer, since DangerBot is the small blind
        myTable.ManuallyAddPlayer("DangerBot11", 1500.0, botToTest);
        myTable.ManuallyAddPlayer("MultiBotV", 1500.0, &mS);
        myTable.ManuallyAddPlayer("ConservativeBotV", 1500, &cS);
        myTable.ManuallyAddPlayer("SpaceBotV", 1500.0, &sS);

        const playernumber_t dealer = 4;


        DeckLocation card;

        {
            CommunityPlus handToTest; // Jh Qd

            card.SetByIndex(37);
            handToTest.AddToHand(card);

            card.SetByIndex(43);
            handToTest.AddToHand(card);

            botToTest->StoreDealtHand(handToTest);
        }
        
        
        myTable.BeginInitialState(11);
        myTable.BeginNewHands(b, false, dealer);
        

        /*
        ConservativeBotV folds
        SpaceBotV folds
        GearBotV folds
        ActionBotV folds
        NormalBotV folds
        TrapBotV folds
        Ali calls $10 ($25)
        DangerBotV raises to $385 ($405)
        MultiBotV folds
        Ali folds
        
        All fold! DangerBotV wins $20
        
        
*/
        myTable.PlayRound_BeginHand();
        assert(myTable.GetPotSize() < 120);

    }


    // Set up a simple situation (e.g. post-river) with FoldGainWaitLength and then FoldGainModel
    //  + make sure there is a bunch of pot money from previous rounds, to test whether past pot is correctly accounted for in winnings
    void testRegression_010() {

        DeckLocation card;

        CommunityPlus withCommunity; // Td Ad

        card.SetByIndex(35);
        withCommunity.AddToHand(card);

        card.SetByIndex(51);
        withCommunity.AddToHand(card);




        CommunityPlus communityToTest; // 2d Qd Kd 2h 2s

        card.SetByIndex(3);
        withCommunity.AddToHand(card);
        communityToTest.AddToHand(card);

        card.SetByIndex(43);
        withCommunity.AddToHand(card);
        communityToTest.AddToHand(card);

        card.SetByIndex(47);
        withCommunity.AddToHand(card);
        communityToTest.AddToHand(card);

        card.SetByIndex(0);
        withCommunity.AddToHand(card);
        communityToTest.AddToHand(card);

        card.SetByIndex(1);
        withCommunity.AddToHand(card);
        communityToTest.AddToHand(card);

        const int8 cardsInCommunity = 5;
        const float64 avgBlind = 0.5;
        const float64 pastPot = 60.0;
        const float64 myConstributionToPastPot = 5.0; // so lots of limpers out of the 60.0 that's there
        const float64 myBetThisRound = 2.0;
        const float64 iveBeenReraisedTo = 40.0;

        StatResultProbabilities statprob;

        ///Compute CallStats
        StatsManager::QueryDefense(statprob.core.handcumu,withCommunity,communityToTest,cardsInCommunity);
        statprob.core.foldcumu = statprob.core.handcumu;
        statprob.core.foldcumu.ReversePerspective();

        ///Compute CommunityCallStats
        StatsManager::QueryOffense(statprob.core.callcumu,withCommunity,communityToTest,cardsInCommunity,0);

        ///Compute WinStats
        DistrShape w_wl(0);
        DistrShape detailPCT(0);
        StatsManager::Query(0,&detailPCT,&w_wl,withCommunity,communityToTest,cardsInCommunity);
        statprob.core.playerID = 0;
        statprob.core.statmean = CombinedStatResultsGeom::ComposeBreakdown(detailPCT.mean,w_wl.mean);

        FoldWaitLengthModel fw;
        // From the opponent's point of view if he knows he's against a flush
        fw.setW( 0.0 ); // Their current hand does not pair the board so loses to an ace-high flush (but note there is a ~30% chance that they hit a pair other than a deuce and a ~4% chance of having one deuce)
        fw.meanConv = &(statprob.core.foldcumu);
        fw.amountSacrificeForced = avgBlind;
        fw.bankroll = 1000.0;
        fw.setAmountSacrificeVoluntary(myConstributionToPastPot + myBetThisRound - avgBlind);
        fw.opponents = 1; // Keep it simple for now
        fw.betSize = iveBeenReraisedTo;
        fw.prevPot = pastPot;

//    TEST:
        // Since w == 0.0, we have numHands === numOpportunities here. We'll add a separate test to test w > 0.0
        assert(fw.f(16) < 0); // Profit of waiting 15 hands is... probably too much. You're burning at ~200 chips to win what? ~100?
        
        assert(fw.f(3) > 0); // Waiting three hands costs ~20 chips, but it gives good chances of winning the 100 in the pot

        assert(fw.f(6) > 0); // Waiting six hands costs ~40 chips, but it gives great changes of winning the 100 in the pot

        // Need a test to expose whether derivatives match

        
    }


    // Need a test to expose whether grossSacrifice is too large.
    //  + make sure there is a high voluntary but high rarity so that numHands is high but numFolds is still relatively low, to test that amountSacrificedVoluntary isn't overestimated.
    void testRegression_010b() {


        const float64 avgBlind = 0.5;
        const float64 pastPot = 10.0;
        const float64 myConstributionToPastPot = 5.0; // so heads-up, battle of the blinds, no antes
        const float64 myBetThisRound = 2.0;
        const float64 iveBeenReraisedTo = 190.0;


        FoldWaitLengthModel fw;

        fw.setW( 0.75 ); // You have a decent hand, but it could be better and the re-raise was ridiculously enormous.
        fw.meanConv = nullptr;
        fw.amountSacrificeForced = avgBlind;
        fw.bankroll = 1000.0;
        fw.setAmountSacrificeVoluntary(myConstributionToPastPot + myBetThisRound - avgBlind);
        fw.opponents = 1; // Keep it simple for now
        fw.betSize = iveBeenReraisedTo;
        fw.prevPot = pastPot;

        const float64 evPlay = (pastPot + iveBeenReraisedTo) * (2.0 * fw.getW() - 1.0);
        assert(evPlay == 100.0); // sanity check only

        //    TEST:
        // Since w == 0.75, we expect to be in this situation every 4 hands. This costs us 4 avgBlinds (2.0 total) and one fold (loss of 7.0 total) every 4 hands.
        // So we lose 9.0 every 4 hands we wait.
        const float64 f1 = fw.f(1);
        assert(f1 < 0); // It's not profitable to fold this hand and play the next hand.

        const float64 f40 = fw.f(40);
        assert(f40 > evPlay); // If we wait 40 hands, we'll lose 90 chips, but put ourselves in a good position to win 200.0
        
        // Need a test to expose whether derivatives match

        { // Test dF_dAmountSacrifice per hand
            const float64 n = 40.0;
            const float64 xa = fw.amountSacrificeVoluntary - 0.01;
            const float64 xb = fw.amountSacrificeVoluntary + 0.01;
            const float64 xd = fw.amountSacrificeVoluntary;

            fw.amountSacrificeVoluntary = xa; const float64 ya = fw.f(n);
            fw.amountSacrificeVoluntary = xb; const float64 yb = fw.f(n);
            fw.amountSacrificeVoluntary = xd; fw.f(n); const float64 actual = fw.d_dC(n);
            const float64 expected = (yb - ya) / (xb - xa);
            assert(fabs(expected - actual) < fabs(expected) * 1e-12);
        }


        { // Test d_dw
            const float64 n = 40.0;
            const float64 xa = fw.getW() - 0.001;
            const float64 xb = fw.getW() + 0.001;
            const float64 xd = fw.getW();

            fw.setW( xa ); const float64 ya = fw.f(n);
            fw.setW( xb ); const float64 yb = fw.f(n);
            fw.setW( xd ); fw.f(n); const float64 actual = fw.d_dw(n);
            const float64 expected = (yb - ya) / (xb - xa);
            assert(fabs(expected - actual) < fabs(expected) * 1e-4);
        }
    }


    // Need a test to expose whether grossSacrifice is too small.
    //  + make sure there is a high blind and high rarity so that numHands is high but numFolds is still relatively low, to test that amountSacrificedVoluntary isn't underestimated.
    void testRegression_010c() {


        const float64 avgBlind = 30.0; // Blinds are 40.0/20.0, heads-up. No antes
        const float64 pastPot = 0.0; // We're pre-flop
        const float64 myConstributionToPastPot = 0.0;
        const float64 myBetThisRound = 20.0; // we're in the small blind
        const float64 iveBeenReraisedTo = 5000.0;


        FoldWaitLengthModel fw;

        fw.setW(0.75); // You have a decent hand, but it could be better and the raise was large.
        fw.meanConv = nullptr;
        fw.amountSacrificeForced = avgBlind;
        fw.bankroll = 10000.0;
        fw.setAmountSacrificeVoluntary(myConstributionToPastPot + myBetThisRound - avgBlind);
        fw.opponents = 1; // Keep it simple for now
        fw.betSize = iveBeenReraisedTo;
        fw.prevPot = pastPot;

        const float64 evPlay = (pastPot + iveBeenReraisedTo) * (2.0 * fw.getW() - 1.0);
        assert(evPlay == 2500.0); // sanity check only

        //    TEST:
        // Since w == 0.75, we expect the opponent to be in this situation every 4 hands.
        // Each time the opponent is in this situation, we except to also be in this situation one out of every 4 of those.
        // This, we can repeat this situation every 16 hands if we want, and wait for a better hand next time.
        // This costs us 16 avgBlinds (480.0 total) and one fold (loss of 0.0 total) every 16 hands.
        // So we lose 480.0 every 16 hands we wait.

    /*
        // Argument: But we should only expense this blind once out of every 4 hands. Other hands we should break even (each player on averaging having the best hand once, winning the blinds).
        // Rebuttal: But we will only receive a raise of 5000.0 with some frequency too.
        // If we were truly heads-up and we choose to play every Nth hand, we _do_ change our average hand strength but we do sacrifice every blind -- not just the ones we play.
     */
        const float64 f1 = fw.f(1);
        assert(f1 < 0); // It's not profitable to fold this hand and play the next hand.

        const float64 f20 = fw.f(20);
        // If we play now, we can win EV 2500.0 chips based on a 75% gamble on 5000.0 chips.
        // If we wait 20 hands, we sacrifice 20 big blinds, which loses 600.0 (not 150.0), but gives us a 80% gamble on 5000.0 which nets EV 3000.0
        // It's 500.0 chips more profitable of a gamble, but costs 600.0 to get there.
        assert(0.0 < f20);
        assert(f20 < evPlay);

        const float64 f40 = fw.f(40);
        assert(f40 > evPlay); // If we wait 40 hands, we'll lose 1200.0 chips, but put ourselves in a 90% chance position to win 5000.0 (= EV 4000.0), which is better than taking a 75% gamble on 5000.0 now.

        // Need a test to expose whether derivatives match
        assertDerivative(fw, 39.99, 40.01, 1e-6);

        { // Test d_dBetsize
            const float64 n = 40.0;
            const float64 xa = fw.betSize - 0.01;
            const float64 xb = fw.betSize + 0.01;
            const float64 xd = fw.betSize;

            fw.betSize = xa; const float64 ya = fw.f(n);
            fw.betSize = xb; const float64 yb = fw.f(n);
            fw.betSize = xd; fw.f(n); const float64 actual = fw.d_dbetSize(n);
            const float64 expected = (yb - ya) / (xb - xa);
            assert(fabs(expected - actual) < fabs(expected) * 1e-10);
        }

    }


    // Test OpposingHandOpportunity derivatives
    void testRegression_008() {

        /*
        Preflop
        (Pot: $0)
        (4 players)
        [A $100.0]
        [B $150.0]
        [C $75.0]
        [D $22.0]


        A posts SB of $0.125 ($0.125)
        B posts BB of $0.25 ($0.375)
        */

        struct BlindValues b;
        b.SetSmallBigBlind(0.125);

        HoldemArena myTable(b.GetSmallBlind(), std::cout, true, true);
        myTable.setSmallestChip(0.125);

        const std::vector<float64> aC({0.25});
        FixedReplayPlayerStrategy cS(aC);
        FixedReplayPlayerStrategy dS(aC);

        FixedReplayPlayerStrategy aS(aC);
        FixedReplayPlayerStrategy bS(aC);

        myTable.ManuallyAddPlayer("A", 100.0, &aS);
        myTable.ManuallyAddPlayer("B", 150.0, &bS);
        myTable.ManuallyAddPlayer("C", 75.0, &cS);
        myTable.ManuallyAddPlayer("D", 22.0, &dS);
        const playernumber_t dealer = 3;


        myTable.BeginInitialState();
        myTable.BeginNewHands(b, false, dealer);


        /*
         C calls $0.25 ($0.625)
         D calls $0.25 ($0.875)
         A calls $0.25 ($1.0)
         B checks
         */
        assert(myTable.PlayRound_BeginHand() != -1);

        //myTable.PrepBettingRound(false,2); //turn, river remaining

        // ===

        DeckLocation card;

        CommunityPlus withCommunity; // 3c Qc

        card.SetByIndex(6);
        withCommunity.AddToHand(card);

        card.SetByIndex(42);
        withCommunity.AddToHand(card);

        std::cout << "Starting next round..." << endl;

        CommunityPlus communityToTest; // Jc Ks Ah

        card.SetByIndex(38);
        withCommunity.AddToHand(card);
        communityToTest.AddToHand(card);

        card.SetByIndex(44);
        withCommunity.AddToHand(card);
        communityToTest.AddToHand(card);

        card.SetByIndex(49);
        withCommunity.AddToHand(card);
        communityToTest.AddToHand(card);

        const int8 cardsInCommunity = 3;


        StatResultProbabilities statprob;

        ///Compute CallStats
        StatsManager::QueryDefense(statprob.core.handcumu,withCommunity,communityToTest,cardsInCommunity);
        statprob.core.foldcumu = statprob.core.handcumu;
        statprob.core.foldcumu.ReversePerspective();

        ///Compute CommunityCallStats
        StatsManager::QueryOffense(statprob.core.callcumu,withCommunity,communityToTest,cardsInCommunity,0);


        const float64 testBet = 2.0;
        
        OpponentHandOpportunity test(1, myTable, statprob.core);



        test.query(testBet);
        const float64 actual_y = test.handsToBeat();
        const float64 actual_Dy = test.d_HandsToBeat_dbetSize();

        assert(actual_y >= 3.5);
        assert(actual_Dy > 0); // betting more should increase N even more

        CombinedStatResultsPessimistic testC(test, statprob.core);
        //testC.query(testBet);

        const float64 s1 = testC.ViewShape(testBet).splits;
        const float64 w = testC.getWinProb(testBet);
        const float64 l = testC.getLoseProb(testBet);
        const float64 dw = testC.get_d_WinProb_dbetSize(testBet);
        const float64 dl = testC.get_d_LoseProb_dbetSize(testBet);

        assert(testC.ViewShape(testBet).wins + testC.ViewShape(testBet).splits + testC.ViewShape(testBet).loss == 1.0);

        assert(w < 0.1);
        assert(l+w > 0.86);
        assert(s1 < 0.25);
        assert(dw < 0);
        assert(dl == -dw);
    }



    // Test callcumu sanity checks
    void testRegression_007c() {

        DeckLocation card;

        CommunityPlus withCommunity; // Kd Ad

        card.SetByIndex(47);
        withCommunity.AddToHand(card);

        card.SetByIndex(51);
        withCommunity.AddToHand(card);




        CommunityPlus communityToTest;

        const int8 cardsInCommunity = 0;


        StatResultProbabilities statprob;

        ///Compute CallStats
        StatsManager::QueryDefense(statprob.core.handcumu,withCommunity,communityToTest,cardsInCommunity);
        statprob.core.foldcumu = statprob.core.handcumu;
        statprob.core.foldcumu.ReversePerspective();

        ///Compute CommunityCallStats
        StatsManager::QueryOffense(statprob.core.callcumu,withCommunity,communityToTest,cardsInCommunity,0);

        ///Compute WinStats
        DistrShape w_wl(0);
        DistrShape detailPCT(0);
        StatsManager::Query(0,&detailPCT,&w_wl,withCommunity,communityToTest,cardsInCommunity);
        statprob.core.playerID = 0;
        statprob.core.statmean = CombinedStatResultsGeom::ComposeBreakdown(detailPCT.mean,w_wl.mean);

        // TEST (from CACHE!!):
        assert(statprob.core.statRelation().pct > 0.94); // This hand is very good. StatRelation should do very well.
        assert(statprob.core.statRanking().pct > 0.96); // This is one of the best hands you can have in this situation. StatRanking should do very well.

        // Against most opponents, I have a strong chance to win.
        assert(statprob.core.getFractionOfOpponentsWhereMyWinrateIsAtLeast(0.6) > 0.93);

        // 19th strongest out of 20 hands
        const float64 actualWinPct = statprob.core.callcumu.nearest_winPCT_given_rank(0.95);
        assert(actualWinPct < 0.8); // Such a hand has about a 65% chance to win? Even aces have only 70 something right?
        assert(0.6457 < actualWinPct);
        
    }


    // Test foldcumu sanity checks
    void testRegression_007b() {

        DeckLocation card;

        CommunityPlus withCommunity; // Td Ad

        card.SetByIndex(35);
        withCommunity.AddToHand(card);

        card.SetByIndex(51);
        withCommunity.AddToHand(card);




        CommunityPlus communityToTest; // 2d Qd Kd

        card.SetByIndex(3);
        withCommunity.AddToHand(card);
        communityToTest.AddToHand(card);

        card.SetByIndex(43);
        withCommunity.AddToHand(card);
        communityToTest.AddToHand(card);

        card.SetByIndex(47);
        withCommunity.AddToHand(card);
        communityToTest.AddToHand(card);

        const int8 cardsInCommunity = 3;


        StatResultProbabilities statprob;

        ///Compute CallStats
        StatsManager::QueryDefense(statprob.core.handcumu,withCommunity,communityToTest,cardsInCommunity);
        statprob.core.foldcumu = statprob.core.handcumu;
        statprob.core.foldcumu.ReversePerspective();

        ///Compute CommunityCallStats
        StatsManager::QueryOffense(statprob.core.callcumu,withCommunity,communityToTest,cardsInCommunity,0);

        ///Compute WinStats
        DistrShape w_wl(0);
        DistrShape detailPCT(0);
        StatsManager::Query(0,&detailPCT,&w_wl,withCommunity,communityToTest,cardsInCommunity);
        statprob.core.playerID = 0;
        statprob.core.statmean = CombinedStatResultsGeom::ComposeBreakdown(detailPCT.mean,w_wl.mean);

        // TEST:
        assert(statprob.core.statRelation().pct > 0.95); // This hand is very good. StatRelation should do very well.
        assert(statprob.core.statRanking().pct > 0.97); // This is one of the best hands you can have in this situation. StatRanking should do very well.

        // Against most opponents, I have a very strong chance to win.
        assert(statprob.core.getFractionOfOpponentsWhereMyWinrateIsAtLeast(0.85) > 0.9);

    }
    


    // Test oddsAgainstBestXHands derivative
    void testRegression_007() {

        DeckLocation card;

        CommunityPlus withCommunity; // Td Ad

        card.SetByIndex(35);
        withCommunity.AddToHand(card);

        card.SetByIndex(51);
        withCommunity.AddToHand(card);




        CommunityPlus communityToTest; // 2d Qd Kc
        
        card.SetByIndex(3);
        withCommunity.AddToHand(card);
        communityToTest.AddToHand(card);

        card.SetByIndex(43);
        withCommunity.AddToHand(card);
        communityToTest.AddToHand(card);

        card.SetByIndex(46);
        withCommunity.AddToHand(card);
        communityToTest.AddToHand(card);

        const int8 cardsInCommunity = 3;
        

        StatResultProbabilities statprob;

        ///Compute CallStats
        StatsManager::QueryDefense(statprob.core.handcumu,withCommunity,communityToTest,cardsInCommunity);
        statprob.core.foldcumu = statprob.core.handcumu;
        statprob.core.foldcumu.ReversePerspective();


        // TEST:
        const float64 xa = 0.3;
        const float64 xb = 0.3001;
        std::pair<StatResult, float64> ya = statprob.core.handcumu.bestXHands(xa);
        std::pair<StatResult, float64> yb = statprob.core.handcumu.bestXHands(xb);

        const float64 expected = (yb.first.pct - ya.first.pct) / (xb - xa);
        const float64 actual = (ya.second + yb.second) / 2.0;

        assert(fabs(actual - expected) < fabs(expected) * 1.0e-7);
    }


    // The issue here is ActionBot calls a bet with (known?) zero percent chance of winning.
    // ImproveGainStrategy never passed this, and was retired in favour of PureGainStrategy because of it.
    void testRegression_006() {
        /*


         Preflop
         (Pot: $0)
         (8 players)
         [GearBotV $1488.75]
         [ActionBotV $3031.88]
         [NormalBotV $2240]
         [Ali $1500]
         [DangerBotV $1495]
         [MultiBotV $760]
         [ConservativeBotV $1484.38]
         [SpaceBotV $1500]
         

         GearBotV posts SB of $5.625 ($5.625)
         ActionBotV posts BB of $11.25 ($16.875)
*/

        struct BlindValues b;
        b.SetSmallBigBlind(5.625);

        HoldemArena myTable(b.GetSmallBlind(), std::cout, true, true);
        myTable.setSmallestChip(5.0);

        const std::vector<float64> foldOnly({0});
        const std::vector<float64> nA({11.25, 80.0, 30.0, 125.0});
        FixedReplayPlayerStrategy gS(foldOnly);
        
        FixedReplayPlayerStrategy nS(nA);
        FixedReplayPlayerStrategy pS(foldOnly);
        FixedReplayPlayerStrategy dS(foldOnly);
        FixedReplayPlayerStrategy mS(foldOnly);
        FixedReplayPlayerStrategy cS(foldOnly);
        FixedReplayPlayerStrategy sS(foldOnly);


        PlayerStrategy * const botToTest = new PureGainStrategy(4);

        myTable.ManuallyAddPlayer("GearBotV", 1488.75, &gS);
        myTable.ManuallyAddPlayer("ActionBot6", 3031.88, botToTest);
        myTable.ManuallyAddPlayer("NormalBotV", 2240.0, &nS); // NormalBot is the dealer, since GearBot is the small blind
        myTable.ManuallyAddPlayer("Ali", 1500.0, &pS);
        myTable.ManuallyAddPlayer("DangerBotV", 1495.0, &dS);
        myTable.ManuallyAddPlayer("MultiBotV", 760.0, &mS);
        myTable.ManuallyAddPlayer("ConservativeBotV", 1484.38, &cS);
        myTable.ManuallyAddPlayer("SpaceBotV", 1500.0, &sS);

        const playernumber_t dealer = 7;


        DeckLocation card;

        {
            CommunityPlus handToTest; // 3c Qc

            card.SetByIndex(6);
            handToTest.AddToHand(card);

            card.SetByIndex(42);
            handToTest.AddToHand(card);

            botToTest->StoreDealtHand(handToTest);
        }


        myTable.BeginInitialState(6);
        myTable.BeginNewHands(b, false, dealer);


        /*
         NormalBotV calls $11.25 ($28.125)
         Ali folds
         DangerBotV folds
         MultiBotV folds
         ConservativeBotV folds
         SpaceBotV folds
         GearBotV folds
         ActionBotV checks
*/
        assert(myTable.PlayRound_BeginHand() != -1);



        /*
         Flop:	Jc Ks Ah    (Pot: $28.125)
*/

        CommunityPlus myFlop;

        card.SetByIndex(38);
        myFlop.AddToHand(card);

        card.SetByIndex(44);
        myFlop.AddToHand(card);

        card.SetByIndex(49);
        myFlop.AddToHand(card);

        /*
         (2 players)
         [ActionBotV $3020.62]
         [NormalBotV $2228.75]
         
         ActionBotV bets $11.25 ($39.375)
         NormalBotV raises to $80 ($119.375)
         ActionBotV calls $68.75 ($188.125)
         */

        playernumber_t highBet = myTable.PlayRound_Flop(myFlop);
        if (highBet == -1) {
            // All fold. In this case, make sure ActionBot folded.
            assert(botToTest->ViewPlayer().GetBetSize() < nS.ViewPlayer().GetBetSize());
            return;
        }
/*
         Turn:	Jc Ks Ah 9d   (Pot: $188.125)
 */
        DeckLocation myTurn; // 9d
        myTurn.SetByIndex(31);
        
        /*
         (2 players)
         [ActionBotV $2940.62]
         [NormalBotV $2148.75]

         ActionBotV checks
         NormalBotV bets $30 ($218.125)
         ActionBotV calls $30 ($248.125)
*/
        assert(myTable.PlayRound_Turn(myFlop, myTurn) != -1);
        
        /*
         River:	Jc Ks Ah 9d Ad  (Pot: $248.125)
         */
        DeckLocation myRiver; // Ad
        myRiver.SetByIndex(51);
        /*

         (2 players)
         [ActionBotV $2910.62]
         [NormalBotV $2118.75]

         ActionBotV checks
         NormalBotV bets $125 ($373.125)
         ActionBotV calls $125 ($498.125)
         */
        assert(myTable.PlayRound_River(myFlop, myTurn, myRiver) == -1);
        // NOTE: Other than the bet of 80.0 after the flop, all of these bets are relatively low compared to the pot, and we call and don't raise.
        // If you assume that we don't have any information about the opposing hand due to these small bets, then we are getting decently good odds on these small bets.
        // Therefore, go investigate the 80.0

        /*

         ----------
         |Showdown|
         ----------
         Final Community Cards:
         9d Jc Ks Ah Ad



         NormalBotV reveals: Qd Ac 
         Making,
         105		Trip Aces
         K A J A 9 Q A 	AKQJT98765432
         s h c c d d d 	-11----------
         
         ActionBotV mucks 
         
         NormalBotV can win 251.875 or less	(controls 498.125 of 498.125)
         * * *Comparing hands* * *
         NormalBotV takes 498.125 from the pot, earning 251.875 this round
         
         
         ==========
         CHIP COUNT
         ActionBotV now has $2785.62
         NormalBotV now has $2491.88
         Ali now has $1500
         SpaceBotV now has $1500
         DangerBotV now has $1495
         ConservativeBotV now has $1484.38
         GearBotV now has $1483.12
         MultiBotV now has $760
         */
    }

    // The issue here is the all-in re-raise by ConservativeBot.
    // I think foldgain uses mean while playgain uses rank -- biasing toward play and against folding.
    void testRegression_005() {
        /*

         Next Dealer is NormalBotV
         ================================================================
         ============================New Hand #2========================
         BEGIN


         Preflop
         (Pot: $0)
         (8 players)
         [GearBotV $3004.5]
         [ConservativeBotV $1500]
         [MultiBotV $1500]
         [SpaceBotV $1500]
         [ActionBotV $1500]
         [DangerBotV $1500]
         [Ali $1500]
         [NormalBotV $1495.5]
         */

        struct BlindValues b;
        b.SetSmallBigBlind(5.0625);

        HoldemArena myTable(b.GetSmallBlind(), std::cout, true, true);

        const std::vector<float64> foldOnly({0});
        const std::vector<float64> aA({10.125, 18.0, 805.5, 1489.88});
        FixedReplayPlayerStrategy gS(foldOnly);

        FixedReplayPlayerStrategy mS(foldOnly);
        FixedReplayPlayerStrategy sS(foldOnly);
        FixedReplayPlayerStrategy aS(aA);
        FixedReplayPlayerStrategy dS(foldOnly);
        FixedReplayPlayerStrategy pS(foldOnly);
        FixedReplayPlayerStrategy nS(foldOnly);


        PlayerStrategy * const botToTest = new PureGainStrategy(2); // Originally DeterredGainStrategy(0);

        myTable.ManuallyAddPlayer("GearBotV", 3004.5, &gS);
        myTable.ManuallyAddPlayer("ConservativeBotV", 1500.0, botToTest);
        myTable.ManuallyAddPlayer("MultiBotV", 1500.0, &mS); // NormalBot is the dealer, since GearBot is the small blind
        myTable.ManuallyAddPlayer("SpaceBotV", 1500.0, &sS);
        myTable.ManuallyAddPlayer("ActionBotV", 1500.0, &aS);
        myTable.ManuallyAddPlayer("DangerBotV", 1500.0, &dS);
        myTable.ManuallyAddPlayer("Ali", 1500.0, &pS);
        myTable.ManuallyAddPlayer("NormalBotV", 1495.5, &nS);
        
        const playernumber_t dealer = 7;
        myTable.setSmallestChip(4.5);

        DeckLocation card;

        {
            CommunityPlus handToTest; // 5c 7d

            card.SetByIndex(14);
            handToTest.AddToHand(card);

            card.SetByIndex(23);
            handToTest.AddToHand(card);

            botToTest->StoreDealtHand(handToTest);
        }
        
        
        myTable.BeginInitialState(5);
        myTable.BeginNewHands(b, false, dealer);
        

         /*


         GearBotV posts SB of $5.0625 ($5.0625)
         ConservativeBotV posts BB of $10.125 ($15.1875)
         MultiBotV folds
         SpaceBotV folds
         ActionBotV calls $10.125 ($25.3125)
         DangerBotV folds
         Ali folds
         NormalBotV folds
         GearBotV folds
         ConservativeBotV checks
*/

        assert(myTable.PlayRound_BeginHand() != -1);


        /*
         Flop:	4s 5h Kd    (Pot: $25.3125)
         (2 players)
         [ConservativeBotV $1489.88]
         [ActionBotV $1489.88]
         */


        CommunityPlus myFlop;

        card.SetByIndex(9);
        myFlop.AddToHand(card);

        card.SetByIndex(13);
        myFlop.AddToHand(card);

        card.SetByIndex(47);
        myFlop.AddToHand(card);
        
        /*

         ConservativeBotV checks
         ActionBotV bets $18 ($43.3125)
         ConservativeBotV raises to $112.5 ($155.812)
         ActionBotV raises to $805.5 ($943.312)
         ConservativeBotV raises all-in to $1489.88 ($2320.69)
         ActionBotV calls $684.375 ($3005.06)

         */

        const playernumber_t flopCalledIdx = myTable.PlayRound_Flop(myFlop);
        //assert(myTable.PlayRound_Flop(myFlop) != 1);
        if (flopCalledIdx != -1) {

        /*
         Turn:	4s 5h Kd Ah   (Pot: $3005.06)
         (2 players)
*/

            DeckLocation myTurn; // Ah
            myTurn.SetByIndex(49);

            assert(myTable.PlayRound_Turn(myFlop, myTurn) != 1);
        }

        /*

         River:	4s 5h Kd Ah Qd  (Pot: $3005.06)
         (2 players)


         ----------
         |Showdown|
         ----------
         Final Community Cards:
         4s 5h Qd Kd Ah



         ConservativeBotV is ahead with: 5c 7d
         Trying to stay alive, makes
         005		Pair of Fives
         4 5 A 5 7 Q K 	AKQJT98765432
         s h h c d d d 	111----------
         
         ActionBotV is ahead with: Ks As 
         Trying to stay alive, makes 
         092		Aces and Kings
         4 K A 5 A Q K 	AKQJT98765432
         s s s h h d d 	--1----------
         
         ActionBotV can win 1505.06 or less	(controls 3005.06 of 3005.06)
         * * *Comparing hands* * *
         ActionBotV takes 3005.06 from the pot, earning 1505.06 this round
         
         
         ==========
         CHIP COUNT
         ActionBotV now has $3005.06
         GearBotV now has $2999.44
         DangerBotV now has $1500
         SpaceBotV now has $1500
         MultiBotV now has $1500
         Ali now has $1500
         NormalBotV now has $1495.5
         */
    }

    void testRegression_004() {
        /*
        Next Dealer is TrapBotV
        ================================================================
        ============================New Hand #8========================
        BEGIN


        Preflop
        (Pot: $0)
        (9 players)
        [SpaceBotV $1498]
        [Nav $2960]
        [GearBotV $1507] <-- playing as --NORMAL--
        [ConservativeBotV $346]
        [DangerBotV $1496]
        [MultiBotV $2657]
        [NormalBotV $1064]
        [ActionBotV $475]
        [TrapBotV $1497]
         */

        struct BlindValues b;
        b.SetSmallBigBlind(1.0);

        HoldemArena myTable(b.GetSmallBlind(), std::cout, true, true);

        const std::vector<float64> foldOnly({0});
        const std::vector<float64> pA({std::nan(""), 0, 10.0, 28.0, 50.0, 347.0, 736.0, 927.0});
        FixedReplayPlayerStrategy sS(foldOnly);
        FixedReplayPlayerStrategy pS(pA);
        
        FixedReplayPlayerStrategy cS(foldOnly);
        FixedReplayPlayerStrategy dS(foldOnly);
        FixedReplayPlayerStrategy mS(foldOnly);
        FixedReplayPlayerStrategy nS(foldOnly);
        FixedReplayPlayerStrategy aS(foldOnly);
        FixedReplayPlayerStrategy tS(foldOnly);


        PlayerStrategy * const botToTest = new PureGainStrategy(2); // originally ImproveGainStrategy(0);

        myTable.ManuallyAddPlayer("SpaceBotV", 1498.0, &sS);
        myTable.ManuallyAddPlayer("Nav", 2960.0, &pS);
        myTable.ManuallyAddPlayer("GearBot4", 1507.0, botToTest); // TrapBot is the dealer, since SpaceBot is the small blind
        myTable.ManuallyAddPlayer("ConservativeBotV", 346.0, &cS);
        myTable.ManuallyAddPlayer("DangerBotV", 1496.0, &dS);
        myTable.ManuallyAddPlayer("MultiBotV", 2657.0, &mS);
        myTable.ManuallyAddPlayer("NormalBotV", 1064.0, &nS);
        myTable.ManuallyAddPlayer("ActionBotV", 475.0, &aS);
        myTable.ManuallyAddPlayer("TrapBotV", 1497.0, &tS);

        const playernumber_t dealer = 8;
        myTable.setSmallestChip(1.0);

        DeckLocation card;

        {
            CommunityPlus handToTest; // Qs As

            card.SetByIndex(40);
            handToTest.AddToHand(card);

            card.SetByIndex(48);
            handToTest.AddToHand(card);

            botToTest->StoreDealtHand(handToTest);
        }
        
        myTable.BeginInitialState(4);
        myTable.BeginNewHands(b, false, dealer);

        /*

        SpaceBotV posts SB of $1 ($1)
        Nav posts BB of $2 ($3)
        GearBotV raises to $5 ($8)
        ConservativeBotV folds
        DangerBotV folds
        MultiBotV folds
        NormalBotV folds
        ActionBotV folds
        TrapBotV folds
        SpaceBotV folds
        Nav calls $3 ($11)
         */
        assert(myTable.PlayRound_BeginHand() != -1);


        /*

    Flop:	6d Jd Qc    (Pot: $11)
        (2 players)
        [Nav $2955]
        [GearBotV $1502]
*/
        CommunityPlus myFlop;

        card.SetByIndex(19);
        myFlop.AddToHand(card);

        card.SetByIndex(39);
        myFlop.AddToHand(card);

        card.SetByIndex(42);
        myFlop.AddToHand(card);
        
        

        
        /*
        Nav checks
        GearBotV checks
*/
        assert(myTable.PlayRound_Flop(myFlop) != -1);

        /*
    Turn:	6d Jd Qc 9d   (Pot: $11)
        (2 players)
        [Nav $2955]
        [GearBotV $1502]
*/
        DeckLocation myTurn; // 9d
        myTurn.SetByIndex(32);

        /*
        Nav bets $10 ($21)
        GearBotV raises to $31 ($52)
        Nav calls $21 ($73)
         */
        assert(myTable.PlayRound_Turn(myFlop, myTurn) != -1);
        /*

    River:	6d Jd Qc 9d 5h  (Pot: $73)
        (2 players)
        [Nav $2924]
        [GearBotV $1471]
*/
        DeckLocation myRiver; // 5h
        myRiver.SetByIndex(13);
        /*
        Nav bets $50 ($123)
        GearBotV raises to $347 ($470)
        Nav calls $297 ($767)
*/
        playernumber_t highBet = myTable.PlayRound_River(myFlop, myTurn, myRiver);
        //assert (myTable.ViewPlayer(highBet) == &botToTest->ViewPlayer());
        assert(highBet != -1); // do not get pushed out, you have a winning hand
        /*
        ----------
        |Showdown|
        ----------
        Final Community Cards:
        5h 6d 9d Jd Qc



        GearBotV reveals: Qs As
        Making,
        012		Pair of Queens
        Q A 5 Q 6 9 J 	AKQJT98765432
        s s h c d d d 	1--1-1-------
        
        Nav mucks 
        
        GearBotV can win 384 or less	(controls 767 of 767)
        * * *Comparing hands* * *
        GearBotV takes 767 from the pot, earning 384 this round
        
        
        ==========
        CHIP COUNT
        MultiBotV now has $2657
        Nav now has $2577
        GearBotV now has $1891
        SpaceBotV now has $1497
        TrapBotV now has $1497
        DangerBotV now has $1496
        NormalBotV now has $1064
        ActionBotV now has $475
        ConservativeBotV now has $346
        
*/
    }



    /**
     * Discussion about Geom vs. Algb
     * Bet-fraction is weird mid-game because what is it a fraction of? Your remaining money? Your uncommitted money?
     *
     * It certainly would be simpler with Algb only.
     *
     *
     * Other discussion:
     *  Why don't you bet all-in? Is it because of the Kelly criterion, or is it because of optimism?
     *  Why don't you _call_ all-in? Is it because of the Kelley criterion, or is it because of optimism?
     *
     * "If someone bets all-in, and you have the option to call it, you pick your spots. You can wait for a better opportunity. Obviously you don't take this one because you are putting too much of your money at risk at once" (thus, use Kelly criterion, a.k.a. GainModel, when calling or less, and use AlgbModel, a.k.a. GainModelNoRisk for raising)
     * "If you are the one raising all-in, the deterrent for not always doing this is that you will only be called by good hands." (Thus, we should use AlgbModel as the default for raises but scale from X to statworse as your raise gets higher.)
     * -Nav
     *
     * (A) Worst could be based on handsDealt so that it is sufficiently pessimistic, if you think it's not pessimistic enough.
     * (B) Also you can cut a couple rank bots to make room for this
     */
    void testRegression_003() {

        DeckLocation card;

        CommunityPlus withCommunity; // Td Ad

        card.SetByIndex(35);
        withCommunity.AddToHand(card);

        card.SetByIndex(51);
        withCommunity.AddToHand(card);



        
        CommunityPlus communityToTest; // 2d Qd Kc 2s 7c
        
        card.SetByIndex(3);
        withCommunity.AddToHand(card);
        communityToTest.AddToHand(card);

        card.SetByIndex(43);
        withCommunity.AddToHand(card);
        communityToTest.AddToHand(card);

        card.SetByIndex(46);
        withCommunity.AddToHand(card);
        communityToTest.AddToHand(card);


        card.SetByIndex(0);
        withCommunity.AddToHand(card);
        communityToTest.AddToHand(card);


        card.SetByIndex(22);
        withCommunity.AddToHand(card);
        communityToTest.AddToHand(card);


        const int8 cardsInCommunity = 5;
        /*

         River:	2d Qd Kc 2s 7c  (Pot: $447.375)
         */

        DistrShape detailPCT(0);
        DistrShape w_wl(0);
        StatResultProbabilities statprob;

        ///Compute CallStats
        StatsManager::QueryDefense(statprob.core.handcumu,withCommunity,communityToTest,cardsInCommunity);
        statprob.core.foldcumu = statprob.core.handcumu;
        statprob.core.foldcumu.ReversePerspective();

        ///Compute CommunityCallStats
        StatsManager::QueryOffense(statprob.core.callcumu,withCommunity,communityToTest,cardsInCommunity,0);

        ///Compute WinStats
        StatsManager::Query(0,&detailPCT,&w_wl,withCommunity,communityToTest,cardsInCommunity);
        
        statprob.core.statmean = CombinedStatResultsGeom::ComposeBreakdown(detailPCT.mean,w_wl.mean);


        ///====================================
        ///   Compute Relevant Probabilities
        ///====================================
        
        statprob.Process_FoldCallMean();

        StatResult statWorse = statprob.statworse(2);
        assert(0.95 <= statWorse.loss);
    }

    void testRegression_002b() {
        ChipPositionState oppCPS(2474,7.875,0,0, 0.0);
// tableinfo->RiskLoss(0, 2474, 4, 6.75, 0, 0) = 0.0
// tableinfo->RiskLoss(0, 2474, 4, 11.25, 0, 0) == 0.0
        float64 w_r_rank0 = ExactCallD::facedOdds_raise_Geom_forTest(0.0, 1.0 /* denom */, 6.75 + oppCPS.alreadyBet, 0.0 /* RiskLoss */ , 0.31640625 /* avgBlind */
                                                                     ,oppCPS,4.5, 4,false,true,0);
        float64 w_r_rank1 = ExactCallD::facedOdds_raise_Geom_forTest(w_r_rank0, 1.0, 11.25 + oppCPS.alreadyBet, 0.0 ,  0.31640625
                                                                     ,oppCPS, 4.5, 4,false,true,0);

        // The bug is: These two values, if otherwise equal, can end up being within half-quantum.
        assert(w_r_rank0 <= w_r_rank1);
    }

    

    // 2013.08.30-19.58.15
    // Hand #11
    // Perspective, SpaceBot
    void testRegression_002() {

        /*
         BEGIN


         Preflop
         (Pot: $0)
         (8 players)
         [ConservativeBotV $344]
         [DangerBotV $1496]
         [MultiBotV $2657]
         [NormalBotV $1064]
         [ActionBotV $475]
         [SpaceBotV $717]
         [Nav $2474]
         [GearBotV $4273]
         */
        struct BlindValues b;
        b.SetSmallBigBlind(1.125);

        HoldemArena myTable(b.GetSmallBlind(), std::cout, true, true);

        const std::vector<float64> foldOnly({0});
        const std::vector<float64> pA({5.0, 12.5, 49, 168.0, 459.0, 495.0});
        FixedReplayPlayerStrategy cS(foldOnly);
        FixedReplayPlayerStrategy dS(foldOnly);
        FixedReplayPlayerStrategy mS(foldOnly);
        FixedReplayPlayerStrategy nS(foldOnly);
        FixedReplayPlayerStrategy aS(foldOnly);
        FixedReplayPlayerStrategy pS(pA);
        FixedReplayPlayerStrategy gS(foldOnly);

        PureGainStrategy * const botToTest = new PureGainStrategy(2);

        myTable.ManuallyAddPlayer("ConservativeBotV", 344.0, &cS);
        myTable.ManuallyAddPlayer("DangerBotV", 1496.0, &dS);
        myTable.ManuallyAddPlayer("MultiBotV", 2657.0, &mS);
        myTable.ManuallyAddPlayer("NormalBotV", 1064.0, &nS);
        myTable.ManuallyAddPlayer("ActionBotV", 475.0, &aS);
        myTable.ManuallyAddPlayer("SpaceBotV", 717.0, botToTest);
        myTable.ManuallyAddPlayer("Nav", 2474.0, &pS);
        myTable.ManuallyAddPlayer("GearBotV", 4273.0, &gS); // gearbot is the dealer, since ConservativeBot is the small blind

        const playernumber_t dealer = 7;
        myTable.setSmallestChip(1.0);

        myTable.BeginInitialState();
        myTable.BeginNewHands(b, false, dealer);


        // 2S 2H 2C 2D 3S 3H 3C 3D 4S 4H
        // 4C 4D 5S 5H 5C 5D 6S 6H 6C 6D
        // 7S 7H 7C 7D 8S 8H 8C 8D 9S 9H
        // 9C 9D TS TH TC TD J. J. J. J.
        // QS QH QC QD KS KH KC KD AS AH
        // AC AD
        DeckLocation card;

        {
            CommunityPlus handToTest; // Td Ad

            card.SetByIndex(35);
            handToTest.AddToHand(card);

            card.SetByIndex(51);
            handToTest.AddToHand(card);

            botToTest->StoreDealtHand(handToTest);
        }
        
        /*
         ConservativeBotV posts SB of $1.125 ($1.125)
         DangerBotV posts BB of $2.25 ($3.375)
         MultiBotV folds
         NormalBotV folds
         ActionBotV folds
         SpaceBotV raises to $5 ($8.375)
         Nav calls $5 ($13.375)
         GearBotV folds
         ConservativeBotV folds
         DangerBotV folds
         */

        myTable.PrepBettingRound(true,3);  //flop, turn, river remaining
        HoldemArenaBetting r( &myTable, CommunityPlus::EMPTY_COMPLUS, 0 );
        
        r.MakeBet(0.0);
        r.MakeBet(0.0);
        r.MakeBet(0.0);
        botToTest->MakeBet(); // ASSERT THAT OppRAISEChance isn't messed up.


        /*
        Why didn't I bet lower?
        OppRAISEChance [*] 0.284004 @ $4.5	fold -- left0.0491339  0.0491339 right
        OppRAISEChance [*] 0.284004 @ $6.75	fold -- left0.143  0.143 right
        --
        What am I expecting now?
        OppRAISEChance [*] 0.262638 @ $7.25	fold -- left0.156728  0.156728 right
        OppRAISEChance [*] 0.274272 @ $12.25	fold -- left0.494979  0.494979 right
        OppRAISEChance [*] 0.276047 @ $22.25	fold -- left0.800282  0.800282 right
        OppRAISEChance [*] 0.273322 @ $42.25	fold -- left0.855941  0.855941 right
        OppRAISEChance [*] 0.268886 @ $82.25	fold -- left0.886188  0.886188 right
        OppRAISEChance [*] 0.254511 @ $162.25	fold -- left0.902322  0.902322 right
        OppRAISEChance [*] 0.222713 @ $322.25	fold -- left0.909337  0.909337 right
        OppRAISEChance [F] 0.0312251 @ $642.25	fold -- left0.912856  0.912856 right
        OppRAISEChance [F] 0 @ $717	fold -- left0.912856  0.912856 right
        Guaranteed > $0 is in the pot for sure
            OppFoldChance% ...    0.0640157   d\0.0109861
            if playstyle is Danger/Conservative, overall utility is 0.0295592
         */
    }

    

    // 2013.08.30-19.58.15
    // Hand #11
    // Perspective, SpaceBot
    void testRegression_009() {

    /*
     BEGIN


     Preflop
     (Pot: $0)
     (8 players)
     [ConservativeBotV $344]
     [DangerBotV $1496]
     [MultiBotV $2657]
     [NormalBotV $1064]
     [ActionBotV $475]
     [SpaceBotV $717]
     [Nav $2474]
     [GearBotV $4273]
*/
        struct BlindValues b;
        b.SetSmallBigBlind(1.125);

        HoldemArena myTable(b.GetSmallBlind(), std::cout, true, true);

        const std::vector<float64> foldOnly({0});
        const std::vector<float64> pA({std::nan(""), 12.5, std::nan(""), 228.0, 459.0, 495.0});
        FixedReplayPlayerStrategy cS(foldOnly);
        FixedReplayPlayerStrategy dS(foldOnly);
        FixedReplayPlayerStrategy mS(foldOnly);
        FixedReplayPlayerStrategy nS(foldOnly);
        FixedReplayPlayerStrategy aS(foldOnly);
        FixedReplayPlayerStrategy pS(pA);
        FixedReplayPlayerStrategy gS(foldOnly);

        PlayerStrategy * const botToTest = new PureGainStrategy(2); // originally DeterredGainStrategy(2);

        myTable.ManuallyAddPlayer("ConservativeBotV", 344.0, &cS);
        myTable.ManuallyAddPlayer("DangerBotV", 1496.0, &dS);
        myTable.ManuallyAddPlayer("MultiBotV", 2657.0, &mS);
        myTable.ManuallyAddPlayer("NormalBotV", 1064.0, &nS);
        myTable.ManuallyAddPlayer("ActionBotV", 475.0, &aS);
        myTable.ManuallyAddPlayer("SpaceBotV", 717.0, botToTest);
        myTable.ManuallyAddPlayer("Nav", 2474.0, &pS);
        myTable.ManuallyAddPlayer("GearBotV", 4273.0, &gS); // gearbot is the dealer, since ConservativeBot is the small blind

        const playernumber_t dealer = 7;
        myTable.setSmallestChip(1.0);

        

        myTable.BeginInitialState(9);
        myTable.BeginNewHands(b, false, dealer);

        //  [0][1][2][3][4][5][6][7][8][9]
        //0. 2S 2H 2C 2D 3S 3H 3C 3D 4S 4H
        //1. 4C 4D 5S 5H 5C 5D 6S 6H 6C 6D
        //2. 7S 7H 7C 7D 8S 8H 8C 8D 9S 9H
        //3. 9C 9D TS TH TC TD JS JH JC JD
        //4. QS QH QC QD KS KH KC KD AS AH
        //5. AC AD
        DeckLocation card;

        {
            CommunityPlus handToTest; // Td Ad

            card.SetByIndex(35);
            handToTest.AddToHand(card);

            card.SetByIndex(51);
            handToTest.AddToHand(card);

            botToTest->StoreDealtHand(handToTest);
        }


    /*
     ConservativeBotV posts SB of $1.125 ($1.125)
     DangerBotV posts BB of $2.25 ($3.375)
     MultiBotV folds
     NormalBotV folds
     ActionBotV folds
     SpaceBotV raises to $5 ($8.375)
     Nav calls $5 ($13.375)
     GearBotV folds
     ConservativeBotV folds
     DangerBotV folds
     */
        assert(myTable.PlayRound_BeginHand() != -1);


        CommunityPlus myFlop; // 2d Qd Kc

        card.SetByIndex(3);
        myFlop.AddToHand(card);

        card.SetByIndex(43);
        myFlop.AddToHand(card);

        card.SetByIndex(46);
        myFlop.AddToHand(card);

        
        
    /*

     Flop:	2d Qd Kc    (Pot: $13.375)
     (2 players)
     [SpaceBotV $712]
     [Nav $2469]
*/

        assert(myTable.PlayRound_Flop(myFlop) != -1);
        /*
     SpaceBotV bets $2.25 ($15.625)
     Nav raises to $12.25 ($27.875)
     SpaceBotV raises to $49 ($74.625)
     Nav calls $36.75 ($111.375)
*/
        DeckLocation myTurn; // 2s
        myTurn.SetByIndex(0);
/*
     Turn:	2d Qd Kc 2s   (Pot: $111.375)
     (2 players)
     [SpaceBotV $663]
     [Nav $2420]
 */
        assert(myTable.PlayRound_Turn(myFlop, myTurn) != -1);
        /*

     SpaceBotV bets $83 ($194.375)
     Nav raises to $168 ($362.375)
     SpaceBotV calls $85 ($447.375)
*/
        DeckLocation myRiver; // 7c
        myRiver.SetByIndex(22);
        /*
     River:	2d Qd Kc 2s 7c  (Pot: $447.375)
     (2 players)
     [SpaceBotV $495]
     [Nav $2252]
         */
        assert(myTable.PlayRound_River(myFlop, myTurn, myRiver) == -1);
/*
     SpaceBotV bets $4 ($451.375)
     Nav raises to $459 ($910.375)
     SpaceBotV raises all-in to $495 ($1401.38)
     Nav calls $36 ($1437.38)
*/
        
        /*
     ----------
     |Showdown|
     ----------
     Final Community Cards:
     2s 2d 7c Qd Kc



     Nav reveals: 2h Ah
     Making,
     093	 Trip Deuces
     2 2 A 7 K 2 Q AKQJT98765432
     s h h c c d d 11-----------

     SpaceBotV turns over Td Ad
     Is eliminated after making only
     002	 Pair of Deuces
     2 7 K 2 T Q A AKQJT98765432
     s c c d d d d 111----------
     
     Nav can win 720.375 or less	(controls 1437.38 of 1437.38)
     * * *Comparing hands* * *
     Nav takes 1437.38 from the pot, earning 720.375 this round
     
     */
    }
}

static void regenerateDb() {

    const int8 cardsInCommunity = 0;



    std::vector<std::pair<DeckLocation,DeckLocation>> handList;

    // Pocket-pairs
    for (int8 pocketRank = 0; pocketRank < 13; ++ pocketRank) {
        DeckLocation card1;
        card1.SetByIndex(pocketRank * 4);

        DeckLocation card2;
        card2.SetByIndex(pocketRank * 4 + 1);

        handList.emplace_back(std::pair<DeckLocation, DeckLocation>(card1, card2));

    }

    // All non-pairs
    for (int8 firstCardRank = 0; firstCardRank < 13; ++firstCardRank) {
        for(int8 secondCardRank = firstCardRank + 1; secondCardRank < 13; ++secondCardRank) {
            // Suited
            {
                DeckLocation card1;
                card1.SetByIndex(firstCardRank * 4);

                DeckLocation card2;
                card2.SetByIndex(secondCardRank * 4 + 1);

                handList.emplace_back(std::pair<DeckLocation, DeckLocation>(card1, card2));
            }


            // Not suited
            {
                DeckLocation card1;
                card1.SetByIndex(firstCardRank * 4);

                DeckLocation card2;
                card2.SetByIndex(secondCardRank * 4);

                handList.emplace_back(std::pair<DeckLocation, DeckLocation>(card1, card2));

            }
        }

    }


    assert(handList.size() == 169);

    std::cout << "Begin.\n";
    size_t counter = 0;
    for (const std::pair<DeckLocation,DeckLocation> & holeCards : handList) {
        CommunityPlus withCommunity;

        withCommunity.AddToHand(holeCards.first);

        withCommunity.AddToHand(holeCards.second);

        NamedTriviaDeck o;
        o.OmitCards(withCommunity);
        o.DiffHand(CommunityPlus::EMPTY_COMPLUS);
        o.sortSuits();
        string handName = o.NamePockets();



        StatResultProbabilities statprob;

        {
            const time_t now = time(nullptr);
            std::cout << asctime(std::localtime(&now));
            std::cout << "Computing   " << handName << "   CallStats (.holdemC)\n";
            std::cout.flush(); // Flush for timestamping
        }

        ///Compute CallStats
        StatsManager::QueryDefense(statprob.core.handcumu,withCommunity,CommunityPlus::EMPTY_COMPLUS,cardsInCommunity);

        {
            const time_t now = time(nullptr);
            std::cout << asctime(std::localtime(&now));
            std::cout << "Computing CommunityCallStats (depends on *.holdemW)\n";
            std::cout.flush(); // Flush for timestamping
        }

        ///Compute CommunityCallStats
        StatsManager::QueryOffense(statprob.core.callcumu,withCommunity,CommunityPlus::EMPTY_COMPLUS,cardsInCommunity,0);
        ++counter;

        std::cout << "=== Complete!   " << static_cast<int>(counter) << " of " << static_cast<int>(handList.size()) << "   ===\n\n";
        std::cout.flush(); // Flush for timestamping
    }

    const time_t now = time(nullptr);
    std::cout << asctime(std::localtime(&now));


}

int main(int argc, const char * argv[])
{

 // Run all unit tests.
    NamedTriviaDeckTests::testNamePockets();


    RegressionTests::testRegression_010c();
    RegressionTests::testRegression_010b();
    RegressionTests::testRegression_010();
    RegressionTests::testRegression_008();
    RegressionTests::testRegression_007();
    RegressionTests::testRegression_007b();
    RegressionTests::testRegression_007c();


    RegressionTests::testRegression_006();
    RegressionTests::testRegression_013a();
    RegressionTests::testRegression_014a();
    RegressionTests::testRegression_012();
    RegressionTests::testRegression_011();


    RegressionTests::testRegression_005();
    RegressionTests::testRegression_004();
    RegressionTests::testRegression_002b();
    RegressionTests::testRegression_002();


    RegressionTests::testRegression_003();
    RegressionTests::testRegression_009();



    // Regenerate the DB?
    //regenerateDb();
 }

