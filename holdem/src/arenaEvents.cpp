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

#include "arenaEvents.h"

#undef DELAYED_NONFOLD_UPDATE
#define IMMEDIATE_NONFOLD_UPDATE

#if defined(DELAYED_NONFOLD_UPDATE) && defined(IMMEDIATE_NONFOLD_UPDATE)
#error "Please define only one of DELAYED_NONFOLD_UPDATE or IMMEDIATE_NONFOLD_UPDATE"
#endif

#include <sstream>
#include <algorithm>
using std::sort;

void HoldemArenaBetting::finishBettingRound()
{


///If the round goes check-check-check, as-is highestBetter would suggest that the dealer (is) the higher better.
///We actually want the NEXT person, though.
	///---------------------------------
	///Handle a special case: highestBetter
	///---------------------------------
/*
http://www.playwinningpoker.com/poker/rules/basics/
If everyone checks (or is all-in) on the final betting round, the player who acted first is the first to show the hand. If there is wagering on the final betting round, the last player to take aggressive action by a bet or raise is the first to show the hand. In order to speed up the game, a player holding a probable winner is encouraged to show the hand without delay. If there is a side pot, players involved in the side pot should show their hands before anyone all-in for only the main pot.
*/
	if ( highBet == 0 )
	{
	    if( GetNumberInHandInclAllIn() == playersAllIn)
	    {///Find the first player after the dealer
            do
            {
                incrIndex(highestBetter);
            }while( PlayerAllIn(*(p[highestBetter])) < 0);
	    }else if( GetNumberInHandExclAllIn() == 1)
	    {///Find the player that is not allin -- in this (non-standard) implementation, all-in players ALWAYS show their cards
	        do
            {
                incrIndex(highestBetter);
            }while( (!(myTable->IsAlive(highestBetter))) || PlayerAllIn(*(p[highestBetter])) > 0);//Obviously you have to skip dead people too
	    }else
	    {///Find the first player to check
            do
            {
                incrIndex(highestBetter);
            }while( !IsInHand(highestBetter) );
	    }
	}
	///INVARIANT: highestBetter is the "called" player (ie. must show his/her hand first in the showdown)

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
    allInsNow = 0;
    

	myBetSum = 0;
	highBet = 0;



	if( GetNumberInHandInclAllIn() > 1 )
	{
		//Except for showdown order, this is only necessarily NOT 'return -1;'
		playerCalled = highestBetter;
        bBetState = 'C';
		return;
	}


    ///End hand if all folded
    if( GetNumberInHandInclAllIn() == 1 )
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


        ///When the highest better is allIn, it skips preparations on certain resolveActions
        if( PlayerAllIn(*(p[highestBetter])) >= 0 )
        {
            myPot = PlayerAllIn(*(p[highestBetter]));
        }


        const float64 highContribution = PlayerHandBetTotal(*(p[highestBetter]));
        //What if you fold to an all-in? I think it will work just fine.

        float64 rh = static_cast<float64>(highestBetter)+2;
        randRem /= myPot*highContribution+rh;
        randRem *= rh;
        PlayerMoney(*(p[highestBetter]))+= myPot;
#ifdef DEBUGASSERT
    }else
    {
        std::cerr << "Too many people folded! (Even the winner!?)" << endl;
        exit(1);
#endif
    }
    //playerCalled = -1;
    bBetState = 'F';
    return;


}

void HoldemArenaBetting::allInsAppend(const playernumber_t& a)
{
	++playersAllIn;

    allInsNow[allInsNowCount] = a;
    ++allInsNowCount;
}

void HoldemArenaBetting::allInsReset()
{
	allInsNow = new int8[GetTotalPlayers()];
	allInsNowCount = 0;
}

		
bool HoldemArenaBetting::IsHeadsUp() const
{
	return ((comSize == 0) && (myTable->NumberStartedRoundInclAllIn() == 2));
}


