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

using std::cout;
using std::endl;

const OrderedDeck OrderedDeck::EMPTY_ODECK;

///Assumes sorted
bool OrderedDeck::operator==(const OrderedDeck& o) const
{
	bool bMatch = true;
	int8 csuit = firstSuit;
	int8 osuit = o.firstSuit;
	bMatch &= dealtHand[csuit] == o.dealtHand[osuit];
	//if ( dealtHand[csuit] != dealtHand[osuit] ) return false;
	csuit = nextSuit[csuit];osuit = o.nextSuit[osuit];
	bMatch &= dealtHand[csuit] == o.dealtHand[osuit];
	//if ( dealtHand[csuit] != dealtHand[osuit] ) return false;
	csuit = nextSuit[csuit];osuit = o.nextSuit[osuit];
	bMatch &= dealtHand[csuit] == o.dealtHand[osuit];
	//if ( dealtHand[csuit] != dealtHand[osuit] ) return false;
	csuit = nextSuit[csuit];osuit = o.nextSuit[osuit];
	bMatch &= dealtHand[csuit] == o.dealtHand[osuit];
	//if ( dealtHand[csuit] != dealtHand[osuit] ) return false;
	return bMatch;
}

void OrderedDeck::sortSuits()
{
    int8 s0 = firstSuit;
    int8 s1 = nextSuit[s0];
    int8 s2 = nextSuit[s1];
    int8 s3 = nextSuit[s2];


	//Mergesort, stable

    //We implicitly divide the hands into two groups.
    //f1 represents suit 0 and suit 1
    //f2 represents suit 2 and suit 3
	int8 f1, f2;

//We sort the f1 subgroup
	if( dealtHand[s0] >= dealtHand[s1] )
	{
		f1 = s0;
		nextSuit[s0] = s1;
		nextSuit[s1] = HoldemConstants::NO_SUIT;
	}
	else
	{
		f1 = s1;
		nextSuit[s1] = s0;
		nextSuit[s0] = HoldemConstants::NO_SUIT;
	}

//We sort the f2 subgroup
	if( dealtHand[s2] >= dealtHand[s3] )
	{
		f2 = s2;
		nextSuit[s2] = s3;
		nextSuit[s3] = HoldemConstants::NO_SUIT;
	}
	else
	{
		f2 = s3;
		nextSuit[s3] = s2;
		nextSuit[s2] = HoldemConstants::NO_SUIT;
	}

//We select from the head of the two sublists to form firstSuit, and then shorten that sublist
	if ( dealtHand[f1] >= dealtHand[f2] )
	{
		firstSuit = f1;
		f1 = nextSuit[f1];
	}
	else
	{
		firstSuit = f2;
		f2 = nextSuit[f2];
	}

	int8 curSuit = firstSuit;


//While both lists exist, continue to take the higher of the two sublist heads
	while(f2 != HoldemConstants::NO_SUIT && f1 != HoldemConstants::NO_SUIT)
	{

		if (dealtHand[f1] >= dealtHand[f2] )
		{
			nextSuit[curSuit] = f1;
			curSuit = f1;
			f1 = nextSuit[f1];
		}
		else
		{
			nextSuit[curSuit] = f2;
			curSuit = f2;
			f2 = nextSuit[f2];
		}
	}

	if( f2 == HoldemConstants::NO_SUIT )
	{
		nextSuit[curSuit] = f1;
	}
	else
	{
		nextSuit[curSuit] = f2;
	}

	prevSuit[firstSuit] = HoldemConstants::NO_SUIT;

	curSuit = firstSuit;


//Assign all the prevsuits
	while(nextSuit[curSuit] != HoldemConstants::NO_SUIT)
	{

		prevSuit[nextSuit[curSuit]] = curSuit;
		curSuit=nextSuit[curSuit];
	}


}

