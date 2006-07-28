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


//#define DEBUGLEAK
//#define DEBUGGROUPING
//#define DEBUGNEW
//#define DEBUGDROP
//#define DEBUGCALC
//#define DEBUGCOMPARE
//#define DEBUGFINALCALC
#define DEBUGCALLPCT

#define PROGRESSUPDATE
//#define SUPERPROGRESSUPDATE

#include <cmath>
#include "ai.h"
#include "engine.h"

using std::cout;
using std::endl;
using std::sort;

const void PlayStats::Compare(const float64 occ)
{
#ifdef DEBUGCOMPARE
	myStrength.DisplayHandBig();
	cout << "\nvs" << endl;
	oppStrength.DisplayHandBig();
	cout << endl << endl;
#endif
	if ( myStrength.strength > oppStrength.strength )
	{
		//myWins[statGroup].wins += occ/myOcc[statGroup];
		countWin(occ);
	}
	else if (myStrength.strength < oppStrength.strength)
	{
		//myWins[statGroup].loss += occ/myOcc[statGroup];
		countLoss(occ);
	}
	else
	{
		if ( myStrength.valueset > oppStrength.valueset )
		{
			//myWins[statGroup].wins += occ/myOcc[statGroup];
			countWin(occ);
		}
		else if (myStrength.valueset == oppStrength.valueset)
		{
			//myWins[statGroup].splits += (occ/myOcc[statGroup]);
			countSplit(occ);
		}
		else
		{
			//myWins[statGroup].loss += occ/myOcc[statGroup];
			countLoss(occ);
		}
	}

#ifdef DEBUGCALC
float64 ttt = myWins[statGroup].loss+myWins[statGroup].splits+myWins[statGroup].wins;
if( ttt >= myChancesEach - 2 || ttt == 0 )
		cout << endl << "{" << statGroup << "}" << myWins[statGroup].loss << " l + " <<
		myWins[statGroup].splits << " s + " << myWins[statGroup].wins << " w = " <<
		myWins[statGroup].loss+myWins[statGroup].splits+myWins[statGroup].wins
			<< flush;
#endif

}

const void PlayStats::countWin(const float64 occ)
{
	myWins[statGroup].wins += occ;
	//myWins[statGroup].pct += occ;
}
const void PlayStats::countSplit(const float64 occ)
{
	myWins[statGroup].splits += occ;
	//myWins[statGroup].pct += occ/2;
}
const void PlayStats::countLoss(const float64 occ)
{
	myWins[statGroup].loss += occ;
}

const void WinStats::countSplit(const float64 occ)
{
	PlayStats::countSplit(occ);
	myAvg.splits += occ * myWins[statGroup].repeated;// / myTotalChances;

}
const void WinStats::countLoss(const float64 occ)
{
	PlayStats::countLoss(occ);
	myAvg.loss += occ * myWins[statGroup].repeated;// / myTotalChances;
}
const void WinStats::countWin(const float64 occ)
{
	PlayStats::countWin(occ);
	myAvg.wins += occ * myWins[statGroup].repeated;// / myTotalChances;

}

