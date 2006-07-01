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
#include <fstream>


using std::string;
using std::ifstream;
using std::ofstream;

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
The two *distrShape
            [PCT,W/(W+L),Pyth]
                AvgDev
                StdDev
                Improve
                Skew
                Kurtosis
*/

class StatsManager
{
private:
    static const char*	CONFIGFILENAME;
    static const int8	CACHEABLESTAGE;

    static void initPath();

    static string dbFileName(const Hand& withCommunity, const Hand& onlyCommunity);

    static void serializeDistrShape(ofstream& dataf, const DistrShape& d);
    static void serializeStatResult(ofstream& dataf, const StatResult& d);
    static bool unserializeDistrShape(ifstream& dataf, DistrShape* d);
    static bool unserializeStatResult(ifstream& dataf, StatResult* d);

protected:
    static void SerializeC(ofstream& dataf, CallCumulation& q);
    static void SerializeW(ofstream& dataf, const StatResult& myAvg, const DistrShape& dPCT, const DistrShape& dWL);
    static bool UnserializeC(ifstream& dataf, CallCumulation& q);
    static bool UnserializeW(ifstream& dataf, StatResult* myAvg, DistrShape* dPCT, DistrShape* dWL);
    static string baseDataPath;
public:

    /*
    static void Query(DistrShape& dPCT, const Hand& withCommunity, const Hand& onlyCommunity, int8 n);
    static void Query(DistrShape& dPCT, StatResult& myAvg, const Hand& withCommunity, const Hand& onlyCommunity, int8 n);
    static void Query(DistrShape& dPCT, DistrShape& dWL, const Hand& withCommunity, const Hand& onlyCommunity, int8 n);
    */
    static void Query(StatResult* myAvg, DistrShape* dPCT, DistrShape* dWL, const CommunityPlus& withCommunity, const CommunityPlus& onlyCommunity, int8 n);
    static void Query(CallCumulation& q, const CommunityPlus& withCommunity, const CommunityPlus& onlyCommunity, int8 n);


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
