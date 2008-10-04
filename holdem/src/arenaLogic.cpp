/***************************************************************************
 *   Copyright (C) 2008 by Joseph Huang                                    *
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
#include <fstream>
#include <algorithm>
#include <string.h>





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

void HoldemArena::compareAllHands(const int8 called, vector<ShowdownRep>& winners)
{
    HoldemArenaShowdown w(this,called,winners);

    while(w.bRoundState != '!')
    {
        if( bExternalDealer )
        {
            bool bMuck = p[curIndex]->bSync;

            if(bMuck) //then you must be human player, what's your hand?
            {
                const int16 INPUTLEN = 5;
                char inputBuf[INPUTLEN];


                std::cerr << p[curIndex]->GetIdent().c_str() << ", enter your cards (no whitespace)" << endl;
                std::cerr << "or enter nothing to muck: " << endl;
                std::cin.sync();
                std::cin.clear();

                if( std::cin.getline( inputBuf, INPUTLEN ) != 0 )
                {
                    if( 0 != inputBuf[0] )
                    {
                        bMuck = false;
                        p[curIndex]->myHand.AddToHand(ExternalQueryCard(std::cin));
                        p[curIndex]->myHand.AddToHand(ExternalQueryCard(std::cin));
                        #ifdef DEBUGSAVEGAME
                        if( !bLoadGame )
                        {
                            std::ofstream saveFile(DEBUGSAVEGAME,std::ios::app);
                            p[curIndex]->myHand.HandPlus::DisplayHand(saveFile);
                            saveFile.close();
                        }
                        #endif
                    }
                }//else, error on input

                std::cin.sync();
                std::cin.clear();
            }

            if( bMuck )
            {
                ShowdownRep comp(curIndex);
                comp.SetMuck();
                w.RevealHand(comp);
            }
            else
            {
                ShowdownRep comp(&(p[curIndex]->myHand), &community, curIndex);
                w.RevealHand(comp);
            }
        }
        else
        {
            ShowdownRep comp(&(p[curIndex]->myHand), &community, curIndex);
            w.RevealHand(comp);
        }

    }
}


void HoldemArena::PlayShowdown(const int8 called)
{
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

void HoldemArena::prepareRound(const int8 comSize)
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
			withP.myStrat->SeeCommunity(community, comSize);
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
//Why did I comment this out? I couldn't see the need for handBetTotal to be considered at all in this situation
//The subroutine begins with [allIn = myPot - myBetSum] which should take care of all action before this round
/*

		allInP.allIn += allInP.handBetTotal; //Account for your own bet

		allInP.myMoney -= allInP.handBetTotal;
*/
//The following line was added as a (working) substitute for the above block
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


int8 HoldemArena::PlayRound(const int8 comSize)
{
    HoldemArenaBetting b(this, comSize);


    while(b.bBetState == 'b')
    {
        b.MakeBet(p[curIndex]->myStrat->MakeBet());
    }

    return b.playerCalled;
}





DeckLocation HoldemArena::ExternalQueryCard(std::istream& s)
{
    DeckLocation userCard;
    #ifdef DEBUGSAVEGAME
    bool bNextCardSaved;
    bNextCardSaved = bLoadGame && (!loadFile.eof());
    bNextCardSaved = bNextCardSaved && (loadFile.rdbuf()->in_avail() > 0);
    bNextCardSaved = bNextCardSaved && (loadFile.is_open());


    while( bNextCardSaved )
    {
        int16 upcomingChar = loadFile.peek();
        if( upcomingChar == '\n' || upcomingChar == '\r'  || upcomingChar == ' ' )
        {
            loadFile.ignore(1);
            bNextCardSaved = bLoadGame && (loadFile.is_open()) && (!loadFile.eof()) && (loadFile.rdbuf()->in_avail() > 0);
        }else
        {
            bNextCardSaved = true;
            break;
        }
    }
    if( bNextCardSaved )
    {
        userCard.SetByIndex( HoldemUtil::ReadCard( loadFile ) );
    }else
    #endif
    {
        bLoadGame = false;
        userCard.SetByIndex( HoldemUtil::ReadCard( s ) );
    }
    return userCard;
}

