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

#include "callRarity.h"
#include "BluffGainInc.h"
//#include "stratSearch.h"

#include "debug_flags.h"



///RULE OF THUMB?
///It looks like increasing winPCT loosens up the player
///However, you add aggressiveness by modifying exf?



class PositionalStrategy : virtual public PlayerStrategy
{
public:
    static void printCommunityOutcomes(std::ostream &logF, const CoarseCommunityHistogram &h, const DistrShape &distrPct);


    protected:
        static void printStateModel(std::ofstream &logF, float64 betSize, StateModel &choicemodel, const Player &me);
        static void printPessimisticWinPct(std::ofstream & logF, float64 betSize, CombinedStatResultsPessimistic * csrp);

        template< typename T >
        void printBetGradient(ofstream &logF, ExactCallBluffD & rl, ExactCallBluffD & rr, T & m, ExpectedCallD & tablestate, float64 separatorBet, CombinedStatResultsPessimistic * csrp) const;

        void printFoldGain(float64 raiseGain, CallCumulationD * e, ExpectedCallD & estat);



		DistrShape detailPCT;
		StatResultProbabilities statprob;


		float64 myMoney;


        float64 highBet;
        float64 betToCall;

        float64 myBet;
        float64 maxShowdown;

        struct PositionalStrategyLogOptions logOptions;
        #ifdef LOGPOSITION
        ofstream logFile;
        #endif

        // Shared logging code
        void printCommon(const ExpectedCallD &tablestate);

        void setupPosition();
        float64 solveGainModel(HoldemFunctionModel*, CallCumulationD* const e);
	public:

        void HardOpenLogFile();
        void SoftOpenLogFile();
        void ReleaseLogFile();

    PositionalStrategy(const std::string &logfilename, bool bMean=false,bool bRanking=false,bool bWorse=true,bool bHybrid=false )
    : PlayerStrategy()
    ,
    detailPCT(DistrShape::newEmptyDistrShape())
    ,
    fLogFilename(logfilename)
        {
            logOptions.bLogMean = bMean;
            logOptions.bLogRanking = bRanking;
            logOptions.bLogWorse = bWorse;
            logOptions.bLogHybrid = bHybrid;
        }
		virtual ~PositionalStrategy();


		virtual void SeeCommunity(const Hand&, const int8);
		virtual float64 MakeBet() = 0;
		virtual void SeeOppHand(const int8, const Hand&){};
        virtual void SeeAction(const HoldemAction&);
        virtual void FinishHand(){};

    #ifdef DUMP_CSV_PLOTS
    string csvpre;
    #endif

private:
    // The destination where this bot will write its log.
    const std::string fLogFilename;
}
;


class ImproveGainStrategy : public PositionalStrategy
{
protected:
    int8 bGamble;
public:
    ImproveGainStrategy(const std::string &logfilename, int8 riskymode =0) : PositionalStrategy(logfilename, riskymode ? false : true,true,riskymode ? true : false,false), bGamble(riskymode) {}

    float64 MakeBet() override final;
}
;

//Deterrent
class DeterredGainStrategy : public PositionalStrategy
{
    protected:
    int8 bGamble;
    public:
    DeterredGainStrategy(const std::string &logfilename, int8 riskymode) : PositionalStrategy(logfilename, riskymode ? false : true,true,false,false), bGamble(riskymode) {}

    virtual float64 MakeBet();
}
;

/**
 * The basic premise here is based on the following discussion:
 *  Why don't you bet all-in? Is it because of the Kelly criterion, or is it because of optimism?
 *  Why don't you _call_ all-in? Is it because of the Kelley criterion, or is it because of optimism?
 *
 * "If someone bets all-in, and you have the option to call it, you pick your spots. You can wait for a better opportunity. Obviously you don't take this one because you are putting too much of your money at risk at once" (thus, use Kelly criterion, a.k.a. GainModel, when calling or less, and use AlgbModel, a.k.a. GainModelNoRisk for raising)
 * "If you are the one raising all-in, the deterrent for not always doing this is that you will only be called by good hands." (Thus, we should use AlgbModel as the default for raises but scale from X to statworse as your raise gets higher.)
 * -Nav

 * So:
 * + Use CombinedStatResultsPessimistic when raising (any amount)
 * + Use geom to call, always. Since calling is the smallest bet, do we hard algb/worse for any raise? Sure. At least it's simple. Let's try it.
 *
 */
class PureGainStrategy : public PositionalStrategy
{
protected:
    int8 bGamble;
public:
    PureGainStrategy(const std::string &logfilename, int8 riskymode) : PositionalStrategy(logfilename, true,true,false,false), bGamble(riskymode) {}

    virtual float64 MakeBet();
}
;




#endif