const void WinStats::Analyze()
{


#ifdef DEBUGFINALCALC
	cout << endl << statCount << " with " << myChancesEach << "'s each" << endl;
	for(int32 i=0;i<statCount;i++)
	{
		cout << endl << "{" << i << "}" << myWins[i].loss << " l + "
				<< myWins[i].splits << " s + " << myWins[i].wins << " w = " <<
				myWins[i].loss+myWins[i].splits+myWins[i].wins
				<< "\t�"<< myWins[i].repeated   <<endl;
	}


#endif

	clearDistr();
	myAvg.wins /= myTotalChances;
	myAvg.loss /= myTotalChances;
	myAvg.splits /= myTotalChances;
	myAvg.genPCT();

	myDistrPCT = new DistrShape(myTotalChances, myAvg.pct);
	myDistrWL = new DistrShape(myTotalChances, myAvg.genPeripheral());

	for(int32 i=0;i<statCount;i++)
	{
		StatResult& wr = myWins[i];
		wr.genPCT();
			#ifdef DEBUGFINALCALC
				cout << wr.pct * wr.repeated << "\t (x" << wr.repeated << ")" << endl;
			#endif
		myDistrPCT->AddVal( wr.pct, wr.repeated );
		myDistrWL->AddVal( wr.genPeripheral(), wr.repeated );

#ifdef DEBUGFINALCALC
	//cout << "= " << myPCTStdDev << endl;
//	cout << "W.P.L. = " << myWPLStdDev << endl;
#endif

	}

//	myDistrPCT->Complete();
 //   	myDistrWL->Complete();
    myDistrPCT->Complete(myChancesEach);
    myDistrWL->Complete(myChancesEach);





//	myWPLStdDev /= myTotalChances;
//	myWPLStdDev = sqrt(myWPLStdDev);
//	myImproveChance /= 2;
//That line needs to be uncommented (I forgot TO comment it when removing WPL)




#ifdef DEBUGFINALCALC

	cout << endl << "AVG " << myAvg.loss << " l + "
				<< myAvg.splits << " s + " << myAvg.wins << " w = " <<
				myAvg.loss+myAvg.splits+myAvg.wins << "\t�"<< myAvg.repeated <<endl;

    cout << "myAvg.genPCT " << myAvg.pct << "!"  << endl;
    cout << "(Mean) " << myDistrPCT->mean * 100 << "%"  << endl;
	cout << endl << "Adjusted improve? " << myDistrPCT->improve * 100 << "%"  << endl;
	cout << "Worst:" << myDistrPCT->worst *100 << "%" << endl;
	cout << "Standard Deviations:" << myDistrPCT->stdDev*100 << "%" << endl;
	cout << "Average Absolute Fluctuation:" << myDistrPCT->avgDev*100 << "%" << endl;
	cout << "Skew:" << myDistrPCT->skew*100 << "%" << endl;
	cout << "Kurtosis:" << (myDistrPCT->kurtosis)*100 << "%" << endl;
	//cout << "W.P.L. = " << myWPLStdDev << endl;
#endif

}

const DistrShape& WinStats::pctDistr()
{
    if( myDistrPCT == NULL ) myDistrPCT = new DistrShape(0);
    return *myDistrPCT;
}

const DistrShape& WinStats::wlDistr()
{
    if( myDistrWL == NULL ) myDistrWL = new DistrShape(0);
    return *myDistrWL;
}

const StatResult& WinStats::avgStat()
{
    return myAvg;
}

//Returns whether or not you need to reset addend
StatRequest WinStats::NewCard(const DeckLocation deck, float64 occ)
{
    StatRequest r;
	++currentCard;

		#ifdef SUPERPROGRESSUPDATE
		if( statGroup > 10)
		{
			cout << (currentCard % 10) << " -- " << statGroup << "\n" << flush;
		//}
		//else
		//{
		//	cout << "W\r" << flush;
		}
		#endif

		#ifdef DEBUGNEW
			cout << endl << "adding brings to " << currentCard << endl;
			cout << endl << "cardsToNextBet = " << cardsToNextBet << "\t" << flush;
			cout << endl << "cardsLeft = " << moreCards - currentCard + 1 << endl;
		#endif

	if (currentCard <= cardsToNextBet )
		//That is, we're dealing deal out the first batch (counting the upcoming card as dealt)
	{
		myUndo[currentCard-1].SetUnique(myStrength);
		oppUndo[currentCard-1].SetUnique(oppStrength);
		myStrength.AddToHand(deck);
		oppStrength.AddToHand(deck);
		if (currentCard == cardsToNextBet)
		{
			//SPECIAL CASE: Dealing river
			if (moreCards == 3)
			{
				myStrength.evaluateStrength();
			}

			//Complete batch
			++statGroup;

				#ifdef PROGRESSUPDATE
				if (statGroup == 0 ) cout << endl << endl;
					cout << "\rW: " << statGroup << "/" << statCount << "  \b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\r" << flush;
				#endif

				#ifdef DEBUGLEAK
				if(statGroup >= statCount)
				{
					cout << endl<<endl << "{"<< statGroup << "}1x2x3x4x5xstatGroup >= statCount"
							<< endl << endl;
					statGroup = 0;
					return r;
				}
				else
				{
					cout << endl << "statGroup = " << statGroup << endl;
				}
				#endif


			myWins[statGroup].repeated = occ;
			if (occ > 1)
			{
				statCount -= static_cast<int32>(occ) - 1;
			}


			r.bTareOcc = true;
			r.bNewHand = true;
			return r;
		}
		else
		{
			//Mid-batch (probably only if mid-flop)
            r.bTareOcc = false;
			r.bNewHand = false;
			return r;
		}

	}
	else ///we're within the statGroup
	{
		short cardsLeft = moreCards - currentCard + 1;
		//don't count the upcoming card as dealt
		oppUndo[currentCard-1].SetUnique(oppStrength);
		oppStrength.AddToHand(deck);

#ifdef DEBUGNEW
	HandPlus u;
	u.FillShowHand(oppHand.cardset,false);cout << endl;
#endif

		if( cardsLeft >= 3 )
		{
			myUndo[currentCard-1].SetUnique(myStrength);
			myStrength.AddToHand(deck);
			if (cardsLeft == 3) //Complete community
			{

				myStrength.evaluateStrength();
#ifdef DEBUGNEW
				myStrength.DisplayHandBig();
#endif
				//r.bTareOcc = false;
                r.bNewHand = true;
                //return r;
			}
			/*else if (cardsLeft == 1)
			{
				oppStrength.evaluateStrength();
			}*/
            //r.bTareOcc = false;
			//r.bNewHand = false;
			return r;
		}
		else //only add to oppHand
		{
			if( cardsLeft == 1 )
			{

				oppStrength.evaluateStrength();
			}
            //r.bTareOcc = false;
            //r.bNewHand = false;
            return r;
        }
	}
}

