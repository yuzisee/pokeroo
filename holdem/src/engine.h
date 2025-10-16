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

#include "engine_base.h"
#include "aiInformation.h"

class DealRemainder : public virtual DealableOrderedDeck
{
	private:
        Hand addendSum;
        Hand justDealt;

        template<typename T> static float64 executeComparison(const DealRemainder & refDeck, T (* const lastStats), const float64 fromRuns);
        template<typename T> static float64 executeDealing(DealRemainder & deckState, T (* const lastStats), const int16 moreCards, const float64 fromRuns);
        template<typename T> static float64 executeRecursive(const DealRemainder & refDeck, T (* const lastStats), const int16 moreCards);
        template<typename T> static float64 AnalyzeComplete_impl(DealRemainder * const dealSource, T * const lastStats);

        void UpdateSameSuits();

   		bool addendSameSuit[4][4];

	public:

        /**
         * DealCard()
         *  Parameters:
         *    Hand &h:
         *      The hand we are looking to evaluate (by dealing out all remaining combinations)
         *  Return Value:
         *    The number of possible hands that were dealt.
         *    (This function will call itself recursively and use this returned count to multiply-accumulate up the stack.)
         */
        virtual float64 DealCard(Hand&) override final;


   		void OmitSet(const CommunityPlus& setOne, const CommunityPlus& setTwo);
   		void LockNewAddend(); //And re-sort as needed, stable.
   		///Conditions for sorting:
   		/// 1) cardset's must be descending
   		/// 2) Identical suits must be beside each other
   		/// 3) Afterwards, using addendSameSuit or otherwise, whether two adjacent cardsets are identical or not must be known.

		void CleanStats(); //Releasing memory?

		float64 AnalyzeComplete(CommunityCallStats* instructions);
		float64 AnalyzeComplete(CallStats* instructions);
		float64 AnalyzeComplete(WinStats* instructions);

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

// HOLDEM_BaseStrategy
#endif
