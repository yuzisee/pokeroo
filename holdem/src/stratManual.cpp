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

#include "stratManual.h"
#include <iostream>
#include <string>

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
	cout << relvPlayer.GetIdent() << " " << flush;
	HoldemArena::ToString(e,cout);
#endif
}

void ConsoleStrategy::SeeCommunity(const Hand& h, const int8 cardsInCommunity)
{
	bNoPrint = (cardsInCommunity <= 0);
	comBuf.SetEmpty();
	comBuf.AppendUnique(h);
}

void UserConsoleStrategy::SeeCommunity(const Hand& h, const int8 n)
{
	ConsoleStrategy::SeeCommunity(h,n);
	if ( !bNoPrint ) printCommunity();
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
		cout << "The community now shows:" << endl;
		u.SetUnique(comBuf);
		u.DisplayHandBig(cout);
}

void ConsoleStrategy::printActions()
{

	cout << endl;

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
				cout << withP.GetIdent() << " " << flush;


				if( myTable.HasFolded(tempIndex) )
				{
					if( withP.GetLastBet() == HoldemArena::FOLDED )
					{
						cout << "has folded." << endl;
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
							cout << "bets " << otherBets << endl;
						}
						else
						{
							cout << "raises " << (wrtBets - lastBet) <<
									" to " << otherBets << endl;
						}

					}
					else if( wrtBets == lastBet )
					{
						if ( otherBets == 0 )
						{
							cout << "checks" << endl;
						}
						else
						{
							cout << "calls" << endl;
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


	++tempIndex;
	tempIndex %= totalPlayers;
	cout << endl << "You are betting " << ViewPlayer().GetBetSize() << " and have " << (ViewPlayer().GetMoney() - ViewPlayer().GetBetSize()) << " remaining." << endl;
	cout << "OPPONENT CHIPS LEFT" << endl;
	while( tempIndex != myIndex )
	{
		if( ViewTable().IsInHand(tempIndex) )
		{
			const Player& withP = *(myTable.ViewPlayer(tempIndex));
			cout << "\t" << withP.GetIdent();
			if( withP.GetBetSize() > 0 )
			{
				cout << " bet " << withP.GetBetSize() << " of";
			}
			else
			{
				cout << " has";
			}

			cout << " " << withP.GetMoney() << endl;
		}
		++tempIndex;
		tempIndex %= totalPlayers;
	}

	cout << "The pot contains " << ViewTable().GetPrevPotSize() << " from previous rounds and "
			<< ViewTable().GetRoundPotSize() << " from this round" << endl;

	cout << "You ("<< ViewPlayer().GetIdent() <<") have:" << flush;

	HandPlus u;
	u.SetUnique(ViewHand());
	u.DisplayHand(cout);

	cout << endl;

}

///TODO: HOLD ON!
///I added cin.sync() to stuff, if that doesn't help the input please revert it.
//Only ^Z will cause cin.eof()
float64 UserConsoleStrategy::queryAction()
{
	const int16 INPUTLEN = 10;
	int8 bExtraTry = 1;
	char inputBuf[INPUTLEN];
	char defaultAction = 0;
	inputBuf[0] = 0;
	float64 returnMe;

	showSituation();

        #ifdef DEBUGSAVEGAME
            if( !logFile.is_open() )
            {
                logFile.open(DEBUGSAVEGAME,std::ios::app);
            }
        #endif


	while( bExtraTry != 0 )
	{
        if( myFifo->eof() )
        {
            //cout << "EOF" << endl;
            myFifo = &(cin);
            cin.sync();
            cin.clear();
        }else
        {
            cout << endl << "[ENTER ACTION]" << endl;
            if( ViewTable().GetBetToCall() == ViewPlayer().GetBetSize() )
            {
                cout << "*check" << endl;
                defaultAction = 'c';
            }
            else
            {
                cout << "*fold" << endl;
                defaultAction = 'f';
                cout << "call (" << ViewTable().GetBetToCall() << flush;
                if( ViewPlayer().GetBetSize() > 0 )
                {
                    cout << " = +" << (ViewTable().GetBetToCall() - ViewPlayer().GetBetSize());
                }
                cout << ")" << endl;
            }
            cout << "raiseto" << endl << "raiseby" << endl << "-->____\b\b\b\b" << flush;
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
                                logFile.write("fold\n",5);
                            }
                        #endif
					returnMe = -1;
					bExtraTry = 0;
				}
				else
				{
					bExtraTry = 2;
					cout << "You can still check..." << endl;
				}

			}
			else if ( strncmp(inputBuf, "check", 5) == 0 )
			{
				if( ViewTable().GetBetToCall() == ViewPlayer().GetBetSize() )
				{
                        #ifdef DEBUGSAVEGAME
                            if( myFifo == &cin )
                            {
                                logFile.write("check\n",6);
                            }
                        #endif

					returnMe = ViewTable().GetBetToCall();
					bExtraTry = 0;
				}
				else
				{
					cout << "Can't check here" << endl;
				}
			}
			else if ( strncmp(inputBuf, "call", 4) == 0 )
			{
                    #ifdef DEBUGSAVEGAME
                        if( myFifo == &cin )
                        {
                            logFile.write("call\n",5);
                        }
                    #endif
				returnMe = ViewTable().GetBetToCall();
				bExtraTry = 0;
			}
			else if ( strncmp(inputBuf, "raiseby", 7) == 0 )
			{
				cout << "By how much?" << endl;
				while( bExtraTry != 0)
				{

					if( (*myFifo) >> returnMe )
					{
						if( returnMe > 0 )
						{
                                #ifdef DEBUGSAVEGAME
                                    if( myFifo == &cin )
                                    {
                                        logFile.write("raiseby\n",8);
                                        logFile << returnMe << endl;
                                    }
                                #endif
							returnMe += ViewTable().GetBetToCall();
							bExtraTry = 0;
						}

//                        cin.sync();
                        while( myFifo->peek() == '\n' || myFifo->peek() == '\r' )
                        {
                            myFifo->ignore( 1 , '\n');
                        }

					}
					else
					{
                        cin.sync();
                        //cin.ignore( cin.rdbuf()->in_avail() ,'\n');
                        cin.clear();
					}
				}
			}
			else if ( strncmp(inputBuf, "raiseto", 7) == 0 )
			{
                cout << "To how much?" << endl;
				while( bExtraTry != 0)
				{

                    if( (*myFifo) >> returnMe )
					{
						if( returnMe > ViewTable().GetBetToCall() )
						{
                                #ifdef DEBUGSAVEGAME

                                    if( myFifo == &cin )
                                    {
                                        logFile.write("raiseto\n",8);
                                        logFile << returnMe << endl;
                                    }
                                #endif
							bExtraTry = 0;
						}

						//cin.sync();
                        while( myFifo->peek() == '\n' || myFifo->peek() == '\r' )
                        {
                            myFifo->ignore( 1 , '\n');
                        }
					}
					else
					{
						cin.sync();
						//cin.ignore( cin.rdbuf()->in_avail() ,'\n');
						cin.clear();
					}

				}
			}
			else if ( 0 == inputBuf[0] )
			{///Just press [ENTER]: do default action
                if( ViewTable().GetBetToCall() == ViewPlayer().GetBetSize() )
				{
					returnMe = ViewTable().GetBetToCall();
				}else
				{
				    returnMe = -1;
				}
				bExtraTry = 0;
                //cout << "(default)" << endl;
                    #ifdef DEBUGSAVEGAME
                        if( myFifo == &cin )
                        {
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
                                    cout << "No default action found!" << endl;
                                    exit(1);
                                    #endif
                                    break;
                            }
                        }

                    #endif
			}
			else
			{
				cout << inputBuf << " is not a correct choice." << endl;
			}
		}
		else //error on input
		{
            #ifdef DEBUGSAVEGAME
                if( !(myFifo->eof()) )
                {
            #endif
            cout << "Error on input." << endl;
            cin.sync();
            cin.clear();
            #ifdef DEBUGSAVEGAME
                }
/*            else
                {
                cout << "eof start" << endl;
                }*/
            #endif
		}

	//End of loop
	}
#ifdef USERFEEDBACK
	cout << "To bet " << returnMe << endl;



	cout << "Command accepted" << endl;
#endif

    #ifdef DEBUGSAVEGAME
        if( logFile.is_open() )
        {
            logFile.close();
        }
    #endif


	return returnMe;
}

