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

//#define USERFEEDBACK
//#define SELF_SPECTATE
//#define USERINPUT
//#define FANCYUNDERLINE
#define SPACE_UI
#define USER_DELAY_HANDS

#define UI_DESCRIPTOR std::cerr

#include "stratManual.h"
#include <iostream>
#include <string.h>

#ifdef INFOASSIST_STRONG
#include "../src/functionmodel.h"
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

	#ifdef INFOASSIST_STRONG
        CallCumulationD possibleHands;
        StatResult rankPCT;

	    CommunityPlus onlyCommunity;
        onlyCommunity.SetUnique(h);

        CommunityPlus withCommunity;
        withCommunity.SetUnique(ViewDealtHand());
        withCommunity.AppendUnique(onlyCommunity);

	    StatsManager::Query(&detailPCT,withCommunity,onlyCommunity,cardsInCommunity);
        winMean = detailPCT.mean;


        ViewTable().CachedQueryOffense(possibleHands,onlyCommunity,withCommunity);

        rarity = possibleHands.Pr_haveWinPCT_strictlyBetterThan(winMean.pct - EPS_WIN_PCT);

//You can tie in rank if and only if you tie in mean
        rankPCT.wins = 1 - rarity;
        rankPCT.splits = winMean.splits;
        rankPCT.loss = rarity;
        const float64 scaleTotal = rankPCT.wins + rankPCT.splits + rankPCT.loss;
        rankPCT.wins /= scaleTotal;
        rankPCT.splits /= scaleTotal;
        rankPCT.loss /= scaleTotal;
        rankPCT.genPCT();


	#endif
}

void UserConsoleStrategy::SeeCommunity(const Hand& h, const int8 n)
{

	ConsoleStrategy::SeeCommunity(h,n);
	if ( !bNoPrint ){ printCommunity(); }

	#ifdef INFOASSIST_STRONG
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
    std::cin.clear();
    std::cin.sync();

    int16 getChar = std::cin.get();
    if( getChar == 'X' )
    {
        exit(0);
    }

    std::cin.clear();
    std::cin.sync();
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
#ifdef INFOASSIST_UPDATE
	HandPlus u;
	UI_DESCRIPTOR << "The community now shows:" << endl;
	u.SetUnique(comBuf);
	u.DisplayHandBig(UI_DESCRIPTOR);
#endif
}

