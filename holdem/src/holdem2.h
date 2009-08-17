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

#ifndef HOLDEM_Community
#define HOLDEM_Community

// TODO: Refactor hand evaluator as separate module.
//       Incorporate poker-eval from http://pokersource.info/developers/ or RayW from http://pokerai.org/pf3/viewtopic.php?f=3&t=16 (C version is http://pokerai.org/pf3/viewtopic.php?f=3&t=16&st=0&sk=t&sd=a&start=130)

#include "holdemutil.h"
#include <iostream>

using std::endl;


class CommunityPlus : public virtual HandPlus //greater strength wins, then greater valueset
{
private:
    uint32 prestraight;
    int8 bFlushSuit;//In order to cleanlasttwo of a flush, we need to know the number of surplus cards in the flush suit
                    //Therefore, BOTH bFlushSuit AND flushCount need to be accurate at all times
    int8 flushCount[4]; //number of cards in that suit

	uint8 threeOfAKind;




	uint8 bestPair;
	uint8 nextbestPair;


	void preEvalStrength();
    void cleanLastTwo();
public:
    static const CommunityPlus EMPTY_COMPLUS;


	void evaluateStrength();
    char strength;

	virtual void AppendUnique(const Hand&);
	virtual void AppendUnique(const HandPlus&);
	virtual void AppendUnique(const CommunityPlus&);


    virtual void AddToHand(const DeckLocation& deck)
    {	AddToHand(deck.Suit,deck.Rank,deck.Value);	}
    virtual void RemoveFromHand(const DeckLocation& deck)
    {	RemoveFromHand(deck.Suit,deck.Rank,deck.Value);	}

	virtual void AddToHand(const int8,const uint8,const uint32);
	virtual void RemoveFromHand(const int8,const uint8,const uint32);

	virtual void SetUnique(const Hand&);
	virtual void SetUnique(const HandPlus&);
	virtual void SetUnique(const CommunityPlus&);

    virtual void SetEmpty();

    CommunityPlus();

	const int8 CardsInSuit(const int8) const;
	virtual void DisplayHand(std::ostream&) const;
	virtual void DisplayHandText(std::ostream&) const;
    virtual void DisplayHandBig(std::ostream&) const;
    void PrintInterpretHand(std::ostream&) const;
}
;


//This class allows hands to be COMPARABLE at a showdown, that's all...
class ShowdownRep
{
	public:

		ShowdownRep(const playernumber_t playerID) : strength(0), valueset(0), playerIndex(playerID), revtiebreak(0) {}


		ShowdownRep(const CommunityPlus* h, const CommunityPlus* h2, const playernumber_t pIndex)
			: playerIndex(pIndex), revtiebreak(0)
		{
			CommunityPlus utilHand;
            utilHand.SetUnique(*h2);
			//utilHand.AppendUnique(h2);
			utilHand.AppendUnique(*h);
			comp.SetUnique(utilHand);
			comp.evaluateStrength();
			strength = comp.strength;
			valueset = comp.valueset;
		}


		ShowdownRep(const CommunityPlus* h, const playernumber_t pIndex)
			: playerIndex(pIndex), revtiebreak(0)
		{
			comp.SetUnique(*h);
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

    const ShowdownRep & operator=(const ShowdownRep& a)
    {
        comp.SetUnique(a.comp);
        strength = a.strength;
        valueset = a.valueset;
        playerIndex = a.playerIndex;
        revtiebreak = a.revtiebreak;
        return *this;
    }

    bool bIdenticalTo (const ShowdownRep& x) const
    {
        return (
        (strength == x.strength)
        && (valueset == x.valueset)
        && (playerIndex == x.playerIndex)
        && (revtiebreak == x.revtiebreak)
        );
    }

	void DisplayHandBig(std::ostream& o) const { comp.DisplayHandBig(o); }
	void DisplayHandText(std::ostream& o) const { comp.DisplayHandText(o); }
	void DisplayHand(std::ostream& o) const { comp.DisplayHand(o); }

    bool IsMuck()
    {
        return( (strength == 0) && (valueset == 0) && (revtiebreak == 0) && comp.IsEmpty() );
    }

    void SetMuck()
    {
        comp.SetEmpty();
        strength = 0;
        valueset = 0;
        revtiebreak = 0;
    }

    void Reset(const CommunityPlus* h)
    {
        comp.SetUnique(*h);
        comp.evaluateStrength();
        strength = comp.strength;
        valueset = comp.valueset;
    }

	CommunityPlus comp;
	uint8 strength;
	uint32 valueset;
	playernumber_t playerIndex;
	double revtiebreak;
	/*	What is revtiebreak? Well, it's an interesting multiuse variable.
		The first reason is it remembers the amount a player can win. But the
		second reason is that it is used as a comparable when you > or <.
		Why you ask? When we sort a vector of ShowdownReps, we want them to be
		in JUST the right order, such that our organizeWinnings() can process
		them in O(n) time. Have a look at organizeWinnings() for more!
	*/
}
;




#endif
