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

#include <math.h>
#include "stratPosition.h"
#include <float.h>

#include "stratFear.h"

//#define DEBUG_TRAP_AS_NORMAL

//Okay, so riskprice is how you control attitude, e.g. geom to algb, or trap_left to trap_right
//but ACTREACTUSES is how you control fear.






/// Summary of contained strategies:
/// 1. ImproveStrategy (This is the old fashioned Geom<-->Algb vs. Rank<-->WorseNextCard)
/// 2. ImproveGainStrategy (This is Norm/Trap/Ace using empirical data)
/// 3. DeterredGainStrategy (DangerBot: focus more on hybridMagnified to make decisions)


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

        }

        else
        {
            logFile << "==========#" << ViewTable().handnum << "==========" << endl;
        }
    #endif


    DistrShape w_wl(0);


    CommunityPlus onlyCommunity;
    onlyCommunity.SetUnique(h);

    CommunityPlus withCommunity;
    withCommunity.SetUnique(ViewDealtHand());
    withCommunity.AppendUnique(onlyCommunity);

    //if(bLogWorse)
    {
        StatsManager::QueryDefense(foldcumu,withCommunity,onlyCommunity,cardsInCommunity);
        foldcumu.ReversePerspective();
    }
    //else{

    //}
    ViewTable().CachedQueryOffense(callcumu,onlyCommunity, withCommunity);
    //StatsManager::QueryOffense(callcumu,withCommunity,onlyCommunity,cardsInCommunity );
    StatsManager::Query(0,&detailPCT,&w_wl,withCommunity,onlyCommunity,cardsInCommunity);
    statmean = GainModel::ComposeBreakdown(detailPCT.mean,w_wl.mean);
    statworse = foldcumu.oddsAgainstBestTwoHands(); //GainModel::ComposeBreakdown(detailPCT.worst,w_wl.worst);
    //CallStats is foldcumu

    #ifdef LOGPOSITION
    logFile << "*" << endl;
    #endif





    const float64 rarityA3 = foldcumu.Pr_haveWinPCT_orbetter(0.5);

//You can tie in rank if and only if you tie in mean
    statrelation.wins = 1 - rarityA3;
    statrelation.splits = statmean.splits;
    statrelation.loss = rarityA3;
    const float64 scaleTotalA3 = statrelation.wins + statrelation.splits + statrelation.loss;
    statrelation.wins /= scaleTotalA3;
    statrelation.splits /= scaleTotalA3;
    statrelation.loss /= scaleTotalA3;
    statrelation.genPCT();


    const float64 rarity3 = callcumu.Pr_haveWinPCT_orbetter(statmean.pct);

    statranking.wins = 1 - rarity3;
    statranking.splits = statmean.splits;
    statranking.loss = rarity3;
    const float64 scaleTotal3 = statranking.wins + statranking.splits + statranking.loss;
    statranking.wins /= scaleTotal3;
    statranking.splits /= scaleTotal3;
    statranking.loss /= scaleTotal3;
    statranking.genPCT();

//Pick the better one for hybrid
    StatResult statHybridR;
    if( statranking.pct > statrelation.pct )
    {
        statHybridR = statranking;
    }else
    {
        statHybridR = statrelation;
    }

    hybridMagnified.wins = sqrt(statmean.wins*statHybridR.wins);
    hybridMagnified.splits = sqrt(statmean.splits*statHybridR.splits);
    hybridMagnified.loss = sqrt(statmean.loss*statHybridR.loss);
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
    logFile << "(Worst) " << statworse.pct * 100 << "%"  << std::endl;
    if(bLogWorse)
    {
        logFile << "(W.w) " << statworse.wins * 100 << "%"  << std::endl;
        logFile << "(W.s) " << statworse.splits * 100 << "%"  << std::endl;
        logFile << "(W.l) " << statworse.loss * 100 << "%"  << std::endl;
    }
    if(bLogRanking)
    {
        logFile << "(All-in) " << statrelation.pct * 100 << "%"  << std::endl;
        logFile << "(Re.s) " << statrelation.splits * 100 << "%"  << std::endl;
        logFile << "(Outright) " << statranking.pct * 100 << "%"  << std::endl;
        logFile << "(Ra.s) " << statranking.splits * 100 << "%"  << std::endl;
    }
    if(bLogHybrid)
    {
        logFile << "(Hybrid) " << hybridMagnified.pct * 100 << "%"  << std::endl;
        //logFile << "(H.w) " << hybridMagnified.wins * 100 << "%"  << std::endl;
        logFile << "(H.s) " << hybridMagnified.splits * 100 << "%"  << std::endl;
        //logFile << "(H.l) " << hybridMagnified.loss * 100 << "%"  << std::endl;
    }
