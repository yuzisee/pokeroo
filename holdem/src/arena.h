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





#ifndef HOLDEM_Arena
#define HOLDEM_Arena



#include "debug_flags.h"


#ifndef NO_LOG_FILES

#define DEBUGSAVEGAME "savegame"
#define DEBUGSAVEGAME_ALL "saves"
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
		float64 forcedBetTotal;
		float64 myBetSize; //Within a betting round, the current bet on the table
		float64 lastBetSize; //Within a betting round, the bet before myBetSize



		Player( float64 money, const std::string name, PlayerStrategy* strat, float64 init_play)
		: myStrat(strat), allIn(init_play), myMoney(money)
		 , handBetTotal(0), forcedBetTotal(0), myBetSize(0), lastBetSize(init_play)
		{
			myName = name;
		}

	public:

		bool IsBot() const { return (myStrat != 0); }

		const std::string & GetIdent() const
		{	return myName;	}

		float64 GetMoney() const
		{	return myMoney;	}

		float64 GetOfficialContribution() const
		{	return handBetTotal;	}

		float64 GetVoluntaryContribution() const
		{	return (handBetTotal - forcedBetTotal);	}

		float64 GetInvoluntaryContribution() const
		{	return forcedBetTotal;	}

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

		PlayerStrategy() : me(0), game(0), myPositionIndex(-1) {}
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
{//NO ASSIGNMENT OPERATOR, but POD type
	public:
        const std::string myPlayerName;
		const int8 myPlayerIndex;
		const float64 bet;
		const float64 callAmount;
		const bool bCheckBlind;
		const bool bAllIn;

		//See also blinds.h: BlindValues::playersInBlind
        static const int8 SMALLBLIND = 1;
        static const int8 BIGBLIND = 2;

        const float64 oldPotSize;
        const float64 newPotSize;
        const float64 chipsBefore;
        const float64 chipsLeft;
        const float64 incrBet;
        const int8 bBlind;

    HoldemAction(bool error)
    :
    myPlayerName("")
    ,
    myPlayerIndex(-1)
    ,
    bet(std::numeric_limits<float64>::signaling_NaN())
    ,
    callAmount(std::numeric_limits<float64>::signaling_NaN())
    ,
    bCheckBlind(false)
    ,
    bAllIn(false)
    ,
    oldPotSize(std::numeric_limits<float64>::signaling_NaN())
    ,
    newPotSize(std::numeric_limits<float64>::signaling_NaN())
    ,
    chipsBefore(std::numeric_limits<float64>::signaling_NaN())
    ,
    chipsLeft(std::numeric_limits<float64>::signaling_NaN())
    ,
    incrBet(std::numeric_limits<float64>::signaling_NaN())
    ,
    bBlind(-1)
    {}

    HoldemAction(const std::string &ident, const float64 oldpot, const float64 oldchips, const int8 i, const float64 b, const float64 incr, const float64 c, const int8 blind=0, const bool checked = false, const bool allin = false)
	:
    myPlayerName(ident)
    ,
    myPlayerIndex(i), bet(b), callAmount(c), bCheckBlind(checked), bAllIn(allin)
    ,
    oldPotSize(oldpot)
    ,
    newPotSize(oldpot + incr)
    ,
    chipsBefore(oldchips)
    ,
    chipsLeft(oldchips - b)
    , incrBet(incr), bBlind(blind)
    {}

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

        bool IsPostBlind() const {return (bBlind != 0);}

}
;

typedef
struct playercounts
{
	playernumber_t total;
	playernumber_t allInsOnly;

	void ResetNewHands(playernumber_t numLive) { total = numLive; allInsOnly = 0; }

    //Number of players (e.g. NumberInShowdown)
    playernumber_t inclAllIn() const { return total; }

	//Number of players that could bet (e.g. NumberWithBetting)
	playernumber_t exclAllIn() const { return total - allInsOnly; }
}
playercounts_t
;

struct OrganizedWinnings {
    playernumber_t fIdx;
    std::string fIdent;
    float64 fTotalShowdownPot; // The full pot size of the showdown
    float64 fRemainingPot; // The pot that you have won
    float64 fProfit; // Your profit this hand after claiming fRemainingPot

