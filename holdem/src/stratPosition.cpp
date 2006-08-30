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

#include <math.h>
#include "stratPosition.h"
#include "functionmodel.h"

PositionalStrategy::~PositionalStrategy()
{
    #ifdef LOGPOSITION
        if( logFile.is_open() )
        {
            logFile.close();
        }
    #endif
}


void PositionalStrategy::SeeCommunity(const Hand& h, const int8 cardsInCommunity)
{
    roundNumber = cardsInCommunity*cardsInCommunity; /// Round number is [0,1,2,3]=[Pre-Flop,Right After Flop,Between Turn and River,Final]<-[0,3,4,5]
    roundNumber /= 8;

    DistrShape w_wl(0);
    DistrShape w_pct(0);

    CommunityPlus onlyCommunity;
    onlyCommunity.SetUnique(h);

    CommunityPlus withCommunity;
    withCommunity.SetUnique(ViewHand());
    withCommunity.AppendUnique(onlyCommunity);


    StatsManager::Query(callcumu,withCommunity,onlyCommunity,cardsInCommunity);
    StatsManager::Query(0,&w_pct,&w_wl,withCommunity,onlyCommunity,cardsInCommunity);
    statmean = GainModel::ComposeBreakdown(w_pct.mean,w_wl.mean);
    statworse = GainModel::ComposeBreakdown(w_pct.worst,w_wl.worst);

        #ifdef LOGPOSITION
            if( !(logFile.is_open()) )
            {
                logFile.open((ViewPlayer().GetIdent() + ".Positional.log").c_str());
            }
            logFile << endl;
            HandPlus convertOutput;
            if( !(convertOutput == h) )
            {
                convertOutput.SetUnique(h);
                convertOutput.DisplayHand(logFile);
                logFile << "community" << endl;
            }
        #endif

}

float64 PositionalStrategy::MakeBet()
{
    const float64 myMoney = ViewPlayer().GetMoney();
    const float64 betToCall = ViewTable().GetBetToCall();

    ExactCallD myExpectedCall(myPositionIndex, &(ViewTable()), &callcumu);
    //ZeroCallD myExpectedCall(myPositionIndex, *(ViewTable()), &callcumu);

    GainModel gainmean(statmean,&myExpectedCall);
    float64 goalPoint = gainmean.FindBestBet();
    float64 goalFold = gainmean.FindZero(goalPoint,myMoney);

    GainModel gainworse(statworse,&myExpectedCall);
    float64 leastPoint = gainworse.FindBestBet();
    float64 leastFold = gainworse.FindZero(leastPoint,myMoney);

    std::ofstream excel("functionlog.csv");
    if( !excel.is_open() ) std::cerr << "\n!functionlog.cvs file access denied" << std::endl;
    gainmean.breakdown(100,excel,ViewTable().GetBetToCall(),ViewPlayer().GetMoney());
    excel.close();
    excel.open("functionlog.safe.csv");
    if( !excel.is_open() ) std::cerr << "\n!functionlog.safe.cvs file access denied" << std::endl;
    gainworse.breakdown(100,excel,ViewTable().GetBetToCall(),ViewPlayer().GetMoney());
    excel.close();

        #ifdef LOGPOSITION

            if( !(logFile.is_open()) )
            {
                logFile.open((ViewPlayer().GetIdent() + ".Positional.log").c_str());
            }


            HandPlus convertOutput;
            convertOutput.SetUnique(ViewHand());
            convertOutput.DisplayHand(logFile);

            logFile << "Bet to call " << betToCall << endl;
            logFile << "Goal bet " << goalPoint << endl;
            logFile << "Fold bet " << goalFold << endl;
            logFile << "Minimum target " << leastPoint << endl;
            logFile << "Safe fold bet " << leastFold << endl;

            if( goalPoint < betToCall || leastPoint < betToCall )
            {
                cout << "Failed assertion" << endl;
                goalPoint = gainmean.FindBestBet();
                goalFold = gainmean.FindZero(goalPoint,myMoney);


                leastPoint = gainworse.FindBestBet();
                leastFold = gainworse.FindZero(leastPoint,myMoney);
            }
        #endif


    ///TODO: Enhance this. Maybe scale each separately depending on different factors? Ooooh.
    float64 choiceScale = roundNumber;
    choiceScale /= 4;
    float64 choicePoint = leastPoint * (1-choiceScale) + goalPoint * (choiceScale);
    float64 choiceFold = leastFold * (1-choiceScale) + goalFold * (choiceScale);

    if( choicePoint == choiceFold )
    {///The highest point was the point closest to zero.
        if( betToCall == choiceFold )
        {
            logFile << "CHECK/FOLD" << endl;
            return 0;
        }
    }

    if( betToCall <= choicePoint )
	{
	    logFile << "RAISETO " << choicePoint << endl;
		return choicePoint;
	}//else
    {
            #ifdef DEBUGASSERT
                if( !(betToCall >= choiceFold) )
                {
                    cout << "ALL choiceXXXX should be AT LEAST betToCall as defined!" << endl;
                }
            #endif
       /* if( betToCall >= choiceFold )
        {
            logFile << "FOLD" << endl;
            return 0;
        }//else*/
        {
            logFile << "CALL " << choicePoint << endl;
            return betToCall;
        }
    }

}
