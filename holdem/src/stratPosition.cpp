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
#include "functionmodel.h"
#include "stratPosition.h"

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
    roundNumber[0] = cardsInCommunity*cardsInCommunity; /// Round number is [0,1,2,3]=[Pre-Flop,Right After Flop,Between Turn and River,Final]<-[0,3,4,5]
    roundNumber[0] /= 8;

    ///2/7 is very tight preflop.
    /// Round number is [0,1,2,3]=[Pre-Flop,Right After Flop,Between Turn and River,Final]<-[2,5,6,7]
    roundNumber[1] = cardsInCommunity + 2;

    /// Round number is [0,1,2,3]=[Pre-Flop,Right After Flop,Between Turn and River,Final]<-[0,2,5,6]
    roundNumber[2] = roundNumber[0]*2;
    roundNumber[2] += roundNumber[2]/4 - roundNumber[2]/6;

    DistrShape w_wl(0);


    CommunityPlus onlyCommunity;
    onlyCommunity.SetUnique(h);

    CommunityPlus withCommunity;
    withCommunity.SetUnique(ViewHand());
    withCommunity.AppendUnique(onlyCommunity);


    //StatsManager::QueryDefense(foldcumu,withCommunity,onlyCommunity,cardsInCommunity);
    ViewTable().CachedQueryOffense(callcumu,withCommunity);
    //StatsManager::QueryOffense(callcumu,withCommunity,onlyCommunity,cardsInCommunity );
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
/*
    const int8 DT = 2;
    const float64 timing[3] =   {
                                    static_cast<float64>(roundNumber[0])/5.0
                                    , static_cast<float64>(roundNumber[1])/7.0
                                    , static_cast<float64>(roundNumber[2])/6.0
                                };
*/
    const float64 myMoney = ViewPlayer().GetMoney();


    const float64 highBet = ViewTable().GetBetToCall();
    float64 betToCall = highBet;

    const float64 myBet = ViewPlayer().GetBetSize();



    if( myMoney < betToCall ) betToCall = myMoney;
    float64 maxShowdown = ViewTable().GetMaxShowdown();
    if( maxShowdown > myMoney ) maxShowdown = myMoney;
    //float64 choiceScale = (betToCall - myBet)/(maxShowdown - myBet);


    float64 impliedFactor;
    const float64 improvePure = (detailPCT.improve+1)/2;
    const float64 improveDev = detailPCT.stdDev * (1-improvePure) + detailPCT.avgDev * improvePure;

    if( detailPCT.n == 1 )
    {
        impliedFactor = 1;
        //improvePure = 0.5;
    }else
    {
        if( bGamble / 2 == 0 ) // 0,1
        {
            impliedFactor = 1;
        }
        else if( bGamble / 2 == 1 ) // 2,3
        {
            impliedFactor = 1 + improveDev*2*improvePure;
        }else if( bGamble / 2 == 2 )// 4,5
        {
            impliedFactor = 1 + improveDev*2*(1-improvePure);
        }
    }



    float64 distrScale = improvePure;
    //float64 distrScale = 0.5 ;
    //float64 distrScale = myMoney / ViewTable().GetAllChips() ;


    if( bGamble % 2 == 0 ) // 0,2,4
    {///Protecting against the drop
        distrScale = 1 - distrScale;
    }// else 1,3,5
    ///Banking on the upswing

    if( distrScale > 1 ) distrScale = 1;
    if( distrScale < 0 ) distrScale = 0;



    //const float64 ranking3 = callcumu.pctWillCall(statmean.loss); //wins+splits
    //const float64 ranking = callcumu.pctWillCall(1-statmean.wins); //wins
    const float64 ranking3 = callcumu.pctWillCall_tiefactor(1 - statmean.pct, 1); //wins+splits
    const float64 ranking = callcumu.pctWillCall_tiefactor(1 - statmean.pct, 0); //wins

    //std::cout << "=" << ranking << "..." << ranking2 << "..." << ranking3 << std::endl;
    //std::cout << "=" << (ranking3 - ranking) << "\tsplits " << statmean.splits << std::endl;

    StatResult statranking;
    statranking.wins = ranking;
    statranking.splits = ranking3 - ranking;
    statranking.loss = 1 - ranking3;
    statranking.genPCT();


    ///VARIABLE: the slider can move due to avgDev too, maybe....
    CallCumulationD &choicecumu = callcumu;
    //SlidingPairCallCumulationD choicecumu( &callcumu, &foldcumu, timing[DT]/2 );
    //SlidingPairCallCumulationD choicecumu( &callcumu, &foldcumu, detailPct.avgDev*2 );



    ExactCallD myExpectedCall(myPositionIndex, &(ViewTable()), &choicecumu);
    myExpectedCall.SetImpliedFactor(impliedFactor);






    GainModel choicegain_rev(statranking,&myExpectedCall);
    //GainModelNoRisk choicegain_rnr(statranking,&myExpectedCall);


    GainModel choicegain_base(statmean,&myExpectedCall);
    GainModelNoRisk choicegain_nr(statworse,&myExpectedCall);


	SlidingPairFunction gp(&choicegain_base,&choicegain_rev,distrScale,&myExpectedCall);

	//SlidingPairFunction ap(&choicegain_nr,&choicegain_rnr,distrScale,&myExpectedCall);

    //const float64 MAX_UPTO = 1.0/2.0;
    const float64 MAX_UPTO = 1;

    AutoScalingFunction choicegain(&gp,&choicegain_base,myBet,maxShowdown,MAX_UPTO,&myExpectedCall);
    SlidingPairFunction choicegain_upto(&gp,&choicegain_base,MAX_UPTO,&myExpectedCall);

    HoldemFunctionModel* targetModel;
    if( maxShowdown == myBet || maxShowdown < betToCall )
    {
        ///TODO: THIS IS THE ONE STILL NEEDED
        targetModel = &choicegain_upto;
    }else
    {
        targetModel = &choicegain;
    }





    /*

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


/*
            excel.open((ViewPlayer().GetIdent() + "functionlog.anti.csv").c_str());
            if( !excel.is_open() ) std::cerr << "\n!functionlog.safe.cvs file access denied" << std::endl;
            choicegain_rnr.breakdown(1000,excel,betToCall,maxShowdown);
            //g.breakdownE(40,excel);
            excel.close();
*/
          excel.open((ViewPlayer().GetIdent() + "functionlog.GP.csv").c_str());

            if( !excel.is_open() ) std::cerr << "\n!functionlog.safe.cvs file access denied" << std::endl;
            gp.breakdown(1000,excel,betToCall,maxShowdown);
            //g.breakdownE(40,excel);
            excel.close();

