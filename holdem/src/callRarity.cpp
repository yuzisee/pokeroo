
#include "callRarity.h"
#include "arenaSave.h"

//TODO: Is there a disparity between notActedPlayers using Excl and firstActionPlayers using Incl?

const playernumber_t OpponentStandard::NO_FIRST_ACTION_PLAYER = 1;

void OpponentStandard::NewRound()
{
	//firstActionPlayers = 1;
    //notActedPlayers = ViewTable().NumberStartedRoundExclAllIn();
	foldsObserved = 0;
	bMyActionOccurred = false;
	bNonBlindNonFoldObserved = false;
	firstActionPlayers = NO_FIRST_ACTION_PLAYER;
}

void OpponentStandard::SeeFold()
{
	++foldsObserved;
	//if( notActedPlayers > 0 ) --notActedPlayers;
}

void OpponentStandard::SeeNonBlindNonFold(const playernumber_t playersRemaining)
{
	//if( bNonBlindNonFoldObserved ) return;
	if( !bNonBlindNonFoldObserved ) 
	{
		bNonBlindNonFoldObserved = true;
		firstActionPlayers = playersRemaining - foldsObserved;
	}
}
//&& (firstActionPlayers == 1) ) firstActionPlayers = ViewTable().NumberInHandInclAllIn();

void StatResultProbabilities::Process_FoldCallMean(const Hand& holeCards, const CommunityPlus& onlyCommunity)
{
///===============
///   statworse
///===============
	statworse = foldcumu.oddsAgainstBestTwoHands(); //GainModel::ComposeBreakdown(detailPCT.worst,w_wl.worst);

///==================
///   statrelation
///==================
	const float64 rarityA3 = foldcumu.Pr_haveWinPCT_orbetter(0.5);

//You can tie in rank if and only if you tie in mean
    statrelation.wins = 1 - rarityA3;
    statrelation.splits = statmean.splits;
    statrelation.loss = rarityA3;
    statrelation.forceRenormalize();

///==================
///   statranking
///==================
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