void HoldemArenaBetting::startBettingRound(std::ostream * const spectateLog)
{



    forcedBetSum = 0;
    blindOnlySum = 0;
	bBlinds      = curDealer;	//Used to account for soft-bets:
								//ie, if called, you can still reraise.
								//BUT, it also handles the check-check-check
	bHighBetCalled = false;     //BUT! If the blind is never called, bBlinds is meaningless (eg. all players in the hand are all-in less than the big blind)

	curHighBlind = -1;

    if( IsHeadsUp() )
    {
        do
		{
			incrIndex(bBlinds); ///The dealer always bets last. However, in heads-up the dealer posts the SMALL blind (the first blind instead)
		}while( ! IsInHand(bBlinds) );
    }


	highBet = 0;
	highestBetter = bBlinds;


	lastRaise = myTable->GetBlindValues().GetBigBlind();





/*
	Rules for allInsNow:
		it contains a list of players that have bet all in but as of
		yet do not know how much they can win before a side pot takes place
	"Maybe we can use handBetTotal to determine. Aha! No need."
		Maybe Use (myPot - myBetSum) to account for previous rounds?


*/


	allInsReset();



	///Weird stuff to simulate blind bets
	if( comSize == 0 && myTable->NumberInHandInclAllIn() >= 2)
	{
        curIndex = bBlinds;
	    do
		{
			incrIndex();
		}while( ! IsInHand(curIndex) );

        int8 smallBlindIndex = curIndex;
        Player& withP1 = *p[curIndex];
        PlayerBet(withP1) = myTable->GetBlindValues().GetSmallBlind();

        if( PlayerBet(withP1) >= PlayerMoney(withP1) - GetChipDenom()/2.0 )
        {
            PlayerBet(withP1) = PlayerMoney(withP1);
            PlayerAllIn(withP1) = PlayerMoney(withP1);
                //we must remember allIn as above: it's what we can win/person

			allInsAppend(curIndex);
        }

        PlayerForcedBetTotal(withP1) = PlayerBet(withP1);

            //if( bVerbose ) //Blinds are broadcasted always regardless of bVerbose, because bots may need to see it.
            {
                const HoldemAction currentMove(withP1.GetIdent(), myPot, withP1.GetMoney()
                                               ,
                                               curIndex, PlayerBet(withP1), PlayerBet(withP1), 0.0, 1, false, PlayerAllIn(withP1) > 0);
                broadcastCurrentMove(currentMove);
                if (spectateLog) {
                    HoldemArena::ToString(currentMove, *spectateLog);
                }
            }

		do
		{
			incrIndex();
		}while( ! IsInHand(curIndex) );

		Player& withP2 = *p[curIndex];
		PlayerBet(withP2) = myTable->GetBlindValues().GetBigBlind();

        if( PlayerBet(withP2) >= PlayerMoney(withP2) - GetChipDenom()/2.0 )
        {
            PlayerBet(withP2) = PlayerMoney(withP2);
            PlayerAllIn(withP2) = PlayerMoney(withP2);
                //we must remember allIn as above: it's what we can win/person

            allInsAppend(curIndex);
        }
        addBets(PlayerBet(withP1));

        PlayerForcedBetTotal(withP2) = PlayerBet(withP2);

            //if( bVerbose ) //Blinds are broadcasted always regardless of bVerbose, because bots may need to see it.
            {
                const HoldemAction currentMove(withP2.GetIdent(), myPot, withP2.GetMoney()
                                               ,
                                               curIndex, PlayerBet(withP2),PlayerBet(withP2), 0.0, 2, false, PlayerAllIn(withP2) > 0);
                broadcastCurrentMove(currentMove);
                if (spectateLog) {
                    HoldemArena::ToString(currentMove, *spectateLog);
                }
            }


		bBlinds = curIndex;

        forcedBetSum = PlayerBet(withP1)+PlayerBet(withP2);
        blindOnlySum = forcedBetSum;
		addBets(PlayerBet(withP2));

		highestBetter = curIndex;
		highBet = PlayerBet(withP2);
		if( PlayerBet(withP1) > PlayerBet(withP2) )
		{
		    highestBetter = smallBlindIndex;
            highBet = PlayerBet(withP1);
		}


        curHighBlind = highestBetter;

        #ifdef OLD_DISPLAY_STYLE
        if( bVerbose )
        {
            gamelog << "(Blinds posted)" << endl << endl;
        }
        #endif

        //POSTCONDITION: Player::forcedBetTotal is assigned correctly for BB and SB players
        //POSTCONDITION: highestBetter is correct as of after both blinds are posted (accounting for short-stack all-in's etc.)
        //POSTCONDITION: highBet is correct as of after both blinds are posted (accounting for short-stack all-in's etc.)
        //POSTCONDITION: bBlinds is the BB player's index
        //POSTCONDITION: pot contains both blinds, accounting for short-stack all-in's etc.
	}


	incrIndex();

    if( !(GetNumberInHandExclAllIn() + allInsNowCount > 1) )
    {
        finishBettingRound();
    }else{

        while (!( myTable->CanStillBet(curIndex) ))
        {
            incrPlayerNumber(*(p[curIndex]));

            if(readyToFinish())
            {
                finishBettingRound();
                return;
            }
        }
    }

}

