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
#define DEBUGCALLPCT
//#define DEBUGWHICHCARDS
#define DEBUGHASHCHOICE 1


//#define CARDA 0
//#define CARDB 39

#include "aiCombo.h"

void PocketHand::PopulateByIndex()
{
	CommunityPlus q;
	DeckLocation assume;
	assume.SetByIndex(a);  q.AddToHand(assume);
	assume.SetByIndex(b);  q.AddToHand(assume);
	result.Reset(q);
}



///Selected because there are ~2500 possible entries in the hash table
const uint32 PocketsMap::PRIMESIZE_1d= 2999; //5099;
//const unsigned long PocketsMap::PRIMESIZE_2d= 79;



const uint32 HandHash::hashCardPattern(const Hand& h, uint32 tableSize)
{
	return(
			( (h.SeeCards(0) % tableSize) + 1 ) *
			( (h.SeeCards(1) % tableSize) + 1 ) *
			( (h.SeeCards(2) % tableSize) + 1 ) *
			( (h.SeeCards(3) % tableSize) + 1 ) ) % tableSize;
	/*return	(
			  (( (h.SeeCards(0)>>1) ^ (h.SeeCards(0)>>7) & 0x8F )+1)
			* (( (h.SeeCards(1)>>1) ^ (h.SeeCards(1)>>7) & 0x8F )+1)
			* (( (h.SeeCards(2)>>1) ^ (h.SeeCards(2)>>7) & 0x8F )+1)
			* (( (h.SeeCards(3)>>1) ^ (h.SeeCards(3)>>7) & 0x8F )+1)
			) % tableSize;*/
}

const uint32 HandHash::hashValueset(const HandPlus& h, uint32 tableSize)
{
	return h.getValueset() % tableSize;
}

const uint32 HandHash::hashValues(const Hand& h, uint32 tableSize)
{
	return (( h.SeeCards(0)
			+ h.SeeCards(1)
			+ h.SeeCards(2)
			+ h.SeeCards(3) ) >> 1) % tableSize;
}

const uint32 HandHash::hashSuitCounts(const CommunityPlus& h, uint32 tableSize)
{
	uint32 hashval = 1;
	for(int8 i=0;i<4;++i)
	{
		hashval *= HoldemUtil::PRIMES[h.CardsInSuit(i)] % tableSize;
		hashval %= tableSize;
	}
	return hashval;
}

const uint32 PocketHash::hashRank(const struct PocketHand& p, const uint32 tableSize)
{
	return (HoldemUtil::PRIMES[HoldemUtil::CardRank(p.a)]
							* HoldemUtil::PRIMES[HoldemUtil::CardRank(p.b)]); //% tableSize

}

static const bool sameDealtHand(const PocketHand& a, const PocketHand& b)
{
	return ((a.a == b.a) && (a.b == b.b)); //&& a.pocketHand == b.pocketHand
}

