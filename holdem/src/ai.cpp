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



#include <algorithm>
#include <math.h>
#include "ai.h"
#include "engine.h"

using std::endl;
using std::sort;

void PlayStats::Compare(const float64 occ)
{
#ifdef DEBUGCOMPARE
        myStrength.DisplayHandBig(std::cout);
        std::cout << "\nvs (" << occ << ")" << endl;
        oppStrength.DisplayHandBig(std::cout);
        std::cout << endl << endl;
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
		std::cout << endl << "{" << statGroup << "}" << myWins[statGroup].loss << " l + " <<
		myWins[statGroup].splits << " s + " << myWins[statGroup].wins << " w = " <<
		myWins[statGroup].loss+myWins[statGroup].splits+myWins[statGroup].wins
			<< endl;
#endif

}

void PlayStats::countWin(const float64 occ)
{
#ifdef DEBUG_AA
    if( bDEBUG )
    {
        myStrength.DisplayHand(std::cout);
        std::cout << "\tbeats vs (" << occ << ")" << flush;
        oppStrength.DisplayHand(std::cout);
        std::cout << endl << endl;

        myStrength.DisplayHandBig(std::cout);
        std::cout << "\tbeats vs (" << occ << ")" << flush;
        oppStrength.DisplayHandBig(std::cout);
        std::cout << endl << endl;
    }
#endif


	myWins[statGroup].wins += occ;
	//myWins[statGroup].pct += occ;
}
void PlayStats::countSplit(const float64 occ)
{
	myWins[statGroup].splits += occ;
	//myWins[statGroup].pct += occ/2;
}
void PlayStats::countLoss(const float64 occ)
{
#ifdef DEBUG_AA
    if( bDEBUG )
    {
        myStrength.DisplayHand(std::cout);
        std::cout << "\tloses vs (" << occ << ")" << flush;
        oppStrength.DisplayHand(std::cout);
        std::cout << endl << endl;
    }
#endif

	myWins[statGroup].loss += occ;
}

void WinStats::countSplit(const float64 occ)
{
    float64 dOccRep = oppReps * occ;
	PlayStats::countSplit(dOccRep);
	myAvg.splits += dOccRep * myWins[statGroup].repeated;// / myTotalChances;

}
void WinStats::countLoss(const float64 occ)
{
    float64 dOccRep = oppReps * occ;
	PlayStats::countLoss(dOccRep);
	myAvg.loss += dOccRep * myWins[statGroup].repeated;// / myTotalChances;
}
void WinStats::countWin(const float64 occ)
{
    float64 dOccRep = oppReps * occ;
	PlayStats::countWin(dOccRep);
	myAvg.wins += dOccRep * myWins[statGroup].repeated;// / myTotalChances;

}

