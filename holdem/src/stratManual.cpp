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

//#define USERFEEDBACK
//#define SELF_SPECTATE
//#define USERINPUT
//#define FANCYUNDERLINE
#define SPACE_UI
#define USER_DELAY_HANDS

#define UI_DESCRIPTOR std::cerr

#include "stratManual.h"
#include <iostream>
#include <string>

#ifdef INFOASSIST
#include "functionmodel.h"
#endif


using std::cout;
using std::endl;
using std::cin;



#ifdef DEBUGSAVEGAME
std::ofstream UserConsoleStrategy::logFile;
#endif

void UserConsoleStrategy::SeeAction(const HoldemAction& e)
{
#ifdef SELF_SPECTATE
	const Player& relvPlayer = *(ViewTable().ViewPlayer(e.GetPlayerID()));
	UI_DESCRIPTOR << relvPlayer.GetIdent() << " " << flush;
	HoldemArena::ToString(e,UI_DESCRIPTOR);
#endif
}

void ConsoleStrategy::SeeCommunity(const Hand& h, const int8 cardsInCommunity)
{
	bNoPrint = (cardsInCommunity <= 0);
	comBuf.SetEmpty();
	comBuf.AppendUnique(h);

	#ifdef INFOASSIST
        CallCumulationD possibleHands;
        StatResult rankPCT;

	    CommunityPlus onlyCommunity;
        onlyCommunity.SetUnique(h);

        CommunityPlus withCommunity;
        withCommunity.SetUnique(ViewHand());
        withCommunity.AppendUnique(onlyCommunity);

        DistrShape w_wl(0);
	    StatsManager::Query(0,&detailPCT,&w_wl,withCommunity,onlyCommunity,cardsInCommunity);
        winMean = GainModel::ComposeBreakdown(detailPCT.mean,w_wl.mean);


        ViewTable().CachedQueryOffense(possibleHands,withCommunity);

        const float64 ranking3 = possibleHands.pctWillCall_tiefactor(1 - winMean.pct, 1); //wins+splits
        const float64 ranking = possibleHands.pctWillCall_tiefactor(1 - winMean.pct, 0); //wins

        rankPCT.wins = ranking;
        rankPCT.splits = ranking3 - ranking;
        rankPCT.loss = 1 - ranking3;
        rankPCT.genPCT();
        rarity = 1.0 - rankPCT.pct;


	#endif
}

void UserConsoleStrategy::SeeCommunity(const Hand& h, const int8 n)
{
	ConsoleStrategy::SeeCommunity(h,n);
	if ( !bNoPrint ){ printCommunity(); }

	#ifdef INFOASSIST
        bComSize = n;
	#endif

	#if defined(SPACE_UI) && !defined(USER_DELAY_HANDS)
	UI_DESCRIPTOR << endl;
	UI_DESCRIPTOR << endl;
	UI_DESCRIPTOR << endl;
	#endif
}

void UserConsoleStrategy::FinishHand()
{
#ifdef USER_DELAY_HANDS
    UI_DESCRIPTOR << endl << "Press [Enter] to begin hand" << endl;
    std::cin.sync();
    std::cin.clear();

    std::cin.get();

    std::cin.sync();
    std::cin.clear();
#ifdef SPACE_UI
    UI_DESCRIPTOR << endl;
#endif
#endif

}

float64 ConsoleStrategy::MakeBet()
{
	/*
	if( !bNoPrint ) {
		printCommunity();
		printActions();
	}
	*/
	return ViewTable().GetBetToCall();
}

float64 ConsoleStepStrategy::MakeBet()
{

	if(bNoPrint){
		system("pause");
	}
	return ConsoleStrategy::MakeBet();
}


float64 UserConsoleStrategy::MakeBet()
{
    #ifdef SELF_SPECTATE
	if( !bNoPrint ) {
		printActions();
	}
	#endif

	return queryAction();
}
void ConsoleStrategy::printCommunity()
{
	HandPlus u;
	UI_DESCRIPTOR << "The community now shows:" << endl;
	u.SetUnique(comBuf);
	u.DisplayHandBig(UI_DESCRIPTOR);
}

