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


//#define DEBUG_TRAP_AS_NORMAL

#define RISKPRICE

#ifdef RISKPRICE
//#define ACTREACTUSESIN riskprice
#define ACTREACTUSESIN maxShowdown

//Okay, so riskprice is how you control attitude, e.g. geom to algb, or trap_left to trap_right
//but ACTREACTUSES is how you control fear.


#define ACTREACTUSES_RA (riskprice)
#define ACTREACTUSES_HD riskprice
//#define ACTREACTUSES_HD maxShowdown
#define DELAYENEMYOPP 0.0
//#define DELAYENEMYOPP riskprice
#else
#define ACTREACTUSES maxShowdown
#endif










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
        foldcumu.ReversePerspective();
    }
    //else{

    //}
    ViewTable().CachedQueryOffense(callcumu,withCommunity);
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
    if( statranking.pct > statrelation.pct )
    {
        statversus = statranking;
    }else
    {
        statversus = statrelation;
    }

    hybridMagnified.wins = sqrt(statmean.wins*statversus.wins);
    hybridMagnified.splits = sqrt(statmean.splits*statversus.splits);
    hybridMagnified.loss = sqrt(statmean.loss*statversus.loss);
    hybridMagnified.genPCT();
    const float64 adjust = hybridMagnified.wins + hybridMagnified.splits + hybridMagnified.loss;
    hybridMagnified = hybridMagnified * ( 1.0 / adjust );
    hybridMagnified.repeated = 0; ///.repeated WILL otherwise ACCUMULATE!

//Pick the safer one for play
    if( statranking.pct < statrelation.pct )
    {
        statversus = statranking;
    }else
    {
        statversus = statrelation;
    }



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
        logFile << "(Versus) " << statrelation.pct * 100 << "%"  << std::endl;
        logFile << "(V.s) " << statrelation.splits * 100 << "%"  << std::endl;
        logFile << "(Outright) " << statranking.pct * 100 << "%"  << std::endl;
        logFile << "(O.s) " << statranking.splits * 100 << "%"  << std::endl;
    }
    if(bLogHybrid)
    {
		if( !bLogMean ) logFile << "(Mean) " << statmean.pct * 100 << "%"  << std::endl;
		if( !bLogRanking ) logFile << "(Versus) " << statversus.pct * 100 << "%"  << std::endl;
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

}

float64 PositionalStrategy::solveGainModel(HoldemFunctionModel* targetModel, CallCumulationD* const e)
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
            float64 xw;
            float64 foldgainVal = (targetModel->GetFoldGain(e, &xw));


            //logFile << "selected risk  " << (choicePoint - myBet)/(maxShowdown - myBet) << endl;

            logFile << "Choice Optimal " << choicePoint << endl;
            logFile << "Choice Fold " << choiceFold << endl;
			logFile << "FoldGain()=" << foldgainVal;
			logFile << " x " << xw << "\tvs play:" << (raiseGain + foldgainVal) << endl;
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
    ExpectedCallD   tablestate(myPositionIndex,  &(ViewTable()), statranking.pct, statmean.pct);
    ExactCallBluffD myDeterredCall(&tablestate, &choicecumu, &raisecumu);
    ExactCallBluffD myDeterredCall_left(&tablestate, &choicecumu, &raisecumu);
    ExactCallBluffD myDeterredCall_right(&tablestate, &choicecumu, &raisecumu);
#else
    ExactCallBluffD myDeterredCall(myPositionIndex, &(ViewTable()), &choicecumu, &raisecumu);
#endif



#ifdef RISKPRICE
    float64 riskprice = myDeterredCall.RiskPrice();
#else
    float64 riskprice = maxShowdown;
#endif


    const float64 fullVersus = myDeterredCall.callingPlayers();
    const float64 peopleDrawing = (1 - improvePure) * (ViewTable().NumberInHand() - 1);//You probably don't have to beat the people who folded, especially if you are going to improve your hand
    const float64 newVersus = (fullVersus - peopleDrawing*(1-improvePure)*detailPCT.stdDev);


//bGamble == 2 is ActionBot
//bGamble == 1 is TrapBot


    const float64 actOrReact = myDeterredCall.ActOrReact(betToCall,myBet,ACTREACTUSESIN);