const void WinStats::DropCard(const DeckLocation deck)
{


		#ifdef DEBUGDROP
			HandPlus u;
			u.FillShowHand(oppHand.cardset,false);cout << endl;
			u.FillShowHand(myHand.cardset,false);cout << endl;
		#endif
	///TODO: Confirms...
	//oppStrength.RemoveFromHand(deck);
	oppStrength.SetUnique(oppUndo[currentCard-1]);


	short cardsLeft = moreCards - currentCard;
	if (cardsLeft >= 2)
	{
		///TODO: Confirms
		myStrength.SetUnique(myUndo[currentCard-1]);
		//myStrength.RemoveFromHand(deck);
	}


	--currentCard;

		#ifdef DEBUGDROP
			u.FillShowHand(oppHand.cardset,false);cout << endl;
			u.FillShowHand(myHand.cardset,false);cout << "---" << cardsLeft << endl;
		#endif




}

//         |statCount-|
//                    |ChancesEach-----------
//community|before bet|fillCom|His Hole Cards

const void WinStats::initW(const int8 cardsInCommunity)
{

		#ifdef DEBUGASSERT
			int temp1 = oppStrength.CardsInSuit( 0 ) +
				oppStrength.CardsInSuit( 1 ) +
				oppStrength.CardsInSuit( 2 ) +
				oppStrength.CardsInSuit( 3 ) ;
			if( cardsInCommunity != temp1 )
			{
				cout << "MISDEAL COMMUNITY PARAMETERS! WATCH IT." << endl;
				return;
			}
				temp1 = myStrength.CardsInSuit( 0 ) +
				myStrength.CardsInSuit( 1 ) +
				myStrength.CardsInSuit( 2 ) +
				myStrength.CardsInSuit( 3 )  -temp1;
			if( 2 !=temp1 )
			{
				cout << "MISDEAL ! WATCH " << temp1 << " cards reported in hand" << endl;
				cout << "COMMUNITY HAS " << cardsInCommunity << endl;
				return;
			}
		#endif

	PlayStats::moreCards = (5-cardsInCommunity+2);

	myUndo = new CommunityPlus[moreCards-2];
	oppUndo = new CommunityPlus[moreCards];

	if( moreCards == 2)
	{
		cardsToNextBet = 0;
			//No more community cards
	}
	else if ( moreCards < 5 )
	{
		cardsToNextBet = 1;
			//It was the turn or river to start with
	}
	else if ( moreCards == 7)
	{
		cardsToNextBet = 3;
			//It's pre-flop
	}
	else
	{
		cardsToNextBet = -1;
			//ERROR! What the hell. Either moreCards is 6 or 5 and that's
			//impossible 'cause you start mid-flop, or else moreCards is more than 7
			//in which case you don't have your cards yet.

			//Note - Assumption: Hold'em
	}


	//SPECIAL CASE:
	if( moreCards == 2 )
	{
		myStrength.evaluateStrength();
		myWins = new StatResult[1];

#ifdef DEBUGLEAK
		cout << "ALLOCATED 1" << endl;
#endif
		myWins[0].repeated = 1;
		statGroup = 0;
		myTotalChances = 1;
		statCount = 1;
		myChancesEach = 990;
	}
	else
	{

		int8 cardsDealt = cardsInCommunity + 2;

		statCount = HoldemUtil::nchoosep<int32>(52-cardsDealt,cardsToNextBet);
		myTotalChances = static_cast<float64>(statCount);

		myWins = new StatResult[statCount];


#ifdef DEBUGLEAK
		cout << "ALLOCATED " << statCount << endl;
#endif

		cardsDealt += cardsToNextBet;
		//moreCards is the number of cards to be dealt to table + opponent
		myChancesEach = HoldemUtil::nchoosep<float64>(52 - cardsDealt,moreCards-cardsToNextBet-2);
		cardsDealt += moreCards-cardsToNextBet-2;
		myChancesEach *= HoldemUtil::nchoosep<float64>(52 - cardsDealt,2);
	}


}