void WinStats::Analyze()
{


#ifdef DEBUGFINALCALC
	cout << endl << statCount << " with " << myChancesEach << "'s each" << endl;
	for(int32 i=0;i<statCount;i++)
	{
		cout << endl << "{" << i << "}" << myWins[i].loss << " l + "
				<< myWins[i].splits << " s + " << myWins[i].wins << " w = " <<
				myWins[i].loss+myWins[i].splits+myWins[i].wins
				<< "\tx;"<< myWins[i].repeated   <<endl;
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
				myAvg.loss+myAvg.splits+myAvg.wins << "\tx;"<< myAvg.repeated <<endl;

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
			std::cout << (currentCard % 10) << " -- " << statGroup << "\n" << flush;
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
                if (statGroup == 0 ) std::cerr << endl << endl;
                std::cerr << "\rW: " << statGroup << "/" << statCount << "  \b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\r" << flush;
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

#ifdef DEBUGNEW
	HandPlus u;
	u.FillShowHand(oppHand.cardset,false);cout << endl;
#endif

		if( cardsLeft >= 3 )
		{
			myUndo[currentCard-1].SetUnique(myStrength);
			myStrength.AddToHand(deck);
			if (cardsLeft == 3) //Complete community (three cards were left: the river, and the opponent's two)
			{
                //You just dealt the last community card
				myStrength.evaluateStrength();
#ifdef DEBUGNEW
				myStrength.DisplayHandBig(cout);
#endif
                oppReps = occ;
                r.bNewSet = true;
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
			{//You just dealt the last card

				oppStrength.evaluateStrength();
			}
            //r.bTareOcc = false;
            //r.bNewHand = false;
            return r;
        }
	}
}

void WinStats::DropCard(const DeckLocation deck)
{



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



		#ifdef DEBUGDROP
		   if( currentCard == cardsToNextBet )
		   {
		       const StatResult & xd = myWins[statGroup];
		        std::cout << endl << "TOTAL "  << xd.loss << " l + "
                << xd.splits << " s + " << xd.wins << " w = " <<
                xd.loss+xd.splits+xd.wins
                << "\tx;"<< xd.repeated  << " w" << oppReps <<endl;

		   }
			//u.FillShowHand(oppHand.cardset,false);cout << endl;
			//u.FillShowHand(myHand.cardset,false);cout << "---" << cardsLeft << endl;
		#endif




	--currentCard;

#ifdef DEBUGDROP
		   if( currentCard == cardsToNextBet )
		   {
		       const StatResult & xd = myWins[statGroup];
		        std::cout << endl << "CUMUL "  << xd.loss << " l + "
                << xd.splits << " s + " << xd.wins << " w = " <<
                xd.loss+xd.splits+xd.wins
                << "\tx;"<< xd.repeated  << " w" << oppReps <<endl;

		   }
			//u.FillShowHand(oppHand.cardset,false);cout << endl;
			//u.FillShowHand(myHand.cardset,false);cout << "---" << cardsLeft << endl;
		#endif


}

//         |statCount-|
//                    |ChancesEach-----------
//community|before bet|fillCom|His Hole Cards

void WinStats::initW(const int8 cardsInCommunity)
{

		#ifdef DEBUGASSERT
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
    oppReps = 1;

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


void CallStats::Analyze()
{

#ifdef DEBUGCALLPCT
    std::cout << endl << statCount << " with " << myChancesEach << "'s each" << endl;
	for(int32 i=0;i<statCount;i++)
	{
		std::cout << endl << "{" << i << "}" << myWins[i].loss << " l + "
				<< myWins[i].splits << " s + " << myWins[i].wins << " w = " <<
        myWins[i].loss+myWins[i].splits+myWins[i].wins << "(" << myWins[i].pct << ")"
				<< "\tx;"<< myWins[i].repeated <<flush;
	}

    std::cout << endl << statCount << " with " << myChancesEach << "'s each" << endl;
	for(int32 i=0;i<statCount;i++)
	{
	    float64 px = myWins[i].loss+myWins[i].splits+myWins[i].wins;
		std::cout << endl << "{" << i << "}" << ((myWins[i].loss)/px) << "% l + "
				<< ((myWins[i].splits)/px) << "% s + " << ((myWins[i].wins)/px) << "% w = " <<
        1 << "(" << ((myWins[i].pct)/px) << ")"
				<< "\tx;"<< myWins[i].repeated <<flush;
	}

#endif

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
#ifdef DEBUGCALLPCT
	std::cout << endl << "=============SORT!=============" << endl;
	for(int32 i=0;i<statCount;i++)
	{
		std::cout << endl << "{" << i << "}" << myWins[i].loss << " l + "
				<< myWins[i].splits << " s + " << myWins[i].wins << " w = " <<
				myWins[i].loss+myWins[i].splits+myWins[i].wins << "(" << myWins[i].pct << ")"
				<< "\tx;"<< myWins[i].repeated <<flush;
	}
#endif

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
#ifdef DEBUGCALLPCT
	std::cout << endl << "======= C O M B I N E D =======" << endl;
	for(size_t j=0;j<vectorLast;++j)
	{
		std::cout << endl << "{" << j << "}" << calc->cumulation[j].loss << " l + "
				<< calc->cumulation[j].splits << " s + " << calc->cumulation[j].wins << " w =\t" << flush;
				std::cout.precision(8);
				std::cout << calc->cumulation[j].pct
				//<< " pct\tx;"<< calc->cumulation[i].repeated <<flush;
				<< " \t"<< calc->cumulation[j].repeated << "\ttocall" << flush;
	}
#endif

#ifdef DEBUGASSERT
    if( cumulate != myTotalChances )
    {
        std::cerr << "Raw count after sorting does not match: " << cumulate << " counted, but " << myTotalChances << " expected." << endl;
        exit(1);
    }

#endif

	for(size_t k=0;k<=vectorLast;++k)
	{
            #ifdef DEBUGCALLPCT
                const float64 & t_myChancesEach = myChancesEach;
                const float64 & t_myTotalChances = myTotalChances;
            #endif

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
		temptarget.pct = 1 - temptarget.pct/myChancesEach;///This is where we convert myPCT to oppPCT.
		temptarget.repeated /= myTotalChances;
	}


#ifdef DEBUGCALLPCT
std::cout << endl << "=============Reduced=============" << endl;
	std::cout.precision(2);
	for(size_t j=0;j<=vectorLast;++j)
	{
		std::cout << endl << "{" << j << "}" << calc->cumulation[j].loss << " l + "
				<< calc->cumulation[j].splits << " s + " << calc->cumulation[j].wins << " w =\t" << flush;
				std::cout.precision(8);
				std::cout << calc->cumulation[j].pct
				//<< " pct\tx;"<< calc->cumulation[i].repeated <<flush;
				<< " \t"<< calc->cumulation[j].repeated << "\ttocall" << flush;
	}
	std::cout << endl;
#endif

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

/*
        #ifdef DEBUGCALLPCT
            if( currentCard == 2 )
            {

                myStrength.DisplayHand(std::cout);
                oppStrength.DisplayHand(std::cout);


                const int32 & i = statGroup;
                std::cout << endl << "{" << i << "}";

                HoldemUtil::PrintCard(std::cout,namecorrelate[0].GetIndex());
                HoldemUtil::PrintCard(std::cout,namecorrelate[1].GetIndex());




                std::cout << myWins[i].loss << " l + "
                        << myWins[i].splits << " s + " << myWins[i].wins << " w = " <<
                myWins[i].loss+myWins[i].splits+myWins[i].wins << "(" << myWins[i].pct << ")"
                        << "\tx;"<< myWins[i].repeated <<flush;

                std::cout << endl << ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,," ;
                std::cout << endl;
                std::cout << endl;
                std::cout << endl;

            }
        #endif
*/

	///TODO: Confirm
	//oppStrength.RemoveFromHand(deck);
	oppStrength.SetUnique(oppUndo[currentCard-1]);

	if (currentCard > 2)
	{
		///TODO: Confirm...
		myRevert(currentCard-3);
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

#ifdef DEBUG_AA
        //bDEBUG = ( oppStrength.SeeCards(2) == HoldemConstants::CARD_ACEHIGH && oppStrength.SeeCards(3) == HoldemConstants::CARD_ACEHIGH );
        bDEBUG = ( oppStrength.SeeCards(1) == HoldemConstants::CARD_TREY + HoldemConstants::CARD_DEUCE + HoldemConstants::CARD_FOUR + HoldemConstants::CARD_SIX );

#endif

            if( moreCards == 2) oppStrength.evaluateStrength();

            ++statGroup;


            r.bNewSet = true;

            setCurrentGroupOcc(occ);

            if(occ > 1)
            {
                statCount -= static_cast<int32>(occ) - 1;
            }


#ifdef PROGRESSUPDATE
            showProgressUpdate();
            #ifdef DEBUG_AA
            if( !bDEBUG )
            {
                std::cout << "SKIP: " << flush;
                oppStrength.DisplayHand(std::cout);
                std::cout << endl << endl;
            }
            #endif
#endif

        }

        #ifdef DEBUGCALLPCT
            if( currentCard == 1 ){ namecorrelate = deck; }
            else
            {
                const int32 & i = statGroup;
                std::cout << endl << "{" << i << "}";

                HoldemUtil::PrintCard(std::cout,namecorrelate.GetIndex());
                HoldemUtil::PrintCard(std::cout,deck.GetIndex());
            }
        #endif

	}

	return r;

}

//         |statCount ----|
//                        |ChancesEach---
//community|His Hole Cards|To 5 community

const int8 CallStats::realCardsAvailable(const int8 cardsInCommunity) const
{
    return 52 - cardsInCommunity - 2;
}

void CallStats::initC(const int8 cardsInCommunity)
{
	calc = new CallCumulation();

	moreCards = 7-cardsInCommunity;

	myUndo = new CommunityPlus[moreCards-2];
	oppUndo = new CommunityPlus[moreCards];

    int8 cardsAvail = realCardsAvailable(cardsInCommunity);

    int32 oppHands = cardsAvail*(cardsAvail-1)/2;
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