//NormalBot uses this setup.
    StatResult left = statversus;//hybridMagnified;
    StatResult base_right = statmean;

//TrapBot and ActionBot are based on statversus only
    if( bGamble >= 1 )
    {
        const float64 enemyChances = 0.5;//(ViewTable().GetNumberInHand() - 1.0) / ViewTable().GetNumberInHand() / 2;
        left = statversus;
        #ifndef DEBUG_TRAP_AS_NORMAL
        //targetWorsenBy is here
        left.wins -= detailPCT.avgDev*enemyChances;
        left.loss += detailPCT.avgDev*enemyChances;
        left.pct -= detailPCT.avgDev*enemyChances;
        //Since detailPCT is based on statmean, not statversus, it is possible for zero crossings
        if( left.pct < 0 || left.wins < 0 )
        {
            left.wins = 0;
            left.loss = 1 - left.splits;
            left.genPCT();
        }
        #endif
        //Need scaling
        myDeterredCall_right.insuranceDeterrent = oppInsuranceBigBet;


        base_right = statversus;


        if( bGamble >= 2 )
        {
            myDeterredCall_left.SetImpliedFactor(impliedOddsGain);

//Need scaling
            myDeterredCall_right.callingPlayers(newVersus);
        }
    }

    StatResult right = statworse;
    right.repeated = (1 - actOrReact);//Generally ignored, only base_right.repeated is really used
    base_right.repeated = actOrReact;


    left.repeated = 1;
	GainModel geomModel(left,right,myDeterredCall_left);
	GainModel geomModel_fear(base_right,right,myDeterredCall_left);
	statversus.repeated = 1;
	GainModelNoRisk algbModel(statversus,right,myDeterredCall_right);
	GainModelNoRisk algbModel_fear(base_right,right,myDeterredCall_right);



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
    logFile << " Act or React? React " << (actOrReact * 100) << "% --> pct of " << base_right.pct << " ... " << algbModel_fear.ViewShape().pct << " ... " << statworse.pct << endl;
#endif

///From geom to algb
	AutoScalingFunction<GainModel,GainModelNoRisk> hybridgainDeterred_aggressive(geomModel,algbModel,0.0,riskprice,left.pct*base_right.pct,&tablestate);
	AutoScalingFunction<GainModel,GainModelNoRisk> hybridgain_aggressive(geomModel_fear,algbModel_fear,0.0,riskprice,left.pct*base_right.pct,&tablestate);


///From regular to fear (x2)
	AutoScalingFunction<  AutoScalingFunction<GainModel,GainModelNoRisk>  ,  AutoScalingFunction<GainModel,GainModelNoRisk>  >
            ap(hybridgainDeterred_aggressive,hybridgain_aggressive,DELAYENEMYOPP,ACTREACTUSES_RA,&tablestate);

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
	AutoScalingFunction<  AutoScalingFunction<GainModel,GainModelNoRisk>  ,  AutoScalingFunction<GainModel,GainModelNoRisk>  >
            ap_right(hybridgainDeterred_aggressive,hybridgain_aggressive,DELAYENEMYOPP,ACTREACTUSES_RA,&tablestate);
    StateModel<  AutoScalingFunction<GainModel,GainModelNoRisk>  ,  AutoScalingFunction<GainModel,GainModelNoRisk>  >
            choicemodel_right( myDeterredCall_right, &ap_right );



    AutoScalingFunction<   StateModel<  AutoScalingFunction<GainModel,GainModelNoRisk>  ,  AutoScalingFunction<GainModel,GainModelNoRisk>  >
                         , StateModel<  AutoScalingFunction<GainModel,GainModelNoRisk>  ,  AutoScalingFunction<GainModel,GainModelNoRisk>  >
                       >
            rolemodel(choicemodel,choicemodel_right,betToCall,riskprice,&tablestate);




