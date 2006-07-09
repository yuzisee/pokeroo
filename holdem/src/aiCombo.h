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


#ifndef HOLDEM_AICombo
#define HOLDEM_AICombo

#include "ai.h"
#include "engine.h"



struct MappedHand
{
	OrderedDeck mySortedHand;
	/*unsigned char a;
	unsigned char b;*/
	StatResult r;
}
;

class CallCumulationMap
{
private:

	//static const uint32 PRIMESIZE_2d;
	static const uint32 PRIMESIZE_1d;
	static const uint32 PRIMES[14];


	const uint32 hashCardPattern(const Hand&) const;
	const uint32 hashValueset(const HandPlus&) const;
	const uint32 hashValues(const Hand&) const;
	const uint32 hashSuitCounts(const CommunityPlus&) const;

	virtual void cleanup();

protected:
///TODO: on new, store BOTH [31][52] and [52][31]
///TODO: on hit, store INTO BOTH ^^^^^^^^^^^^^^^^
	MappedHand **table;
	uint32 tableSize;
	uint32 itrIndex;

	StatResult *(callWinsPtr[52][52]);

	const virtual void add(const Hand&, const uint8, const uint8) ;

public:

#ifdef DEBUGASSERT
	double
#else
	void
#endif
virtual BuildTable(const Hand& h);

	//void Hash2D(const CommunityPlus&, uint32 &, uint32 &) const;
	CallCumulationMap() : itrIndex(0)
	{
		for(int8 ia=0;ia<52;++ia)
		{for(int8 ib=0;ib<52;++ib){
			callWinsPtr[ia][ib] = 0;
		}}

		tableSize=PRIMESIZE_1d;
		table = new MappedHand *[tableSize];
		for(uint32 i=0;i<tableSize;++i)
		{
			table[i] = 0;
		}
	}

	virtual ~CallCumulationMap();

	virtual StatResult & fetchStat(const uint8, const uint8);
	virtual void BeginIteration();
	virtual StatResult * IterateNext();

}
;

class WinCallStats : public virtual WinStats
{
private:
	const void loadCumulation();
	const void initWC(const CommunityPlus&, const int8);
	//static const void mergeIn(StatResult&, const StatResult&);
protected:
	const virtual void countWin(const float64);
	const virtual void countSplit(const float64);
	const virtual void countLoss(const float64);
//TODO: Init and delete calcmap
	CallCumulationMap calcmap;
	CallCumulation calc;
	double callChancesEach;
	double callTotalChances;
	DeckLocation callDeck[2];

	virtual StatResult & getCallWins();

public:
	float64 pctWillCall(const float64 oddsFaced) const
	{
		return calc.pctWillCall(oddsFaced);
	}

	const virtual void Analyze();
	virtual StatRequest NewCard(const DeckLocation, const float64 occ);

	WinCallStats(const CommunityPlus& myP, const CommunityPlus& cP,
		const short cardsInCommunity)
		 : PlayStats(myP, cP), WinStats(myP, cP, cardsInCommunity)
	{
		initWC(myP, cardsInCommunity);
	}

		#ifdef DEBUGCALLPART
			void debugPrint()
			{
				cout << endl << "s[" << callDeck[0].GetIndex() << "," << callDeck[1].GetIndex() <<
				"]\t x" << myWins[statGroup].repeated;
				oppStrength.DisplayHandBig();
				myStrength.DisplayHandBig();
			}

			const DeckLocation* debugViewD(int i)
			{
				return callDeck+i;
			}
		#endif

}
;


#endif