#endif

}



void PositionalStrategy::setupPosition()
{
        #ifdef LOGPOSITION
            logFile << endl;
            HandPlus convertOutput;
            convertOutput.SetUnique(ViewDealtHand());
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

}

float64 PositionalStrategy::solveGainModel(HoldemFunctionModel* targetModel, CallCumulationD* const e)
{

        #if defined(DEBUG_GAIN) && defined(DEBUG_SINGLE_HAND)
            std::ofstream excel( (ViewPlayer().GetIdent() + ".functionlog.csv").c_str() );
            if( !excel.is_open() ) std::cerr << "\n!functionlog.cvs file access denied" << std::endl;
            targetModel->breakdown(1000,excel,betToCall,maxShowdown);
            //myExpectedCall.breakdown(0.005,excel);

            excel.close();
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
            float64 xw;
            float64 foldgainVal = (targetModel->GetFoldGain(e, &xw));
			float64 numfolds = xw * e->Pr_haveWinPCT_orbetter_continuous(statmean.pct);

            //logFile << "selected risk  " << (choicePoint - myBet)/(maxShowdown - myBet) << endl;

            logFile << "Choice Optimal " << choicePoint << endl;
            logFile << "Choice Fold " << choiceFold << endl;
			logFile << "FoldGain()=" << foldgainVal;
			logFile << " x " << xw << "(=" << numfolds << " folds)\tvs play:" << (raiseGain + foldgainVal) << endl;
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

template< typename T >
void PositionalStrategy::printBetGradient(ExactCallBluffD & rl, ExactCallBluffD & rr, T & m, ExpectedCallD & tablestate, float64 separatorBet)
{

    int32 maxcallStep = -1;
    int32 raiseStep = 0;
    float64 rAmount =  rl.RaiseAmount(betToCall,raiseStep);
    while( rAmount < separatorBet )
    {
        rAmount =  rl.RaiseAmount(betToCall,raiseStep);
        const float64 oppRaisedFoldGain = rl.FoldGain(betToCall - tablestate.alreadyBet(),rAmount);
        logFile << "OppRAISEChance";
        if( oppRaisedFoldGain > m.g_raised(betToCall,rAmount) ){  logFile << " [F] ";  } else {  logFile << " [*] ";  maxcallStep = raiseStep+1; }

        logFile << rl.pRaise(betToCall,raiseStep,maxcallStep) << " @ $" << rAmount;
        logFile << "\tfold -- left" << rl.pWin(rAmount) << "  " << rr.pWin(rAmount) << " right" << endl;


        if( rAmount >= maxShowdown ) break;

        ++raiseStep;
    }

    const float64 minNextRaiseTo = (separatorBet*2-betToCall);
    if( maxShowdown - minNextRaiseTo < DBL_EPSILON ) return;

    logFile << "\t--" << endl;

    maxcallStep = -1;
    raiseStep = 0;
    rAmount =  rl.RaiseAmount(separatorBet,raiseStep);
    while( rAmount <= maxShowdown )
    {
        rAmount =  rl.RaiseAmount(separatorBet,raiseStep);
        const float64 oppRaisedFoldGain = rl.FoldGain(separatorBet - tablestate.alreadyBet(),rAmount);
        logFile << "OppRAISEChance";
        if( oppRaisedFoldGain > m.g_raised(separatorBet,rAmount) ){  logFile << " [F] ";  } else {  logFile << " [*] ";  maxcallStep = raiseStep+1; }

        logFile << rl.pRaise(separatorBet,raiseStep,maxcallStep) << " @ $" << rAmount;
        logFile << "\tfold -- left" << rl.pWin(rAmount) << "  " << rr.pWin(rAmount) << " right" << endl;


        if( rAmount >= maxShowdown ) break;

        ++raiseStep;
    }
}






///==============================
///   AceBot, TrapBot, NormBot
///==============================



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


    const float64 awayFromDrawingHands = 1.0 / (ViewTable().NumberInHand() - 1);
    StatResult statversus = (statrelation * (awayFromDrawingHands)) + (statranking * (1.0-awayFromDrawingHands));
    statversus.genPCT();


    CallCumulationD &choicecumu = callcumu;
    CallCumulationD &raisecumu = foldcumu;


#ifdef ANTI_PRESSURE_FOLDGAIN
    ExpectedCallD   tablestate(myPositionIndex,  &(ViewTable()), statranking.pct, statmean.pct);
    ExactCallBluffD myDeterredCall(&tablestate, &choicecumu, &raisecumu);
    ExactCallBluffD myDeterredCall_left(&tablestate, &choicecumu, &raisecumu);
    ExactCallBluffD myDeterredCall_right(&tablestate, &choicecumu, &raisecumu);
#else
    ExactCallBluffD myDeterredCall(myPositionIndex, &(ViewTable()), &choicecumu, &raisecumu);
#endif

    OpponentFoldWait myFearControl(&tablestate);


    const float64 riskprice = myDeterredCall.RiskPrice();
    const float64 geom_algb_scaler = (riskprice < maxShowdown) ? riskprice : maxShowdown;
    const float64 min_worst_scaler = myFearControl.FearStartingBet(myDeterredCall,statworse.repeated,riskprice);


    const float64 fullVersus = myDeterredCall.callingPlayers();
    const float64 peopleDrawing = (1 - improvePure) * (ViewTable().NumberInHand() - 1);//You probably don't have to beat the people who folded, especially if you are going to improve your hand
    const float64 newVersus = (fullVersus - peopleDrawing*(1-improvePure)*detailPCT.stdDev);


//bGamble == 2 is ActionBot
//bGamble == 1 is TrapBot

//In the future actOrReact should be based on opponent betting patterns
#ifdef MODEL_REACTION
    float64 actOrReact = myDeterredCall.ActOrReact(betToCall,myBet,maxShowdown);
	actOrReact = 1 - (1-actOrReact)*(1-actOrReact);
#endif // MODEL_REACTION


    StatResult left = statversus;
    StatResult base_right = statversus;//statmean;  (NormalBot used this setup.)

    StatResult right = statworse;
//TrapBot and ActionBot are based on statversus only
    if( bGamble >= 1 )
    {
        base_right = statversus;
        #ifdef MODEL_REACTION
            const float64 enemyChances = actOrReact;
        #else
            const float64 enemyChances = 1.0;//(ViewTable().GetNumberInHand() - 1.0) / ViewTable().GetNumberInHand() / 2;
        #endif // MODEL_REACTION, with #else
        left = statversus;
        #ifndef DEBUG_TRAP_AS_NORMAL
        //targetWorsenBy is here
        left.wins -= detailPCT.avgDev/2.0*enemyChances; //2.0 is to estimate median
        left.loss += detailPCT.avgDev/2.0*enemyChances;
        left.pct -= detailPCT.avgDev/2.0*enemyChances;
        //Since detailPCT is based on statmean, not statversus, it is possible for zero crossings
        if( left.pct < 0 || left.wins < 0 )
        {
            left.wins = 0;
            left.loss = 1 - left.splits;
            left.genPCT();
        }
        //Need scaling
        #endif

        //targetWorsenBy is also here
		#ifdef MODEL_REACTION
			base_right.wins += detailPCT.avgDev/2.0;
			base_right.loss -= detailPCT.avgDev/2.0;
			base_right.pct += detailPCT.avgDev/2.0;
			if( base_right.pct > 1 || base_right.wins > 1-base_right.splits )
			{
				base_right.wins = 1 - base_right.splits;
				base_right.loss = 0;
				base_right.genPCT();
			}
        //Need scaling
		#else
			right.wins += detailPCT.avgDev/2.0;
			right.loss -= detailPCT.avgDev/2.0;
			right.pct += detailPCT.avgDev/2.0;
			if( right.pct > 1 || right.wins > 1-right.splits )
			{
				right.wins = 1 - right.splits;
				right.loss = 0;
				right.genPCT();
			}
			//Need scaling
		#endif



        myDeterredCall_right.insuranceDeterrent = oppInsuranceBigBet;





        if( bGamble >= 2 )
        {
            myDeterredCall_left.SetImpliedFactor(impliedOddsGain);

//Need scaling
            myDeterredCall_right.callingPlayers(newVersus);
        }
    }

#ifdef MODEL_REACTION
    right.repeated = (1 - actOrReact);//Generally ignored, only base_right.repeated is really used
    base_right.repeated = actOrReact;


    left.repeated = 1;
	GainModel geomModel(left,right,myDeterredCall_left); //"right" probably doesn't have to be there, since left.repeated is 1.
	GainModel geomModel_fear(base_right,right,myDeterredCall_right);
	statversus.repeated = 1;
	GainModelNoRisk algbModel(statversus,right,myDeterredCall_left); //"right" probably doesn't have to be there, since left.repeated is 1.
	GainModelNoRisk algbModel_fear(base_right,right,myDeterredCall_right);
#else
    GainModel geomModel(left,myDeterredCall_left);
    GainModelNoRisk algbModel(statversus,myDeterredCall_right);

	GainModel geomModel_fear(right,myDeterredCall_left);
	GainModelNoRisk algbModel_fear(right,myDeterredCall_right);
#endif // MODEL_REACTION, with #else


#ifdef LOGPOSITION
	if( bGamble == 0 )
	{ logFile << " -  NORMAL  - " << endl;}
	else if( bGamble == 1 )
	{ logFile << " -  TRAP  - " << endl;}
	else if( bGamble == 2 )
	{ logFile << " -  ACTION  - " << endl;}


    if( bGamble >= 1 )
    {
        #ifdef LOGPOSITION
//            logFile << minWin <<" ... "<< minWin+2 <<" = impliedfactor " << distrScale << endl;
            logFile << " " << improvePure <<" improvePure " << endl;
            logFile << "Likely Worsen By "<< targetWorsenBy << endl; //TRAPBOT, ACTIONBOT
            if( bGamble >= 2 ) logFile << "impliedOddsGain would be " << impliedOddsGain << endl; //ACTIONBOT
            logFile << "opp Likely to fold " << oppInsuranceBigBet << " (to a big bet) is the same as improveMod/2 if positive" << endl; //TRAPBOT, ACTIONBOT
            if( bGamble >= 2 ) logFile << "Can push expectedVersus from " << fullVersus << " ... " << newVersus << " ... " << (fullVersus - peopleDrawing) << endl; //ACTIONBOT
        #endif
    }
    logFile <<
    #ifdef MODEL_REACTION
	 " Act(0%) or React(100%)? " << (actOrReact * 100) << "% --> pct of " << base_right.pct << ":React ... " << algbModel_fear.ViewShape().pct <<
	#else
	 geomModel.ViewShape().pct << ":React ... " <<
	#endif // MODEL_REACTION
	 " ... " << algbModel_fear.ViewShape().pct << ":Act" << endl;
#endif

///From geom to algb
	AutoScalingFunction<GainModel,GainModelNoRisk> hybridgainDeterred_aggressive(geomModel,algbModel,0.0,geom_algb_scaler,left.pct*base_right.pct,&tablestate);
	AutoScalingFunction<GainModel,GainModelNoRisk> hybridgain_fear(geomModel_fear,algbModel_fear,0.0,geom_algb_scaler,left.pct*base_right.pct,&tablestate);


///From regular to fear A(x2)
	AutoScalingFunction<  AutoScalingFunction<GainModel,GainModelNoRisk>  ,  AutoScalingFunction<GainModel,GainModelNoRisk>  >
            ap(hybridgainDeterred_aggressive,hybridgain_fear,min_worst_scaler,riskprice,&tablestate);

	StateModel<  AutoScalingFunction<GainModel,GainModelNoRisk>  ,  AutoScalingFunction<GainModel,GainModelNoRisk>  >
	        choicemodel( myDeterredCall_left, &ap );
#ifdef DEBUG_TRAP_AS_NORMAL
#ifdef LOGPOSITION
logFile << "  DEBUGTRAPASNORMAL DEBUGTRAPASNORMAL DEBUGTRAPASNORMAL  " << endl;
#endif
    //StateModel choicemodel( &myDeterredCall_left, &hybridgainDeterred_aggressive );
    const float64 bestBet = solveGainModel(&choicemodel) ;
    StateModel & rolemodel = choicemodel;
#else
	///From regular to fear B(x2)
	AutoScalingFunction<  AutoScalingFunction<GainModel,GainModelNoRisk>  ,  AutoScalingFunction<GainModel,GainModelNoRisk>  >
            ap_right(hybridgainDeterred_aggressive,hybridgain_fear,min_worst_scaler,riskprice,&tablestate);
    StateModel<  AutoScalingFunction<GainModel,GainModelNoRisk>  ,  AutoScalingFunction<GainModel,GainModelNoRisk>  >
            choicemodel_right( myDeterredCall_right, &ap_right );



    AutoScalingFunction<   StateModel<  AutoScalingFunction<GainModel,GainModelNoRisk>  ,  AutoScalingFunction<GainModel,GainModelNoRisk>  >
                         , StateModel<  AutoScalingFunction<GainModel,GainModelNoRisk>  ,  AutoScalingFunction<GainModel,GainModelNoRisk>  >
                       >
            rolemodel(choicemodel,choicemodel_right,betToCall,riskprice,&tablestate);




//DEBUG //
/*
 //   if( bGamble == 1 && ViewTable().GetPrevPotSize() > 3.0 )
    if( bGamble == 2 )
    {

		choicemodel.bTraceEnable = true;
		const float64 bestBet = (bGamble == 0) ? solveGainModel(&choicemodel) : solveGainModel(&rolemodel);
		logFile << bestBet << endl;


		std::cout << "riskprice @ " << riskprice << endl;

        #ifdef DEBUG_TRACE_DEXF
            myDeterredCall_right.traceOut = &(std::cout);
		#endif

algbModel.bTraceEnable = true;
ap.bTraceEnable = true;
hybridgainDeterred_aggressive.bTraceEnable = true;

        const float64 gr1 = choicemodel.g_raised(0.44,0.9);
		const float64 gdr1 = choicemodel.gd_raised(0.44,0.9, gr1);


		std::cout << gdr1 << "  <-- d with raiseamount 0.9" << endl;



		const float64 y1 = choicemodel.f(0.44);
		const float64 dy1 = choicemodel.fd(0.44,y1);
		//const float64 y2 = choicemodel.f(115.05);
		//const float64 dy2 = choicemodel.fd(115.05,y1);


		std::cout << y1 << " <-- choicemodel.f(0.44)" << endl;
		std::cout << dy1 << " <-- d choicemodel.f(0.44)" << endl;





		const float64 by1 = hybridgainDeterred_aggressive.f(0.44);



		//std::cout << ay1 << "   <-- ap" << endl;
		std::cout << ady1 << "   <-- ap" << endl;
		//std::cout << by1 << "   <-- lowbet" << endl;
		std::cout << bdy1 << "   <-- lowbet" << endl;
		//std::cout << cy1 << "   <-- fearbet" << endl;



        #ifdef DEBUG_TRACE_PWIN
            myDeterredCall.traceOut = &logFile;
            myDeterredCall_left.traceOut = &logFile;
            //myDeterredCall_right.traceOut = &logfile;
		#endif
        float64 rAmount =  myDeterredCall.RaiseAmount(0.25,3);
        logFile << myDeterredCall.pRaise(0.25,3) << " @ $" << rAmount;
        logFile << "\tfold -- left" << myDeterredCall_left.pWin(rAmount) << "  " << myDeterredCall_right.pWin(rAmount) << " right" << endl;

        exit(1);




hybridgainDeterred_aggressive.bTraceEnable = true;





        const float64 ady1 = hybridgainDeterred_aggressive.f(65);
        const float64 c11 = hybridgain_fear.f(65);
        const float64 ay1 = ap_right.f(65);

        const float64 z0 = rolemodel.f(65);
        const float64 bdy1 = choicemodel_right.f(65);

        std::cout << ady1 << "   <-- regular" << endl;
        std::cout << c11 << "   <-- fear" << endl;
        std::cout << ay1 << "   (should pick fear)" << endl;
        std::cout << bdy1 << "   with state!" << endl;
        std::cout << z0 << "  @ ultimate" << endl;

		exit(1);
	}
	*//*
ap.bTraceEnable = true;
geomModel.bTraceEnable = true;
geomModel_fear.bTraceEnable = true;
const float64 y1 = ap.f(betToCall);
		const float64 dy1 = ap.fd(betToCall,y1);

		std::cout << y1 << " <-- ap.f(betToCall)" << endl;
		std::cout << dy1 << " <-- d ap.f(betToCall)" << endl;
exit(1);
*/



    const float64 bestBet = (bGamble == 0) ? solveGainModel(&choicemodel, &callcumu) : solveGainModel(&rolemodel, &callcumu);

#endif








#ifdef LOGPOSITION
    const float64 nextBet = betToCall + ViewTable().GetMinRaise();
    const float64 viewBet = ( bestBet < betToCall + ViewTable().GetChipDenom() ) ? nextBet : bestBet;

    logFile << "\"riskprice\"... " << riskprice << "(" << geom_algb_scaler << ")" << endl;
    logFile << "When betting " << min_worst_scaler << ", oppFoldChance is first " << statworse.repeated << endl;

#ifdef VERBOSE_STATEMODEL_INTERFACE
    choicemodel.f(betToCall);
    logFile << " AgainstCall("<< betToCall <<")=" << choicemodel.gainNormal << endl;
    logFile << "AgainstRaise("<< betToCall <<")=" << choicemodel.gainRaised << endl;
    logFile << "        Push("<< betToCall <<")=" << choicemodel.gainWithFold << endl;



    choicemodel.f(viewBet);
    logFile << " AgainstCall("<< viewBet <<")=" << choicemodel.gainNormal << endl;
    logFile << "AgainstRaise("<< viewBet <<")=" << choicemodel.gainRaised << endl;
    logFile << "        Push("<< viewBet <<")=" << choicemodel.gainWithFold << endl;

    if( bGamble != 0 )
    {
        choicemodel_right.f(viewBet);
        logFile << " AgainstCall OtherDeter("<< viewBet <<")=" << choicemodel_right.gainNormal << endl;
        logFile << "AgainstRaise OtherDeter("<< viewBet <<")=" << choicemodel_right.gainRaised << endl;
        logFile << "        Push OtherDeter("<< viewBet <<")=" << choicemodel_right.gainWithFold << endl;
    }
#endif
    logFile << "Call Regular("<< viewBet <<")=" << hybridgainDeterred_aggressive.f(bestBet) << endl;
    logFile << "   Call Fear("<< viewBet <<")=" << hybridgain_fear.f(bestBet) << endl;


    printBetGradient< StateModel<  AutoScalingFunction<GainModel,GainModelNoRisk>  ,  AutoScalingFunction<GainModel,GainModelNoRisk>  > >
                     (myDeterredCall_left, myDeterredCall_right, choicemodel, tablestate, viewBet);


    logFile << "Guaranteed > $" << tablestate.stagnantPot() << " is in the pot for sure" << endl;
    logFile << "OppFoldChance% ... left " << myDeterredCall_left.pWin(viewBet) << " --" << myDeterredCall_right.pWin(viewBet) << " right" << endl;
    if( myDeterredCall.pWin(bestBet) > 0 )
    {
        logFile << "confirm Normal " << choicemodel.f(viewBet) << endl;
        logFile << "confirm " << rolemodel.f(viewBet) << endl;
    }

#endif

    return bestBet;


}






