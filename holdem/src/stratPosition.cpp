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


const bool CorePositionalStrategy::lkupLogMean[BGAMBLE_MAX] = {false,true,false,false,true,false,true,true,false ,false,true,false,false,true,true,false};
const bool CorePositionalStrategy::lkupLogRanking[BGAMBLE_MAX] = {true,false,false,true,false,true,false,true,false ,true,false,false,true,false,true,false};
const bool CorePositionalStrategy::lkupLogWorse[BGAMBLE_MAX] = {false,false,true,false,false,false,true,false,false ,false,false,true,false,false,false,false};
const bool CorePositionalStrategy::lkupLogHybrid[BGAMBLE_MAX] = {false,false,false,false,false,false,false,true,true ,false,false,false,false,false,true,true};

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

    //if(bLogWorse)
    {
        StatsManager::QueryDefense(foldcumu,withCommunity,onlyCommunity,cardsInCommunity);
    }//else{

    //}
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
            logFile << "Bet to call " << betToCall << " (from " << myBet << ")" << endl;
        #endif

    const float64 raiseBattle = betToCall ? betToCall : ViewTable().GetChipDenom();
    ExactCallD myExpectedCall(myPositionIndex, &(ViewTable()), &callcumu);
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
    const float64 callGain = targetModel->f(betToCall)
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
            logFile << "relBPHG()=" << (1-targetModel->GetFoldGain()) << " ." << endl;
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


