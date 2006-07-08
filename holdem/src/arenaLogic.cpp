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
#define DEBUGROUNDINDEX
//#define EXTRAMONEYUPDATE
//#define DEBUGALLINS

#include "arena.h"
#include <iostream>

using std::cout;
using std::endl;

void HoldemArena::compareAllHands(const int8 called, vector<ShowdownRep>& winners)
{
	curIndex = called;

	ShowdownRep best;


	do
	{

//cout << p[curIndex]->GetIdent() << "\tallIn=" << p[curIndex]->allIn << endl;
		if( IsInHand(curIndex) ) //If STILL IN HAND (not all in)
		{

		    Player& withP = *p[curIndex];
            ShowdownRep comp(withP.myHand, community, curIndex);

			if( comp > best ) //best hand yet
			{

				///We set the allIn, since this player "IsInHand()"
				withP.allIn = myPot;

				broadcastHand(withP.myHand);
				if( bVerbose )
				{


					cout << endl << withP.GetIdent() << flush;
					cout << " reveals: " << flush;
					HandPlus viewHand;
					viewHand.SetUnique(withP.myHand);
					viewHand.ShowHand(false);
					cout << endl << "Making," << flush;
					comp.DisplayHandBig();
				}

				//winnerCount = 1;
				winners.clear();
				winners.push_back(comp);
				best = comp;

			}
			else if( comp == best ) //can only split, if not beaten later
			{

				broadcastHand(withP.myHand);
				if( bVerbose )
				{
					HandPlus viewHand;

					cout << endl << withP.GetIdent() << flush;
					cout << " turns up: " << endl;
					viewHand.SetUnique(withP.myHand);
					viewHand.ShowHand(true);

					cout << "Split..." << flush;
					comp.DisplayHand();
					cout << endl;
				}

				winners.push_back(comp);
				//winnerCount++;
			}
			else
			{
				cout << endl << withP.GetIdent() << " mucks " << endl;
			}
		}

		incrIndex();

	}while(curIndex != called);

//cout << "Next phase" << endl;
	///Non all-in players show first,
	///All-in players are manditorily showing afterwards.

	do
	{
	    Player& withP = *p[curIndex];

//cout << p[curIndex]->GetIdent() << "\t now " << p[curIndex]->allIn << endl;
		if ( withP.allIn >= 0 && !IsInHand(curIndex)) //If all in with no money remaining
		{
		    ShowdownRep comp(withP.myHand, community, curIndex);
		    if(  !( comp < best )  )
		    {
                broadcastHand(withP.myHand);
                if( bVerbose )
                {
                    cout << endl << withP.GetIdent() << flush;
                    cout << " is ahead with: " << flush;
                    HandPlus viewHand;
                    viewHand.SetUnique(withP.myHand);
                    viewHand.ShowHand(false);
                    cout << endl << "Trying to stay alive, makes" << flush;
                    comp.DisplayHandBig();
                }
                winners.push_back(comp);
                best = comp;
		    }else
		    {///His hand was worse, but since he is all-in he MUST show his hand.
		    //  http://www.texasholdem-poker.com/holdem_rules.php
		        broadcastHand(withP.myHand);
		        if( bVerbose )
                {
                    cout << endl << withP.GetIdent() << flush;
                    cout << " turns over " << flush;
                    HandPlus viewHand;
                    viewHand.SetUnique(withP.myHand);
                    viewHand.ShowHand(false);
                    cout << endl << "Is eliminated after making only" << flush;
                    comp.DisplayHandBig();
                }
		    }
		}
		incrIndex();

	}while(curIndex != called);
}

