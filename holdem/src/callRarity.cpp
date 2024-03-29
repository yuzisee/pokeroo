
#include "callRarity.h"
#include "arenaSave.h"
#include <cmath>


void StatResultProbabilities::logfileAppendStatResultProbabilities(struct PositionalStrategyLogOptions const &logOptions, std::ostream &logFile) const
{
    logfileAppendPercentages(logFile, logOptions.bLogMean,"M","M.w","M.s","M.l",core.statmean);


    logfileAppendPercentages(logFile, logOptions.bLogRanking,"Better All-in",0,"Re.s",0,statrelation);
    logfileAppendPercentages(logFile, logOptions.bLogRanking,"Better Mean Rank",0,"Ra.s",0,statranking);

    logfileAppendPercentages(logFile, logOptions.bLogHybrid,"Geomean Win&Rank",0,"H.s",0,hybridMagnified);
}

void StatResultProbabilities::logfileAppendStatResultProbability_statworse(std::ostream &logFile, const StatResult & p, playernumber_t handsCompeting) const
{
    logFile << "Worst.handsCompeting = " << (int)handsCompeting << std::endl;
    logfileAppendPercentage(logFile, "Worst", p.pct);
    logfileAppendPercentages(logFile, true,0,"P.w","P.s","P.l",p);
}

void StatResultProbabilities::Process_FoldCallMean()
{
    statrelation = core.statRelation();
    statranking = core.statRanking();

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

    hybridMagnified.wins = sqrt(core.statmean.wins*statHybridR.wins);
    hybridMagnified.splits = sqrt(core.statmean.splits*statHybridR.splits);
    hybridMagnified.loss = sqrt(core.statmean.loss*statHybridR.loss);
    hybridMagnified.genPCT();
    const float64 adjust = hybridMagnified.wins + hybridMagnified.splits + hybridMagnified.loss;
    hybridMagnified = hybridMagnified * ( 1.0 / adjust );
    hybridMagnified.repeated = 0; ///.repeated WILL otherwise ACCUMULATE!
}