float64 ImproveStrategy::MakeBet()
{
    setupPosition();


    const float64 improveMod = detailPCT.improve; //Generally preflop is negative here, so you probably don't want to accentuate that
    const float64 improvePure = (improveMod+1)/2;
    //const float64 improveDev = detailPCT.stdDev * (1-improvePure) + detailPCT.avgDev * improvePure;

    float64 distrScale = improvePure;
    //float64 distrScale = 0.5 ;
    //float64 distrScale = myMoney / ViewTable().GetAllChips() ;


    if( distrScale > 1 ) distrScale = 1;
    if( distrScale < 0 ) distrScale = 0;

    CallCumulationD &choicecumu = callcumu;

    ExactCallBluffD myExpectedCall(myPositionIndex, &(ViewTable()), &choicecumu,&choicecumu);
    ExactCallBluffD myLimitCall(myPositionIndex, &(ViewTable()), &choicecumu,&choicecumu);

    myLimitCall.callingPlayers(  expectedVS  );


    GainModel rankGeom_passive(statranking,&myExpectedCall);
    GainModelNoRisk worstAlgb_passive(statworse,&myLimitCall);

    GainModelBluff rankGeom_aggressive(statranking,&myExpectedCall);
    GainModelNoRiskBluff worstAlgb_aggressive(statworse,&myLimitCall);

    HoldemFunctionModel *(rankGeom_aggressiveness[2]) = {&rankGeom_passive, &rankGeom_aggressive };
    HoldemFunctionModel *(worstAlgb_aggressiveness[2]) = {&worstAlgb_passive, &worstAlgb_aggressive };

    SlidingPairFunction gp(worstAlgb_aggressiveness[bGamble],rankGeom_aggressiveness[bGamble],distrScale,&myExpectedCall);


    #ifdef LOGPOSITION
        logFile << "offense/defense =" << distrScale << endl;
    #endif

    const float64 bestBet = solveGainModel(&gp);

    #ifdef LOGPOSITION
    if( bGamble == 1 )
    {
        const float64 pWinWorst = myLimitCall.pWin(bestBet);
        const float64 pWinRank = myExpectedCall.pWin(bestBet);

        const float64 pWinWorstD = myLimitCall.pWinD(bestBet);
        const float64 pWinRankD = myExpectedCall.pWinD(bestBet);


        const float64 pWinSlider = pWinWorst*(1-distrScale) + pWinRank*distrScale;

        logFile << "OppFoldChance% ... " << pWinSlider << "   d\\" << pWinWorstD*(1-distrScale) + pWinRankD*distrScale << endl;
        if( pWinSlider > 0 )
        {
            logFile << "confirm " << gp.f(bestBet) << endl;
        }
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
    const float64 volatilityFactor = 1 - sqrt(  detailPCT.stdDev*detailPCT.stdDev + uncertainty*uncertainty  );

	const float64 futureFold = volatilityFactor*(1-certainty) + certainty;

#ifdef LOGPOSITION
    logFile << "uncertainty      " << uncertainty << endl;
    logFile << "detailPCT.stdDev " << detailPCT.stdDev << endl;
    logFile << "V Factor         " << volatilityFactor << endl;
    logFile << "BetToCall PCT    " << certainty << endl;
    logFile << "impliedFactor... " << futureFold << endl;
#endif


    CallCumulationD &choicecumu = callcumu;

    //ExactCallD myExpectedCall(myPositionIndex, &(ViewTable()), &choicecumu);
    ExactCallBluffD myDeterredCall(myPositionIndex, &(ViewTable()), &choicecumu, &choicecumu);
    myDeterredCall.SetImpliedFactor(futureFold);


	GainModel hybridgainDeterred_passive(hybridMagnified,&myDeterredCall);
	GainModelNoRisk hybridgain_passive(statmean,&myDeterredCall);
	GainModelBluff hybridgainDeterred_aggressive(hybridMagnified,&myDeterredCall);
	GainModelNoRiskBluff hybridgain_aggressive(statmean,&myDeterredCall);

    HoldemFunctionModel *(hybridgainDeterred_agressiveness[2]) = {&hybridgainDeterred_passive, &hybridgainDeterred_aggressive };
    HoldemFunctionModel *(hybridgain_agressiveness[2]) = {&hybridgain_passive, &hybridgain_aggressive };

	AutoScalingFunction choicemodel(hybridgainDeterred_agressiveness[bGamble],hybridgain_agressiveness[bGamble],0.0,maxShowdown,hybridMagnified.pct*statmean.pct,&myDeterredCall);


    const float64 bestBet = solveGainModel(&choicemodel);

#ifdef LOGPOSITION
	logFile << "\"shuftip\"... " << hybridMagnified.pct*statmean.pct << endl;
	logFile << "Geom("<< bestBet <<")=" << hybridgainDeterred_agressiveness[bGamble]->f(bestBet) << endl;
	logFile << "Algb("<< bestBet <<")=" << hybridgain_agressiveness[bGamble]->f(bestBet) << endl;

    if( bGamble == 1 )
    {
        logFile << "OppFoldChance% ... " << myDeterredCall.pWin(bestBet) << "   d\\" << myDeterredCall.pWinD(bestBet) << endl;
        if( myDeterredCall.pWin(bestBet) > 0 )
        {
            logFile << "confirm " << choicemodel.f(bestBet) << endl;
        }
    }
#endif

    return bestBet;


}







float64 HybridScalingStrategy::MakeBet()
{
	setupPosition();

	const float64 oppGift = (ViewTable().GetPotSize() - betToCall) / (ViewTable().GetNumberInHand()-1) ;
	const float64 oppGiftMax = ViewTable().GetAllChips()/(ViewTable().GetNumberInHand()-1);

	const float64 shiftscale2 = myMoney / ( myMoney + oppGift ) - myMoney / ( myMoney + oppGiftMax );

    const float64 eVSshift = expectedVS / (ViewTable().GetNumberAtTable()-1) ;

    const float64 myRevealed =  ViewPlayer().GetContribution() + myBet;
	const float64 estimateRevealed = (ViewTable().GetUnfoldedPotSize() - myRevealed) / ViewTable().GetAllChips() ;


    ExactCallBluffD myExpectedCall(myPositionIndex, &(ViewTable()), &callcumu, &callcumu);
    myExpectedCall.callingPlayers( (ViewTable().GetNumberAtTable()-1) / (1-estimateRevealed) );


#ifdef LOGPOSITION
    logFile << "suggested strength of field : " << 1/(1-estimateRevealed) << endl;
	logFile << "difficulty of field : " << eVSshift << endl;
	logFile << "oppGift   { " << oppGift << " }" << endl;
	logFile << "shiftscale{ " << shiftscale2 << " }," << shiftscale2*shiftscale2 << endl;
#endif



	GainModel highPlayers_passive(statmean,&myExpectedCall);
	GainModel lowPlayers_passive(statranking,&myExpectedCall);

	GainModelNoRisk highPlayersN_passive(statmean,&myExpectedCall);
	GainModelNoRisk lowPlayersN_passive(statranking,&myExpectedCall);


	GainModelBluff highPlayers_aggressive(statmean,&myExpectedCall);
	GainModelBluff lowPlayers_aggressive(statranking,&myExpectedCall);

	GainModelNoRiskBluff highPlayersN_aggressive(statmean,&myExpectedCall);
	GainModelNoRiskBluff lowPlayersN_aggressive(statranking,&myExpectedCall);

    HoldemFunctionModel *(lowPlayers_agressiveness[2]) = {&lowPlayers_passive, &lowPlayers_aggressive };
    HoldemFunctionModel *(highPlayers_agressiveness[2]) = {&highPlayers_passive, &highPlayers_aggressive };
    HoldemFunctionModel *(lowPlayersN_agressiveness[2]) = {&lowPlayersN_passive, &lowPlayersN_aggressive };
    HoldemFunctionModel *(highPlayersN_agressiveness[2]) = {&highPlayersN_passive, &highPlayersN_aggressive };


	SlidingPairFunction choicemodelP(lowPlayers_agressiveness[bGamble],highPlayers_agressiveness[bGamble],eVSshift,&myExpectedCall);
	SlidingPairFunction choicemodelN(lowPlayersN_agressiveness[bGamble],highPlayersN_agressiveness[bGamble],eVSshift,&myExpectedCall);

	AutoScalingFunction allModel(&choicemodelP,&choicemodelN,0.0,maxShowdown,&myExpectedCall);





    const float64 bestBet = solveGainModel(&allModel);


	const float64 ssunits = myExpectedCall.betFraction(  myExpectedCall.exf(bestBet)  );
	const float64 gainPC = allModel.f(bestBet) - shiftscale2*shiftscale2*ssunits ;

#ifdef LOGPOSITION
    if( bGamble == 1 )
    {
        logFile << "OppFoldChance% ... " << myExpectedCall.pWin(bestBet) << "   d\\" << myExpectedCall.pWinD(bestBet) << endl;
		if( myExpectedCall.pWin(bestBet) >= 1 )
		{
			std::cout << "Examine myExpectedCall.pWin(bestBet)" << endl;
		}
    }


	logFile << "choicemodel("<< bestBet <<")=" << allModel.f(bestBet) << "  -->  " << gainPC << endl;
	logFile << "                   (* " << ssunits << ")²" << endl;
#endif

	if( bestBet > 0 )
	{
		logFile << "ALTERNATE?" << endl;

		if( gainPC < 0 )
		{
			#ifdef LOGPOSITION
				logFile << "A-CHECK/FOLD " << myBet << endl;
			#endif
			return myBet;
		}
	}

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
	ExactCallBluffD myBluffFoldCall(myPositionIndex, &(ViewTable()), &choicecumu, &choicecumu);//foldcumu);
    ExactCallD myLimitCall(myPositionIndex, &(ViewTable()), &choicecumu);
    ExactCallBluffD myLimitFoldCall(myPositionIndex, &(ViewTable()), &choicecumu, &choicecumu);//foldcumu);
    const float64 committed = ViewPlayer().GetContribution() + myBet;
    ExactCallD myCommittalCall(myPositionIndex, &(ViewTable()), &choicecumu, committed );

    float64 raiseBattle = ViewTable().GetChipDenom();
    if( betToCall > raiseBattle )
    {
        raiseBattle = betToCall;
    }

    myLimitCall.callingPlayers(  expectedVS  );
    myLimitFoldCall.callingPlayers(  expectedVS  );


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


	GainModelBluff rankGeomBluff(statranking,&myBluffFoldCall);
	GainModelBluff meanGeomBluff(statmean,&myBluffFoldCall);
    GainModelNoRiskBluff worstAlgbBluff(statworse,&myLimitFoldCall);
    GainModelNoRiskBluff rankAlgbBluff(statranking,&myBluffFoldCall);
    GainModelNoRiskBluff meanAlgbBluff(statmean,&myBluffFoldCall);
    GainModelBluff hybridGeomBluff(hybridMagnified,&myBluffFoldCall);

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
			logFile << "OppFoldChance% ... " << myBluffFoldCall.pWin(bestBet) << "   d\\" << myBluffFoldCall.pWinD(bestBet) << endl;
			if( myBluffFoldCall.pWin(bestBet) > 0 )
			{
				logFile << "confirm " << lookup[bGamble]->f(bestBet) << endl;
			}
		}
	#endif

    return bestBet;


}
