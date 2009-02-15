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



#include "debug_flags.h"


#ifndef NO_LOG_FILES

#define DEBUGSAVEGAME "savegame"
#define DEBUGSAVEGAME_ALL "saves"
#define DEBUGHOLECARDS "holecards.txt"
#endif

#include "engine.h"
#include "arenaSave.h"
#include "blinds.h"
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

	friend class HoldemArena;

	private:
		PlayerStrategy* myStrat;
		string myName;


		float64 allIn;
		float64 myMoney;
		float64 handBetTotal; //Sum of bets made during COMPLETED betting rounds
		float64 myBetSize; //Within a betting round, the current bet on the table
		float64 lastBetSize; //Within a betting round, the bet before myBetSize



		Player( float64 money, const std::string name, PlayerStrategy* strat, float64 init_play)
		: myStrat(strat),  allIn(init_play), myMoney(money)
		 , handBetTotal(0), myBetSize(0), lastBetSize(init_play)
		{
			myName = name;
		}

	public:

		bool IsBot() const { return (myStrat != 0); }

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
	private:

		CommunityPlus myDealtHand;
		Player* me;
		HoldemArena* game;
    protected:

        int8 myPositionIndex;


	public:


        const CommunityPlus& ViewDealtHand() const { return myDealtHand; }
        virtual void StoreDealtHand(const CommunityPlus & o){ myDealtHand.SetUnique(o); }
        virtual void ClearDealtHand(){ myDealtHand.SetEmpty(); }

		const Player& ViewPlayer() const {return *me;}
		const HoldemArena& ViewTable() const {return *game; }

		PlayerStrategy() : me(0), game(0), myPositionIndex(0) {}
		virtual ~PlayerStrategy(){};

        virtual void Link(PlayerStrategy * o)
        {
            me = o->me;
            game = o->game;
            myPositionIndex = o->myPositionIndex;

			StoreDealtHand(o->ViewDealtHand());
        }

        virtual void Link(Player * const p, HoldemArena * const g, const int8 & i)
        {
            me = p;
            game = g;
            myPositionIndex = i;
        }

	virtual void SeeCommunity(const Hand&, const int8) = 0;


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
{//NO ASSIGNMENT OPERATOR
	private:
		int8 myPlayerIndex;
		float64 bet;
		float64 callAmount;
		bool bCheckBlind;
		bool bAllIn;
	public:
		//See also blinds.h: BlindValues::playersInBlind
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

		playernumber_t curDealer;

		void incrIndex();
		playernumber_t curIndex;
		playernumber_t nextNewPlayer;

        playernumber_t AddPlayer(const char* const id, float64 money, PlayerStrategy* newStrat);

		float64 & PlayerBet(Player& target){ return target.myBetSize; }
		float64 & PlayerLastBet(Player& target){ return target.lastBetSize; }
		float64 & PlayerAllIn(Player& target){ return target.allIn; }
		float64 & PlayerHandBetTotal(Player& target){ return target.handBetTotal; }
		float64 & PlayerMoney(Player& target){ return target.myMoney; }


    protected:

        std::ostream& gamelog;

		float64 randRem;

        int8 cardsInCommunity;
        uint8 bettingRoundsRemaining;
		const bool bVerbose;
		const bool bSpectate;


		playernumber_t livePlayers;
		playernumber_t roundPlayers;
		playernumber_t playersInHand;
		playernumber_t playersAllIn;

        playernumber_t curHighBlind;

        BlindValues myBlinds;
		float64 smallestChip;
		float64 allChips;

		void addBets(float64);
		float64 lastRaise;
		float64 highBet;

        float64 myPot; //incl. betSum
        float64 myFoldedPot;
		float64 myBetSum; //Just the current round.
		float64 prevRoundFoldedPot;
		float64 prevRoundPot;
		float64 forcedBetSum; //Folded bets this round and bets that have been made blind before the player has had a chance to make another bet
		float64 blindOnlySum; //Bets that have been made blind before the player has had a chance to make another bet
        Player *(p[SEATS_AT_TABLE]);


    #ifdef GLOBAL_AICACHE_SPEEDUP
        mutable CommunityCallStats *communityBuffer;
    #endif

        void PrintPositions(std::ostream& o);
		void broadcastHand(const Hand&,const int8 broadcaster);
		void broadcastCurrentMove(const playernumber_t& playerID, const float64& theBet, const float64 theIncrBet
                                , const float64& toCall, const int8 bBlind, const bool& isBlindCheck, const bool& isAllIn);
		void defineSidePotsFor(Player&, const playernumber_t);
		void resolveActions(Player&);

		void compareAllHands(const CommunityPlus & , const int8, vector<ShowdownRep>& );
		double* organizeWinnings(int8&, vector<ShowdownRep>&, vector<ShowdownRep>&);

        DeckLocation ExternalQueryCard(std::istream& s);



		void prepareRound(const CommunityPlus &, const int8);



	public:


//===========================
//   Marshalling Functions
//===========================

        char pTypes[SEATS_AT_TABLE];

#ifdef DEBUGSAVEGAME
        void SerializeRoundStart(std::ostream & fileSaveState);
        void UnserializeRoundStart(std::istream & fileSaveState);
#endif


//=================
//   Bookkeeping
//=================
        handnum_t handnum;

    #ifdef GLOBAL_AICACHE_SPEEDUP
        void CachedQueryOffense(CallCumulation& q, const CommunityPlus& community, const CommunityPlus& withCommunity) const;
    #endif

		void incrIndex(playernumber_t&) const;

        static void ToString(const HoldemAction& e, std::ostream& o);
        static void FileNumberString(handnum_t value, char * str);

		static const float64 FOLDED;
		static const float64 INVALID;



//============================
//   Constructor/Destructor
//============================


		HoldemArena(float64 smallestChipAmount, std::ostream& targetout, bool illustrate, bool spectate)
		: curIndex(-1),  nextNewPlayer(0)
        ,gamelog(targetout)
        ,bVerbose(illustrate),bSpectate(spectate)
        ,livePlayers(0),curHighBlind(-1),smallestChip(smallestChipAmount),allChips(0)
		,lastRaise(0),highBet(0), myPot(0), myFoldedPot(0), myBetSum(0), prevRoundFoldedPot(0), prevRoundPot(0),forcedBetSum(0), blindOnlySum(0)
		#ifdef GLOBAL_AICACHE_SPEEDUP
		,communityBuffer(0)
        #endif
		{
		    //p = new Player * [SEATS_AT_TABLE];
		    for(playernumber_t n=0;n<SEATS_AT_TABLE;++n){
                pTypes[n] = '\0';
		        p[n] = 0;
            }

        }

		~HoldemArena();


//============================
//   Flow Control Functions
//============================

		void AssertInitialState();
		void LoadBeginInitialState();
		void BeginInitialState();
		Player * FinalizeReportWinner();

		// HoldemArenaBetting events and PlayShowdown will both update the deterministic-random-seed assistant
        void ResetDRseed(); //You may reset the seed here (recommended at the beginning of each hand, before the first HoldemArenaBetting event
        float64 GetDRseed(); //You may get a seed at any time, but it is best to do so after PlayShowdown or when no more HoldemArenaBetting events will take place

		void BeginNewHands(const BlindValues & roundBlindValues, const bool & bNewBlindValues, playernumber_t newDealer = -1);
        void DealAllHands(SerializeRandomDeck * tableDealer, std::ofstream & holecardsData);

		void PrepBettingRound(bool bFirstBettingRound, uint8 otherBettingRounds);
        //returns the first person to reveal cards (-1 if all fold)
        playernumber_t PlayRound(const CommunityPlus &, const int8);
		playernumber_t PlayRound_BeginHand();
		playernumber_t PlayRound_Flop(const CommunityPlus & flop);
		playernumber_t PlayRound_Turn(const CommunityPlus & flop, const DeckLocation & turn);
		playernumber_t PlayRound_River(const CommunityPlus & flop, const DeckLocation & turn, const DeckLocation & river);

		void PlayShowdown(const CommunityPlus &,const playernumber_t );

		void RequestCards(SerializeRandomDeck *, uint8, CommunityPlus &, const char * request_str);//, std::ofstream * saveCards);
		DeckLocation RequestCard(SerializeRandomDeck *);//, std::ofstream *);
        void RefreshPlayers();


		void PrepShowdownRound(const CommunityPlus & community);
		void ProcessShowdownResults(vector<ShowdownRep> & winners);

//==============================
//   Initialization Functions
//==============================

        playernumber_t AddHumanOpponent(const char* const id, float64 money);
        playernumber_t AddStrategyBot(const char* const id, float64 money, char botType);

        ///Be very careful with AddPlayerManual
        ///All manual adds must take place prior to AddHumanOpponent or AddStrategyBot calls.
        ///When saving and loading, addPlayerManual players will not be restored
        playernumber_t ManuallyAddPlayer(const char* const id, float64 money, PlayerStrategy* newStrat);
        void FreePlayer(Player* playerToDelete, char botType);
//===================================
//   In-Game Information Accessors
//===================================

		playernumber_t NumberAtRound() const;
        playernumber_t NumberInHand() const;
		playernumber_t NumberAtTable() const;

		playernumber_t GetTotalPlayers() const;

		playernumber_t GetCurPlayer() const;
		playernumber_t GetDealer() const;

		const Player* ViewPlayer(playernumber_t) const;
		bool ShowHoleCards(const Player & withP, const CommunityPlus & dealHandP);
		float64 GetBetDecision(playernumber_t);
		char GetPlayerBotType(playernumber_t) const;

		bool IsAlive(playernumber_t) const;
		bool IsInHand(playernumber_t) const;
		bool HasFolded(playernumber_t) const;
		bool CanStillBet(playernumber_t) const; //This will not include players who have pushed all in
		uint8 RaiseOpportunities(int8,int8) const;
		uint8 FutureRounds() const;

        float64 GetAllChips() const;
        float64 GetFoldedPotSize() const;
        float64 GetUnfoldedPotSize() const;
		float64 GetDeadPotSize() const; //That's pot - betSum;
		float64 GetLivePotSize() const;
		float64 GetRoundPotSize() const; //ThisRound pot size
		float64 GetPrevPotSize() const; //Pot size from previous rounds
   		float64 GetPrevFoldedRetroactive() const;
   		float64 GetRoundBetsTotal() const; //Bets made this round by players still in hand, excludes blind bets
   		float64 GetUnbetBlindsTotal() const; //blindOnlySum
		float64 GetPotSize() const;
		float64 GetBetToCall() const; //since ROUND start
		float64 GetMaxShowdown(const float64 myMoney = -1) const;
		float64 GetMinRaise() const;
		float64 GetChipDenom() const;
		const BlindValues & GetBlindValues() const{ return myBlinds; }




//======================
//   In-Game Mutators
//======================

		bool OverridePlayerMoney(playernumber_t n, float64 m);
}
;