void ConsoleStrategy::printActions()
{

	UI_DESCRIPTOR << endl;

	const HoldemArena & myTable = ViewTable();
	const int8 myIndex = myTable.GetCurPlayer();
	int8 tempIndex = myIndex;
	const float64 refBetSize = ViewPlayer().GetBetSize();
	float64 otherBets;
	float64 wrtBets;
	float64 lastBet = 0;

	myTable.incrIndex(tempIndex);

	while( tempIndex != myIndex )
	{

		const Player& withP = *(myTable.ViewPlayer(tempIndex));

		if( myTable.IsInHand(tempIndex) )
		{
			if (withP.GetLastBet() != HoldemArena::INVALID)
			{
				UI_DESCRIPTOR << withP.GetIdent() << " " << flush;


				if( myTable.HasFolded(tempIndex) )
				{
					if( withP.GetLastBet() == HoldemArena::FOLDED )
					{
						UI_DESCRIPTOR << "has folded." << endl;
					}
				}
				else
				{
					otherBets = withP.GetBetSize();
					wrtBets = otherBets - refBetSize;

					if ( wrtBets > lastBet )
					{
						if( lastBet == 0 )
						{
							UI_DESCRIPTOR << "bets " << otherBets << endl;
						}
						else
						{
							UI_DESCRIPTOR << "raises " << (wrtBets - lastBet) <<
									" to " << otherBets << endl;
						}

					}
					else if( wrtBets == lastBet )
					{
						if ( otherBets == 0 )
						{
							UI_DESCRIPTOR << "checks" << endl;
						}
						else
						{
							UI_DESCRIPTOR << "calls" << endl;
						}
					}

					lastBet = wrtBets;
				}
			}
		}

	myTable.incrIndex(tempIndex);

	}
}

UserConsoleStrategy::~UserConsoleStrategy()
{
    #ifdef DEBUGSAVEGAME
        if( logFile.is_open() )
        {
            logFile.close();
        }
    #endif

}

