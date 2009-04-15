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

        void AssignSuitsFrom(const OrderedDeck & suitorder);

	public:
        static const OrderedDeck EMPTY_ODECK;
		void sortSuits(); //Stable Mergesort
    //static const int Occurrences(unsigned long*);
    //int lastOccurrences;
		void SetEmpty();//Make sure you Empty all the hands!
		void OmitCard(const DeckLocation&);
		void OmitCards(const Hand&);


    //after redealing it, set dealtSuit and dealtValue to
    //0 and ACELOW. This allows the next card to be independant
        OrderedDeck(const OrderedDeck& other)
        {
            AssignSuitsFrom(other);
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
        virtual float64 DealCard(Hand&)=0;
        virtual void UndealCard(const DeckLocation & deck); //Can be undealt only if not locked into addendSum

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




class DealRemainder : public virtual DealableOrderedDeck
{
	private:
        Hand addendSum;
        Hand justDealt;

		static float64 executeRecursive(const DealRemainder & refDeck, PlayStats (* const lastStats), const int16 moreCards);
		static float64 executeDealing(DealRemainder & refDeck, PlayStats (* const lastStats), const int16 moreCards, const float64 fromRuns);
		static float64 executeComparison(const DealRemainder & refDeck, PlayStats (* const lastStats), const float64 fromRuns);

        void UpdateSameSuits();

    protected:

   		bool addendSameSuit[4][4];
	public:
        virtual float64 DealCard(Hand&);


   		void OmitSet(const CommunityPlus& setOne, const CommunityPlus& setTwo);
   		void LockNewAddend(); //And re-sort as needed, stable.
   		///Conditions for sorting:
   		/// 1) cardset's must be descending
   		/// 2) Identical suits must be beside each other
   		/// 3) Afterwards, using addendSameSuit or otherwise, whether two adjacent cardsets are identical or not must be known.

		void CleanStats(); //Releasing memory?


		float64 AnalyzeComplete(PlayStats* instructions);

        DealRemainder(const DealRemainder & other) : DealableOrderedDeck(other)
        {

            addendSum.SetUnique(other.addendSum);
            justDealt.SetUnique(other.justDealt);

            for( int8 i=0; i<4 ; ++i )
            {
                for( int8 j=0; j<4 ; ++j )
                {
                    addendSameSuit[i][j] = other.addendSameSuit[i][j];
                }
            }


        }

		DealRemainder() : DealableOrderedDeck()
		{//They start true, and once two suits are different, there is no changing it. They stay different forever
            for( int8 i=0; i<4 ; ++i )
            {
                for( int8 j=0; j<4 ; ++j )
                {
                    addendSameSuit[i][j] = true;
                }
            }

            justDealt.SetEmpty();
		}


		virtual ~DealRemainder();
}
;

#endif