double* HoldemArena::organizeWinnings(int8& potDistrSize, vector<ShowdownRep>&
	potDistr, vector<ShowdownRep>& winners )
{


	///The following loop weeds out positionally IRRELEVANT players
	//The relevant players are placed in potDistr
	ShowdownRep curComp;
	ShowdownRep lastComp;
	potDistrSize=0;
	while( !winners.empty() )
	{
		lastComp = curComp; //(we use default values of ShowdownRep carefully)
		curComp = winners.back();//best hand so far

		if( lastComp > curComp && lastComp.revtiebreak == myPot )
		{	//lastCom was the deciding hand in all accounts
			winners.clear();
		}
		else
		{
			//if ( curComp.revtiebreak >= mostRecentAllIn )
			//{
				potDistr.push_back(curComp);
				++potDistrSize;
			//}

			winners.pop_back();
		}
	}

	/// Now the best hand is STARTING AT THE FRONT!
	double * moneyWon = new double[potDistrSize];

/*

Procedure:
	[revtiebreaks are all assigned]
	[moneyWins are assigned linearly, downward]
	[splitcount is assigned to each during an upward pass]



revtiebreak		moneywin		splitcount (backwards pass)

40				40				1

/60				20				(2) -> split between next (2) forward
\300			240				1

/1000			700				(2) -> split between next (2) forward
\1000			0				1


Legend:
	Index[0] is on top
	The best hand is on top (notice that it wins the least money)
	Inferior hands are left below, but qualify for a side pot.

*/

	///Populate moneyWon
	int8 i=1;int8 j=0;

#ifdef DEBUGALLINS
cout << "Moneywon " << potDistrSize << endl;
#endif

	moneyWon[0] = potDistr[0].revtiebreak;
	if( bVerbose )
	{
		cout << endl << p[potDistr[0].playerIndex]->GetIdent() << " can win " <<
		(potDistr[0].revtiebreak - p[potDistr[0].playerIndex]->handBetTotal) <<
		"\t(controls " << moneyWon[0] << ")" << endl;
	}

	while(i<potDistrSize)
	{

		moneyWon[i] = potDistr[i].revtiebreak - potDistr[j].revtiebreak;
		if( bVerbose )
		{
			cout << p[potDistr[i].playerIndex]->GetIdent() << " can win " <<
			(potDistr[i].revtiebreak - p[potDistr[i].playerIndex]->handBetTotal) <<
			"\t(controls " << moneyWon[i] << ")" << endl;
		}

		++i;++j;
	}

#ifdef DEBUGALLINS
cout << "^^^^^^^^^^^^^^^" << endl;
#endif

	return moneyWon;
}

void HoldemArena::PlayShowdown(const int8 called)
{
	if( bVerbose )
	{
		cout << endl;
		cout << "\t!!!!!!!!!!" <<
		endl << "\t!Showdown!" << endl;
		cout << "\t!!!!!!!!!!" << endl;

		cout << "Final Community Cards:" << endl;

        HandPlus displayCom;
        displayCom.SetUnique(community);
        displayCom.DisplayHand();
        cout << endl << endl;
	}

	vector<ShowdownRep> winners;
	///------------------------------------
	///  GENERATE A LIST OF WINNERS
	compareAllHands(called, winners);
	///------------------------------------


	size_t vectorSize=winners.size();
	double winnable;


	///Prepare for sorting
	for(size_t i=0;i<vectorSize;++i)
	{
		int8 curWinner = winners[i].playerIndex;
		winnable = p[curWinner]->allIn;
		if( winnable == INVALID )
		{
			winnable = myPot;
		}
		winners[i].revtiebreak = winnable;
	}

	sort(winners.begin(), winners.end());
	//Perfect, we start at the back, and that's the best hand
#ifdef DEBUGALLINS
if (winners.size() > 1)
{
	cout << winners[0].strength << "\t" << winners[1].strength << endl;
}
#endif

	int8 potDistrSize = 0;
	vector<ShowdownRep> potDistr;
	///-------------------------------------------------------
	///  Figure out all these side-pots and split pots...
	double* moneyWon = organizeWinnings(potDistrSize, potDistr, winners);
	///-------------------------------------------------------
	if(bVerbose)
	{cout << "Comparing hands..." << endl;}

	int8 j = potDistrSize - 1;
	int8 i = j-1;

#ifdef DEBUGALLINS
cout << p[potDistr[j].playerIndex]->GetIdent() << "\t"
<< potDistr[j].revtiebreak << "\t" << moneyWon[j] << "\t"
<< 1 << endl;
#endif

	randRem *= myPot*potDistrSize*p[potDistr[j].playerIndex]->handBetTotal;
	randRem -= potDistr[j].playerIndex;



	///Now, distribute the money
	int8 splitCount=1;
	while(i >= 0)
	{

		if( potDistr[i] == potDistr[j] )
		{
			++splitCount;
			float64 splitFactor = static_cast<float64>(splitCount);
			randRem *= splitFactor;
			moneyWon[i] /= splitFactor;

			if( bVerbose )
			{
				cout << p[potDistr[i].playerIndex]->GetIdent() << " splits with " <<
				(int)(splitCount) << " other" << endl;
			}

			///Loop through each player that:
			///  splits hand with [i]
			///  and bet more than [i]
			for(int8 n=i+1;n<i+splitCount;++n)
			{
				moneyWon[n] += moneyWon[i];
			}

		}
		else
		{
			splitCount = 1;
		}

#ifdef DEBUGALLINS
cout << p[potDistr[i].playerIndex]->GetIdent() << "\t"
<< potDistr[i].revtiebreak << "\t" << moneyWon[i] << "\t"
<< splitCount << endl;
#endif


		randRem -= p[potDistr[i].playerIndex]->handBetTotal;
		--i;--j;
	}

	for(int8 k=0;k<potDistrSize;++k)
	{
		int8 curWinner = potDistr[k].playerIndex;
		p[curWinner]->myMoney += moneyWon[k];
		if( bVerbose )
		{
			float64 earnings = moneyWon[k];
			if ( earnings > 0 )
			{  //If the player takes any money at all
				cout << p[curWinner]->GetIdent() << " takes "
				<< moneyWon[k] << " from the pot, " << flush;

				earnings = (moneyWon[k] - p[curWinner]->handBetTotal);
				if ( earnings >= 0 )
				{
					cout << "earning " << earnings << " this round" << endl;
				}
				else
				{
					earnings *= -1;
					cout << "for a net loss of " << earnings << " this round" << endl;
				}
			}
		}
	}

	delete [] moneyWon;
}