void OrderedDeck::AssignSuitsFrom(const OrderedDeck & suitorder)
{
    firstSuit = suitorder.firstSuit;

    for( int8 i=0;i<4;++i)
    {
        nextSuit[i] = suitorder.nextSuit[i];
        prevSuit[i] = suitorder.prevSuit[i];
        dealtHand[i] = suitorder.dealtHand[i];
    }
}



void DealableOrderedDeck::UndealCard(const DeckLocation& deck)
{
	dealtHand[deck.Suit] &= ~deck.Value;
	dealt = deck;
}



float64 DealRemainder::DealCard(Hand& h)
{
    const int8 & qprevSuit = prevSuit[dealt.Suit];

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
	else if ( (dealtHand[dealt.Suit] & dealt.Value) != 0 )
	{//card already dealt/omitted, i.e. needs to be skipped

		return DealCard(h);
	}
	else if(prevSuit[dealt.Suit] != HoldemConstants::NO_SUIT) //unless we are in the first suit, we have to check for certain cases
	{
        ///What we're looking for is...
        ///    (Action) Reason...
        /// ------------------------
        /// 1. (Skip) Dealing into already identical suits: bPastMatchesOld + suits are the same (transient) in the hand being dealt to
        /// 2. (occBase = 1) Even though bMatchesOld? If for example, hHere and hBack are different, but it has coincidentally
        ///                  Imagine 73 of Spades, and just a 7 of Hearts. You could possibly deal 3 of Hearts into h, then bMatchesOld!
        ///                  So, when you deal the next card, and it doesn't violate the "greater first" rule, but because hBack!=hHere it also doesn't skip the suit,

        /// 3. (matchesnew)  Imagine having 2 Spades, and then dealing a 2 of Hearts. That has �3, but then what if the next card is 2 of Clubs, without LockNewAddend?


		const uint32 hHere=h.SeeCards(dealt.Suit);
		const uint32 hBack=h.SeeCards(qprevSuit);


        const bool bPreviouslyIdentical = addendSameSuit[dealt.Suit][qprevSuit];

		if(
             bPreviouslyIdentical//if eligible for "greater first" rule
             &&
             (hHere | dealt.Value) > hBack //would violate "greater first" rule
           )
        {
            //To optimize redundant paths, we keep additions sorted in previously identical suits
			SetNextSuit();
			return DealCard(h);
		}


        if (bPreviouslyIdentical && hBack==hHere) //bMatchesOld here implies bPreviouslyIdentical, if also hBack==hHere
		{
            //Essentially, we need to avoid adding to this suit, since it is the same as the last suit, which would already
            //have been counted for double!
			SetNextSuit();
			return DealCard(h);
		}
	}


	//successful!
	float64 occBase = 0;

	uint32 baseInto = dealtHand[dealt.Suit]; ///This is prior to {dealtHand[dealt.Suit] |= dealt.Value;}

		for( int8 i=dealt.Suit;i!=HoldemConstants::NO_SUIT;i = nextSuit[i])
		{//Although we could check all four here, it is assumed that you would only deal into the first of identical suits, plus it may help sort out the 2/2/2 corner case

			if ((dealtHand[i] == baseInto) && (addendSameSuit[dealt.Suit][i]))
			{
				++occBase;
			}
			else
				break;
		}


	dealtHand[dealt.Suit] |= dealt.Value;


	h.AddToHand(dealt);

	uint32 dealtTo = dealtHand[dealt.Suit];
	uint32 addedTo = h.SeeCards(dealt.Suit);


	float64 matchesNew = 0; //matchesNew reflects how many new duplicate suits formed.

	for(int8 i=0;i<4;++i)
	{
		if (dealtHand[i] == dealtTo &&
				  h.SeeCards(i) == addedTo &&
				        addendSameSuit[dealt.Suit][i] )  ++matchesNew;
	}


	return occBase/matchesNew;
}


void OrderedDeck::OmitCard(const DeckLocation& deck)
{
	dealtHand[deck.Suit] |= deck.Value;
}

