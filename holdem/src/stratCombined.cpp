

/***************************************************************************
 *   Copyright (C) 2006 by Joseph Huang                                    *
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


#include "stratCombined.h"



uint32 TrendStrategy::RandomSeed()
{
    CallCumulationD possibleHands;
            StatResult randomSeed;



            CommunityPlus withCommunity;
            withCommunity.SetUnique(ViewHand());

            ViewTable().CachedQueryOffense(possibleHands,withCommunity);

            DistrShape tempP(0), tempW(0);

            StatsManager::Query(0,&tempP,&tempW,withCommunity,CommunityPlus::EMPTY_COMPLUS,0);
            StatResult myRank = GainModel::ComposeBreakdown(tempP.mean,tempW.mean);


            const float64 ranking3 = possibleHands.pctWillCall_tiefactor(1 - myRank.pct, 1); //wins+splits
            const float64 ranking = possibleHands.pctWillCall_tiefactor(1 - myRank.pct, 0); //wins

            randomSeed.wins = ranking;
            randomSeed.splits = ranking3 - ranking;
            randomSeed.loss = 1 - ranking3;
            randomSeed.genPCT();

}

void TrendStrategy::SeeCommunity(const Hand& h, const int8 n)
{

    int8 indexStrategy;// = order[currentStrategy];

    if( n == 0 )
    {
        handNumber += 1;

        if( PassCount( picks[indexStrategy] ) < 2 )
        {
            ++currentStrategy;
            RestartTrend();
        }

        if( currentStrategy == stratcount )
        {

            RestartCycle(RandomSeed());
        }
    }


    strats[indexStrategy]->SeeCommunity(h,n);
}




void MultiStrategy::SeeCommunity(const Hand& h, const int8 n)
{
    if( n == 0 )
    {

        SaveState();//SAVE STATE

        handNumber += 1;

        float64 nowMoney = ViewPlayer().GetMoney();
        if( prevMoney > -1 )
        {
            if( nowMoney < prevMoney - ViewTable().GetChipDenom()/2 )
            {//Lost more than half a chip
                picks[currentStrategy].nonZeroWinLose -= 1;
                picks[currentStrategy].totalMoneyDelta -= (prevMoney - nowMoney);
            }/*
            else if( nowMoney > prevMoney + ViewTable().GetChipDenom()/2 )
            {//Won more than half a chip

            }*/
        }

        //Pick the top strategy
        PerformanceHistory::SortAndOffset( picks, stratcount );
        currentStrategy = 0;

        prevMoney = nowMoney;
    }

    strats[currentStrategy]->SeeCommunity(h,n);
}

float64 MultiStrategy::MakeBet()
{
    return strats[currentStrategy]->MakeBet();
}

void MultiStrategy::initM()
{
    LoadState();
}

void MultiStrategy::Unserialize( std::istream& loadFile )
{
    loadFile >> handNumber;
    handNumber -= 1;
    prevMoney = HoldemUtil::ReadFloat64(loadFile);
    for(uint8 i=0;i<stratcount;++i)
    {
        picks[i] = HistoryStrategy::UnserializeOne(loadFile);
    }
}

void MultiStrategy::Serialize( std::ostream& saveFile )
{
    saveFile << (handNumber+1) << endl;
    HoldemUtil::WriteFloat64(saveFile,prevMoney);
    saveFile << endl;
    for(uint8 i=0;i<stratcount;++i)
    {
        HistoryStrategy::SerializeOne(saveFile,picks[i]);
    }
}
