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

#ifndef HOLDEM_PositionalStrat
#define HOLDEM_PositionalStrat

#include "functionmodel.h"
#include "stratSearch.h"
#include "ai.h"



#define LOGPOSITION
//#define DEBUGSPECIFIC
//#define ARBITARY_DISTANCE


#define BGAMBLE_MAX 8

///RULE OF THUMB?
///It looks like increasing winPCT loosens up the player
///However, you add aggressiveness by modifying exf?



class PositionalStrategy : virtual public PlayerStrategy
{
    protected:

#ifdef ARBITARY_DISTANCE
        int8 roundNumber[3];
#endif
        DistrShape detailPCT;
        StatResult statmean;
        StatResult statworse;
        StatResult statranking;
        StatResult hybridMagnified;
        CallCumulationD foldcumu;
        CallCumulationD callcumu;

        float64 myMoney;


        float64 highBet;
        float64 betToCall;

        float64 myBet;
        float64 maxShowdown;
        float64 expectedVS;

        #ifdef LOGPOSITION
        bool bLogMean;
        bool bLogRanking;
        bool bLogWorse;
        bool bLogHybrid;
        ofstream logFile;
        #endif

        void setupPosition();
        float64 solveGainModel(HoldemFunctionModel*);
	public:

		PositionalStrategy( bool bMean=false,bool bRanking=false,bool bWorse=false,bool bHybrid=false ) : PlayerStrategy(), detailPCT(0), bLogMean(bMean), bLogRanking(bRanking), bLogWorse(bWorse), bLogHybrid(bHybrid) {}
		virtual ~PositionalStrategy();

		virtual void SeeCommunity(const Hand&, const int8);
		virtual float64 MakeBet() = 0;
		virtual void SeeOppHand(const int8, const Hand&){};
        virtual void SeeAction(const HoldemAction&) {};
}
;

//Scaled ImprovePure
class ImproveStrategy : public PositionalStrategy
{
    protected:
    int8 bGamble;
    public:
    ImproveStrategy(int8 riskymode =0) : PositionalStrategy(false,true,true,false), bGamble(riskymode) {}

    virtual float64 MakeBet();
}
;

//Deterrent
class DeterredGainStrategy : public PositionalStrategy
{
    protected:
    int8 bGamble;
    public:
    DeterredGainStrategy(int8 riskymode =0) : PositionalStrategy(false,false,false,true), bGamble(riskymode) {}

    virtual float64 MakeBet();
}
;

class HybridScalingStrategy : public PositionalStrategy
{
    protected:
    int8 bGamble;
    public:
    HybridScalingStrategy(int8 riskymode =0) : PositionalStrategy(true,true,false,false), bGamble(riskymode) {}

    virtual float64 MakeBet();
}
;

class CorePositionalStrategy : public PositionalStrategy
{
    private:
    const static bool lkupLogMean[BGAMBLE_MAX];
    const static bool lkupLogRanking[BGAMBLE_MAX];
    const static bool lkupLogWorse[BGAMBLE_MAX];
    const static bool lkupLogHybrid[BGAMBLE_MAX];
    protected:
    int8 bGamble;
    public:
    CorePositionalStrategy(int8 riskymode) : bGamble(riskymode) {}

    virtual float64 MakeBet();
}
;
/*
0   statranking, Geom <-- needs to prevent pot committal: {Use this when high chance to improve}
1   statmean, Geom <-- needs to prevent pot committal: {Use this when high chance to improve}
2   statworst, Algb <-- needs to play more hands
3   statranking, Algb <-- Callstation-ish, needs to play tighter
4   statmean, Algb <-- Callstation-ish, needs to play tighter
5   statranking, Geom, potCommit <-- PotCommit doesn't help when all in, and you need 100% anyways
6   statmean, Geom, potCommit



*/




#endif