void HoldemArenaBetting::incrPlayerNumber(Player& currentPlayer)
{
///The goal of this function is to increment curIndex to the next player on his left.
///incrIndex is used to increment curIndex while wrapping back around to 0 as needed.
///incrPlayerNumber() is generally used in tandem with readyToFinish(), therefore:
///	 (A)This function is also responsible for updating bBlinds, as the variable is used to determine if subsequent incrPlayerNumber calls will take place
///  (B)This function must leave curIndex at the player index of the highest better if readyToFinish will report that the round has ended


    if( curIndex == bBlinds )
    {
		///See note (A) above:
        //Account for the weird blind structure where a call can be
        //reraised by a person in the blinds

        //But it must also maintain the check-check-check events

        bBlinds = -1;
        if ( PlayerBet(currentPlayer) > PlayerLastBet(currentPlayer)
                ||
            curIndex != highestBetter )
        {///You only extend play if there is additional bet here: See note (B) above
            incrIndex();
        }


    }
    else
    {
        if( GetNumberInHandInclAllIn() == 1 )
        {
            bBlinds = -1;
        }
        incrIndex();
    }
    ///INVARIANT: curIndex is the next player to bet

    //If this player's last bet was a blind, it wasn't counted in RoundBetsTotal. Now his entire bet should count.
    if( bBlinds != -1 )
    {//If we haven't gone once around the table betting yet,
        Player& nextPlayer = *p[curIndex];

        if( PlayerBet(nextPlayer) > 0 )
        {
            forcedBetSum -= PlayerBet(nextPlayer);
            blindOnlySum -= PlayerBet(nextPlayer);
            if( curIndex == curHighBlind ) curHighBlind = -1; //Reset curHighBlind. Nobody is left who "seems to have called" but hasn't.
        }

        if( forcedBetSum < 0 ) forcedBetSum = 0;
        if( blindOnlySum < 0 ) blindOnlySum = 0;
    }
}



