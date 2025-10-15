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

void CallStats::Compare(const float64 occ) { SimpleCompare::simple_compare(this, occ); }
void WinStats::Compare(const float64 occ) { SimpleCompare::simple_compare(this, occ); }

template<typename T> void SimpleCompare::simple_compare(T * const play_stats, const float64 occ)
{

	if ( play_stats->winloss_counter.myStrength.strength > play_stats->winloss_counter.oppStrength.strength )
	{
		play_stats->countWin(occ);
	}
	else if (play_stats->winloss_counter.myStrength.strength < play_stats->winloss_counter.oppStrength.strength)
	{
		play_stats->countLoss(occ);
	}
	else
	{
		if ( play_stats->winloss_counter.myStrength.hand_logic.valueset > play_stats->winloss_counter.oppStrength.hand_logic.valueset )
		{
			play_stats->countWin(occ);
		}
		else if (play_stats->winloss_counter.myStrength.hand_logic.valueset == play_stats->winloss_counter.oppStrength.hand_logic.valueset)
		{
			play_stats->countSplit(occ);
		}
		else
		{
			play_stats->countLoss(occ);
		}
	}

}
template void SimpleCompare::simple_compare<CallStats>(CallStats * const, const float64);
template void SimpleCompare::simple_compare<WinStats>(WinStats * const, const float64);

void CallStats::countWin(const float64 occ)
{
	winloss_counter.myWins[winloss_counter.statGroup].wins += occ;
}
void CallStats::countSplit(const float64 occ)
{
	winloss_counter.myWins[winloss_counter.statGroup].splits += occ;
}
void CallStats::countLoss(const float64 occ)
{
	winloss_counter.myWins[winloss_counter.statGroup].loss += occ;
}

void WinStats::countSplit(const float64 occ)
{
    float64 dOccRep = oppReps * occ;
    winloss_counter.myWins[winloss_counter.statGroup].splits += dOccRep; // i.e. `PlayStats::countSplit(dOccRep);`
	myAvg.splits += dOccRep * winloss_counter.myWins[winloss_counter.statGroup].repeated;

}
void WinStats::countLoss(const float64 occ)
{
    float64 dOccRep = oppReps * occ;
    winloss_counter.myWins[winloss_counter.statGroup].loss += dOccRep; // i.e. `PlayStats::countLoss(dOccRep);`
	myAvg.loss += dOccRep * winloss_counter.myWins[winloss_counter.statGroup].repeated;
}
void WinStats::countWin(const float64 occ)
{
    float64 dOccRep = oppReps * occ;
    winloss_counter.myWins[winloss_counter.statGroup].wins += dOccRep; // i.e.  `PlayStats::countWin(dOccRep);`
	myAvg.wins += dOccRep * winloss_counter.myWins[winloss_counter.statGroup].repeated;

}