const void CallStats::Analyze()
{
/*
#ifdef DEBUGFINALCALC
	cout << endl << statCount << " with " << myChancesEach << "'s each" << endl;
	for(int32 i=0;i<statCount;i++)
	{
		cout << endl << "{" << i << "}" << myWins[i].loss << " l + "
				<< myWins[i].splits << " s + " << myWins[i].wins << " w = " <<
				myWins[i].loss+myWins[i].splits+myWins[i].wins
				<< "\t�"<< myWins[i].repeated <<flush;
	}
#endif
*/
	for(int32 k=0;k<statCount;++k)
	{
		myWins[k].genPCT();
	}


	sort(myWins,myWins+statCount);//Ascending by default
                                //  http://www.ddj.com/dept/cpp/184403792
#ifdef DEBUGCALLPCT
	cout << endl << "=============SORT!=============" << endl;
	for(int32 i=0;i<statCount;i++)
	{
		cout << endl << "{" << i << "}" << myWins[i].loss << " l + "
				<< myWins[i].splits << " s + " << myWins[i].wins << " w = " <<
				myWins[i].loss+myWins[i].splits+myWins[i].wins
				<< "\t�"<< myWins[i].repeated <<flush;
	}
#endif

//populate cumulation
	int32 i=1;

	vector<StatResult>& cpop = calc->cumulation;
	cpop.reserve(statCount);

    float64 cumulate = myWins[0].repeated;

	cpop.push_back(myWins[0]);
	size_t vectorLast = cpop.size()-1; //(zero)
	int lastStreak=1;

	while(i < statCount)
	{

		cumulate += myWins[i].repeated ;

        StatResult &b = cpop.back();

		//if (!( myWins[i].bIdenticalTo( myWins[i-1]) ))
		if( myWins[i].pct != myWins[i-1].pct )
		{	//add new StatResult
            b.wins /= lastStreak;
            b.splits /= lastStreak;
            b.loss /= lastStreak;
			cpop.push_back(myWins[i]);
			++vectorLast;
			lastStreak = 1;
		}else
        {
            b.wins += myWins[i].wins;
            b.splits += myWins[i].splits;
            b.loss += myWins[i].loss;
            ++lastStreak;
		}
		//cumulation[vectorLast].repeated = cumulate;
		cpop.back().repeated = cumulate;
		++i;
	}
    StatResult &b = cpop.back();
    b.wins /= lastStreak;
    b.splits /= lastStreak;
    b.loss /= lastStreak;


	for(size_t k=0;k<=vectorLast;++k)
	{
	    StatResult& temptarget = calc->cumulation[k];
		temptarget.wins /= myChancesEach;
		temptarget.loss /= myChancesEach;
		temptarget.splits /= myChancesEach;
		temptarget.pct = 1 - temptarget.pct/myChancesEach;
		temptarget.repeated /= myTotalChances;
	}


#ifdef DEBUGCALLPCT
cout << endl << "=============Reduced=============" << endl;
	cout.precision(4);
	for(size_t i=0;i<=vectorLast;i++)
	{
		cout << endl << "{" << i << "}" << calc->cumulation[i].loss << " l +\t"
				<< calc->cumulation[i].splits << " s +\t" << calc->cumulation[i].wins << " w =\t" <<
				calc->cumulation[i].pct
				<< " pct\t�"<< calc->cumulation[i].repeated <<flush;
	}
	cout << endl;
#endif

//How many of them would call a bet of x?
//It's the number of myWins elements that have a PCT above the pot odds implied by x.

}

