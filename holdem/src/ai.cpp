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



#include <algorithm>
#include <math.h>
#include "ai.h"

using std::endl;
using std::sort;

/*
#ifdef DUMP_CSV_PLOTS
void CallStats::dump_csv_plot(const char * dump_filename)
{

}
#endif
*/

#ifdef DEBUGASSERT
  [[noreturn]] void hard_exit() {
    std::exit(1);
    __builtin_unreachable();
  }
#endif

void PlayStats::Compare(const float64 occ)
{

	if ( myStrength.strength > oppStrength.strength )
	{
		countWin(occ);
	}
	else if (myStrength.strength < oppStrength.strength)
	{
		countLoss(occ);
	}
	else
	{
		if ( myStrength.valueset > oppStrength.valueset )
		{
			countWin(occ);
		}
		else if (myStrength.valueset == oppStrength.valueset)
		{
			countSplit(occ);
		}
		else
		{
			countLoss(occ);
		}
	}

}

void PlayStats::countWin(const float64 occ)
{
	myWins[statGroup].wins += occ;
}
void PlayStats::countSplit(const float64 occ)
{
	myWins[statGroup].splits += occ;
}
void PlayStats::countLoss(const float64 occ)
{
	myWins[statGroup].loss += occ;
}

void WinStats::countSplit(const float64 occ)
{
    float64 dOccRep = oppReps * occ;
	PlayStats::countSplit(dOccRep);
	myAvg.splits += dOccRep * myWins[statGroup].repeated;

}
void WinStats::countLoss(const float64 occ)
{
    float64 dOccRep = oppReps * occ;
	PlayStats::countLoss(dOccRep);
	myAvg.loss += dOccRep * myWins[statGroup].repeated;
}
void WinStats::countWin(const float64 occ)
{
    float64 dOccRep = oppReps * occ;
	PlayStats::countWin(dOccRep);
	myAvg.wins += dOccRep * myWins[statGroup].repeated;

}

// [!NOTE]
// this->myDistr is the part that ultimately gets saved during `StatsManager::SerializeW`
void WinStats::Analyze()
{

	clearDistr();
	myAvg.wins /= myTotalChances;
	myAvg.loss /= myTotalChances;
	myAvg.splits /= myTotalChances;
	myAvg.genPCT();

    myWins[0].genPCT();
    StatResult best = myWins[0];
    StatResult worst = myWins[0];
    for(int32 i=0;i<statCount;i++)
	{
		StatResult& wr = myWins[i];
		wr.genPCT();

        if (best < wr) { best = wr; }
        if (wr < worst) { worst = wr; }

	}

  // [!NOTE]
  // The constructor initializes myDistr->coarseHistogram[...] to all zeroes
  // See for yourself at src/inferentials.cpp:DistrShape::DistrShape
	myDistr = new DistrShape(myTotalChances, worst, myAvg, best);

	for(int32 i=0;i<statCount;i++)
	{
		const StatResult& wr = myWins[i];
		myDistr->AddVal( wr );

	}

    myDistr->Complete(myChancesEach);
}

const DistrShape& WinStats::getDistr()
{
    if( myDistr == NULL ) myDistr = new DistrShape(DistrShape::newEmptyDistrShape());
    return *myDistr;
}
const StatResult& WinStats::avgStat() const
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
			std::cout << (currentCard % 10) << " -- " << statGroup << "\n" << flush;
		//}
		//else
		//{
		//	cout << "W\r" << flush;
		}
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
			  	if ((statGroup % PROGRESSUPDATE) == 0) {
                if (statGroup == 0 ) std::cerr << endl << endl;
                std::cerr << "\rW: " << statGroup << "/" << statCount << "  \b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\r" << flush;
					}
				#endif


			myWins[statGroup].repeated = occ;
			if (occ > 1)
			{
				statCount -= static_cast<int32>(occ) - 1;
			}


			r.bNewSet = true;
			return r;
		}
		else
		{
			//Mid-batch (probably only if mid-flop)
            //r.bNewSet = false; //Valid and unncessary -- constructor defaults to false
			return r;
		}

	}
	else ///we're within the statGroup
	{
		short cardsLeft = moreCards - currentCard + 1;
		//don't count the upcoming card as dealt
		oppUndo[currentCard-1].SetUnique(oppStrength);
		oppStrength.AddToHand(deck);


		if( cardsLeft >= 3 )
		{
			myUndo[currentCard-1].SetUnique(myStrength);
			myStrength.AddToHand(deck);
			if (cardsLeft == 3) //Complete community (three cards were left: the river, and the opponent's two)
			{
                //You just dealt the last community card
				myStrength.evaluateStrength();

                oppReps = occ;
                r.bNewSet = true;
			}

			return r;
		}
		else //only add to oppHand
		{
			if( cardsLeft == 1 )
			{//You just dealt the last card

				oppStrength.evaluateStrength();
			}
            return r;
        }
	}
}

