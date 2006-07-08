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

//#define LOGTHRESHOLD

#include <math.h>
#include "stratThreshold.h"
ThresholdStrategy::~ThresholdStrategy()
{
}

void ThresholdStrategy::SeeCommunity(const Hand& h, const int8 cardsInCommunity)
{



    if( 0 == w ) w = new DistrShape(0);

    CommunityPlus onlyCommunity;
    onlyCommunity.SetUnique(h);

    CommunityPlus withCommunity;
    withCommunity.SetUnique(ViewHand());
    withCommunity.AppendUnique(onlyCommunity);



    StatsManager::Query(0,w,0,withCommunity,onlyCommunity,cardsInCommunity);

}

float64 ThresholdStrategy::MakeBet()
{
        #ifdef LOGTHRESHOLD
            cout << w->mean << " > " << aiThreshold << "?" << endl;
        #endif
	if (w->mean > aiThreshold)
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
    float64 multiThreshhold = pow(w->mean,ViewTable().GetNumberInHand());
        #ifdef LOGTHRESHOLD
            HandPlus convertOutput;
            convertOutput.SetUnique(ViewHand());
            convertOutput.DisplayHand();
            cout << "ThresholdAI" << endl;

            cout << multiThreshhold << " = " << w->mean << "^" << (int)(ViewTable().GetNumberInHand()) << endl;
        #endif
	if (multiThreshhold > aiThreshold)
	{
		return ViewPlayer().GetMoney();
	}
	else
	{
		return ThresholdStrategy::MakeBet();
	}
}