float64 CallStats::pctWillCall(const float64 oddsFaced) const
{
	return calc->pctWillCall(oddsFaced);
}

const void CallStats::DropCard(const DeckLocation deck)
{
	///TODO: Confirm
	//oppStrength.RemoveFromHand(deck);
	oppStrength.SetUnique(oppUndo[currentCard-1]);

	if (currentCard > 2)
	{
		///TODO: Confirm...
		myStrength.SetUnique(myUndo[currentCard-3]);
		//myStrength.RemoveFromHand(deck);
	}



	--currentCard;

}

StatRequest CallStats::NewCard(const DeckLocation deck, float64 occ)
{


	StatRequest r;
	++currentCard;

		#ifdef SUPERPROGRESSUPDATE
		if( currentCard < moreCards )
		{
			cout << (currentCard % 10) << "\r" << flush;
		//}
		//else
		//{
		//	cout << "c\r" << flush;
		}
		#endif


	oppUndo[currentCard-1].SetUnique(oppStrength);
	oppStrength.AddToHand(deck);

	if ( currentCard > 2 )
	{
		myUndo[currentCard-3].SetUnique(myStrength);
		//past dealing opp hand
		myStrength.AddToHand(deck);
		if ( currentCard == moreCards )
		{
			myStrength.evaluateStrength();
			oppStrength.evaluateStrength();
		}
	}
	else if ( currentCard == 2 )
	{

		if( moreCards == 2) oppStrength.evaluateStrength();

		++statGroup;

#ifdef PROGRESSUPDATE
	if (statGroup == 0 ) cout << endl << endl;
	cout << "C: " << statGroup << "/" << statCount << "  \b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\r" << flush;

#endif

		r.bTareOcc = true;
		r.bNewHand = true;

		myWins[statGroup].repeated = occ;

		if(occ > 1)
		{
			statCount -= static_cast<int32>(occ) - 1;
		}

	}

	return r;

}

//         |statCount ----|
//                        |ChancesEach---
//community|His Hole Cards|To 5 community


const void CallStats::initC(const int8 cardsInCommunity)
{
	calc = new CallCumulation();

	moreCards = 7-cardsInCommunity;

	myUndo = new CommunityPlus[moreCards-2];
	oppUndo = new CommunityPlus[moreCards];

    int8 cardsAvail = 52 - cardsInCommunity - 2;

    int32 oppHands = cardsAvail*(cardsAvail-1)/2;
    myTotalChances = static_cast<float64>(oppHands);
	statCount = oppHands;

    myWins = new StatResult[oppHands];

    myChancesEach = HoldemUtil::nchoosep<float64>(cardsAvail - 2,5-cardsInCommunity);

	if (moreCards == 2)
	{
		myStrength.evaluateStrength();
	}
}

void WinStats::clearDistr()
{
	if( myDistrPCT != NULL ) delete myDistrPCT;
	if( myDistrWL != NULL ) delete myDistrWL;
}

WinStats::~WinStats()
{
	clearDistr();
	delete [] myUndo;
	delete [] oppUndo;
}

CallStats::~CallStats()
{
	delete calc;
	delete [] myUndo;
	delete [] oppUndo;
}

PlayStats::~PlayStats()
{
    if (myWins != NULL) delete [] myWins;
    myWins = NULL;
}





/*	int32 i=statCount-1;
	float64 cumulate = myWins[i].repeated/myTotalChances;

	myWins[i].wins /= myChancesEach;
	myWins[i].loss /= myChancesEach;
	myWins[i].splits /= myChancesEach;
	myWins[i].pct /= myChancesEach;

	cumulation.reserve(statCount);

	cumulation.push_back(myWins[i]);
	size_t vectorLast = cumulation.size()-1;

	while(i > 0)
	{

		--i;
		myWins[i].wins /= myChancesEach;
		myWins[i].loss /= myChancesEach;
		myWins[i].splits /= myChancesEach;
		myWins[i].pct /= myChancesEach;
		cumulate += myWins[i].repeated/myTotalChances;
		if (!( myWins[i].bIdenticalTo( myWins[i+1]) ))
		{	//add new StatResult
			cumulation.push_back(myWins[i]);
			++vectorLast;
		}
		//cumulation[vectorLa`st].repeated = cumulate;
		cumulation.back().repeated = cumulate;

	}
*/
