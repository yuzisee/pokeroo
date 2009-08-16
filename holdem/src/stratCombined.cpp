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


#include "stratCombined.h"


#ifdef TREND_STRAT_ENABLE

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

#endif

void MultiStrategy::SeeOppHand(const int8 pID, const Hand& h)
{
    if( pID == myPositionIndex ) bHandShown = true;
}

void MultiStrategy::SeeCommunity(const Hand& h, const int8 n)
{
    if( n == 0 )
    {
        if( prevMoney < 0 )
        {//If there is no previous hand, initialize
            if( !initM() )
            {//Arena new game
                //bookHandNumber = 1;
                prevMoney = -1;
            }
        }else
        {//Every hand runs through here
            SaveState();//SAVE STATE
            //bookHandNumber += 1;
        }




        float64 nowMoney = ViewPlayer().GetMoney();
        if( prevMoney > -1 )
        {
            const float64 avgBlind = ViewTable().GetBlindValues().AverageForcedBetPerHand(ViewTable().NumberAtTable());
            if( bGamble > 0 )
            {
                picks[currentStrategy].totalMoneyDelta -= avgBlind;
            }

            if (bHandShown)
            {//Revealed something
                picks[currentStrategy].numHandsAboveBelow -= 1;
            }


            if( nowMoney < prevMoney - ViewTable().GetChipDenom()/2 )
            {//Lost more than half a chip
                picks[currentStrategy].nonZeroWinLose -= 1 + bGamble;
                picks[currentStrategy].totalMoneyDelta -= (prevMoney - nowMoney);
            }
            else if( nowMoney > prevMoney + ViewTable().GetChipDenom()/2 )
            {//Won more than half a chip
                if( bGamble > 0 )
                {
                    picks[currentStrategy].nonZeroWinLose += (1 + bGamble)*(ViewTable().NumberAtTable()) - 1;
                    picks[currentStrategy].totalMoneyDelta += (nowMoney - prevMoney) - avgBlind;

                    if (!bHandShown)
                    {//Won a hand WITHOUT showing my hand
                        picks[currentStrategy].numHandsAboveBelow += 1;
                    }
                }
            }
        }

        //Pick the top strategy by sorting and selecting the favourite
        PerformanceHistory::SortAndOffset( picks, stratcount );
        currentStrategy = 0;

        strats[picks[currentStrategy].id]->Link(this);
        strats[picks[currentStrategy].id]->SoftOpenLogFile();
        picks[currentStrategy].score += 1;
        prevMoney = nowMoney;
        bHandShown = false;
    }

    strats[picks[currentStrategy].id]->SeeCommunity(h,n);
}

float64 MultiStrategy::MakeBet()
{
    return strats[picks[currentStrategy].id]->MakeBet();
}


bool MultiStrategy::initM()
{
    bool bLoadedState = LoadState();
//    if( bLoadedState ) bookHandNumber = ViewTable().handnum;
    currentStrategy = 0;
    strats[0]->Link(this);
    strats[0]->HardOpenLogFile();
    strats[0]->ReleaseLogFile();
    return bLoadedState;
}

void MultiStrategy::Unserialize( std::istream& loadFile )
{
//    loadFile >> bookHandNumber;
//    bookHandNumber -= 1;
    prevMoney = HoldemUtil::ReadFloat64(loadFile);
    for(uint8 i=0;i<stratcount;++i)
    {
        picks[i] = HistoryStrategy::UnserializeOne(loadFile);
    }
}

void MultiStrategy::Serialize( std::ostream& saveFile )
{
//    saveFile << (bookHandNumber+1) << endl;
    HoldemUtil::WriteFloat64(saveFile,prevMoney);
    saveFile << endl;
    for(uint8 i=0;i<stratcount;++i)
    {
        HistoryStrategy::SerializeOne(saveFile,picks[i]);
    }
}