void HoldemArena::prepareRound(const int8 comSize)
{
	if( bVerbose )
	{
		cout <<endl<<endl<<endl;
		if( comSize == 3 )
		{
			cout << "\t------" << endl;
			cout << "\t|FLOP|" << endl;
			cout << "\t------" << endl;
		}
		else if (comSize == 4)
		{
			cout << "\t------" <<
			endl << "\t|TURN|" << endl;
			cout << "\t------" << endl;
		}
		else if (comSize == 5)
		{
			cout << "\t-------" <<
			endl << "\t|RIVER|" << endl;
			cout << "\t-------" << endl;
		}
		cout <<endl<<endl;
	}

	curIndex = curDealer;

	///React to community cards (each player gets to do that first)
	do{
		incrIndex();
		if( IsInHand(curIndex) )
		{
			Player& withP = *(p[curIndex]);
			withP.myStrat->SeeCommunity(community, comSize);
		}
	}while( curIndex != curDealer);

}

void HoldemArena::defineSidePotsFor(Player& allInP, const int8 id)
{
	//allInP.myBetSize = allInP.allIn - allInP.handBetTotal;
		allInP.allIn = myPot - myBetSum;

#ifdef DEBUGALLINS
cout << allInP.GetIdent() << " determined to be all in." << endl;
cout << "This round bet: " << allInP.myBetSize << endl;
cout << "Past round pots " << allInP.allIn << endl;
#endif
			curIndex = id;
			incrIndex();

			do{

				Player& withP = *(p[curIndex]);

				float64 matchedAmount;

				//cout << "why " << withP.GetIdent();

				if( withP.myBetSize == FOLDED)
				{
					if(allInP.myBetSize < withP.handBetTotal)
					{
						matchedAmount = allInP.myBetSize ;
					}
					else
					{
						matchedAmount = withP.handBetTotal ;
					}

				}
				else if( withP.myBetSize != INVALID )
				{
					if(allInP.myBetSize < withP.myBetSize)
					{
						matchedAmount = allInP.myBetSize ;
					}
					else
					{
						matchedAmount = withP.myBetSize ;
					}

				}
				else
				{
					//TODO: What happens here?
					matchedAmount = 0;
				}

#ifdef DEBUGALLINS
cout << "Adding " << matchedAmount << ", " << withP.GetIdent() << endl;
#endif

				if ( matchedAmount > allInP.handBetTotal )
				{
					allInP.handBetTotal = matchedAmount;
				}
				allInP.allIn += matchedAmount;

				incrIndex();
			}while( curIndex != id );


#ifdef DEBUGALLINS
cout << "And of course " << allInP.handBetTotal << " of money was bet... " << endl;
#endif

		allInP.allIn += allInP.handBetTotal; //Account for you own bet

#ifdef DEBUGALLINS
cout << "Ultimately, allIn=" << allInP.allIn << endl;
#endif

		allInP.myMoney -= allInP.handBetTotal;
}

