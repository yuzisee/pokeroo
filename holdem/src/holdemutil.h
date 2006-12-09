/***************************************************************************
 *   Copyright (C) 2006 by Joseph Huang                                    *
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

#ifndef HOLDEM_BaseClasses
#define HOLDEM_BaseClasses

#define DEBUGASSERT

#include "portability.h"

#include <iostream>

using std::endl;
using std::flush;

class Hand;

class HoldemUtil
{

    /*
        //anywhere there are no hanging/off-cards competitive values will be stored

        //possible strengths of FIVE (from seven) CARD hands
        //0		cannot occur without wild
        //1			high card to order
        //			[TIES: Remove lowest two remaining via VALUESET]
        //2-14		there are 13 pairs
        //			[TIES: Remove pair via VALUESET, treat as "high card to order"]
        //15-92		there are 13*3*2 = 78 twopairs
        //			[TIES: Remove pairs via VALUESET, treat as "high card to order"]
        //93-105	there are 13 trips
        //			[TIES: Remove trip via VALUESET, treat as "high card to order"]
        //106		straight
        //			[TIES: Leave ONLY the AND-AND-ANDed even as VALUESET = CARDSET]
        //(107?)	flush (reserved)
        //			[TIES: treat as "high card to order"]
        //108		boat 13*13 valueset choices
        //			[TIES: Leaving the pair AND-extracted VALUESET is enough]
        //109-121	there are 13 quads
        //			[TIES: Remove quad, remove lowest two remaining]
        //122		straight flush (similarily reserved)
        //			[TIES: treat as "straight"]
    */
public:

	static const uint32 PRIMES[14];

	static const uint32 CARDORDER[14];
    static const uint32 VALUEORDER[14];
    static const uint32 INCRORDER[14];
	static const char VALKEY[16];
	static const char SUITKEY[5];

    ///COMMON PROBLEMS:
    ///THIS ORDERS THE CARDS LIKE: 2S 2H 2C 2D 3S 3H ...
    ///it's not 23456789tJQKA.
    const static uint8 CardSuit(const uint8 d)
    {
		return (d & 3); //That's a 11 mask
    }
    const static uint8 CardRank(const uint8 d) //returns 0 for two
    {
		return (d >> 2);
    }
    const static void PrintCard(std::ostream& target, const int8 s, uint32 v)
	{
		int8 vn=0;
		for(int8 val=2;val<=14;++val)
		{
			v >>= 1;
			if ((v & 1) == 1)
			{
				vn = val;
				break;
			}
		}
		target << VALKEY[vn]<< SUITKEY[s] << flush;
	}
    const static int8 ReadCard(std::istream& charSource)
    {
        unsigned char nextChar[2];
        charSource >> nextChar[0];
        charSource >> nextChar[1];

            return      
            ParseCard(nextChar[0],nextChar[1]);

        
    }
    const static int8 ParseCard(char valchar, char suitchar)
    {
        int8 valNum = -1;
        int8 suitNum = -1;
        for( int8 i=2;i<=14;++i )
        {
            if( VALKEY[i] == valchar )
            {
                valNum = i-2;
                break;
            }
        }

        for( int8 i=0;i<4;++i )
        {
            if( SUITKEY[i] == suitchar )
            {
                suitNum = i-2;
                break;
            }
        }
        
#ifdef DEBUGASSERT
        if( valNum == -1 || suitNum == -1 )
        {
            std::cout << "Wrong reverse-PrintCard" << endl;
            exit(1);
//            return -1;
        }
#endif
        return suitNum + valNum*4;
        
    }
    
	const static uint8 cleanz(const uint32);

		template<typename T>
        static T nchoosep(const int32 n, int32 p) //inline
        {
			if( (n-p) < p ) p = n-p;/* OPTIMIZATION INCLUDED LATER */

            T r = 1;
            for(int32 factorial=0;factorial < p;++factorial)
            {
                r*=(n-factorial);
                r/= factorial+1;
            }
            return r;
        }
}
;

class DeckLocation
{
public:
	uint32 Value;   //adjust these to set start values
	int8 Suit;              //When you undeal the last card of any hand,
	uint8 Rank;

	const uint8 GetIndex() const
	{
		return static_cast<uint8>(Suit) + (Rank-1)*4;
	}
	void SetByIndex(int8 n)
	{
		Rank = HoldemUtil::CardRank(n) + 1;
		Suit = HoldemUtil::CardSuit(n);
		Value = HoldemUtil::CARDORDER[Rank];
	}
}
;




class Hand
{

protected:
    uint32 cardset[4];
public:
    static const Hand EMPTY_HAND;

	uint32 SeeCards(const int8 someSuit) const
	{
		return cardset[someSuit];
	}
//    const int Occurrences() const;
    virtual void SetEmpty();
    const virtual bool IsEmpty() const;

    virtual void AddToHand(const DeckLocation& deck)
    {
    	AddToHand(deck.Suit,deck.Rank,deck.Value);
	}
    virtual void RemoveFromHand(const DeckLocation& deck)
    {	RemoveFromHand(deck.Suit,deck.Rank,deck.Value);	}

    virtual void RemoveFromHand(const int8 aSuit,const uint8 aIndex,const uint32 aCard)
    {
        cardset[aSuit] &= ~aCard;
    }
    virtual void AddToHand(const int8 aSuit,const uint8 aIndex,const uint32 aCard)
    {///NO ERROR CHECKING. Use carefully.
#ifdef DEBUGASSERT
		if ( HoldemUtil::CARDORDER[aIndex] != aCard )
		{
 			std::cerr << endl << "DEBUGASSERT:\t Please add cards consistently!"
				<< endl << "\tindex=" << (int)aIndex << "\tcard=" << aCard << endl;
            exit(1);
		}
#endif
        cardset[aSuit] |= aCard;
    }

	virtual void AppendUnique(const Hand&);
	virtual void SetUnique(const Hand&);

	const Hand& operator=(const Hand& h);
	bool operator==(const Hand& h) const;


    Hand()
    {
        SetEmpty();
    }

    Hand(const Hand& o)
    {
        //Empty();
        SetUnique(o);
    }

    virtual ~Hand();
}
;

class HandPlus : public virtual Hand
{
	protected:
		void populateValueset();
    //unsigned long tempcardset[4]; //short?

	public:
	uint32 valueset; //use most significant 6 (5? 3?) bits to store info?
	virtual void DisplayHand(std::ostream&) const;
    virtual void DisplayHandBig(std::ostream&) const;
	const uint32 getValueset() const;


	HandPlus() : Hand(), valueset(0)
	{
		SetEmpty();
	}
	virtual void AppendUnique(const HandPlus&);
	virtual void AppendUnique(const Hand&);
	virtual void SetUnique(const Hand&);
	virtual void SetUnique(const HandPlus&);

    virtual void AddToHand(const DeckLocation& deck)
    {	AddToHand(deck.Suit,deck.Rank,deck.Value);	}
    virtual void RemoveFromHand(const DeckLocation& deck)
    {	RemoveFromHand(deck.Suit,deck.Rank,deck.Value);	}

	virtual void AddToHand(const int8,const uint8,const uint32);
	virtual void RemoveFromHand(const int8,const uint8,const uint32);

	virtual void SetEmpty();
	const virtual bool IsEmpty() const;

}
;


#endif
