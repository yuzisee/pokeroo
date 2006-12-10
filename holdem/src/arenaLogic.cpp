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

//#define EXTRAMONEYUPDATE
//#define DEBUGALLINS
//#define FORCEPAUSE


#include "arena.h"
#include <fstream>
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

#ifdef DEBUGALLINS
gamelog << "Moneywon " << potDistrSize << endl;
#endif

	moneyWon[0] = potDistr[0].revtiebreak;
	if( bVerbose )
	{
		gamelog << endl << p[potDistr[0].playerIndex]->GetIdent() << " can win " <<
		(potDistr[0].revtiebreak - p[potDistr[0].playerIndex]->handBetTotal) <<
		"\t(controls " << moneyWon[0] << " of " << myPot << ")" << endl;
	}

	while(i<potDistrSize)
	{

		moneyWon[i] = potDistr[i].revtiebreak - potDistr[j].revtiebreak;
		if( bVerbose )
		{
			gamelog << p[potDistr[i].playerIndex]->GetIdent() << " can win " <<
			(potDistr[i].revtiebreak - p[potDistr[i].playerIndex]->handBetTotal) <<
			"\t(controls " << moneyWon[i] << " of " << myPot << ")" << endl;
		}

		++i;++j;
	}

#ifdef DEBUGALLINS
gamelog << "^^^^^^^^^^^^^^^" << endl;
#endif

	return moneyWon;
}

void HoldemArena::compareAllHands(const int8 called, vector<ShowdownRep>& winners)
{
    HoldemArenaShowdown w(this,called,winners);

    while(w.bRoundState != '!')
    {
        ShowdownRep comp(&(p[curIndex]->myHand), &community, curIndex);
        w.RevealHand(comp);
    }
}


