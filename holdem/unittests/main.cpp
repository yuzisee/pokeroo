//
//  main.cpp
//  UnitTests
//
//  Created by Joseph Huang on 2013-06-22.
//  Copyright (c) 2013 Joseph Huang. All rights reserved.
//

#include <ctime>
#include <cassert>
#include <limits>

#include "../src/aiCache.h"
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
// TODO(from joseph): You could unit test "matrixbase.h"/"matrixbase.cpp" here too, if you'd like

#include "../src/callRarity.h"
#include "../src/functionmodel.h"
#include "../src/stratPosition.h"
namespace UnitTests {


    class FixedStatResult : public ICombinedStatResults {
    public:
        playernumber_t fOpponents;

        StatResult fShape;


        FixedStatResult() {}

        virtual ~FixedStatResult() {}

        playernumber_t splitOpponents() const override final { return fOpponents; }

        const StatResult & ViewShape(float64 betSize) override final { return fShape; } // per-player outcome: wins and splits are used to calculate split possibilities
        float64 getLoseProb(float64 betSize) override final {
            return 1.0 - std::pow(1.0 - fShape.loss, fOpponents);
        }
        float64 getWinProb(float64 betSize) override final {
            return std::pow(fShape.wins, fOpponents);
        }

        float64 get_d_LoseProb_dbetSize(float64 betSize) override final { return 0.0; }
        float64 get_d_WinProb_dbetSize(float64 betSize) override final { return 0.0; }
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


    // Test CoarseHistogramBin
    void testUnit_024() {

        DeckLocation card;


        CommunityPlus withCommunity; // 7h 2h

        card.SetByIndex(21);
        withCommunity.AddToHand(card);

        card.SetByIndex(1);
        withCommunity.AddToHand(card);


        CommunityPlus communityToTest; // 9c Qh Kh

        card.SetByIndex(30);
        withCommunity.AddToHand(card);
        communityToTest.AddToHand(card);

        card.SetByIndex(41);
        withCommunity.AddToHand(card);
        communityToTest.AddToHand(card);

        card.SetByIndex(45);
        withCommunity.AddToHand(card);
        communityToTest.AddToHand(card);

        const int8 cardsInCommunity = 3;


        // =================================

        StatResultProbabilities statprob;

        ///Compute CallStats
        StatsManager::QueryDefense(statprob.core.handcumu,withCommunity,communityToTest,cardsInCommunity);
        statprob.core.foldcumu = CoreProbabilities::ReversePerspective(statprob.core.handcumu);

        ///Compute CommunityCallStats
        StatsManager::QueryOffense(statprob.core.callcumu,withCommunity,communityToTest,cardsInCommunity,0);

        ///Compute WinStats
        DistrShape detailPCT(DistrShape::newEmptyDistrShape());
        StatsManager::Query(&detailPCT,withCommunity,communityToTest,cardsInCommunity);
        statprob.core.playerID = 0;
        statprob.core.statmean = detailPCT.mean;

        // ==================================

        CoarseCommunityHistogram a(detailPCT, statprob.core.statRanking());

        // >>> A = numpy.array([[0.004420167362780927, 0.02132858973531938, 0.1285728447521483], [0.4166831357048748, 0.6292105841018885, 0.8417380324989019], [1, 1, 1]])
        // >>> numpy.linalg.solve(   A, numpy.array([0.05816449589744065, 0.4831674749343575, 1.0])   )
        // array([ 1.22355938, -0.75994582,  0.53638645])

        PositionalStrategy::printCommunityOutcomes(std::cout, a, detailPCT);


    }


    // Verify Householder QR
    void testMatrixbase_023() {
        // Test matrix from http://www.cs.nthu.edu.tw/~cherung/teaching/2008cs3331/chap4%20example.pdf

        /*
        NormalizedSystemOfEquations a(4);
        float64 coefs1[] = {1, 1, 1, 1};
        float64 coefs2[] = {-1, 4, 4, -1};
        float64 coefs3[] = {4, -2, 2, 0};

        NormalizedSystemOfEquations_addEquation(a, coefs1, 0.0);
        NormalizedSystemOfEquations_addEquation(a, coefs2, 0.0);
        NormalizedSystemOfEquations_addEquation(a, coefs3, 0.0);

        a.solve();

        float64 result[4];
        struct SizedArray actual;
        actual.data = result;
        actual.size = 4;

        a.getSolution(&actual);

        assert(actual.data[0] == -0.5);
        assert(actual.data[1] == -0.5);
        assert(actual.data[2] ==  0.5);
        assert(actual.data[3] ==  0.5);
         */
    }

    // Verify RaiseRatio behaviour
    void testUnit_020() {
        // Assume the following raise pattern:

        // $1 <-- mine.
        // $2 <-- their first (this round)

        // $4

        // $8

        // $16 <-- their last (in round 4)

        // 4 rounds
        // total $30 = $2 + $4 + $8 + $16  <-- but not +$1
        RaiseRatio a(0.0001, 1.0, 30.0, 4);
        const float64 actual = a.FindRatio();
        const float64 expected = 2.0;

        assert(fabs(expected - actual) < 0.0001);

        assert(a.f(2.5) > 0.0);
        assert(a.f(1.5) < 0.0);

        assertDerivative(a, 2.49995, 2.50005, 1e-9);
        assertDerivative(a, 1.49995, 1.50005, 1e-9);
        assertDerivative(a, 1.99995, 2.00005, 1e-9);
    }

    // Sanity check that GainModelGeom probabilities add up to 1.0
    // Take some known simple win percentages and measure the outcome.
    void testUnit_016() {
        //GainModelGeom
        //GainModelNoRisk


        const float64 betFraction = 0.2;
        const float64 betSize = std::numeric_limits<float64>::signaling_NaN();
        const float64 exf = 0.5; // there's a lot of money in previous rounds and/or predicted money in this round
        const float64 f_pot = 0.1; // only a small amount is from previous rounds or from players who have folded this round



        {
            FixedStatResult a;
            // Simple case: One opponent
            a.fOpponents = 1.0;

            a.fShape.wins = 0.75;
            a.fShape.splits = 0.0;
            a.fShape.loss = 0.25;


            // Drop to 0.8, 15% the time (because betFraction == 0.2)
            // Increase to 1.5, 75% of the time (because exf == 0.5)
            float64 expected = std::pow(0.8, 0.25) * std::pow(1.5, 0.75);
            float64 actual = GainModelGeom::h(betFraction, betSize, exf, f_pot, a);
            assert(fabs(expected - actual) < fabs(expected) * 1e-14);

            float64 dx = 0.001;
            float64 dexf = 0.3;

            float64 x1 = betFraction;
            float64 x2 = betFraction + dx;
            float64 y1 = actual;
            float64 y2 = GainModelGeom::h(x2, betSize, exf + dexf * dx, f_pot, a);

            float64 xd = (x1 + x2)/2;
            float64 yd = GainModelGeom::h(xd, betSize, exf + dexf * dx/2, f_pot, a);

            float64 actualD = GainModelGeom::hdx(xd, betSize, exf + dexf * dx/2, dexf, f_pot, dx, a, yd);
            float64 expectedD = (y2 - y1)/(x2-x1);
            assert(fabs(expectedD - actualD) < fabs(expectedD) * 1e-6);

        }



        {
            FixedStatResult a;
            // Simple case: One opponent
            a.fOpponents = 1.0;

            a.fShape.wins = 0.75;
            a.fShape.splits = 0.10;
            a.fShape.loss = 0.15;


            // Drop to 0.8, 15% the time (because betFraction == 0.2)
            // Increase to 1.5, 75% of the time (because exf == 0.5)
            // Split two ways has probability 0.1 and payout 0.8 + (0.2 + 0.3) / 2
            float64 expected = std::pow(0.8, 0.15) * std::pow(1.5, 0.75) * std::pow(0.8 + 0.5 / 2, 0.1);
            float64 actual = GainModelGeom::h(betFraction, betSize, exf, f_pot, a);
            assert(fabs(expected - actual) < fabs(expected) * 1e-14);

            float64 dx = 0.001;
            float64 dexf = 0.3;

            float64 x1 = betFraction;
            float64 x2 = betFraction + dx;
            float64 y1 = actual;
            float64 y2 = GainModelGeom::h(x2, betSize, exf + dexf * dx, f_pot, a);

            float64 xd = (x1 + x2)/2;
            float64 yd = GainModelGeom::h(xd, betSize, exf + dexf * dx/2, f_pot, a);

            float64 actualD = GainModelGeom::hdx(xd, betSize, exf + dexf * dx/2, dexf, f_pot, dx, a, yd);
            float64 expectedD = (y2 - y1)/(x2-x1);
            assert(fabs(expectedD - actualD) < fabs(expectedD) * 1e-6);

        }

        // =====
        {
            FixedStatResult a;
            // No funny business, just two opponents.
            a.fOpponents = 2.0;

            a.fShape.wins = 0.75;
            a.fShape.splits = 0.10;
            a.fShape.loss = 0.15;


            // Drop to 0.8, 27.75% the time (because betFraction == 0.2)
            // Increase to 1.5, 9.0/16.0 of the time (because exf == 0.5)
            // Split three ways has probability 0.1*0.1, and payout 0.8 + (0.2 + 0.5) / 3
            // Split two ways has probability 0.1 * 0.75 * 2 and payout 0.8 + (0.2 + 0.3) / 2

            float64 expected = std::pow(0.8, 0.2775) * std::pow(1.5, 9.0/16.0) * std::pow((0.8 + 0.7 / 3), 0.1*0.1) * std::pow(0.8 + (0.1 + 0.2 + 0.2) / 2, 0.1 * 0.75 * 2);
            float64 actual = GainModelGeom::h(betFraction, betSize, exf, f_pot, a);

            assert(fabs(expected - actual) < fabs(expected) * 1e-14);
        }
    }


    // Sanity check that GainModelNoRisk probabilities add up to 1.0
    // Take some known simple win percentages and measure the outcome.
    void testUnit_015() {
        //GainModelGeom
        //GainModelNoRisk


        const float64 betFraction = 0.2;
        const float64 betSize = std::numeric_limits<float64>::signaling_NaN();
        const float64 exf = 0.5; // there's a lot of money in previous rounds and/or predicted money in this round
        const float64 f_pot = 0.1; // only a small amount is from previous rounds or from players who have folded this round





        {
            FixedStatResult a;
            // Simple case: One opponent
            a.fOpponents = 1.0;

            a.fShape.wins = 0.75;
            a.fShape.splits = 0.10;
            a.fShape.loss = 0.15;


            // Drop to 0.8, 15% the time (because betFraction == 0.2)
            // Increase to 1.5, 75% of the time (because exf == 0.5)
            // Split two ways has probability 0.1 and payout 0.8 + (0.2 + 0.3) / 2
            float64 expected = 0.8 * 0.15 + 1.5 * 0.75 + 0.1 * (0.8 + (0.2 + 0.3) / 2);
            float64 actual = GainModelNoRisk::h(betFraction, betSize, exf, f_pot, a);
            assert(fabs(expected - actual) < fabs(expected) * 1e-14);

            float64 dx = 0.001;
            float64 dexf = 0.3;

            float64 x1 = betFraction;
            float64 x2 = betFraction + dx;
            float64 y1 = actual;
            float64 y2 = GainModelNoRisk::h(x2, betSize, exf + dexf * dx, f_pot, a);

            float64 xd = (x1 + x2)/2;
            float64 yd = GainModelNoRisk::h(xd, betSize, exf + dexf * dx/2, f_pot, a);

            float64 actualD = GainModelNoRisk::hdx(xd, betSize, exf + dexf * dx/2, dexf, a, yd);
            float64 expectedD = (y2 - y1)/(x2-x1);
            assert(fabs(expectedD - actualD) < fabs(expectedD) * 1e-11);

        }

        // =====
        {
            FixedStatResult a;
            // No funny business, just two opponents.
            a.fOpponents = 2.0;

            a.fShape.wins = 0.75;
            a.fShape.splits = 0.10;
            a.fShape.loss = 0.15;


            // Drop to 0.8, 27.75% the time (because betFraction == 0.2)
            // Increase to 1.5, 9.0/16.0 of the time (because exf == 0.5)
            // Split three ways has probability 0.1*0.1, and payout 0.8 + (0.2 + 0.5) / 3
            // Split two ways has probability 0.1 * 0.75 * 2 and payout 0.8 + (0.2 + 0.3) / 2
            float64 expected = 0.8 * 0.2775 + 1.5 * 9.0/16.0 + 0.1*0.1 * (0.8 + 0.7 / 3) + 0.1 * 0.75 * 2 * (0.8 + (0.1 + 0.2 + 0.2) / 2);
            float64 actual = GainModelNoRisk::h(betFraction, betSize, exf, f_pot, a);

            assert(fabs(expected - actual) < fabs(expected) * 1e-14);
        }
    }