void HoldemArena::resolveActions(Player& withP)
{
	//cout << "why " << withP.GetIdent();

		if( withP.myBetSize == FOLDED)
		{
			withP.myBetSize = INVALID;

		}

		if( withP.myBetSize != INVALID )
		{

			withP.lastBetSize = INVALID;

			if( withP.allIn > 0 )
			{
				withP.myBetSize = INVALID;
			}
			else
			{
				withP.handBetTotal += withP.GetBetSize();
				withP.myMoney -= withP.GetBetSize();
				//cout << " clear";
				withP.myBetSize = 0;
			}
		}

		//cout << endl;
}

///This is a three part function.
/// 1. Prepare the round for play
///			Round pot, round bets set to 0

/// 2. Action time!
///			As bets take place, the round pot increases, and round bets increase
///			If someone folds, money -= betSize, but it's already in round pot

/// 3. Cleanup and finish
///			First, all-ins are taken into account here
///			Finally, the round pot and round bets are moved into hand bets/pot
int8 HoldemArena::PlayRound(const int8 comSize)
{

	///----------
	prepareRound(comSize);
	///----------

	int8 bBlinds = curDealer;	//Used to account for soft-bets:
								//ie, if called, you can still reraise.
								//BUT, it also handles the check-check-check
	myBetSum = 0;
	int8 highestBetter = curDealer;
	highBet = 0;

	curIndex = curDealer;




/*
	Rules for allInsNow:
		it contains a list of players that have bet all in but as of
		yet do not know how much they can win before a side pot takes place
	"Maybe we can use handBetTotal to determine. Aha! No need."
		Maybe Use (myPot - myBetSum) to account for previous rounds?


*/


	int8* allInsNow = new int8[nextNewPlayer];
	int8 allInsNowCount = 0;

	///Weird stuff to simulate blind bets
	if( comSize == 0 && playersInHand >= 2)
	{

	    ///TODO: If a blind puts you all-in, that mechanic has to occur:
	    /*
            Player& withP = p[curIndex];
            if( withP.myBetSize > withP.myMoney )
            {
				withP.myBetSize = withP.myMoney;
				withP.allIn = withP.myMoney;
					//we must remember allIn as above: it's what we can win/person

				++playersAllIn;

				allInsNow[allInsNowCount] = curIndex;
				++allInsNowCount;
            }
	    */
		do
		{
			incrIndex();
		}while(!IsInHand(curIndex));

        int8 smallBlindIndex = curIndex;
        Player& withP1 = *p[curIndex];
        withP1.myBetSize = GetSmallBlind();

        if( withP1.myBetSize > withP1.myMoney )
        {
            withP1.myBetSize = withP1.myMoney;
            withP1.allIn = withP1.myMoney;
                //we must remember allIn as above: it's what we can win/person

            ++playersAllIn;

            allInsNow[allInsNowCount] = curIndex;
            ++allInsNowCount;
        }

            if( bVerbose )
            {
                cout  << withP1.GetIdent() <<
                    " posts " << withP1.myBetSize << endl;
            }

		do
		{
			incrIndex();
		}while(!IsInHand(curIndex));

		Player& withP2 = *p[curIndex];
		withP2.myBetSize = GetBigBlind();

        if( withP2.myBetSize > withP2.myMoney )
        {
            withP2.myBetSize = withP2.myMoney;
            withP2.allIn = withP2.myMoney;
                //we must remember allIn as above: it's what we can win/person

            ++playersAllIn;

            allInsNow[allInsNowCount] = curIndex;
            ++allInsNowCount;
        }

            if( bVerbose )
            {
                cout  << withP2.GetIdent() <<
                    " posts " << withP2.myBetSize << endl;
            }

		bBlinds = curIndex;

		addBets(withP1.myBetSize+withP2.myBetSize);
		highestBetter = curIndex;
		highBet = withP2.myBetSize;
		if( withP1.myBetSize > withP2.myBetSize )
		{
		    highestBetter = smallBlindIndex;
            highBet = withP1.myBetSize;
		}
	}
/*	else
	{
		//bBlinds = -1;
		incrIndex(highestBetter);
	}*/

	incrIndex();




	///---------------------------------
	///Action time!
	///---------------------------------
	while(
			(curIndex != highestBetter || bBlinds == highestBetter)
			&&
			(playersInHand - playersAllIn + allInsNowCount > 1)
		  ){
		///The loop continues until:
		/*
			Betting gets back to the highest better, but it's not the big blind
			OR
			There is one player left in the hand
		*/

		Player& withP = *(p[curIndex]);
#ifdef DEBUGALLINS
cout << withP.GetIdent() << "'s turn" << endl;
#endif

		if( IsInHand(curIndex) && p[curIndex]->allIn == INVALID )
		{
#ifdef DEBUGALLINS
cout << IsInHand(curIndex) << " && " << (p[curIndex]->allIn == INVALID) << endl;
#endif

			///Get a bet from the player
			withP.lastBetSize = withP.myBetSize;

			if ( withP.myMoney == 0 )
			{
				withP.myBetSize = 0;
			}
			else
			{
				withP.myBetSize = withP.myStrat->MakeBet();
			}

#ifdef DEBUGALLINS
cout << "Entered, " << withP.myBetSize << " vs " << highBet << endl;
#endif

			///Decide what to do with the bet
			if( withP.myBetSize >= withP.myMoney )
			{
						randRem += withP.lastBetSize/withP.myBetSize;

				///ALL-IN. Notice that allIn combines handBet with myBet
				///However, myBetSize is now the WHOLE HAND.
				withP.myBetSize = withP.myMoney;
				withP.allIn = withP.handBetTotal + withP.myBetSize;
					//we must remember allIn as above: it's what we can win/person

				++playersAllIn;

				allInsNow[allInsNowCount] = curIndex;
				++allInsNowCount;
			}
			else if( withP.myBetSize < highBet )
			{///Player folds.
				randRem /= myBetSum+playersInHand;

				///You're taking money away here. addBets happenned a while ago
				withP.handBetTotal = withP.lastBetSize;
						//CAUTION: TRICKY USE OF handBetTotal
				withP.myMoney -= withP.lastBetSize;
				withP.myBetSize = FOLDED;
				--playersInHand;



			}

			broadcastCurrentMove(curIndex, withP.myBetSize, highBet
					, curIndex == bBlinds && comSize == 0,withP.allIn > 0);

			if( withP.myBetSize >= highBet )
			{
				addBets(withP.myBetSize - withP.lastBetSize);

				if( withP.myBetSize > highBet )
				{
					highBet = withP.myBetSize;
					highestBetter = curIndex;


                        randRem *= myBetSum*myPot;
				}
                    randRem /= myPot + curIndex;


			}
			else if( withP.allIn > 0 )
			{   ///If you are going all in with LESS money than to call
                ///The money you have left over still NEEDs to be added to the pot
                ///Without this section, it wouldn't happen
			    addBets(withP.myBetSize - withP.lastBetSize);

                    randRem += myBetSum/curIndex;
			}

#ifdef DEBUGALLINS
cout << p[curIndex]->GetIdent() << " up now" << endl;
#endif

		}


#ifdef DEBUGALLINS
cout << p[highestBetter]->GetIdent() << " last to raise" << endl;
#endif


#ifdef DEBUGALLINS
cout << curIndex << " == " << bBlinds << endl;
if ( bBlinds != -1 )
{
	cout << "Why is bBlinds " << p[bBlinds]->GetIdent() <<
	"? Is it comSize = " << comSize << endl;
}
#endif


		if( curIndex == bBlinds )
		{
			///Account for the weird blind structure where a call can be
			///reraised by a person in the blinds

			///But it must also maintain the check-check-check events

			bBlinds = -1;
			if ( withP.myBetSize > withP.lastBetSize
					||
				curIndex != highestBetter )
			{//You only extend play if there is additional bet here
				incrIndex();
			}


		}
		else
		{
			if( playersInHand == 1 )
			{
				bBlinds = -1;
			}
			incrIndex();
		}
#ifdef DEBUGALLINS
cout << p[curIndex]->GetIdent() << " up next... same as before?" << endl;
#endif

	}//End of 'Action!' loop


///If the round goes check-check-check, it technically means the dealer is the higher better. We want the NEXT person.
/*
http://www.playwinningpoker.com/poker/rules/basics/
If everyone checks (or is all-in) on the final betting round, the player who acted first is the first to show the hand. If there is wagering on the final betting round, the last player to take aggressive action by a bet or raise is the first to show the hand. In order to speed up the game, a player holding a probable winner is encouraged to show the hand without delay. If there is a side pot, players involved in the side pot should show their hands before anyone all-in for only the main pot.
*/
	if ( highBet == 0 )
	{
	    if( playersInHand == playersAllIn)
	    {///Find the first player after the dealer
            do
            {
                incrIndex(highestBetter);
            }while( p[highestBetter]->allIn < 0);
	    }else
	    {///Find the first player to check
            do
            {
                incrIndex(highestBetter);
            }while(!IsInHand(highestBetter));
	    }
	}

	///---------------------------------
	///Resolve actions, and cleanup status...
	///---------------------------------
	//ie. push all the chips into a single pile in the middle of the table.
	for(int8 i=0;i<allInsNowCount;++i)
	{

		Player& allInP = *(p[allInsNow[i]]);
		defineSidePotsFor(allInP, allInsNow[i]);

	}

	curIndex = highestBetter;

	do{
		incrIndex();
		Player& withP = *(p[curIndex]);

		resolveActions(withP);

	}while( curIndex != highestBetter);

	///At this point all "THIS ROUND ONLY" values are useless EXCEPT allIn

	delete [] allInsNow;

	myBetSum = 0;
	highBet = 0;


	///End hand if all folded
	if( playersInHand > 1 )
	{
		//Except for showdown order, this is only necessarily NOT 'return -1;'
		return highestBetter;
	}
	else
	{
		if( playersInHand == 1 )
		{

		    ///What if the player that folds is the highest better at the moment?
            ///This can only happen on blinds.
            if( !IsInHand(highestBetter) )
            {
                int8 findHighestBetter = highestBetter;

                incrIndex(findHighestBetter);
                while(findHighestBetter != curIndex)
                {
                    if( p[findHighestBetter]->myBetSize == highBet )
                    {
                        highestBetter = findHighestBetter;
                    }
                    incrIndex(findHighestBetter);
                }
            }

			//What if you fold to an all-in? I think it will work just fine.
			if( bVerbose )
			{
				cout << "All fold! " << p[highestBetter]->GetIdent() <<
				" wins " << (myPot - p[highestBetter]->handBetTotal) << endl;
			}
			float64 rh = static_cast<float64>(highestBetter);
			randRem /= myPot*p[highestBetter]->handBetTotal+rh;
			randRem *= rh;
			p[highestBetter]->myMoney += myPot;

		}

		return -1;
	}

}

