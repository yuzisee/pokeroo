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


/// Summary of contained strategies:
/// 1. ImproveStrategy (This is the old fashioned Geom<-->Algb vs. Rank<-->WorseNextCard)
/// 2. ImproveGainStrategy (This is Norm/Trap/Ace using empirical data)
/// 3. DeterredGainStrategy (ComBot: volatility/certainty/uncertainty avoids committals)
/// 4. HybridScalingStrategy (SpaceBot, experimental stuff)

/// *RANK variants replace Geom<->Linear(NoRisk) AutoScaling with a Geom ONLY.

const bool CorePositionalStrategy::lkupLogMean[BGAMBLE_MAX] = {false,true,false,false,true,false,true,true,false ,false,true,false,false,true,true,false};
const bool CorePositionalStrategy::lkupLogRanking[BGAMBLE_MAX] = {true,false,false,true,false,true,false,true,false ,true,false,false,true,false,true,false};
const bool CorePositionalStrategy::lkupLogWorse[BGAMBLE_MAX] = {false,false,true,false,false,false,true,false,false ,false,false,true,false,false,false,false};
const bool CorePositionalStrategy::lkupLogHybrid[BGAMBLE_MAX] = {false,false,false,false,false,false,false,true,true ,false,false,false,false,false,true,true};

// &rankGeom, &meanGeom, &worstAlgb, &rankAlgb, &meanAlgb, &rankGeomPC, &meanGeomPC, &hybridGeom

PositionalStrategy::~PositionalStrategy()
{
    #ifdef LOGPOSITION
        ReleaseLogFile();
    #endif
}

void PositionalStrategy::HardOpenLogFile()
{
    #ifdef LOGPOSITION
        if( !(logFile.is_open()) )
        {
            logFile.open((ViewPlayer().GetIdent() + ".Thoughts.txt").c_str()
            #ifdef WINRELEASE
            , std::ios::app
            #endif
            );
        }
    #endif

}


void PositionalStrategy::SoftOpenLogFile()
{
    #ifdef LOGPOSITION
        if( !(logFile.is_open()) )
        {
            logFile.open((ViewPlayer().GetIdent() + ".Thoughts.txt").c_str()
            , std::ios::app
            );
        }
    #endif

}

void PositionalStrategy::ReleaseLogFile()
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
        HardOpenLogFile();
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

    //if(bLogWorse)
    {
        StatsManager::QueryDefense(foldcumu,withCommunity,onlyCommunity,cardsInCommunity);
    }
    //else{

    //}
    ViewTable().CachedQueryOffense(callcumu,withCommunity);
    //StatsManager::QueryOffense(callcumu,withCommunity,onlyCommunity,cardsInCommunity );
    StatsManager::Query(0,&detailPCT,&w_wl,withCommunity,onlyCommunity,cardsInCommunity);
    statmean = GainModel::ComposeBreakdown(detailPCT.mean,w_wl.mean);
    statworse = foldcumu.strongestOpponent(); //GainModel::ComposeBreakdown(detailPCT.worst,w_wl.worst);
    //CallStats is foldcumu

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
    if(bLogMean)
    {
        logFile << "(M) " << statmean.pct * 100 << "%"  << std::endl;
        logFile << "(M.w) " << statmean.wins * 100 << "%"  << std::endl;
        logFile << "(M.s) " << statmean.splits * 100 << "%"  << std::endl;
        logFile << "(M.l) " << statmean.loss * 100 << "%"  << std::endl;
    }
    if(bLogWorse)
    {
        logFile << "(Worst) " << statworse.pct * 100 << "%"  << std::endl;
        logFile << "(W.w) " << statworse.wins * 100 << "%"  << std::endl;
        logFile << "(W.s) " << statworse.splits * 100 << "%"  << std::endl;
        logFile << "(W.l) " << statworse.loss * 100 << "%"  << std::endl;
    }
    if(bLogRanking)
    {
        logFile << "(Outright) " << statranking.pct * 100 << "%"  << std::endl;
        logFile << "(O.w) " << statranking.wins * 100 << "%"  << std::endl;
        logFile << "(O.s) " << statranking.splits * 100 << "%"  << std::endl;
        logFile << "(O.l) " << statranking.loss * 100 << "%"  << std::endl;
    }
    if(bLogHybrid)
    {
		if( !bLogMean ) logFile << "(Mean) " << statmean.pct * 100 << "%"  << std::endl;
		if( !bLogRanking ) logFile << "(Outright) " << statranking.pct * 100 << "%"  << std::endl;
        logFile << "(Hybrid) " << hybridMagnified.pct * 100 << "%"  << std::endl;
        logFile << "(H.w) " << hybridMagnified.wins * 100 << "%"  << std::endl;
        logFile << "(H.s) " << hybridMagnified.splits * 100 << "%"  << std::endl;
        logFile << "(H.l) " << hybridMagnified.loss * 100 << "%"  << std::endl;
    }
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
            logFile << "Bet to call " << betToCall << " (from " << myBet << ") at " << ViewTable().GetPotSize() << " pot" << endl;
        #endif

    const float64 raiseBattle = betToCall ? betToCall : ViewTable().GetChipDenom();