// [!NOTE]
// this->myDistr is the part that ultimately gets saved during `StatsManager::SerializeW`
void WinStats::Analyze()
{

	clearDistr();
	myAvg.wins /= winloss_counter.myTotalChances;
	myAvg.loss /= winloss_counter.myTotalChances;
	myAvg.splits /= winloss_counter.myTotalChances;
	myAvg.genPCT();

    winloss_counter.myWins[0].genPCT();
    StatResult best = winloss_counter.myWins[0];
    StatResult worst = winloss_counter.myWins[0];
    for(int32 i=0;i<winloss_counter.statCount;i++)
	{
		StatResult& wr = winloss_counter.myWins[i];
		wr.genPCT();

        if (best < wr) { best = wr; }
        if (wr < worst) { worst = wr; }

	}

  // [!NOTE]
  // The constructor initializes myDistr->coarseHistogram[...] to all zeroes
  // See for yourself at src/inferentials.cpp:DistrShape::DistrShape
	myDistr = new DistrShape(winloss_counter.myTotalChances, worst, myAvg, best);

	for(int32 i=0;i<winloss_counter.statCount;i++)
	{
		const StatResult& wr = winloss_counter.myWins[i];
		myDistr->AddVal( wr );

	}

    myDistr->Complete(winloss_counter.myChancesEach);
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
    const int16 undoCurrentCard = winloss_counter.currentCard;
	++(winloss_counter.currentCard);

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


	if (winloss_counter.currentCard <= cardsToNextBet )
		//That is, we're dealing deal out the first batch (counting the upcoming card as dealt)
	{
		myUndo[undoCurrentCard].SetUnique(winloss_counter.myStrength);
		oppUndo[undoCurrentCard].SetUnique(winloss_counter.oppStrength);
		winloss_counter.myStrength.AddToHand(deck);
		winloss_counter.oppStrength.AddToHand(deck);
		if (winloss_counter.currentCard == cardsToNextBet)
		{
			//SPECIAL CASE: Dealing river
			if (winloss_counter.moreCards == 3)
			{
				winloss_counter.myStrength.evaluateStrength();
			}

			//Complete batch
			++(winloss_counter.statGroup);

				#ifdef PROGRESSUPDATE
			  	if ((statGroup % PROGRESSUPDATE) == 0) {
                if (statGroup == 0 ) std::cerr << endl << endl;
                std::cerr << "\rW: " << statGroup << "/" << statCount << "  \b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\r" << flush;
					}
				#endif


			winloss_counter.myWins[winloss_counter.statGroup].repeated = occ;
			if (occ > 1)
			{
				winloss_counter.statCount -= static_cast<int32>(occ) - 1;
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
		short cardsLeft = winloss_counter.moreCards - winloss_counter.currentCard + 1;
		//don't count the upcoming card as dealt
		oppUndo[undoCurrentCard].SetUnique(winloss_counter.oppStrength);
		winloss_counter.oppStrength.AddToHand(deck);


		if( cardsLeft >= 3 )
		{
			myUndo[undoCurrentCard].SetUnique(winloss_counter.myStrength);
			winloss_counter.myStrength.AddToHand(deck);
			if (cardsLeft == 3) //Complete community (three cards were left: the river, and the opponent's two)
			{
                //You just dealt the last community card
				winloss_counter.myStrength.evaluateStrength();

                oppReps = occ;
                r.bNewSet = true;
			}

			return r;
		}
		else //only add to oppHand
		{
			if( cardsLeft == 1 )
			{//You just dealt the last card

				winloss_counter.oppStrength.evaluateStrength();
			}
            return r;
        }
	}
}

void WinStats::DropCard(const DeckLocation deck)
{

	winloss_counter.oppStrength.SetUnique(oppUndo[winloss_counter.currentCard - 1]);


	short cardsLeft = winloss_counter.moreCards - winloss_counter.currentCard;
	if (cardsLeft >= 2)
	{
		winloss_counter.myStrength.SetUnique(myUndo[winloss_counter.currentCard-1]);
	}


	--(winloss_counter.currentCard);

}

//         |statCount-|
//                    |ChancesEach-----------
//community|before bet|fillCom|His Hole Cards

void WinStats::initW(const int8 cardsInCommunity)
{
  winloss_counter.moreCards = (5-cardsInCommunity+2);

    const size_t moreCardsAlloc = winloss_counter.moreCards;
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

			int8 temp1 = winloss_counter.oppStrength.CardsInSuit( 0 ) +
			             winloss_counter.oppStrength.CardsInSuit( 1 ) +
			             winloss_counter.oppStrength.CardsInSuit( 2 ) +
			             winloss_counter.oppStrength.CardsInSuit( 3 );

			if( cardsInCommunity != temp1 )
			{
                std::cerr << "MISDEAL COMMUNITY PARAMETERS! WATCH IT." << endl;
				exit(1);
				return;
			}
				temp1 = winloss_counter.myStrength.CardsInSuit( 0 ) +
				        winloss_counter.myStrength.CardsInSuit( 1 ) +
				        winloss_counter.myStrength.CardsInSuit( 2 ) +
				        winloss_counter.myStrength.CardsInSuit( 3 )  -temp1;

			if( 2 !=temp1 )
			{
				std::cerr << "MISDEAL!!\nWATCH " << static_cast<int>(temp1) << " cards reported in hand" << endl;
				std::cerr << "COMMUNITY HAS " << (int)(cardsInCommunity) << endl;
				return;
			}
		#endif

	if( winloss_counter.moreCards == 2)
	{
		cardsToNextBet = 0;
			//No more community cards
	}
	else if ( winloss_counter.moreCards < 5 )
	{
		cardsToNextBet = 1;
			//It was the turn or river to start with
	}
	else if ( winloss_counter.moreCards == 7)
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
	if( winloss_counter.moreCards == 2 )
	{
		winloss_counter.myStrength.evaluateStrength();
		winloss_counter.myWins = new StatResult[1];


		winloss_counter.myWins[0].repeated = 1;
		winloss_counter.statGroup = 0;
		winloss_counter.myTotalChances = 1;
		winloss_counter.statCount = 1;
		winloss_counter.myChancesEach = 990;
	}
	else
	{

		int8 cardsDealt = cardsInCommunity + 2;

		winloss_counter.statCount = HoldemUtil::nchoosep<int32>(52-cardsDealt,cardsToNextBet);
		winloss_counter.myTotalChances = static_cast<float64>(winloss_counter.statCount);

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
		  if(winloss_counter.statCount > 19600) {
				std::cerr << "WinStats::initW(" << static_cast<int>(cardsInCommunity) << ") caused moreCards=" << winloss_counter.moreCards
				  << " cardsToNextBet=" << static_cast<int>(cardsToNextBet)
				  << " cardsDealt=" << static_cast<int>(cardsDealt)
					<< " HoldemUtil::nchoosep<int32>(" << static_cast<int>(52-cardsDealt) << "," << static_cast<int>(cardsToNextBet) << ")"
					<< " = " << winloss_counter.statCount << std::endl;
				return exit(1);
			}
			if(winloss_counter.statCount < 46) {
				std::cerr << "HoldemUtil::nchoosep<int32>(" << static_cast<int>(52-cardsDealt) << "," << static_cast<int>(cardsToNextBet) << ")"
				  << " = " << winloss_counter.statCount << " would never return a negative value. And otherwise You should have hit the special case above for (moreCards == 2)" << std::endl
					<< "So then, how did WinStats::initW(" << static_cast<int>(cardsInCommunity) << ") cause "
					<< " cardsToNextBet=" << static_cast<int>(cardsToNextBet)
				  << " cardsDealt=" << static_cast<int>(cardsDealt) << std::endl;
				return exit(1);
			}
		#endif
		winloss_counter.myWins = new StatResult[winloss_counter.statCount];


		cardsDealt += cardsToNextBet;
		//moreCards is the number of cards to be dealt to table + opponent
		winloss_counter.myChancesEach = HoldemUtil::nchoosep<float64>(52 - cardsDealt, winloss_counter.moreCards - cardsToNextBet-2);
		cardsDealt += winloss_counter.moreCards - cardsToNextBet - 2;
		winloss_counter.myChancesEach *= HoldemUtil::nchoosep<float64>(52 - cardsDealt,2);
	}


}


void CallStats::Analyze()
{
	for(int32 k=0; k < winloss_counter.statCount ;++k)
	{
		winloss_counter.myWins[k].genPCT();
		#ifdef DEBUGASSERT
          if( winloss_counter.myWins[k].forceSum() > winloss_counter.myChancesEach + 0.1 || winloss_counter.myWins[k].forceSum() < winloss_counter.myChancesEach - 0.1  )
          {
              std::cerr << "Failure to generate w+s+l=" << winloss_counter.myChancesEach << " with {"<< k <<"}. Instead, w+s+l=" << (winloss_counter.myWins[k].forceSum()) << endl;
              exit(1);
          }
	    #endif
	}

	sort(winloss_counter.myWins,winloss_counter.myWins + winloss_counter.statCount);//Ascending by default
                                //  http://www.ddj.com/dept/cpp/184403792

//populate cumulation
	int32 count=1;

	vector<StatResult>& cpop = calc->cumulation;
	cpop.reserve(winloss_counter.statCount);

    float64 cumulate = winloss_counter.myWins[0].repeated;
    winloss_counter.myWins[0].wins *= cumulate;
	winloss_counter.myWins[0].splits *= cumulate;
	winloss_counter.myWins[0].loss *= cumulate;

	cpop.push_back(winloss_counter.myWins[0]);
	size_t vectorLast = cpop.size()-1; //(zero)
	float64 lastStreak=cumulate; //ASSERT: lastStreak == myWins[0].repeated

	while(count < winloss_counter.statCount)
	{

		cumulate += winloss_counter.myWins[count].repeated ;

        StatResult &b = cpop.back();

		//if (!( myWins[i].bIdenticalTo( myWins[i-1]) ))
		if( winloss_counter.myWins[count].pct != winloss_counter.myWins[count-1].pct )
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
			lastStreak = winloss_counter.myWins[count].repeated;
			winloss_counter.myWins[count].wins *= lastStreak;
			winloss_counter.myWins[count].splits *= lastStreak;
			winloss_counter.myWins[count].loss *= lastStreak;


			//Load for cumulation
			cpop.push_back(winloss_counter.myWins[count]);
			++vectorLast;
		}else
        {
            b.wins += winloss_counter.myWins[count].wins * winloss_counter.myWins[count].repeated;
            b.splits += winloss_counter.myWins[count].splits * winloss_counter.myWins[count].repeated;
            b.loss += winloss_counter.myWins[count].loss * winloss_counter.myWins[count].repeated;
            lastStreak += winloss_counter.myWins[count].repeated;
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
    if( cumulate != winloss_counter.myTotalChances )
    {
        std::cerr << "Raw count after sorting does not match: " << cumulate << " counted, but " << winloss_counter.myTotalChances << " expected." << endl;
        exit(1);
    }

#endif

	for(size_t k=0;k<=vectorLast;++k)
	{

	    StatResult& temptarget = calc->cumulation[k];

	    #ifdef DEBUGASSERT
          if( temptarget.forceSum() > winloss_counter.myChancesEach + 0.1 || temptarget.forceSum() < winloss_counter.myChancesEach - 0.1  )
          {
              std::cerr << "Failure to maintain w+s+l=" << winloss_counter.myChancesEach << " after combine step on {"<< k <<"}. Instead, w+s+l=" << (temptarget.forceSum()) << endl;
              exit(1);
          }
	    #endif

		temptarget.wins /= winloss_counter.myChancesEach;
		temptarget.loss /= winloss_counter.myChancesEach;
		temptarget.splits /= winloss_counter.myChancesEach;
        ///All outcomes so far are myPCT (having the first hand dealt. For CallStats that's the fixed hand I have. For CommunityCallStats that's the pockets named in the distribution.
        temptarget.pct = temptarget.pct/winloss_counter.myChancesEach;
    // TODO(from joseph): We have `.scaleOrdinateOnly()` you should probably use instead
		temptarget.repeated /= winloss_counter.myTotalChances;
	}


//How many of them would call a bet of x?
//It's the number of myWins elements that have a PCT above the pot odds implied by x.

}



void CallStats::myAddCard(const DeckLocation& cardinfo, const int16 undoIndex)
{
    myUndo[undoIndex].SetUnique(winloss_counter.myStrength);
    winloss_counter.myStrength.AddToHand(cardinfo);
}

void CallStats::myRevert(const int16 undoIndex)
{
    winloss_counter.myStrength.SetUnique(myUndo[undoIndex]);
}

void CallStats::myEval()
{
    winloss_counter.myStrength.evaluateStrength();
}

void CallStats::setCurrentGroupOcc(const float64 occ)
{
    winloss_counter.myWins[winloss_counter.statGroup].repeated = occ;
}

void CallStats::showProgressUpdate() const
{
    if (winloss_counter.statGroup == 0 ) std::cerr << endl << endl;
    std::cerr << "C: " << winloss_counter.statGroup << "/" << winloss_counter.statCount << "  \b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\r" << flush;
}

void CallStats::DropCard(const DeckLocation deck)
{

	winloss_counter.oppStrength.SetUnique(oppUndo[winloss_counter.currentCard - 1]);

	if (winloss_counter.currentCard > 2)
	{
		myRevert(winloss_counter.currentCard-3);
	}


	--(winloss_counter.currentCard);

}

StatRequest CallStats::NewCard(const DeckLocation deck, float64 occ)
{

	StatRequest r;
	const int16 undoCurrentCard = winloss_counter.currentCard;
	++(winloss_counter.currentCard);

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


	oppUndo[undoCurrentCard].SetUnique(winloss_counter.oppStrength);
	winloss_counter.oppStrength.AddToHand(deck);

	if ( winloss_counter.currentCard > 2 )
	{
		//past dealing opp hand
		myAddCard(deck, winloss_counter.currentCard - 3);
		if ( winloss_counter.currentCard == winloss_counter.moreCards )
		{
			myEval();
			winloss_counter.oppStrength.evaluateStrength();
		}
	}
	else
	{

	    mynoAddCard(deck, undoCurrentCard);
	    if ( winloss_counter.currentCard == 2 )
        {


            if( winloss_counter.moreCards == 2) winloss_counter.oppStrength.evaluateStrength();

            ++(winloss_counter.statGroup);


            r.bNewSet = true;

            setCurrentGroupOcc(occ);

            if(occ > 1)
            {
                winloss_counter.statCount -= static_cast<int32>(occ) - 1;
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

	winloss_counter.moreCards = 7-cardsInCommunity;

		const size_t moreCardsAlloc = winloss_counter.moreCards;

		if ((2 <= moreCardsAlloc) && (moreCardsAlloc <= 7)) {
				myUndo = new CommunityPlus[moreCardsAlloc-2];
				oppUndo = new CommunityPlus[moreCardsAlloc];
		}
		#ifdef DEBUGASSERT
		else if (winloss_counter.moreCards < 2) {
				std::cerr << "How did we get `cardsInCommunity > 5` causing CallStats::initC(" << static_cast<int>(cardsInCommunity) << ")" << std::endl;
				return exit(1);
		} else if (winloss_counter.moreCards > 7) {
				std::cerr << "CallStats::initC(" << static_cast<int>(cardsInCommunity) << ") is being called with a negative value??" << std::endl;
				return exit(1);
		} else {
				std::cerr << "There's no way to get here. We guarded on `moreCardsAlloc` in multiple places: " << static_cast<int>(winloss_counter.moreCards) << " : " << moreCardsAlloc << std::endl;
				return exit(1);
		}

	if (oppHands > 1225)
					{
									std::cerr << "CallStats::realCardsAvailable(" << static_cast<int>(cardsInCommunity) << ") means " << static_cast<int>(cardsAvail) << " = `cardsAvail > 50`" << std::endl;
									hard_exit();
					}
	#endif

				winloss_counter.myTotalChances = static_cast<float64>(oppHands);
	winloss_counter.statCount = oppHands;

				winloss_counter.myWins = new StatResult[oppHands];


				winloss_counter.myChancesEach = HoldemUtil::nchoosep<float64>(cardsAvail - 2,5-cardsInCommunity);

	if (winloss_counter.moreCards == 2)
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
