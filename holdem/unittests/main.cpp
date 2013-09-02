//
//  main.cpp
//  UnitTests
//
//  Created by Joseph Huang on 2013-06-22.
//  Copyright (c) 2013 Joseph Huang. All rights reserved.
//

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
            return myBet;
        }

        virtual void SeeOppHand(const int8, const Hand&) {};
        
        virtual void SeeAction(const HoldemAction&) {};
        
        virtual void FinishHand() {};

    private:
        const std::vector<float64> bets; // a list of predetermined bets
        size_t i; // the index of the next bet to make
    }
    ;

    

    // 2013.08.30-19.58.15
    // Hand #11
    // Perspective, SpaceBot
    void testRegression_001() {

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

        DeterredGainStrategy * const botToTest = new DeterredGainStrategy(2);

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

int main(int argc, const char * argv[])
{
    // Run all unit tests.
    NamedTriviaDeckTests::testNamePockets();

    RegressionTests::testRegression_001();
    
}