void ConsoleStrategy::showSituation()
{

	const HoldemArena & myTable = ViewTable();
	const int8 myIndex = myTable.GetCurPlayer();
	int8 tempIndex = myIndex;
	const int8 totalPlayers = myTable.GetTotalPlayers();


    #ifdef OLD_DISPLAY_STYLE
	UI_DESCRIPTOR << endl << "The pot contains " << ViewTable().GetPrevPotSize() << " from previous rounds and "
			<< ViewTable().GetRoundPotSize() << " from this round" << endl;
    #else
    UI_DESCRIPTOR << endl << "The pot contains $" << ViewTable().GetPrevPotSize() + ViewTable().GetRoundPotSize() << endl;
    #endif

    #ifdef INFOASSIST
        const float64 xBet = ViewTable().GetBetToCall() - ViewPlayer().GetBetSize();
        if( xBet > 0 )
        {
            const float64 winAmount = ViewTable().GetPrevPotSize() + ViewTable().GetRoundPotSize() +  xBet;
            UI_DESCRIPTOR << "\tYou can bet $" << xBet << " more to win $" << winAmount - xBet << " plus your $" << xBet << endl;
            UI_DESCRIPTOR << "\tThis works out to be " << winAmount/xBet << " : 1 odds (" << 100*xBet/winAmount << "%)" << endl;
        }
    #endif


	++tempIndex;
	tempIndex %= totalPlayers;
	#ifdef OLD_DISPLAY_STYLE
	UI_DESCRIPTOR << endl << "You are betting " << ViewPlayer().GetBetSize() << " and have " << (ViewPlayer().GetMoney() - ViewPlayer().GetBetSize()) << " remaining." << endl;
	UI_DESCRIPTOR << "OPPONENTS IN HAND" << endl;

	while( tempIndex != myIndex )
	{
		if( ViewTable().IsInHand(tempIndex) )
		{
			const Player& withP = *(myTable.ViewPlayer(tempIndex));
			UI_DESCRIPTOR << "\t" << withP.GetIdent();
			if( withP.GetBetSize() > 0 )
			{
				UI_DESCRIPTOR << " bet " << withP.GetBetSize() << " of";
			}
			else
			{
				UI_DESCRIPTOR << " has";
			}

			UI_DESCRIPTOR << " " << withP.GetMoney() << endl;
		}
		++tempIndex;
		tempIndex %= totalPlayers;
	}
	#else
	UI_DESCRIPTOR << endl ;
	UI_DESCRIPTOR << "PLAYER SUMMARY" << endl;


	int8 dealPos;
	int8 sbPos = -1;
	int8 bbPos = -1;


    ///Find Dealer, SB, BB
    tempIndex = ViewTable().GetDealer();
    dealPos = tempIndex;
    do
    {
        ViewTable().incrIndex(tempIndex);
        if( ViewTable().IsAlive(tempIndex) )
        {
            if( sbPos == -1 ) sbPos = tempIndex;
            else if( bbPos == -1 ) bbPos = tempIndex;
        }
    }while( bbPos == -1 );

    tempIndex = myIndex;
    do
    {
        ViewTable().incrIndex(tempIndex);

        if( ViewTable().IsAlive(tempIndex) )
	{

        const Player& withP = *(myTable.ViewPlayer(tempIndex));
		if( tempIndex == sbPos )
		{
		    UI_DESCRIPTOR << "SB" << flush;
		}else if( tempIndex == bbPos )
		{
			UI_DESCRIPTOR << "BB" << flush;
		}else if( tempIndex == dealPos )
		{
		    UI_DESCRIPTOR << "D" << flush;
		}


		if( ViewTable().IsInHand(tempIndex) )
		{
			UI_DESCRIPTOR << "\t" << withP.GetIdent() << flush;
			if( ViewTable().CanStillBet(tempIndex) )
			{
				UI_DESCRIPTOR << " has $" << withP.GetMoney() - withP.GetBetSize() << " left";
			}else
			{
				UI_DESCRIPTOR << " is all-in" ;
			}
		}else
		{
			UI_DESCRIPTOR << "\t(" << withP.GetIdent() << flush;
			UI_DESCRIPTOR << " folded)" << flush;
		}

		UI_DESCRIPTOR << endl;

	}
    }while( tempIndex !=  myIndex);//ViewTable().GetDealer() );

	#endif
    UI_DESCRIPTOR << endl ;

	UI_DESCRIPTOR << endl << "You ("<< ViewPlayer().GetIdent() <<") have: " << flush;


	HandPlus u;
	u.SetUnique(ViewHand());
	u.DisplayHand(UI_DESCRIPTOR);

#ifdef INFOASSIST
    UI_DESCRIPTOR.precision(4);

	if( rarity > 0 )
	{
	    const float64 freqBetter = 1/rarity;
        UI_DESCRIPTOR << "  (one in " << freqBetter << " hands is better)";
	}

	UI_DESCRIPTOR << endl;


        float64 wsplitChance = 0;
        const int8 opphandsDealt = ViewTable().GetNumberAtTable() - 1;
        for( int8 i=0;i<=opphandsDealt;++i)
        {
            wsplitChance += pow(winMean.wins,i) * pow(winMean.splits,opphandsDealt-i);
        }
        UI_DESCRIPTOR << endl << "\t(" << pow(winMean.wins,opphandsDealt) * 100 << "% chance to win outright)" << endl;
        UI_DESCRIPTOR << "\t(" << wsplitChance * 100 << "% chance to split or win)" << flush;
        //UI_DESCRIPTOR << winMean.wins << endl;
        //UI_DESCRIPTOR << winMean.splits << endl;
        //UI_DESCRIPTOR << winMean.loss << endl;
        if( bComSize < 5 )
        {
            UI_DESCRIPTOR << endl << endl << (detailPCT.improve + 1) * 50 << "% of the time, you will be more likely to win after the " << flush;
            switch( bComSize )
            {
                case 0:
                    UI_DESCRIPTOR << "flop" << flush;
                    break;
                case 3:
                    UI_DESCRIPTOR << "turn" << flush;
                    break;
                case 4:
                    UI_DESCRIPTOR << "river" << flush;
                    break;
                default:
                    UI_DESCRIPTOR << "next community cards" << flush;
                    break;
            }
            UI_DESCRIPTOR << "." << endl << 50 * (1 - detailPCT.improve) << "% of the time, you are more likely to win now." << flush;
        }

    UI_DESCRIPTOR.precision(6);
    #endif


}