void WinStats::DropCard(const DeckLocation deck)
{

	oppStrength.SetUnique(oppUndo[currentCard-1]);


	short cardsLeft = moreCards - currentCard;
	if (cardsLeft >= 2)
	{
		myStrength.SetUnique(myUndo[currentCard-1]);
	}





	--currentCard;



}

//         |statCount-|
//                    |ChancesEach-----------
//community|before bet|fillCom|His Hole Cards

void WinStats::initW(const int8 cardsInCommunity)
{
  PlayStats::moreCards = (5-cardsInCommunity+2);

    const size_t moreCardsAlloc = moreCards;
		if ((2 <= moreCardsAlloc) && (moreCardsAlloc <= 7)) {
		  myUndo = new CommunityPlus[moreCardsAlloc-2];
		  oppUndo = new CommunityPlus[moreCardsAlloc];
		}
		#ifdef DEBUGASSERT
		else
    {
        std::cerr << "WinStats::initW(" << static_cast<int>(cardsInCommunity) << ") means `cardsInCommunity < 0` or `cardsInCommunity > 5` but that's not possible. The River is the last round and it's 5 community cards." << std::endl;
        exit(1);
    }

			int8 temp1 = oppStrength.CardsInSuit( 0 ) +
				oppStrength.CardsInSuit( 1 ) +
				oppStrength.CardsInSuit( 2 ) +
				oppStrength.CardsInSuit( 3 ) ;
			if( cardsInCommunity != temp1 )
			{
                std::cerr << "MISDEAL COMMUNITY PARAMETERS! WATCH IT." << endl;
				exit(1);
				return;
			}
				temp1 = myStrength.CardsInSuit( 0 ) +
				myStrength.CardsInSuit( 1 ) +
				myStrength.CardsInSuit( 2 ) +
				myStrength.CardsInSuit( 3 )  -temp1;
			if( 2 !=temp1 )
			{
				std::cerr << "MISDEAL!!\nWATCH " << static_cast<int>(temp1) << " cards reported in hand" << endl;
				std::cerr << "COMMUNITY HAS " << (int)(cardsInCommunity) << endl;
				return;
			}
		#endif

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
    oppReps = 1;

	//SPECIAL CASE:
	if( moreCards == 2 )
	{
		myStrength.evaluateStrength();
		myWins = new StatResult[1];


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

		#ifdef DEBUGASSERT
		  // cardsInCommunity inclusive of [0..5]
		  // cardsDealt inclusive of [2..7]
			/*
		   statCount is one of [
				 HoldemUtil::nchoosep<int32>(52-7,0) i.e. 1 (after the River) BUT there is a special case for (moreCards == 2) already above, so...
				 HoldemUtil::nchoosep<int32>(52-6,1) i.e. 46 (right after the Turn)
				 HoldemUtil::nchoosep<int32>(52-5,1) i.e. 47 (right after the Flop)
				 HoldemUtil::nchoosep<int32>(52-2,3) i.e. 19600 (Pre-flop)
				]
			*/
		  if(statCount > 19600) {
				std::cerr << "WinStats::initW(" << static_cast<int>(cardsInCommunity) << ") caused moreCards=" << PlayStats::moreCards
				  << " cardsToNextBet=" << static_cast<int>(cardsToNextBet)
				  << " cardsDealt=" << static_cast<int>(cardsDealt)
					<< " HoldemUtil::nchoosep<int32>(" << static_cast<int>(52-cardsDealt) << "," << static_cast<int>(cardsToNextBet) << ")"
					<< " = " << statCount << std::endl;
				return exit(1);
			}
			if(statCount < 46) {
				std::cerr << "HoldemUtil::nchoosep<int32>(" << static_cast<int>(52-cardsDealt) << "," << static_cast<int>(cardsToNextBet) << ")"
				  << " = " << statCount << " would never return a negative value. And otherwise You should have hit the special case above for (moreCards == 2)" << std::endl
					<< "So then, how did WinStats::initW(" << static_cast<int>(cardsInCommunity) << ") cause "
					<< " cardsToNextBet=" << static_cast<int>(cardsToNextBet)
				  << " cardsDealt=" << static_cast<int>(cardsDealt) << std::endl;
				return exit(1);
			}
		#endif
		myWins = new StatResult[statCount];


		cardsDealt += cardsToNextBet;
		//moreCards is the number of cards to be dealt to table + opponent
		myChancesEach = HoldemUtil::nchoosep<float64>(52 - cardsDealt,moreCards-cardsToNextBet-2);
		cardsDealt += moreCards-cardsToNextBet-2;
		myChancesEach *= HoldemUtil::nchoosep<float64>(52 - cardsDealt,2);
	}


}