void HoldemArena::DealHands()
{
	roundPlayers = livePlayers;

    myPot        = 0;
    prevRoundPot = 0;
    prevRoundFoldedPot = 0;
    myFoldedPot  = 0;

    curIndex = curDealer;

    if( bVerbose && bSpectate )
    {
        gamelog << endl << "Next Dealer is " << p[curDealer]->GetIdent() << endl;
    }


	if( bVerbose )
	{
		gamelog << "================================================================" << endl;
		gamelog << "============================New Hand" <<
		#if defined(GRAPHMONEY)
		" #"<< handnum <<
		#else
		"==" <<
		#endif //GRAPHMONEY, with #else
		"========================" << endl;

	}





    if( bExternalDealer )
    {


        #ifdef DEBUGHOLECARDS
            holecardsData <<
                    #if defined(GRAPHMONEY)
                    "############ Hand " << handnum << " " <<
                    #endif
            "############" << endl;

        #endif


        do
        {
            incrIndex();

            Player& withP = *(p[curIndex]);

            if(withP.myMoney > 0)
            {

                if(!(withP.bSync))
                {
                    std::cerr << withP.GetIdent().c_str() << ", enter your cards (no whitespace): " << endl;
                    std::cin.sync();
                    std::cin.clear();
                    withP.myHand.AddToHand(ExternalQueryCard(std::cin));
                    withP.myHand.AddToHand(ExternalQueryCard(std::cin));
                    std::cin.sync();
                    std::cin.clear();

                    #ifdef DEBUGSAVEGAME
                    if( !bLoadGame )
                    {
                        std::ofstream saveFile(DEBUGSAVEGAME,std::ios::app);
                        (withP.myHand).HandPlus::DisplayHand(saveFile);
                        saveFile.close();
                    }
                    #endif

                    (withP.myHand).HandPlus::DisplayHand(holecardsData);
                    holecardsData << withP.GetIdent().c_str() << endl;
                }
            }
            else
            {
                withP.lastBetSize = INVALID;
                withP.myBetSize = INVALID;
            }



        }while(curDealer != curIndex);
    }
    else
    {


        int8 dealtEach = 0;

        #ifdef DEBUGSAVEGAME
        if( bLoadGame )
        {
            bLoadGame = false;
        }else
        #endif
        {
            //Shuffle the deck here
            dealer.UndealAll();
            if(randRem != 0)
            {
                #ifdef DEBUGSAVEGAME
                std::ofstream shuffleData( DEBUGSAVEGAME, std::ios::app );
                dealer.LoggedShuffle(shuffleData, randRem);
                shuffleData << endl;
                shuffleData.close();
                    #if defined(DEBUGSAVEGAME_ALL) && defined(GRAPHMONEY)
                char handnumtxt/*[12] = "";
                char namebase*/[23+12] = "./" DEBUGSAVEGAME_ALL "/" DEBUGSAVEGAME "-";

                FileNumberString( handnum , handnumtxt + strlen(handnumtxt) );
                handnumtxt[23+12-1] = '\0'; //just to be safe

                shuffleData.open( handnumtxt , std::ios::app );
                dealer.LogDeckState( shuffleData );
                shuffleData << endl;
                shuffleData.close();
                    #endif
                #else
                dealer.ShuffleDeck( randRem );
                #endif


        #ifdef DEBUGASSERT
            }
            else
            {
                std::cerr << "YOU DIDN'T SHUFFLE" << endl;
                gamelog << "YOU DIDN'T SHUFFLE" << endl;
                exit(1);
        #endif
            }
        }



        #ifdef DEBUGHOLECARDS
            holecardsData <<
                    #if defined(GRAPHMONEY)
                    "############ Hand " << handnum << " " <<
                    #endif
            "############" << endl;

        #endif

        while(dealtEach < 2)
        {
            incrIndex();

            Player& withP = *(p[curIndex]);

            if(withP.myMoney > 0)
            {
                if (!dealer.DealCard(withP.myHand)) gamelog << "OUT OF CARDS ERROR" << endl;

                    #ifdef DEBUGHOLECARDS
                        if( dealtEach == 1 )
                        {
                            (withP.myHand).HandPlus::DisplayHand(holecardsData);
                            holecardsData << withP.GetIdent().c_str() << endl;
            //                holecardsData << endl << endl;
                        }
                    #endif

            }
            else
            {
                withP.lastBetSize = INVALID;
                withP.myBetSize = INVALID;
            }

            if(curDealer == curIndex) ++dealtEach;
        }
    }//if EXTERNAL_DEALER, else


randRem = 1;

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
#ifdef GRAPHMONEY
    scoreboard << handnum << flush;
#endif

    for(int8 i=0;i<nextNewPlayer;++i)
    {
        Player& withP = *(p[i]);
        if( withP.myMoney == 0 )
        {
            withP.myStrat->FinishHand();
        }
    }

    bool bPlayerElimBlindUp = false;


    for(int8 i=0;i<nextNewPlayer;++i)
    {
        Player& withP = *(p[i]);
        if( withP.myMoney == 0 )
        {
            --livePlayers;
            withP.myMoney = -1;
            withP.myBetSize = INVALID;
            bPlayerElimBlindUp |= blinds->PlayerEliminated();

#ifdef GRAPHMONEY
            scoreboard << ",0";
#endif

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

#ifdef GRAPHMONEY
            if( withP.GetMoney() < 0 )
            {
                scoreboard << ",0";
            }else
            {
                scoreboard << "," << flush;
                scoreboard.precision(10);
                scoreboard << withP.myMoney << flush;
            }
#endif

        }
        withP.handBetTotal = 0;
        withP.allIn = INVALID;

        withP.myHand.SetEmpty();


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
        if( withP.myMoney > 0 )
        {
            withP.myStrat->FinishHand();
        }
    }


#ifdef GRAPHMONEY
    scoreboard << endl;
    ++handnum;
#endif


    if( bPlayerElimBlindUp && bVerbose )
    {
        gamelog << "Blinds increased to " << blinds->mySmallBlind << "/" << blinds->myBigBlind << endl;
    }


    do{
        ++curDealer;
        curDealer %= nextNewPlayer;
    }while(!IsAlive(curDealer));



#ifdef DEBUGSAVEGAME
    #ifdef RELOAD_LAST_HAND
	        if( NumberAtTable() > 1 )
    #endif
        	{  saveState();  }
#endif


}



