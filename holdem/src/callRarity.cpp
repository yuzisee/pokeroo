
#include "callRarity.h"

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