void CallStats::Analyze()
{


	for(int32 k=0;k<statCount;++k)
	{
		myWins[k].genPCT();
		#ifdef DEBUGASSERT
          if( myWins[k].loss+myWins[k].splits+myWins[k].wins > myChancesEach + 0.1 || myWins[k].loss+myWins[k].splits+myWins[k].wins < myChancesEach - 0.1  )
          {
              std::cerr << "Failure to generate w+s+l=" << myChancesEach << " with {"<< k <<"}. Instead, w+s+l=" << (myWins[k].loss+myWins[k].splits+myWins[k].wins) << endl;
              exit(1);
          }
	    #endif
	}


	sort(myWins,myWins+statCount);//Ascending by default
                                //  http://www.ddj.com/dept/cpp/184403792


//populate cumulation
	int32 count=1;

	vector<StatResult>& cpop = calc->cumulation;
	cpop.reserve(statCount);

    float64 cumulate = myWins[0].repeated;
    myWins[0].wins *= cumulate;
	myWins[0].splits *= cumulate;
	myWins[0].loss *= cumulate;

	cpop.push_back(myWins[0]);
	size_t vectorLast = cpop.size()-1; //(zero)
	float64 lastStreak=cumulate; //ASSERT: lastStreak == myWins[0].repeated

	while(count < statCount)
	{

		cumulate += myWins[count].repeated ;

        StatResult &b = cpop.back();

		//if (!( myWins[i].bIdenticalTo( myWins[i-1]) ))
		if( myWins[count].pct != myWins[count-1].pct )
		{	//add new StatResult
            b.wins /= lastStreak;
            b.splits /= lastStreak;
            b.loss /= lastStreak;
            //Final set b


#ifdef DEBUGASSERT

            if( lastStreak != b.repeated  -  (vectorLast ? cpop[vectorLast-1].repeated : 0) )
            {
                std::cerr << "Inconsistent lastStreak! " << ( b.repeated  -  cpop[vectorLast-1].repeated) << " were generated, but only " << lastStreak << " were counted." << endl;
                exit(1);
            }
#endif

			//Prep existing
			lastStreak = myWins[count].repeated;
			myWins[count].wins *= lastStreak;
			myWins[count].splits *= lastStreak;
			myWins[count].loss *= lastStreak;


			//Load for cumulation
			cpop.push_back(myWins[count]);
			++vectorLast;
		}else
        {
            b.wins += myWins[count].wins * myWins[count].repeated;
            b.splits += myWins[count].splits * myWins[count].repeated;
            b.loss += myWins[count].loss * myWins[count].repeated;
            lastStreak += myWins[count].repeated;
		}
		//cumulation[vectorLast].repeated = cumulate;
		cpop.back().repeated = cumulate;
		++count;
	}
    StatResult &b = cpop.back();
    b.wins /= lastStreak;
    b.splits /= lastStreak;
    b.loss /= lastStreak;

                               //  http://www.ddj.com/dept/cpp/184403792


#ifdef DEBUGASSERT
    if( cumulate != myTotalChances )
    {
        std::cerr << "Raw count after sorting does not match: " << cumulate << " counted, but " << myTotalChances << " expected." << endl;
        exit(1);
    }

#endif

	for(size_t k=0;k<=vectorLast;++k)
	{

	    StatResult& temptarget = calc->cumulation[k];

	    #ifdef DEBUGASSERT
          if( temptarget.wins + temptarget.splits + temptarget.loss > myChancesEach + 0.1 || temptarget.wins + temptarget.splits + temptarget.loss < myChancesEach - 0.1  )
          {
              std::cerr << "Failure to maintain w+s+l=" << myChancesEach << " after combine step on {"<< k <<"}. Instead, w+s+l=" << (temptarget.wins + temptarget.splits + temptarget.loss) << endl;
              exit(1);
          }
	    #endif

		temptarget.wins /= myChancesEach;
		temptarget.loss /= myChancesEach;
		temptarget.splits /= myChancesEach;
        ///All outcomes so far are myPCT (having the first hand dealt. For CallStats that's the fixed hand I have. For CommunityCallStats that's the pockets named in the distribution.
        temptarget.pct = temptarget.pct/myChancesEach;
		temptarget.repeated /= myTotalChances;
	}


//How many of them would call a bet of x?
//It's the number of myWins elements that have a PCT above the pot odds implied by x.

}