void HoldemArena::PlayShowdown(const int8 called)
{
	if( bVerbose )
	{
		gamelog << endl;
		gamelog << "\t!!!!!!!!!!" <<
		endl << "\t!Showdown!" << endl;
		gamelog << "\t!!!!!!!!!!" << endl;

		gamelog << "Final Community Cards:" << endl;

        HandPlus displayCom;
        displayCom.SetUnique(community);
        displayCom.DisplayHand(gamelog);
        gamelog << endl << endl;
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

    //Ascending by default:  http://www.ddj.com/dept/cpp/184403792
    sort(winners.begin(), winners.end());
	//Perfect, we start at the back, and that's the best hand
#ifdef DEBUGALLINS
if (winners.size() > 1)
{
	gamelog << winners[0].strength << "\t" << winners[1].strength << endl;
}
#endif

	int8 potDistrSize = 0;
	vector<ShowdownRep> potDistr;
	///-------------------------------------------------------
	///  Figure out all these side-pots and split pots...
	double* moneyWon = organizeWinnings(potDistrSize, potDistr, winners);
	///-------------------------------------------------------
	if(bVerbose)
	{gamelog << "Comparing hands..." << endl;}

	int8 j = potDistrSize - 1;
	int8 i = j-1;

#ifdef DEBUGALLINS
gamelog << p[potDistr[j].playerIndex]->GetIdent() << "\t"
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

#ifdef DEBUGALLINS
gamelog << p[potDistr[i].playerIndex]->GetIdent() << "\t"
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

void HoldemArena::prepareRound(const int8 comSize)
{
    cardsInCommunity = comSize;

	if( bVerbose )
	{
		gamelog <<endl<<endl<<endl;
		if( comSize == 3 )
		{
			gamelog << "\t------" << endl;
			gamelog << "\t|FLOP|" << endl;
			gamelog << "\t------" << endl;
		}
		else if (comSize == 4)
		{
			gamelog << "\t------" <<
			endl << "\t|TURN|" << endl;
			gamelog << "\t------" << endl;
		}
		else if (comSize == 5)
		{
			gamelog << "\t-------" <<
			endl << "\t|RIVER|" << endl;
			gamelog << "\t-------" << endl;
		}
		gamelog <<endl<<endl;
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

	myBetSum = 0;
	prevRoundPot = myPot;

	curIndex = curDealer;

}

void HoldemArena::defineSidePotsFor(Player& allInP, const int8 id)
{
	//allInP.myBetSize = allInP.allIn - allInP.handBetTotal;
		allInP.allIn = myPot - myBetSum; ///Bets from previous rounds

#ifdef DEBUGALLINS
gamelog << allInP.GetIdent() << " determined to be all in." << endl;
gamelog << "This round bet: " << allInP.myBetSize << endl;
gamelog << "Past round pots " << allInP.allIn << endl;
#endif
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

#ifdef DEBUGALLINS
gamelog << "Adding " << matchedAmount << ", " << withP.GetIdent() << endl;
#endif

//Why did I comment this out? (See below)
/*				if ( matchedAmount > allInP.handBetTotal )
				{
					allInP.handBetTotal = matchedAmount;
				}*/
				allInP.allIn += matchedAmount;

				incrIndex();
			}while( curIndex != id );
//Why did I comment this out? I couldn't see the need for handBetTotal to be considered at all in this situation
//The subroutine begins with [allIn = myPot - myBetSum] which should take care of all action before this round
/*
#ifdef DEBUGALLINS
gamelog << "And of course " << allInP.handBetTotal << " of money was bet... " << endl;
#endif

		allInP.allIn += allInP.handBetTotal; //Account for you own bet

#ifdef DEBUGALLINS
gamelog << "Ultimately, allIn=" << allInP.allIn << endl;
#endif

		allInP.myMoney -= allInP.handBetTotal;
*/
//The following lines were added as a (working) substitute for the above block
//During resolveActions handBetTotal is usually updated...
//resolveActions is the next thing to happen after calling defineSidePotsFor
    allInP.allIn += allInP.myBetSize; ///You are able to win back your own bet.
}

void HoldemArena::resolveActions(Player& withP)
{

		if( withP.myBetSize == FOLDED)
		{
			withP.myBetSize = INVALID;

		}

		if( withP.myBetSize != INVALID )
		{

			withP.lastBetSize = INVALID;

				withP.handBetTotal += withP.GetBetSize();
				withP.myMoney -= withP.GetBetSize();

			if( withP.allIn > 0 )
			{
				withP.myBetSize = INVALID;
			}
			else
			{




				//gamelog << " clear";
				withP.myBetSize = 0;
			}
		}

		//gamelog << endl;
}



///This was a three part function. (LECAGY COMMENT)
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
    HoldemArenaBetting b(this, comSize);

    while(b.bBetState == 'b')
    {
        b.MakeBet(p[curIndex]->myStrat->MakeBet());
    }

    return b.playerCalled;
}




void HoldemArena::PlayGame()
{

	if( bVerbose )
	{
		gamelog << "================================================================" << endl;
		gamelog << "============================New Hand" <<
		#if defined(DEBUGSPECIFIC) || defined(REPRODUCIBLE)
		" #"<< handnum <<
		#else
		"==" <<
		#endif
		"========================" << endl;

		#ifdef DEBUGSPECIFIC
		if (handnum == DEBUGSPECIFIC)
		{
		    gamelog << "Monitor situation" << endl;
		}
		#endif

	}

	blinds->HandPlayed(0);

	playersInHand = livePlayers;
	playersAllIn = 0;

	community.SetEmpty();

	if( PlayRound(0) == -1 ) return;

	if (!dealer.DealCard(community))  gamelog << "OUT OF CARDS ERROR" << endl;
	if (!dealer.DealCard(community))  gamelog << "OUT OF CARDS ERROR" << endl;
	if (!dealer.DealCard(community))  gamelog << "OUT OF CARDS ERROR" << endl;
    if( bSpectate )
    {

        gamelog << "Dealer deals\t" << flush;
        community.DisplayHand(gamelog);
    }


	if( PlayRound(3) == -1 ) return;

    if( bSpectate )
    {
        gamelog << "Previously\t" << flush;
        community.DisplayHand(gamelog);
        gamelog << "Dealer deals\t" << flush;
    }
	if (!dealer.DealCard(community))  gamelog << "OUT OF CARDS ERROR" << endl;
	if( bSpectate ) HoldemUtil::PrintCard(gamelog, dealer.dealt.Suit,dealer.dealt.Value);



	if( PlayRound(4) == -1 ) return;


    if( bSpectate )
    {
        gamelog << "Previously\t" << flush;
        community.DisplayHand(gamelog);
        gamelog << "Dealer deals\t" << flush;
    }
	dealer.DealCard(community);
    if( bSpectate ) HoldemUtil::PrintCard(gamelog, dealer.dealt.Suit,dealer.dealt.Value);


	int8 playerToReveal = PlayRound(5);

	if( playerToReveal == -1 ) return;

	PlayShowdown(playerToReveal);
}



void HoldemArena::DealHands()
{
    do{
        ++curDealer;
        curDealer %= nextNewPlayer;
    }while(!IsAlive(curDealer));

    myPot = 0;
    prevRoundPot = 0;

    curIndex = curDealer;

    if( bVerbose && bSpectate )
    {
        gamelog << "Next Dealer is " << p[curDealer]->GetIdent() << endl;
    }




	int8 dealtEach = 0;
	dealer.UndealAll();
	if(randRem != 0)
	{
		dealer.ShuffleDeck(randRem);
		randRem = 1;
#ifdef DEBUGASSERT
	}
	else
	{
        std::cerr << "YOU DIDN'T SHUFFLE" << endl;
        gamelog << "YOU DIDN'T SHUFFLE" << endl;
        exit(1);
#endif
    }

	while(dealtEach < 2)
	{
		incrIndex();

		Player& withP = *(p[curIndex]);

		if(withP.myMoney > 0)
		{
			if (!dealer.DealCard(withP.myHand)) gamelog << "OUT OF CARDS ERROR" << endl;
		}
		else
		{
			withP.lastBetSize = INVALID;
			withP.myBetSize = INVALID;
		}

		if(curDealer == curIndex) ++dealtEach;
	}
}

void HoldemArena::RefreshPlayers()
{
    if( bSpectate )
    {
        gamelog << "\n\n==========\nCHIP COUNT" << endl;
    }
#ifdef GRAPHMONEY
    scoreboard << handnum << flush;
#endif

    for(int8 i=0;i<nextNewPlayer;++i)
    {
        Player& withP = *(p[i]);
        if( withP.myMoney == 0 )
        {
            --livePlayers;
            withP.myMoney = -1;
            withP.myBetSize = INVALID;
            ///TODO: THAT LINE
            blinds->PlayerEliminated();
#ifdef GRAPHMONEY
            scoreboard << ",0";
#endif

        }
        else
        {

            withP.myBetSize = 0;

            if( bSpectate )
            {
                if( withP.GetMoney() > 0 ) gamelog << withP.GetIdent() << " now has " << withP.GetMoney() << endl;
            }
#ifdef GRAPHMONEY
            if( withP.GetMoney() < 0 )
            {
                scoreboard << ",0";
            }else
            {
                scoreboard.precision(10);
                scoreboard << "," << withP.GetMoney();
            }
#endif

        }
        withP.handBetTotal = 0;
        withP.allIn = INVALID;

        withP.myHand.SetEmpty();


    }
#ifdef DEBUGSPECIFIC
    ++handnum;
#else
#ifdef GRAPHMONEY
    scoreboard << endl;
    ++handnum;
#endif
#endif

}


Player* HoldemArena::PlayTable()
{
	if( p.empty() ) return 0;

	dealer.ShuffleDeck(static_cast<float64>(livePlayers));
	curIndex = 0;
	curDealer = 0;


	#ifdef DEBUGSPECIFIC

        randRem = 1;
        handnum = 1;
    #else

        #ifdef GRAPHMONEY

            handnum = 1;
            scoreboard.open(GRAPHMONEY);
            scoreboard << "Hand #";
            for(int8 i=0;i<nextNewPlayer;++i)
            {
                scoreboard << "," << (p[i])->GetIdent();
            }
            scoreboard << endl;
        #endif
    #endif

    #ifdef REPRODUCIBLE
        randRem = 1;
    #endif
/*
    #ifdef DEBUGSAVEGAME
        std::ofstream killfile(DEBUGSAVEGAME,std::ios::out | std::ios::trunc);
        killfile.close();
    #endif
*/


	while(livePlayers > 1)
	{
        DealHands();
		PlayGame();
        RefreshPlayers(); ///New Hand
	}

#ifdef GRAPHMONEY
    scoreboard.close();
#endif

#ifdef FORCEPAUSE
    gamelog << "Quit. (std::cin >> curIndex;)"<<endl;
    std::cin >> curIndex;
#endif

    for(int8 i=0;i<nextNewPlayer;++i)
    {
        Player *withP = (p[i]);
        if( withP->myMoney > 0 ) return withP;
    }
	return 0;

}


