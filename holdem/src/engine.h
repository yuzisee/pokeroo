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




#ifndef HOLDEM_BaseStrategy
#define HOLDEM_BaseStrategy

#include "holdem2.h"
#include "ai.h"


#define GLOBAL_AICACHE_SPEEDUP

typedef bool SuitsUsedBool[13][4];

class OrderedDeck
{
	protected:

		uint32 dealtHand[4]; //master "dealt or not" list

		int8 firstSuit;
		int8 nextSuit[4];
		int8 prevSuit[4];
	public:
        static const OrderedDeck EMPTY_ODECK;
		void sortSuits();
    //static const int Occurrences(unsigned long*);
    //int lastOccurrences;
		void SetEmpty();//Make sure you Empty all the hands!
		void OmitCard(const DeckLocation&);
		void OmitCards(const Hand&);


    //after redealing it, set dealtSuit and dealtValue to
    //0 and ACELOW. This allows the next card to be independant
        OrderedDeck(const OrderedDeck& other)
        {
            firstSuit = other.firstSuit;

            for( int8 i=0;i<4;++i)
            {
                nextSuit[i] = other.nextSuit[i];
                prevSuit[i] = other.prevSuit[i];
                dealtHand[i] = other.dealtHand[i];
            }
        }

		OrderedDeck()
		{
			firstSuit = 0;

			nextSuit[0] = 1;
			nextSuit[1] = 2;
			nextSuit[2] = 3;
			nextSuit[3] = HoldemConstants::NO_SUIT;
			prevSuit[0] = HoldemConstants::NO_SUIT;
			prevSuit[1] = 0;
			prevSuit[2] = 1;
			prevSuit[3] = 2;

			SetEmpty();
		}

                virtual ~OrderedDeck();

		bool operator==(const OrderedDeck&) const;
}
;


class DealableOrderedDeck : public OrderedDeck
{

	public:
		virtual float64 DealCard(Hand&) = 0;
		void UndealCard(const DeckLocation&);

		const uint32 BaseDealtValue() const
		{
			return HoldemConstants::CARD_ACELOW;
		}
		const int8 BaseDealtSuit() const
		{
			return firstSuit;
		}
		const uint8 BaseDealtRank() const
		{
			return 0;
		}

		DeckLocation dealt;

		void SetIndependant();
		void SetNextSuit();
		void UndealAll();//Make sure you Empty all the hands!

    //after redealing it, set dealtSuit and dealtValue to
    //0 and ACELOW. This allows the next card to be independant
        DealableOrderedDeck(const DealableOrderedDeck& other) : OrderedDeck(other)
        {
			dealt = other.dealt;
        }

		DealableOrderedDeck() : OrderedDeck()
		{
		}

        virtual ~DealableOrderedDeck();

}
;


class DealRemainder : public DealableOrderedDeck
{
	private:

		Hand baseAddend;
		float64 executeIterative();
		int16 moreCards;
    protected:
        //SuitsUsedBool * dealtTables;
	public:
        virtual float64 DealCard(Hand&);
        void sortSuitsStable(const Hand & addend);

		PlayStats (*lastStats);

		void CleanStats();

		float64 AnalyzeComplete();

		DealRemainder(PlayStats* instructions) : DealableOrderedDeck(), moreCards(instructions->moreCards), lastStats(instructions)
		{
		    //dealtTables = new SuitsUsedBool[instructions->moreCards];
			baseAddend.SetEmpty();
		}


		virtual ~DealRemainder();
}
;

#endif
