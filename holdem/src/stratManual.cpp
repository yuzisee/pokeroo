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

#include "stratManual.h"
#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::cin;

void UserConsoleStrategy::SeeAction(const HoldemAction& e)
{
	const Player& relvPlayer = *(ViewTable().ViewPlayer(e.GetPlayerID()));
	cout << relvPlayer.GetIdent() << " " << flush;
	if ( e.IsFold() )
	{
		cout << "folds." << endl;
	}
	else if ( e.IsCheck() )
	{
		cout << "checks." << endl;
	}
	else if ( e.IsCall() )
	{
		cout << "calls." << endl;
	}
	else
	{
		if ( e.IsRaise() )
		{
			cout << "raises by " << e.GetRaiseBy()
				<< " to " << flush;
		}
		else
		{
			cout << "bets " << flush;
		}
		cout << e.GetAmount() << "." << endl;
	}
}

void ConsoleStrategy::SeeCommunity(const Hand& h, const int8 cardsInCommunity)
{
	bNoPrint = (cardsInCommunity <= 0);
	comBuf.Empty();
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
	if( !bNoPrint ) {
		printActions();
	}

	return queryAction();
}
void ConsoleStrategy::printCommunity()
{
	HandPlus u;
		cout << "The community now shows:" << endl;
		u.SetUnique(comBuf);
		u.ShowHand(true);
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

void ConsoleStrategy::showSituation()
{

	const HoldemArena & myTable = ViewTable();
	const int8 myIndex = myTable.GetCurPlayer();
	int8 tempIndex = myIndex;
	const int8 totalPlayers = myTable.GetTotalPlayers();


	++tempIndex;
	tempIndex %= totalPlayers;
	cout << "You are betting " << ViewPlayer().GetBetSize() << " and have " << (ViewPlayer().GetMoney() - ViewPlayer().GetBetSize()) << " remaining." << endl;
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

	cout << "The pot contains " << ViewTable().GetDeadPotSize() << " from previous rounds and "
			<< ViewTable().GetRoundPotSize() << " from this round" << endl;

	cout << "You ("<< ViewPlayer().GetIdent() <<") have:" << flush;

	HandPlus u;
	u.SetUnique(ViewHand());
	u.ShowHand(false);

	cout << endl;

}

///TODO: HOLD ON!
///I added cin.sync() to stuff, if that doesn't help the input please revert it.
float64 UserConsoleStrategy::queryAction()
{
	const int16 INPUTLEN = 10;
	int8 bExtraTry = 1;
	char inputBuf[INPUTLEN];
	inputBuf[0] = 0;
	float64 returnMe;

	showSituation();


	while( bExtraTry != 0 )
	{
		cout << endl << "[ENTER ACTION]" << endl;
		if( ViewTable().GetBetToCall() == ViewPlayer().GetBetSize() )
		{
			cout << "*check" << endl;
		}
		else
		{
			cout << "*fold" << endl;
			cout << "call (" << ViewTable().GetBetToCall() << flush;
			if( ViewPlayer().GetBetSize() > 0 )
			{
				cout << " = +" << (ViewTable().GetBetToCall() - ViewPlayer().GetBetSize());
			}
			cout << ")" << endl;
		}
		cout << "raiseto" << endl << "raiseby" << endl << "-->____\b\b\b\b" << flush;


		if( cin.getline( inputBuf, INPUTLEN ) != 0 )
		{

			if( strncmp(inputBuf,"fold",4) == 0 )
			{
				if( bExtraTry == 2 || ViewTable().GetBetToCall() > ViewPlayer().GetBetSize())
				{
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
				returnMe = ViewTable().GetBetToCall();
				bExtraTry = 0;
			}
			else if ( strncmp(inputBuf, "raiseby", 7) == 0 )
			{
				cout << "By how much?" << endl;
				while( bExtraTry != 0)
				{
					if(( cin >> returnMe ))
					{
						if( returnMe > 0 )
						{
							returnMe += ViewTable().GetBetToCall();
							bExtraTry = 0;
						}

						cin.ignore(2000, '\n');
						cin.sync();


					}
					else
					{
						cin.clear();
						cin.ignore(80,'\n');
						cin.sync();
					}
				}
			}
			else if ( strncmp(inputBuf, "raiseto", 7) == 0 )
			{
				cout << "To how much?" << endl;
				while( bExtraTry != 0)
				{
					if(( cin >> returnMe ))
					{
						if( returnMe > ViewTable().GetBetToCall() )
						{
							bExtraTry = 0;
						}

						cin.ignore(2000, '\n');
                        cin.sync();

					}
					else
					{
						cin.clear();
						cin.ignore(2000,'\n');
						cin.sync();
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
			}
			else
			{
				cout << inputBuf << " is not a correct choice." << endl;
			}
		}
		else //error on input
		{
			cin.clear();
			cin.ignore(2000,'\n');
			cin.sync();
		}

	//End of loop
	}
#ifdef USERFEEDBACK
	cout << "To bet " << returnMe << endl;



	cout << "Command accepted" << endl;
#endif
	return returnMe;
}

