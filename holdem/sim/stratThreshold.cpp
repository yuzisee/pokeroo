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

#define ALLBET 21

#include <math.h>
#include "stratThreshold.h"
ThresholdStrategy::~ThresholdStrategy()
{
    #ifdef LOGTHRESHOLD
        if( logFile.is_open() )
        {
            logFile.close();
        }
    #endif
}

void MultiThresholdStrategy::SeeCommunity(const Hand& h, const int8 cardsInCommunity)
{
    #ifdef LOGTHRESHOLD
        if( !(logFile.is_open()) )
        {
            logFile.open((ViewPlayer().GetIdent() + ".MultiThresh.txt").c_str());
        }
    #endif
    ThresholdStrategy::SeeCommunity(h, cardsInCommunity);
}

void ThresholdStrategy::SeeCommunity(const Hand& h, const int8 cardsInCommunity)
{



    if( 0 == w ) w = new DistrShape(DistrShape::newEmptyDistrShape());

    CommunityPlus onlyCommunity;
    onlyCommunity.SetUnique(h);

    CommunityPlus withCommunity;
    withCommunity.SetUnique(ViewDealtHand());
    withCommunity.AppendUnique(onlyCommunity);



    StatsManager::Query(w,withCommunity,onlyCommunity,cardsInCommunity);

        #ifdef LOGTHRESHOLD
            if( !(logFile.is_open()) )
            {
                logFile.open((ViewPlayer().GetIdent() + ".Thresh.txt").c_str());
            }
            logFile << endl;
            if( !(Hand::EMPTY_HAND == h) )
            {
                HandPlus::DisplayHand(logFile, h);
                logFile << "community" << endl;
            }
            else
            {
                    logFile << "==========#" << ViewTable().handnum << "==========" << endl;
            }
        #endif

}

float64 ThresholdStrategy::MakeBet()
{
        #ifdef LOGTHRESHOLD
            if( !(logFile.is_open()) )
            {
                logFile.open((ViewPlayer().GetIdent() + ".Thresh.txt").c_str());
            }

            logFile << w->mean.pct << " > " << aiThreshold << "?" << endl;
        #endif
	if (w->mean.pct > aiThreshold)
	{
		return ViewTable().GetBetToCall();
	}
	else
	{
		return ViewPlayer().GetBetSize();
	}
}

float64 MultiThresholdStrategy::MakeBet()
{
    const float64 bb = ViewTable().GetBlindValues().GetBigBlind();

    if( bCall == 4 ) return ViewTable().GetBetToCall() + ALLBET;
    if( bCall == 3 )
    {
        //if( w->mean < 1.0/220.0 ) return ViewPlayer().GetBetSize();

        const float64 defaultBetUp = ((ViewTable().GetDeadPotSize() + bb) / 2.0 + bb*3); // /w->mean;

        const bool bRiver = (ViewTable().FutureRounds() == 0);
        const bool bHandSucks = (w->mean.pct < 0.5);

        if( bRiver && bHandSucks )  return 0;
        else                        return defaultBetUp;
    }

    // TODO(from yuzisee): handsToBeat() here.
	const playernumber_t toBeat = ViewTable().NumberStartedRoundInclAllIn()-1; // This is the "established" hand strength requirement of anyone willing to claim they will win this hand.


    float64 multiThreshhold = std::pow(w->mean.pct,toBeat+redundancy); //subtract yourself
        #ifdef LOGTHRESHOLD

            if( !(logFile.is_open()) )
            {
                logFile.open((ViewPlayer().GetIdent() + ".MultiThresh.txt").c_str());
            }

            HandPlus::DisplayHand(logFile, ViewDealtHand().hand_logic().hand_impl);
            logFile << "ThresholdAI" << endl;

            logFile << multiThreshhold << " = " << w->mean.pct << "^" << (int)(toBeat+redundancy) << endl;
        #endif

	if (multiThreshhold > aiThreshold)
	{
		return ViewPlayer().GetMoney();
	}
	else
	{
	    #ifdef LOGTHRESHOLD
	    logFile << "call" << endl;
	    #endif
        if( bCall == 2 )
	    {
	        if( ThresholdStrategy::MakeBet() > ViewPlayer().GetBetSize() ) return bb*2;
        }
		if( bCall == 1 ) return ThresholdStrategy::MakeBet();
		return ViewPlayer().GetBetSize();
	}
}