HoldemAction HoldemArenaBetting::MakeBet(float64 betSize, struct MinRaiseError *out)
{
    struct MinRaiseError &message = *out;

    if( bBetState != 'b' ) return HoldemAction(false);


	///---------------------------------
	///Action time!
	///---------------------------------



		///The loop continues until:
		/*
			Betting gets back to the highest better, but it's not the big blind
			OR
			There is one player left in the hand
		*/



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

    
    
    
    
    
    
			/// ===== Decide what to do with the bet =====
    
    
			if( PlayerBet(withP) >= PlayerMoney(withP) - GetChipDenom()/2.0 )
			{
						randRem *= (PlayerLastBet(withP)+1.0) / PlayerBet(withP) ;

				///ALL-IN. Notice that allIn combines handBet with myBet
				//? //However, myBetSize is now the WHOLE HAND.
				PlayerBet(withP) = PlayerMoney(withP) ;
				PlayerAllIn(withP)  = PlayerHandBetTotal(withP) + PlayerBet(withP);
					//we must remember allIn as above: it's what we can win/person

				allInsAppend(curIndex);
                                
                nonfoldActionOccurred();
			}
			else
			{//Not all-in
				if( PlayerBet(withP) < highBet )
				{///Player folds.
					randRem /= myBetSum + GetNumberInHandInclAllIn() + GetNumberInHandExclAllIn();

                    myFoldedPot += PlayerHandBetTotal( withP ) + PlayerLastBet(withP);
                    prevRoundFoldedPot += PlayerHandBetTotal( withP ); //Retroactive

					///You're taking money away here. addBets happenned a while ago
					PlayerHandBetTotal( withP )= PlayerLastBet(withP);
							//CAUTION: TRICKY USE OF handBetTotal
					PlayerMoney(withP) -= PlayerLastBet(withP);
                    forcedBetSum += PlayerLastBet(withP);

					PlayerBet(withP) = HoldemArena::FOLDED;
                    
                    // Decrement playersInHand, etc.
                    foldActionOccurred();
				}else
				{ //Not a fold.

                    if( PlayerBet(withP) > highBet && PlayerBet(withP) < highBet + myTable->GetMinRaise() )
                    {///You raised less than the MinRaise

                        message.minRaiseBy = myTable->GetMinRaise();
                        message.minRaiseTo = highBet + myTable->GetMinRaise();
                        message.error = PlayerBet(withP) - message.minRaiseTo;

                        const float64 distFromCall = PlayerBet(withP) - highBet;
                        const float64 distFromMinRaise = myTable->GetMinRaise() - distFromCall;

                        if( distFromCall > distFromMinRaise && PlayerMoney(withP) >= highBet + myTable->GetMinRaise() )
                        {
                            PlayerBet(withP) = highBet + myTable->GetMinRaise();
                        }else
                        {
                            PlayerBet(withP) = highBet;
                        }

                        message.result = PlayerBet(withP);

                    } //end if: raised less than MinRaise
                    
                    nonfoldActionOccurred();
				}//end if: fold, else not fold
			}//end if: all-in, else not all-in
///TODO Reraises need to say RERAISE.
    
    
            // =====     =====
    
    
    
			const HoldemAction currentMove(withP.GetIdent(), myPot, withP.GetMoney()
                                           ,
                                 curIndex, PlayerBet(withP), PlayerBet(withP) - PlayerLastBet(withP), highBet, 0
					, curIndex == bBlinds && comSize == 0 && curIndex == highestBetter,PlayerAllIn(withP) > 0);
            broadcastCurrentMove(currentMove);


			if( PlayerBet(withP) >= highBet )
			{
				bHighBetCalled = true;

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
                    randRem /= curIndex - myPot;


			}
			else if( PlayerAllIn(withP) > 0 )
			{   ///If you are going all in with LESS money than to call
                ///The money you have left over still NEEDs to be added to the pot
                ///Without this section, it wouldn't happen
			    addBets(PlayerBet(withP) - PlayerLastBet(withP));

                    randRem *= (myBetSum)/(curIndex+2.5);
			}




        do
		{
		    incrPlayerNumber(*(p[curIndex]));


            if(readyToFinish())
            {
                 finishBettingRound();
                 return currentMove;
            }
		}while (!( myTable->CanStillBet(curIndex) ));

        //bRoundState = 'b';
    return currentMove;

}

bool HoldemArenaBetting::readyToFinish() const
{
	const bool bBlindCheckOpportunity = (bBlinds == highestBetter) && bHighBetCalled; //The BB was never raised, but yet it has been called.

	const bool bBettorsAvailable = (playersInHand - playersAllIn + allInsNowCount > 1);
	const bool bReasonToBet = (curIndex != highestBetter || bBlindCheckOpportunity); //There are still players that haven't had their chance

	return ((!bReasonToBet) || (!bBettorsAvailable));
}

HoldemArenaBetting::~HoldemArenaBetting()
{
    if (allInsNow != 0) {
        delete [] allInsNow;
    }
}



