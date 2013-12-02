/***************************************************************************
 *   Copyright (C) 2009 by Joseph Huang                                    *
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





#ifndef HOLDEM_ArenaEvents
#define HOLDEM_ArenaEvents


#include "debug_flags.h"
#include "arena.h"

/**
 * A "View" onto HoldemArena that can be passed around.
 * This class contains const references to some fields of HoldemArena and mutable references to others.
 */
class HoldemArenaEventBase
{//NO ASSIGNMENT OPERATOR
    protected:
    HoldemArena * myTable;
    //std::ostream& gamelog;

    playernumber_t & curHighBlind;
    float64 & highBet;
    float64 & lastRaise;
    float64 & forcedBetSum;
    float64 & blindOnlySum;
    playernumber_t & playersInHand;
    playernumber_t & playersAllIn;
    const playernumber_t & curDealer;
	playernumber_t & curIndex;
    float64 & myPot;
    float64 & myFoldedPot;
    float64 & prevRoundFoldedPot;
    float64 & myBetSum;
    Player* ((&p)[SEATS_AT_TABLE]);
    const bool & bVerbose;
    float64 & randRem;


    void incrIndex(){ myTable->incrIndex(); }
    void incrIndex(int8 & c){ myTable->incrIndex(c); }

    void addBets(float64 betAmount){ myTable->addBets(betAmount); }
    bool IsInHand(int8 ind) { return myTable->IsInHand(ind); }

    float64 & PlayerBet(Player& target){ return myTable->PlayerBet(target); }
    float64 & PlayerMoney(Player& target){ return myTable->PlayerMoney(target); }
    float64 & PlayerAllIn(Player& target){ return myTable->PlayerAllIn(target); }
    float64 & PlayerLastBet(Player& target){ return myTable->PlayerLastBet(target); }
    float64 & PlayerHandBetTotal(Player& target){ return myTable->PlayerHandBetTotal(target); }
    float64 & PlayerForcedBetTotal(Player& target){ return myTable->PlayerForcedBetTotal(target); }

    void defineSidePotsFor(Player& allInP, const int8 id){myTable->defineSidePotsFor(allInP,id);}
    void resolveActions(Player& withP){myTable->resolveActions(withP);}
    void broadcastHand(const Hand& h,const int8 broadcaster){ myTable->broadcastHand(h,broadcaster); }
    void broadcastCurrentMove(const HoldemAction &action)
                                {
                                    myTable->broadcastCurrentMove(action);
                                }
    void prepareRound(const CommunityPlus & community, const int8 comSize){ myTable->prepareRound(community, comSize); };
    void foldActionOccurred() { myTable->foldActionOccurred(); }
    void nonfoldActionOccurred() { myTable->nonfoldActionOccurred(); }

    playernumber_t GetTotalPlayers() const { return myTable->GetTotalPlayers(); }
    playernumber_t GetNumberInHandInclAllIn() const { return myTable->NumberInHandInclAllIn(); }
	playernumber_t GetNumberInHandExclAllIn() const { return myTable->NumberInHandExclAllIn(); }
	float64 GetChipDenom() const { return myTable->GetChipDenom(); }

    public:

        playernumber_t WhoIsNext() const { return curIndex; }

    HoldemArenaEventBase(HoldemArena * table) : myTable(table)
    //, gamelog(myTable->gamelog)
    , curHighBlind(table->curHighBlind)
    , highBet(table->highBet), lastRaise(table->lastRaise), forcedBetSum(myTable->forcedBetSum), blindOnlySum(myTable->blindOnlySum)
	, playersInHand(table->playersInHand.total),playersAllIn(table->playersInHand.allInsOnly)
	, curDealer(table->curDealer) , curIndex(table->curIndex)
    , myPot(table->myPot), myFoldedPot(table->myFoldedPot), prevRoundFoldedPot(table->prevRoundFoldedPot), myBetSum(table->myBetSum), p(table->p)
    ,
    bVerbose(table->bVerbose)
    ,
    randRem(table->randRem)
    {

    }

}
;

// This struct is populated if a bet is made that doesn't obey the minRaise rules.
struct MinRaiseError {
    float64 minRaiseBy;
    float64 minRaiseTo;
    float64 error;
    float64 result; // your new raiseTo or callTo because we had to choose one.

