/***************************************************************************
 *   Copyright (C) 2005 by Joseph Huang                                    *
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

//#define DEBUGRANDOMDEAL
//#include <iostream>

#define FORCESEED

//#define COOLSEEDINGVIEWER

#include "randomDeck.h"
#include <cstdlib>
#include <ctime>


/*
	dealer.dealtValue = HoldemUtil::CARDORDER[ (rand() % 13) + 1 ];
	dealer.dealtSuit = rand() % 4;

*/

void RandomDeck::ShuffleDeck()
{
	#ifdef FORCESEED
	srand(80); //5
	#else
	srand(time(0));
	#endif


	firstDealtPos = lastDealtPos;

	for(uint8 i=0;i<DECKSIZE;++i)
	{
		uint8 swapWith = (rand()%(DECKSIZE-i))+i;
		unsigned char tempSwap;
		tempSwap = deckOrder[i];
		deckOrder[i] = deckOrder[swapWith];
		deckOrder[swapWith] = tempSwap;
	}

}

void RandomDeck::ShuffleDeck(float64 seedShift)
{

	++lastDealtPos;
	lastDealtPos %= DECKSIZE;


	//lastDealtIs okay up to 5 bits and then one more...
	uint32 wrap = (lastDealtPos & 31) | ((lastDealtPos & 32) << 26);


	ShuffleDeck();


	//int is long by default (4-byte)
	uint32* warp = reinterpret_cast<uint32*>(&seedShift);
	uint32 psychoRandom = *(warp) ^ *(warp+1);
	wrap ^= (psychoRandom >> 8);
	wrap ^= (psychoRandom << 8);
	psychoRandom = psychoRandom ^ wrap;

#ifdef COOLSEEDINGVIEWER

	const uint32 viewMask = 1 << 31;
	uint32 viewSeed;

	viewSeed = psychoRandom;
	int8 bitsSeen = 0;
	cout << "Randomizer seed " << seedShift << " produces {" << flush;
	while(viewSeed != 0)
	{
		cout << ((viewSeed & viewMask) >> 31) << flush;
		++bitsSeen;
		viewSeed <<= 1;
	}
	cout << "...&"<< (32-bitsSeen) <<"}" << endl;
/*
	viewSeed = *(warp);
	cout << "\t" << flush;
	while(viewSeed != 0)
	{
		cout << ((viewSeed & viewMask) >> 31) << flush;
		viewSeed <<= 1;
	}

	viewSeed = *(warp + 1);
	cout << "...," << flush;
	while(viewSeed != 0)
	{
		cout << ((viewSeed & viewMask) >> 31) << flush;
		viewSeed <<= 1;
	}
	cout << "... and using " << lastDealtPos << endl;
*/
#endif

	srand(psychoRandom);

	for(int8 i=0;i<DECKSIZE;++i)
	{
		int8 swapWith = (rand()%(DECKSIZE-i))+i;
		uint8 tempSwap;
		tempSwap = deckOrder[i];
		deckOrder[i] = deckOrder[swapWith];
		deckOrder[swapWith] = tempSwap;
	}

}


float64 RandomDeck::DealCard(Hand& h)
{
	++lastDealtPos;
	lastDealtPos %= DECKSIZE;

	if( lastDealtPos == firstDealtPos )
	{
		return 0;
	}

	dealt.Rank = HoldemUtil::CardRank(deckOrder[lastDealtPos])+1;
	dealt.Value = HoldemUtil::CARDORDER[dealt.Rank];
	dealt.Suit = HoldemUtil::CardSuit(deckOrder[lastDealtPos]);



#ifdef DEBUGRANDOMDEAL
	cout << "DEAL!\tSuit=" << HoldemUtil::CardSuit(deckOrder[lastDealtPos])<< ":" << dealtSuit << "\tCard=" << (HoldemUtil::CardRank(deckOrder[lastDealtPos])+1) <<":"<< dealtValue << endl;
#endif

	h.AddToHand(dealt);

	return 1;//return 0; if no cards left
}


