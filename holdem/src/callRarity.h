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


#ifndef HOLDEM_CallRarity
#define HOLDEM_CallRarity

#include "debug_flags.h"
#include "portability.h"

#include "inferentials.h"
#include "holdem2.h"

#ifdef LOGPOSITION
  #include <iostream>
#endif


class OpponentStandard
{
private:
	static const playernumber_t NO_FIRST_ACTION_PLAYER;
protected:
	playernumber_t foldsObserved;
	bool bMyActionOccurred;
	bool bNonBlindNonFoldObserved;
	playernumber_t firstActionPlayers;
    playernumber_t roundStartPlayers;

	/*
	playernumber_t firstActionPlayers;
    playernumber_t notActedPlayers;
	*/
public:
	void NewRound(const playernumber_t playersStartingRound);
	void SeeFold();
	void SeeNonBlindNonFold(const playernumber_t playersRemaining);
    
    playernumber_t getNumPlayersAtFirstAction();
}
;

struct PositionalStrategyLogOptions
{
    bool bLogMean;
    bool bLogRanking;
    bool bLogWorse;
    bool bLogHybrid;
};


class StatResultProbabilities
{
private:
///Equations, Logging, etc.

    void logfileAppendPercentage(std::ostream &logFile, const char * label, const float64 vpct)
    {
        logFile << "(";
        logFile << label;
        logFile << ") " << vpct * 100 << "%"  << std::endl;
    }
    void logfileAppendPercentages(std::ostream &logFile, const bool bWrite, const char * label, const char * label_w, const char * label_s, const char * label_l, const StatResult vpcts)
    {
        if(bWrite)
        {
            if( label ) logfileAppendPercentage(logFile, label,vpcts.pct);
            if( label_w ) logfileAppendPercentage(logFile, label_w,vpcts.wins);
            if( label_s ) logfileAppendPercentage(logFile, label_s,vpcts.splits);
            if( label_l ) logfileAppendPercentage(logFile, label_l,vpcts.loss);
        }
    }
//protected:
// TODO(from jdhuang)
public:

///Single Opponent Probabilities
    StatResult statworse;   // Probability of winning against the worst-case opposing hand
    
    StatResult statrelation; // Against what fraction of opponents will you have the better hand?
    StatResult statranking; // How often do you get a hand this good?
    // The distinction here is that some hands with typically high change of winning in general may do poorly against the particular hand you're holding, and vice versa.

	StatResult hybridMagnified;

///Group Probabilities (a function of 

public:
///Core Probabilities
    CallCumulationD foldcumu; // CallStats (probabiliy of winning against each possible opponent hand)
    CallCumulationD callcumu; // CommunityCallStats (each hole cards' inherent probability of winning)
	StatResult statmean; // (Your hole cards' current inherent probability of winning)
	
///Initialize foldcumu, callcumu, and statmean, and then call Process_FoldCallMean().
	void Process_FoldCallMean();

    void logfileAppendStatResultProbabilities(struct PositionalStrategyLogOptions const &logOptions, std::ostream &logFile);


}
;

#endif // HOLDEM_CallRarity

