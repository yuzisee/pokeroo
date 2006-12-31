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


const bool CorePositionalStrategy::lkupLogMean[BGAMBLE_MAX] = {false,true,false,false,true,false,true,true,false};
const bool CorePositionalStrategy::lkupLogRanking[BGAMBLE_MAX] = {true,false,false,true,false,true,false,true,false};
const bool CorePositionalStrategy::lkupLogWorse[BGAMBLE_MAX] = {false,false,true,false,false,false,true,false,false};
const bool CorePositionalStrategy::lkupLogHybrid[BGAMBLE_MAX] = {false,false,false,false,false,false,false,true,true};

// &rankGeom, &meanGeom, &worstAlgb, &rankAlgb, &meanAlgb, &rankGeomPC, &meanGeomPC, &hybridGeom

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
    #ifdef LOGPOSITION
        if( !(logFile.is_open()) )
        {
            logFile.open((ViewPlayer().GetIdent() + ".Positional.log").c_str());
        }
    #endif
    #ifdef LOGPOSITION
        logFile << endl;
        HandPlus convertOutput;
        if( !(h == Hand::EMPTY_HAND) )
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
    #endif


    DistrShape w_wl(0);


    CommunityPlus onlyCommunity;
    onlyCommunity.SetUnique(h);

    CommunityPlus withCommunity;
    withCommunity.SetUnique(ViewHand());
    withCommunity.AppendUnique(onlyCommunity);


    StatsManager::QueryDefense(foldcumu,withCommunity,onlyCommunity,cardsInCommunity);
    ViewTable().CachedQueryOffense(callcumu,withCommunity);
    //StatsManager::QueryOffense(callcumu,withCommunity,onlyCommunity,cardsInCommunity );
    StatsManager::Query(0,&detailPCT,&w_wl,withCommunity,onlyCommunity,cardsInCommunity);
    statmean = GainModel::ComposeBreakdown(detailPCT.mean,w_wl.mean);
    statworse = foldcumu.strongestOpponent(); //GainModel::ComposeBreakdown(detailPCT.worst,w_wl.worst);

    #ifdef LOGPOSITION
    logFile << "*" << endl;
    #endif




    //const float64 ranking3 = callcumu.pctWillCall(statmean.loss); //wins+splits
    //const float64 ranking = callcumu.pctWillCall(1-statmean.wins); //wins
    const float64 ranking3 = callcumu.pctWillCall_tiefactor(1 - statmean.pct, 1); //wins+splits
    const float64 ranking = callcumu.pctWillCall_tiefactor(1 - statmean.pct, 0); //wins

    //std::cout << "=" << ranking << "..." << ranking2 << "..." << ranking3 << std::endl;
    //std::cout << "=" << (ranking3 - ranking) << "\tsplits " << statmean.splits << std::endl;

    statranking.wins = ranking;
    statranking.splits = ranking3 - ranking;
    statranking.loss = 1 - ranking3;
    statranking.genPCT();

    hybridMagnified.wins = sqrt(statmean.wins*statranking.wins);
    hybridMagnified.splits = sqrt(statmean.splits*statranking.splits);
    hybridMagnified.loss = sqrt(statmean.loss*statranking.loss);
    hybridMagnified.genPCT();
    const float64 adjust = hybridMagnified.wins + hybridMagnified.splits + hybridMagnified.loss;
    hybridMagnified = hybridMagnified * ( 1.0 / adjust );
    hybridMagnified.repeated = 0; ///.repeated WILL otherwise ACCUMULATE!

#ifdef LOGPOSITION
    logFile << "(Mean) " << statmean.pct * 100 << "%"  << std::endl;
    logFile << "(Mean.wins) " << statmean.wins * 100 << "%"  << std::endl;
    logFile << "(Mean.splits) " << statmean.splits * 100 << "%"  << std::endl;
    logFile << "(Mean.loss) " << statmean.loss * 100 << "%"  << std::endl;
    logFile << "(Worst) " << statworse.pct * 100 << "%"  << std::endl;
    logFile << "(Worst.wins) " << statworse.wins * 100 << "%"  << std::endl;
    logFile << "(Worst.splits) " << statworse.splits * 100 << "%"  << std::endl;
    logFile << "(Worst.loss) " << statworse.loss * 100 << "%"  << std::endl;
    logFile << "(Outright) " << statranking.pct * 100 << "%"  << std::endl;
    logFile << "(Outright.wins) " << statranking.wins * 100 << "%"  << std::endl;
    logFile << "(Outright.splits) " << statranking.splits * 100 << "%"  << std::endl;
    logFile << "(Outright.loss) " << statranking.loss * 100 << "%"  << std::endl;
    logFile << "(Hybrid) " << hybridMagnified.pct * 100 << "%"  << std::endl;
    logFile << "(Hybrid.wins) " << hybridMagnified.wins * 100 << "%"  << std::endl;
    logFile << "(Hybrid.splits) " << hybridMagnified.splits * 100 << "%"  << std::endl;
    logFile << "(Hybrid.loss) " << hybridMagnified.loss * 100 << "%"  << std::endl;