/*
            excel.open((ViewPlayer().GetIdent() + "functionlog.AP.csv").c_str());
            if( !excel.is_open() ) std::cerr << "\n!functionlog.safe.cvs file access denied" << std::endl;
            ap.breakdown(1000,excel,betToCall,maxShowdown);
            //g.breakdownE(40,excel);
            excel.close();
*/
            excel.open((ViewPlayer().GetIdent() + "functionlog.safe.csv").c_str());
            if( !excel.is_open() ) std::cerr << "\n!functionlog.safe.cvs file access denied" << std::endl;
            choicegain.breakdown(1000,excel,betToCall,maxShowdown);
            //g.breakdownE(40,excel);
            excel.close();
        }
        #endif

// #############################################################################
// MATHEMATIC SOLVING HAPPENS HERE


    float64 choicePoint = targetModel->FindBestBet();

    
    
    if( choicePoint < betToCall )
    {///It's probably really close though
            #ifdef LOGPOSITION
                    logFile << "Choice Optimal < Bet to call" << endl;
            #endif
            #ifdef DEBUGASSERT
                    if ( choicePoint + ViewTable().GetChipDenom()/2 < betToCall )
                    {
                        std::cerr << "Failed assertion" << endl;
                        exit(1);
                    }
            #endif
        
        choicePoint = betToCall;
        
    }
    
    
    const float64 choiceFold = targetModel->FindFoldBet(choicePoint);

    //const float64 callGain = gainmean.f(betToCall); ///Using most accurate gain see if it is worth folding
    const float64 callGain = targetModel->f(betToCall);

    
#ifdef DEBUGASSERT
    const float64 raiseGain = targetModel->f(choicePoint);
#endif
    
    

// MATHEMATIC SOLVING ENDS HERE
// #############################################################################

        #ifdef LOGPOSITION

            //GainModel gainmean(statmean,&myExpectedCall);

            //float64 goalPoint = gainmean.FindBestBet();
            //float64 goalFold = gainmean.FindZero(goalPoint,myMoney);
