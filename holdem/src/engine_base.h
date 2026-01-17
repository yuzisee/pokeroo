#define GLOBAL_AICACHE_SPEEDUP

#ifndef HOLDEM_BaseDeck
#define HOLDEM_BaseDeck


#include "holdem2.h"
#include "ai.h"


typedef bool SuitsUsedBool[13][4];

/**
// A deck of 52 cards.
// For each suit, we use a bitfield-type structure to keep track of which card is still in the deck and which are no longer present.
// The suit order is stored in a doubly-linked list.
 *
 * The main value of this class is that you can sort the suits.
 * The suit with the highest dealt card goes at the front.
 *
 * Then, in DealableOrderedDeck we can leverage symmetry as long as suits are always descending.
 * That is, we don't have to deal higher valued cards in later suits because that would have already been covered when the higher value card was dealt from the previous suit.
 */
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

                ~OrderedDeck();

		constexpr bool operator==(const OrderedDeck&) const;

		friend class DealableOrderedDeck;
		friend class DealRemainder;
}
;

typedef HandPlus dealatom_t;

class DealableOrderedDeck
{

	public:
    OrderedDeck deck_impl;
        virtual float64 DealCard(dealatom_t&)=0;
        virtual void UndealCard(const DeckLocation & deck); //Can be undealt only if not locked into addendSum

		constexpr uint32 BaseDealtValue() const
		{
			return HoldemConstants::CARD_ACELOW;
		}
		constexpr int8 BaseDealtSuit() const
		{
			return deck_impl.firstSuit;
		}
		constexpr uint8 BaseDealtRank() const
		{
			return 0;
		}

		DeckLocation dealt;

		void SetIndependant();
		void SetNextSuit();
		void UndealAll();//Make sure you Empty all the hands!

    //after redealing it, set dealtSuit and dealtValue to
    //0 and ACELOW. This allows the next card to be independant
        DealableOrderedDeck(const DealableOrderedDeck& other) : deck_impl(other.deck_impl)
        {
			dealt = other.dealt;
        }

        DealableOrderedDeck() = default;

        virtual ~DealableOrderedDeck();

}
;


// HOLDEM_BaseDeck
#endif
