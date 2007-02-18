/***************************************************************************
 *   Copyright (C) 2005 by Joseph Huang                                    *
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



#ifndef HOLDEM_Arena
#define HOLDEM_Arena


//#define DEBUGSPECIFIC 1
#define GRAPHMONEY "chipcount.csv"
#define REPRODUCIBLE
///COMMENT OUT DEBUGSPECIFIC IF YOU WISH TO GRAPHMONEY

//#define DEBUGBETMODEL
#define DEBUGSAVEGAME "savegame"
#define DEBUGSAVEGAME_ALL "saves"
//#define DEBUGSAVE_EXTRATOKEN 32
#define DEBUGHOLECARDS "holecards.txt"


#include "engine.h"
#include "arenaSave.h"
#include "blinds.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

#ifdef GLOBAL_AICACHE_SPEEDUP
#include "aiCache.h"
//class CommunityCallStats;
#endif


using std::string;


class HoldemAction;
class HoldemArena;
class PlayerStrategy;


class Player
{
#ifdef DEBUGBETMODEL
    friend class DebugArena;
#endif
	friend class HoldemArena;

	private:
		static const int16 NAMECHARS = 40;
		PlayerStrategy* myStrat;
		string myName;


		float64 allIn;
		float64 myMoney;
		float64 handBetTotal; //Sum of bets made during COMPLETED betting rounds
		float64 myBetSize; //Within a betting round, the current bet on the table
		float64 lastBetSize; //Within a betting round, the bet before myBetSize
		bool bSync;



		Player( float64 money, const std::string name, PlayerStrategy* strat, float64 init_play, bool syncHuman = false)
		: myStrat(strat),  allIn(init_play), myMoney(money),
		 handBetTotal(0), myBetSize(0), lastBetSize(init_play), bSync(syncHuman)
		{
			myName = name;
			myHand.SetEmpty();
		}

	protected:

		CommunityPlus myHand;
		const Hand& GetHand() const { return myHand; }

	public:

		const std::string & GetIdent() const
		{	return myName;	}

		float64 GetMoney() const
		{	return myMoney;	}

		float64 GetContribution() const
		{	return handBetTotal;	}

		//BetSize is since ROUND start, active players only
		float64 GetBetSize() const
		{	return myBetSize;	}

		float64 GetLastBet() const
		{	return lastBetSize;	}

}
;




class PlayerStrategy
{
	friend class HoldemArena;
	private:

		Hand* myHand;
		Player* me;
		HoldemArena* game;
    protected:

        int8 myPositionIndex;

	public:

		const Hand& ViewHand() const { return *myHand; }
		const Player& ViewPlayer() const {return *me;}
		const HoldemArena& ViewTable() const {return *game; }

		PlayerStrategy() : me(0), game(0), myPositionIndex(0) {}
		virtual ~PlayerStrategy(){};

		virtual void SeeCommunity(const Hand&, const int8) = 0;
				/*
		double myBetsIn; //(fraction)
		double p; //dead money in pot (fraction)
		double BetToCall; //(fraction)
				*/
		virtual float64 MakeBet() = 0;
			// 0 for check/fold
			// -1 will definately fold
			// this is the bet SINCE ROUND

		virtual void SeeOppHand(const int8, const Hand&) = 0;

		virtual void SeeAction(const HoldemAction&) = 0;

		virtual void FinishHand() = 0;
}
;


class HoldemAction
{
	private:
		int8 myPlayerIndex;
		float64 bet;
		float64 callAmount;
		bool bCheckBlind;
		bool bAllIn;
	public:
        static const int8 SMALLBLIND = 1;
        static const int8 BIGBLIND = 2;

        const float64 newPotSize;
        const float64 chipsLeft;
        const float64 incrBet;
        const int8 bBlind;