//DEBUG //
/*
 //   if( bGamble == 1 && ViewTable().GetPrevPotSize() > 3.0 )
    if( bGamble == 0 )
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


		const float64 ay1 = ap.f(0.44);
		const float64 ady1 = ap.fd(0.44,ay1);

		const float64 by1 = hybridgainDeterred_aggressive.f(0.44);
		const float64 bdy1 = hybridgainDeterred_aggressive.fd(0.44,by1);

		const float64 cy1 = hybridgain_aggressive.f(0.44);
		const float64 cdy1 = hybridgain_aggressive.fd(0.44,by1);


		//std::cout << ay1 << "   <-- ap" << endl;
		std::cout << ady1 << "   <-- ap" << endl;
		//std::cout << by1 << "   <-- lowbet" << endl;
		std::cout << bdy1 << "   <-- lowbet" << endl;
		//std::cout << cy1 << "   <-- fearbet" << endl;
		std::cout << cdy1 << "   <-- fearbet" << endl;


        #ifdef DEBUG_TRACE_PWIN
            myDeterredCall.traceOut = &logFile;
            myDeterredCall_left.traceOut = &logFile;
            //myDeterredCall_right.traceOut = &logfile;
		#endif
        float64 rAmount =  myDeterredCall.RaiseAmount(0.25,3);
        logFile << myDeterredCall.pRaise(0.25,3) << " @ $" << rAmount;
        logFile << "\tfold -- left" << myDeterredCall_left.pWin(rAmount) << "  " << myDeterredCall_right.pWin(rAmount) << " right" << endl;

        exit(1);

		if(betToCall > 0.9 || bestBet > 0.3)
		{


        const float64 b21 = geomModel_fear.f(.9500001);
        const float64 b22 = algbModel_fear.f(.9500001);
        const float64 b11 = geomModel.f(.9500001);
        const float64 b12 = algbModel.f(.9500001);

        const float64 c21 = hybridgain_aggressive.left.f(.9500001);
        const float64 c22 = hybridgain_aggressive.right.f(.9500001);
        const float64 c11 = hybridgainDeterred_aggressive.left.f(.9500001);
        const float64 c12 = hybridgainDeterred_aggressive.right.f(.9500001);

        const float64 b1 = hybridgainDeterred_aggressive.f(.9500001);
        const float64 b2 = hybridgain_aggressive.f(.9500001);

        const float64 a0 = ap.f(.9500001);
        //const float64 a1 = ap.f(.5);
        const float64 a2 = ap_right.f(.9500001);

        //const float64 theyfold = myDeterredCall_left.pWin( .95 );
        //const float64 z = 0;




        //HEY! NormBot only uses a0 anyways, and z0/z2 don't apply
		logFile << a0 << " " << " " << a2 << " <--- " << (ap.bLeft ? "N/A " : "Auto") << "Scaling from (" << DELAYENEMYOPP << "," << ACTREACTUSES_RA << "]" << endl;
        logFile << b1 << " " << b2  << " <--- Should be between these two values" << endl;
        logFile << c11 << " " << c12 << " <-- which should be between --> " << c21 << " " << c22 << endl;
        logFile << b11 << " " << b12 << " <-- which equals --> " << b21 << " " << b22 << endl;





			logFile << bestBet << endl;
			const float64 z0 = choicemodel.f(.9500001);
			const float64 z2 = choicemodel_right.f(.9500001);
            logFile << z0 << "  " << z2 << " State of autoscale." << endl;

			std::cerr << "DEBUG QUIT" << endl;
			exit(0);
		}



hybridgainDeterred_aggressive.bTraceEnable = true;



		const float64 ey1 = geomModel.f(0.45);
		const float64 edy1 = geomModel.fd(0.45,ey1);
		const float64 jy1 = algbModel.f(0.45);
		const float64 jdy1 = algbModel.fd(0.45,jy1);


		std::cout << ey1 << "   <-- geomlow" << endl;
		std::cout << edy1 << "   <-- geomlow" << endl;
		std::cout << jy1 << "   <-- algblow" << endl;
		std::cout << jdy1 << "   <-- algblow" << endl;

		exit(1);
	}

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
    //if( bestBet < betToCall + ViewTable().GetChipDenom() )
    {
        logFile << "\"riskprice\"... " << riskprice << endl;

#ifdef VERBOSE_STATEMODEL_INTERFACE
		choicemodel.f(bestBet);
		logFile << "        Play("<< bestBet <<")=" << choicemodel.gainNormal << endl;
		logFile << "AgainstRaise("<< bestBet <<")=" << choicemodel.gainRaised << endl;
		logFile << "        Push("<< bestBet <<")=" << choicemodel.gainWithFold << endl;

		if( bGamble != 0 )
		{
			choicemodel_right.f(bestBet);
			logFile << "        Play OtherDeter("<< bestBet <<")=" << choicemodel_right.gainNormal << endl;
			logFile << "AgainstRaise OtherDeter("<< bestBet <<")=" << choicemodel_right.gainRaised << endl;
			logFile << "        Push OtherDeter("<< bestBet <<")=" << choicemodel_right.gainWithFold << endl;
		}
#endif
        logFile << "Call Regular("<< bestBet <<")=" << hybridgainDeterred_aggressive.f(bestBet) << endl;
        logFile << "   Call Fear("<< bestBet <<")=" << hybridgain_aggressive.f(bestBet) << endl;

    }


	if( bestBet >= betToCall - ViewTable().GetChipDenom() )
	{
		int32 maxcallStep = -1;
	    int32 raiseStep = 0;
        float64 rAmount =  myDeterredCall.RaiseAmount(betToCall,raiseStep);
        while( rAmount < bestBet )
        {
            rAmount =  myDeterredCall.RaiseAmount(betToCall,raiseStep);
            const float64 oppRaisedFoldGain = myDeterredCall_left.FoldGain(betToCall - tablestate.alreadyBet(),rAmount);
            logFile << "OppRAISEChance";
            if( oppRaisedFoldGain > choicemodel.g_raised(betToCall,rAmount) ){  logFile << " [F] ";  } else {  logFile << " [*] ";  maxcallStep = raiseStep+1; }

            logFile << myDeterredCall.pRaise(betToCall,raiseStep,maxcallStep) << " @ $" << rAmount;
            logFile << "\tfold -- left" << myDeterredCall_left.pWin(rAmount) << "  " << myDeterredCall_right.pWin(rAmount) << " right" << endl;


            if( rAmount >= maxShowdown ) break;

            ++raiseStep;
        }

		logFile << "\t--" << endl;

		maxcallStep = -1;
		raiseStep = 0;
        rAmount =  myDeterredCall.RaiseAmount(bestBet,raiseStep);
        while( rAmount <= maxShowdown )
        {
            rAmount =  myDeterredCall.RaiseAmount(bestBet,raiseStep);
            const float64 oppRaisedFoldGain = myDeterredCall_left.FoldGain(bestBet - tablestate.alreadyBet(),rAmount);
            logFile << "OppRAISEChance";
            if( oppRaisedFoldGain > choicemodel.g_raised(bestBet,rAmount) ){  logFile << " [F] ";  } else {  logFile << " [*] ";  maxcallStep = raiseStep+1; }

            logFile << myDeterredCall.pRaise(bestBet,raiseStep,maxcallStep) << " @ $" << rAmount;
            logFile << "\tfold -- left" << myDeterredCall_left.pWin(rAmount) << "  " << myDeterredCall_right.pWin(rAmount) << " right" << endl;


            if( rAmount >= maxShowdown ) break;

            ++raiseStep;
        }
	}

        logFile << "Guaranteed $" << tablestate.stagnantPot() << endl;
        logFile << "OppFoldChance% ... left " << myDeterredCall_left.pWin(bestBet) << " --" << myDeterredCall_right.pWin(bestBet) << " right" << endl;
        if( myDeterredCall.pWin(bestBet) > 0 )
        {
            logFile << "confirm Normal " << choicemodel.f(bestBet) << endl;
            logFile << "confirm " << rolemodel.f(bestBet) << endl;
        }

#endif

    return bestBet;


}


float64 DeterredGainStrategy::MakeBet()
{
	setupPosition();

	if( maxShowdown <= 0 ) return 0;



    CallCumulationD &choicecumu = callcumu;
    CallCumulationD &raisecumu = foldcumu;


#ifdef ANTI_PRESSURE_FOLDGAIN
    ExpectedCallD   tablestate(myPositionIndex,  &(ViewTable()), statranking.pct, statmean.pct);
    ExactCallBluffD myDeterredCall(&tablestate, &choicecumu, &raisecumu);
#else
    //ExactCallD myExpectedCall(myPositionIndex, &(ViewTable()), &choicecumu);
    ExactCallBluffD myDeterredCall(myPositionIndex, &(ViewTable()), &choicecumu, &raisecumu);
#endif



#ifdef RISKPRICE
    float64 riskprice = myDeterredCall.RiskPrice();
#else
    float64 riskprice = maxShowdown;
#endif



    const float64 certainty = myDeterredCall.ActOrReact(betToCall,myBet,ACTREACTUSESIN);
    //const float64 certainty = (betToCall > maxShowdown) ? 1 : (betToCall / maxShowdown);
    const float64 uncertainty = fabs( statranking.pct - statmean.pct );
    const float64 timeLeft = (  detailPCT.stdDev*detailPCT.stdDev + uncertainty*uncertainty  );
    const float64 nonvolatilityFactor = 1 - timeLeft;

	const float64 nearEndOfBets = nonvolatilityFactor*(1-certainty) + certainty;



    if( bGamble < 1 ) //ComBot only, not DangerBot
    {
        //small insuranceDeterrent means more likely for opponent to fold vs. call
        myDeterredCall.SetImpliedFactor( 1 / nearEndOfBets );
        //myDeterredCall.insuranceDeterrent = ;
    }

    StatResult left = statmean;
    left.repeated = certainty;
    StatResult right = statworse;
    right.repeated = 1-certainty;

    statversus.repeated = 1;
    hybridMagnified.repeated = 1;

	GainModel geomModel(hybridMagnified,right,myDeterredCall);
	GainModel geomModel_fear(left,right,myDeterredCall);
	GainModelNoRisk algbModel(hybridMagnified,right,myDeterredCall);
	GainModelNoRisk algbModel_fear(left,right,myDeterredCall);


#ifdef LOGPOSITION
    if( bGamble == 0 )
    {
        logFile << " -  Conservative  - " << endl;
        logFile << "timeLeft      " << timeLeft << endl;
        logFile << "  uncertainty      " << uncertainty << endl;
        logFile << "  detailPCT.stdDev " << detailPCT.stdDev << endl;
        logFile << "nearEndOfBets         " << nearEndOfBets << endl;
        logFile << "impliedFactor... " << 1 / nearEndOfBets << endl;
    }else
    {
        logFile << " -  Danger  - " << endl;
    }
    logFile << "BetToCall " << certainty << ", pct " << statmean.pct << " ... " << algbModel_fear.ViewShape().pct << " ... " << statworse.pct << endl;

#endif

    ///Choose from geom to algb
	AutoScalingFunction<GainModel,GainModelNoRisk> hybridgainDeterred(geomModel,algbModel,0.0,riskprice,hybridMagnified.pct*statmean.pct,&tablestate);
	AutoScalingFunction<GainModel,GainModelNoRisk> hybridgain(geomModel_fear,algbModel_fear,0.0,riskprice,hybridMagnified.pct*statmean.pct,&tablestate);

    ///Choose from regular to fear (using raisefrom)
	AutoScalingFunction<  AutoScalingFunction<GainModel,GainModelNoRisk>
                        , AutoScalingFunction<GainModel,GainModelNoRisk>
                       >
            ap_passive(hybridgainDeterred,hybridgain,DELAYENEMYOPP,ACTREACTUSES_HD,&tablestate);



    StateModel<  AutoScalingFunction<GainModel,GainModelNoRisk>
                        , AutoScalingFunction<GainModel,GainModelNoRisk>
                       >
            ap_aggressive( myDeterredCall, &ap_passive );



    HoldemFunctionModel& choicemodel = ap_aggressive;


////DEB UG
/*
if( ViewTable().NumberInHand() < ViewTable().NumberAtTable() )
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
    std::cout << "        Play("<< 1 <<")=" << ap_aggressive.gainNormal << endl;
	std::cout << "AgainstRaise("<< 1 <<")=" << ap_aggressive.gainRaised << endl;
	std::cout << "        Push("<< 1 <<")=" << ap_aggressive.gainWithFold << endl;
    exit(1);
}
*/

    const float64 bestBet = solveGainModel(&choicemodel, &callcumu);

