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

///Selected because there are ~2500 possible entries in the hash table
const uint32 CallCumulationMap::PRIMESIZE_1d= 2999; //5099;
//const unsigned long CallCumulationMap::PRIMESIZE_2d= 79;
const uint32 CallCumulationMap::PRIMES[14] =
	{1,2,3,5,7,11,13,17,19,23,29,31,37,41};

const uint32 CallCumulationMap::hashCardPattern(const Hand& h) const
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

const uint32 CallCumulationMap::hashValueset(const HandPlus& h) const
{
	return h.getValueset() % tableSize;
}

const uint32 CallCumulationMap::hashValues(const Hand& h) const
{
	return (( h.SeeCards(0)
			+ h.SeeCards(1)
			+ h.SeeCards(2)
			+ h.SeeCards(3) ) >> 1) % tableSize;
}

const uint32 CallCumulationMap::hashSuitCounts(const CommunityPlus& h) const
{
	uint32 hashval = 1;
	for(int8 i=0;i<4;++i)
	{
		hashval *= PRIMES[h.CardsInSuit(i)] % tableSize;
		hashval %= tableSize;
	}
	return hashval;
}

///uint32 hashv = (hashCardPattern(h) * hashSuitCounts(h)) % tableSize;
///hashValueset(h); //On miss
const void CallCumulationMap::add(const Hand& h, const uint8 carda, const uint8 cardb)
{

	uint32 ihash = PRIMES[HoldemUtil::CardRank(carda)]
							* PRIMES[HoldemUtil::CardRank(cardb)];

	/*uint32 hashv = ihash;
	hashv %= tableSize;
	hashv *= hashCardPattern(h);
	hashv %= tableSize;
	hashv *= hashSuitCounts(h);
	hashv %= tableSize;*/
	uint32 hashv = hashCardPattern(h); //* hashSuitCounts(h)) % tableSize; is also acceptable
	//cout << "\t" << hashv;

	MappedHand * sortedHand = new MappedHand();
	sortedHand->mySortedHand.UndealAll();
	sortedHand->mySortedHand.OmitCards(h);
	sortedHand->mySortedHand.sortSuits();


	while( table[hashv] != 0 )
	{
		OrderedDeck& hitLoc = (table[hashv])->mySortedHand;
		if (hitLoc == sortedHand->mySortedHand)
		{
                #ifdef DEBUGHASHCHOICE
                    cout << "<<  " << (int)carda << "," << (int)cardb << "\t MATCH " << hashv << endl;
                #endif

			StatResult* targetHit = &(table[hashv]->r);

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

			delete sortedHand;
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
		callWinsPtr[carda][cardb] = &(sortedHand->r);
		callWinsPtr[cardb][carda] = callWinsPtr[carda][cardb];
	#ifdef DEBUGASSERT
	}
	else
	{
		cout << "ERROR: Repeated calls to Add with the same hand!" << endl;
	}
	#endif


}

void CallCumulationMap::cleanup()
{
	for(uint32 i=0;i<tableSize;++i)
	{
		MappedHand *delMH = table[i];
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

#ifdef DEBUGASSERT
	double
#else
	void
#endif
CallCumulationMap::BuildTable(const Hand& h1)
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
					//cout << "Deal " << (int)carda << " " << (int)cardb << flush;
					add(hb, carda, cardb);

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

void CallCumulationMap::BeginIteration()
{
	itrIndex = 0;
}

StatResult * CallCumulationMap::IterateNext()
{
	while(itrIndex < tableSize)
	{
		MappedHand *temp = table[itrIndex];
		if( temp != 0 )
		{
                #ifdef DEBUGHASHCHOICE
                    cout << " >> " << itrIndex << endl;
                #endif

			++itrIndex;
			return &(temp->r);
		}
		++itrIndex;
	}
	return 0;
}

CallCumulationMap::~CallCumulationMap()
{
	cleanup();
	delete [] table;
}


StatResult & CallCumulationMap::fetchStat(const uint8 a, const uint8 b)
{
	return *(callWinsPtr[a][b]);
}

StatResult & WinCallStats::getCallWins()
{

	return calcmap.fetchStat(callDeck[0].GetIndex(),callDeck[1].GetIndex());
}

const void WinCallStats::countSplit(const double occ)
{
	WinStats::countSplit(occ);
	StatResult & s = getCallWins();
	s.splits += occ * myWins[statGroup].repeated;
		#ifdef DEBUGWHICHCARDS
			if(	(callDeck[0].GetIndex() == CARDA && callDeck[1].GetIndex() == CARDB)
			||	(callDeck[1].GetIndex() == CARDA && callDeck[0].GetIndex() == CARDB))
			{
				cout << "s[" << callDeck[0].GetIndex() << "," << callDeck[1].GetIndex() << "]\t"
				<< occ <<" × "<< myWins[statGroup].repeated << endl;
				oppStrength.DisplayHandBig();
				myStrength.DisplayHandBig();
			}
		#endif
	//if( ((callDeck[0].GetIndex() == 3 && callDeck[1].GetIndex() == 43) || (callDeck[1].GetIndex() == 3 && callDeck[0].GetIndex() == 43) ) ){ myStrength.DisplayHand(); cout << s.splits + s.loss + s.wins << endl;}
}
const void WinCallStats::countLoss(const double occ)
{
	WinStats::countLoss(occ);
	StatResult & s = getCallWins();
	s.loss += occ * myWins[statGroup].repeated;

		#ifdef DEBUGWHICHCARDS
			if(	(callDeck[0].GetIndex() == CARDA && callDeck[1].GetIndex() == CARDB)
			||	(callDeck[1].GetIndex() == CARDA && callDeck[0].GetIndex() == CARDB))
			{
				cout << "l[" << callDeck[0].GetIndex() << "," << callDeck[1].GetIndex() << "]\t"
				<< occ <<" × "<< myWins[statGroup].repeated << endl;
				//cout << "bestpair" << oppStrength.bestPair << endl;
				oppStrength.DisplayHandBig();
				myStrength.DisplayHandBig();
			}
		#endif
//if( ((callDeck[0].GetIndex() == 3 && callDeck[1].GetIndex() == 43) || (callDeck[1].GetIndex() == 3 && callDeck[0].GetIndex() == 43) ) ){ myStrength.DisplayHand(); cout << s.splits + s.loss + s.wins << endl;}
}
const void WinCallStats::countWin(const double occ)
{
	WinStats::countWin(occ);
	StatResult & s = getCallWins();
	s.wins += occ * myWins[statGroup].repeated;
		#ifdef DEBUGWHICHCARDS
			if(	(callDeck[0].GetIndex() == CARDA && callDeck[1].GetIndex() == CARDB)
			||	(callDeck[1].GetIndex() == CARDA && callDeck[0].GetIndex() == CARDB))
			{
				cout << "w[" << callDeck[0].GetIndex() << "," << callDeck[1].GetIndex() << "]\t"
				<< occ <<" × "<< myWins[statGroup].repeated << endl;
				oppStrength.DisplayHandBig();
				myStrength.DisplayHandBig();
			}
		#endif
        if( ((callDeck[0].GetIndex() == 3 && callDeck[1].GetIndex() == 43) || (callDeck[1].GetIndex() == 3 && callDeck[0].GetIndex() == 43) ) ){ myStrength.DisplayHand(); /*cout << s.splits + s.loss + s.wins << endl;*/ cout << occ << "\t repeated=" << myWins[statGroup].repeated << endl;}
}

StatRequest WinCallStats::NewCard(const DeckLocation deck, double occ)
{
	StatRequest ret = WinStats::NewCard(deck,occ);
	//currentCard has now been adjusted
	short cardsLeft = moreCards - currentCard + 1;
	if( cardsLeft <= 2 )
	{
		callDeck[cardsLeft-1] = deck;
        if( cardsLeft == 1 && ((callDeck[0].GetIndex() == 3 && callDeck[1].GetIndex() == 43) || (callDeck[1].GetIndex() == 3 && callDeck[0].GetIndex() == 43) ) ) myStrength.DisplayHand();
	}

	return ret;
}

const void WinCallStats::Analyze()
{
	loadCumulation();
	WinStats::Analyze();
}

const void WinCallStats::initWC(const CommunityPlus& withCom, const int8 cardsInCommunity)
{
	int8 cardsAvail = 52 - cardsInCommunity - 2;
	callTotalChances = (cardsAvail * (cardsAvail - 1)) / 2;
	#ifdef DEBUGASSERT
	if( callTotalChances != calcmap.BuildTable(withCom) )
	{
		cout << "CAREFULLY CHECK HASH TABLE BUILD FUNCTIONALITY!" << endl;
		cout << "CAREFULLY CHECK HASH TABLE BUILD FUNCTIONALITY!" << endl;
		cout << "CAREFULLY CHECK HASH TABLE BUILD FUNCTIONALITY!" << endl;
		cout << "CAREFULLY CHECK HASH TABLE BUILD FUNCTIONALITY!" << endl;
		cout << "CAREFULLY CHECK HASH TABLE BUILD FUNCTIONALITY!" << endl;
	}
	#endif
	callChancesEach = HoldemUtil::nchoosep<float64>(cardsAvail - 2,5-cardsInCommunity);
}

/*
const void WinCallStats::mergeIn(StatResult& base, const StatResult& in)
{
	if(	in.loss > 0 ||
			in.splits > 0 ||
			in.wins > 0 )
		{
			base.loss += in.loss*in.repeated;
			base.splits += in.splits*in.repeated;
			base.wins += in.wins*in.repeated;
			base.repeated = 1;
		}
}
*/

const void WinCallStats::loadCumulation()
{

	int32 callHands = 0;

	vector<StatResult> forC;


	calcmap.BeginIteration();
	StatResult *nextStat;
	nextStat = calcmap.IterateNext();
	while(nextStat != 0){

		double dblRep = nextStat->repeated;

		nextStat->loss /= dblRep;
		nextStat->wins /= dblRep;
		nextStat->splits /= dblRep;
		nextStat->genPCT();
		forC.push_back(*nextStat);

		++callHands;
		nextStat = calcmap.IterateNext();
	}


			#ifdef DEBUGCALLPCT
				cout << endl << "=============Ai CACHE (unsorted) =============" << endl;
				for(int32 i=0;i<callHands;i++)
				{
					cout << endl << "{" << i << "}" << forC[i].loss << " l + "
							<< forC[i].splits << " s + " << forC[i].wins << " w = " <<
							forC[i].loss+forC[i].splits+forC[i].wins
							<< "\t×"<< forC[i].repeated <<flush;
				}
			#endif



	sort(forC.begin(), forC.end());


/*
			#ifdef DEBUGCALLPCT
				cout << endl << "=============Ai CACHE (sorted) =============" << endl;
				for(int32 i=0;i<callHands;i++)
				{
					cout << endl << "{" << i << "}" << forC[i].loss << " l + "
							<< forC[i].splits << " s + " << forC[i].wins << " w = " <<
							forC[i].loss+forC[i].splits+forC[i].wins
							<< "\t×"<< forC[i].repeated <<flush;
				}
			#endif
*/



	//populate cumulation
	int32 j;
	double cumulate = 0;

	j=1;

	calc.cumulation.reserve(forC.size());

	calc.cumulation.push_back(forC[0]);
	size_t vectorLast = calc.cumulation.size()-1;//(zero)

	while(j < callHands)
	{
		StatResult &thisSR = forC[j];

		cumulate += thisSR.repeated;

		if (!( thisSR.bIdenticalTo( forC[j-1]) ))
		{	//add new StatResult
			calc.cumulation.push_back(thisSR);
			++vectorLast;
		}
		//cumulation[vectorLast].repeated = cumulate;
		calc.cumulation.back().repeated = cumulate;
		++j;
	}

	/*
	for(size_t k=0;k<=vectorLast;++k)
	{
		calc.cumulation[k].wins /= callChancesEach;
		calc.cumulation[k].loss /= callChancesEach;
		calc.cumulation[k].splits /= callChancesEach;
		calc.cumulation[k].pct = 1 - calc.cumulation[k].pct/callChancesEach;
		calc.cumulation[k].repeated /= callTotalChances;
	}
*/

#ifdef DEBUGCALLPCT
cout << endl << "=============Ai CACHE Optimal=============" << endl;
	cout.precision(4);
	for(size_t i=0;i<=vectorLast;i++)
	{
		cout << endl << "{" << i << "}" << calc.cumulation[i].loss << " l +\t"
				<< calc.cumulation[i].splits << " s +\t" << calc.cumulation[i].wins << " w =\t" <<
				calc.cumulation[i].pct
				<< " pct\t×"<< calc.cumulation[i].repeated <<flush;
	}
#endif

}
