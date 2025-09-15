#include "engine_base.h"

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
