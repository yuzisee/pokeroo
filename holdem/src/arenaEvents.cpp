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

#include "arena.h"

#include <algorithm>
using std::sort;

void HoldemArenaBetting::finishBettingRound()
{
/*
    float64 & highBet = myTable->highBet;
    int8 & playersInHand = myTable->playersInHand;
    int8 & playersAllIn = myTable->playersAllIn;
    int8 & curIndex = myTable->curIndex;
    float64 & myPot = myTable->myPot;
    vector<Player*> &p = myTable->p;
*/

///If the round goes check-check-check, it technically makes/means the dealer (is) the higher better. We want the NEXT person.
/*
http://www.playwinningpoker.com/poker/rules/basics/
If everyone checks (or is all-in) on the final betting round, the player who acted first is the first to show the hand. If there is wagering on the final betting round, the last player to take aggressive action by a bet or raise is the first to show the hand. In order to speed up the game, a player holding a probable winner is encouraged to show the hand without delay. If there is a side pot, players involved in the side pot should show their hands before anyone all-in for only the main pot.
*/
	if ( highBet == 0 )
	{
	    if( GetNumberInHand() == playersAllIn)
	    {///Find the first player after the dealer
            do
            {
                incrIndex(highestBetter);
            }while( PlayerAllIn(*(p[highestBetter])) < 0);
	    }else if( GetNumberInHand() - 1 == playersAllIn)
	    {///Find the player that is not allin
	        do
            {
                incrIndex(highestBetter);
            }while( (!(myTable->IsAlive(highestBetter))) || PlayerAllIn(*(p[highestBetter])) < 0);//Obviously you have to skip dead people too
	    }else
	    {///Find the first player to check
            do
            {
                incrIndex(highestBetter);
            }while( !IsInHand(highestBetter) );
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

		Player& withP = *(p[curIndex]);

		resolveActions(withP);
        incrIndex();
	}while( curIndex != highestBetter);

	///At this point all "THIS ROUND ONLY" values are useless EXCEPT allIn

	delete [] allInsNow;

	myBetSum = 0;
	highBet = 0;



	if( GetNumberInHand() > 1 )
	{
		//Except for showdown order, this is only necessarily NOT 'return -1;'
		playerCalled = highestBetter;
        bBetState = 'C';
		return;
	}
//	else ///End hand if all folded
	{
		if( GetNumberInHand() == 1 )
		{



		    ///What if the player that folds is the highest better at the moment?
            ///This can happen on blinds
            //std::cout << p[highestBetter]->GetIdent() << " bet the most first, but now is " << p[highestBetter]->GetBetSize() << endl;
            //if( myTable->HasFolded(highestBetter) )
            if( PlayerBet(*p[highestBetter]) != highBet ) //We would check HasFolded, except resolveActions() changes myBet to INVALID if it was FOLDED. This is safer, I think.
            {///If it's due to blinds, figure out who really wins.
                int8 findHighestBetter = highestBetter;

                incrIndex(findHighestBetter);
                while(findHighestBetter != curIndex)
                {
                    if( PlayerBet(*(p[findHighestBetter])) == highBet )
                    {
                        highestBetter = findHighestBetter;
                    }
                    incrIndex(findHighestBetter);
                }
            }

            /*if(handnum == 9 )
            {
                gamelog << "POT << " << myPot << endl;
                gamelog << "money before << " << p[highestBetter]->myMoney << endl;
                gamelog << "handBetTotal << " << p[highestBetter]->handBetTotal << endl;
                gamelog << "allIn << " << p[highestBetter]->allIn << endl;
            }*/

            ///When the highest better is allIn, it skips preparations on certain resolveActions
            if( PlayerAllIn(*(p[highestBetter])) >= 0 )
            {
                myPot = PlayerAllIn(*(p[highestBetter]));
            }


            const float64 highContribution = PlayerHandBetTotal(*(p[highestBetter]));
			//What if you fold to an all-in? I think it will work just fine.
			if( bVerbose )
			{
			    gamelog << endl;
				gamelog << "All fold! " << p[highestBetter]->GetIdent() <<
				" wins $" << (   myPot - highContribution   ) << endl;
			}
			float64 rh = static_cast<float64>(highestBetter)+2;
			randRem /= myPot*highContribution+rh;
			randRem *= rh;
			PlayerMoney(*(p[highestBetter]))+= myPot;
#ifdef DEBUGASSERT
		}else
		{
            std::cerr << "Too many people folded! (Even the winner!?)" << endl;
            gamelog << "Too many people folded! (Even the winner!?)" << endl;
            exit(1);
#endif
        }
		//playerCalled = -1;
		bBetState = 'F';
		return;
	}

}

void HoldemArenaBetting::startBettingRound()
{
    ///----------
    prepareRound(comSize);
    ///----------


    forcedBetSum = 0;
    blindOnlySum = 0;
	bBlinds      = curDealer;	//Used to account for soft-bets:
								//ie, if called, you can still reraise.
								//BUT, it also handles the check-check-check

    if( comSize == 0 && myTable->GetNumberInHand() == 2)
    {
        do
		{
			incrIndex(bBlinds); ///The dealer always bets last. However, in heads-up the dealer posts the SMALL blind (the first blind instead)
		}while( ! IsInHand(bBlinds) );
    }


	highBet = 0;
	highestBetter = bBlinds;


	lastRaise = myTable->GetBigBlind();





/*
	Rules for allInsNow:
		it contains a list of players that have bet all in but as of
		yet do not know how much they can win before a side pot takes place
	"Maybe we can use handBetTotal to determine. Aha! No need."
		Maybe Use (myPot - myBetSum) to account for previous rounds?


*/


	allInsNow = new int8[GetTotalPlayers()];
	allInsNowCount = 0;
/*
    const bool bVerbose = myTable->bVerbose;

    int8 & curIndex = myTable->curIndex;
    int8 & playersAllIn = myTable->playersAllIn;
    vector<Player*> &p = myTable->p;
*/

	///Weird stuff to simulate blind bets
	if( comSize == 0 && myTable->GetNumberInHand() >= 2)
	{
        curIndex = bBlinds;
	    do
		{
			incrIndex();
		}while( ! IsInHand(curIndex) );

        int8 smallBlindIndex = curIndex;
        Player& withP1 = *p[curIndex];
        PlayerBet(withP1) = myTable->GetSmallBlind();

        if( PlayerBet(withP1) > PlayerMoney(withP1) )
        {
            PlayerBet(withP1) = PlayerMoney(withP1);
            PlayerAllIn(withP1) = PlayerMoney(withP1);
                //we must remember allIn as above: it's what we can win/person

            ++playersAllIn;

            allInsNow[allInsNowCount] = curIndex;
            ++allInsNowCount;
        }

            if( bVerbose )
            {
                broadcastCurrentMove(curIndex, PlayerBet(withP1), PlayerBet(withP1), 0, 1, false, PlayerAllIn(withP1) > 0);
            }

		do
		{
			incrIndex();
		}while( ! IsInHand(curIndex) );

		Player& withP2 = *p[curIndex];
		PlayerBet(withP2) = myTable->GetBigBlind();

        if( PlayerBet(withP2) > PlayerMoney(withP2) )
        {
            PlayerBet(withP2) = PlayerMoney(withP2);
            PlayerAllIn(withP2) = PlayerMoney(withP2);
                //we must remember allIn as above: it's what we can win/person

            ++playersAllIn;

            allInsNow[allInsNowCount] = curIndex;
            ++allInsNowCount;
        }

            if( bVerbose )
            {
                broadcastCurrentMove(curIndex, PlayerBet(withP2),PlayerBet(withP2), 0, 2, false, PlayerAllIn(withP2) > 0);
            }

		bBlinds = curIndex;

        forcedBetSum = PlayerBet(withP1)+PlayerBet(withP2);
        blindOnlySum = forcedBetSum;
		addBets(forcedBetSum);

		highestBetter = curIndex;
		highBet = PlayerBet(withP2);
		if( PlayerBet(withP1) > PlayerBet(withP2) )
		{
		    highestBetter = smallBlindIndex;
            highBet = PlayerBet(withP1);
		}

        #ifdef OLD_DISPLAY_STYLE
        if( bVerbose )
        {
            gamelog << "(Blinds posted)" << endl << endl;
        }
        #endif

	}
/*	else
	{
		//bBlinds = -1;
		incrIndex(highestBetter);
	}*/

	incrIndex();

    //if( playersAllIn > 0 ) gamelog << "<GetNumberInHand(),playersAllIn,allInsNowCount>=<" << (int)(GetNumberInHand()) << "," << (int)playersAllIn << "," << (int)allInsNowCount << ">" << endl;
    if( !(GetNumberInHand() - playersAllIn + allInsNowCount > 1) )
    {
        finishBettingRound();
    }else{

        while (!( myTable->CanStillBet(curIndex) ))
        {
            incrPlayerNumber(*(p[curIndex]));

            if(!(
                 (curIndex != highestBetter || bBlinds == highestBetter)
                 &&
                 (GetNumberInHand() - playersAllIn + allInsNowCount > 1)
                 )
               )
            {
                finishBettingRound();
                return;
            }
        }
    }

}

void HoldemArenaBetting::incrPlayerNumber(Player& currentPlayer)
{
    /*
    int8 & curIndex = myTable->curIndex;
    */
#ifdef DEBUGALLINS
gamelog << p[highestBetter]->GetIdent() << " last to raise" << endl;
#endif


#ifdef DEBUGALLINS
gamelog << curIndex << " == " << bBlinds << endl;
if ( bBlinds != -1 )
{
	gamelog << "Why is bBlinds " << p[bBlinds]->GetIdent() <<
	"? Is it comSize = " << comSize << endl;
}
#endif



#ifdef DEBUGALLINS
gamelog << p[curIndex]->GetIdent() << " up next... same as before?" << endl;
#endif



    if( curIndex == bBlinds )
    {
        ///Account for the weird blind structure where a call can be
        ///reraised by a person in the blinds

        ///But it must also maintain the check-check-check events

        bBlinds = -1;
        if ( PlayerBet(currentPlayer) > PlayerLastBet(currentPlayer)
                ||
            curIndex != highestBetter )
        {//You only extend play if there is additional bet here
            incrIndex();
        }


    }
    else
    {
        if( myTable->GetNumberInHand() == 1 )
        {
            bBlinds = -1;
        }
        incrIndex();
    }

    //If this player's last bet was a blind, it wasn't counted in RoundBetsTotal. Now his entire bet should count.
    if( bBlinds != -1 )
    {//If we haven't gone once around the table betting yet,
        Player& nextPlayer = *p[curIndex];

        if( PlayerBet(nextPlayer) > 0 )
        {
            forcedBetSum -= PlayerBet(nextPlayer);
            blindOnlySum -= PlayerBet(nextPlayer);
        }

        if( forcedBetSum < 0 ) forcedBetSum = 0;
        if( blindOnlySum < 0 ) blindOnlySum = 0;
    }
}


void HoldemArenaBetting::MakeBet(float64 betSize)
{

    if( bBetState != 'b' ) return;

/*
    int8 & curIndex = myTable->curIndex;
    float64 & highBet = myTable->highBet;
    int8 & playersAllIn = myTable->playersAllIn;
    int8 & playersInHand = myTable->playersInHand;
    vector<Player*> &p = myTable->p;
    float64 & randRem = myTable->randRem;
*/

	///---------------------------------
	///Action time!
	///---------------------------------



		///The loop continues until:
		/*
			Betting gets back to the highest better, but it's not the big blind
			OR
			There is one player left in the hand
		*/

#ifdef DEBUGALLINS
gamelog << IsInHand(curIndex) << " && " << (p[curIndex]->allIn == INVALID) << endl;
#endif


        Player& withP = *(p[curIndex]);

			///Get a bet from the player
			PlayerLastBet(withP) = PlayerBet(withP);


			if ( PlayerMoney(withP) == 0 )
			{
				PlayerBet(withP) = 0;
			}
			else
			{
				PlayerBet(withP) = betSize;
			}

#ifdef DEBUGALLINS
gamelog << "Entered, " << PlayerBet(withP) << " vs " << highBet << endl;
#endif

			///Decide what to do with the bet
			if( PlayerBet(withP) >= PlayerMoney(withP) )
			{
						randRem += (PlayerLastBet(withP)+1.0) / PlayerBet(withP) ;

				///ALL-IN. Notice that allIn combines handBet with myBet
				//? //However, myBetSize is now the WHOLE HAND.
				PlayerBet(withP) = PlayerMoney(withP) ;
				PlayerAllIn(withP)  = PlayerHandBetTotal(withP) + PlayerBet(withP);
					//we must remember allIn as above: it's what we can win/person

				++playersAllIn;

				allInsNow[allInsNowCount] = curIndex;
				++allInsNowCount;
			}
			else
			{//Not all-in
				if( PlayerBet(withP) < highBet )
				{///Player folds.
					randRem /= myBetSum + GetNumberInHand();

                    myFoldedPot += PlayerHandBetTotal( withP ) + PlayerLastBet(withP);

					///You're taking money away here. addBets happenned a while ago
					PlayerHandBetTotal( withP )= PlayerLastBet(withP);
							//CAUTION: TRICKY USE OF handBetTotal
					PlayerMoney(withP) -= PlayerLastBet(withP);
                    forcedBetSum += PlayerLastBet(withP);

					PlayerBet(withP) = HoldemArena::FOLDED;
					--playersInHand;
				}else if( PlayerBet(withP) > highBet && PlayerBet(withP) < highBet + myTable->GetMinRaise() )
				{///You raised less than the MinRaise

                        if( bVerbose )
                        {
                            gamelog << "The minimum raise bet is by " << myTable->GetMinRaise() << " to " << highBet + myTable->GetMinRaise() << endl;
                        }

                    const float64 distFromCall = PlayerBet(withP) - highBet;
                    const float64 distFromMinRaise = myTable->GetMinRaise() - distFromCall;

                    if( distFromCall > distFromMinRaise )
                    {
                        PlayerBet(withP) = highBet + myTable->GetMinRaise();
                    }else
                    {
                        PlayerBet(withP) = highBet;
                    }
				}
			}
///TODO Reraises need to say RERAISE.
			broadcastCurrentMove(curIndex, PlayerBet(withP), PlayerBet(withP) - PlayerLastBet(withP), highBet, 0
					, curIndex == bBlinds && comSize == 0 && curIndex == highestBetter,PlayerAllIn(withP) > 0);

			if( PlayerBet(withP) >= highBet )
			{
				addBets(PlayerBet(withP) - PlayerLastBet(withP));

				if( PlayerBet(withP) > highBet )
				{//It was a raise
                    if( PlayerBet(withP) - highBet > lastRaise )
                    {//We must take this into account because you can bet all-in without obeying this rule.
                        lastRaise = PlayerBet(withP) - highBet;
                    }
					highBet = PlayerBet(withP);
					highestBetter = curIndex;

                        randRem *= -1- (myBetSum)*(myPot);
				}
                    randRem /= myPot + curIndex;


			}
			else if( PlayerAllIn(withP) > 0 )
			{   ///If you are going all in with LESS money than to call
                ///The money you have left over still NEEDs to be added to the pot
                ///Without this section, it wouldn't happen
			    addBets(PlayerBet(withP) - PlayerLastBet(withP));

                    randRem += (myBetSum)/(curIndex+2.5);
			}

#ifdef DEBUGALLINS
gamelog << p[curIndex]->GetIdent() << " up now" << endl;
#endif


        do
		{
		    incrPlayerNumber(*(p[curIndex]));

            if(!(
                    (curIndex != highestBetter || bBlinds == highestBetter)
                    &&
                    (playersInHand - playersAllIn + allInsNowCount > 1)
                  )
                 )
                {
                     finishBettingRound();
                     return ;
                }
		}while (!( myTable->CanStillBet(curIndex) ));

        //bRoundState = 'b';

}

HoldemArenaBetting::~HoldemArenaBetting()
{

}



void HoldemArenaShowdown::startShowdown()
{
    curIndex = called;


    while ( !IsInHand(curIndex) )
    {
        incrIndex();

        if(curIndex == called)
        {
            startAllIns();
            return;
        }
    }

}

void HoldemArenaShowdown::startAllIns()
{
    //gamelog << "The following players are all in..." << endl;
	///Non all-in players show first,
	///All-in players are manditorily showing afterwards.
    //float64 topAllIn=myPot;

    bRoundState = 'a';
    bool bFoundAllIns = false;

    do
	{
	    Player& withP = *(p[curIndex]);

        //gamelog << p[curIndex]->GetIdent() << "\t now " << p[curIndex]->allIn << endl;
		if ( PlayerAllIn(withP) >= 0 && !IsInHand(curIndex)) //If all in with no money remaining
		{
            ShowdownRep comp(curIndex);
            comp.valueset=0;
            comp.strength=0;
            comp.revtiebreak = PlayerAllIn(withP);
            allInRevealOrder.push_back(comp);
            bFoundAllIns = true;
		}
		incrIndex();

	}while(curIndex != called);

    ///All-in players reveal in descending order of stack size

    sort(allInRevealOrder.begin(),allInRevealOrder.end());
    ///Note: When the vector is sorted it will be in descending order of stack size, since
    ///revtiebreak is always sorted in the OPPOSITE direction

    if( bFoundAllIns == false )
    {
        finishShowdown();
        return;
    }

    nextReveal = allInRevealOrder.begin();
    curIndex = nextReveal->playerIndex;

}

void HoldemArenaShowdown::finishShowdown()
{
    bRoundState = '!';
    curIndex = called;
}

void HoldemArenaShowdown::MuckHand()
{
    ShowdownRep worstHand(curIndex); ///worstHand defaults to the worst hand. You only need to initialize playerIndex
    RevealHand(worstHand);
}

void HoldemArenaShowdown::RevealHandAllIns(const ShowdownRep& comp)
{
    Player& withP = *p[curIndex];


    if( comp > best || comp == best ) //Better hand or tie
    {
        broadcastHand(PlayerHand(withP),curIndex);
        if( bVerbose )
        {
            gamelog << endl << withP.GetIdent() << flush;
            if( comp == best )
                gamelog << " also has: " << flush;
            else
                gamelog << " is ahead with: " << flush;
            HandPlus viewHand;
            viewHand.SetUnique(PlayerHand(withP));
            viewHand.DisplayHand(gamelog);
            gamelog << endl << "Trying to stay alive, makes " << flush;
            //#ifdef OLD_DISPLAY_STYLE
            comp.DisplayHandBig(gamelog);
            //#else
            //comp.DisplayHandText(gamelog);
            //#endif
        }
        winners.push_back(comp);
        best = comp;
        //topAllIn = withP.allIn;
    }
    ///His hand was worse, but since he is all-in he MUST show his hand.
    else
        /*if( withP.allIn > topAllIn )
    {///May qualify for a side pot! (maybe)
            broadcastHand(withP.myHand);
            if( bVerbose )
            {
                gamelog << endl << withP.GetIdent() << flush;
                gamelog << " is ahead with: " << flush;
                HandPlus viewHand;
                viewHand.SetUnique(withP.myHand);
                viewHand.ShowHand(false);
                gamelog << endl << "Trying to stay alive, makes" << flush;
                comp.DisplayHandBig(gamelog);
            }
            winners.push_back(comp);
    }
        else*/
    {///Distinctly defeated
     //  http://www.texasholdem-poker.com/holdem_rules.php
        broadcastHand(PlayerHand(withP),curIndex);
        if( bVerbose )
        {
            gamelog << endl << withP.GetIdent() << flush;
            gamelog << " turns over " << flush;
            HandPlus viewHand;
            viewHand.SetUnique(PlayerHand(withP));
            viewHand.DisplayHand(gamelog);
            gamelog << endl << "Is eliminated after making only" << flush;
            //#ifdef OLD_DISPLAY_STYLE
            comp.DisplayHandBig(gamelog);
            //#else
            //comp.DisplayHandText(gamelog);
            //#endif
        }
    }

    ++nextReveal;

    if(nextReveal == allInRevealOrder.end())
    {
        finishShowdown();
    }else
    {
        curIndex = nextReveal->playerIndex;
    }

}

void HoldemArenaShowdown::RevealHandMain(const ShowdownRep& comp)
{

///At this point curIndex is STILL IN THE HAND (not all in)


//gamelog << p[curIndex]->GetIdent() << "\tallIn=" << p[curIndex]->allIn << endl;

		    Player& withP = *p[curIndex];
            //const ShowdownRep comp(&(withP.myHand), &community, curIndex);

			if( comp > best ) //best hand yet
			{

				///We set the allIn, since this player "IsInHand()"
				PlayerAllIn(withP) = myPot;

				broadcastHand(PlayerHand(withP),curIndex);
				if( bVerbose )
				{


					gamelog << endl << withP.GetIdent() << flush;
					gamelog << " reveals: " << flush;
                    HandPlus viewHand;
					viewHand.SetUnique(PlayerHand(withP));
					viewHand.DisplayHand(gamelog);
					gamelog << endl << "Making," << flush;
					//#ifdef OLD_DISPLAY_STYLE
					comp.DisplayHandBig(gamelog);
					//#else
					//comp.DisplayHandText(gamelog);
					//#endif
				}

				//winnerCount = 1;
				winners.clear();
				winners.push_back(comp);
				best = comp;

			}
			else if( comp == best ) //can only split, if not beaten later
			{

				broadcastHand(PlayerHand(withP),curIndex);
				if( bVerbose )
				{
					HandPlus viewHand;

					gamelog << endl << withP.GetIdent() << flush;
					gamelog << " turns up: " << endl;
					viewHand.SetUnique(PlayerHand(withP));
					#ifdef OLD_DISPLAY_STYLE
					viewHand.DisplayHandBig(gamelog);
					#else
					viewHand.DisplayHand(gamelog);
					#endif
					gamelog << "Split... " << flush;
					//#ifdef OLD_DISPLAY_STYLE
					comp.DisplayHand(gamelog);
					//#else
					//comp.DisplayHandText(gamelog);
					//#endif
					gamelog << endl;
				}

				winners.push_back(comp);
				//winnerCount++;
			}
			else
			{
				gamelog << endl << withP.GetIdent() << " mucks " << endl;
			}



    do{
        incrIndex();

        if(curIndex == called)
        {
            startAllIns();
            return;
        }
    }while ( !IsInHand(curIndex) );

}

void HoldemArenaShowdown::RevealHand(const ShowdownRep& comp)
{
    if( bRoundState == '!' ) return;

    switch( bRoundState )
    {
        case 'w':
            RevealHandMain(comp);
            break;
        case 'a':
            RevealHandAllIns(comp);
            break;
    }
}