#ifdef LOGPOSITION


#ifdef VERBOSE_STATEMODEL_INTERFACE
const float64 displaybet = (bestBet < betToCall) ? betToCall : bestBet;
		choicemodel.f(displaybet); //since choicemodel is ap_aggressive
		logFile << "        Play("<< displaybet <<")=" << ap_aggressive.gainNormal << endl;
		logFile << "AgainstRaise("<< displaybet <<")=" << ap_aggressive.gainRaised << endl;
		logFile << "        Push("<< displaybet <<")=" << ap_aggressive.gainWithFold << endl;

#endif

    //if( bestBet < betToCall + ViewTable().GetChipDenom() )
    {
        logFile << "\"riskprice\"... " << riskprice << endl;
        logFile << "Geom("<< displaybet <<")=" << hybridgainDeterred.f(displaybet) << endl;
        logFile << "Algb("<< displaybet <<")=" << hybridgain.f(displaybet) << endl;
    }


	if( bestBet >= betToCall - ViewTable().GetChipDenom() )
	{
		int32 maxcallStep = -1;
	    int32 raiseStep = 0;
        float64 rAmount =  myDeterredCall.RaiseAmount(betToCall,raiseStep);
        while( rAmount < bestBet )
        {
            rAmount =  myDeterredCall.RaiseAmount(betToCall,raiseStep);
            const float64 oppRaisedFoldGain = myDeterredCall.FoldGain(betToCall - tablestate.alreadyBet(),rAmount);
            logFile << "OppRAISEChance";
            if( oppRaisedFoldGain > ap_aggressive.g_raised(betToCall,rAmount) ){ logFile << " [F] "; }else { logFile << " [*] "; maxcallStep = raiseStep+1; }
            logFile << myDeterredCall.pRaise(betToCall,raiseStep,maxcallStep) << " @ $" << rAmount;
            logFile << "\tBetWouldFold%" << myDeterredCall.pWin(rAmount) << endl;


            if( rAmount >= maxShowdown ) break;

            ++raiseStep;
        }

		logFile << "\t--" << endl;

		maxcallStep = -1;
		raiseStep = 0;
        rAmount =  myDeterredCall.RaiseAmount(bestBet,raiseStep);
        while( rAmount <= maxShowdown )
        {
            rAmount =  myDeterredCall.RaiseAmount(bestBet,raiseStep);
            const float64 oppRaisedFoldGain = myDeterredCall.FoldGain(bestBet - tablestate.alreadyBet(),rAmount);
            logFile << "OppRAISEChance";
            if( oppRaisedFoldGain > ap_aggressive.g_raised(bestBet,rAmount) ){ logFile << " [F] "; }else { logFile << " [*] "; maxcallStep = raiseStep+1; }
            ///ASSUMPTION: ap_aggressive is choicemodel!

            logFile << myDeterredCall.pRaise(bestBet,raiseStep,maxcallStep) << " @ $" << rAmount;
            logFile << "\tBetWouldFold%" << myDeterredCall.pWin(rAmount) << endl;

            if( rAmount >= maxShowdown ) break;

            ++raiseStep;
        }
	}
    logFile << "Guaranteed $" << tablestate.stagnantPot() << endl;

        logFile << "OppFoldChance% ...    " << myDeterredCall.pWin(bestBet) << "   d\\" << myDeterredCall.pWinD(bestBet) << endl;
        if( myDeterredCall.pWin(bestBet) > 0 )
        {
            logFile << "confirm " << choicemodel.f(bestBet) << endl;
        }

