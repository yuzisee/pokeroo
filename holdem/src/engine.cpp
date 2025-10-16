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

#include "engine.h"
#include "holdem2.h"
#include "ai.h"

// TODO(from joseph): Optimize data layout for cache utilization
// return (addendSameSuit_bits >> (suitNumA * 4 + suitNumB)) & 1;
#define isAddendSameSuit(suitA, suitB) ( this->addendSameSuit[(suitA)][(suitB)] )

float64 DealRemainder::DealCard(Hand& h)
{
    const int8 & qprevSuit = deck_impl.prevSuit[dealt.Suit];

	++(dealt.Rank);
	dealt.Value <<= 1;


	if(dealt.Suit == HoldemConstants::NO_SUIT)
	{//no more cards. That's it for this configuration entirely.

		SetIndependant();
		return 0;
	}
	else if (HoldemConstants::CARD_MISC == dealt.Value)
	{//time for next suit

		SetNextSuit();
		return DealCard(h);

	}
	else if ( (deck_impl.dealtHand[dealt.Suit] & dealt.Value) != 0 )
	{//card already dealt/omitted, i.e. needs to be skipped

		return DealCard(h);
	}
	else if(deck_impl.prevSuit[dealt.Suit] != HoldemConstants::NO_SUIT) //unless we are in the first suit, we have to check for certain cases
	{
        ///What we're looking for is...
        ///    (Action) Reason...
        /// ------------------------
        /// 1. (Skip) Dealing into already identical suits: bPastMatchesOld + suits are the same (transient) in the hand being dealt to
        /// 2. (occBase = 1) Even though bMatchesOld? If for example, hHere and hBack are different, but it has coincidentally
        ///                  Imagine 73 of Spades, and just a 7 of Hearts. You could possibly deal 3 of Hearts into h, then bMatchesOld!
        ///                  So, when you deal the next card, and it doesn't violate the "greater first" rule, but because hBack!=hHere it also doesn't skip the suit,

        /// 3. (matchesnew)  Imagine having 2 Spades, and then dealing a 2 of Hearts. That has ×3, but then what if the next card is 2 of Clubs, without LockNewAddend?


		const uint32 hHere=h.SeeCards(dealt.Suit);
		const uint32 hBack=h.SeeCards(qprevSuit);

		// Remember, `dealtHand` is everything that has been dealt previously.
		// `h.SeeCards(…)` is what's currently in the hand

        const bool bPreviouslyIdentical = isAddendSameSuit(dealt.Suit, qprevSuit);
   if (bPreviouslyIdentical) { //if eligible for "greater first" rule
		if(
             (hHere | dealt.Value) > hBack //would violate "greater first" rule
           )
        {
            //To optimize redundant paths, we keep additions sorted in previously identical suits
			SetNextSuit();
			return DealCard(h);
		}


		// TODO(from joseph): Since https://github.com/yuzisee/pokeroo/commit/b6105aed2141aac9164861aa01102ad2dc750cf2 it seems this condition is no longer possible????
		//                    If `hBack == hHere` we would have essentially tested for `(hHere | dealt.Value) > hHere` above, already.
		//                    But that means `hHere` CONTAINS `dealt.Value` and we are attempting to deal a card that the hand already has!!!
		// Instead, we should raise an assertion if this happens: "Hand h already contains the card we are trying to deal"
        if (hBack==hHere) //bMatchesOld here implies bPreviouslyIdentical, if also hBack==hHere
		{
            //Essentially, we need to avoid adding to this suit, since it is the same as the last suit, which would already
            //have been counted for double!

			SetNextSuit();
			return DealCard(h);
		}
   } // end if bPreviouslyIdentical
	}

	//successful!
	uint8 occBase = 0;

	uint32 baseInto = deck_impl.dealtHand[dealt.Suit]; ///This is prior to {dealtHand[dealt.Suit] |= dealt.Value;}

		for( int8 i=dealt.Suit;i!=HoldemConstants::NO_SUIT;i = deck_impl.nextSuit[i])
		{//Although we could check all four here, it is assumed that you would only deal into the first of identical suits, plus it may help sort out the 2/2/2 corner case

			if ((deck_impl.dealtHand[i] == baseInto) && (isAddendSameSuit(dealt.Suit, i)))
			{
				++occBase;
			}
			else
				break;
		}


	deck_impl.dealtHand[dealt.Suit] |= dealt.Value;


	h.AddToHand(dealt);

	uint32 dealtTo = deck_impl.dealtHand[dealt.Suit];
	uint32 addedTo = h.SeeCards(dealt.Suit);


	uint8 matchesNew = 0; //matchesNew reflects how many new duplicate suits formed.

	for(int8 i=0;i<4;++i)
	{
		if (deck_impl.dealtHand[i] == dealtTo &&
				  h.SeeCards(i) == addedTo &&
				        isAddendSameSuit(dealt.Suit, i) )  ++matchesNew;
	}

	return static_cast<float64>(occBase)/static_cast<float64>(matchesNew);
} // end DealCard


