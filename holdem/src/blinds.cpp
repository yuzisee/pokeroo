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

#include "blinds.h"
#include <math.h>

const playernumber_t BlindValues::playersInBlind = PLAYERS_IN_BLIND;

struct BlindUpdate GeomPlayerBlinds::UpdateSituation(struct BlindUpdate currentBlinds, struct BlindUpdate updateSituation)
{
    updateSituation.b = currentBlinds.b;
    updateSituation.bNew = false;


    playernumber_t playersEliminated = updateSituation.playersLeft - currentBlinds.playersLeft;
    while( playersEliminated > 0 )
    {
        updateSituation.b *= incrRatio;
        updateSituation.bNew = true;
        --playersEliminated;
    }
    return updateSituation;
}

struct BlindUpdate StackPlayerBlinds::UpdateSituation(struct BlindUpdate currentBlinds, struct BlindUpdate updateSituation)
{
    updateSituation.bNew = (updateSituation.playersLeft > currentBlinds.playersLeft);

    const float64 averageStackSmallBlind = (totalChips/updateSituation.playersLeft)*smallBlindRatio;

    updateSituation.b.SetSmallBigBlind(averageStackSmallBlind);

    return updateSituation;
}


void SitAndGoBlinds::InitializeHist(const float64 small,const float64 big)
{
    const float64 rat = small/big;
        hist[0] = small*rat*rat;
        hist[1] = small*rat;
        hist[2] = hist[0]+hist[1];

    nextUpdate = 1+handPeriod;
}

float64 SitAndGoBlinds::fibIncr(float64 a, float64 b)
{
    const float64 roundBy = b-a;
    return floor((a+b)/roundBy)*roundBy;
}

struct BlindUpdate SitAndGoBlinds::UpdateSituation(struct BlindUpdate currentBlinds, struct BlindUpdate updateSituation)
{
    updateSituation.bNew = (updateSituation.handNumber >= nextUpdate);

    if( updateSituation.bNew )
    {
        nextUpdate = updateSituation.handNumber + handPeriod;

        updateSituation.b.SetSmallBigBlind(hist[0] + hist[1] + hist[2]);

        const float64 newHist = fibIncr(hist[2],hist[1]);

        hist[0] = hist[1];
        hist[1] = hist[2];
        hist[2] = newHist;
    }

    return updateSituation;

}

void SitAndGoBlinds::Serialize(std::ostream & saveToFile)
{
    HoldemUtil::WriteFloat64( saveToFile, hist[0] );
    saveToFile << "$";

    HoldemUtil::WriteFloat64( saveToFile, hist[1] );
    saveToFile << "$";

    HoldemUtil::WriteFloat64( saveToFile, hist[2] );
    saveToFile << "=";

    saveToFile << nextUpdate << ";";
}

void SitAndGoBlinds::UnSerialize(std::istream & restoreFromFile)
{
    hist[0] = HoldemUtil::ReadFloat64( restoreFromFile );
    restoreFromFile.ignore(1,'$');

    hist[1] = HoldemUtil::ReadFloat64( restoreFromFile );
    restoreFromFile.ignore(1,'$');

    hist[2] = HoldemUtil::ReadFloat64( restoreFromFile );
    restoreFromFile.ignore(1,'=');

    restoreFromFile >> nextUpdate;
    restoreFromFile.ignore(1,';');
}