void HoldemArena::PlayHand()
{

	if( bVerbose )
	{
		cout << "================================================================" << endl;
		cout << "============================New Hand============================" << endl;
	}

	blinds->HandPlayed(0);

	playersInHand = livePlayers;
	playersAllIn = 0;

	community.Empty();

	if( PlayRound(0) == -1 ) return;

	dealer.DealCard(community);
	dealer.DealCard(community);
	dealer.DealCard(community);

	if( PlayRound(3) == -1 ) return;

	dealer.DealCard(community);

	if( PlayRound(4) == -1 ) return;

	dealer.DealCard(community);

	int8 playerToReveal = PlayRound(5);

	if( playerToReveal == -1 ) return;

	PlayShowdown(playerToReveal);
}


void HoldemArena::DealHand()
{
	int8 dealtEach = 0;
	dealer.UndealAll();
	if(randRem != 0)
	{
		dealer.ShuffleDeck(randRem);
		randRem = 1;
	}

	while(dealtEach < 2)
	{
		incrIndex();

		Player& withP = *(p[curIndex]);

		if(withP.myMoney > 0)
		{
			dealer.DealCard(withP.myHand);
		}
		else
		{
			withP.lastBetSize = INVALID;
			withP.myBetSize = INVALID;
		}

		if(curDealer == curIndex) ++dealtEach;
	}
}

void HoldemArena::PlayGame()
{
	if( p.empty() ) return;

	dealer.ShuffleDeck(static_cast<float64>(livePlayers));
	curIndex = 0;
	curDealer = 0;

	while(livePlayers > 1)
	{

		do{
			++curDealer;
			curDealer %= nextNewPlayer;
		}while(!IsAlive(curDealer));

		myPot = 0;

		curIndex = curDealer;

		DealHand();
		PlayHand();
		for(int8 i=0;i<nextNewPlayer;++i)
		{
			Player& withP = *(p[i]);
			if( withP.myMoney == 0 )
			{
				--livePlayers;
				withP.myMoney = -1;
				withP.myBetSize = INVALID;
			}
			else
			{

/*			    if(bVerbose)
			    {
			        cout << withP.GetIdent() << " now has " << withP.GetMoney() << endl;
			    }*/
				withP.myBetSize = 0;
			}
			withP.handBetTotal = 0;
			withP.allIn = INVALID;

			withP.myHand.Empty();
		}

	}

}