		HoldemAction(const float64 newpot, const float64 incr, const float64 newchips, const int8 i, const float64 b, const float64 c, const int8 blind=0, const bool checked = false, const bool allin = false)
	: myPlayerIndex(i), bet(b), callAmount(c), bCheckBlind(checked), bAllIn(allin), newPotSize(newpot), chipsLeft(newchips), incrBet(incr), bBlind(blind) {} ;

		int8 GetPlayerID() const {return myPlayerIndex;}
		float64 GetAmount() const {return bet;}
		float64 GetRaiseBy() const
		{
			if ( IsRaise() )
			{
				return bet - callAmount;
			}
			else
			{
				return 0;
			}
		}

		bool IsFold() const {return bet < callAmount && !bAllIn;}
		bool IsCheck() const {return (bet == 0) || (bCheckBlind && bet == callAmount);}
		bool IsCall() const {return (bet == callAmount || (bet < callAmount && bAllIn)) && callAmount > 0;}
		bool IsRaise() const {return bet > callAmount && callAmount > 0;}
        bool IsAllIn() const {return bAllIn;}

}
;


class HoldemArena
{
    ///These friend classes are used to handle game loops in a trigger/event based model
    friend class HoldemArenaEventBase;
	private:

		static const float64 BASE_CHIP_COUNT;

		int8 curDealer;

		void incrIndex();
		int8 curIndex;
		int8 nextNewPlayer;

		float64 & PlayerBet(Player& target){ return target.myBetSize; }
		float64 & PlayerLastBet(Player& target){ return target.lastBetSize; }
		float64 & PlayerAllIn(Player& target){ return target.allIn; }
		float64 & PlayerHandBetTotal(Player& target){ return target.handBetTotal; }
		float64 & PlayerMoney(Player& target){ return target.myMoney; }
		CommunityPlus & PlayerHand(Player& target){ return target.myHand; }

#ifdef DEBUGSAVEGAME
        std::ifstream loadFile;
        bool bLoadGame;
        void saveState();
#endif

#ifdef DEBUGHOLECARDS
        std::ofstream holecardsData;
#endif

protected:

        std::ostream& gamelog;
        #ifdef GRAPHMONEY
            std::ofstream scoreboard;
        #endif
		SerializeRandomDeck dealer;
		float64 randRem;

        int8 cardsInCommunity;
		CommunityPlus community;
		bool bVerbose;
		bool bSpectate;

		int8 livePlayers;
		int8 playersInHand;
		int8 playersAllIn;

		BlindStructure* blinds;
		float64 smallestChip;
		float64 allChips;

		void addBets(float64);
		float64 lastRaise;
		float64 highBet;

        float64 myPot; //incl. betSum
        float64 myFoldedPot;
		float64 myBetSum; //Just the current round.
		float64 prevRoundPot;
		float64 forcedBetSum; //Folded bets this round and bets that have been made blind before the player has had a chance to make another bet
		float64 blindOnlySum; //Bets that have been made blind before the player has had a chance to make another bet
        vector<Player*> p;


    #ifdef GLOBAL_AICACHE_SPEEDUP
        mutable CommunityCallStats *communityBuffer;
    #endif

        void PrintPositions(std::ostream& o);
		void broadcastHand(const Hand&,const int8 broadcaster);
		void broadcastCurrentMove(const int8& playerID, const float64& theBet, const float64 theIncrBet
                                , const float64& toCall, const int8 bBlind, const bool& isBlindCheck, const bool& isAllIn);
            void RefreshPlayers();
        void PlayGame();
			void defineSidePotsFor(Player&, const int8);
			void resolveActions(Player&);
		void PlayShowdown(const int8);
			void compareAllHands(const int8, vector<ShowdownRep>& );
			double* organizeWinnings(int8&, vector<ShowdownRep>&, vector<ShowdownRep>&);
		void DealHands();
		void prepareRound(const int8);
		int8 PlayRound(const int8);
			//returns the first person to reveal cards (-1 if all fold)

	public:

#ifdef DEBUGSAVEGAME
            std::istream* LoadState();
#endif