class HoldemArenaEventBase
{//NO ASSIGNMENT OPERATOR
    protected:
    HoldemArena * myTable;
    std::ostream& gamelog;

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

    void defineSidePotsFor(Player& allInP, const int8 id){myTable->defineSidePotsFor(allInP,id);}
    void resolveActions(Player& withP){myTable->resolveActions(withP);}
    void broadcastHand(const Hand& h,const int8 broadcaster){ myTable->broadcastHand(h,broadcaster); }
    void broadcastCurrentMove(const int8& playerID, const float64& theBet, const float64 theIncrBet
                                , const float64& toCall, const int8 bBlind, const bool& isBlindCheck, const bool& isAllIn)
                                {
                                    myTable->broadcastCurrentMove(playerID,theBet,theIncrBet,toCall,bBlind,isBlindCheck,isAllIn);
                                }
    void prepareRound(const CommunityPlus & community, const int8 comSize){ myTable->prepareRound(community, comSize); };


    playernumber_t GetTotalPlayers() const { return myTable->GetTotalPlayers(); }
    playernumber_t GetNumberInHand() const { return myTable->NumberInHand(); }

    public:

		const playernumber_t WhoIsNext(){ return curIndex; }

    HoldemArenaEventBase(HoldemArena * table) : myTable(table)
    , gamelog(myTable->gamelog)
    , curHighBlind(table->curHighBlind)
    , highBet(table->highBet), lastRaise(table->lastRaise), forcedBetSum(myTable->forcedBetSum), blindOnlySum(myTable->blindOnlySum)
    , playersInHand(table->playersInHand),playersAllIn(table->playersAllIn)
	, curDealer(table->curDealer) , curIndex(table->curIndex)
    , myPot(table->myPot), myFoldedPot(table->myFoldedPot), prevRoundFoldedPot(table->prevRoundFoldedPot), myBetSum(table->myBetSum), p(table->p)
    , bVerbose(table->bVerbose), randRem(table->randRem)
    {

    }

}
;

class HoldemArenaBetting : public HoldemArenaEventBase
{//NO ASSIGNMENT OPERATOR
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
    HoldemArenaBetting(HoldemArena * table, const CommunityPlus & community, int8 communitySize)
     : HoldemArenaEventBase(table), comSize(communitySize)

	,  playerCalled(-1), bBetState('b')
    {
    	prepareRound(community, comSize);
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

        void RevealHandMain(const ShowdownRep& comp, const CommunityPlus & playerHand);
        void RevealHandAllIns(const ShowdownRep& comp, const CommunityPlus & playerHand);
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
    void RevealHand(const CommunityPlus & playerHand, const CommunityPlus & community);
    ///ShowHand reveals the hand of myTable->p[curIndex]
    void MuckHand();
    ///MuckHand mucks the hand of myTable->p[curIndex]

}
;


#endif

