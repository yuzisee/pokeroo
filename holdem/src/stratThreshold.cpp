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

#include "stratThreshold.h"

/*
void ThresholdStrategy::cleanstats()
{
	if(w != 0) delete w;
	w = 0;
}
*/
ThresholdStrategy::~ThresholdStrategy()
{
	//cleanstats();
}

void ThresholdStrategy::SeeCommunity(const CommunityPlus& h, const int8 cardsInCommunity)
{
    CommunityPlus withCommunity;
    withCommunity.SetUnique(h);
    withCommunity.AppendUnique(ViewHand());
    StatsManager::Query(&w,0,0,withCommunity,h,cardsInCommunity);
}

float64 ThresholdStrategy::MakeBet()
{
	if (w.pct > aiThreshold)
	{
		return ViewPlayer().GetMoney();
	}
	else
	{
		return 0;
	}
}