	#if defined(DEBUGSPECIFIC) || defined(GRAPHMONEY)
        uint32 handnum;
    #endif

#ifdef DEBUGSAVE_EXTRATOKEN
        char * EXTRATOKEN;
#endif
    #ifdef GLOBAL_AICACHE_SPEEDUP
        void CachedQueryOffense(CallCumulation& q, const CommunityPlus& withCommunity) const;
    #endif

		void incrIndex(int8&) const;
		Player* PlayTable();

        static void ToString(const HoldemAction& e, std::ostream& o);

		static const float64 FOLDED;
		static const float64 INVALID;

		HoldemArena(BlindStructure* b, std::ostream& targetout, bool illustrate, bool spectate)
		: curIndex(-1),  nextNewPlayer(0)
#ifdef DEBUGSAVEGAME
        ,bLoadGame(false)
#endif
        ,gamelog(targetout)
        ,bVerbose(illustrate),bSpectate(spectate),livePlayers(0), blinds(b),allChips(0)
		,lastRaise(0),highBet(0), myPot(0), myFoldedPot(0), myBetSum(0), prevRoundPot(0),forcedBetSum(0), blindOnlySum(0)
		#ifdef GLOBAL_AICACHE_SPEEDUP
		,communityBuffer(0)
        #endif
		{
		    smallestChip = b->SmallBlind(); ///This INITIAL small blind should be assumed to be one chip.
        }

		virtual ~HoldemArena();

		virtual int8 AddPlayer(const char*, PlayerStrategy*);
		virtual int8 AddPlayer(const char* id, float64 money, PlayerStrategy* newStrat);

        virtual int8 GetNumberInHand() const;
		virtual int8 GetNumberAtTable() const;
		virtual int8 GetTotalPlayers() const;

		virtual int8 GetCurPlayer() const;
		virtual int8 GetDealer() const;

		virtual const Player* ViewPlayer(int8) const;

		virtual bool IsAlive(int8) const;
		virtual bool IsInHand(int8) const;
		virtual bool HasFolded(int8) const;
		virtual bool CanStillBet(int8) const; //This will not include players who have pushed all in

        virtual float64 GetAllChips() const;
        virtual float64 GetFoldedPotSize() const;
        virtual float64 GetUnfoldedPotSize() const;
		virtual float64 GetDeadPotSize() const; //That's pot - betSum;
		virtual float64 GetLivePotSize() const;
		virtual float64 GetRoundPotSize() const; //ThisRound pot size
		virtual float64 GetPrevPotSize() const; //Pot size from previous rounds
   		virtual float64 GetRoundBetsTotal() const; //Bets made this round by players still in hand, excludes blind bets
   		virtual float64 GetUnbetBlindsTotal() const; //blindOnlySum
		virtual float64 GetPotSize() const;
		virtual float64 GetBetToCall() const; //since ROUND start
		virtual float64 GetMaxShowdown() const;
		virtual float64 GetMinRaise() const;
		virtual float64 GetBigBlind() const;
		virtual float64 GetSmallBlind() const;
		virtual float64 GetChipDenom() const;
}
;

class HoldemArenaEventBase
{
    protected:
    HoldemArena * myTable;
    std::ostream& gamelog;


    float64 & highBet;
    float64 & lastRaise;
    float64 & forcedBetSum;
    float64 & blindOnlySum;
    int8 & playersInHand;
    int8 & playersAllIn;
    int8 & curIndex;
    int8 & curDealer;
    float64 & myPot;
    float64 & myFoldedPot;
    float64 & myBetSum;
    const vector<Player*> &p;
    const bool & bVerbose;
    float64 & randRem;
    const CommunityPlus & community;


    void incrIndex(){ myTable->incrIndex(); }
    void incrIndex(int8 & c){ myTable->incrIndex(c); }

    void addBets(float64 betAmount){ myTable->addBets(betAmount); }
    bool IsInHand(int8 ind) { return myTable->IsInHand(ind); }

