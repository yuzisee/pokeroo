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

#ifndef HOLDEM_ShowdownAllIns
#define HOLDEM_ShowdownAllIns

#include "ai.h"

class AllInShowdown : virtual public PlayStats
{

    //friend void StatsManager::Query(CallCumulation& q, const CommunityPlus& withCommunity, const CommunityPlus& onlyCommunity, int8 n);
private:
	void initS(const int8);
protected:

	CommunityPlus* oppUndo;

    
public:
        float64 pctWillCall(const float64) const;
    
    //double myCallPct(double); //give pct of HIS percieved bankroll and returns chance to call
    virtual void Analyze();
    virtual void DropCard(const DeckLocation);
    virtual StatRequest NewCard(const DeckLocation, const float64 occ);
    
	AllInShowdown(const CommunityPlus& hP, const CommunityPlus& onlycommunity,
              int8 cardsInCommunity) : PlayStats(hP,onlycommunity)
	{
                  initS(cardsInCommunity);
	}
	~AllInShowdown();
    

}
;