#endif

    return bestBet;


}






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
    ExpectedCallD tablestate(myPositionIndex, &(ViewTable()),HANDRANK_MACRO statmean.pct);
    ExactCallBluffD myExpectedCall(&tablestate, &choicecumu, &raisecumu);//foldcumu);
    ExactCallBluffD myLimitCall(&tablestate, &choicecumu, &raisecumu);//foldcumu);

#ifdef ANTI_PRESSURE_FOLDGAIN
#undef HANDRANK_MACRO
#endif
    float64 raiseBattle = ViewTable().GetChipDenom();
    if( betToCall > raiseBattle )
    {
        raiseBattle = betToCall;
    }

#ifdef ANTI_PRESSURE_FOLDGAIN
	ExactCallD myVSCall(&tablestate, &callcumu);

#else
	ExactCallD myVSCall(myPositionIndex, &(ViewTable()), &callcumu);
#endif
    float64 expectedVS = ( myVSCall.exf(raiseBattle) - ViewTable().GetPotSize() + ViewTable().GetUnbetBlindsTotal() + ViewTable().GetRoundBetsTotal() ) /raiseBattle;
    if( expectedVS <= 0 ) //You have no money
    {
        expectedVS = ( tablestate.betFraction(ViewTable().GetChipDenom()) ) ;
    }

    myLimitCall.callingPlayers(  expectedVS  );



    GainModel rankGeom(statversus,myExpectedCall);
    GainModel hybridGeom(hybridMagnified,myExpectedCall);
    GainModelNoRisk hybridAlgb(hybridMagnified,myExpectedCall);
    //GainModelNoRisk choicegain_rnr(statversus,myExpectedCall);


    GainModel meanGeom(statmean,myExpectedCall);


    GainModelNoRisk worstAlgb(statworse,myLimitCall);
    GainModelNoRisk rankAlgb(statversus,myExpectedCall);
    GainModelNoRisk meanAlgb(statmean,myExpectedCall);


	StateModel<GainModel,GainModel> rankGeomBluff(myExpectedCall,rankGeom,rankGeom);
	StateModel<GainModel,GainModel> meanGeomBluff(myExpectedCall,meanGeom,meanGeom);
    StateModel<GainModelNoRisk,GainModelNoRisk> worstAlgbBluff(myLimitCall,worstAlgb,worstAlgb);
    StateModel<GainModelNoRisk,GainModelNoRisk> rankAlgbBluff(myExpectedCall,rankAlgb,rankAlgb);
    StateModel<GainModelNoRisk,GainModelNoRisk> meanAlgbBluff(myExpectedCall,meanAlgb,meanAlgb);
    StateModel<GainModel,GainModel> hybridGeomBluff(myExpectedCall,hybridGeom,hybridGeom);

    #ifdef DEBUGASSERT
        if( bGamble >= BGAMBLE_MAX )
        {
                std::cout << "Incorrect bGAMBLE" << endl;
                exit(1);
        }
    #endif


    HoldemFunctionModel* (lookup[BGAMBLE_MAX]) = { &rankGeom, &meanGeom, &worstAlgb, &rankAlgb, &meanAlgb, &rankGeom, &meanGeom, &hybridGeom, &hybridAlgb
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
            solveGainModel(lookup[otherGamble], &callcumu);
            logFile << "Main {" << (int)(bGamble) << "}" << endl;
        }

    #endif
    const float64 bestBet = solveGainModel(lookup[bGamble], &callcumu);


	#ifdef LOGPOSITION
		if( bGamble >= 9 && bGamble <= 15 )
		{
//			int32 maxcallStep = -1;
		    int32 raiseStep = 0;
            float64 rAmount =  myExpectedCall.RaiseAmount(bestBet,raiseStep);
            while( rAmount < maxShowdown )
            {
                rAmount =  myExpectedCall.RaiseAmount(bestBet,raiseStep);

				//if( oppRaisedFoldGain < lookup[bGamble]->g_raised(betToCall,rAmount) ){ logFile << " [*] "; maxcallStep = raiseStep+1; }

                logFile << "OppRAISEChance% ... ";
                //logFile << myExpectedCall.pRaise(bestBet,raiseStep);
                logFile << " @ $" << rAmount;
                logFile << "\tBetWouldFold%" << myExpectedCall.pWin(rAmount) << endl;
                ++raiseStep;
            }
            logFile << "Guaranteed $" << tablestate.stagnantPot() << endl;
			logFile << "OppFoldChance% ... " << myExpectedCall.pWin(bestBet) << "   d\\" << myExpectedCall.pWinD(bestBet) << endl;

			if( myExpectedCall.pWin(bestBet) > 0 )
			{
				logFile << "confirm " << lookup[bGamble]->f(bestBet) << endl;
			}
		}
	#endif

    return bestBet;


}
