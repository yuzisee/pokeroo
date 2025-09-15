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


#ifndef HOLDEM_AICache
#define HOLDEM_AICache

#include "aiInformation.h"
#include <string>
#include <fstream>

#define DEF_CACHEABLE_MIN 0

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

class PreflopCallStats;

// [!TIP]
// SerializeC and UnserializeC and `*.holdemC` files are CallStats containing the `CallCumulation` used by QueryDefense (a vector of StatResult corresponding to each possible hand your opponent could have)
// SerializeW and UnserializeW and `*.holdemW` files are for Query and give a `DistrShape` (the general statistics about how good your hand is)
// See `holdem/src/inferentials.h` for more
class StatsManager
{
private:
    static const char*	CONFIGFILENAME;
    static const int8	CACHEABLESTAGE;


    #ifdef GLOBAL_AICACHE_SPEEDUP
    static CommunityPlus dsCommunity;
    #endif

    static void initPath();
    static bool readPathFromIni();
    static bool readPathFromEnv();

    static void StatResultToJSON(std::stringstream& dataf, const StatResult &target);
    static void serializeDistrShape(ofstream& dataf, const DistrShape& d);
    static void serializeStatResult(ofstream& dataf, const StatResult& d);
    static bool unserializeDistrShape(ifstream& dataf, DistrShape* d);
    static bool unserializeStatResult(ifstream& dataf, StatResult* d);

protected:
    static void SerializeC(ofstream& dataf, const CallCumulation& q);
    static void SerializeW(ofstream& dataf, const DistrShape& dPCT);
    static bool UnserializeC(ifstream& dataf, CallCumulation& q);
    static bool UnserializeW(ifstream& dataf, DistrShape* dPCT);
    static string baseDataPath;

#ifdef PROGRESSUPDATE
  public:
#endif
  static void holdemWtoJSON(std::stringstream& dataf, const DistrShape& dPCT);
  static void holdemCtoJSON(std::stringstream& dataf, const CallCumulation& q);
#ifndef PROGRESSUPDATE
  public:
#endif

    static const string& dbFolderPath() { StatsManager::initPath(); return baseDataPath; }

    static string dbFileName(const Hand& withCommunity, const Hand& onlyCommunity, const string label);

    // relies on `*.holdemW`
    static void Query(DistrShape* dPCT, const CommunityPlus& withCommunity, const CommunityPlus& onlyCommunity, int8 n);

    // relies on `*.holdemC`
    static void QueryDefense(CallCumulation& q, const CommunityPlus& withCommunity, const CommunityPlus& onlyCommunity, int8 n);

    #ifdef GLOBAL_AICACHE_SPEEDUP
    static void QueryOffense(CallCumulation& q, const CommunityPlus& withCommunity, const CommunityPlus& onlyCommunity, int8 n, CommunityCallStats  ** lastds=0);
    #else
    static void QueryOffense(CallCumulation& q, const CommunityPlus& withCommunity, const CommunityPlus& onlyCommunity, int8 n);
    #endif
    static void QueryOffense(CallCumulation& q, const CommunityPlus& withCommunity);
}
;

/**
 * An implementation of CallStats that uses StatsManager::Query() to lookup its values from the "opening book" database.
 * In contrast, a typical CallStats would compute its values at runtime.
 *
 * On a typical machine you'll want to use this for pre-flop because it is too slow to compute dynamically.
 * Furthermore, StatsManager::Query is designed to ship with an "opening book" database for the 169 the pre-flop hole cards anyway.
 */
class PreflopCallStats : public virtual CallStats
{
    private:
        void initPC();

        int8 largestDiscount(int8 * discount);
        int8 oppSuitedOcc(int8 * discount, char mySuited );
        int8 getDiscount(const char carda, const char cardb);
        int8 popSet(const int8 carda, const int8 cardb);

        //const virtual int8 realCardsAvailable(const int8 cardsInCommunity) const;
    public:
        PreflopCallStats(const CommunityPlus& hP, const CommunityPlus& onlycommunity)
            : PlayStats(hP,onlycommunity),CallStats(hP,onlycommunity,0)
            {
                initPC();
            }

    /** Call popSet() once for each of the 169 proflop hole card combinations */
    virtual void AutoPopulate();
}
;

/**
 * NamePockets() will return a unique name for your pocket cards if there are exactly two cards dealt.
 * If there are more than two cards dealt, the behaviour is undefined probably (or crashes)
 */
class NamedTriviaDeck : public TriviaDeck
{
    public:
    string NamePockets() const;

}
;


#endif