///=================================
///   DangerBot, SpaceBot, ComBot
///=================================


float64 DeterredGainStrategy::MakeBet()
{
	setupPosition();

	if( maxShowdown <= 0 ) return 0;


    const float64 awayFromDrawingHands = 1.0 / (ViewTable().NumberInHand() - 1);
    StatResult statversus = (statrelation * (awayFromDrawingHands)) + (statranking * (1.0-awayFromDrawingHands));
    statversus.genPCT();

    CallCumulationD &choicecumu = callcumu;
    CallCumulationD &raisecumu = foldcumu;


#ifdef ANTI_PRESSURE_FOLDGAIN
    ExpectedCallD   tablestate(myPositionIndex,  &(ViewTable()), statranking.pct, statmean.pct);
    ExactCallBluffD myDeterredCall(&tablestate, &choicecumu, &raisecumu);
#else
    //ExactCallD myExpectedCall(myPositionIndex, &(ViewTable()), &choicecumu);
    ExactCallBluffD myDeterredCall(myPositionIndex, &(ViewTable()), &choicecumu, &raisecumu);
#endif

    OpponentFoldWait myFearControl(&tablestate);


    const float64 riskprice = myDeterredCall.RiskPrice();
    const float64 geom_algb_scaler = (riskprice < maxShowdown) ? riskprice : maxShowdown;
    const float64 min_worst_scaler = myFearControl.FearStartingBet(myDeterredCall,statworse.repeated,geom_algb_scaler);


    const float64 certainty = myFearControl.ActOrReact(betToCall,myBet,maxShowdown);

    const float64 uncertainty = fabs( statranking.pct - statmean.pct );
    const float64 timeLeft = (  detailPCT.stdDev*detailPCT.stdDev + uncertainty*uncertainty  );
    const float64 nonvolatilityFactor = 1 - timeLeft;

	const float64 nearEndOfBets = nonvolatilityFactor*(1-certainty) + certainty;



    if( bGamble <= 1 ) //DangerBot, ComBot, not SpaceBot
    {
        //small insuranceDeterrent means more likely for opponent to fold vs. call
        myDeterredCall.SetImpliedFactor( 1 / nearEndOfBets );
    }

    StatResult left = statversus;
	if( bGamble == 0 ) left = hybridMagnified;

    GainModel geomModel(left,myDeterredCall);

    StatResult right = statworse;
    right.repeated = 1-certainty;

    left.repeated = certainty;

#ifdef MODEL_REACTION
	GainModelNoRisk algbModel(left,right,myDeterredCall);
#else
	GainModelNoRisk algbModel(right,myDeterredCall);
#endif // MODEL_REACTION, with #else


#ifdef LOGPOSITION
    if( bGamble == 0 )
    {
        logFile << " -  Conservative  - " << endl;
    }
    else if( bGamble == 2 )
    {
        logFile << " -  SpaceRank  - " << endl;
    }
    else
    {
        logFile << " -  Danger  - " << endl;
	}

	if( bGamble <= 1 )
	{
        logFile << "timeLeft      " << timeLeft << endl;
        logFile << "  uncertainty      " << uncertainty << endl;
        logFile << "  detailPCT.stdDev " << detailPCT.stdDev << endl;
        logFile << "nearEndOfBets         " << nearEndOfBets << endl;
        logFile << "impliedFactor... " << 1 / nearEndOfBets << endl;
    }
    logFile << "BetToCall " << certainty << ", pct " << left.pct << " ... " << algbModel.ViewShape().pct << " ... " << right.pct << endl;

#endif

    ///Choose from geom to algb
	AutoScalingFunction<GainModel,GainModelNoRisk> hybridgainDeterred(geomModel,algbModel,min_worst_scaler,geom_algb_scaler,&tablestate);


    StateModel<  GainModel, GainModelNoRisk >
            ap_aggressive( myDeterredCall, &hybridgainDeterred );



    HoldemFunctionModel& choicemodel = ap_aggressive;


////DEB UG
/*
if( ViewTable().FutureRounds() < 2 )
{
//    const float64 z1 = ap_passive.f(60);


//    const float64 a1 = ap_passive.f_raised(60,90);
//    const float64 b1 = ap_passive.f_raised(60,150);
    //const float64 z11 = hybridgainDeterred.f(60);
    //const float64 a11 = hybridgainDeterred.f_raised(60,90);
    //const float64 b11 = hybridgainDeterred.f_raised(60,150);
    const float64 o111e = myDeterredCall.exf(30);
    const float64 z111e = myDeterredCall.exf(60);
    const float64 a111e = myDeterredCall.exf(90);
    const float64 b111e = myDeterredCall.exf(150);
    const float64 o111 = geomModel.f(30);
    const float64 z111 = geomModel.f(60);
    const float64 a111 = geomModel.f(90);
    const float64 b111 = geomModel.f(150);
    //const float64 z112 = algbModel.f(60);
    //const float64 a112 = algbModel.f(90);
    //const float64 b112 = algbModel.f(150);
    const float64 z12 = hybridgain.f(60);
    const float64 a12 = hybridgain.f_raised(60,90);
    const float64 b12 = hybridgain.f_raised(60,150);

	if( betToCall > 100 && bGamble )
    {
        const float64 z12 = hybridgain.f(betToCall); //You're in the blind, so folding is -2.25 chips. Then what?
		const float64 ff = myDeterredCall.foldGain();
        //const float64 a12 = hybridgain.f_raised(60,90);
        //const float64 b12 = hybridgain.f_raised(60,150);
    }


    logFile << endl << "1.62316" << endl;
	logFile << myDeterredCall.pWin(1.62316) << endl;
    logFile << choicemodel.f(1.62316) << endl;

    choicemodel.bTraceEnable = true;
    //const float64 bestBet = solveGainModel(&choicemodel);
    //logFile << bestBet << endl;
    logFile << choicemodel.f(1);
    std::cout << " AgainstCall("<< 1 <<")=" << ap_aggressive.gainNormal << endl;
	std::cout << "AgainstRaise("<< 1 <<")=" << ap_aggressive.gainRaised << endl;
	std::cout << "        Push("<< 1 <<")=" << ap_aggressive.gainWithFold << endl;

    algbModel.bTraceEnable = true;
    logFile << choicemodel.f(26.25);
    exit(1);
}
*/

    const float64 bestBet = solveGainModel(&choicemodel, &callcumu);

#ifdef LOGPOSITION


#ifdef VERBOSE_STATEMODEL_INTERFACE
const float64 displaybet = (bestBet < betToCall) ? betToCall : bestBet;
		choicemodel.f(displaybet); //since choicemodel is ap_aggressive
		logFile << " AgainstCall("<< displaybet <<")=" << ap_aggressive.gainNormal << endl;
		logFile << "AgainstRaise("<< displaybet <<")=" << ap_aggressive.gainRaised << endl;
		logFile << "        Push("<< displaybet <<")=" << ap_aggressive.gainWithFold << endl;

#endif

    //if( bestBet < betToCall + ViewTable().GetChipDenom() )
    {
        logFile << "\"riskprice\"... " << riskprice << "(" << geom_algb_scaler << ")" << endl;
        logFile << "When betting " << min_worst_scaler << ", oppFoldChance is first " << statworse.repeated << endl;

        logFile << "Geom("<< displaybet <<")=" << geomModel.f(displaybet) << endl;
        logFile << "Algb("<< displaybet <<")=" << algbModel.f(displaybet) << endl;
    }


        printBetGradient< StateModel<  GainModel, GainModelNoRisk > >
            (myDeterredCall, myDeterredCall, ap_aggressive, tablestate, displaybet);


    logFile << "Guaranteed > $" << tablestate.stagnantPot() << " is in the pot for sure" << endl;

        logFile << "OppFoldChance% ...    " << myDeterredCall.pWin(displaybet) << "   d\\" << myDeterredCall.pWinD(displaybet) << endl;
        if( myDeterredCall.pWin(displaybet) > 0 )
        {
            logFile << "confirm " << choicemodel.f(displaybet) << endl;
        }

#endif

    return bestBet;


}