void OrderedDeck::OmitCards(const Hand& h)
{
	OrderedDeck::dealtHand[0] |= h.SeeCards(0);
	OrderedDeck::dealtHand[1] |= h.SeeCards(1);
	OrderedDeck::dealtHand[2] |= h.SeeCards(2);
	OrderedDeck::dealtHand[3] |= h.SeeCards(3);
}

void DealableOrderedDeck::SetIndependant()
{
	dealt.Value = BaseDealtValue();
	dealt.Suit = BaseDealtSuit();
	dealt.Rank = BaseDealtRank();
}

void DealableOrderedDeck::SetNextSuit()
{
	dealt.Suit = nextSuit[dealt.Suit];
	dealt.Value = BaseDealtValue();
	dealt.Rank = BaseDealtRank();
}

void OrderedDeck::SetEmpty()
{
	OrderedDeck::dealtHand[0] = 0;
	OrderedDeck::dealtHand[1] = 0;
	OrderedDeck::dealtHand[2] = 0;
	OrderedDeck::dealtHand[3] = 0;
}


void DealableOrderedDeck::UndealAll()
{
	OrderedDeck::SetEmpty();
	SetIndependant();
}


OrderedDeck::~OrderedDeck()
{
}


DealableOrderedDeck::~DealableOrderedDeck()
{
}

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




    if( setOne.IsEmpty() )
    {//This is used especially by aiInformation
        addendSum.SetUnique(setTwo);
        OmitCards(setTwo); UpdateSameSuits();
    }else
    {
        addendSum.SetUnique(setOne);//usually onlyCommunity?
        OmitCards(setOne); UpdateSameSuits();
        addendSum.SetUnique(setTwo);
        OmitCards(setTwo); UpdateSameSuits();
    }

}

void DealRemainder::UpdateSameSuits()
{
    for( int8 suitNumA=0;suitNumA<4;++suitNumA)
    {
        for( int8 suitNumB=0;suitNumB<4;++suitNumB)
        {
            addendSameSuit[suitNumA][suitNumB] &= (addendSum.SeeCards(suitNumA) == addendSum.SeeCards(suitNumB));
        }
    }

    sortSuits();
}

void DealRemainder::LockNewAddend()
{
    addendSum.ResetCardset( dealtHand );
    UpdateSameSuits();

    SetIndependant();
}

float64 DealRemainder::AnalyzeComplete(PlayStats* lastStats)
{

    #ifdef PROGRESSUPDATE
    lastStats->handsComputed = 0;
    #endif

	DeckLocation pos;
	pos.Rank = BaseDealtRank();
	pos.Value = BaseDealtValue();
	pos.Suit = BaseDealtSuit();


	CleanStats();
	float64 returnResult;

    const int16 & moreCards = lastStats->moreCards;


	if( moreCards == 0 )
	{
		lastStats->Compare(1);
		returnResult = 1;
	}else
	{
		returnResult = executeRecursive(*this,lastStats,lastStats->moreCards);
	}

	lastStats->Analyze();

	return returnResult;
}

float64 DealRemainder::executeComparison(const DealRemainder & refDeck, PlayStats (* const lastStats), const float64 fromRuns)
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
++(lastStats->handsComputed);
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

float64 DealRemainder::executeDealing(DealRemainder & deckState, PlayStats (* const lastStats), const int16 moreCards, const float64 fromRuns)
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

///TODO: Remove totalruns? (no need to return something)
//We need to distinguish between counting occurrences that contribute to statgroup.repeated, versus counting occurrences that contribute to Comparisons,
//fromRuns is carried through for the purpose of compounding, and is set to 1 when needed during traversal
float64 DealRemainder::executeRecursive(const DealRemainder & refDeck, PlayStats (* const lastStats), const int16 moreCards)
{




    DealRemainder deckState(refDeck);

//The card you just dealt completes an addendSet. The next card must be part of a new addendSet.

    deckState.LockNewAddend();
    deckState.justDealt.SetEmpty();

    ///We used to store suitorder here, but we keep a stack during deckState traversal now.

    return executeDealing(deckState, lastStats, moreCards, 1);

}
