
#include "callRarity.h"
#include "arenaSave.h"
#include <cmath>


void StatResultProbabilities::logfileAppendStatResultProbabilities(struct PositionalStrategyLogOptions const &logOptions, std::ostream &logFile)
{
    logfileAppendPercentages(logFile, logOptions.bLogMean,"M","M.w","M.s","M.l",statmean);

    logfileAppendPercentage(logFile, "Worst",statworse.pct);
    logfileAppendPercentages(logFile, logOptions.bLogWorse,0,"W.w","W.s","W.l",statworse);

    logfileAppendPercentages(logFile, logOptions.bLogRanking,"Better All-in",0,"Re.s",0,statrelation);
    logfileAppendPercentages(logFile, logOptions.bLogRanking,"Better Mean Rank",0,"Ra.s",0,statranking);

    logfileAppendPercentages(logFile, logOptions.bLogHybrid,"Geomean Win&Rank",0,"H.s",0,hybridMagnified);
}

void StatResultProbabilities::Process_FoldCallMean()
{
///===============
///   statworse
///===============
	statworse = foldcumu.oddsAgainstBestTwoHands(); //GainModel::ComposeBreakdown(detailPCT.worst,w_wl.worst);

///==================
///   statrelation
///==================
    // Against what fraction of opponents will you have the better hand?
	const float64 rarityA3 = foldcumu.Pr_haveWinPCT_orbetter(0.5);

//You can tie in rank if and only if you tie in mean
    statrelation.wins = 1 - rarityA3;
    statrelation.splits = statmean.splits;
    statrelation.loss = rarityA3;
    statrelation.forceRenormalize();

///==================
///   statranking
///==================
    // How often do you get a hand this good?
    const float64 rarity3 = callcumu.Pr_haveWinPCT_orbetter(statmean.pct);

    statranking.wins = 1 - rarity3;
    statranking.splits = statmean.splits;
    statranking.loss = rarity3;
    statranking.forceRenormalize();

///=====================
///   hybridMagnified
///=====================
//Pick the worse one for hybrid
    StatResult statHybridR;
    if( statranking.pct < statrelation.pct )
    {
        statHybridR = statranking;
    }else
    {
        statHybridR = statrelation;
    }

    hybridMagnified.wins = sqrt(statmean.wins*statHybridR.wins);
    hybridMagnified.splits = sqrt(statmean.splits*statHybridR.splits);
    hybridMagnified.loss = sqrt(statmean.loss*statHybridR.loss);
    hybridMagnified.genPCT();
    const float64 adjust = hybridMagnified.wins + hybridMagnified.splits + hybridMagnified.loss;
    hybridMagnified = hybridMagnified * ( 1.0 / adjust );
    hybridMagnified.repeated = 0; ///.repeated WILL otherwise ACCUMULATE!
}