void HoldemArenaShowdown::startShowdown()
{
    curIndex = called;

	//The player who was called may be all in.
	// If so, that player won't be IsInHand(curIndex).
	//We need to loop just to find the first IsInHand player.
	//Obviously if it goes all the way around, we build the allIns order, and start from there. (Decreasing chip stack)
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

void HoldemArenaShowdown::MuckHand(std::ostream &gamelog)
{
    ShowdownRep worstHand(curIndex); ///worstHand defaults to the worst hand. You only need to initialize playerIndex
    RevealHand(CommunityPlus::EMPTY_COMPLUS, CommunityPlus::EMPTY_COMPLUS, gamelog);
}

void HoldemArenaShowdown::RevealHandAllIns(const ShowdownRep& comp, const CommunityPlus & playerHand, std::ostream &gamelog)
{
    Player& withP = *p[curIndex];


    if( comp > best || comp == best ) //Better hand or tie
    {
        broadcastHand(playerHand,curIndex);
        if( bVerbose )
        {
            gamelog << endl << withP.GetIdent() << flush;
            if( comp == best )
                gamelog << " also has: " << flush;
            else
                gamelog << " is ahead with: " << flush;
            HandPlus viewHand;
            viewHand.SetUnique(playerHand);
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
    }
    ///His hand was worse, but since he is all-in he MUST show his hand (unless we don't know it)
    else
    {///Distinctly defeated
     //  http://www.texasholdem-poker.com/holdem_rules.php
        broadcastHand(playerHand,curIndex);
        if( bVerbose )
        {
            gamelog << endl << withP.GetIdent() << flush;
            gamelog << " turns over " << flush;
            HandPlus viewHand;
            viewHand.SetUnique(playerHand);
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

Reveal HoldemArenaShowdown::RevealHandMain(const ShowdownRep& comp, const CommunityPlus & playerHand, std::ostream &gamelog)
{

///At this point curIndex is STILL IN THE HAND (not all in)
    std::ostringstream revealAction;
    const playernumber_t revealPlayer = curIndex;

		    Player& withP = *p[curIndex];

			if( comp > best ) //best hand yet
			{

				///We set the allIn, since this player "IsInHand()"
				PlayerAllIn(withP) = myPot;

				broadcastHand(playerHand,curIndex);
                comp.DisplayHandText(revealAction);
				if( bVerbose )
				{


					gamelog << endl << withP.GetIdent() << flush;
					gamelog << " reveals: " << flush;
                    			HandPlus viewHand;
					viewHand.SetUnique(playerHand);
					viewHand.DisplayHand(gamelog);
					gamelog << endl << "Making," << flush;
					//#ifdef OLD_DISPLAY_STYLE
					comp.DisplayHandBig(gamelog);
					//#else
					//comp.DisplayHandText(gamelog);
					//#endif
				}

				winners.clear();
				winners.push_back(comp);
				best = comp;

			}
			else if( comp == best ) //can only split, if not beaten later
			{

				broadcastHand(playerHand,curIndex);
                comp.DisplayHandText(revealAction);
				if( bVerbose )
				{
					HandPlus viewHand;

					gamelog << endl << withP.GetIdent() << flush;
					gamelog << " turns up: ";
					viewHand.SetUnique(playerHand);
					#ifdef OLD_DISPLAY_STYLE
					viewHand.DisplayHandBig(gamelog);
					#else
					viewHand.DisplayHand(gamelog);
					#endif
					gamelog << endl << "Split... " << flush;
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
                revealAction << "mucks";
				gamelog << endl << withP.GetIdent() << " mucks " << endl;
			}


    struct Reveal result(revealPlayer, revealAction.str());

    do{
        incrIndex();

        if(curIndex == called)
        {
            startAllIns();
            return result;
        }
    }while ( !IsInHand(curIndex) );

    return result;
}

Reveal HoldemArenaShowdown::RevealHand(const CommunityPlus & playerHand, const CommunityPlus & community, std::ostream &gamelog)
{
    if( bRoundState == '!' ) return Reveal(-1, ""); // error

    ShowdownRep comp(curIndex);

    CommunityPlus withHandP;

    if( playerHand.IsEmpty() )
    {
        comp.SetMuck();
    }else
    {
		withHandP.SetUnique( playerHand );
        withHandP.AppendUnique(community);
        comp.Reset(&withHandP);
    }

    switch( bRoundState )
    {
        case 'w':
        {
            return RevealHandMain(comp, playerHand, gamelog);
        }
        case 'a':
        {
            std::ostringstream revealAction;
            comp.DisplayHandText(revealAction);
            struct Reveal result(curIndex, revealAction.str());

            RevealHandAllIns(comp, playerHand, gamelog);
            return result;
        }
        default:
        {
            return Reveal(-1, ""); // error
        }
    }
}



