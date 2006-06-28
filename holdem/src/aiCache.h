/***************************************************************************
 *   Copyright (C) 2006 by Joseph Huang                                    *
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


#ifndef HOLDEM_AICache
#define HOLDEM_AICache

#include "engine.h"
#include "ai.h"
#include <string>
#include <iostream>  // I/O
#include <fstream>   // file I/O

using std::string;

//#include <iomanip.h>   // format manipulation

/*
One must store:
myChancesEach; (init?)
mytotalChances; (init?)
statCount;
moreCards;
(statGroup;)
(currentCard;)
(myWins;)
*/

/*
CallStats:
cumulation (vector)
*/

/*
WinStats:
myAvg
[PCT,W/(W+L),Pyth]
	AvgDev
	StdDev
	Improve
	Skew
	Kurtosis
*/

class CacheManager
{
private:
    FILE* configfile;

	WinStats* myCachedW;
	CallStats* myCachedC;
	DealRemainder myStatBuilder;

	void initCM();

	string dbFileName(const Hand& withCommunity, const Hand& onlyCommunity);

public:

    void Query(WinStats& q, const Hand& withCommunity, const Hand& onlyCommunity, int8 n);
    void Query(CallStats& q, const Hand& withCommunity, const Hand& onlyCommunity, int8 n);

	CacheManager() : myCachedW(0), myCachedC(0)
	{
	    initCM();
	}

}
;

class TriviaDeck : public OrderedDeck
{
    private:
        const static uint32 largestCard(uint32 suitcards);
    public:
    string NamePockets() const;
    const void DiffHand(const Hand&);
}
;


#endif
