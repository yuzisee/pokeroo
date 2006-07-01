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

//#define DEBUGTOTALRUNS

//#define DEBUGTARE
//#define DEBUGINFLOOP
//#define DEBUGDEAL
//#define DEBUGSTATSDEAL
//#define DEBUGORDER
//#define DEBUGORDERING
//#include <iostream>
//#define NULL 0

#include "engine.h"
#include "holdem2.h"
#include "ai.h"

using std::cout;
using std::endl;

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

const void OrderedDeck::sortSuits()
{
	//Mergesort

	int8 f1, f2;

	if( dealtHand[0] > dealtHand[2] )
	{
		f1 = 0;
		nextSuit[0] = 2;
		nextSuit[2] = HoldemConstants::NO_SUIT;
	}
	else
	{
		f1 = 2;
		nextSuit[2] = 0;
		nextSuit[0] = HoldemConstants::NO_SUIT;
	}

	if( dealtHand[1] > dealtHand[3] )
	{
		f2 = 1;
		nextSuit[1] = 3;
		nextSuit[3] = HoldemConstants::NO_SUIT;
	}
	else
	{
		f2 = 3;
		nextSuit[3] = 1;
		nextSuit[1] = HoldemConstants::NO_SUIT;
	}

	if ( dealtHand[f1] > dealtHand[f2] )
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



	while(f2 != HoldemConstants::NO_SUIT && f1 != HoldemConstants::NO_SUIT)
	{

		if (dealtHand[f1] > dealtHand[f2] )
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

	prevSuit[firstSuit] = -1;

	curSuit = firstSuit;




	while(nextSuit[curSuit] != HoldemConstants::NO_SUIT)
	{
		prevSuit[nextSuit[curSuit]] = curSuit;
		curSuit=nextSuit[curSuit];
	}
}


const void OrderedDeck::UndealCard(const DeckLocation& deck)
{
	dealtHand[deck.Suit] &= ~deck.Value;
	dealt = deck;

}



float64 OrderedDeck::DealCard(Hand& h) //(int maxSuit, unsigned long maxCard)
//startSuit=0 and startValue=ACELOW will deal all cards
{
	bool bMatchesOld = false;

	++(dealt.Rank);
	dealt.Value <<= 1;

#ifdef DEBUGDEAL
cout << " dealing@" << flush;
HoldemUtil::PrintCard(dealtSuit,dealtValue);
#endif

	if(dealt.Suit == HoldemConstants::NO_SUIT)
	{//no more cards. That's it for this configuration entirely.

	#ifdef DEBUGDEAL
	cout << " unavailable" << endl;
	#endif
		SetIndependant();
		return 0;
	}
	else if (HoldemConstants::CARD_MISC == dealt.Value)
	{

#ifdef DEBUGDEAL
   cout << " next suit" << flush;
#endif
		SetNextSuit();
		return DealCard(h);

	}
	else if ( (dealtHand[dealt.Suit] & dealt.Value) != 0 )
	{//card already dealt, or needs to be skipped

#ifdef DEBUGDEAL
       cout << " (already dealt)" << flush;
#endif
		return DealCard(h);
	}
	else if(prevSuit[dealt.Suit] != HoldemConstants::NO_SUIT)
	{
		int8 qprevSuit = prevSuit[dealt.Suit];
		uint32 hHere=h.SeeCards(dealt.Suit);
		uint32 hBack=h.SeeCards(qprevSuit);

		//bMatchesOld = ( dealtHand[dealtSuit] & (~dealtHand[qprevSuit]) )
							 //>= dealtHand[qprevSuit];

		/******************
		*ANY PROBLEMS?!?!? uncomment.
		*******************/
		bMatchesOld = dealtHand[dealt.Suit] >= dealtHand[qprevSuit];




#ifdef DEBUGDEAL
       {
		   if(bMatchesOld){cout << " AddToSame" << flush;}
		else{ cout << " AddToDifferent" << flush;}
	   }
#endif
		if(
		(
			(
				 ( (hHere | dealt.Value) > hBack) //violates "greater first" rule
					//||
					//(hHere == hBack)
				 )
				 &&
				 (
					 (dealtHand[qprevSuit]&~hBack)
					 == //if eligible for "greater first" rule
					 (dealtHand[dealt.Suit]&~hHere)
				 )
			)
			||
			(bMatchesOld && hBack==hHere)
		  )
		{

			SetNextSuit();
			return DealCard(h);
		}
	}


	//successful!
	float64 occBase = 0;

	uint32 baseInto = dealtHand[dealt.Suit]; ///This is prior to {dealtHand[dealt.Suit] |= dealt.Value;}
	if (bMatchesOld)
	{
		occBase=1;
	}
	else
	{
		/*if (dealtHand[0] == baseInto) ++occBase;
		if (dealtHand[1] == baseInto) ++occBase;
		if (dealtHand[2] == baseInto) ++occBase;
		if (dealtHand[3] == baseInto) ++occBase;*/
		for( int8 i=dealt.Suit;i!=HoldemConstants::NO_SUIT;i = nextSuit[i])
		{

			if (dealtHand[i] == baseInto)
			{
				++occBase;
			}
			else
				break;
		}
	}

	dealtHand[dealt.Suit] |= dealt.Value;


	h.AddToHand(dealt);

	uint32 dealtTo = dealtHand[dealt.Suit];
	uint32 addedTo = h.SeeCards(dealt.Suit);


	float64 matchesNew = 0; //new duplicate suits formed.
	//int8 occHand = 1;
	//int8 withoutHC;
	for(int8 i=0;i<4;++i)
	{
		if (dealtHand[i] == dealtTo &&
				  h.SeeCards(i) == addedTo) ++matchesNew;
	}


#ifdef DEBUGDEAL
	cout << " [OK]"<< occBase/matchesNew <<endl;
#endif

	//return occBase*24/matchesNew;
	return occBase/matchesNew;
}


const void OrderedDeck::OmitCards(const Hand& h)
{
	OrderedDeck::dealtHand[0] |= h.SeeCards(0);
	OrderedDeck::dealtHand[1] |= h.SeeCards(1);
	OrderedDeck::dealtHand[2] |= h.SeeCards(2);
	OrderedDeck::dealtHand[3] |= h.SeeCards(3);
}

const void OrderedDeck::SetIndependant()
{
	dealt.Value = BaseDealtValue();
	dealt.Suit = BaseDealtSuit();
	dealt.Rank = BaseDealtRank();
}

const void OrderedDeck::SetNextSuit()
{
	dealt.Suit = nextSuit[dealt.Suit];
	dealt.Value = BaseDealtValue();
	dealt.Rank = BaseDealtRank();
}

const void OrderedDeck::UndealAll()
{
	OrderedDeck::dealtHand[0] = 0;
	OrderedDeck::dealtHand[1] = 0;
	OrderedDeck::dealtHand[2] = 0;
	OrderedDeck::dealtHand[3] = 0;
	OrderedDeck::SetIndependant();
}

OrderedDeck::~OrderedDeck()
{
}

DealRemainder::~DealRemainder()
{
	CleanStats();
}
const void DealRemainder::CleanStats()
{
	/*
    if (lastStats != NULL)
	{
		delete lastStats;
	}
	*/
	lastStats = NULL;
}

float64 DealRemainder::Analyze(PlayStats* i,
			const int8 dsuit, const uint8 drank, const uint32 dvalue)
{
	DeckLocation pos;
	pos.Rank = drank;
	pos.Value = dvalue;
	pos.Suit = dsuit;
	return Analyze(i, pos);
}

float64 DealRemainder::AnalyzeComplete(PlayStats* i)
{
	sortSuits();
	DeckLocation pos;
	pos.Rank = BaseDealtRank();
	pos.Value = BaseDealtValue();
	pos.Suit = BaseDealtSuit();
	return Analyze(i, pos);
}

float64 DealRemainder::Analyze(PlayStats* instructions,
								const DeckLocation& pos)
{

	dealt = pos;
	return Analyze(instructions);
}

float64 DealRemainder::Analyze(PlayStats* instructions)
{

	moreCards = instructions->moreCards;
//	deals=0;

	CleanStats();
	lastStats = instructions;
	float64 returnResult;
//	if ( bRecursive )
//		returnResult = executeRecursive(dealtSuit,dealtValue,1);
//	else
		returnResult = executeIterative();

	lastStats->Analyze();

	return returnResult;
}


///TODO: Can we efficientize the iterativeness of this loop?
///TODO: Remove totalruns? (no need to return something)
float64 DealRemainder::executeIterative()
{

	if( moreCards == 0 )
	{
		lastStats->Compare(1);
		return 1;
	}

	//int depth = moreCards;
	int8 curDepth = 0;
	int8 maxDepth = moreCards;
	char *execState = new char[maxDepth];
	execState[curDepth] = 'B';
	/*
		Possible states:
		'B'efore recursive call spot
		'F'ollowing recursive call spot
	*/

	DeckLocation *lastDealt = new DeckLocation[maxDepth];

	float64 *fromRuns = new float64[maxDepth];
	float64 *totalRuns = new float64[maxDepth+1];
	StatRequest *r = new StatRequest[maxDepth];
	float64 *dOcc = new float64[maxDepth];
	Hand *justDealt = new Hand[maxDepth];

	int8 *storeFirstSuit = new int8[maxDepth];
	int8 *(storeNextSuit[4]);
	int8 *(storePrevSuit[4]);
	for( int8 k=0;k<4;++k)
	{
		storeNextSuit[k] = new int8[maxDepth];
		storePrevSuit[k] = new int8[maxDepth];
	}




	lastDealt[0] = dealt;
	fromRuns[0] = 1;
	totalRuns[0] = 0;
	justDealt[0].Empty();

		#ifdef DEBUGORDER
		bool bDebugVerbose = false;
		long int debugCount[maxDepth];
		debugCount[0] = 0;
		#endif
		#ifdef DEBUGCALLPART
		bool bDebugVerbose = false;
		#endif

	while( curDepth >= 0 )
	{


		if( execState[curDepth] == 'B' )
		{


			dOcc[curDepth] = this->DealCard(justDealt[curDepth])*fromRuns[curDepth];

				#ifdef DEBUGORDER
				++debugCount[curDepth];
				if( debugCount[0] >= 40 )
				{
					bDebugVerbose = true;
					cout << lastStats->statGroup << "\t" << flush;
					for(int8 i=0;i<=curDepth;++i)
					{
						cout << debugCount[i] << " " << flush;
					}
					cout << "\t[" << dOcc[curDepth] << "]" << endl;
				}
				#endif




			if (this->dealt.Value != HoldemConstants::CARD_ACELOW && dOcc[curDepth] > 0)
			{
				lastDealt[curDepth] = this->dealt;

				moreCards--;

					/*#ifdef DEBUGCALLPART
						if( curDepth == maxDepth - 2 || bDebugVerbose)
						{
							cout << "=============At this stage " << dOcc[curDepth] << endl;
							if(lastStats->debugViewD(0) != 0 && dOcc[curDepth] == 2)
							{
								//bDebugVerbose = true;
								cout << "\t\t\t\ttrace." << endl;
							}
						}
					#endif*/

				r[curDepth] = lastStats->NewCard(lastDealt[curDepth],dOcc[curDepth]);


				int8 thisDepth = curDepth;
				++curDepth;

				execState[thisDepth] = 'F';



				if ( curDepth == maxDepth )
				{
						#ifdef DEBUGCALLPART
							if(lastStats->debugViewD(0) == 0)
							{
								if( lastDealt[0].Rank == 1 && lastDealt[1].Rank == 1 )
								{
									if(r[thisDepth].bTareOcc){cout << endl << "dOcc = 1";}
									else{cout << endl << "dOcc = " << dOcc[curDepth-1];}

									lastStats->debugPrint();
								}
							}
							else if (	(*(lastStats->debugViewD(0))).Rank == 1 &&
										(*(lastStats->debugViewD(1))).Rank == 1 )
							{
								if(r[thisDepth].bTareOcc){cout << endl << "dOcc = 1";}
								else{cout << endl << "dOcc = " << dOcc[curDepth-1];}
								lastStats->debugPrint();
							}
						#endif
					execState[thisDepth] = 'B';
					--curDepth;
					if(r[thisDepth].bTareOcc)
					{
						lastStats->Compare(1);
					}
					else
					{
						lastStats->Compare( dOcc[curDepth] );
					}
					totalRuns[curDepth] += dOcc[curDepth];



					justDealt[curDepth].RemoveFromHand(lastDealt[curDepth]);
					lastStats->DropCard(lastDealt[curDepth]);

					this->UndealCard(lastDealt[curDepth]);


					moreCards++;


				}
				else
				{
					totalRuns[curDepth] = 0;
					execState[curDepth] = 'B';

					if (r[thisDepth].bNewHand)
					{
						//new hand deal


						storeFirstSuit[thisDepth] = firstSuit;

						storePrevSuit[0][thisDepth] = prevSuit[0];
						storePrevSuit[1][thisDepth] = prevSuit[1];
						storePrevSuit[2][thisDepth] = prevSuit[2];
						storePrevSuit[3][thisDepth] = prevSuit[3];

						storeNextSuit[0][thisDepth] = nextSuit[0];
						storeNextSuit[1][thisDepth] = nextSuit[1];
						storeNextSuit[2][thisDepth] = nextSuit[2];
						storeNextSuit[3][thisDepth] = nextSuit[3];


						sortSuits();

						lastDealt[curDepth].Suit = BaseDealtSuit();
						lastDealt[curDepth].Value = BaseDealtValue();
						lastDealt[curDepth].Rank = BaseDealtRank();


					}
					else
					{
						justDealt[curDepth].SetUnique(justDealt[thisDepth]);

						lastDealt[curDepth] = lastDealt[thisDepth];



					}


					this->dealt = lastDealt[curDepth];
						#ifdef DEBUGORDER
						debugCount[curDepth]=0;
						#endif

					if(r[thisDepth].bTareOcc)
					{
							#ifdef DEBUGCALLPART
								cout << "TARE" << endl;
							#endif
						fromRuns[curDepth] = 1;
							//totalRuns+=executeRecursive(BaseDealtSuit(),BaseDealtValue(),1)*dOcc[thisDepth];
					}
					else
					{
						fromRuns[curDepth] = dOcc[thisDepth];
							//totalRuns+=executeRecursive(BaseDealtSuit(),BaseDealtValue(),dOcc[thisDepth]);
					}

				}
			}
			else //No valid card dealt
			{
				--curDepth;
			}
		}
		else if( execState[curDepth] == 'F' )
		{

#ifdef DEBUGTOTALRUNS
cout << "curDepth = " << curDepth << "\t totalRuns is " << totalRuns[curDepth] << endl;
#endif

			if(r[curDepth].bTareOcc)
			{
				totalRuns[curDepth] += totalRuns[curDepth+1]*dOcc[curDepth];
			}
			else
			{
				totalRuns[curDepth] += totalRuns[curDepth+1];
			}

			if (r[curDepth].bNewHand && curDepth != maxDepth - 1)
			{

				firstSuit = storeFirstSuit[curDepth];
				prevSuit[0] = storePrevSuit[0][curDepth];
				prevSuit[1] = storePrevSuit[1][curDepth];
				prevSuit[2] = storePrevSuit[2][curDepth];
				prevSuit[3] = storePrevSuit[3][curDepth];

				nextSuit[0] = storeNextSuit[0][curDepth];
				nextSuit[1] = storeNextSuit[1][curDepth];
				nextSuit[2] = storeNextSuit[2][curDepth];
				nextSuit[3] = storeNextSuit[3][curDepth];
			}
			else
			{

			/*Here we want to make
					justDealt[curDepth] = justDealt[curDepth - 1]
				but I think this already happenned.
				I mean, the justDealt at the next depth would have
				added a card and then removed it, rinse and repeat.
				*/
			}


			justDealt[curDepth].RemoveFromHand(lastDealt[curDepth]);
			lastStats->DropCard(lastDealt[curDepth]);

			this->UndealCard(lastDealt[curDepth]);


			moreCards++;

			execState[curDepth] = 'B';

			///THOSE LINES BELOW ARE BEFORE THE LOOP, MAKE SURE THEY ARE SET PROPERLY
			/*
			totalRuns[curDepth] = 0;

			this->dealtSuit = lastSuit[curDepth];
			this->dealtValue = lastCard[curDepth];
			dOcc[curDepth] = this->DealCard(justDealt[curDepth])*fromRuns[curDepth];
			*/

		}

	}

	delete [] execState;

	delete [] lastDealt;

	delete [] fromRuns;

float64 rval = totalRuns[0];

	delete [] totalRuns;
	delete [] r;
	delete [] dOcc;
	delete [] justDealt;

	delete [] storeFirstSuit;
	for( int8 k=0;k<4;++k)
	{
		delete [] (storeNextSuit[k]);
		delete [] (storePrevSuit[k]);
	}

cout << "Return..." << endl;
	return rval;
}
/*
double DealRemainder::executeRecursive(int lastSuit,
								unsigned long lastCard, double fromRuns)
{
#ifdef DEBUGORDERING
	cout << "\t\t" << flush;
	for(int i = 1;i<moreCards;++i){cout << "." << flush;}
	if (moreCards > 1)
	{
		cout << ".." <<  fromRuns << flush;
	}
#endif

#ifdef DEBUGINFLOOP
cout << "^^-A^^" << flush;
#endif

	if (0 == moreCards)
	{
#ifdef DEBUGINFLOOP
cout << "^^-A1^^" << flush;
#endif
		//c.DisplayHandBig();
//		deals += fromRuns;
		//if(deals % 100000 == 0) printf("\n%ld",deals);
		lastStats->Compare(fromRuns);
		return fromRuns;
	}
	else
	{
		double dOcc; //occurences of that deal
		double totalRuns=0;
		this->dealtSuit = lastSuit;
		this->dealtValue = lastCard;
#ifdef DEBUGINFLOOP
cout << "^^R^^" << flush;
#endif
		dOcc = this->DealCard(addend)*fromRuns;
#ifdef DEBUGINFLOOP
cout << "^^S^^" << flush;
#endif
		while (this->dealtValue != HoldemConstants::CARD_ACELOW && dOcc > 0)
		{
#ifdef DEBUGINFLOOP
cout << "^^C1^^" << flush;
#endif

			lastSuit = this->dealtSuit;
			lastCard = this->dealtValue;

			moreCards--;



#ifdef DEBUGORDER
if(lastStats->statGroup >= 21 && lastStats->statGroup <= 46)
{

bDebug = true;
if(moreCards>=0)
{//20262


cout << lastStats->statGroup << "x" << moreCards << "\t+" << flush;

 HoldemUtil::PrintCard(lastSuit,lastCard);

}




}
else
{


bDebug = false;
}
#endif

#ifdef DEBUGINFLOOP
cout << "^^E*^^" << flush;
#endif


			StatRequest r = lastStats->NewCard(lastSuit,lastCard,dOcc);



			if (r.bNewHand)
			{
#ifdef DEBUGINFLOOP
cout << "^^F1^^" << flush;
#endif

				//new hand deal

				Hand lastAddend;
				lastAddend.AppendUnique(addend);

				int storeFirstSuit = firstSuit;
				int storePrevSuit[4];
				storePrevSuit[0] = prevSuit[0];
				storePrevSuit[1] = prevSuit[1];
				storePrevSuit[2] = prevSuit[2];
				storePrevSuit[3] = prevSuit[3];

				int storeNextSuit[4];
				storeNextSuit[0] = nextSuit[0];
				storeNextSuit[1] = nextSuit[1];
				storeNextSuit[2] = nextSuit[2];
				storeNextSuit[3] = nextSuit[3];

				sortSuits();

				addend.Empty();


				if(r.bTareOcc)
				{
#ifdef DEBUGTARE
	cout << "\tTARE! " << dOcc << "\t" << flush;
#endif
					totalRuns+=executeRecursive(BaseDealtSuit(),BaseDealtValue(),1)*dOcc;
                }
                else
                {
					totalRuns+=executeRecursive(BaseDealtSuit(),BaseDealtValue(),dOcc);
                }


				addend.Empty();
				addend.AppendUnique(lastAddend);

				firstSuit = storeFirstSuit;
				prevSuit[0] = storePrevSuit[0];
				prevSuit[1] = storePrevSuit[1];
				prevSuit[2] = storePrevSuit[2];
				prevSuit[3] = storePrevSuit[3];

				nextSuit[0] = storeNextSuit[0];
				nextSuit[1] = storeNextSuit[1];
				nextSuit[2] = storeNextSuit[2];
				nextSuit[3] = storeNextSuit[3];
			}
			else
			{
#ifdef DEBUGINFLOOP
cout << "^^F2^^" << flush;
#endif

				//basic deal
				if(r.bTareOcc)
				{
#ifdef DEBUGORDER
cout << endl << endl << endl << endl << "NOT LIKELY!" << endl << "NOT LIKELY!" << endl << "NOT LIKELY!" << endl;

#endif
                    //This is not actually likely.
                    //If you bTareOcc, it's mostly because of certain bNewHands
                    totalRuns+=executeRecursive(lastSuit,lastCard,1)*dOcc;
                }
                else
                {
                    totalRuns+=executeRecursive(lastSuit,lastCard,dOcc);
                }
			}

			addend.RemoveFromHand(lastSuit,lastCard);
			lastStats->DropCard(lastSuit,lastCard);

			this->UndealCard(lastSuit,lastCard);

#ifdef DEBUGORDER
if(lastStats->statGroup >= 21 && lastStats->statGroup <= 46)
{
bDebug = true;

				cout << "\t" << deals << "\t" << flush;
				for(int i = 0;i<moreCards;++i){cout << "*" << flush;}
				if(moreCards>=0){cout << "(" << fromRuns << ")" << totalRuns << endl;

					cout << endl << lastStats->statGroup << "x" << moreCards << "\t-";
				 HoldemUtil::PrintCard(lastSuit,lastCard);
				}

}
else
{
bDebug = false;
}
#endif

			dOcc = this->DealCard(addend)*fromRuns;
			moreCards++;

#ifdef DEBUGINFLOOP
cout << "^^B^^" << flush;
#endif

		}

#ifdef DEBUGINFLOOP
cout << "^^C0^^" << flush;
#endif
		//totalRuns = totalRuns*fromRuns;
#ifdef DEBUGORDERING
		cout << "\t\t" << flush;
		for(int i = 0;i<moreCards;++i){cout << "~" << flush;}
		cout << "(" << fromRuns << ")" << totalRuns << endl << endl ;
#endif
		return totalRuns;

	}

}

*/