//Only ^Z will cause cin.eof()
///Aha!
float64 UserConsoleStrategy::queryAction()
{
	const int16 INPUTLEN = 10;
	int8 bExtraTry = 1;
	char inputBuf[INPUTLEN];
	char defaultAction = 0;
	inputBuf[0] = 0;
	float64 returnMe;

	showSituation();



	while( bExtraTry != 0 )
	{
	    if( myFifo != &cin )
        {
            #ifdef USERINPUT
            UI_DESCRIPTOR << "Mark fileinput" << endl;
            #endif
            while( myFifo->peek() == '\n' || myFifo->peek() == '\r' )
            {
                #ifdef USERINPUT
                UI_DESCRIPTOR << "ExtraE" << endl;
                #endif
                myFifo->ignore( 1 , '\n');
            }
            if( myFifo->eof() )
            {
                #ifdef USERINPUT
                UI_DESCRIPTOR << "StreamClear" << endl;
                #endif
                myFifo = &(cin);
                cin.sync();
                cin.clear();
            }
        }


        if( myFifo->eof() )
        {
            #ifdef USERINPUT
            UI_DESCRIPTOR << "EOF" << endl;
            #endif
            myFifo = &(cin);
            cin.sync();
            cin.clear();
        }else
        {
            UI_DESCRIPTOR << endl << endl;
            UI_DESCRIPTOR << "==ENTER ACTION==";
            UI_DESCRIPTOR << "      (press only [Enter] for check/fold)" << endl;
            if( ViewTable().GetBetToCall() == ViewPlayer().GetBetSize() )
            {
                UI_DESCRIPTOR << "check" << endl;
                defaultAction = 'c';
            }
            else
            {
                UI_DESCRIPTOR << "fold" << endl;
                defaultAction = 'f';
                UI_DESCRIPTOR << "call (" << ViewTable().GetBetToCall() << flush;
                if( ViewPlayer().GetBetSize() > 0 )
                {
                    UI_DESCRIPTOR << " = +" << (ViewTable().GetBetToCall() - ViewPlayer().GetBetSize());
                }
                UI_DESCRIPTOR << ")" << endl;
            }
            UI_DESCRIPTOR << "raiseto" << endl << "raiseby" << endl << "-->" << flush;
            #ifdef FANCYUNDERLINE
            std::cerr << "____\b\b\b\b" << flush;
            #endif
        }

		if( myFifo->getline( inputBuf, INPUTLEN ) != 0 )
		{

			if( strncmp(inputBuf,"fold",4) == 0 )
			{
				if( bExtraTry == 2 || ViewTable().GetBetToCall() > ViewPlayer().GetBetSize())
				{
				        #ifdef DEBUGSAVEGAME
                            if( myFifo == &cin )
                            {
                                if( !logFile.is_open() )
                                {
                                    logFile.open(DEBUGSAVEGAME,std::ios::app);
                                }
                                logFile.write("fold\n",5);
                            }
                        #endif
					returnMe = -1;
					bExtraTry = 0;
				}
				else
				{
					bExtraTry = 2;
					UI_DESCRIPTOR << "You can still check..." << endl;
				}

			}
			else if ( strncmp(inputBuf, "check", 5) == 0 )
			{
				if( ViewTable().GetBetToCall() == ViewPlayer().GetBetSize() )
				{
                        #ifdef DEBUGSAVEGAME
                            if( myFifo == &cin )
                            {
                                if( !logFile.is_open() )
                                {
                                    logFile.open(DEBUGSAVEGAME,std::ios::app);
                                }
                                logFile.write("check\n",6);
                            }
                        #endif

					returnMe = ViewTable().GetBetToCall();
					bExtraTry = 0;
				}
				else
				{
					UI_DESCRIPTOR << "Can't check here" << endl;
				}
			}
			else if ( strncmp(inputBuf, "call", 4) == 0 )
			{
                    #ifdef DEBUGSAVEGAME
                        if( myFifo == &cin )
                        {
                            if( !logFile.is_open() )
                            {
                                logFile.open(DEBUGSAVEGAME,std::ios::app);
                            }
                            logFile.write("call\n",5);
                        }
                    #endif
				returnMe = ViewTable().GetBetToCall();
				bExtraTry = 0;
			}
			else if ( strncmp(inputBuf, "raiseby", 7) == 0 )
			{
			    const float64 minRaiseBy = ViewTable().GetMinRaise();
				UI_DESCRIPTOR << "By how much?  (Minimum by " << minRaiseBy << ")" << endl;
				while( bExtraTry != 0)
				{

					if( (*myFifo) >> returnMe )
					{
						if( returnMe > 0 )
						{
						    if( returnMe < minRaiseBy )
						    {
						        UI_DESCRIPTOR << "Minimum raise was by " << minRaiseBy << ". Please try again." << endl;
						    }else
						    {

                                #ifdef DEBUGSAVEGAME
                                    if( myFifo == &cin )
                                    {
                                        if( !logFile.is_open() )
                                        {
                                            logFile.open(DEBUGSAVEGAME,std::ios::app);
                                        }
                                        logFile.write("raiseby\n",8);
                                        logFile << returnMe << endl;
                                    }
                                #endif
                                bExtraTry = 0;
						    }

							returnMe += ViewTable().GetBetToCall();
						}

                        if( myFifo->rdbuf()->in_avail() > 0 )
                        {
                            while( myFifo->peek() == '\n' || myFifo->peek() == '\r' )
                            {
                                #ifdef USERINPUT
                                int numericPeek = myFifo->peek();
                                UI_DESCRIPTOR << "avail:" << myFifo->rdbuf()->in_avail() << endl;
                                UI_DESCRIPTOR << "SkipE" << numericPeek << endl;
                                #endif
                                myFifo->ignore(1);
                                //myFifo->rdbuf()->sbumpc();
                            }
                            #ifdef USERINPUT
                            UI_DESCRIPTOR << ".seComplete" << endl;
                            #endif
                        }
                        cin.sync();
					}
					else
					{
					    #ifdef USERINPUT
					    UI_DESCRIPTOR << "ApE" << endl;
					    #endif
                        cin.sync();
                        //cin.ignore( cin.rdbuf()->in_avail() ,'\n');
                        cin.clear();
					}
				}
			}
			else if ( strncmp(inputBuf, "raiseto", 7) == 0 )
			{
			    const float64 minRaiseTo  = ViewTable().GetMinRaise() + ViewTable().GetBetToCall();
                UI_DESCRIPTOR << "To how much?  (Minimum is " << minRaiseTo << ")" << endl;
				while( bExtraTry != 0)
				{

                    if( (*myFifo) >> returnMe )
					{
						if( returnMe > 0 )
						{
						    if( returnMe < minRaiseTo )
						    {
						        UI_DESCRIPTOR << "Minimum raise was to " << minRaiseTo << ". Please try again." << endl;
						        //returnMe = ViewTable().GetBetToCall();
						    }
                            else
                            {
                                #ifdef DEBUGSAVEGAME

                                    if( myFifo == &cin )
                                    {
                                        if( !logFile.is_open() )
                                        {
                                            logFile.open(DEBUGSAVEGAME,std::ios::app);
                                        }
                                        logFile.write("raiseto\n",8);
                                        logFile << returnMe << endl;
                                    }
                                #endif

                                bExtraTry = 0;
                            }

						}


                        if( myFifo->rdbuf()->in_avail() > 0 )
                        {
                            while( myFifo->peek() == '\n' || myFifo->peek() == '\r' )
                            {
                                #ifdef USERINPUT
                                int numericPeek = myFifo->peek();
                                UI_DESCRIPTOR << "avail:" << myFifo->rdbuf()->in_avail() << endl;
                                UI_DESCRIPTOR << "SkipE" << numericPeek << endl;
                                #endif
                                myFifo->ignore(1);
                                //myFifo->rdbuf()->sbumpc();
                            }
                            #ifdef USERINPUT
                            UI_DESCRIPTOR << ".seComplete" << endl;
                            #endif
                        }
                        cin.sync();

					}
					else
					{
					    #ifdef USERINPUT
					    UI_DESCRIPTOR << "ApE" << endl;
					    #endif
						cin.sync();
						//cin.ignore( cin.rdbuf()->in_avail() ,'\n');
						cin.clear();
					}

				}
			}
			else if ( 0 == inputBuf[0] )
			{///Just press [ENTER]: do default action
			    #ifdef DEBUGASSERT
			    //This can't occur if reading from a file though
			    if( myFifo != &cin )
			    {
			        UI_DESCRIPTOR << "Blank lines in file got caught" << endl;
			        exit(1);
			    }
			    #endif


                if( ViewTable().GetBetToCall() == ViewPlayer().GetBetSize() )
				{
					returnMe = ViewTable().GetBetToCall();
				}else
				{
				    returnMe = -1;
				}
				bExtraTry = 0;
                //UI_DESCRIPTOR << "(default)" << endl;
                    #ifdef DEBUGSAVEGAME
                        if( myFifo == &cin )
                        {
                            if( !logFile.is_open() )
                            {
                                logFile.open(DEBUGSAVEGAME,std::ios::app);
                            }
                            switch( defaultAction )
                            {
                                case 'c':
                                    logFile << "check" << endl;
                                    break;
                                case 'f':
                                    logFile << "fold" << endl;
                                    break;
                                default:
                                    logFile << endl;
                                    #ifdef DEBUGASSERT
                                    UI_DESCRIPTOR << "No default action found!" << endl;
                                    exit(1);
                                    #endif
                                    break;
                            }
                        }

                    #endif
			}
			else
			{
				UI_DESCRIPTOR << inputBuf << " is not a correct choice." << endl;
			}
		}
		else //error on input
		{
            #ifdef DEBUGSAVEGAME
                if( !(myFifo->eof()) )
                {
            #endif
            UI_DESCRIPTOR << "Error on input." << endl;
            while(myFifo->gcount() == INPUTLEN)
            {
                #ifdef USERINPUT
                UI_DESCRIPTOR << "Clearing" << endl;
                #endif
                 myFifo->getline( inputBuf, INPUTLEN );
            }
            cin.sync();
            cin.clear();

            #ifdef DEBUGSAVEGAME
                }
                #ifdef USERINPUT
                    else
                        {
                        UI_DESCRIPTOR << "eof start" << endl;
                        }
                #endif
            #endif
		}

	//End of loop
	}
#ifdef USERFEEDBACK
	UI_DESCRIPTOR << "To bet " << returnMe << endl;



	UI_DESCRIPTOR << "Command accepted" << endl;
#endif

    #ifdef DEBUGSAVEGAME
        if( logFile.is_open() )
        {
            logFile.close();
        }
    #endif


	return returnMe;
}

