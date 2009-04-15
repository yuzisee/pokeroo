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


#define NO_REDUNDANT_SEECOMMUNITY
#define RELOAD_LAST_HAND

#include "arena.h"
#include "stratHistory.h"
#include <algorithm>





using std::endl;


double* HoldemArena::organizeWinnings(int8& potDistrSize, vector<ShowdownRep>&
	potDistr, vector<ShowdownRep>& winners )
{

    ///TODO: Is this loop serving its purpose?
	///The following loop weeds out positionally IRRELEVANT players
	//The relevant players are placed in potDistr (remember the best hand starts at the back)
	ShowdownRep curComp(-1);
	ShowdownRep lastComp(-1); //last RELEVANT entry
	lastComp = curComp; //(we use default values of ShowdownRep carefully)

	///Since sorting puts the lower betting players better hands ahead in the list,
	///This can be updated to keep track of who is still relevant
	float64 highestBetStrongerHands = 0;

	potDistrSize=0;
	while( !winners.empty() )
	{
		lastComp = curComp;
		curComp = winners.back();//best hand so far

        winners.pop_back();


		if( lastComp == curComp )
		{//Split
            //if(  curComp.revtiebreak >= highestBetStrongerHands )
            //ALWAYS TRUE
            //{
                potDistr.push_back(curComp);
                ++potDistrSize;
                highestBetStrongerHands = curComp.revtiebreak;
                //lastComp = curComp;
            //}
		}else ///Assuming sorting worked, this hand is worse or first
		{

			if ( curComp.revtiebreak > highestBetStrongerHands )
			{
				potDistr.push_back(curComp);
				++potDistrSize;
				highestBetStrongerHands = curComp.revtiebreak;
				lastComp = curComp;
			}

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


	moneyWon[0] = potDistr[0].revtiebreak;
	if( bVerbose )
	{
		gamelog << endl << p[potDistr[0].playerIndex]->GetIdent() << " can win " <<
		(potDistr[0].revtiebreak - p[potDistr[0].playerIndex]->handBetTotal) <<
		" or less\t(controls " << moneyWon[0] << " of " << myPot << ")" << endl;
	}

	while(i<potDistrSize)
	{

		moneyWon[i] = potDistr[i].revtiebreak - potDistr[j].revtiebreak;
		if( bVerbose )
		{
			gamelog << p[potDistr[i].playerIndex]->GetIdent() << " can win " <<
			(potDistr[i].revtiebreak - p[potDistr[i].playerIndex]->handBetTotal) <<
			" or less\t(controls " << moneyWon[i] << " of " << myPot << ")" << endl;
		}

		++i;++j;
	}


	return moneyWon;
}

void HoldemArena::compareAllHands(const CommunityPlus & community, const int8 called, vector<ShowdownRep>& winners)
{
    HoldemArenaShowdown w(this,called);

    while(w.bRoundState != '!')
    {
        //Player& withP = *(p[curIndex]);
		w.RevealHand(p[curIndex]->myStrat->ViewDealtHand(), community);
    }

	winners.assign(w.winners.begin(),w.winners.end());
}

void HoldemArena::PrepShowdownRound(const CommunityPlus & community)
{
	roundPlayers = livePlayers;

	if( bVerbose )
	{
		gamelog << endl;
		gamelog << "\t----------" <<
		endl << "\t|Showdown|" << endl;
		gamelog << "\t----------" << endl;

		gamelog << "Final Community Cards:" << endl;

        HandPlus displayCom;
        displayCom.SetUnique(community);
        displayCom.HandPlus::DisplayHand(gamelog);
        gamelog << endl << endl << endl;
	}
}

void HoldemArena::ProcessShowdownResults(vector<ShowdownRep> & winners)
{

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

    //Ascending by default:  http://www.ddj.com/dept/cpp/184403792
    sort(winners.begin(), winners.end());
	//Perfect, we start at the back, and that's the best hand


	int8 potDistrSize = 0;
	vector<ShowdownRep> potDistr;
	///-------------------------------------------------------
	///  Figure out all these side-pots and split pots...
	double* moneyWon = organizeWinnings(potDistrSize, potDistr, winners);
	///-------------------------------------------------------
	if(bVerbose)
	{gamelog << "* * *Comparing hands* * *" << endl;}

	int8 j = potDistrSize - 1;
	int8 i = j-1;



	randRem *= myPot+potDistrSize*p[potDistr[j].playerIndex]->handBetTotal;
	randRem /= -potDistr[j].playerIndex;



	///Now, distribute the money
	int8 splitCount=1;
	while(i >= 0)
	{

		if( potDistr[i] == potDistr[j] )
		{
			++splitCount;
			float64 splitFactor = static_cast<float64>(splitCount);
			randRem *= splitFactor+1.5;
			moneyWon[i] /= splitFactor;

			if( bVerbose )
			{
				gamelog << p[potDistr[i].playerIndex]->GetIdent() << " splits with " <<
				(int)(splitCount-1) << " other" << endl;
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


		randRem /= p[potDistr[i].playerIndex]->handBetTotal;
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
				gamelog << p[curWinner]->GetIdent() << " takes "
				<< moneyWon[k] << " from the pot, " << flush;

				earnings = (moneyWon[k] - p[curWinner]->handBetTotal);
				if ( earnings >= 0 )
				{
					gamelog << "earning " << earnings << " this round" << endl;
				}
				else
				{
					earnings *= -1;
					gamelog << "for a net loss of " << earnings << " this round" << endl;
				}
			}
		}
	}

	delete [] moneyWon;
}


void HoldemArena::PlayShowdown(const CommunityPlus & community, const int8 called)
{
    PrepShowdownRound(community);

	vector<ShowdownRep> winners;
	///------------------------------------
	///  GENERATE A LIST OF WINNERS
	compareAllHands(community, called, winners);
	///------------------------------------

	ProcessShowdownResults(winners);
}

void HoldemArena::prepareRound(const CommunityPlus& community, const int8 comSize)
{
    cardsInCommunity = comSize;

	if( bVerbose )
	{

	    #ifdef OLD_DISPLAY_STYLE
		gamelog <<endl<<endl<<endl;
		#else
		if( comSize == 0 )
		{
		    gamelog << endl << endl << "Preflop" << endl;
		}
		#endif

		#ifdef OLD_DISPLAY_STYLE
		gamelog <<endl<<endl;
		#else
		gamelog << "(Pot: $" << myPot << ")" << endl;
		PrintPositions(gamelog);
		gamelog <<endl;
		#endif
	}

	curIndex = curDealer;

	///React to community cards (each player gets to do that first)
	do{
		incrIndex();
#ifdef NO_REDUNDANT_SEECOMMUNITY
		if( CanStillBet(curIndex) )
#else
		if( IsInHand(curIndex) )
#endif
		{
			Player& withP = *(p[curIndex]);
            if( withP.IsBot() )
            {
                withP.myStrat->SeeCommunity(community, comSize);
            }
		}
	}while( curIndex != curDealer);

	myBetSum = 0;
	prevRoundPot = myPot;
	prevRoundFoldedPot = myFoldedPot;

	curIndex = curDealer;

}

void HoldemArena::defineSidePotsFor(Player& allInP, const int8 id)
{
		allInP.allIn = myPot - myBetSum; ///Bets from previous rounds


			curIndex = id;
			incrIndex();

			do{

				Player& withP = *(p[curIndex]);

				float64 matchedAmount;///How much for this round are we picking up?

				//gamelog << "why " << withP.GetIdent();

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


//Why did I comment this out? (See below)
/*				if ( matchedAmount > allInP.handBetTotal )
				{
					allInP.handBetTotal = matchedAmount;
				}*/
				allInP.allIn += matchedAmount;

				incrIndex();
			}while( curIndex != id );
//QUESTION: Why did I comment this out?
//  ANSWER: I couldn't see the need for handBetTotal to be considered at all in this situation
//          The subroutine begins with [allIn = myPot - myBetSum] which should take care of all action before this round
/*

		allInP.allIn += allInP.handBetTotal; //Account for your own bet

		allInP.myMoney -= allInP.handBetTotal;
*/
//ALSO: The following line was added as a (working) substitute for the above block
//      During resolveActions handBetTotal is usually updated...
//      resolveActions is the next thing to happen after calling defineSidePotsFor
    allInP.allIn += allInP.myBetSize; ///You are able to win back your own bet amount.
}

void HoldemArena::resolveActions(Player& withP)
{

		if( withP.myBetSize == FOLDED)
		{
			withP.myBetSize = INVALID;

		}

		if( withP.myBetSize != INVALID )
		{

			withP.lastBetSize = INVALID; //Since lastBetSize is for the same betting round as myBetSize, it doesn't make sense once the round is over

				withP.handBetTotal += withP.GetBetSize();
				withP.myMoney -= withP.GetBetSize();

			if( withP.allIn > 0 )
			{
				withP.myBetSize = INVALID;
			}
			else
			{
				withP.myBetSize = 0;
			}
		}
}



///HoldemEvents are used to handle each game
/// 1. Prepare the round for play
///			Round pot, round bets set to 0

/// 2. Action time!
///			As bets take place, the round pot increases, and round bets increase
///			If someone folds, money -= betSize, but it's already in round pot

/// 3. Cleanup and finish
///			First, all-ins are taken into account here
///			Finally, the round pot and round bets are moved into hand bets/pot


int8 HoldemArena::PlayRound(const CommunityPlus & community, const int8 comSize)
{
    HoldemArenaBetting b( this, community, comSize );



    while(b.bBetState == 'b')
    {
        b.MakeBet(p[curIndex]->myStrat->MakeBet());
    }

    return b.playerCalled;
}





void HoldemArena::RefreshPlayers()
{

    PerformanceHistory * leaderboard;
    int8 leaderboardSize = 0;

    if( bSpectate )
    {
        leaderboard = new PerformanceHistory[livePlayers];
        for( int8 i=0;i<livePlayers;++i)
        {
            leaderboard[i].sortMode = SORT_TOTAL_DELTA;
        }

        gamelog << "\n\n==========\nCHIP COUNT" << endl;
    }else
    {
        leaderboard = 0 ;
    }


    for(int8 i=0;i<nextNewPlayer;++i)
    {
        Player& withP = *(p[i]);
        if( withP.IsBot() )
        {
            if( withP.myMoney == 0 )
            {
                withP.myStrat->FinishHand();
            }
        }
    }


    for(int8 i=0;i<nextNewPlayer;++i)
    {
        Player& withP = *(p[i]);
        if( withP.myMoney == 0 )
        {
            --livePlayers;
            withP.myMoney = -1;
            withP.myBetSize = INVALID;
//            bPlayerElimBlindUp |= blinds->PlayerEliminated();
        }
        else
        {
            if( bSpectate )
            {
                if( withP.myMoney > 0 )
                {
                    leaderboard[leaderboardSize].id = i;
                    leaderboard[leaderboardSize].totalMoneyDelta = withP.myMoney;
                    ++leaderboardSize;
                }
            }

            withP.myBetSize = 0;


        }
        withP.handBetTotal = 0;
        withP.allIn = INVALID;

		if( withP.IsBot() )
		{
	        withP.myStrat->ClearDealtHand();
		}


    }



    if( bSpectate )
    {
        std::sort(leaderboard,leaderboard+leaderboardSize);
        for(int8 i=leaderboardSize-1;i>=0;--i)
        {
            Player& withP = *(p[leaderboard[i].id]);
            gamelog << withP.GetIdent() << " now has $" << withP.GetMoney() << endl;
        }
        delete [] leaderboard;
    }

    for(int8 i=0;i<nextNewPlayer;++i)
    {
        Player& withP = *(p[i]);
        if( withP.IsBot() )
        {
            if( withP.myMoney > 0 )
            {
                withP.myStrat->FinishHand();
            }
        }
    }



    ++handnum;


		do{
			++curDealer;
			curDealer %= nextNewPlayer;
		}while(!IsAlive(curDealer));



}