#endif

}

void PositionalStrategy::setupPosition()
{
        #ifdef LOGPOSITION
            logFile << endl;
            HandPlus convertOutput;
            convertOutput.SetUnique(ViewHand());
            convertOutput.DisplayHand(logFile);
        #endif


    myMoney = ViewPlayer().GetMoney();


    highBet = ViewTable().GetBetToCall();
    betToCall = highBet;

    myBet = ViewPlayer().GetBetSize();



    if( myMoney < betToCall ) betToCall = myMoney;
    maxShowdown = ViewTable().GetMaxShowdown();
    if( maxShowdown > myMoney ) maxShowdown = myMoney;
    //float64 choiceScale = (betToCall - myBet)/(maxShowdown - myBet);


        #ifdef LOGPOSITION
            logFile << "Bet to call " << betToCall << " (from " << myBet << ")" << endl;
        #endif

    const float64 raiseBattle = betToCall ? betToCall : ViewTable().GetChipDenom();
    ExactCallD myExpectedCall(myPositionIndex, &(ViewTable()), &callcumu);
    expectedVS = ( myExpectedCall.exf(raiseBattle) - ViewTable().GetPotSize() + ViewTable().GetUnbetBlindsTotal() + ViewTable().GetRoundBetsTotal() ) /raiseBattle;
}