void ConsoleStrategy::printActions()
{

#ifdef INFOASSIST_UPDATE
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

#endif // INFOASSIST_UPDATE
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
#ifdef INFOASSIST_UPDATE
	const HoldemArena & myTable = ViewTable();
	const int8 myIndex = myTable.GetCurPlayer();
	int8 tempIndex = myIndex;
	const int8 totalPlayers = myTable.GetTotalPlayers();
#endif // INFOASSIST_UPDATE

	  // UI_DESCRIPTOR << endl << "The pot contains " << ViewTable().GetPrevPotSize() << " from previous rounds and " << ViewTable().GetRoundPotSize() << " from this round" << endl;
    UI_DESCRIPTOR << endl << "The pot contains $" << ViewTable().GetPrevPotSize() + ViewTable().GetRoundPotSize() << endl;


    #ifdef INFOASSIST_STRONG
        const float64 xBet = ViewTable().GetBetToCall() - ViewPlayer().GetBetSize();
        if( xBet > 0 )
        {
            const float64 winAmount = ViewTable().GetPrevPotSize() + ViewTable().GetRoundPotSize() +  xBet;
            UI_DESCRIPTOR << "\tYou can bet $" << xBet << " more to win $" << winAmount - xBet << " plus your $" << xBet << endl;
            UI_DESCRIPTOR << "\tThis works out to be " << winAmount/xBet << " : 1 odds (" << 100*xBet/winAmount << "%)" << endl;
        }
    #endif // INFOASSIST_STRONG

#ifdef INFOASSIST_UPDATE

	++tempIndex;
	tempIndex %= totalPlayers;

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


#endif // INFOASSIST_UPDATE
    UI_DESCRIPTOR << endl ;
    UI_DESCRIPTOR << "Position Strength of first bet = " << (int)(ViewTable().NumberAtFirstActionOfRound().total) << endl ;
    UI_DESCRIPTOR << endl ;

	UI_DESCRIPTOR << endl << "You ("<< ViewPlayer().GetIdent() <<") have: " << flush;


	HandPlus u;
	u.SetUnique(ViewDealtHand());
	u.DisplayHand(UI_DESCRIPTOR);

#ifdef INFOASSIST_STRONG
    UI_DESCRIPTOR.precision(4);

	if( rarity > 0 )
	{
	    const float64 freqBetter = 1/rarity;
        UI_DESCRIPTOR << "  (one in " << freqBetter << " hands is better)";
	}

	UI_DESCRIPTOR << endl;

/*
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
*/
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


void DualInputStream::SetFileStream(std::istream * fileStream)
{
    myFifo[0] = fileStream;
    currentStream = 0;
}

int DualInputStream::stream_peek_update()
{
    while( (myFifo[currentStream]->peek() == EOF) && (currentStream <= 0) ) //Either CTRL+Z or end of text file
    {
        ++currentStream;
    }

    return myFifo[currentStream]->peek();
}
/*
void DualInputStream::stream_ignore1()
{
    if( stream_peek_update() != EOF ) ;
}
*/

bool DualInputStream::charIsWhitespace(char a)
{
    return (    a == '\r'
             || a == '\n'
	     || a == ' '
	     || a == '\t'
	   )
    ;
}

void DualInputStream::skipWhitespaceSection()
{
    while  ( charIsWhitespace(stream_peek_update()) )  {
        myFifo[currentStream]->ignore(1);//stream_ignore1();
    }
}

void DualInputStream::AbsorbNewline()
{
    int next_char;

    if( IsFileInput() ) return; //No need, skipWhitespaceSection() is always called for file inputs...

    for(;;)
    {
	next_char = myFifo[currentStream]->get();
	if( next_char == EOF || next_char == '\n' || next_char == '\0' ) break;
    }
}

float64 DualInputStream::GetPositiveFloat64()
{
    float64 usersFloat;

    skipWhitespaceSection();

    if( (*(myFifo[currentStream])) >> usersFloat )	return usersFloat;
    else						return 0;
}


void DualInputStream::GetCommandString(char * inputBuf, const int MAXINPUTLEN)
{
    int bufidx = 0;
    int next_char;

    if( IsFileInput() )
    {
        skipWhitespaceSection(); //which includes peek_update...
    }

    if( !IsFileInput() )
    {
    #ifdef USERINPUT
    UI_DESCRIPTOR << endl << "Force clear" << endl;
    #endif
	cin.clear();
        cin.sync();
    }

    //You don't want to switch streams mid-command.
    //Furthermore, currentStream should be an accurate indicator of where the LAST command came from.
    //(So that we don't double-log savegame text)
    while( !charIsWhitespace(next_char = myFifo[currentStream]->peek()) )
    {
        if( next_char == 0 || next_char == EOF ) break;

        myFifo[currentStream]->ignore(1);
	inputBuf[bufidx] = next_char;
        ++bufidx;

	if( bufidx >= MAXINPUTLEN - 1 ) break;
    }

    inputBuf[bufidx] = '\0';
}



enum UserConsoleStrategyAction { ACTION_FOLD,
				 ACTION_CHECK,
				 ACTION_CALL,
				 ACTION_RAISEBY,
				 ACTION_RAISETO,
				 ACTION_DEFAULT,
				 ACTION_UNKNOWN
				};


enum UserConsoleStrategyAction queryAction_determineAction(DualInputStream & userInstream)
{
    const int16 MAXINPUTLEN = 10;
    char inputBuf[MAXINPUTLEN];
    inputBuf[0] = 0;
    userInstream.GetCommandString(inputBuf,MAXINPUTLEN);

    if( strncmp(inputBuf, "fold",4) == 0 ) return ACTION_FOLD;
    else if( strncmp(inputBuf, "check", 5) == 0 ) return ACTION_CHECK;
    else if( strncmp(inputBuf, "call", 4) == 0 ) return ACTION_CALL;
    else if( strncmp(inputBuf, "raiseby", 7) == 0 ) return ACTION_RAISEBY;
    else if( strncmp(inputBuf, "raiseto", 7) == 0 ) return ACTION_RAISETO;
    else if( strncmp(inputBuf, "EXIT", 4) == 0 ) exit(0);
    else if( 0 == inputBuf[0] ) return ACTION_DEFAULT;
    else
    {
	UI_DESCRIPTOR << inputBuf << " is not a correct choice." << endl;
	return ACTION_UNKNOWN;
    }
}


//Only ^Z will cause cin.eof()
float64 UserConsoleStrategy::queryAction()
{
    int8 bExtraTry = 1;
    char defaultAction = 0;
    float64 returnMe = ViewPlayer().GetBetSize();

    cin.clear();
    cin.sync();

    showSituation();


    while( bExtraTry != 0 ) //Main input loop
    {

	//==============================
	// Display user input choices
	//==============================

	UI_DESCRIPTOR << endl << endl;
	UI_DESCRIPTOR << "== ENTER ACTION: " << ViewPlayer().GetIdent().c_str() << " ==";
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


        const float64 minRaiseBy = ViewTable().GetMinRaise();
        const float64 minRaiseTo  = ViewTable().GetMinRaise() + ViewTable().GetBetToCall();


	switch( queryAction_determineAction( myFifos ) )
	{
	    case ACTION_FOLD:
		myFifos.AbsorbNewline();
                if( bExtraTry == 2 || ViewTable().GetBetToCall() > ViewPlayer().GetBetSize())
                {
                        #ifdef DEBUGSAVEGAME
                            if( !myFifos.IsFileInput() )
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
		break;

	    case ACTION_CHECK:
		myFifos.AbsorbNewline();
                if( ViewTable().GetBetToCall() == ViewPlayer().GetBetSize() )
                {
                        #ifdef DEBUGSAVEGAME
                            if( !myFifos.IsFileInput() )
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
            	break;

	    case ACTION_CALL:
		myFifos.AbsorbNewline();
                    #ifdef DEBUGSAVEGAME
                        if( !myFifos.IsFileInput() )
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
                break;

	    case ACTION_RAISEBY:
                UI_DESCRIPTOR << "By how much?  (Minimum by " << minRaiseBy << ")" << endl;
                while( bExtraTry != 0)
                {
		    returnMe = myFifos.GetPositiveFloat64();
		    myFifos.AbsorbNewline();

		    if( returnMe > 0 )
		    {
			if( returnMe < minRaiseBy )
			{
			    UI_DESCRIPTOR << "Minimum raise was by " << minRaiseBy << ". Please try again." << endl;
			}else
			{

			    #ifdef DEBUGSAVEGAME
                                if( !myFifos.IsFileInput() )
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

		    }//end if: valid positive float recieved

		}//end loop: acceptable raise
            	break;

	    case ACTION_RAISETO:
                UI_DESCRIPTOR << "To how much?  (Minimum is " << minRaiseTo << ")" << endl;
                while( bExtraTry != 0)
                {
		    returnMe = myFifos.GetPositiveFloat64();
		    myFifos.AbsorbNewline();

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

                                if( !myFifos.IsFileInput() )
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

		    }//end if: valid positive float recieved

		}//end loop: acceptable raise
                break;

            case ACTION_DEFAULT:
		myFifos.AbsorbNewline();
            ///Just press [ENTER]: do default action
                #ifdef DEBUGASSERT
                //This can't occur if reading from a file though
                if( myFifos.IsFileInput() )
                {
                    UI_DESCRIPTOR << "Blank lines in file got caught" << endl;
                    exit(74); // man sysexits → EX_IOERR
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
                	if( !myFifos.IsFileInput() )
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
                                    exit(65); // man sysexits → EX_DATAERR
                                    #endif
                                    break;
                            }
                        }

                    #endif
            	break;



	    case ACTION_UNKNOWN:
	    default:
		myFifos.AbsorbNewline();
		UI_DESCRIPTOR << "Error on input?" << endl;

		    #ifdef USERINPUT
		    UI_DESCRIPTOR << "Clearing" << endl;
		    #endif

		cin.clear();
		cin.sync();

		break;


        }

    //End of main input loop
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
