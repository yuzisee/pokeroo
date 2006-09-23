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


    CommunityPlus onlyCommunity;
    onlyCommunity.SetUnique(h);

    CommunityPlus withCommunity;
    withCommunity.SetUnique(ViewHand());
    withCommunity.AppendUnique(onlyCommunity);


    StatsManager::QueryOffense(callcumu,withCommunity,onlyCommunity,cardsInCommunity);
    StatsManager::Query(0,&detailPCT,&w_wl,withCommunity,onlyCommunity,cardsInCommunity);
    statmean = GainModel::ComposeBreakdown(detailPCT.mean,w_wl.mean);
    statworse = GainModel::ComposeBreakdown(detailPCT.worst,w_wl.worst);

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

                    #ifdef GRAPHMONEY
            }

            else
            {
                    logFile << "==========#" << ViewTable().handnum << "==========" << endl;
                    #endif
            }
/*
            std::ofstream excel("positioninfo.txt");
            if( !excel.is_open() ) std::cerr << "\n!positioninfo.txt file access denied" << std::endl;

            excel << "Cards available to me" << endl;
            HandPlus uhand;
            uhand.SetUnique(ViewHand());
            uhand.DisplayHand(excel);
            excel << endl;
            excel << "Cards with community" << endl;
            uhand.SetUnique(withCommunity);
            uhand.DisplayHand(excel);
            excel << endl;

            cout << endl;

            CallCumulation::displayCallCumulation(excel, callcumu);
            excel << endl << endl << "(Mean) " << statmean.pct * 100 << "%"  << std::endl;
            excel << "(Worst) " << statworse.pct * 100 << "%"  << std::endl;
            excel.close();*/
        #endif

}