#ifdef ANTI_PRESSURE_FOLDGAIN
	ExactCallD myExpectedCall(myPositionIndex, &(ViewTable()), statranking.pct, &callcumu);

#else
	ExactCallD myExpectedCall(myPositionIndex, &(ViewTable()), &callcumu);
#endif
    expectedVS = ( myExpectedCall.exf(raiseBattle) - ViewTable().GetPotSize() + ViewTable().GetUnbetBlindsTotal() + ViewTable().GetRoundBetsTotal() ) /raiseBattle;
    if( expectedVS <= 0 ) //You have no money
    {
        expectedVS = ( myExpectedCall.betFraction(ViewTable().GetChipDenom()) ) ;
    }
}

float64 PositionalStrategy::solveGainModel(HoldemFunctionModel* targetModel)
{

        #ifdef DEBUGSPECIFIC
        if( ViewTable().handnum == DEBUGSPECIFIC )
        {
            std::ofstream excel( (ViewPlayer().GetIdent() + ".functionlog.csv").c_str() );
            if( !excel.is_open() ) std::cerr << "\n!functionlog.cvs file access denied" << std::endl;
            targetModel->breakdown(1000,excel,betToCall,maxShowdown);
            //myExpectedCall.breakdown(0.005,excel);

            excel.close();
        }
        #endif


	if( maxShowdown <= 0 )
	{	//This probably shouldn't happen unless two people are left in the hand and one of them has zero.
		//At the time there was a bug in arenaEvents where it would still let the person with money, bet.
		return maxShowdown;
	}

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
    const float64 callGain = (betToCall < myMoney) ? (targetModel->f(betToCall)) : (targetModel->f(myMoney))
    ;


#ifdef DEBUGASSERT
    const float64 raiseGain = targetModel->f(choicePoint)
    ;
#endif


// #############################################################################
/// MATHEMATIC SOLVING ENDS HERE
// #############################################################################

        #ifdef LOGPOSITION


            //logFile << "selected risk  " << (choicePoint - myBet)/(maxShowdown - myBet) << endl;

            logFile << "Choice Optimal " << choicePoint << endl;
            logFile << "Choice Fold " << choiceFold << endl;
            logFile << "FoldGain()=" << (targetModel->GetFoldGain()) << " ." << endl;
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



    if( raiseGain < 0 )
    {
        #ifdef LOGPOSITION
        logFile << "raiseGain: f("<< choicePoint <<")=" << raiseGain << endl;
		logFile << "CHECK/FOLD" << endl;
        #endif
        return myBet;
    }


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


float64 ImproveGainStrategy::MakeBet()
{
	setupPosition();

	if( maxShowdown <= 0 ) return 0;

    const float64 improveMod = detailPCT.improve; //Generally preflop is negative here, so you probably don't want to accentuate that
    const float64 improvePure= (improveMod+1)/2;

    //const float64 targetImproveBy = detailPCT.avgDev / 2 / improvePure;
    const float64 targetWorsenBy = detailPCT.avgDev / 2 / (1 - improvePure);
    const float64 impliedOddsGain = (statmean.pct + detailPCT.avgDev / 2) / statmean.pct;
    //const float64 oppInsuranceSmallBet = (1 - statmean.pct + targetWorsenBy) / (1 - statmean.pct);
    const float64 oppInsuranceBigBet = (improveMod>0)?(improveMod/2):0;

/*
    const float64 minWin = pow(statworse.pct,expectedVS);
    const float64 improveDev = detailPCT.stdDev * (1-improvePure) + detailPCT.avgDev * improvePure;


    float64 distrScale = improveMod+minWin+1 ;
    if( bGamble >= 2 )
    {
        distrScale = (-improveMod/2)+minWin+1 ;
    }
*/


    CallCumulationD &choicecumu = callcumu;
    CallCumulationD &raisecumu = foldcumu;


#ifdef ANTI_PRESSURE_FOLDGAIN
    ExactCallBluffD myDeterredCall(myPositionIndex, &(ViewTable()), statranking.pct, &choicecumu, &raisecumu);
    ExactCallBluffD myDeterredCall_left(myPositionIndex, &(ViewTable()), statranking.pct, &choicecumu, &raisecumu);
    ExactCallBluffD myDeterredCall_right(myPositionIndex, &(ViewTable()), statranking.pct, &choicecumu, &raisecumu);
#else
    ExactCallBluffD myDeterredCall(myPositionIndex, &(ViewTable()), &choicecumu, &raisecumu);
#endif

    const float64 fullVersus = ViewTable().GetNumberAtTable() - 1;
    const float64 peopleDrawing = (1 - improvePure) * (ViewTable().GetNumberInHand() - 1);
    const float64 newVersus = (fullVersus - peopleDrawing*(1-improvePure)*detailPCT.stdDev);


//bGamble == 2 is ActionBot
//bGamble == 1 is TrapBot

    const float64 actOrReact = betToCall / maxShowdown;

//NormalBot uses this setup.
    StatResult left = hybridMagnified;
    StatResult base_right = statmean;

//TrapBot and ActionBot are based on statranking only
    if( bGamble >= 1 )
    {
        left = statranking;
        left.wins -= detailPCT.avgDev/2;
        left.loss += detailPCT.avgDev/2;
        left.pct -= detailPCT.avgDev/2;


        base_right = statranking;

//Need scaling
        myDeterredCall_right.insuranceDeterrent = oppInsuranceBigBet;

        if( bGamble >= 2 )
        {
            myDeterredCall_left.SetImpliedFactor(impliedOddsGain);

//Need scaling
            myDeterredCall_right.callingPlayers(newVersus);
        }
    }

    StatResult right = (actOrReact > 1) ? base_right: ((base_right * actOrReact) + statworse * (1 - actOrReact));

	GainModel hybridgainDeterred_aggressive(left,&myDeterredCall_left);
	GainModelNoRisk hybridgain_aggressive(right,&myDeterredCall_right);

#ifdef LOGPOSITION
    if( bGamble >= 1 )
    {
        #ifdef LOGPOSITION
//            logFile << minWin <<" ... "<< minWin+2 <<" = impliedfactor " << distrScale << endl;
            logFile << improvePure <<" improvePure " << endl;
            logFile << " Likely Worsen By "<< targetWorsenBy << endl; //TRAPBOT, ACTIONBOT
            if( bGamble >= 2 ) logFile << "impliedOddsGain would be " << impliedOddsGain << endl; //ACTIONBOT
            logFile << "opp Likely to fold " << oppInsuranceBigBet << endl; //TRAPBOT, ACTIONBOT
            if( bGamble >= 2 ) logFile << "Can push expectedVersus from " << fullVersus << " ... " << newVersus << " ... " << (fullVersus - peopleDrawing) << endl; //ACTIONBOT
        #endif
    }
    logFile << "Act or React? React " << (actOrReact * 100) << "% --> pct of " << base_right.pct << " ... " << right.pct << " ... " << statworse.pct << endl;
#endif


	AutoScalingFunction ap(&hybridgainDeterred_aggressive,&hybridgain_aggressive,0.0,maxShowdown,left.pct*base_right.pct*actOrReact - actOrReact + 1,&myDeterredCall_left);
	AutoScalingFunction ap_right(&hybridgainDeterred_aggressive,&hybridgain_aggressive,0.0,maxShowdown,left.pct*base_right.pct*actOrReact - actOrReact + 1,&myDeterredCall_right);

    StateModel choicemodel( &myDeterredCall_left, &ap );
    StateModel choicemodel_right( &myDeterredCall_right, &ap_right );

    AutoScalingFunction rolemodel(&choicemodel,&choicemodel_right,betToCall,maxShowdown,&myDeterredCall_left);

    const float64 bestBet = (bGamble == 0) ? solveGainModel(&choicemodel) : solveGainModel(&rolemodel);

#ifdef LOGPOSITION
    //if( bestBet < betToCall + ViewTable().GetChipDenom() )
    {
        logFile << "\"shuftip\"... " << hybridMagnified.pct*statmean.pct << endl;
        logFile << "Geom("<< bestBet <<")=" << hybridgainDeterred_aggressive.f(bestBet) << endl;
        logFile << "Algb("<< bestBet <<")=" << hybridgain_aggressive.f(bestBet) << endl;

    }


	if( bestBet >= betToCall - ViewTable().GetChipDenom() )
	{
		int32 raiseStep = 0;
        float64 rAmount =  myDeterredCall.RaiseAmount(bestBet,raiseStep);
        while( rAmount < maxShowdown )
        {
            rAmount =  myDeterredCall.RaiseAmount(bestBet,raiseStep);
            logFile << "OppRAISEChance% ... " << myDeterredCall.pRaise(bestBet,raiseStep) << " @ $" << rAmount;
//            logFile << "\tfold " << myDeterredCall_left.pWin(rAmount) << " left -- right " << myDeterredCall_right.pWin(rAmount) << endl;
            ++raiseStep;
        }
	}

        logFile << "Guaranteed $" << myDeterredCall.stagnantPot() << endl;
        logFile << "OppFoldChance% ... " << myDeterredCall_left.pWin(bestBet) << " left -- right " << myDeterredCall_right.pWin(bestBet) << endl;
        if( myDeterredCall.pWin(bestBet) > 0 )
        {
            logFile << "confirm " << choicemodel.f(bestBet) << endl;
        }

#endif

    return bestBet;


}


float64 DeterredGainStrategy::MakeBet()
{
	setupPosition();

	if( maxShowdown <= 0 ) return 0;


    const float64 certainty = betToCall / maxShowdown;
    const float64 uncertainty = fabs( statranking.pct - statmean.pct );
    const float64 timeLeft = sqrt(  detailPCT.stdDev*detailPCT.stdDev + uncertainty*uncertainty  );
    const float64 volatilityFactor = 1 - timeLeft;

	const float64 futureFold = volatilityFactor*(1-certainty) + certainty;



    CallCumulationD &choicecumu = callcumu;
    CallCumulationD &raisecumu = foldcumu;


#ifdef ANTI_PRESSURE_FOLDGAIN
    //ExactCallD myExpectedCall(myPositionIndex, &(ViewTable()), statranking.pct, &choicecumu);
    ExactCallBluffD myDeterredCall(myPositionIndex, &(ViewTable()), statranking.pct, &choicecumu, &raisecumu);
#else
    //ExactCallD myExpectedCall(myPositionIndex, &(ViewTable()), &choicecumu);
    ExactCallBluffD myDeterredCall(myPositionIndex, &(ViewTable()), &choicecumu, &raisecumu);
#endif
    myDeterredCall.SetImpliedFactor(futureFold);
    if( bGamble < 1 ) //ComBot only, not DangerBot
    {
        myDeterredCall.insuranceDeterrent = 1-futureFold; //more likely to fold due to uncertainty
    }

    StatResult right = (certainty > 1) ? statmean : ((statmean * certainty) + statworse * (1 - certainty));

	GainModel hybridgainDeterred(hybridMagnified,&myDeterredCall);
	GainModelNoRisk hybridgain(right,&myDeterredCall);

#ifdef LOGPOSITION
    if( bGamble == 0 )
    {
        logFile << "uncertainty      " << uncertainty << endl;
        logFile << "detailPCT.stdDev " << detailPCT.stdDev << endl;
        logFile << "V Factor         " << volatilityFactor << endl;
    }
    logFile << "BetToCall " << certainty << ", pct " << statmean.pct << " ... " << right.pct << " ... " << statworse.pct << endl;
    logFile << "impliedFactor... " << futureFold << endl;
#endif



	AutoScalingFunction ap_passive(&hybridgainDeterred,&hybridgain,0.0,maxShowdown,hybridMagnified.pct*statmean.pct*certainty - certainty + 1,&myDeterredCall);

    HoldemFunctionModel * (hybridChoice[2]) =  { &ap_passive, &hybridgainDeterred };

    StateModel ap_aggressive( &myDeterredCall, hybridChoice[bGamble] );



    HoldemFunctionModel& choicemodel = ap_aggressive;

    const float64 bestBet = solveGainModel(&choicemodel);

#ifdef LOGPOSITION
    if( bestBet < betToCall + ViewTable().GetChipDenom() )
    {
        logFile << "\"shuftip\"... " << hybridMagnified.pct*statmean.pct << endl;
        logFile << "Geom("<< bestBet <<")=" << hybridgainDeterred.f(bestBet) << endl;
        logFile << "Algb("<< bestBet <<")=" << hybridgain.f(bestBet) << endl;
    }


	if( bestBet >= betToCall - ViewTable().GetChipDenom() )
	{
		int32 raiseStep = 0;
        float64 rAmount =  myDeterredCall.RaiseAmount(bestBet,raiseStep);
        while( rAmount < maxShowdown )
        {
            rAmount =  myDeterredCall.RaiseAmount(bestBet,raiseStep);
            logFile << "OppRAISEChance% ... " << myDeterredCall.pRaise(bestBet,raiseStep) << " @ $" << rAmount;
            logFile << "\tBetWouldFold%" << myDeterredCall.pWin(rAmount) << endl;
            ++raiseStep;
        }
	}
    logFile << "Guaranteed $" << myDeterredCall.stagnantPot() << endl;

        logFile << "OppFoldChance% ... " << myDeterredCall.pWin(bestBet) << "   d\\" << myDeterredCall.pWinD(bestBet) << endl;
        if( myDeterredCall.pWin(bestBet) > 0 )
        {
            logFile << "confirm " << choicemodel.f(bestBet) << endl;
        }

#endif

    return bestBet;


}


#ifndef NO_AWKWARD_MODELS

float64 ImproveGainRankStrategy::MakeBet()
{
	setupPosition();

	if( maxShowdown <= 0 ) return 0;

    const float64 improveMod = detailPCT.improve; //Generally preflop is negative here, so you probably don't want to accentuate that
    const float64 improvePure= (improveMod+1)/2;
    const float64 minWin = pow(statworse.pct,expectedVS);
    const float64 improveDev = detailPCT.stdDev * (1-improvePure) + detailPCT.avgDev * improvePure;

    float64 distrScale = improveMod+minWin+1 ;
    if( bGamble >= 2 )
    {
        distrScale = (-improveMod/2)+minWin+1 ;
    }

    if( bGamble >= 1 )
    {
        #ifdef LOGPOSITION
            logFile << minWin <<" ... "<< minWin+2 <<" =" << distrScale << endl;
        #endif
    }


    CallCumulationD &choicecumu = rankcumu;
    CallCumulationD &raisecumu = rankcumu;


#ifdef ANTI_PRESSURE_FOLDGAIN
    ExactCallBluffD myDeterredCall(myPositionIndex, &(ViewTable()), statranking.pct, &choicecumu, &raisecumu);
#else
    ExactCallBluffD myDeterredCall(myPositionIndex, &(ViewTable()), &choicecumu, &raisecumu);
#endif

    myDeterredCall.insuranceDeterrent = statranking.pct;

    const float64 fullVersus = myDeterredCall.callingPlayers();
    if( bGamble >= 2 )
    {
        const float64 improveLevel = (improveMod*sqrt(improveDev*2)*2+1)/2;
        const float64 rVersus = expectedVS*improveLevel*improveLevel + fullVersus*(1-improveLevel*improveLevel);
        myDeterredCall.callingPlayers( rVersus );
        #ifdef LOGPOSITION
            logFile << expectedVS <<" ... "<< fullVersus <<" =" << rVersus  << "\t --expectedVersus ... fullVersus" << endl;
        #endif
	}

    StatResult left = hybridMagnified;
    //StatResult right = statmean;
    if( bGamble >= 1 )
    {
        myDeterredCall.SetImpliedFactor(distrScale);
        left = statranking;
    //    right = statranking;
    }

	GainModel hybridgainDeterred(left,&myDeterredCall);
    StateModel hybridgainDeterred_aggressive(&myDeterredCall,&hybridgainDeterred);


    const float64 bestBet = solveGainModel(&hybridgainDeterred_aggressive);

#ifdef LOGPOSITION
    if( bestBet < betToCall + ViewTable().GetChipDenom() )
    {
        logFile << "\"shuftip\"... " << hybridMagnified.pct*statmean.pct << endl;
        logFile << "Geom("<< bestBet <<")=" << hybridgainDeterred_aggressive.f(bestBet) << endl;

    }

	if( bestBet >= betToCall - ViewTable().GetChipDenom() )
	{
		int32 raiseStep = 0;
        float64 rAmount =  myDeterredCall.RaiseAmount(bestBet,raiseStep);
        while( rAmount < maxShowdown )
        {
            rAmount =  myDeterredCall.RaiseAmount(bestBet,raiseStep);
            logFile << "OppRAISEChance% ... " << myDeterredCall.pRaise(bestBet,raiseStep) << " @ $" << myDeterredCall.RaiseAmount(bestBet,raiseStep);
            logFile << "\tBetWouldFold%" << myDeterredCall.pWin(rAmount) << endl;
            ++raiseStep;
        }
	}
        logFile << "Guaranteed $" << myDeterredCall.stagnantPot() << endl;
        logFile << "OppFoldChance% ... " << myDeterredCall.pWin(bestBet) << "   d\\" << myDeterredCall.pWinD(bestBet) << endl;

        if( myDeterredCall.pWin(bestBet) > 0 )
        {
            logFile << "confirm " << hybridgainDeterred_aggressive.f(bestBet) << endl;
        }

#endif

    return bestBet;


}


float64 DeterredGainRankStrategy::MakeBet()
{
	setupPosition();

	if( maxShowdown <= 0 ) return 0;


    const float64 certainty = betToCall / maxShowdown;
    const float64 uncertainty = fabs( statranking.pct - statmean.pct );
    const float64 volatilityFactor = 1 - sqrt(  detailPCT.stdDev*detailPCT.stdDev + uncertainty*uncertainty  );

	const float64 futureFold = volatilityFactor*(1-certainty) + certainty;

#ifdef LOGPOSITION
    logFile << "uncertainty      " << uncertainty << endl;
    logFile << "detailPCT.stdDev " << detailPCT.stdDev << endl;
    logFile << "V Factor         " << volatilityFactor << endl;
    logFile << "BetToCall PCT    " << certainty << endl;
    logFile << "impliedFactor... " << futureFold << endl;
#endif



    CallCumulationD &choicecumu = rankcumu;
    CallCumulationD &raisecumu = rankcumu;


#ifdef ANTI_PRESSURE_FOLDGAIN
    //ExactCallD myExpectedCall(myPositionIndex, &(ViewTable()), statranking.pct, &choicecumu);
    ExactCallBluffD myDeterredCall(myPositionIndex, &(ViewTable()), statranking.pct, &choicecumu, &raisecumu);
#else
    //ExactCallD myExpectedCall(myPositionIndex, &(ViewTable()), &choicecumu);
    ExactCallBluffD myDeterredCall(myPositionIndex, &(ViewTable()), &choicecumu, &raisecumu);
#endif

    myDeterredCall.insuranceDeterrent = statranking.pct;

    myDeterredCall.SetImpliedFactor(futureFold);


	GainModel hybridgainDeterred_passive(hybridMagnified,&myDeterredCall);
	//GainModelNoRisk hybridgain_passive(statmean,&myDeterredCall);
	StateModel hybridgainDeterred_aggressive(&myDeterredCall,&hybridgainDeterred_passive);
	//GainModelNoRiskBluff hybridgain_aggressive(statmean,&myDeterredCall);

    HoldemFunctionModel *(hybridgainDeterred_agressiveness[2]) = {&hybridgainDeterred_passive, &hybridgainDeterred_aggressive };
    //HoldemFunctionModel *(hybridgain_agressiveness[2]) = {&hybridgain_passive, &hybridgain_aggressive };

	//AutoScalingFunction choicemodel(hybridgainDeterred_agressiveness[bGamble],hybridgain_agressiveness[bGamble],0.0,maxShowdown,hybridMagnified.pct*statmean.pct,&myDeterredCall);


    const float64 bestBet = solveGainModel(hybridgainDeterred_agressiveness[bGamble]);

#ifdef LOGPOSITION
    if( bestBet < betToCall + ViewTable().GetChipDenom() )
    {
        logFile << "\"shuftip\"... " << hybridMagnified.pct*statmean.pct << endl;
        logFile << "Geom("<< bestBet <<")=" << hybridgainDeterred_agressiveness[bGamble]->f(bestBet) << endl;


    }


	if( bestBet >= betToCall - ViewTable().GetChipDenom() )
	{
		int32 raiseStep = 0;
        float64 rAmount =  myDeterredCall.RaiseAmount(bestBet,raiseStep);
        while( rAmount < maxShowdown )
        {
            rAmount =  myDeterredCall.RaiseAmount(bestBet,raiseStep);
            logFile << "OppRAISEChance% ... " << myDeterredCall.pRaise(bestBet,raiseStep) << " @ $" << myDeterredCall.RaiseAmount(bestBet,raiseStep);
            logFile << "\tBetWouldFold%" << myDeterredCall.pWin(rAmount) << endl;
            ++raiseStep;
        }
	}
    logFile << "Guaranteed $" << myDeterredCall.stagnantPot() << endl;
    if( bGamble == 1 )
    {
        logFile << "OppFoldChance% ... " << myDeterredCall.pWin(bestBet) << "   d\\" << myDeterredCall.pWinD(bestBet) << endl;

        if( myDeterredCall.pWin(bestBet) > 0 )
        {
            logFile << "confirm " << hybridgainDeterred_agressiveness[bGamble]->f(bestBet) << endl;
        }
    }
#endif

    return bestBet;


}
#endif



float64 CorePositionalStrategy::MakeBet()
{
    setupPosition();


    ///VARIABLE: the slider can move due to avgDev too, maybe....

//#ifdef RANK_CALL_CUMULATION
//	CallCumulationD &choicecumu = foldcumu;
//#else
    CallCumulationD &choicecumu = callcumu;
    CallCumulationD &raisecumu = foldcumu;
//#endif



    //SlidingPairCallCumulationD choicecumu( &callcumu, &foldcumu, timing[DT]/2 );
    //SlidingPairCallCumulationD choicecumu( &callcumu, &foldcumu, detailPct.avgDev*2 );


#ifdef ANTI_PRESSURE_FOLDGAIN
#define HANDRANK_MACRO  statranking.pct,
#endif
    ExactCallBluffD myExpectedCall(myPositionIndex, &(ViewTable()),HANDRANK_MACRO &choicecumu, &raisecumu);//foldcumu);
    ExactCallBluffD myLimitCall(myPositionIndex, &(ViewTable()),HANDRANK_MACRO &choicecumu, &raisecumu);//foldcumu);
    const float64 committed = ViewPlayer().GetContribution() + myBet;
    ExactCallD myCommittalCall(myPositionIndex, &(ViewTable()),HANDRANK_MACRO &choicecumu, committed );
#ifdef ANTI_PRESSURE_FOLDGAIN
#undef HANDRANK_MACRO
#endif
    float64 raiseBattle = ViewTable().GetChipDenom();
    if( betToCall > raiseBattle )
    {
        raiseBattle = betToCall;
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


	StateModel rankGeomBluff(&myExpectedCall,&rankGeom);
	StateModel meanGeomBluff(&myExpectedCall,&meanGeom);
    StateModel worstAlgbBluff(&myLimitCall,&worstAlgb);
    StateModel rankAlgbBluff(&myExpectedCall,&rankAlgb);
    StateModel meanAlgbBluff(&myExpectedCall,&meanAlgb);
    StateModel hybridGeomBluff(&myExpectedCall,&hybridGeom);

    #ifdef DEBUGASSERT
        if( bGamble >= BGAMBLE_MAX )
        {
                std::cout << "Incorrect bGAMBLE" << endl;
                exit(1);
        }
    #endif

    HoldemFunctionModel* (lookup[BGAMBLE_MAX]) = { &rankGeom, &meanGeom, &worstAlgb, &rankAlgb, &meanAlgb, &rankGeomPC, &meanGeomPC, &hybridGeom, &hybridAlgb
		,&rankGeomBluff ,&meanGeomBluff,&worstAlgbBluff,&rankAlgbBluff,&meanAlgbBluff,&hybridGeomBluff	};

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

	#ifdef LOGPOSITION
		if( bGamble >= 9 && bGamble <= 15 )
		{
		    int32 raiseStep = 0;
            float64 rAmount =  myExpectedCall.RaiseAmount(bestBet,raiseStep);
            while( rAmount < maxShowdown )
            {
                rAmount =  myExpectedCall.RaiseAmount(bestBet,raiseStep);
                logFile << "OppRAISEChance% ... " << myExpectedCall.pRaise(bestBet,raiseStep) << " @ $" << rAmount;
                logFile << "\tBetWouldFold%" << myExpectedCall.pWin(rAmount) << endl;
                ++raiseStep;
            }
            logFile << "Guaranteed $" << myExpectedCall.stagnantPot() << endl;
			logFile << "OppFoldChance% ... " << myExpectedCall.pWin(bestBet) << "   d\\" << myExpectedCall.pWinD(bestBet) << endl;

			if( myExpectedCall.pWin(bestBet) > 0 )
			{
				logFile << "confirm " << lookup[bGamble]->f(bestBet) << endl;
			}
		}
	#endif

    return bestBet;


}