/*
            GainModel gainworse(statworse,&myExpectedCall);
            float64 leastPoint = gainworse.FindBestBet();
            float64 leastFold = gainworse.FindZero(leastPoint,myMoney);
*/

            //float64 gpPoint = gp.FindBestBet();
            //float64 gpFold = gp.FindZero(gpPoint,myMoney);

            HandPlus convertOutput;
            convertOutput.SetUnique(ViewHand());
            convertOutput.DisplayHand(logFile);

            logFile << "Bet to call " << betToCall << " (from " << myBet << ")" << endl;
            logFile << "(Mean) " << statmean.pct * 100 << "%"  << std::endl;
            logFile << "(Mean.wins) " << statmean.wins * 100 << "%"  << std::endl;
            logFile << "(Mean.splits) " << statmean.splits * 100 << "%"  << std::endl;
            logFile << "(Mean.loss) " << statmean.loss * 100 << "%"  << std::endl;
            logFile << "(Worst) " << statworse.pct * 100 << "%"  << std::endl;

            logFile << "(Outright) " << statranking.pct * 100 << "%"  << std::endl;
            logFile << "(Outright.wins) " << statranking.wins * 100 << "%"  << std::endl;
            logFile << "(Outright.splits) " << statranking.splits * 100 << "%"  << std::endl;
            logFile << "(Outright.loss) " << statranking.loss * 100 << "%"  << std::endl;


            //logFile << "Goal bet " << goalPoint << endl;
            //logFile << "Fold bet " << goalFold << endl;
            //logFile << "GP target " << gpPoint << endl;
            //logFile << "GP fold bet " << gpFold << endl;
            //logFile << "Safe target " << leastPoint << endl;
            //logFile << "Safe fold bet " << leastFold << endl;
            logFile << "offense/defense(" << distrScale << ")" << endl;
            logFile << "selected risk  " << (choicePoint - myBet)/(maxShowdown - myBet) << endl;
            //logFile << "timing[" << (int)DT << "](" << timing[DT] << ")" << endl;
            logFile << "impliedFactor " << impliedFactor << endl;
            /*if( bGamble / 4 == 1 ){
                float64 alreadyCalled = 0;
                if( betToCall != 0 ) alreadyCalled = (ViewTable().GetRoundBetsTotal() - ViewPlayer().GetBetSize())/highBet;
                logFile << "expected versus (" << myExpectedCall.callingPlayers() << ") from " << alreadyCalled << endl;
            }*/
            logFile << "Choice Optimal " << choicePoint << endl;
            logFile << "Choice Fold " << choiceFold << endl;
            logFile << "f("<< betToCall <<")=" << callGain << endl;


        #endif

                 

//############################
///  DECISION TIME
//############################
    
    //Remember, due to rounding and chipDenom, (betToCall < choiceFold) is possible
    if( choicePoint >= choiceFold && betToCall >= choiceFold && callGain <= 0 )
    {///The highest point was the point closest to zero, and the least you can bet if you call--still worse than folding
        #ifdef LOGPOSITION
        logFile << "CHECK/FOLD" << endl;
        #endif
        return myBet;
    }
    ///Else you play wherever choicePoint is.


    #ifdef DEBUGASSERT
        if( raiseGain < 0 )
        {
            #ifdef LOGPOSITION
            logFile << "raiseGain: f("<< choicePoint <<")=" << raiseGain << endl;
            logFile << "Redundant CHECK/FOLD detect required" << endl;
            #endif
            return myBet;
        }
    #endif

    if( betToCall < choicePoint )
	{
	    #ifdef LOGPOSITION
	    if( choicePoint - betToCall > ViewTable().GetMinRaise() ) logFile << "*MinRaise " << (ViewTable().GetMinRaise() + betToCall) << endl;
	    logFile << "RAISETO " << choicePoint << endl << endl;
	    #endif
		return choicePoint;
	}

/*
    if( betToCall > choiceFold)
    {///Due to rounding
        #ifdef DEBUGASSERT
        if (betToCall - ViewTable().GetChipDenom()/2 > choiceFold)
        {
            logFile << "Choice Fold < Bet to call" << endl;
            std::cerr << "ALL choiceXXXX should be AT LEAST betToCall as defined!" << endl;
            exit(1);
        }
        #endif
    }
*/
    #ifdef LOGPOSITION
    logFile << "CALL " << choicePoint << endl;
    #endif
    return betToCall;


}