float64 PositionalStrategy::MakeBet()
{



    const float64 myMoney = ViewPlayer().GetMoney();
    float64 betToCall = ViewTable().GetBetToCall();
    const float64 myBet = ViewPlayer().GetBetSize();

    if( myMoney < betToCall ) betToCall = myMoney;





    ///TODO: Enhance this. Maybe scale each separately depending on different factors? Ooooh.
    ///I want you to remember that after the river is dealt and no more community cards will come, that
    ///skew and kurtosis are undefined and other DistrShape related values are meaningless.

    float64 maxShowdown = ViewTable().GetMaxShowdown();
    if( maxShowdown > myMoney ) maxShowdown = myMoney;
    float64 choiceScale = (betToCall - myBet)/(maxShowdown - myBet);
    if( maxShowdown == myBet || maxShowdown < betToCall ) choiceScale = 1;
    if( choiceScale > 1 ) choiceScale = 1;
    if( choiceScale < 0 ) choiceScale = 0;

    //float64 distrScale = 0.5 ;
    float64 distrScale = myMoney / ViewTable().GetAllChips() ;

    if( bGamble / 2 == 0 )/* 0,1 */
    {
        distrScale = 1 - distrScale;
    }/* else 2,3 */

    if( bGamble % 2 == 0 ) /* 0,2 */
    {
        distrScale += detailPCT.improve*detailPCT.stdDev;

    }else /* 1,3 */
    {
        distrScale -= detailPCT.improve*detailPCT.stdDev;
    }
    if( distrScale > 1 ) distrScale = 1;
    if( distrScale < 0 ) distrScale = 0;

    const StatResult statchoice = statworse * (1-choiceScale) + statmean * (choiceScale);


    ExactCallD myExpectedCall(myPositionIndex, &(ViewTable()), &callcumu);
    //ZeroCallD myExpectedCall(myPositionIndex, *(ViewTable()), &callcumu);


    //GainModel choicegain_r(statchoice,&myExpectedCall);
    GainModel choicegain_base(statchoice,&myExpectedCall);
    GainModelReverse choicegain_rev(statchoice,&myExpectedCall);

    GainModelNoRisk choicegain_nr(statworse,&myExpectedCall);
    GainModelReverseNoRisk choicegain_rnr(statworse,&myExpectedCall);
    //GainModelNoRisk gainmean_nr(statmean,&myExpectedCall);

	SlidingPairFunction gp(&choicegain_base,&choicegain_rev,distrScale,&myExpectedCall);
	SlidingPairFunction ap(&choicegain_nr,&choicegain_rnr,distrScale,&myExpectedCall);

    SlidingPairFunction choicegain(&gp,&ap,choiceScale/2,&myExpectedCall);
    //SlidingPairFunction gainmean(&gainmean_r,&gainmean_nr,choiceScale,&myExpectedCall);



    /*
    GainModel* targetModel = &choicegain;
    GainModel* targetFold = &gainmean;
    if( choiceScale == 1 )
    {
        targetModel = &choicegain_nr;
        targetFold = &gainmean_nr;
    }
*/

        #ifdef DEBUGSPECIFIC
        if( ViewTable().handnum == DEBUGSPECIFIC )
        {
            std::ofstream excel( (ViewPlayer().GetIdent() + "functionlog.csv").c_str() );
            if( !excel.is_open() ) std::cerr << "\n!functionlog.cvs file access denied" << std::endl;
            choicegain_base.breakdown(1000,excel,betToCall,maxShowdown);
            //myExpectedCall.breakdown(0.005,excel);

            excel.close();

            excel.open ((ViewPlayer().GetIdent() + "functionlog.reverse.csv").c_str());
            if( !excel.is_open() ) std::cerr << "\n!functionlog.cvs file access denied" << std::endl;
            choicegain_rev.breakdown(1000,excel,betToCall,maxShowdown);
            //myExpectedCall.breakdown(0.005,excel);

            excel.close();

            excel.open((ViewPlayer().GetIdent() + "functionlog.norisk.csv").c_str());
            if( !excel.is_open() ) std::cerr << "\n!functionlog.safe.cvs file access denied" << std::endl;
            choicegain_nr.breakdown(1000,excel,betToCall,maxShowdown);
            //g.breakdownE(40,excel);
            excel.close();



            excel.open((ViewPlayer().GetIdent() + "functionlog.anti.csv").c_str());
            if( !excel.is_open() ) std::cerr << "\n!functionlog.safe.cvs file access denied" << std::endl;
            choicegain_rnr.breakdown(1000,excel,betToCall,maxShowdown);
            //g.breakdownE(40,excel);
            excel.close();

          excel.open((ViewPlayer().GetIdent() + "functionlog.GP.csv").c_str());

            if( !excel.is_open() ) std::cerr << "\n!functionlog.safe.cvs file access denied" << std::endl;
            gp.breakdown(1000,excel,betToCall,maxShowdown);
            //g.breakdownE(40,excel);
            excel.close();


            excel.open((ViewPlayer().GetIdent() + "functionlog.AP.csv").c_str());
            if( !excel.is_open() ) std::cerr << "\n!functionlog.safe.cvs file access denied" << std::endl;
            ap.breakdown(1000,excel,betToCall,maxShowdown);
            //g.breakdownE(40,excel);
            excel.close();

            excel.open((ViewPlayer().GetIdent() + "functionlog.safe.csv").c_str());
            if( !excel.is_open() ) std::cerr << "\n!functionlog.safe.cvs file access denied" << std::endl;
            choicegain.breakdown(1000,excel,betToCall,maxShowdown);
            //g.breakdownE(40,excel);
            excel.close();
        }
        #endif




    const float64 choicePoint = choicegain.FindBestBet();
    const float64 choiceFold = choicegain.FindZero(choicePoint,myMoney);

    //const float64 callGain = gainmean.f(betToCall); ///Using most accurate gain see if it is worth folding
    const float64 callGain = choicegain.f(betToCall);


        #ifdef LOGPOSITION

            GainModel gainmean(statmean,&myExpectedCall);
            float64 goalPoint = gainmean.FindBestBet();
            float64 goalFold = gainmean.FindZero(goalPoint,myMoney);
/*
            GainModel gainworse(statworse,&myExpectedCall);
            float64 leastPoint = gainworse.FindBestBet();
            float64 leastFold = gainworse.FindZero(leastPoint,myMoney);
*/
            float64 gpPoint = gp.FindBestBet();
            float64 gpFold = gp.FindZero(gpPoint,myMoney);

            HandPlus convertOutput;
            convertOutput.SetUnique(ViewHand());
            convertOutput.DisplayHand(logFile);

            logFile << "Bet to call " << betToCall << " (from " << myBet << ")" << endl;
            logFile << "(Mean) " << statmean.pct * 100 << "%"  << std::endl;
            logFile << "(Worst) " << statworse.pct * 100 << "%"  << std::endl;
            logFile << "Goal bet " << goalPoint << endl;
            logFile << "Fold bet " << goalFold << endl;
            logFile << "GP target " << gpPoint << endl;
            logFile << "GP fold bet " << gpFold << endl;
            //logFile << "Safe target " << leastPoint << endl;
            //logFile << "Safe fold bet " << leastFold << endl;
            logFile << "switch(" << distrScale << ")" << endl;
            logFile << "scale(" << choiceScale << ")" << endl;
            logFile << "Choice Optimal " << choicePoint << endl;
            logFile << "Choice Fold " << choiceFold << endl;
            logFile << "f("<< betToCall <<")=" << callGain << endl;

            if( goalPoint < betToCall )
            {
                cout << "Failed assertion" << endl;

                exit(1);
            }

        #endif


    if( choicePoint == choiceFold && betToCall == choiceFold && callGain <= 0 )
    {///The highest point was the point closest to zero, and the least you can bet if you call--still worse than folding
        logFile << "CHECK/FOLD" << endl;
        return 0;
    }


    if( betToCall < choicePoint )
	{
	    logFile << "RAISETO " << choicePoint << endl << endl;
		return choicePoint;
	}//else
    {
            #ifdef DEBUGASSERT
                if( (betToCall > choiceFold) )
                {
                    cout << "ALL choiceXXXX should be AT LEAST betToCall as defined!" << endl;
                    exit(1);
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