    float64 & PlayerBet(Player& target){ return myTable->PlayerBet(target); }
    float64 & PlayerMoney(Player& target){ return myTable->PlayerMoney(target); }
    float64 & PlayerAllIn(Player& target){ return myTable->PlayerAllIn(target); }
    float64 & PlayerLastBet(Player& target){ return myTable->PlayerLastBet(target); }
    float64 & PlayerHandBetTotal(Player& target){ return myTable->PlayerHandBetTotal(target); }
    const CommunityPlus & PlayerHand(Player& target){ return myTable->PlayerHand(target); }

    void defineSidePotsFor(Player& allInP, const int8 id){myTable->defineSidePotsFor(allInP,id);}
    void resolveActions(Player& withP){myTable->resolveActions(withP);}
    void broadcastHand(const Hand& h,const int8 broadcaster){ myTable->broadcastHand(h,broadcaster); }
    void broadcastCurrentMove(const int8& playerID, const float64& theBet, const float64 theIncrBet
                                , const float64& toCall, const int8 bBlind, const bool& isBlindCheck, const bool& isAllIn)
                                {
                                    myTable->broadcastCurrentMove(playerID,theBet,theIncrBet,toCall,bBlind,isBlindCheck,isAllIn);
                                }
    void prepareRound(const int8 comSize){ myTable->prepareRound(comSize); };


    int8 GetTotalPlayers() const { return myTable->GetTotalPlayers(); }
    int8 GetNumberInHand() const { return myTable->GetNumberInHand(); }

    public:

    HoldemArenaEventBase(HoldemArena * table) : myTable(table)
    , gamelog(myTable->gamelog)
    , highBet(table->highBet), lastRaise(table->lastRaise), forcedBetSum(myTable->forcedBetSum), blindOnlySum(myTable->blindOnlySum)
    , playersInHand(table->playersInHand),playersAllIn(table->playersAllIn)
    , curIndex(table->curIndex), curDealer(table->curDealer)
    , myPot(table->myPot), myFoldedPot(table->myFoldedPot), myBetSum(table->myBetSum), p(table->p)
    , bVerbose(table->bVerbose), randRem(table->randRem)
    , community(table->community)
    {

    }

}
;

class HoldemArenaBetting : public HoldemArenaEventBase
{
    private:
    const int8 comSize;

    protected:
    int8 bBlinds;
    int8 highestBetter;
    int8* allInsNow;
    int8 allInsNowCount;



    void startBettingRound();
    void finishBettingRound();
    void incrPlayerNumber(Player& currentPlayer);


    public:
    HoldemArenaBetting(HoldemArena * table, int8 communitySize)
     : HoldemArenaEventBase(table), comSize(communitySize)

	,  playerCalled(-1), bBetState('b')
    {
        startBettingRound();
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

    void MakeBet(float64 betSize);
    ///MakeBet makes a bet for myTable->p[curIndex]
}
;


class HoldemArenaShowdown : public HoldemArenaEventBase
{
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

        void RevealHandMain(const ShowdownRep& comp);
        void RevealHandAllIns(const ShowdownRep& comp);
    public:

    vector<ShowdownRep>& winners;

    HoldemArenaShowdown(HoldemArena * table, const int8 firstPlayer, vector<ShowdownRep>& breakdown_out)
     : HoldemArenaEventBase(table), called(firstPlayer), best(-1) , winners(breakdown_out), bRoundState('w')
     {
         startShowdown();
     }

    char bRoundState;

    ///ShowHand and MuckHand will set bRoundState to one of the following characters:
    ///'w' if there are still people left to reveal/muck their hands "IN TURN"
    ///'a' if only all-in hands are to be revealed now
    ///'!' if winners have been determined and the showdown is complete
    void RevealHand(const ShowdownRep& comp);
    ///ShowHand reveals the hand of myTable->p[curIndex]
    void MuckHand();
    ///MuckHand mucks the hand of myTable->p[curIndex]

}
;


#endif