DealRemainder::~DealRemainder()
{
	CleanStats();
}


void DealRemainder::CleanStats()
{
}

//setOne may be empty, setTwo must contain cards that are a superset of setOne
void DealRemainder::OmitSet(const CommunityPlus& setOne, const CommunityPlus& setTwo)
{//Note: Sorting is done by OmitCards

    if( setOne.hand_logic.IsEmpty() )
    {//This is used especially by aiInformation
        addendSum.SetUnique(setTwo.hand_logic.hand_impl);
        deck_impl.OmitCards(setTwo.hand_logic.hand_impl); UpdateSameSuits();
    }else
    {
        addendSum.SetUnique(setOne.hand_logic.hand_impl);//usually onlyCommunity?
        deck_impl.OmitCards(setOne.hand_logic.hand_impl); UpdateSameSuits();
        addendSum.SetUnique(setTwo.hand_logic.hand_impl);
        deck_impl.OmitCards(setTwo.hand_logic.hand_impl); UpdateSameSuits();
    }

}

// TODO(from joseph): Optimize data layout for cache utilization
// addendSameSuit_bits &= (1 << (suitNumA * 4 + suitNumB));
#define addendSetStillSameSuit(suitA, suitB, stillSame) do { this->addendSameSuit[(suitA)][(suitB)] &= (stillSame); } while (0)

void DealRemainder::UpdateSameSuits()
{
    for( int8 suitNumA=0;suitNumA<4;++suitNumA)
    {
        for( int8 suitNumB=0;suitNumB<4;++suitNumB)
        {
            addendSetStillSameSuit(suitNumA, suitNumB, (addendSum.SeeCards(suitNumA) == addendSum.SeeCards(suitNumB)));
        }
    }

    deck_impl.sortSuits();
}

void DealRemainder::LockNewAddend()
{
    addendSum.ResetCardset( deck_impl.dealtHand );
    UpdateSameSuits();

    SetIndependant();
}


template<typename T> float64 DealRemainder::AnalyzeComplete_impl(DealRemainder * const dealSource, T * const lastStats)
{
    #ifdef PROGRESSUPDATE
    lastStats->winloss_counter.handsComputed = 0;
    #endif

	dealSource->CleanStats();
	float64 returnResult;

    const int16 & moreCards = lastStats->winloss_counter.moreCards;


	if( moreCards == 0 )
	{
		lastStats->Compare(1);
		returnResult = 1;
	}else
	{
		returnResult = DealRemainder::executeRecursive(*dealSource, lastStats,lastStats->winloss_counter.moreCards);
	}

	lastStats->Analyze();

	return returnResult;
}
template float64 DealRemainder::AnalyzeComplete_impl<WinStats>(DealRemainder * const, WinStats * const);
template float64 DealRemainder::AnalyzeComplete_impl<CallStats>(DealRemainder * const, CallStats * const);
template float64 DealRemainder::AnalyzeComplete_impl<CommunityCallStats>(DealRemainder * const, CommunityCallStats * const);

float64 DealRemainder::AnalyzeComplete(CommunityCallStats * const lastStats) {
  return AnalyzeComplete_impl(this, lastStats);
}
float64 DealRemainder::AnalyzeComplete(CallStats * const lastStats) {
  return AnalyzeComplete_impl(this, lastStats);
}
float64 DealRemainder::AnalyzeComplete(WinStats * const lastStats) {
  return AnalyzeComplete_impl(this, lastStats);
}