    OrganizedWinnings() :
    fIdx(-1)
    ,
    fIdent("")
    ,
    fTotalShowdownPot(std::numeric_limits<float64>::signaling_NaN())
    ,
    fRemainingPot(std::numeric_limits<float64>::signaling_NaN())
    ,
    fProfit(std::numeric_limits<float64>::signaling_NaN())
    {}
};

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
		float64 & PlayerForcedBetTotal(Player& target){ return target.forcedBetTotal; }
		float64 & PlayerMoney(Player& target){ return target.myMoney; }

		uint32 jenkins_one_at_a_time_hash(const char *key_null_termninated); //A simple hash for selecting random botTypes.
		char randomBotType(const char *key_null_termninated);


        /**
         * Called at the beginning of each betting round.
         */
        void initRoundPlayers() {
            playersActiveDuringFirstBetOfRound = startRoundPlayers = NumberInHand();
            curFirstNonfold = -1;
        }

    protected:

		float64 randRem;

        int8 cardsInCommunity;
        uint8 bettingRoundsRemaining;
		const bool bVerbose;
		const bool bSpectate;


		playernumber_t livePlayers;

		playercounts_t playersInHand;
		playercounts_t startRoundPlayers;
		playercounts_t playersActiveDuringFirstBetOfRound;
    
        playernumber_t curFirstNonfold; // if -1, all actions (other than posting blinds) so far have been fold. Otherwise, points to the index of the first player not to fold.

        playernumber_t curHighBlind; // if not -1, this is the player who has bet the same amount as the current highest bet, but hasn't actually bet yet (i.e. this is the person in the big blind, who has only been called but hasn't taken their own action yet this round)

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
		void broadcastCurrentMove(const HoldemAction &action);
		void defineSidePotsFor(Player&, const playernumber_t);
		void resolveActions(Player&);


        void compareAllHands(const CommunityPlus & , const int8, vector<ShowdownRep>&, std::ostream &gamelog);
		struct OrganizedWinnings *organizeWinnings(int8&, vector<ShowdownRep>&, vector<ShowdownRep>&);

        static DeckLocation ExternalQueryCard(std::istream& s);

        void foldActionOccurred();
        void nonfoldActionOccurred();

		void prepareRound(const CommunityPlus &, const int8);


	public:


//===========================
//   Marshalling Functions
//===========================

        char pTypes[SEATS_AT_TABLE];

#ifdef DEBUGSAVEGAME
    void SerializeRoundStart(std::ostream & fileSaveState);
    void UnserializeRoundStart(std::istream & fileSaveState, std::string botStorageDir, std::string gameId);
#endif


//=================
//   Bookkeeping
//=================
        handnum_t handnum;

    #ifdef GLOBAL_AICACHE_SPEEDUP
        void CachedQueryOffense(CallCumulation& q, const CommunityPlus& community, const CommunityPlus& withCommunity) const;
    #endif

		void incrIndex(playernumber_t&) const;
		void decrIndex(playernumber_t&) const;

        static void ToString(const HoldemAction& e, std::ostream& o);
        static void FileNumberString(handnum_t value, char * str);

		static const float64 FOLDED;
		static const float64 INVALID;



//============================
//   Constructor/Destructor
//============================


		HoldemArena(float64 smallestChipAmount, bool illustrate, bool spectate)
		: curIndex(-1),  nextNewPlayer(0)

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

//===============
//   Main Loop
//===============
    static void PlayGameInner(HoldemArena & my, GameDeck * tableDealer, std::ostream &gamelog);

