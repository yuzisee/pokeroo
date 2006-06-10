
#ifndef HOLDEM_BaseStrategy
#define HOLDEM_BaseStrategy

#include "holdem2.h"
#include "ai.h"



class OrderedDeck
{

	private:
    //int dealtCardsNum;
		uint32 dealtHand[4]; //master "dealt or not" list
	protected:
		int8 firstSuit;
		int8 nextSuit[4];
		int8 prevSuit[4];
	public:
		const void sortSuits();
    //static const int Occurrences(unsigned long*);
    //int lastOccurrences;
		const void UndealAll();//Make sure you Empty all the hands!
		virtual float64 DealCard(Hand&);
		const void UndealCard(const DeckLocation&);
		const void OmitCards(const Hand&);
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

		const void SetIndependant();
		const void SetNextSuit();

    //after redealing it, set dealtSuit and dealtValue to
    //0 and ACELOW. This allows the next card to be independant
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

		bool operator==(const OrderedDeck&) const;
}
;

class DealRemainder : public OrderedDeck
{
	private:

		Hand addend; //temporary
//		double executeRecursive(int,unsigned long,double);
		float64 executeIterative();
		int8 moreCards;
	public:

//		bool bRecursive;

		PlayStats* lastStats;

		const void CleanStats();

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