    MinRaiseError() :
    minRaiseBy(std::numeric_limits<float64>::signaling_NaN())
    ,
    minRaiseTo(std::numeric_limits<float64>::signaling_NaN())
    ,
    error(std::numeric_limits<float64>::signaling_NaN())
    ,
    result(std::numeric_limits<float64>::signaling_NaN())
    {}
};

class HoldemArenaBetting : public HoldemArenaEventBase
{//NO ASSIGNMENT OPERATOR
    private:
    const int8 comSize;

	playernumber_t* allInsNow;
    playernumber_t allInsNowCount;
	void allInsReset();
	void allInsAppend(const playernumber_t&);

			
	bool IsHeadsUp() const;


    protected:
	bool bHighBetCalled;
    playernumber_t bBlinds;
    playernumber_t highestBetter;



    void startBettingRound(std::ostream * const spectateLog);
    void finishBettingRound();
    void incrPlayerNumber(Player& currentPlayer);

	bool readyToFinish() const;


    public:
    HoldemArenaBetting(HoldemArena * table, const CommunityPlus & community, int8 communitySize, std::ostream * const spectateLog)
     : HoldemArenaEventBase(table), comSize(communitySize)
    ,
    allInsNow(0)

	,  playerCalled(-1), bBetState('b')
    {
    	prepareRound(community, comSize);
        startBettingRound(spectateLog);
    }

    ~HoldemArenaBetting();

    ///playerCalled is used when there is a showdown that needs to take place.
    ///playerCalled will be set to the first player to make the highest bet.
    ///This is the also the player who must reveal his/her cards first
    int8 playerCalled;
    char bBetState;

    ///MakeBet will set bBetState to one of the following characters:
    ///'b' if there is still more betting
    ///'F' if everybody folded except for one player
    ///'C' if everybody remaining has called

    HoldemAction MakeBet(float64 betSize, struct MinRaiseError *out);
    ///MakeBet makes a bet for myTable->p[curIndex]

    playernumber_t getHighestBetter() const { return highestBetter; }
}
;

struct Reveal {
    playernumber_t playerIdx;
    std::string outcome; // Will be blank if we mucked and didn't reveal

    Reveal(playernumber_t idx, const std::string & text) : playerIdx(idx), outcome(text) {}
};

class HoldemArenaShowdown : public HoldemArenaEventBase
{//NO ASSIGNMENT OPERATOR
    private:
        const int8 called;
    protected:
    	ShowdownRep best;
        vector<ShowdownRep> allInRevealOrder;
        vector<ShowdownRep>::const_iterator nextReveal;

        void startShowdown();
        void startAllIns();
        void finishShowdown();
        void ShowdownHand(const ShowdownRep& comp);

        struct Reveal RevealHandMain(const ShowdownRep& comp, const CommunityPlus & playerHand, std::ostream &gamelog);
        void RevealHandAllIns(const ShowdownRep& comp, const CommunityPlus & playerHand, std::ostream &gamelog);
    public:

    vector<ShowdownRep> winners;

    HoldemArenaShowdown(HoldemArena * table, const int8 firstPlayer)
     : HoldemArenaEventBase(table), called(firstPlayer), best(-1), bRoundState('w')
     {
         startShowdown();
     }

    char bRoundState;

	//HoldemArenaShowdown will populate winners with a list of winning hands.
	//Generally, this vector will just contain one item. When there is a split or
	//side pots, you have have more players here.
	//This list is ordered and is used by ProcessShowdownResults()

    ///ShowHand and MuckHand will set bRoundState to one of the following characters:
    ///'w' if there are still people left to reveal/muck their hands "IN TURN"
    ///'a' if only all-in hands are to be revealed now
    ///'!' if winners have been determined and the showdown is complete
    struct Reveal RevealHand(const CommunityPlus & playerHand, const CommunityPlus & community, std::ostream &gamelog);
    ///ShowHand reveals the hand of myTable->p[curIndex]
    void MuckHand(std::ostream &gamelog);
    ///MuckHand mucks the hand of myTable->p[curIndex]

}
;


#endif // HOLDEM_ArenaEvents