//============================
//   Flow Control Functions
//============================

		void AssertInitialState();
		void BeginInitialState(handnum_t game_id = 1);
		Player * FinalizeReportWinner();

		// HoldemArenaBetting events and PlayShowdown will both update the deterministic-random-seed assistant
        void ResetDRseed(); //You may reset the seed here (recommended at the beginning of each hand, before the first HoldemArenaBetting event
        float64 GetDRseed(); //You may get a seed at any time, but it is best to do so after PlayShowdown or when no more HoldemArenaBetting events will take place

        /**
         * Called just before hands are dealt.
         */
		void BeginNewHands(std::ostream &gamelog, const BlindValues & roundBlindValues, const bool & bNewBlindValues, playernumber_t newDealer = -1);
    
        void DealAllHands(GameDeck * tableDealer, std::ostream & holecardsData);

		void PrepBettingRound(bool bFirstBettingRound, uint8 otherBettingRounds);
        //returns the first person to reveal cards (-1 if all fold)
        playernumber_t PlayRound(const CommunityPlus &, const int8, std::ostream &gamelog);
		playernumber_t PlayRound_BeginHand(std::ostream &gamelog);
		playernumber_t PlayRound_Flop(const CommunityPlus & flop, std::ostream &gamelog);
		playernumber_t PlayRound_Turn(const CommunityPlus & flop, const DeckLocation & turn, std::ostream &gamelog);
		playernumber_t PlayRound_River(const CommunityPlus & flop, const DeckLocation & turn, const DeckLocation & river, std::ostream &gamelog);

		void PlayShowdown(const CommunityPlus &,const playernumber_t, std::ostream &gamelog);

		void RequestCards(GameDeck *, uint8, CommunityPlus &, const char * request_str);//, std::ofstream * saveCards);
		static DeckLocation RequestCard(GameDeck *);//, std::ofstream *);
    void RefreshPlayers(std::ostream *spectateLog);


		void PrepShowdownRound(const CommunityPlus & community, std::ostream &gamelog);
		void ProcessShowdownResults(vector<ShowdownRep> & winners, std::ostream &gamelog);

//==============================
//   Initialization Functions
//==============================

        playernumber_t AddHumanOpponent(const char* const id, float64 money);

    /**
     * AddStrategyBot
     *  
     *  Discussion:
     *    Create a bot and sit them at the next available seat.
     *
     *  Parameters:
     *    gameId:
     *      A unique identifier for this game.
     *      We use this to make sure that saved-state/logs/etc. that are relevant for a particular game are not shared/overwritten across multiple games, especially if the same storageDir is used.
     *    storageDir:
     *      This is the path to a direction that this bot can use for storage.
     *      Bots use this directory for logging (so you can debug its reasoning), as well as
     *      to store any persistent state (e.g. MultiBot and GearBot)
     *    id:
     *      This is a username or identifier for this player.
     *    money:
     *      This is the starting chip count of this player.
     *    botType:
     *      A single character describing the bot type to instantiate.
     */
    playernumber_t AddStrategyBot(const std::string gameId, const std::string storageDir, const char* const id, float64 money, char botType);

        ///Be very careful with AddPlayerManual
        ///All manual adds must take place prior to AddHumanOpponent or AddStrategyBot calls.
        ///When saving and loading, addPlayerManual players will not be restored
        playernumber_t ManuallyAddPlayer(const char* const id, float64 money, PlayerStrategy* newStrat);
        void FreePlayer(Player* playerToDelete, char botType);
//===================================
//   In-Game Information Accessors
//===================================

		playernumber_t NumberAtTable() const; //Number of players that have chips (eg. started the gamehand)
		playercounts_t const & NumberStartedRound() const; //Number of players that started the round
		playercounts_t const & NumberAtFirstActionOfRound() const; //Number of players that were present during the first non-blind bet of the round -- Intuition: the first player to bet "sets the tone" by "announcing" the apparent strength of the round based on how early they bet.
		                                            //Number of (established) players in the round after the first non-fold action.
                                                    //In a non-blind round (ie. everything after preflop) NumberStartedRound is the same as NumberAtFirstActionOfRound

		playercounts_t const & NumberInHand() const;

		// ---  Begin deprecated
		playernumber_t NumberStartedRoundInclAllIn() const //Number of players that started the round
		   { return NumberStartedRound().inclAllIn(); }

		playernumber_t NumberStartedRoundExclAllIn() const //Number of players that started the round and could bet
		   { return NumberStartedRound().exclAllIn(); }

		playernumber_t NumberInHandInclAllIn() const //NumberInShowdown
		   { return NumberInHand().inclAllIn(); }

		playernumber_t NumberInHandExclAllIn() const //NumberWithBetting
			{ return NumberInHand().exclAllIn(); }
		// ---  End deprecated

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
		float64 GetMinRaise() const; // Minimum amount to raise BY
		float64 GetChipDenom() const;
		const BlindValues & GetBlindValues() const{ return myBlinds; }

        

//======================
//   In-Game Mutators
//======================

		bool OverridePlayerMoney(playernumber_t n, float64 m);
        void setSmallestChip(float64 smallestChipAmount) {
            this->smallestChip = smallestChipAmount;
        }
}
;


#endif // HOLDEM_Arena