void CallStats::myAddCard(const DeckLocation& cardinfo, const int16 undoIndex)
{
    myUndo[undoIndex].SetUnique(myStrength);
    myStrength.AddToHand(cardinfo);
}

void CallStats::myRevert(const int16 undoIndex)
{
    myStrength.SetUnique(myUndo[undoIndex]);
}

void CallStats::myEval()
{
    myStrength.evaluateStrength();
}

void CallStats::setCurrentGroupOcc(const float64 occ)
{
    myWins[statGroup].repeated = occ;
}

void CallStats::showProgressUpdate() const
{
    if (statGroup == 0 ) std::cerr << endl << endl;
    std::cerr << "C: " << statGroup << "/" << statCount << "  \b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\r" << flush;
}

void CallStats::DropCard(const DeckLocation deck)
{

	oppStrength.SetUnique(oppUndo[currentCard-1]);

	if (currentCard > 2)
	{
		myRevert(currentCard-3);
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
			std::cout << (currentCard % 10) << "\r" << flush;
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
		//past dealing opp hand
		myAddCard(deck,currentCard-3);
		if ( currentCard == moreCards )
		{
			myEval();
			oppStrength.evaluateStrength();
		}
	}
	else
	{

	    mynoAddCard(deck,currentCard-1);
	    if ( currentCard == 2 )
        {


            if( moreCards == 2) oppStrength.evaluateStrength();

            ++statGroup;


            r.bNewSet = true;

            setCurrentGroupOcc(occ);

            if(occ > 1)
            {
                statCount -= static_cast<int32>(occ) - 1;
            }


#ifdef PROGRESSUPDATE
    if ((statGroup % PROGRESSUPDATE) == 0) { showProgressUpdate(); }
#endif

        }

	}

	return r;

}

//         |statCount ----|
//                        |ChancesEach---
//community|His Hole Cards|To 5 community

int8 CallStats::realCardsAvailable(const int8 cardsInCommunity) const
{
    return 52 - cardsInCommunity - 2;
}

void CallStats::initC(const int8 cardsInCommunity)
{
	calc = new CallCumulation();

	const int8 cardsAvail = realCardsAvailable(cardsInCommunity);
		const size_t oppHands = cardsAvail*(cardsAvail-1)/2;

	moreCards = 7-cardsInCommunity;

		const size_t moreCardsAlloc = moreCards;

		if ((2 <= moreCardsAlloc) && (moreCardsAlloc <= 7)) {
				myUndo = new CommunityPlus[moreCardsAlloc-2];
				oppUndo = new CommunityPlus[moreCardsAlloc];
		}
		#ifdef DEBUGASSERT
		else if (moreCards < 2) {
				std::cerr << "How did we get `cardsInCommunity > 5` causing CallStats::initC(" << static_cast<int>(cardsInCommunity) << ")" << std::endl;
				return exit(1);
		} else if (moreCards > 7) {
				std::cerr << "CallStats::initC(" << static_cast<int>(cardsInCommunity) << ") is being called with a negative value??" << std::endl;
				return exit(1);
		} else {
				std::cerr << "There's no way to get here. We guarded on `moreCardsAlloc` in multiple places: " << static_cast<int>(moreCards) << " : " << moreCardsAlloc << std::endl;
				return exit(1);
		}

	if (oppHands > 1225)
					{
									std::cerr << "CallStats::realCardsAvailable(" << static_cast<int>(cardsInCommunity) << ") means " << static_cast<int>(cardsAvail) << " = `cardsAvail > 50`" << std::endl;
									hard_exit();
					}
	#endif

				myTotalChances = static_cast<float64>(oppHands);
	statCount = oppHands;

				myWins = new StatResult[oppHands];


				myChancesEach = HoldemUtil::nchoosep<float64>(cardsAvail - 2,5-cardsInCommunity);

	if (moreCards == 2)
	{
								myEval();
	}
}

void WinStats::clearDistr()
{
	if( myDistr != NULL ) delete myDistr;
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