float64 PositionalStrategy::solveGainModel(HoldemFunctionModel* targetModel)
{

        #ifdef DEBUGSPECIFIC
        if( ViewTable().handnum == DEBUGSPECIFIC )
        {
            std::ofstream excel( (ViewPlayer().GetIdent() + "functionlog.csv").c_str() );
            if( !excel.is_open() ) std::cerr << "\n!functionlog.cvs file access denied" << std::endl;
            targetModel->breakdown(1000,excel,betToCall,maxShowdown);
            //myExpectedCall.breakdown(0.005,excel);

            excel.close();
        }
        #endif



// #############################################################################
/// MATHEMATIC SOLVING BEGINS HERE
// #############################################################################

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


// #############################################################################
/// MATHEMATIC SOLVING ENDS HERE
// #############################################################################

        #ifdef LOGPOSITION


            //logFile << "selected risk  " << (choicePoint - myBet)/(maxShowdown - myBet) << endl;

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
	    /*if( choicePoint - betToCall <= ViewTable().GetMinRaise() ) */
            logFile << "*MinRaise " << (ViewTable().GetMinRaise() + betToCall) << endl;
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


float64 ImproveStrategy::MakeBet()
{
    setupPosition();

    //bGamble reduces the scaling
    const float64 improveMod = detailPCT.improve; //Generally preflop is negative here, so you probably don't want to accentuate that
    const float64 improvePure = (improveMod+1)/2;
    //const float64 improveDev = detailPCT.stdDev * (1-improvePure) + detailPCT.avgDev * improvePure;

    float64 distrScale = improvePure;
    //float64 distrScale = 0.5 ;
    //float64 distrScale = myMoney / ViewTable().GetAllChips() ;

    if( bGamble % 2 == 1 )
    {///Protecting against the drop
        distrScale = 1 - distrScale;
    }
    ///Banking on the upswing

    if( distrScale > 1 ) distrScale = 1;
    if( distrScale < 0 ) distrScale = 0;

    CallCumulationD &choicecumu = callcumu;

    ExactCallD myExpectedCall(myPositionIndex, &(ViewTable()), &choicecumu);
    ExactCallD myLimitCall(myPositionIndex, &(ViewTable()), &choicecumu);

    myLimitCall.callingPlayers(  expectedVS  );
    if( expectedVS < 0 ) //You have no money
    {
        myLimitCall.callingPlayers( myExpectedCall.betFraction(ViewTable().GetChipDenom()) ) ;
    }

    GainModel rankGeom(statranking,&myExpectedCall);
    GainModelNoRisk worstAlgb(statworse,&myLimitCall);


    SlidingPairFunction gp(&worstAlgb,&rankGeom,distrScale,&myExpectedCall);

    #ifdef LOGPOSITION
        logFile << "offense/defense =" << distrScale << endl;
    #endif

    const float64 bestBet = solveGainModel(&gp);
    return bestBet;

}

float64 DeterredGainStrategy::MakeBet()
{
    setupPosition();

    //const float64 certainty = betToCall / maxShowdown;
    const float64 uncertainty = fabs( statranking.pct - statmean.pct );
    const float64 futureFold = 1 - sqrt(  detailPCT.stdDev*detailPCT.stdDev + uncertainty*uncertainty  );


    CallCumulationD &choicecumu = callcumu;

    ExactCallD myExpectedCall(myPositionIndex, &(ViewTable()), &choicecumu);
    ExactCallD myDeterredCall(myPositionIndex, &(ViewTable()), &choicecumu);
    myDeterredCall.SetImpliedFactor(futureFold);


    GainModel hybridgain(statranking,&myExpectedCall);
    GainModel hybridgainDeterred(statranking,&myDeterredCall);

    AutoScalingFunction choicemodel(&hybridgainDeterred,&hybridgain,0.0,maxShowdown,&myExpectedCall);

#ifdef LOGPOSITION
    logFile << "uncertainty      " << uncertainty << endl;
    logFile << "detailPCT.stdDev " << detailPCT.stdDev << endl;
    //logFile << "V Factor         " << volatilityFactor << endl;
    //logFile << "BetToCall PCT    " << certainty << endl;
    logFile << "impliedFactor... " << futureFold << endl;
#endif

    const float64 bestBet = solveGainModel(&hybridgain);

    return bestBet;


}



float64 CorePositionalStrategy::MakeBet()
{
    setupPosition();


    ///VARIABLE: the slider can move due to avgDev too, maybe....
    CallCumulationD &choicecumu = callcumu;
    //SlidingPairCallCumulationD choicecumu( &callcumu, &foldcumu, timing[DT]/2 );
    //SlidingPairCallCumulationD choicecumu( &callcumu, &foldcumu, detailPct.avgDev*2 );



    ExactCallD myExpectedCall(myPositionIndex, &(ViewTable()), &choicecumu);
    ExactCallD myLimitCall(myPositionIndex, &(ViewTable()), &choicecumu);
    const float64 committed = ViewPlayer().GetContribution() + myBet;
    ExactCallD myCommittalCall(myPositionIndex, &(ViewTable()), &choicecumu, committed );

    float64 raiseBattle = ViewTable().GetChipDenom();
    if( betToCall > raiseBattle )
    {
        raiseBattle = betToCall;
    }
    const float64 expectedVS = ( myExpectedCall.exf(raiseBattle) - ViewTable().GetPotSize() + ViewTable().GetUnbetBlindsTotal() + ViewTable().GetRoundBetsTotal() ) /raiseBattle;
    if( expectedVS < 0 ) //You have no money
    {
        myLimitCall.callingPlayers( myExpectedCall.betFraction(ViewTable().GetChipDenom()) ) ;
    }



    myLimitCall.callingPlayers(  expectedVS  );


    GainModel rankGeom(statranking,&myExpectedCall);
    GainModel hybridGeom(hybridMagnified,&myExpectedCall);
    GainModelNoRisk hybridAlgb(hybridMagnified,&myExpectedCall);
    //GainModelNoRisk choicegain_rnr(statranking,&myExpectedCall);


    GainModel meanGeom(statmean,&myExpectedCall);


    GainModelNoRisk worstAlgb(statworse,&myLimitCall);
    GainModelNoRisk rankAlgb(statranking,&myExpectedCall);
    GainModelNoRisk meanAlgb(statmean,&myExpectedCall);
    GainModel rankGeomPC(statranking,&myCommittalCall);
    GainModel meanGeomPC(statmean,&myCommittalCall);


    #ifdef DEBUGASSERT
        if( bGamble >= BGAMBLE_MAX )
        {
                std::cout << "Incorrect bGAMBLE" << endl;
                exit(1);
        }
    #endif

    HoldemFunctionModel* (lookup[BGAMBLE_MAX]) = { &rankGeom, &meanGeom, &worstAlgb, &rankAlgb, &meanAlgb, &rankGeomPC, &meanGeomPC, &hybridGeom, &hybridAlgb };

    #ifdef LOGPOSITION
        const float64 improveMod = detailPCT.improve * 2;
        const float64 improvePure = (improveMod+1)/2;

        logFile << "raw improval " << improvePure << endl;
        logFile << "doublePlay(" << expectedVS << ")" << endl;
    #endif

    #ifdef LOGPOSITION
        if( bGamble == 0 || bGamble == 1 || bGamble == 5 || bGamble == 6 )
        {
            const int8 otherGamble = (bGamble + 5) % 10;
            logFile << "Other {" << (int)(otherGamble) << "}" << endl;
            solveGainModel(lookup[otherGamble]);
            logFile << "Main {" << (int)(bGamble) << "}" << endl;
        }
    #endif
    const float64 bestBet = solveGainModel(lookup[bGamble]);


    return bestBet;


}