template<typename T> float64 DealRemainder::executeComparison(const DealRemainder & refDeck, T (* const lastStats), const float64 fromRuns)
{

    DealRemainder deckState(refDeck);
    float64 totalRuns = 0;
	StatRequest r;
	float64 dOcc;

    while ( (dOcc = deckState.DealCard(deckState.justDealt)*fromRuns) > 0 )
    {
        if(deckState.dealt.Value == HoldemConstants::CARD_ACELOW)
        {
            break;
        }

        r = lastStats->NewCard(deckState.dealt,dOcc);

/// ======================================
///    Process Situation with lastStats
/// ======================================

#ifdef PROGRESSUPDATE
++(lastStats->winloss_counter.handsComputed);
#endif

        if(r.bNewSet)
        {
            lastStats->Compare(1);
            totalRuns += 1;
        }
        else
        {
            lastStats->Compare( dOcc );
            totalRuns += dOcc;
        }

//It's okay to use deckState.dealt here instead of a separate lastDealt, because this deckState isn't going to be passed on anywhere else
        deckState.justDealt.RemoveFromHand(deckState.dealt);
        lastStats->DropCard(deckState.dealt);
        deckState.UndealCard(deckState.dealt);

    }


    return totalRuns;
}
template float64 DealRemainder::executeComparison<CallStats>(const DealRemainder & refDeck, CallStats (* const lastStats), const float64 fromRuns);
template float64 DealRemainder::executeComparison<WinStats>(const DealRemainder & refDeck, WinStats (* const lastStats), const float64 fromRuns);


template<typename T> float64 DealRemainder::executeDealing(DealRemainder & deckState, T (* const lastStats), const int16 moreCards, const float64 fromRuns)
{
    if ( moreCards == 1 )
    {
        return executeComparison(deckState,lastStats,fromRuns);
    }///EARLY RETURN


	float64 totalRuns = 0;
	StatRequest r;
	float64 dOcc;


    DeckLocation lastDealt;

    while( (dOcc = deckState.DealCard(deckState.justDealt)*fromRuns) > 0 )
    {

/// ==================
///    Get New Card
/// ==================


///Note: You can use lastDealt to see which card was dealt
                lastDealt = deckState.dealt;


/// ====================
///    Check Validity
/// ====================

            if(lastDealt.Value == HoldemConstants::CARD_ACELOW || dOcc == 0)
            {//We couldn't deal a card, this is termination
                return totalRuns;
            }///EARLY RETURN


/// ===============================
///    Add New Card to lastStats
/// ===============================

				r = lastStats->NewCard(lastDealt,dOcc); //lastStats is the PlayerStrategy object

/// ===================================
///    Check response from ->NewCard
/// ===================================

                if (r.bNewSet)
                {

                    ///RECURSION HERE
                    totalRuns += executeRecursive(deckState, lastStats, moreCards - 1) * dOcc;
                }
                else
                {
                    ///RECURSION HERE
                        totalRuns += executeDealing(deckState, lastStats, moreCards - 1, dOcc);
                }


			deckState.justDealt.RemoveFromHand(lastDealt);
			lastStats->DropCard(lastDealt);

			deckState.UndealCard(lastDealt);
		}

    return totalRuns;
}
template float64 DealRemainder::executeDealing<CallStats>(DealRemainder &, CallStats (* const lastStats), const int16, const float64);
template float64 DealRemainder::executeDealing<WinStats>(DealRemainder &, WinStats (* const lastStats), const int16, const float64);

///TODO: Remove totalruns? (no need to return something)
//We need to distinguish between counting occurrences that contribute to statgroup.repeated, versus counting occurrences that contribute to Comparisons,
//fromRuns is carried through for the purpose of compounding, and is set to 1 when needed during traversal
template<typename T> float64 DealRemainder::executeRecursive(const DealRemainder & refDeck, T (* const lastStats), const int16 moreCards)
{
    DealRemainder deckState(refDeck);

//The card you just dealt completes an addendSet. The next card must be part of a new addendSet.

    deckState.LockNewAddend();
    deckState.justDealt.SetEmpty();

    ///We used to store suitorder here, but we keep a stack during deckState traversal now.

    return executeDealing(deckState, lastStats, moreCards, 1);
}
template float64 DealRemainder::executeRecursive<CallStats>(const DealRemainder & refDeck, CallStats (* const lastStats), const int16);
template float64 DealRemainder::executeRecursive<WinStats>(const DealRemainder & refDeck, WinStats (* const lastStats), const int16);