    // Set up a simple situation (e.g. post-river) with FoldGainWaitLength and then FoldGainModel
    //  + make sure there is a bunch of pot money from previous rounds, to test whether past pot is correctly accounted for in winnings
    void testUnit_010() {

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
        statprob.core.foldcumu = CoreProbabilities::ReversePerspective(statprob.core.handcumu);

        ///Compute CommunityCallStats
        StatsManager::QueryOffense(statprob.core.callcumu,withCommunity,communityToTest,cardsInCommunity,0);

        ///Compute WinStats
        DistrShape detailPCT(DistrShape::newEmptyDistrShape());
        StatsManager::Query(&detailPCT,withCommunity,communityToTest,cardsInCommunity);
        statprob.core.playerID = 0;
        statprob.core.statmean = detailPCT.mean;

        FoldWaitLengthModel<void, OppositionPerspective> fw;
        // From the opponent's point of view if he knows he's against a flush
        fw.setW( 0.0 ); // Their current hand does not pair the board so loses to an ace-high flush (but note there is a ~30% chance that they hit a pair other than a deuce and a ~4% chance of having one deuce)
        fw.setMeanConv( &(statprob.core.foldcumu) );
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
    void testUnit_010b() {


        const float64 avgBlind = 0.5;
        const float64 pastPot = 10.0;
        const float64 myConstributionToPastPot = 5.0; // so heads-up, battle of the blinds, no antes
        const float64 myBetThisRound = 2.0;
        const float64 iveBeenReraisedTo = 190.0;


        FoldWaitLengthModel<void, void> fw;

        fw.setW( 0.75 ); // You have a decent hand, but it could be better and the re-raise was ridiculously enormous.
        fw.setMeanConv( nullptr );
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
    void testUnit_010c() {


        const float64 avgBlind = 30.0; // Blinds are 40.0/20.0, heads-up. No antes
        const float64 pastPot = 0.0; // We're pre-flop
        const float64 myConstributionToPastPot = 0.0;
        const float64 myBetThisRound = 20.0; // we're in the small blind
        const float64 iveBeenReraisedTo = 5000.0;


        FoldWaitLengthModel<void, void> fw;

        fw.setW(0.75); // You have a decent hand, but it could be better and the raise was large.
        fw.setMeanConv( nullptr );
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



    // Test callcumu sanity checks
    void testUnit_007c() {

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
        statprob.core.foldcumu = CoreProbabilities::ReversePerspective(statprob.core.handcumu);

        ///Compute CommunityCallStats
        StatsManager::QueryOffense(statprob.core.callcumu,withCommunity,communityToTest,cardsInCommunity,0);

        ///Compute WinStats
        DistrShape detailPCT(DistrShape::newEmptyDistrShape());
        StatsManager::Query(&detailPCT,withCommunity,communityToTest,cardsInCommunity);
        statprob.core.playerID = 0;
        statprob.core.statmean = detailPCT.mean;

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
    void testUnit_007b() {

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
        statprob.core.foldcumu = CoreProbabilities::ReversePerspective(statprob.core.handcumu);

        ///Compute CommunityCallStats
        StatsManager::QueryOffense(statprob.core.callcumu,withCommunity,communityToTest,cardsInCommunity,0);

        ///Compute WinStats
        DistrShape detailPCT(DistrShape::newEmptyDistrShape());
        StatsManager::Query(&detailPCT,withCommunity,communityToTest,cardsInCommunity);
        statprob.core.playerID = 0;
        statprob.core.statmean = detailPCT.mean;

        // TEST:
        assert(statprob.core.statRelation().pct > 0.95); // This hand is very good. StatRelation should do very well.
        assert(statprob.core.statRanking().pct > 0.97); // This is one of the best hands you can have in this situation. StatRanking should do very well.

        // Against most opponents, I have a very strong chance to win.
        assert(statprob.core.getFractionOfOpponentsWhereMyWinrateIsAtLeast(0.85) > 0.9);

    }



    // Test oddsAgainstBestXHands derivative
    void testUnit_007() {

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
        statprob.core.foldcumu = CoreProbabilities::ReversePerspective(statprob.core.handcumu);

        // TEST:
        const float64 xa = 0.3;
        const float64 xb = 0.3001;
        std::pair<StatResult, float64> ya = statprob.core.handcumu.bestXHands(xa);
        std::pair<StatResult, float64> yb = statprob.core.handcumu.bestXHands(xb);

        const float64 expected = (yb.first.pct - ya.first.pct) / (xb - xa);
        const float64 actual = (ya.second + yb.second) / 2.0;

        assert(fabs(actual - expected) < fabs(expected) * 1.0e-7);
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
    void testUnit_003() {

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

        DistrShape detailPCT(DistrShape::newEmptyDistrShape());
        StatResultProbabilities statprob;

        ///Compute CallStats
        StatsManager::QueryDefense(statprob.core.handcumu,withCommunity,communityToTest,cardsInCommunity);
        statprob.core.foldcumu = CoreProbabilities::ReversePerspective(statprob.core.handcumu);

        ///Compute CommunityCallStats
        StatsManager::QueryOffense(statprob.core.callcumu,withCommunity,communityToTest,cardsInCommunity,0);

        ///Compute WinStats
        StatsManager::Query(&detailPCT,withCommunity,communityToTest,cardsInCommunity);

        statprob.core.statmean = detailPCT.mean;


        ///====================================
        ///   Compute Relevant Probabilities
        ///====================================

        statprob.Process_FoldCallMean();

        StatResult statWorse = statprob.statworse(2);
        assert(0.95 <= statWorse.loss);
    }

    void testUnit_002b() {
        ChipPositionState oppCPS(2474,7.875,0,0, 0.0);
        // tableinfo->RiskLoss(0, 2474, 4, 6.75, 0, 0) = 0.0
        HypotheticalBet hypothetical0 = {
          oppCPS, 6.75 + oppCPS.alreadyBet,
          std::numeric_limits<float64>::signaling_NaN(), // raiseBy is not needed for this test, unless you also want to test the derivative
          4.5, false, true
        };
        float64 w_r_rank0 = ExactCallD::facedOdds_raise_Geom_forTest<void>(0.0, 1.0 /* denom */, 0.0 /* RiskLoss */ , 0.31640625 /* avgBlind */
                                                                     ,hypothetical0, 4, nullptr);
        // tableinfo->RiskLoss(0, 2474, 4, 11.25, 0, 0) == 0.0
        HypotheticalBet hypothetical1 = {
          oppCPS,
          11.25 + oppCPS.alreadyBet,
          std::numeric_limits<float64>::signaling_NaN(), // raiseBy is not needed for this test, unless you also want to test the derivative
          4.5, false, true
        };
        float64 w_r_rank1 = ExactCallD::facedOdds_raise_Geom_forTest<void>(w_r_rank0, 1.0, 0.0 ,  0.31640625
                                                                     ,hypothetical1, 4, nullptr);

        // The bug is: These two values, if otherwise equal, can end up being within half-quantum.
        assert(w_r_rank0 <= w_r_rank1);
    }


}

#define VectorOf(_ilistarray) std::vector<float64>((_ilistarray), (_ilistarray) + sizeof(_ilistarray) / sizeof((_ilistarray)[0]) )

#include "../src/arena.h"
#include "../src/arenaEvents.h"
#include "../src/stratPosition.h"
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
            assert(i < bets.size()); // the point of a FixedReplayPlayerStrategy is to replay a fixed sequence of actions
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


    void testRegression_028() {


// -  stat_high algb SLIDERX -
        // Qh Ac

        // Add player   P2   $84.0
        // Add player   Multi   $100.0
        // Add player   P4   $210.0
        // Add player   D2   $60.0
        // Add player   Gear   $158.0
        // Add player   P0   $84.0
        // Add player   P3   $204.0
        BlindValues b;
        b.SetSmallBigBlind(2);

        HoldemArena myTable(b.GetSmallBlind(), true, true);
        myTable.setSmallestChip(2);

        const playernumber_t dealer = 6;

        const std::vector<float64> foldOnly(1, 0.0);

        PureGainStrategy bot("28.txt", 4);
        PlayerStrategy * const botToTest = &bot;




        FixedReplayPlayerStrategy d0(foldOnly);
        myTable.ManuallyAddPlayer("P2", 84.0, &d0);


        FixedReplayPlayerStrategy d1(foldOnly);
        myTable.ManuallyAddPlayer("Multi", 100.0, &d1);


        myTable.ManuallyAddPlayer("P4", 210.0, botToTest);


        FixedReplayPlayerStrategy d3(foldOnly);
        myTable.ManuallyAddPlayer("D2", 60.0, &d3);


        static const float64 a4[] = {
            158
        };
        FixedReplayPlayerStrategy d4(VectorOf(a4));
        myTable.ManuallyAddPlayer("Gear", 158.0, &d4);


        FixedReplayPlayerStrategy d5(foldOnly);
        myTable.ManuallyAddPlayer("P0", 84.0, &d5);


        static const float64 a6[] = {
            std::numeric_limits<float64>::signaling_NaN(),
            std::numeric_limits<float64>::signaling_NaN(),
            std::numeric_limits<float64>::signaling_NaN(),
            // [!NOTE]
            // We need a fourth "check" here, otherwise you'll hit
            //   * Assertion failed: (i < bets.size()), function MakeBet, file main.cpp, line 938.
            //   * via HoldemArena::PlayRound (loops until `curIndex` gets to 6)
            //   * during `HoldemArena::PlayRound_River` below
            std::numeric_limits<float64>::signaling_NaN()
            // This fourth "check" action never happened in reality because the original bug wouled have crashed during P4's action on the river (and therefore P3 would never have had a chance to act)
        };
        FixedReplayPlayerStrategy d6(VectorOf(a6));
        myTable.ManuallyAddPlayer("P3", 204.0, &d6);
        // P3 will call pre-flop and then check 3 times



        DeckLocation card;

        {
            CommunityPlus handToTest; // Ah Qc

            card.SetByIndex(41);
            handToTest.AddToHand(card);

            card.SetByIndex(50);
            handToTest.AddToHand(card);

            botToTest->StoreDealtHand(handToTest);
        }


        myTable.BeginInitialState(28);
        myTable.BeginNewHands(std::cout, b, false, dealer);


        assert(  myTable.PlayRound_BeginHand(std::cout)  !=  -1);
               // Flop: 3d 5d 9s

               CommunityPlus myFlop;

               card.SetByIndex(7);
               myFlop.AddToHand(card);

               card.SetByIndex(15);
               myFlop.AddToHand(card);

               card.SetByIndex(28);
               myFlop.AddToHand(card);

               assert(myTable.PlayRound_Flop(myFlop, std::cout) != -1);
               // Turn: Td

               DeckLocation myTurn;
               myTurn.SetByIndex(35);

               assert(  myTable.PlayRound_Turn(myFlop, myTurn, std::cout)  !=  -1);
               // River: 6s

               DeckLocation myRiver;
               myRiver.SetByIndex(16);

        myTable.PlayRound_River(myFlop, myTurn, myRiver, std::cout);
        // Original failure:
        // HoldemArena::PlayRound_River → HoldemArena::PlayRound (loops until `curIndex` gets to 2)
        //   → PureGainStrategy::MakeBet → PositionalStrategy::solveGainModel → HoldemFunctionModel::FindFoldBet
        //   which calls `ScalarFunctionModel::FindZero`
        //   but it had a bug causing the subsequent...
        //    → StateModel::f → StateModel::query → StateModel::g_raised → AutoScalingFunction::f_raised → AutoScalingFunction::query → GainModelNoRisk::g → GainModelNoRisk::h
        //    to crash at the "You can't lose more than all your money: GainModelNoRisk" assertion


    }

    // We hit the "You can't lose more than all your money: GainModelNoRisk" assertion
    void testRegression_027() {
        /*
         Next Dealer is D2


         Preflop
         (Pot: $0)
         (9 players)
         [P0 $78]
         [P3 $344]
         [P2 $28]
         [Multi $108]
         [D0 $108]
         [D2 $106]
         [Gear $128]

         */
        BlindValues b;
        b.SetSmallBigBlind(2);

        HoldemArena myTable(b.GetSmallBlind(), true, true);
        myTable.setSmallestChip(2);

        const std::vector<float64> foldOnly(1, 0.0);

        static const float64 da[] = {
            108.0
        };
        const std::vector<float64> dA(VectorOf(da));


        FixedReplayPlayerStrategy p0(foldOnly);
        FixedReplayPlayerStrategy p3(foldOnly);
        FixedReplayPlayerStrategy mS(dA);
        FixedReplayPlayerStrategy p2(foldOnly);
        FixedReplayPlayerStrategy d2(foldOnly);
        FixedReplayPlayerStrategy d0(foldOnly);


        PureGainStrategy bot("27.txt", 0);
        PlayerStrategy * const botToTest = &bot;
        myTable.ManuallyAddPlayer("Gear", 128, botToTest);
        myTable.ManuallyAddPlayer("P0", 78, &p0);
        myTable.ManuallyAddPlayer("P3", 344, &p3);
        myTable.ManuallyAddPlayer("P2", 28, &p2);
        myTable.ManuallyAddPlayer("Multi", 108, &mS);
        myTable.ManuallyAddPlayer("D0", 108, &d0);
        myTable.ManuallyAddPlayer("D2", 106, &d2);

        const playernumber_t dealer = 0;





        DeckLocation card;

        {
            CommunityPlus handToTest; // 7c Td

            card.SetByIndex(22);
            handToTest.AddToHand(card);

            card.SetByIndex(35);
            handToTest.AddToHand(card);

            botToTest->StoreDealtHand(handToTest);
        }



        myTable.BeginInitialState(27);
        myTable.BeginNewHands(std::cout, b, false, dealer);



        // Gear must not crash
        assert(myTable.PlayRound_BeginHand(std::cout) == -1);


    }

    // We hit the "You can't lose more than all your money: GainModelNoRisk" assertion
    void testRegression_026() {
        /*
         Next Dealer is D2


         Preflop
         (Pot: $0)
         (9 players)
         [Gear $94.2857]
         [P0 $100]
         [P3 $100]
         [P2 $100]
         [Multi $100]
         [i4 $52.8571]
         [P4 $170]
         [D0 $91.4286]
         [D2 $91.4286]
         */
        BlindValues b;
        b.SetSmallBigBlind(2.85714);

        HoldemArena myTable(b.GetSmallBlind(), true, true);
        myTable.setSmallestChip(2.85714);

        const std::vector<float64> foldOnly(1, 0.0);

        static const float64 mA[] = {
            20,
            32.8571,
            std::numeric_limits<float64>::signaling_NaN(),
            std::numeric_limits<float64>::signaling_NaN(),
            std::numeric_limits<float64>::signaling_NaN()
        };
        const std::vector<float64> callOnly(VectorOf(mA));


        FixedReplayPlayerStrategy p0(callOnly);
        FixedReplayPlayerStrategy p3(foldOnly);
        FixedReplayPlayerStrategy p2(foldOnly);
        FixedReplayPlayerStrategy mS(foldOnly);
        FixedReplayPlayerStrategy i4(callOnly);
        FixedReplayPlayerStrategy p4(foldOnly);
        FixedReplayPlayerStrategy d0(foldOnly);
        FixedReplayPlayerStrategy d2(foldOnly);


        PureGainStrategy bot("26.txt", 4);
        PlayerStrategy * const botToTest = &bot;
        myTable.ManuallyAddPlayer("D2", 91.4286, &d2);
        myTable.ManuallyAddPlayer("Gear", 94.2857, botToTest);
        myTable.ManuallyAddPlayer("P0", 100.0, &p0);
        myTable.ManuallyAddPlayer("P3", 100.0, &p3);
        myTable.ManuallyAddPlayer("P2", 100.0, &p2);
        myTable.ManuallyAddPlayer("Multi", 100.0, &mS);
        myTable.ManuallyAddPlayer("i4", 52.8571, &i4);
        myTable.ManuallyAddPlayer("P4", 170.0, &p4);
        myTable.ManuallyAddPlayer("D0", 91.4286, &d0);

        const playernumber_t dealer = 0;





        DeckLocation card;

        {
            CommunityPlus handToTest; // 5d As

            card.SetByIndex(15);
            handToTest.AddToHand(card);

            card.SetByIndex(48);
            handToTest.AddToHand(card);

            botToTest->StoreDealtHand(handToTest);
        }



        myTable.BeginInitialState(26);
        myTable.BeginNewHands(std::cout, b, false, dealer);



        assert(myTable.PlayRound_BeginHand(std::cout) != -1);

        /// ... (Pot: 60)
        // Flop: 6h 9s Tc
        CommunityPlus myFlop;

        card.SetByIndex(17);
        myFlop.AddToHand(card);

        card.SetByIndex(28);
        myFlop.AddToHand(card);

        card.SetByIndex(34);
        myFlop.AddToHand(card);



        // Gear checks
        // P0 checks
        // i4 pushes all-in
        // Gear calls
        // P0 calls
        assert(myTable.PlayRound_Flop(myFlop, std::cout) != -1);


        // Turn Ts



        DeckLocation myTurn;
        myTurn.SetByIndex(32);


        // Check, check
        assert(myTable.PlayRound_Turn(myFlop, myTurn, std::cout) != -1);

        // River 8d


        DeckLocation myRiver;
        myRiver.SetByIndex(27);

// Gear must not crash.

        assert(myTable.PlayRound_River(myFlop, myTurn, myRiver, std::cout) != -1);

    }


    // We hit the "You can't lose more than all your money: GainModelNoRisk" assertion
    void testRegression_025() {
        /*
        Next Dealer is NormR
        ================================================================
        ============================New Hand #21========================
        BEGIN


        Preflop
        (Pot: $0)
        (7 players)
        [TrapR $8.57143]
        [i4 $88.5714]
        [GearBotR $65.7143]
        [MultiBotR $288.571]
        [DangerR $237.143]
        [ComR $137.143]
        [NormR $74.2857]
*/
        BlindValues b;
        b.SetSmallBigBlind(2.85714);

        HoldemArena myTable(b.GetSmallBlind(), true, true);
        myTable.setSmallestChip(2.85714);

        const std::vector<float64> foldOnly(1, 0.0);

        static const float64 mA[] = {
            std::numeric_limits<float64>::signaling_NaN(),
            std::numeric_limits<float64>::signaling_NaN()
        };
        const std::vector<float64> callOnly(VectorOf(mA));

        static const float64 da[] = {
            157.143,
            std::numeric_limits<float64>::signaling_NaN()
        };
        const std::vector<float64> dA(VectorOf(da));


        FixedReplayPlayerStrategy gS(foldOnly);
        FixedReplayPlayerStrategy tS(foldOnly);
        FixedReplayPlayerStrategy dS(dA);
        FixedReplayPlayerStrategy pS(foldOnly);
        FixedReplayPlayerStrategy nS(foldOnly);
        FixedReplayPlayerStrategy mS(callOnly);
        FixedReplayPlayerStrategy cS(foldOnly);


        PureGainStrategy bot("25.txt", 4);
        PlayerStrategy * const botToTest = &bot;
        myTable.ManuallyAddPlayer("NormR", 74.2857, botToTest);
        myTable.ManuallyAddPlayer("TrapR", 8.57143, &tS);
        myTable.ManuallyAddPlayer("i4", 88.5714, &pS);
        myTable.ManuallyAddPlayer("GearBotR", 65.7143, &gS);
        myTable.ManuallyAddPlayer("MultiBotR", 288.571, &mS);
        myTable.ManuallyAddPlayer("DangerR", 237.143, &dS);
        myTable.ManuallyAddPlayer("ComR", 137.143, &cS);


        const playernumber_t dealer = 0;




        /*
        TrapR posts SB of $2.85714 ($2.85714)
        i4 posts BB of $5.71429 ($8.57143)
        GearBotR folds
        MultiBotR calls $5.71429 ($14.2857)
        DangerR raises to $157.143 ($171.429)
        ComR folds
        */



        DeckLocation card;

        {
            CommunityPlus handToTest; // 7s 8c

            card.SetByIndex(20);
            handToTest.AddToHand(card);

            card.SetByIndex(26);
            handToTest.AddToHand(card);

            botToTest->StoreDealtHand(handToTest);
        }



        myTable.BeginInitialState(25);
        myTable.BeginNewHands(std::cout, b, false, dealer);


        // Norm R must not crash

        myTable.PlayRound_BeginHand(std::cout);

    }


    // Hand played live
    void testRegression_022() {



        BlindValues b;
        b.SetSmallBigBlind(1.0);

        HoldemArena myTable(b.GetSmallBlind(), true, true);
        myTable.setSmallestChip(1.0);


        static const float64 callFold[] = {std::numeric_limits<float64>::signaling_NaN(), 0.0};
        static const float64 callCallFold[] = {
            std::numeric_limits<float64>::signaling_NaN(),
            std::numeric_limits<float64>::signaling_NaN(),
            0.0,
            0.0};

        static const float64 aa[] = {
            std::numeric_limits<float64>::signaling_NaN(),
            std::numeric_limits<float64>::signaling_NaN(),
            100.0,
            std::numeric_limits<float64>::signaling_NaN()};

        FixedReplayPlayerStrategy nS(VectorOf(callCallFold));
        FixedReplayPlayerStrategy sS(VectorOf(callFold));
        FixedReplayPlayerStrategy lS(VectorOf(callFold));
        FixedReplayPlayerStrategy jS(VectorOf(callFold));
        FixedReplayPlayerStrategy aS(VectorOf(aa));
        FixedReplayPlayerStrategy bS(VectorOf(callFold));
        FixedReplayPlayerStrategy mS(VectorOf(callFold));


        PureGainStrategy bot("22.txt", 0);
        PlayerStrategy * const botToTest = &bot;
        myTable.ManuallyAddPlayer("h22", 500, botToTest);
        myTable.ManuallyAddPlayer("Nav", 500.0, &nS);
        myTable.ManuallyAddPlayer("Sam", 500.0, &sS);
        myTable.ManuallyAddPlayer("Laily", 500.0, &lS);
        myTable.ManuallyAddPlayer("Joyce", 500, &jS);
        myTable.ManuallyAddPlayer("Andrew", 500, &aS);
        myTable.ManuallyAddPlayer("Badr", 500, &bS);
        myTable.ManuallyAddPlayer("Mona", 500, &mS); // dealer

        const playernumber_t dealer = 7;





        DeckLocation card;

        {
            CommunityPlus handToTest; // As Qs

            card.SetByIndex(48);
            handToTest.AddToHand(card);

            card.SetByIndex(40);
            handToTest.AddToHand(card);

            botToTest->StoreDealtHand(handToTest);
        }



        myTable.BeginInitialState(22);
        myTable.BeginNewHands(std::cout, b, false, dealer);

        /*

         All-limp

         */
        assert(myTable.PlayRound_BeginHand(std::cout) != -1);
        assert(myTable.IsInHand(0));

        // Pot is now $16.

        CommunityPlus myFlop;

        card.SetByIndex(1);
        myFlop.AddToHand(card);

        card.SetByIndex(43);
        myFlop.AddToHand(card);

        card.SetByIndex(50);
        myFlop.AddToHand(card);
        /*

         Flop:	Ac Qd 2h   (Pot: $16)

         */



        assert(myTable.PlayRound_Flop(myFlop, std::cout) != -1);


        /*


         (8 players)
         [Joseph]
         [Nav]
         [Sam]
         [Laily]
         [Joyce]
         [Andrew]
         [Badr]
         [Mona]

         Joseph bets $x (limp / small bet)
         Nav calls $x
         Sam folds
         Laily folds
         Joyce folds
         Andrew calls $x
         Badr folds
         Mona folds
         */



        DeckLocation myTurn;
        myTurn.SetByIndex(3);
        /*

         Turn:	Ac Qd 2h 2d  (Pot: $34)
         */

        // You have top two pair. One of your

        assert(myTable.PlayRound_Turn(myFlop, myTurn, std::cout) != -1); // ASSERT: PureGainStrategy should not fold with top two pair, right?
        /*

         (3 players)
         [Joseph]
         [Nav]
         [Andrew]

         Joseph bets (e.g. $40) or doesn't bet (check)
         Nav check/folds
         Andrew raise to $100
         Joseph call if already $40 comitted, and fold otherwise

         */

        DeckLocation myRiver;
        myRiver.SetByIndex(4);

        /*
         River:	Ac Qd 2h 2d 3s  (Pot: $234)
         */

        // const playernumber_t highbettor = myTable.PlayRound_River(myFlop, myTurn, myRiver, std::cout);
        //assert(highbettor == 4);
        // No all-fold; assert that the pot was increased at least.
        //assert(myTable.GetPotSize() > 55);


        /*

         Joseph bet $140
         Andrew call


         */
    }


    // Hand played live
    void testRegression_021b() {



        BlindValues b;
        b.SetSmallBigBlind(1.0);

        HoldemArena myTable(b.GetSmallBlind(), true, true);
        myTable.setSmallestChip(1.0);

        const std::vector<float64> foldOnly(1, 0.0);

        static const float64 aa[] = {std::numeric_limits<float64>::signaling_NaN(), 0.0, 0.0};
        const std::vector<float64> callCheckFold(aa, aa + sizeof(aa) / sizeof(aa[0]) );

        static const float64 na[] = {std::numeric_limits<float64>::signaling_NaN(), 20.0, 0.0, 0.0};
        static const float64 sa[] = {std::numeric_limits<float64>::signaling_NaN(), 20.0, 0.0, std::numeric_limits<float64>::signaling_NaN()};
        //static const float64 na[] = {std::numeric_limits<float64>::signaling_NaN(), 50.0, 0.0, 0.0};
        //static const float64 sa[] = {std::numeric_limits<float64>::signaling_NaN(), 50.0, 0.0, std::numeric_limits<float64>::signaling_NaN()};

        FixedReplayPlayerStrategy nS(VectorOf(na));
        FixedReplayPlayerStrategy sS(VectorOf(sa));
        FixedReplayPlayerStrategy lS(callCheckFold);
        FixedReplayPlayerStrategy jS(callCheckFold);
        FixedReplayPlayerStrategy aS(callCheckFold);
        FixedReplayPlayerStrategy bS(foldOnly);
        FixedReplayPlayerStrategy mS(callCheckFold);


        PureGainStrategy bot("21b.txt", 3);
        PlayerStrategy * const botToTest = &bot;
        myTable.ManuallyAddPlayer("h21", 2600.0, botToTest);
        myTable.ManuallyAddPlayer("Nav", 300.0, &nS);
        myTable.ManuallyAddPlayer("Sam", 800.0, &sS); // dealer
        myTable.ManuallyAddPlayer("Laily", 140.0, &lS);
        myTable.ManuallyAddPlayer("Joyce", 630, &jS);
        myTable.ManuallyAddPlayer("Andrew", 630, &aS);
        myTable.ManuallyAddPlayer("Badr", 630, &bS);
        myTable.ManuallyAddPlayer("Mona", 630, &mS);

        const playernumber_t dealer = 2;





        DeckLocation card;

        {
            CommunityPlus handToTest;

            card.SetByIndex(49); // Ah
            //card.SetByIndex(21); 7h
            handToTest.AddToHand(card);

            card.SetByIndex(1); // 2h
            handToTest.AddToHand(card);

            botToTest->StoreDealtHand(handToTest);
        }



        myTable.BeginInitialState(21);
        myTable.BeginNewHands(std::cout, b, false, dealer);

        /*

         All-limp except one.

         */



        myTable.PrepBettingRound(true,3);  //flop, turn, river remaining

        {
            HoldemArenaBetting r( &myTable, CommunityPlus::EMPTY_COMPLUS, 0 , &(std::cout));

            struct MinRaiseError msg;


            r.MakeBet(aS.MakeBet(), &msg);

            r.MakeBet(bS.MakeBet(), &msg);

            r.MakeBet(mS.MakeBet(), &msg);

            r.MakeBet(myTable.GetBetToCall(), &msg);

            r.MakeBet(nS.MakeBet(), &msg);

            r.MakeBet(sS.MakeBet(), &msg);

            r.MakeBet(lS.MakeBet(), &msg);
            r.MakeBet(jS.MakeBet(), &msg);

        }



        assert(myTable.IsInHand(0)); // play value hands you have a large stack.
        // If you win you will win a lot since winning hands only have a high win percentage
        // and losing hands have a low win percentage
        // This is based on:
        // NOT the distribution of community hands
        // NOT the distribution of opposing hands
        // YES the distribution of flops

        // Pot is now $14.

        CommunityPlus myFlop;

        card.SetByIndex(30);
        myFlop.AddToHand(card);

        card.SetByIndex(41);
        myFlop.AddToHand(card);

        card.SetByIndex(45);
        myFlop.AddToHand(card);
        /*

         Flop:	9c Qh Kh   (Pot: $14)

         */



        assert(myTable.PlayRound_Flop(myFlop, std::cout) != -1);


        /*


         (7 players)
         [Laily]
         [Joyce]
         [Andrew]
         [Mona]
         [Joseph]
         [Nav]
         [Sam]

         Laily checks
         Joyce checks
         Andrew checks
         Mona checks
         Joseph checks
         Nav bets $50
         Sam calls $50
         Laily folds
         Andrew folds
         Mona folds
         Joseph calls $50
         */



        DeckLocation myTurn;
        myTurn.SetByIndex(32);
        /*

         Turn:	9c Qh Kh 10s   (Pot: $164)
         */


        assert(myTable.PlayRound_Turn(myFlop, myTurn, std::cout) != -1);
        /*

         (3 players)
         [Joseph]
         [Nav]
         [Sam]

         Joseph checks
         Nav checks
         Sam checks

         */

        DeckLocation myRiver;
        myRiver.SetByIndex(10);

        /*
         River:	9c Qh Kh 10s 4c  (Pot: $164)
         */

        const playernumber_t highbettor = myTable.PlayRound_River(myFlop, myTurn, myRiver, std::cout);
        (void)highbettor; // -Wunused-variable
        //assert(highbettor == 4);

        // No all-fold; assert that the pot was increased at least.
        //assert(myTable.GetPotSize() > 55);


        /*

         (3 players)
         [ActionBotV $1495]
         [NormalBotV $1463.52]
         [GearBotV $1453]

         ActionBotV checks
         NormalBotV checks
         GearBotV checks



         */
    }



    // Hand played live
    void testRegression_021() {



        BlindValues b;
        b.SetSmallBigBlind(1.0);

        HoldemArena myTable(b.GetSmallBlind(), true, true);
        myTable.setSmallestChip(1.0);

        const std::vector<float64> foldOnly(1, 0.0);

        static const float64 aa[] = {std::numeric_limits<float64>::signaling_NaN(), 0.0, 0.0};
        const std::vector<float64> callCheckFold(aa, aa + sizeof(aa) / sizeof(aa[0]) );

        static const float64 na[] = {std::numeric_limits<float64>::signaling_NaN(), 50.0, 0.0, 0.0};
        static const float64 sa[] = {std::numeric_limits<float64>::signaling_NaN(), 50.0, 0.0, std::numeric_limits<float64>::signaling_NaN()};

        FixedReplayPlayerStrategy nS(VectorOf(na));
        FixedReplayPlayerStrategy sS(VectorOf(sa));
        FixedReplayPlayerStrategy lS(callCheckFold);
        FixedReplayPlayerStrategy jS(callCheckFold);
        FixedReplayPlayerStrategy aS(callCheckFold);
        FixedReplayPlayerStrategy bS(foldOnly);
        FixedReplayPlayerStrategy mS(callCheckFold);


        PureGainStrategy bot("21.txt", 3);
        PlayerStrategy * const botToTest = &bot;
        myTable.ManuallyAddPlayer("h21", 2600.0, botToTest);
        myTable.ManuallyAddPlayer("Nav", 300.0, &nS);
        myTable.ManuallyAddPlayer("Sam", 800.0, &sS); // dealer
        myTable.ManuallyAddPlayer("Laily", 140.0, &lS);
        myTable.ManuallyAddPlayer("Joyce", 630, &jS);
        myTable.ManuallyAddPlayer("Andrew", 630, &aS);
        myTable.ManuallyAddPlayer("Badr", 630, &bS);
        myTable.ManuallyAddPlayer("Mona", 630, &mS);

        const playernumber_t dealer = 2;





        DeckLocation card;

        {
            CommunityPlus handToTest; // 7h 2h

            card.SetByIndex(21);
            handToTest.AddToHand(card);

            card.SetByIndex(1);
            handToTest.AddToHand(card);

            botToTest->StoreDealtHand(handToTest);
        }



        myTable.BeginInitialState(21);
        myTable.BeginNewHands(std::cout, b, false, dealer);

        /*

         All-limp except one.

         */




        assert(myTable.PlayRound_BeginHand(std::cout) != -1);
        assert(myTable.IsInHand(0)); // play value hands you have a large stack.
        // If you win you will win a lot since winning hands only have a high win percentage
        // and losing hands have a low win percentage
        // This is based on:
        // NOT the distribution of community hands
        // NOT the distribution of opposing hands
        // YES the distribution of flops

        // Pot is now $14.

        CommunityPlus myFlop;

        card.SetByIndex(30);
        myFlop.AddToHand(card);

        card.SetByIndex(41);
        myFlop.AddToHand(card);

        card.SetByIndex(45);
        myFlop.AddToHand(card);
        /*

         Flop:	9c Qh Kh   (Pot: $14)

         */



        assert(myTable.PlayRound_Flop(myFlop, std::cout) != -1);


        /*


         (7 players)
         [Laily]
         [Joyce]
         [Andrew]
         [Mona]
         [Joseph]
         [Nav]
         [Sam]

         Laily checks
         Joyce checks
         Andrew checks
         Mona checks
         Joseph checks
         Nav bets $50
         Sam calls $50
         Laily folds
         Andrew folds
         Mona folds
         Joseph calls $50
         */



        DeckLocation myTurn;
        myTurn.SetByIndex(32);
        /*

         Turn:	9c Qh Kh 10s   (Pot: $164)
         */


        assert(myTable.PlayRound_Turn(myFlop, myTurn, std::cout) != -1);
        /*

         (3 players)
         [Joseph]
         [Nav]
         [Sam]

         Joseph checks
         Nav checks
         Sam checks

         */

        DeckLocation myRiver;
        myRiver.SetByIndex(49);

        /*
         River:	9c Qh Kh 10s 4c  (Pot: $164)
         */

        const playernumber_t highbettor = myTable.PlayRound_River(myFlop, myTurn, myRiver, std::cout);
        (void)highbettor; // -Wunused-variable
        //assert(highbettor == 4);

        // No all-fold; assert that the pot was increased at least.
        //assert(myTable.GetPotSize() > 55);


        /*

         (3 players)
         [ActionBotV $1495]
         [NormalBotV $1463.52]
         [GearBotV $1453]

         ActionBotV checks
         NormalBotV checks
         GearBotV checks



         */
    }



    // Should StateModel have optional sliderx functionality?
    // Because if you call you get the benefit of not pessimistic gainmodel since calling sets sliderx to call.
    // In PureGainStrategy, at least, do raises against count as pessimistic? Probably yes since it's the same as "number at first action".
    // This should explain why our bots like to check/call so much right now.

    // In this test, AgainstRaise(call) is too high, presumably because it draws raises at the (call) winPCT.
    void testRegression_018() {



        BlindValues b;
        b.SetSmallBigBlind(5.0);

        HoldemArena myTable(b.GetSmallBlind(), true, true);
        myTable.setSmallestChip(5.0);

        const std::vector<float64> foldOnly(1, 0.0);
        static const float64 aa[] = {
            std::numeric_limits<float64>::signaling_NaN(),
            std::numeric_limits<float64>::signaling_NaN(),
            std::numeric_limits<float64>::signaling_NaN(),
            std::numeric_limits<float64>::signaling_NaN(),
            std::numeric_limits<float64>::signaling_NaN(),
            std::numeric_limits<float64>::signaling_NaN()};
        const std::vector<float64> callOnly(aa, aa + sizeof(aa) / sizeof(aa[0]) );
        FixedReplayPlayerStrategy gS(callOnly);
        FixedReplayPlayerStrategy tS(foldOnly);
        FixedReplayPlayerStrategy dS(foldOnly);
        FixedReplayPlayerStrategy pS(foldOnly);
        FixedReplayPlayerStrategy nS(callOnly);
        FixedReplayPlayerStrategy mS(foldOnly);
        FixedReplayPlayerStrategy cS(foldOnly);
        FixedReplayPlayerStrategy sS(foldOnly);


        PureGainStrategy bot("18.txt", 2);
        PlayerStrategy * const botToTest = &bot;
        myTable.ManuallyAddPlayer("MultiBotV", 1590.0, &mS);
        myTable.ManuallyAddPlayer("ConservativeBotV", 1582.97, &cS);
        myTable.ManuallyAddPlayer("SpaceBotV", 1485.0, &sS);
        myTable.ManuallyAddPlayer("GearBotV", 1545.0, &gS); // dealer
        myTable.ManuallyAddPlayer("ActionBot18", 1505.0, botToTest);
        myTable.ManuallyAddPlayer("NormalBotV", 1473.52, &nS);
        myTable.ManuallyAddPlayer("TrapBotV", 1473.52, &tS);
        myTable.ManuallyAddPlayer("Nav", 1450.0, &pS);
        myTable.ManuallyAddPlayer("DangerBotV", 1495.0, &dS);

        const playernumber_t dealer = 3;



        /*

         Preflop
         (Pot: $0)
         (9 players)
         [ActionBotV $1505]
         [NormalBotV $1473.52]
         [TrapBotV $1473.52]
         [Nav $1450]
         [DangerBotV $1495]
         [MultiBotV $1590]
         [ConservativeBotV $1582.97]
         [SpaceBotV $1485]
         [GearBotV $1445]

         */



        DeckLocation card;

        {
            CommunityPlus handToTest; // Ac Jd

            card.SetByIndex(39);
            handToTest.AddToHand(card);

            card.SetByIndex(50);
            handToTest.AddToHand(card);

            botToTest->StoreDealtHand(handToTest);
        }



        myTable.BeginInitialState(18);
        myTable.BeginNewHands(std::cout, b, false, dealer);

        /*


         ActionBotV posts SB of $5 ($5)
         NormalBotV posts BB of $10 ($15)

         */
        assert(myTable.PlayRound_BeginHand(std::cout) != -1);
        assert(myTable.IsInHand(4));
        /*
         TrapBotV folds
         Nav folds
         DangerBotV folds
         MultiBotV folds
         ConservativeBotV folds
         SpaceBotV folds
         GearBotV calls $10 ($25)
         ActionBotV calls $5 ($30)
         NormalBotV checks
         */


        CommunityPlus myFlop;

        card.SetByIndex(9);
        myFlop.AddToHand(card);

        card.SetByIndex(23);
        myFlop.AddToHand(card);

        card.SetByIndex(34);
        myFlop.AddToHand(card);
        /*


         Flop:	4h 7d Tc   (Pot: $30)

         */



        assert(myTable.PlayRound_Flop(myFlop, std::cout) != -1);

        /*

         (3 players)
         [ActionBotV $1495]
         [NormalBotV $1463.52]
         [GearBotV $1453]

         ActionBotV checks
         NormalBotV checks
         GearBotV checks

         */

        DeckLocation myTurn;
        myTurn.SetByIndex(51);
        /*

         Turn:	4h 7d Tc Ad   (Pot: $30)
         */


        assert(myTable.PlayRound_Turn(myFlop, myTurn, std::cout) != -1);

        /*


         (3 players)
         [ActionBotV $1495]
         [NormalBotV $1463.52]
         [GearBotV $1453]

         ActionBotV checks
         NormalBotV checks
         GearBotV checks
         */

        DeckLocation myRiver;
        myRiver.SetByIndex(49);
        /*


         River:	4h 7d Tc Ad Ah  (Pot: $30)
         */


        const playernumber_t highbettor = myTable.PlayRound_River(myFlop, myTurn, myRiver, std::cout);
        assert(highbettor == 4);
        // No all-fold; assert that the pot was increased at least.
        assert(myTable.GetPotSize() > 55);


        /*

         (3 players)
         [ActionBotV $1495]
         [NormalBotV $1463.52]
         [GearBotV $1453]

         ActionBotV checks
         NormalBotV checks
         GearBotV checks



         */
    }



    // _____ needs to be more adversarial.
    // If you have a good hand and raise, who is likely to call? Why doesn't pessimistic work there?
    // It's fine if you say only re-raise and fold are possible, but fold should be higher with such an overbet, shouldn't it?
    void testRegression_017() {



        BlindValues b;
        b.SetSmallBigBlind(5.0);

        HoldemArena myTable(b.GetSmallBlind(), true, true);
        myTable.setSmallestChip(5.0);

        const std::vector<float64> foldOnly(1, 0.0);
        static const float64 aa[] = {10.0,
            std::numeric_limits<float64>::signaling_NaN(),
            std::numeric_limits<float64>::signaling_NaN(),
            std::numeric_limits<float64>::signaling_NaN(),
            std::numeric_limits<float64>::signaling_NaN(),
            100.0};
        const std::vector<float64> aA(aa, aa + sizeof(aa) / sizeof(aa[0]) );
        static const float64 pa[] = {std::numeric_limits<float64>::signaling_NaN(),
            std::numeric_limits<float64>::signaling_NaN(),
            std::numeric_limits<float64>::signaling_NaN(),
            std::numeric_limits<float64>::signaling_NaN(),
            100.0};
        const std::vector<float64> pA(pa, pa + sizeof(pa) / sizeof(pa[0]) );
        FixedReplayPlayerStrategy gS(foldOnly);
        FixedReplayPlayerStrategy tS(foldOnly);
        FixedReplayPlayerStrategy dS(foldOnly);
        FixedReplayPlayerStrategy pS(pA);
        FixedReplayPlayerStrategy aS(aA);
        FixedReplayPlayerStrategy mS(foldOnly);
        FixedReplayPlayerStrategy cS(aA);
        FixedReplayPlayerStrategy sS(foldOnly);


        PureGainStrategy bot("17.txt", 4);
        PlayerStrategy * const botToTest = &bot;
        myTable.ManuallyAddPlayer("MultiBotV", 1502.5, &mS);
        myTable.ManuallyAddPlayer("ConservativeBotV", 1500.0, &cS);
        myTable.ManuallyAddPlayer("SpaceBotV", 1500.0, &sS);
        myTable.ManuallyAddPlayer("GearBotV", 1500.0, &gS);
        myTable.ManuallyAddPlayer("DangerBotV", 1495.0, &dS);
        myTable.ManuallyAddPlayer("TrapBotV", 1500.0, &tS);
        myTable.ManuallyAddPlayer("ActionBotV", 1500.0, &aS);
        myTable.ManuallyAddPlayer("NormalBot17", 1502.5, botToTest);
        myTable.ManuallyAddPlayer("Nav", 1500.0, &pS);

        const playernumber_t dealer = 8;



        /*


         Preflop
         (Pot: $0)
         (9 players)
         [MultiBotV $1502.5]
         [ConservativeBotV $1500]
         [SpaceBotV $1500]
         [GearBotV $1500]
         [ActionBotV $1500]
         [NormalBotV $1502.5]
         [TrapBotV $1500]
         [Nav $1500]
         [DangerBotV $1495]
         */



        DeckLocation card;

        {
            CommunityPlus handToTest; // Ah 7h

            card.SetByIndex(21);
            handToTest.AddToHand(card);

            card.SetByIndex(49);
            handToTest.AddToHand(card);

            botToTest->StoreDealtHand(handToTest);
        }



        myTable.BeginInitialState(17);
        myTable.BeginNewHands(std::cout, b, false, dealer);

        /*

         MultiBotV posts SB of $5 ($5)
         ConservativeBotV posts BB of $10 ($15)
         */
        assert(myTable.PlayRound_BeginHand(std::cout) != -1);
        assert(myTable.ViewPlayer(7)->GetBetSize() >= 0);

        /*
         SpaceBotV folds
         GearBotV folds
         DangerBotV folds
         TrapBotV folds
         ActionBotV calls $10 ($25)
         NormalBotV calls $10 ($35)
         Nav calls $10 ($45)
         MultiBotV folds
         ConservativeBotV checks
         */


        CommunityPlus myFlop;

        card.SetByIndex(5);
        myFlop.AddToHand(card);

        card.SetByIndex(26);
        myFlop.AddToHand(card);

        card.SetByIndex(47);
        myFlop.AddToHand(card);
        /*

         Flop:	3h 8c Kd    (Pot: $45)
         */



        assert(myTable.PlayRound_Flop(myFlop, std::cout) != -1);

        /*
         (4 players)
         [ConservativeBotV $1490]
         [ActionBotV $1490]
         [NormalBotV $1492.5]
         [Nav $1490]

         ConservativeBotV checks
         ActionBotV checks
         NormalBotV checks
         Nav checks
         */

        DeckLocation myTurn;
        myTurn.SetByIndex(37);
        /*
         Turn:	3h 8c Kd Jh   (Pot: $45)
         */


        assert(myTable.PlayRound_Turn(myFlop, myTurn, std::cout) != -1);

        /*
         (4 players)
         [ConservativeBotV $1490]
         [ActionBotV $1490]
         [NormalBotV $1492.5]
         [Nav $1490]

         ConservativeBotV checks
         ActionBotV checks
         NormalBotV checks
         Nav checks
         */

        DeckLocation myRiver;
        myRiver.SetByIndex(9);
        /*

         River:	3h 8c Kd Jh 4h  (Pot: $45)
         */



        assert(myTable.PlayRound_River(myFlop, myTurn, myRiver, std::cout) != -1);
        assert(myTable.GetPotSize() > 125.0);

        /*
         (4 players)
         [ConservativeBotV $1490]
         [ActionBotV $1490]
         [NormalBotV $1492.5]
         [Nav $1490]

         ConservativeBotV checks
         ActionBotV checks
         NormalBotV bets $750 ($795)
         Nav folds
         ConservativeBotV folds
         ActionBotV folds

         All fold! NormalBotV wins $35

         */
        // You have the nuts, don't let everyone fold.
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


        BlindValues b;
        b.SetSmallBigBlind(5.0);

        HoldemArena myTable(b.GetSmallBlind(), true, true);
        myTable.setSmallestChip(5.0);

        const std::vector<float64> foldOnly(1, 0.0);
        std::vector<float64> nA; nA.push_back(365); nA.push_back(1045);
        FixedReplayPlayerStrategy gS(foldOnly);
        FixedReplayPlayerStrategy tS(foldOnly);
        FixedReplayPlayerStrategy dS(foldOnly);
        FixedReplayPlayerStrategy pS(foldOnly);
        FixedReplayPlayerStrategy nS(nA);
        FixedReplayPlayerStrategy mS(foldOnly);
        FixedReplayPlayerStrategy cS(foldOnly);
        FixedReplayPlayerStrategy sS(foldOnly);

        PureGainStrategy bot("14a.txt", 2);
        PlayerStrategy * const botToTest = &bot;
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
        myTable.BeginNewHands(std::cout, b, false, dealer);


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
        assert (myTable.PlayRound_BeginHand(std::cout) != -1);

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


        BlindValues b;
        b.SetSmallBigBlind(5.0);

        HoldemArena myTable(b.GetSmallBlind(), true, true);
        myTable.setSmallestChip(5.0);

        const std::vector<float64> foldOnly(1, 0.0);
        static const float64 arr[] = {55, 705, std::numeric_limits<float64>::signaling_NaN()};
        const std::vector<float64> aA(arr, arr + sizeof(arr) / sizeof(arr[0]) );
        FixedReplayPlayerStrategy gS(foldOnly);
        FixedReplayPlayerStrategy tS(foldOnly);
        FixedReplayPlayerStrategy dS(foldOnly);
        FixedReplayPlayerStrategy pS(foldOnly);
        FixedReplayPlayerStrategy aS(aA);
        FixedReplayPlayerStrategy mS(foldOnly);
        FixedReplayPlayerStrategy cS(foldOnly);
        FixedReplayPlayerStrategy sS(foldOnly);


        PureGainStrategy bot("13a.txt", 3);
        PlayerStrategy * const botToTest = &bot;
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
        myTable.BeginNewHands(std::cout, b, false, dealer);


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
        if (myTable.PlayRound_BeginHand(std::cout) == -1) {
            assert(myTable.GetPotSize() < 100);
        } else {
            assert(myTable.GetPotSize() < 200);
        }

    }

    // You have top pair on the river but there are flush threats and straight threats, do you call a small value bet?
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

        BlindValues b;
        b.SetSmallBigBlind(5.0);

        HoldemArena myTable(b.GetSmallBlind(), true, true);
        myTable.setSmallestChip(5.0);

        const std::vector<float64> foldOnly(1, 0.0);
        static const float64 arr[] = {std::numeric_limits<float64>::signaling_NaN(),
            std::numeric_limits<float64>::signaling_NaN(),
            std::numeric_limits<float64>::signaling_NaN(),
            10.0};
        const std::vector<float64> mA(arr, arr + sizeof(arr) / sizeof(arr[0]) );
        FixedReplayPlayerStrategy gS(foldOnly);
        FixedReplayPlayerStrategy tS(foldOnly);
        FixedReplayPlayerStrategy dS(foldOnly);
        FixedReplayPlayerStrategy pS(foldOnly);
        FixedReplayPlayerStrategy aS(foldOnly);
        FixedReplayPlayerStrategy mS(mA);
        FixedReplayPlayerStrategy cS(foldOnly);
        FixedReplayPlayerStrategy sS(foldOnly);



        PureGainStrategy bot("12.txt", 4);
        PlayerStrategy * const botToTest = &bot;
        myTable.ManuallyAddPlayer("NormalBot12", 1500.0, botToTest);
        myTable.ManuallyAddPlayer("DangerBotV", 1500.0, &dS); // is the small blind
        myTable.ManuallyAddPlayer("MultiBotV", 1500.0, &mS);
        myTable.ManuallyAddPlayer("ConservativeBotV", 1500, &cS);
        myTable.ManuallyAddPlayer("SpaceBotV", 1500.0, &sS);
        myTable.ManuallyAddPlayer("GearBotV", 1500.0, &gS);
        myTable.ManuallyAddPlayer("ActionBotV", 1500.0, &aS);
        myTable.ManuallyAddPlayer("TrapBotV", 1500.0, &tS);
        myTable.ManuallyAddPlayer("Ali", 1500.0, &pS); // Ali is the dealer, since DangerBot is the small blind


        const playernumber_t dealer = 0;


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
        myTable.BeginNewHands(std::cout, b, false, dealer);




        /*

         DangerBotV posts SB of $5 ($5)
         MultiBotV posts BB of $10 ($15)
         ConservativeBotV folds
         SpaceBotV folds
         GearBotV folds
         ActionBotV folds
         TrapBotV folds
         Ali folds
         NormalBotV calls $10 ($25)
         DangerBotV folds
         MultiBotV checks
         */

        assert(myTable.PlayRound_BeginHand(std::cout) != -1);

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
        assert(myTable.PlayRound_Flop(myFlop, std::cout) != -1);
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
        assert(myTable.PlayRound_Turn(myFlop, myTurn, std::cout) != -1);

        /*
         River:	4d 7h 9d Ad 8h  (Pot: $25)
         (2 players)
         [MultiBotV $1490]
         [NormalBotV $1490]
         */

        DeckLocation myRiver; // 8h
        myRiver.SetByIndex(25);

        /*

         MultiBotV bets SMALL
         NormalBotV call?
         */
        assert(myTable.PlayRound_River(myFlop, myTurn, myRiver, std::cout) != -1);
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

        BlindValues b;
        b.SetSmallBigBlind(5.0);

        HoldemArena myTable(b.GetSmallBlind(), true, true);
        myTable.setSmallestChip(5.0);

        const std::vector<float64> foldOnly(1, 0.0);
        static const float64 arr[] = {std::numeric_limits<float64>::signaling_NaN(),
            std::numeric_limits<float64>::signaling_NaN()};
        const std::vector<float64> pA(arr, arr + sizeof(arr) / sizeof(arr[0]) );
        FixedReplayPlayerStrategy gS(foldOnly);
        FixedReplayPlayerStrategy tS(foldOnly);
        FixedReplayPlayerStrategy nS(foldOnly);
        FixedReplayPlayerStrategy pS(pA);
        FixedReplayPlayerStrategy aS(foldOnly);
        FixedReplayPlayerStrategy mS(foldOnly);
        FixedReplayPlayerStrategy cS(foldOnly);
        FixedReplayPlayerStrategy sS(foldOnly);



        PureGainStrategy bot("11.txt", 2);
        PlayerStrategy * const botToTest = &bot;
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
        myTable.BeginNewHands(std::cout, b, false, dealer);


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
        myTable.PlayRound_BeginHand(std::cout);
        assert(myTable.GetPotSize() < 120);

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

        BlindValues b;
        b.SetSmallBigBlind(5.625);

        HoldemArena myTable(b.GetSmallBlind(), true, true);
        myTable.setSmallestChip(5.0);

        const std::vector<float64> foldOnly(1, 0.0);
        static const float64 arr[] = {11.25, 80.0, 30.0, 125.0};
        const std::vector<float64> nA(arr, arr + sizeof(arr) / sizeof(arr[0]) );
        FixedReplayPlayerStrategy gS(foldOnly);

        FixedReplayPlayerStrategy nS(nA);
        FixedReplayPlayerStrategy pS(foldOnly);
        FixedReplayPlayerStrategy dS(foldOnly);
        FixedReplayPlayerStrategy mS(foldOnly);
        FixedReplayPlayerStrategy cS(foldOnly);
        FixedReplayPlayerStrategy sS(foldOnly);


        PureGainStrategy bot("6.txt", 2);
        PlayerStrategy * const botToTest = &bot;

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
        myTable.BeginNewHands(std::cout, b, false, dealer);


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
        assert(myTable.PlayRound_BeginHand(std::cout) != -1);



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

        playernumber_t highBet = myTable.PlayRound_Flop(myFlop, std::cout);
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
        assert(myTable.PlayRound_Turn(myFlop, myTurn, std::cout) != -1);

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
        assert(myTable.PlayRound_River(myFlop, myTurn, myRiver, std::cout) == -1);
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

        BlindValues b;
        b.SetSmallBigBlind(5.0625);

        HoldemArena myTable(b.GetSmallBlind(), true, true);

        const std::vector<float64> foldOnly(1, 0.0);
        static const float64 arr[] = {10.125, 18.0, 805.5, 1489.88};
        const std::vector<float64> aA(arr, arr + sizeof(arr) / sizeof(arr[0]) );
        FixedReplayPlayerStrategy gS(foldOnly);

        FixedReplayPlayerStrategy mS(foldOnly);
        FixedReplayPlayerStrategy sS(foldOnly);
        FixedReplayPlayerStrategy aS(aA);
        FixedReplayPlayerStrategy dS(foldOnly);
        FixedReplayPlayerStrategy pS(foldOnly);
        FixedReplayPlayerStrategy nS(foldOnly);


        PureGainStrategy bot("5.txt", 2);
        PlayerStrategy * const botToTest = &bot;

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
        myTable.BeginNewHands(std::cout, b, false, dealer);


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

        assert(myTable.PlayRound_BeginHand(std::cout) != -1);


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

        const playernumber_t flopCalledIdx = myTable.PlayRound_Flop(myFlop, std::cout);
        //assert(myTable.PlayRound_Flop(myFlop, std::cout) != 1);
        if (flopCalledIdx != -1) {

            /*
             Turn:	4s 5h Kd Ah   (Pot: $3005.06)
             (2 players)
             */

            DeckLocation myTurn; // Ah
            myTurn.SetByIndex(49);

            assert(myTable.PlayRound_Turn(myFlop, myTurn, std::cout) != 1);
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

        BlindValues b;
        b.SetSmallBigBlind(1.0);

        HoldemArena myTable(b.GetSmallBlind(), true, true);

        const std::vector<float64> foldOnly(1, 0.0);
        static const float64 arr[] = {5.0, std::numeric_limits<float64>::signaling_NaN(), 0, 10.0, 28.0, 50.0, 347.0, 736.0, 927.0};
        const std::vector<float64> pA(arr, arr + sizeof(arr) / sizeof(arr[0]) );

        FixedReplayPlayerStrategy sS(foldOnly);
        FixedReplayPlayerStrategy pS(pA);

        FixedReplayPlayerStrategy cS(foldOnly);
        FixedReplayPlayerStrategy dS(foldOnly);
        FixedReplayPlayerStrategy mS(foldOnly);
        FixedReplayPlayerStrategy nS(foldOnly);
        FixedReplayPlayerStrategy aS(foldOnly);
        FixedReplayPlayerStrategy tS(foldOnly);


        PureGainStrategy bot("4.txt", 0);
        PlayerStrategy * const botToTest = &bot;
        myTable.ManuallyAddPlayer("TrapBotV", 1497.0, &tS);
        myTable.ManuallyAddPlayer("SpaceBotV", 1498.0, &sS); // small blind
        myTable.ManuallyAddPlayer("Nav", 2960.0, &pS);
        myTable.ManuallyAddPlayer("ConservativeBotV", 346.0, &cS);
        myTable.ManuallyAddPlayer("DangerBotV", 1496.0, &dS);
        myTable.ManuallyAddPlayer("MultiBotV", 2657.0, &mS);
        myTable.ManuallyAddPlayer("GearBot4", 1507.0, botToTest);
        myTable.ManuallyAddPlayer("NormalBotV", 1064.0, &nS);
        myTable.ManuallyAddPlayer("ActionBotV", 475.0, &aS);

        const playernumber_t dealer = 0;
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
        myTable.BeginNewHands(std::cout, b, false, dealer);

        /*
         SpaceBotV posts SB of $1 ($1)
         Nav posts BB of $2 ($3)
         ConservativeBotV folds
         DangerBotV folds
         MultiBotV folds
         GearBotV calls $2 ($5)
         NormalBotV folds
         ActionBotV folds
         TrapBotV folds
         SpaceBotV folds
         Nav raises to $5 ($8)
         GearBotV calls ($11)
         */
        assert(myTable.PlayRound_BeginHand(std::cout) != -1);


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
        assert(myTable.PlayRound_Flop(myFlop, std::cout) != -1);

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
        assert(myTable.PlayRound_Turn(myFlop, myTurn, std::cout) != -1);
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
        playernumber_t highBet = myTable.PlayRound_River(myFlop, myTurn, myRiver, std::cout);
        //assert (myTable.ViewPlayer(highBet) == &botToTest->ViewPlayer());

        // do not get pushed out, you have a winning hand
        if(highBet == -1){
            assert(myTable.ViewPlayer(6)->GetBetSize() >= 0);
        }


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


    // 2013.08.30-19.58.15
    // Hand #11
    // Perspective, SpaceBot
    void testHybrid_002() {

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
        BlindValues b;
        b.SetSmallBigBlind(1.125);

        HoldemArena myTable(b.GetSmallBlind(), true, true);


        const std::vector<float64> foldOnly(1, 0.0);
        static const float64 arr[] = {5.0, 12.5, 49, 168.0, 459.0, 495.0};
        const std::vector<float64> pA(arr, arr + sizeof(arr) / sizeof(arr[0]) );

        FixedReplayPlayerStrategy cS(foldOnly);
        FixedReplayPlayerStrategy dS(foldOnly);
        FixedReplayPlayerStrategy mS(foldOnly);
        FixedReplayPlayerStrategy nS(foldOnly);
        FixedReplayPlayerStrategy aS(foldOnly);
        FixedReplayPlayerStrategy pS(pA);
        FixedReplayPlayerStrategy gS(foldOnly);


        PureGainStrategy bot("2.txt", 2);
        PlayerStrategy * const botToTest = &bot;
        myTable.ManuallyAddPlayer("ConservativeBotV", 344.0, &cS);
        myTable.ManuallyAddPlayer("DangerBotV", 1496.0, &dS);
        myTable.ManuallyAddPlayer("MultiBotV", 2657.0, &mS);
        myTable.ManuallyAddPlayer("NormalBotV", 1064.0, &nS);
        myTable.ManuallyAddPlayer("ActionBotV", 475.0, &aS);
        myTable.ManuallyAddPlayer("SpaceBot2", 717.0, botToTest);
        myTable.ManuallyAddPlayer("Nav", 2474.0, &pS);
        myTable.ManuallyAddPlayer("GearBotV", 4273.0, &gS); // gearbot is the dealer, since ConservativeBot is the small blind

        const playernumber_t dealer = 7;
        myTable.setSmallestChip(1.0);

        myTable.BeginInitialState();
        myTable.BeginNewHands(std::cout, b, false, dealer);


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
        HoldemArenaBetting r( &myTable, CommunityPlus::EMPTY_COMPLUS, 0 , &(std::cout));
        struct MinRaiseError msg;

        r.MakeBet(0.0, &msg);
        r.MakeBet(0.0, &msg);
        r.MakeBet(0.0, &msg);
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





    // The chance of raising can sometimes outweight the probability of the hand strength you consider facing in that environment.
    // e.g. OppRAISEChanceR [F] 0.706773 @ $30	fold -- left0.148716  0.148716 right	W(5.66667x)=0.310007 L=0.649279 1o.w_s=(0.310007,0.040714)
    // where we expect to be raised 70% of the time (by our singular opponent) but the chance to win is way more than againstBest 70%

    // Hypothesis #1: sliderx needs to be passed through to OpponentHandOpportunity in order for the winPCT to make it less profitable for opponents to fold
    // Hypothesis #2: OpponentHandOpportunity needs to also compute the probability of raise code and use that to determine the opponent's setW to find N.
    //
    void testRegression_019() {

        /*

         Next Dealer is ConservativeBotV


         Preflop
         (Pot: $0)
         (9 players)
         [SpaceBotV $1490]
         [GearBotV $1500]
         [ActionBotV $1507.5]
         [NormalBotV $1500]
         [TrapBotV $1500]
         [Nav $1507.5]
         [DangerBotV $1495]
         [MultiBotV $1500]
         [ConservativeBotV $1500]
         */

        const playernumber_t dealer = 0;

        BlindValues bl;
        bl.SetSmallBigBlind(5.0);

        HoldemArena myTable(bl.GetSmallBlind(), true, true);
        myTable.setSmallestChip(5.0);


        PureGainStrategy bot("19.txt", 3);
        PlayerStrategy * const botToTest = &bot;

        const std::vector<float64> foldOnly(1, 0.0);

        FixedReplayPlayerStrategy gS(foldOnly);
        FixedReplayPlayerStrategy cS(foldOnly);
        FixedReplayPlayerStrategy dS(foldOnly);


        myTable.ManuallyAddPlayer("DangerBot", 1495.0, &dS);
        myTable.ManuallyAddPlayer("MultiBot19", 1505.0, botToTest);
        myTable.ManuallyAddPlayer("ConservativeBotV", 1500.0, &cS); // big blind
        myTable.ManuallyAddPlayer("Space", 1500.0, &gS);
        myTable.ManuallyAddPlayer("GearBotV", 1500.0, &gS);
        myTable.ManuallyAddPlayer("Action", 1500.0, &gS);
        myTable.ManuallyAddPlayer("Normal", 1500.0, &gS);
        myTable.ManuallyAddPlayer("Trap", 1500.0, &gS);
        myTable.ManuallyAddPlayer("Nav", 1500.0, &gS);



        myTable.BeginInitialState(19);
        myTable.BeginNewHands(std::cout, bl, false, dealer);



        DeckLocation card;
        CommunityPlus withCommunity; // 4d Kd

        card.SetByIndex(24);
        withCommunity.AddToHand(card);

        card.SetByIndex(47);
        withCommunity.AddToHand(card);

        bot.StoreDealtHand(withCommunity);

        /*



         Preflop
         (Pot: $0)
         (9 players)
         [MultiBotV $1505]
         [ConservativeBotV $1500]
         [SpaceBotV $1500]
         [GearBotV $1500]
         [ActionBotV $1500]
         [NormalBotV $1500]
         [TrapBotV $1500]
         [Nav $1500]
         [DangerBotV $1495]

         MultiBotV posts SB of $5 ($5)
         ConservativeBotV posts BB of $10 ($15)
         SpaceBotV folds
         GearBotV folds
         ActionBotV folds
         NormalBotV folds
         TrapBotV folds
         Nav folds
         DangerBotV folds
         MultiBotV folds


         All fold! ConservativeBotV wins $5



         */
        myTable.PrepBettingRound(true,3);  //flop, turn, river remaining
        HoldemArenaBetting r( &myTable, CommunityPlus::EMPTY_COMPLUS, 0 , &(std::cout));

        struct MinRaiseError msg;

        r.MakeBet(0.0, &msg); // SpaceBot folds
        r.MakeBet(0.0, &msg);
        r.MakeBet(0.0, &msg);
        r.MakeBet(0.0, &msg);
        r.MakeBet(0.0, &msg);
        r.MakeBet(0.0, &msg);
        r.MakeBet(0.0, &msg);

        const float64 actual = bot.MakeBet();

        /*

         ==========#5==========
         Playing as D
         *
         (M) 54.8846%
         (M.w) 52.8888%
         (M.s) 3.99159%
         (M.l) 43.1196%
         (Better All-in) 71.1429%
         (Re.s) 0.244898%
         (Better Mean Rank) 67.7143%
         (Ra.s) 0.244898%

         4d Kd Bet to call 10 (from 5) at 15 pot,
         */

        CommunityPlus communityToTest; // EMPTY_COMPLUS



        const int8 cardsInCommunity = 0;

        StatResultProbabilities statprob;

        ///Compute CallStats
        StatsManager::QueryDefense(statprob.core.handcumu,withCommunity,communityToTest,cardsInCommunity);
        statprob.core.foldcumu = CoreProbabilities::ReversePerspective(statprob.core.handcumu);

        ///Compute CommunityCallStats
        StatsManager::QueryOffense(statprob.core.callcumu,withCommunity,communityToTest,cardsInCommunity,0);

        ///Compute WinStats
        DistrShape detailPCT(DistrShape::newEmptyDistrShape());
        StatsManager::Query(&detailPCT,withCommunity,communityToTest,cardsInCommunity);
        statprob.core.playerID = 0;
        statprob.core.statmean = detailPCT.mean;


        /*
         *
         Playing as N
         *
         (M) 56.0202%
         (M.w) 54.4316%
         (M.s) 3.17708%
         (M.l) 42.3913%
         (Better All-in) 74.7347%
         (Re.s) 0.244898%
         (Better Mean Rank) 73.9184%
         (Ra.s) 0.571429%

         *
         8s Kd Bet to call 10 (from 5) at 15 pot,
         CallStrength W(1c)=0.544316 L=0.423913 o.w_s=(0.544316,0.0317708)
         (MinRaise to $20) 	W(1x)=0.544316 L=0.423913 1o.w_s=(0.544316,0.0317708)
         -  statranking ca (algb) -
         9 dealt, 8 opp. (round), 1 opp. assumed str., 1 opp. still in
         Choice Optimal 10
         Choice Fold 10
         f(10)=0.996481
         CHECK/FOLD
         FoldGain()R=0.999077 x 0(=0 folds)	vs play:0.995558   ->assumes $5 forced
         AgainstCall(10)=1.00024 from +$1.20403 @ 0.295854
         AgainstRaise(10)=0.995321 from -$10 @ 0.704146
         Push(10)=1 from $0 @ 0
         AgainstCall(20)=1.0004 from +$2.40807 @ 0.249265
         AgainstRaise(20)=0.990977 from -$20 @ 0.678975
         Push(20)=1.00057 from +$11.8679 @ 0.0717593
         OppRAISEChanceR [*] 0.468719 @ $20	fold -- left0.0717593  0.0717593 right	W(1x)=0.544316 L=0.423913 1o.w_s=(0.544316,0.0317708)
         OppRAISEChanceR [F] 0.704146 @ $30	fold -- left0.145775  0.145775 right	W(5.66667x)=0.294695 L=0.678838 1o.w_s=(0.294695,0.0264674)
         OppRAISEChanceR [F] 0.487599 @ $50	fold -- left0.398119  0.398119 right	W(7x)=0.273692 L=0.694668 1o.w_s=(0.273692,0.0316399)
         OppRAISEChanceR [F] 0.319759 @ $90	fold -- left0.565918  0.565918 right	W(9x)=0.250999 L=0.709496 1o.w_s=(0.250999,0.0395046)
         OppRAISEChanceR [F] 0.184107 @ $170	fold -- left0.665218  0.665218 right	W(12x)=0.227665 L=0.72714 1o.w_s=(0.227665,0.0451946)
         OppRAISEChanceR [F] 0.0974442 @ $330	fold -- left0.726338  0.726338 right	W(16.3333x)=0.217968 L=0.742605 1o.w_s=(0.217968,0.0394277)
         OppRAISEChanceR [F] 0.0448035 @ $650	fold -- left0.768967  0.768967 right	W(22.6667x)=0.206164 L=0.758251 1o.w_s=(0.206164,0.0355852)
         OppRAISEChanceR [F] 0.0133163 @ $1290	fold -- left0.810305  0.810305 right	W(31.5x)=0.198258 L=0.779667 1o.w_s=(0.198258,0.0220749)
         OppRAISEChanceR [F] 0.00901911 @ $1500	fold -- left0.779127  0.779127 right	W(24.1667x)=0.204801 L=0.762366 1o.w_s=(0.204801,0.0328328)
         Guaranteed > $0 is in the pot for sure
         OppFoldChance% ...    0   d\0

         */
        const playernumber_t myPositionIndex = 1;
        ExpectedCallD   tablestate(myPositionIndex, &myTable, statprob.statranking.pct, statprob.core.statmean.pct);

        ExactCallD pr_opponentcallraise(&tablestate, statprob.core);
        ExactCallBluffD myDeterredCall(&tablestate, statprob.core);

        OpponentHandOpportunity opponentHandOpportunity(myPositionIndex, myTable, statprob.core);

        //const int32 firstFold = 1;
        const int32 i = 1;
        //const float64 raiseCount = myDeterredCall.pRaise(myTable.GetBetToCall(), i, firstFold);

        opponentHandOpportunity.query(ExactCallD::RaiseAmount(tablestate, myTable.GetBetToCall(), i));
        //const float64 pessimisticHandCount = 1.0 / opponentHandOpportunity.handsToBeat();

        // pessimisticHandCount should not be MORE RARE than raiseCount.
        // BUT it doesn't matter because we don't apply raiseGain when we would fold.
        // We simply apply 1.0 - hypotheticalRaiseTo regardless of win percentages.
        // So really, the way we get this to pass is to hope that firstFold is higher.
        //assert(raiseCount <= pessimisticHandCount);


        assert(0 < actual);
    }


    // 2013.08.30-19.58.15
    // Hand #11
    // Perspective, SpaceBot
    //
    // We have a situation with SpaceBot folding pre-flop here due to...?
    //  Hypothesis #1: Re-raise Pessimistic is too strict since it's assuming we're betting directly that much but really it would be a re-raise so we'd be facing that bet.
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
        BlindValues b;
        b.SetSmallBigBlind(1.125);

        HoldemArena myTable(b.GetSmallBlind(), true, true);

        std::vector<float64> bbOnly; bbOnly.push_back(b.GetBigBlind()); bbOnly.push_back(0.0); bbOnly.push_back(0.0); bbOnly.push_back(0.0); bbOnly.push_back(0.0);
        const std::vector<float64> foldOnly(1, 0.0);
        static const float64 arr[] = {5.0, 12.5, 49.0, 100.0, 228.0, 459.0, 495.0};
        const std::vector<float64> pA(arr, arr + sizeof(arr) / sizeof(arr[0]) );

        FixedReplayPlayerStrategy cS(foldOnly);
        FixedReplayPlayerStrategy dS(bbOnly);
        FixedReplayPlayerStrategy mS(foldOnly);
        FixedReplayPlayerStrategy nS(foldOnly);
        FixedReplayPlayerStrategy aS(foldOnly);
        FixedReplayPlayerStrategy pS(pA);
        FixedReplayPlayerStrategy gS(foldOnly);


        PureGainStrategy bot("9.txt", 0);
        PlayerStrategy * const botToTest = &bot;
        myTable.ManuallyAddPlayer("SpaceBot9", 717.0, botToTest);
        myTable.ManuallyAddPlayer("Nav", 2474.0, &pS); // small blind
        myTable.ManuallyAddPlayer("DangerBotV", 1496.0, &dS);
        myTable.ManuallyAddPlayer("MultiBotV", 2657.0, &mS);
        myTable.ManuallyAddPlayer("NormalBotV", 1064.0, &nS);
        myTable.ManuallyAddPlayer("ActionBotV", 475.0, &aS);
        myTable.ManuallyAddPlayer("GearBotV", 4273.0, &gS);
        myTable.ManuallyAddPlayer("ConservativeBotV", 344.0, &cS);

        const playernumber_t dealer = 0;
        myTable.setSmallestChip(1.0);



        myTable.BeginInitialState(9);
        myTable.BeginNewHands(std::cout, b, false, dealer);

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
         GearBotV folds
         SpaceBotV raises to $5 ($8.375)
         Nav calls $5 ($13.375)
         ConservativeBotV folds
         DangerBotV folds
         */
        assert(myTable.PlayRound_BeginHand(std::cout) != -1);
        assert(myTable.ViewPlayer(0)->GetBetSize() >= 0);


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

        assert(myTable.PlayRound_Flop(myFlop, std::cout) != -1);
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
        playernumber_t highbet = myTable.PlayRound_Turn(myFlop, myTurn, std::cout);
        if (highbet == -1) {
            // all-fold?
            assert(myTable.ViewPlayer(0)->GetBetSize() < 0); // SpaceBot should be the one that folded.
        } else
        {
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
        highbet = (myTable.PlayRound_River(myFlop, myTurn, myRiver, std::cout) );
            if (highbet == -1) {
                // all-fold?
                assert(myTable.ViewPlayer(0)->GetBetSize() < 0); // SpaceBot should be the one that folded.
            }
        /*
         SpaceBotV bets $4 ($451.375)
         Nav raises to $459 ($910.375)
         SpaceBotV raises all-in to $495 ($1401.38)
         Nav calls $36 ($1437.38)
         */
        }

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

    // Test OpposingHandOpportunity derivatives
    void testHybrid_008c() {



        BlindValues bl;
        bl.SetSmallBigBlind(5.0);

        HoldemArena myTable(bl.GetSmallBlind(), true, true);
        myTable.setSmallestChip(5.0);

        const std::vector<float64> foldOnly(1, 0.0);
        static const float64 aa[] = {
            std::numeric_limits<float64>::signaling_NaN(),
            std::numeric_limits<float64>::signaling_NaN(),
            std::numeric_limits<float64>::signaling_NaN(),
            std::numeric_limits<float64>::signaling_NaN(),
            std::numeric_limits<float64>::signaling_NaN(),
            std::numeric_limits<float64>::signaling_NaN()};
        const std::vector<float64> callOnly(aa, aa + sizeof(aa) / sizeof(aa[0]) );
        FixedReplayPlayerStrategy gS(callOnly);
        FixedReplayPlayerStrategy tS(foldOnly);
        FixedReplayPlayerStrategy dS(foldOnly);
        FixedReplayPlayerStrategy pS(foldOnly);
        FixedReplayPlayerStrategy nS(callOnly);
        FixedReplayPlayerStrategy mS(foldOnly);
        FixedReplayPlayerStrategy cS(foldOnly);
        FixedReplayPlayerStrategy sS(foldOnly);
        FixedReplayPlayerStrategy aS(foldOnly);


        myTable.ManuallyAddPlayer("MultiBotV", 1590.0, &mS);
        myTable.ManuallyAddPlayer("ConservativeBotV", 1582.97, &cS);
        myTable.ManuallyAddPlayer("SpaceBotV", 1485.0, &sS);
        myTable.ManuallyAddPlayer("GearBotV", 1545.0, &gS);
        myTable.ManuallyAddPlayer("ActionBot18", 1505.0, &aS);
        myTable.ManuallyAddPlayer("NormalBotV", 1473.52, &nS);
        myTable.ManuallyAddPlayer("TrapBotV", 1473.52, &tS);
        myTable.ManuallyAddPlayer("Nav", 1450.0, &pS);
        myTable.ManuallyAddPlayer("DangerBotV", 1495.0, &dS);


        /*

         Preflop
         (Pot: $0)
         (9 players)
         [ActionBotV $1505]
         [NormalBotV $1473.52]
         [TrapBotV $1473.52]
         [Nav $1450]
         [DangerBotV $1495]
         [MultiBotV $1590]
         [ConservativeBotV $1582.97]
         [SpaceBotV $1485]
         [GearBotV $1445]

         */


        const playernumber_t dealer = 3;


        /*


         ActionBotV posts SB of $5 ($5)
         NormalBotV posts BB of $10 ($15)

         */

        myTable.BeginInitialState(18);
        myTable.BeginNewHands(std::cout, bl, false, dealer);

        /*
         TrapBotV folds
         Nav folds
         DangerBotV folds
         MultiBotV folds
         ConservativeBotV folds
         SpaceBotV folds
         GearBotV calls $10 ($25)
         ActionBotV calls $5 ($30)
         NormalBotV checks
         */


        myTable.PrepBettingRound(true,3);  //flop, turn, river remaining

        {
        HoldemArenaBetting r( &myTable, CommunityPlus::EMPTY_COMPLUS, 0 , &(std::cout));

            struct MinRaiseError msg;


        r.MakeBet(0.0, &msg);

        r.MakeBet(0.0, &msg);

        r.MakeBet(0.0, &msg);

        r.MakeBet(0.0, &msg);

        r.MakeBet(0.0, &msg);

        r.MakeBet(0.0, &msg);

        r.MakeBet(10.0, &msg);
        r.MakeBet(10.0, &msg);
        r.MakeBet(10.0, &msg);
        }




        /*


         Flop:	4h 7d Tc   (Pot: $30)

         */

        myTable.PrepBettingRound(false,2); //turn, river remaining


        DeckLocation card;


        CommunityPlus withCommunity;
        CommunityPlus community;

        card.SetByIndex(9);
        community.AddToHand(card);
        withCommunity.AddToHand(card);

        card.SetByIndex(23);
        community.AddToHand(card);
        withCommunity.AddToHand(card);

        card.SetByIndex(34);
        community.AddToHand(card);
        withCommunity.AddToHand(card);

        {
            HoldemArenaBetting r( &myTable, community, 3, &(std::cout) );

            struct MinRaiseError msg;

            r.MakeBet(0.0, &msg);

            r.MakeBet(0.0, &msg);

            r.MakeBet(0.0, &msg);
        }



        /*

         (3 players)
         [ActionBotV $1495]
         [NormalBotV $1463.52]
         [GearBotV $1453]

         ActionBotV checks
         NormalBotV checks
         GearBotV checks

         */

        DeckLocation myTurn;
        myTurn.SetByIndex(51);
        community.AddToHand(myTurn);

        /*

         Turn:	4h 7d Tc Ad   (Pot: $30)
         */


        myTable.PrepBettingRound(false,1); //river remaining


        {
            HoldemArenaBetting r( &myTable, community, 4 , &(std::cout));

            struct MinRaiseError msg;

            r.MakeBet(0.0, &msg);

            r.MakeBet(0.0, &msg);

            r.MakeBet(0.0, &msg);
        }



        /*


         (3 players)
         [ActionBotV $1495]
         [NormalBotV $1463.52]
         [GearBotV $1453]

         ActionBotV checks
         NormalBotV checks
         GearBotV checks
         */

        DeckLocation myRiver;
        myRiver.SetByIndex(49);
        community.AddToHand(myRiver);
        /*


         River:	4h 7d Tc Ad Ah  (Pot: $30)
         */


        myTable.PrepBettingRound(false,0); //last betting round



        /*

         (3 players)
         [ActionBotV $1495]
         [NormalBotV $1463.52]
         [GearBotV $1453]

         ActionBotV checks
         NormalBotV checks
         GearBotV checks



         */



        // Ac Jd
        card.SetByIndex(39);
        withCommunity.AddToHand(card);

        card.SetByIndex(50);
        withCommunity.AddToHand(card);

        // turn, river
        withCommunity.Hand::AddToHand(myTurn);
        withCommunity.Hand::AddToHand(myRiver);

        std::cout << "Starting next round..." << endl;


        StatResultProbabilities statprob;

        ///Compute CallStats
        StatsManager::QueryDefense(statprob.core.handcumu,withCommunity,community,5);
        statprob.core.foldcumu = CoreProbabilities::ReversePerspective(statprob.core.handcumu);

        ///Compute CommunityCallStats
        StatsManager::QueryOffense(statprob.core.callcumu,withCommunity,community,5,0);

        assert(statprob.core.handcumu.cumulation.size() > 2);

        const float64 testBet = 20.0;

        const float64 avgBlinds = myTable.GetBlindValues().OpportunityPerHand(myTable.NumberAtTable());


        const float64 opponentsFacingThem = 1.0;

        FoldWaitLengthModel<void, void> waitLength;
        waitLength.setMeanConv(
          //(opponentsFacingThem > 1.0) ?
          nullptr
          //: &(fCore.callcumu)
        );
        // ( 1 / (x+1) )  ^ (1/x)
        const Player * const normalBot = myTable.ViewPlayer(5);
        waitLength.bankroll = normalBot->GetMoney();
        waitLength.amountSacrificeForced = avgBlinds;
        waitLength.setAmountSacrificeVoluntary( normalBot->GetBetSize()
#ifdef SACRIFICE_COMMITTED
                                                  + normalBot->GetVoluntaryContribution()
#endif
                                                  - waitLength.amountSacrificeForced // exclude forced portion since it wasn't voluntary.
                                                  );
        waitLength.opponents = opponentsFacingThem;
        waitLength.prevPot = myTable.GetPrevPotSize();
        waitLength.betSize = testBet;

        vector<float64> wEffective;
        for (float64 w = 0.005; w < 1.0; w += 0.01) {
            waitLength.setW( w ); // As a baseline, set this so that the overall showdown win percentage required is "1.0 / tableStrength" per person after pow(..., opponents);
            const float64 n = waitLength.FindBestLength();
            const float64 rarity = 1.0 - w;
            const float64 playW = 1.0 - 1.0 / n / rarity;
            if (w < playW) {
                wEffective.push_back(playW);
            } else {
                wEffective.push_back(w);
            }
        }

        float64 meanR = 0.0;
        for(float64 playR : wEffective) {
            meanR += playR;
        }
        meanR /= wEffective.size();
        const float64 nEffective = 1.0 / (1.0 - meanR);


        OpponentHandOpportunity test(4, myTable, statprob.core);



        test.query(testBet);
        const float64 actual_y = test.handsToBeat();
        const float64 actual_Dy = test.d_HandsToBeat_dbetSize();

        assert(nEffective < actual_y);
        assert(actual_y < nEffective * 2);
        assert(actual_Dy > 0); // betting more should increase N even more

/*
        CombinedStatResultsPessimistic testC(test, statprob.core);
        //testC.query(testBet);

        const float64 unreasonableBet = 200.0;

        const float64 s1 = testC.ViewShape(unreasonableBet).splits;
        const float64 w = testC.getWinProb(unreasonableBet);
        const float64 l = testC.getLoseProb(unreasonableBet);
        const float64 dw = testC.get_d_WinProb_dbetSize(unreasonableBet);
        const float64 dl = testC.get_d_LoseProb_dbetSize(unreasonableBet);

        assert(testC.ViewShape(unreasonableBet).wins + testC.ViewShape(unreasonableBet).splits + testC.ViewShape(unreasonableBet).loss == 1.0);

        assert(w == 1.0);
        assert(l+w == 1.0);
        assert(s1 < 0.25);
        assert(dw < 0);
        assert(dl == -dw);
 */

    }


    // Test OpposingHandOpportunity derivatives
    void testHybrid_008() {

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

        BlindValues b;
        b.SetSmallBigBlind(0.125);

        HoldemArena myTable(b.GetSmallBlind(), true, true);
        myTable.setSmallestChip(0.125);

        std::vector<float64> aC; aC.push_back(0.25);
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
        myTable.BeginNewHands(std::cout, b, false, dealer);


        /*
         C calls $0.25 ($0.625)
         D calls $0.25 ($0.875)
         A calls $0.25 ($1.0)
         B checks
         */
        assert(myTable.PlayRound_BeginHand(std::cout) != -1);

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
        statprob.core.foldcumu = CoreProbabilities::ReversePerspective(statprob.core.handcumu);

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


    void testHybrid_drisk_handsIn() {

      FixedReplayPlayerStrategy p1(std::vector<float64>{});
      FixedReplayPlayerStrategy p2(std::vector<float64>{});
      FixedReplayPlayerStrategy px(std::vector<float64>{});
      FixedReplayPlayerStrategy p3(std::vector<float64>{});
      FixedReplayPlayerStrategy p4(std::vector<float64>{});

      BlindValues b;
      b.SetSmallBigBlind(5.0);
      HoldemArena myTable(b.GetSmallBlind(), true, true);
      myTable.setSmallestChip(5.0);

      myTable.ManuallyAddPlayer("P1", 600.0, &p1);
      myTable.ManuallyAddPlayer("Px", 6000.0, &px);
      myTable.ManuallyAddPlayer("P2", 3000.0, &p2); // dealer
      myTable.ManuallyAddPlayer("P3", 18000.0, &p3);
      myTable.ManuallyAddPlayer("P4", 24000.0, &p4);

      const playernumber_t dealer = 2;

      myTable.BeginInitialState(777);
      myTable.BeginNewHands(std::cout, b, false, dealer);

      myTable.PrepBettingRound(true,3);  //flop, turn, river remaining

      {
          HoldemArenaBetting r( &myTable, CommunityPlus::EMPTY_COMPLUS, 0 , &(std::cout));

          struct MinRaiseError msg;

          // P3 small blind $5
          // P4 big blind $10
          r.MakeBet(0, &msg);  // P1
          r.MakeBet(0, &msg);  // Px
          r.MakeBet(0, &msg);  // P2
          // r.MakeBet(20, &msg); // P3

      }
      const playernumber_t myPositionIndex = 3; // P4's turn next

      DeckLocation card;

          // Give P3 a bad hand, so they are more likely to fold (so that RiskLoss gives us an interesting result)
          CommunityPlus handToTest; // 46x
          card.SetByIndex(8);
          handToTest.AddToHand(card);
          card.SetByIndex(17);
          handToTest.AddToHand(card);

      /// BEGIN TEST, P4 to act

          DistrShape detailPCT(DistrShape::newEmptyDistrShape());
          CoreProbabilities core;

          // Mimic src/stratPosition.cpp:PositionalStrategy::SeeCommunity
          {
          ///Compute CallStats
          /*
          StatsManager::QueryDefense(core.handcumu,withCommunity,onlyCommunity,cardsInCommunity);
          core.foldcumu = core.handcumu;
          core.foldcumu.ReversePerspective();
          */

          ///Compute CommunityCallStats
          myTable.CachedQueryOffense(core.callcumu,CommunityPlus::EMPTY_COMPLUS, handToTest);

          ///Compute WinStats
          StatsManager::Query(&detailPCT,handToTest,CommunityPlus::EMPTY_COMPLUS, 0);

          core.statmean = detailPCT.mean; // needed for statRanking()
          }


      // StatResultProbabilities statprob;
      ExpectedCallD   tablestate_tableinfo(myPositionIndex, &myTable, core.statRanking().pct, detailPCT.mean.pct);

      // Mimic src/callPrediction.cpp#ExactCallD::query

      //const float64 totalexf = tablestate_tableinfo.table->GetPotSize() - tablestate_tableinfo.alreadyBet()  +  p4_betSize;
      ChipPositionState cps = {
        p4.ViewPlayer().GetMoney(),
        myTable.GetPotSize(), // totalexf
        p4.ViewPlayer().GetBetSize(),
        p4.ViewPlayer().GetVoluntaryContribution(),
        myTable.GetPrevPotSize()
      };

      const playernumber_t N = tablestate_tableinfo.handsDealt();
      const float64 avgBlind = myTable.GetBlindValues().OpportunityPerHand(N);

      HypotheticalBet hypothetical = {
        cps,
        50.0,
        15.0,
        cps.alreadyBet,
        false,
        true
      };
    } // end testHybrid_drisk_handsIn
}

void print_lineseparator(const char* const separator_msg) {

  std::cout << "────────────────────────────────────────────────────────────────────────────────" << endl;
  const time_t now = time(0);
  std::cout << asctime(std::localtime(&now)) << separator_msg << endl;
  std::cout << "────────────────────────────────────────────────────────────────────────────────" << endl;
}

static void all_unit_tests() {
  // Run all unit tests.
  NamedTriviaDeckTests::testNamePockets();


  UnitTests::testUnit_024();
  UnitTests::testMatrixbase_023();
  UnitTests::testUnit_020();

  UnitTests::testUnit_016();
  UnitTests::testUnit_015();
  UnitTests::testUnit_010c();
  UnitTests::testUnit_010b();
  UnitTests::testUnit_010();
  UnitTests::testUnit_007();
  UnitTests::testUnit_007b();
  UnitTests::testUnit_007c();
  UnitTests::testUnit_002b();
  UnitTests::testUnit_003();
}

static void all_regression_tests() {
    RegressionTests::testRegression_028();
    RegressionTests::testRegression_027();
    RegressionTests::testRegression_026();
    RegressionTests::testRegression_025();
    RegressionTests::testRegression_021b();
    //RegressionTests::testRegression_022(); // In later versions of the code, the bot is more tight-aggressive in this situation so it's not an issue anymore
//    RegressionTests::testRegression_021(); // TODO(from yuzisee): We need to track distribution of flops to identify drawing hands.

    RegressionTests::testRegression_006();
    RegressionTests::testRegression_009();
    RegressionTests::testRegression_004();
    RegressionTests::testRegression_018();
    RegressionTests::testRegression_017();
    RegressionTests::testRegression_011();
    RegressionTests::testRegression_013a();
    RegressionTests::testRegression_014a();
    RegressionTests::testRegression_012();


    RegressionTests::testRegression_005();
    RegressionTests::testRegression_019();
}

// HOLDEMDB_PATH=/Users/joseph/pokeroo-run/lib/holdemdb /Users/joseph/Documents/pokeroo/holdem
// /Users/joseph/Documents/pokeroo/holdem/unittests
int main(int argc, const char * argv[])
{
    print_lineseparator(" * * * Begin unittests/main.cpp");

    std::cout << "::group::Running... unit tests" << std::endl;

    all_unit_tests();

    std::cout << "::endgroup::" << std::endl;

    print_lineseparator(" ↑↑↑ UNIT TESTS PASS, regressiontests next ↓↓↓");
    std::cout << "::group::Running... logreplay tests" << std::endl;
    // Regression tests

    all_regression_tests();

    std::cout << "::endgroup::" << std::endl;

    print_lineseparator(" ↑↑↑ regressiontests PASS, Hybrid tests next ↓↓↓");
    std::cout << "::group::Running... hybrid tests" << std::endl;

    // Regression/Unit hybrid

    RegressionTests::testHybrid_008c();
    RegressionTests::testHybrid_008();
    RegressionTests::testHybrid_002();
    RegressionTests::testHybrid_drisk_handsIn();
    std::cout << "::endgroup::" << std::endl;

    print_lineseparator(" ✓ ALL TESTS PASS ");
}
