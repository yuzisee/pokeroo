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


class OrderedDeck
{
	protected:

		uint32 dealtHand[4]; //master "dealt or not" list

		int8 firstSuit;
		int8 nextSuit[4];
		int8 prevSuit[4];
	public:
		void sortSuits();
    //static const int Occurrences(unsigned long*);
    //int lastOccurrences;
		void UndealAll();//Make sure you Empty all the hands!
		virtual float64 DealCard(Hand&);
		void UndealCard(const DeckLocation&);
		void OmitCard(const DeckLocation&);
		void OmitCards(const Hand&);

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
			dealt = other.dealt;
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

			UndealAll();
		}

                virtual ~OrderedDeck();

		bool operator==(const OrderedDeck&) const;
}
;

class DealRemainder : public OrderedDeck
{
	private:

		Hand addend; //temporary
//		double executeRecursive(int,unsigned long,double);
		float64 executeIterative();
		int16 moreCards;
	public:

//		bool bRecursive;

		PlayStats* lastStats;

   		void DeOmitCards(const Hand&);
        
		void CleanStats();

		//double deals;

		float64 AnalyzeComplete(PlayStats*);
		float64 Analyze(PlayStats*);
		float64 Analyze(PlayStats*,const int8,const uint8,const uint32);
		float64 Analyze(PlayStats*,const DeckLocation&);

		DealRemainder() : OrderedDeck()
		{
			//bRecursive = false;
			lastStats = 0;
//			addend.Empty();
		}


		virtual ~DealRemainder();
}
;

#endif
