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

#include "engine.h"
#include "player.h"
#include "randomDeck.h"
#include "blinds.h"
#include <vector>

class Player;
class PlayerStrategy;


//This class allows hands to be COMPARABLE at a showdown, that's all...
class ShowdownRep
{
	public:

		ShowdownRep() : strength(0), valueset(0), playerIndex(-1), revtiebreak(0) {}

		ShowdownRep(const Hand& h, const Hand& h2, const int8 pIndex)
			: playerIndex(pIndex), revtiebreak(0)
		{
			Hand utilHand;

			utilHand.AppendUnique(h2);
			utilHand.AppendUnique(h);
			comp.SetUnique(utilHand);
			comp.evaluateStrength();
			strength = comp.strength;
			valueset = comp.valueset;
		}

		ShowdownRep(const Hand& h, const int8 pIndex)
			: playerIndex(pIndex), revtiebreak(0)
		{
			comp.SetUnique(h);
			comp.evaluateStrength();
			strength = comp.strength;
			valueset = comp.valueset;
		}

	bool operator> (const ShowdownRep& x) const
	{
		if( strength > x.strength )
		{
			return true;
		}
		else if (strength < x.strength)
		{
			return false;
		}
		else
		{//equal strength
			if( valueset == x.valueset )
			{
				//We need an operation that will correctly return
				//valueset == x.valueset is !(valueset > x.valueset)
				//but will also use the tiebreak if necessary.
				//Notice the tiebreak is compared in the OPPOSITE direction
				return revtiebreak < x.revtiebreak;
			}
			return (valueset > x.valueset);
		}
	}
	bool operator< (const ShowdownRep& x) const
	{
		if( strength < x.strength )
		{
			return true;
		}
		else if (strength > x.strength)
		{
			return false;
		}
		else
		{//equal strength
			if( valueset == x.valueset )
			{
				return revtiebreak > x.revtiebreak;
			}
			return (valueset < x.valueset);
		}
	}
	bool operator== (const ShowdownRep& x) const
	{
		return ((strength == x.strength) && (valueset == x.valueset));
	}


	void DisplayHandBig() { comp.DisplayHandBig(); }
	void DisplayHand() { comp.DisplayHand(); }

	CommunityPlus comp;
	uint8 strength;
	uint32 valueset;
	int8 playerIndex;
	double revtiebreak;
	/*	What is revtiebreak? Well, it's an interesting multiuse variable.
		The first reason is it remembers the amount a player can win. But the
		second reason is that it is used as a comparable when you > or <.
		Why you ask? When we sort a vector of ShowdownReps, we want them to be
		in JUST the right order, such that our organizeWinnings() can process
		them in O(n) time. Have a look!
	*/
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

		HoldemAction(const int8 i, const float64 b, const float64 c, const bool checked = false, const bool allin = false)
	: myPlayerIndex(i), bet(b), callAmount(c), bCheckBlind(checked), bAllIn(allin) {} ;

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
		bool IsCheck() const {return (bet == 0) || bCheckBlind;}
		bool IsCall() const {return (bet == callAmount || bAllIn) && callAmount > 0;}
		bool IsRaise() const {return bet > callAmount && callAmount > 0;}
}
;


class HoldemArena
{
	private:
		static const float64 BASE_CHIP_COUNT;

		int8 curDealer;

		void incrIndex();
		int8 curIndex;
		int8 nextNewPlayer;
	protected:


		RandomDeck dealer;
		float64 randRem;

		Hand community;
		bool bVerbose;

		int8 livePlayers;
		int8 playersInHand;
		int8 playersAllIn;

		BlindStructure* blinds;

		void addBets(float64);
		float64 highBet;
		float64 myPot; //incl. betSum
		float64 myBetSum; //Just the current round.
		vector<Player*> p;

		void broadcastHand(const Hand&);
		void broadcastCurrentMove(const int8&, const float64&,
									const float64&, const bool&, const bool&);
		void PlayHand();
			void defineSidePotsFor(Player&, const int8);
			void resolveActions(Player&);
		void PlayShowdown(const int8);
			void compareAllHands(const int8, vector<ShowdownRep>& );
			double* organizeWinnings(int8&, vector<ShowdownRep>&, vector<ShowdownRep>&);
		void DealHand();
		void prepareRound(const int8);
		int8 PlayRound(const int8);
			//returns the first person to reveal cards (-1 if all fold)

	public:

		void incrIndex(int8&) const;
		void PlayGame();

		static const float64 FOLDED;
		static const float64 INVALID;

		HoldemArena(BlindStructure* b, bool illustrate)
		: curIndex(-1),  nextNewPlayer(0),
		bVerbose(illustrate),livePlayers(0), blinds(b),
		highBet(0), myPot(0), myBetSum(0)

		{}

		~HoldemArena();

		int8 AddPlayer(const char*, PlayerStrategy*);
		int8 AddPlayer(const char*, float64, PlayerStrategy*);


		int8 GetNumberAtTable() const;
		int8 GetTotalPlayers() const;

		int8 GetCurPlayer() const;

		const Player* ViewPlayer(int8) const;

		bool IsAlive(int8) const;
		bool IsInHand(int8) const;
		bool HasFolded(int8) const;


		float64 GetDeadPotSize() const; //That's pot - betSum;
		float64 GetRoundPotSize() const; //ThisRound pot size
		float64 GetPotSize() const;
		float64 GetBetToCall() const; //since ROUND start
		float64 GetMaxShowdown() const;
		float64 GetMinRaise() const;
		float64 GetBigBlind() const;
		float64 GetSmallBlind() const;
}
;



#endif

