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

#ifndef HOLDEM_PositionalStrat
#define HOLDEM_PositionalStrat

#include "BluffGainInc.h"
//#include "stratSearch.h"
#include "ai.h"

#include "debug_flags.h"



///RULE OF THUMB?
///It looks like increasing winPCT loosens up the player
///However, you add aggressiveness by modifying exf?



class PositionalStrategy : virtual public PlayerStrategy
{
    protected:
        template< typename T >
        void printBetGradient(ExactCallBluffD & rl, ExactCallBluffD & rr, T & m, ExpectedCallD & tablestate, float64 separatorBet);


        DistrShape detailPCT;
        StatResult statmean;
        StatResult statworse;
        StatResult statranking;
        StatResult statrelation;
        StatResult hybridMagnified;
        CallCumulationD foldcumu;
        CallCumulationD callcumu;
		CallCumulationFlat rankcumu;
        float64 myMoney;


        float64 highBet;
        float64 betToCall;

        float64 myBet;
        float64 maxShowdown;


        bool bLogMean;
        bool bLogRanking;
        bool bLogWorse;
        bool bLogHybrid;
        #ifdef LOGPOSITION
        ofstream logFile;
        #endif

        void setupPosition();
        float64 solveGainModel(HoldemFunctionModel*, CallCumulationD* const e);
	public:

        void HardOpenLogFile();
        void SoftOpenLogFile();
        void ReleaseLogFile();

		PositionalStrategy( bool bMean=false,bool bRanking=false,bool bWorse=true,bool bHybrid=false ) : PlayerStrategy(), detailPCT(0), bLogMean(bMean), bLogRanking(bRanking), bLogWorse(bWorse), bLogHybrid(bHybrid) {}
		virtual ~PositionalStrategy();


		virtual void SeeCommunity(const Hand&, const int8);
		virtual float64 MakeBet() = 0;
		virtual void SeeOppHand(const int8, const Hand&){};
        virtual void SeeAction(const HoldemAction&) {};
        virtual void FinishHand(){};

    #ifdef DUMP_CSV_PLOTS
    string csvpre;
    #endif
}
;


class ImproveGainStrategy : public PositionalStrategy
{
protected:
    int8 bGamble;
public:
    ImproveGainStrategy(int8 riskymode =0) : PositionalStrategy(riskymode ? false : true,true,riskymode ? true : false,false), bGamble(riskymode) {}

    virtual float64 MakeBet();
}
;

//Deterrent
class DeterredGainStrategy : public PositionalStrategy
{
    protected:
    int8 bGamble;
    public:
    DeterredGainStrategy(int8 riskymode =0) : PositionalStrategy(false,true,false,true), bGamble(riskymode) {}

    virtual float64 MakeBet();
}
;




#endif