///uint32 hashv = (hashCardPattern(h) * hashSuitCounts(h)) % tableSize;
///hashValueset(h); //On miss
const void PocketsMap::add(const struct PocketHand& p)
{

	const uint8& carda = p.a;
	const uint8& cardb = p.b;
	uint32 ihash = PocketHash::hashRank(p,tableSize);
	/*uint32 hashv = ihash;
	hashv %= tableSize;
	hashv *= hashCardPattern(h);
	hashv %= tableSize;
	hashv *= hashSuitCounts(h);
	hashv %= tableSize;*/
	uint32 hashv = HandHash::hashCardPattern(p.result.comp, tableSize); //* hashSuitCounts(h)) % tableSize; is also acceptable
	//cout << "\t" << hashv;


	PocketHand * sortedHand = new PocketHand(p);
/*
	sortedHand->pocketHand.UndealAll();
	sortedHand->pocketHand.OmitCards(h);
	sortedHand->pocketHand.sortSuits();
*/

	while( table[hashv] != 0 )
	{
		//OrderedDeck& hitLoc = (table[hashv])->pocketHand;
		//if (hitLoc == sortedHand->pocketHand)
		if(   sameDealtHand (  *(table[hashv])  , p )   )
		{
                #ifdef DEBUGHASHCHOICE
                    cout << "<<  " << (int)carda << "," << (int)cardb << "\t MATCH " << hashv << endl;
                #endif

			StatResult* targetHit = &(table[hashv]->s);

				#ifdef DEBUGASSERT
				if( callWinsPtr[carda][cardb] == 0 && callWinsPtr[cardb][carda] == 0)
				{
				#endif

			callWinsPtr[carda][cardb] = targetHit;
			callWinsPtr[cardb][carda] = targetHit;
			targetHit->repeated += 1;


				#ifdef DEBUGASSERT
				}
				else
				{
					cout << "ERROR: Repeated calls to Add with the same hand!" << endl;
				}
				#endif

			//delete sortedHand;
			return;
		}

		hashv += ihash; //hashValues(h) is fine here too it seems
		hashv %= tableSize;
	}
	///At this point we reach an empty spot.
	///We must then insert sortedHand here.
	//sortedHand->a = carda;
	//sortedHand->b = cardb;
	table[hashv] = sortedHand;

        #ifdef DEBUGHASHCHOICE
            if( hashv == DEBUGHASHCHOICE ) cout << "<<  " << (int)carda << "," << (int)cardb << "\t NEW " << hashv << endl;
        #endif


	#ifdef DEBUGASSERT
	if( callWinsPtr[carda][cardb] == 0 && callWinsPtr[cardb][carda] == 0)
	{
	#endif
		callWinsPtr[carda][cardb] = &(sortedHand->s);
		callWinsPtr[cardb][carda] = callWinsPtr[carda][cardb];
	#ifdef DEBUGASSERT
	}
	else
	{
		cout << "ERROR: Repeated calls to Add with the same hand!" << endl;
	}
	#endif


}

void PocketsMap::cleanup()
{
	for(uint32 i=0;i<tableSize;++i)
	{
		PocketHand *delMH = table[i];
		if( delMH != 0 )
		{
			/*StatResult *delSR = callWinsPtr[delMH->a][delMH->b];
			if( delSR == 0)
			{
				delete delSR;
			}*/
			delete delMH;
		}
	}
}
/*
#ifdef DEBUGASSERT
	double
#else
	void
#endif
PocketsMap::BuildTable(const Hand& h1)
{
	#ifdef DEBUGASSERT
	double counter=0;
	#endif
	cleanup();

	Hand ha;
	Hand hb;
	DeckLocation a, b;

	for(unsigned char carda=0;carda<52;++carda)
	{
		a.Rank = HoldemUtil::CardRank(carda) + 1;
		a.Value= HoldemUtil::CARDORDER[a.Rank];
		a.Suit = HoldemUtil::CardSuit(carda);
		if( (h1.SeeCards(a.Suit) & a.Value) == 0 )
		{
			ha.SetUnique(h1);
			ha.AddToHand(a);
			for(unsigned char cardb=carda+1;cardb<52;++cardb)
			{
				b.Rank = HoldemUtil::CardRank(cardb)+1;
				b.Value = HoldemUtil::CARDORDER[b.Rank];
				b.Suit = HoldemUtil::CardSuit(cardb);
				if( (h1.SeeCards(b.Suit) & b.Value) == 0 )
				{
					hb.SetUnique(ha);
					hb.AddToHand(b);
					cout << "Deal " << (int)carda << " " << (int)cardb << flush;
					//add(hb, carda, cardb);

					#ifdef DEBUGASSERT
					counter+=1;
					#endif
				}
			}
		}
	}
	#ifdef DEBUGASSERT
	return counter;
	#endif
}
*/
void PocketsMap::BeginIteration()
{
	itrIndex = 0;
}

StatResult * PocketsMap::IterateNext()
{
	while(itrIndex < tableSize)
	{
		PocketHand *temp = table[itrIndex];
		if( temp != 0 )
		{
                #ifdef DEBUGHASHCHOICE
                    cout << " >> " << itrIndex << endl;
                #endif

			++itrIndex;
			return &(temp->s);
		}
		++itrIndex;
	}
	return 0;
}

PocketsMap::~PocketsMap()
{
	cleanup();
	delete [] table;
}


StatResult & PocketsMap::fetchStat(const uint8 a, const uint8 b)
{
	return *(callWinsPtr[a][b]);
}

