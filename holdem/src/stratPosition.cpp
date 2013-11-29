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

    ///=============================
    ///   Log Community/Situation
    ///=============================

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
        logFile << "Playing as " << ViewTable().GetPlayerBotType(myPositionIndex) << endl;
    }
#endif


    ///======================
    ///   Initialize hand
    ///======================



    CommunityPlus onlyCommunity;
    onlyCommunity.SetUnique(h);

    CommunityPlus withCommunity;
    withCommunity.SetUnique(ViewDealtHand());
    withCommunity.AppendUnique(onlyCommunity);

    ///======================
    ///   Compute chances
    ///======================

    DistrShape w_wl(0);

    ///Compute CallStats
    StatsManager::QueryDefense(statprob.core.handcumu,withCommunity,onlyCommunity,cardsInCommunity);
    statprob.core.foldcumu = statprob.core.handcumu;
    statprob.core.foldcumu.ReversePerspective();

    ///Compute CommunityCallStats
    ViewTable().CachedQueryOffense(statprob.core.callcumu,onlyCommunity, withCommunity);

    ///Compute WinStats
    StatsManager::Query(0,&detailPCT,&w_wl,withCommunity,onlyCommunity,cardsInCommunity);



    ///====================================
    ///   Compute Relevant Probabilities
    ///====================================

    statprob.core.playerID = myPositionIndex;
    statprob.core.statmean = CombinedStatResultsGeom::ComposeBreakdown(detailPCT.mean,w_wl.mean);

	statprob.Process_FoldCallMean();

    ///INVARIANT: statprob.statmean, statprob.callcumu, statprob.foldcumu are now all initialized.

    ///=============================
    ///   Log Stats/Probabilities
    ///=============================

#ifdef LOGPOSITION
    logFile << "*" << endl;
#endif


#ifdef DUMP_CSV_PLOTS

    csvpre.clear();
    switch(cardsInCommunity)
    {
        case 0:
            csvpre.append("p");
            break;
        case 3:
            csvpre.append("f");
            break;
        case 4:
            csvpre.append("t");
            break;
        case 5:
            csvpre.append("r");
            break;
        default:
            csvpre.append("x");
            break;
    }
#endif

    /*
     #ifdef DUMP_CSV_PLOTS

     string csvfilename = csvname;
     csvfilename.append("CALLSTATS.csv");
     //std::cout << endl << csvfilename.c_str() << endl;
     csvfilename = StatsManager::dbFileName(withCommunity, onlyCommunity, csvfilename.c_str());
     //std::cout << endl << csvfilename.c_str() << endl ;

     std::ofstream excel(csvfilename.c_str()
     , std::ios::out
     );

     if( !excel.is_open() ) std::cerr << "\n!functionlog.cvs file access denied" << std::endl;

     foldcumu.dump_csv_plots(excel,foldcumu);


     excel.close();

     csvfilename = csvname;
     csvfilename.append("CALLCOMMUNITYSTATS.csv");
     csvfilename = StatsManager::dbFileName(withCommunity, onlyCommunity, csvfilename.c_str());


     excel.open(csvfilename.c_str()
     , std::ios::out
     );

     if( !excel.is_open() ) std::cerr << "\n!functionlog.cvs file access denied" << std::endl;

     callcumu.dump_csv_plots(excel,callcumu);


     excel.close();

     #endif
     */






    ///=================
    ///   Log chances
    ///=================



#ifdef LOGPOSITION
    statprob.logfileAppendStatResultProbabilities(logOptions, logFile);
#endif

}


void PositionalStrategy::SeeAction(const HoldemAction& a)
{
    // NOTE: If you are Multibot or Gearbot, remember to pass this on to your individual PlayerStrategy as well.
}


void PositionalStrategy::setupPosition()
{
#ifdef LOGPOSITION
    logFile << endl << "*" << endl;
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


    ///PENDING: http://sourceforge.net/tracker/?func=detail&aid=2905465&group_id=242264&atid=1118658
#if 0
    const bool bHaveIActed = (notActedPlayers == 0); //If I have acted before and it's my turn again, everybody should have acted by now.

    //If I have acted, subtract one from ViewTable().NumberInHand() to represent "hands to beat"
    const float64 moodLevelOpponents = ViewTable().NumberInHand() - notActedPlayers - (bHaveIActed ? 1 : 0);

    //If I haven't acted, one of the notActedPlayers is myself and I don't count within uniformRandomOpponents
    const float64 uniformRandomOpponents = notActedPlayers - (bHaveIActed ? 1 : 0);
#endif

#ifdef LOGPOSITION
    logFile << "Bet to call " << betToCall << " (from " << myBet << ") at " << ViewTable().GetPotSize() << " pot, ";
    ///PENDING: http://sourceforge.net/tracker/?func=detail&aid=2905465&group_id=242264&atid=1118658
#if 0
    logFile << (int)(moodLevelOpponents) << " acted opponents, ";
    logFile << (int)(uniformRandomOpponents) << " uniform opponents";
#endif
    logFile << std::endl;
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



    // #############################################################################
    /// MATHEMATIC SOLVING ENDS HERE
    // #############################################################################

#ifdef LOGPOSITION
    //logFile << "selected risk  " << (choicePoint - myBet)/(maxShowdown - myBet) << endl;

    logFile << "Choice Optimal " << choicePoint << endl;
    logFile << "Choice Fold " << choiceFold << endl;

    logFile << "f("<< betToCall <<")=" << 1.0+callGain << endl;


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



    const float64 raiseGain = targetModel->f(choicePoint)
    ;

    if( raiseGain < 0 )
    {
#ifdef LOGPOSITION
        logFile << "raiseGain: f("<< choicePoint <<")=" << 1.0+raiseGain << endl;
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

void PositionalStrategy::printCommon(const ExpectedCallD &tablestate) {
#ifdef LOGPOSITION

    logFile << (int)tablestate.handsDealt() << " dealt, "
    << (int)tablestate.handsToOutplay() << " opp. (round), "
    << (int)tablestate.handStrengthOfRound() << " opp. assumed str., "
    << (int)tablestate.handsToShowdownAgainst() << " opp. still in"
    << std::endl;


#endif

}



void PositionalStrategy::printFoldGain(float64 raiseGain, CallCumulationD * e, ExpectedCallD & estat) {
#ifdef LOGPOSITION
    FoldOrCall foldGainCalculator(ViewTable(), statprob.core);
    std::pair<float64, float64> foldgainVal_xw = foldGainCalculator.myFoldGainAndWaitlength(foldGainCalculator.suggestMeanOrRank());
    const float64 &foldgainVal = foldgainVal_xw.first; // gain
    const float64 &xw = foldgainVal_xw.second; // waitlength (in total hands dealt)
    logFile << "FoldGain()";
    switch (foldGainCalculator.suggestMeanOrRank()) {
        case MEAN:
            logFile << "M";
            break;
        case RANK:
            logFile << "R";
            break;
    }
    logFile << "=" << foldgainVal;

    float64 numfolds = xw * e->Pr_haveWinPCT_strictlyBetterThan(statprob.core.statmean.pct - EPS_WIN_PCT); // waitlength (in folds)


    logFile << " x " << xw << "(=" << numfolds << " folds)\tvs play:" << (raiseGain + foldgainVal);
    if( ViewPlayer().GetInvoluntaryContribution() > 0 ) logFile << "   ->assumes $" << ViewPlayer().GetInvoluntaryContribution() << " forced";
    logFile << endl;
#endif // #ifdef LOGPOSITION
}

static const char * chipSignToString(const struct AggregatedState & state) {
    const float64 val = state.value;

    if (val > 1.0) {
        return "+$";
    }
    if (val < 1.0) {
        return "-$";
    }
    if (val == 1.0) {
        return "$";
    }
    return "~";
}

void PositionalStrategy::printStateModel(std::ofstream &logF, float64 displaybet, StateModel &ap_aggressive, const Player &me) {

    ap_aggressive.f(displaybet); // query
    logF << " AgainstCall("<< displaybet <<")=" << ap_aggressive.outcomeCalled.contribution << " from " << chipSignToString(ap_aggressive.outcomeCalled) << (fabs(ap_aggressive.outcomeCalled.value - 1.0) * me.GetMoney()) << " @ " << ap_aggressive.outcomeCalled.pr << endl;
    logF << "AgainstRaise("<< displaybet <<")=" << ap_aggressive.blendedRaises.contribution << " from " << chipSignToString(ap_aggressive.blendedRaises) << (fabs(ap_aggressive.blendedRaises.value - 1.0) * me.GetMoney()) << " @ " << ap_aggressive.blendedRaises.pr  << endl;
    logF << "        Push("<< displaybet <<")=" << ap_aggressive.outcomePush.contribution << " from " << chipSignToString(ap_aggressive.outcomePush) << ((ap_aggressive.outcomePush.value - 1.0) * me.GetMoney()) << " @ " << ap_aggressive.outcomePush.pr << endl;

}

void PositionalStrategy::printPessimisticWinPct(std::ofstream & logF, float64 betSize, CombinedStatResultsPessimistic * csrp) {
    if (csrp != 0) {
        logF << "\tW(" << csrp->getHandsToBeat(betSize) << "x)=" << csrp->getWinProb(betSize) << " L=" << csrp->getLoseProb(betSize) << " " << ((int)(csrp->splitOpponents())) << "o.w_s=(" << csrp->ViewShape(betSize).wins << "," << csrp->ViewShape(betSize).splits << ")";
    }
}



template< typename T >
void PositionalStrategy::printBetGradient(ExactCallBluffD & rl, ExactCallBluffD & rr, T & m, ExpectedCallD & tablestate, float64 separatorBet,  CombinedStatResultsPessimistic * csrp)
{
#ifdef LOGPOSITION


    int32 raiseStep = 0;

    {
        FoldOrCall rlF(*(tablestate.table), rl.fCore);

        int32 firstFoldToRaise = -1;
        float64 orAmount;
        for(raiseStep = 0, orAmount = 0.0; orAmount < maxShowdown; ++raiseStep )
        {
            orAmount =  rl.RaiseAmount(betToCall,raiseStep);

            const float64 oppRaisedFoldGain = rlF.myFoldGainAgainstPredictedRaise(rlF.suggestMeanOrRank(), betToCall, tablestate.alreadyBet(), orAmount);
            const float64 oppRaisedPlayGain = m.g_raised(betToCall,orAmount);
            if( oppRaisedFoldGain > oppRaisedPlayGain ){ break; /* We'd fold at this point. Stop incrementing */ } else {  firstFoldToRaise = raiseStep+1; }
        }

        ;

        if (separatorBet != betToCall) {
            logFile << endl << "Why didn't I call?" << endl;
        }
        for(raiseStep = 0, orAmount = 0.0; orAmount < maxShowdown; ++raiseStep )
        {
            orAmount =  rl.RaiseAmount(betToCall,raiseStep);
            logFile << "OppRAISEChance";
            switch (rlF.suggestMeanOrRank()) {
                case MEAN:
                    logFile << "M";
                    break;
                case RANK:
                    logFile << "R";
                    break;
            }
            if( raiseStep >= firstFoldToRaise ) {  logFile << " [F] ";  } else { logFile << " [*] "; }

            // Here, raiseStep is just the iterator. rl.RaiseAmount(betToCall,raiseStep) is the amount, rl.pRaise(betToCall,raiseStep,maxcallStep) is the probability that we see a raise of (at least) this amount
            logFile << rl.pRaise(betToCall,raiseStep,firstFoldToRaise) << " @ $" << orAmount;

            logFile << "\tfold -- left" << rl.pWin(orAmount) << "  " << rr.pWin(orAmount) << " right"; // This is the probability that everyone else folds (e.g. if they knew what you had and have a uniform distribution of possible hands -- but note that their decision is based on which StatResult you choose, so it can vary from bet to bet as well as bot to bot.)
            printPessimisticWinPct(logFile, orAmount, csrp);
            logFile << endl;

        }
    }

    const float64 minNextRaiseTo = (separatorBet*2-betToCall);
    if( maxShowdown - minNextRaiseTo < DBL_EPSILON ) return;

    if (separatorBet != betToCall) {
        logFile << "\t--" << endl;
        logFile << "What am I expecting now, given my actual bet?" << endl;


        FoldOrCall rrF(*(tablestate.table), rr.fCore);

        int32 firstFoldToRaise = -1;
        float64 mrAmount;
        for(raiseStep = 0, mrAmount = 0.0; mrAmount < maxShowdown; ++raiseStep )
        {
            mrAmount = rl.RaiseAmount(separatorBet,raiseStep);

            const float64 oppRaisedFoldGain = rrF.myFoldGainAgainstPredictedRaise(rrF.suggestMeanOrRank(), separatorBet, tablestate.alreadyBet(), mrAmount);
            const float64 oppRaisedPlayGain = m.g_raised(separatorBet,mrAmount);
            if( oppRaisedFoldGain > oppRaisedPlayGain ){ break; /* We'd fold at this point. Stop incrementing */ } else {  firstFoldToRaise = raiseStep+1; }

        }


        for(raiseStep = 0, mrAmount = 0.0; mrAmount < maxShowdown; ++raiseStep )
        {
            mrAmount =  rl.RaiseAmount(separatorBet,raiseStep);
            logFile << "OppRAISEChance";
            switch (rrF.suggestMeanOrRank()) {
                case MEAN:
                    logFile << "M";
                    break;
                case RANK:
                    logFile << "R";
                    break;
            }
            if( raiseStep >= firstFoldToRaise ) {  logFile << " [F] ";  } else { logFile << " [*] "; }

            logFile << rl.pRaise(separatorBet,raiseStep,firstFoldToRaise) << " @ $" << mrAmount;

            /*
             logFile << "\tfold -- left" << rl.pWin(mrAmount) << "  " << rr.pWin(mrAmount) << " right";
             printPessimisticWinPct(logFile, mrAmount, csrp);
             */
            logFile << endl;
        }
    }
#endif // LOGPOSITION
}






///==============================
///   AceBot, TrapBot, NormBot
///==============================

// First, deter low bets using detailPCT.avgDev (uncertainty)
// Correspondingly, boost high bets by detailPCT.avgDev (uncertainty)

// TRAPBOT:
//                                              Low Bets  |  High Bets
// Many Improve Small & Some Worsen Greatly               |  TRAP includes insuranceDeterrent if more hands are improving hands (makes opponents look like they are more likely to fold to a high bet)
// Some Improve Greatly & Many Worsen Small               |
//
// TRAPBOT deters high bets when there is high chance of improving.

// ACTIONBOT:
//
// Increases implied odds on Low Bets proportional to detailPCT.avgDev
// /* On higher bets: Reduces considered opponents (you probably don't have to beat the people who folded), especially if you are going to improve your hand */
//


float64 ImproveGainStrategy::MakeBet()
{
	setupPosition();

	if( maxShowdown <= 0 ) return 0;

    const float64 improveMod = detailPCT.improve; //Generally preflop is negative here, so you probably don't want to accentuate that
#ifdef LOGPOSITION

    const float64 improvePure= (improveMod+1)/2;

    //const float64 targetImproveBy = detailPCT.avgDev / 2 / improvePure;

    const float64 targetWorsenBy = detailPCT.avgDev / 2 / (1 - improvePure);
#endif //    #ifdef LOGPOSITION
    const float64 impliedOddsGain = (statprob.core.statmean.pct + detailPCT.avgDev / 2) / statprob.core.statmean.pct;
    //const float64 oppInsuranceSmallBet = (1 - statmean.pct + targetWorsenBy) / (1 - statmean.pct);
    const float64 oppInsuranceBigBet = (improveMod>0)?(improveMod/2):0;


    const float64 awayFromDrawingHands = 1.0 / (ViewTable().NumberInHandInclAllIn() - 1);
    StatResult statversus = (statprob.statrelation * (awayFromDrawingHands)) + (statprob.statranking * (1.0-awayFromDrawingHands));
    statversus.genPCT();



#ifdef ANTI_PRESSURE_FOLDGAIN
    ExpectedCallD   tablestate(myPositionIndex,  &(ViewTable()), statprob.statranking.pct, statprob.core.statmean.pct);
    ExactCallBluffD myDeterredCall(&tablestate, statprob.core);
    ExactCallBluffD myDeterredCall_left(&tablestate, statprob.core);
    ExactCallBluffD myDeterredCall_right(&tablestate, statprob.core);
#else
    ExactCallBluffD myDeterredCall(myPositionIndex, &(ViewTable()), &choicecumu, &raisecumu);
#endif


    OpponentFoldWait myFearControl(&tablestate);


    const float64 riskprice = myDeterredCall.RiskPrice(); // If you repeatedly bet this price in this situation, even the average best hand on the table is worth throwing down and you'll only get caught by really strong hands.
    const float64 geom_algb_scaler = (riskprice < maxShowdown) ? riskprice : maxShowdown;
    const float64 min_worst_scaler = myFearControl.FearStartingBet(myDeterredCall,riskprice);

    StatResult statWorse = statprob.statworse(tablestate.handsDealt());

#ifdef LOGPOSITION
    statprob.logfileAppendStatResultProbability_statworse(logFile, statWorse, tablestate.handsDealt());

    // TODO(from yuzisee): handsToBeat() here.
	const float64 fullVersus = ViewTable().NumberStartedRoundInclAllIn() - 1; // This is the "established" hand strength requirement of anyone willing to claim they will win this hand.
    // TODO TODO (from yuzisee): But peopleDrawing is relative to fullVersus?
    const float64 peopleDrawing = (1 - improvePure) * (ViewTable().NumberInHandInclAllIn() - 1);//You probably don't have to beat the people who folded, especially if you are going to improve your hand
    const float64 newVersus = (fullVersus - peopleDrawing*(1-improvePure)*detailPCT.stdDev);
#endif // #ifdef LOGPOSITION

    //bGamble == 2 is ActionBot
    //bGamble == 1 is TrapBot

    ///In the future actOrReact should be based on opponent betting patterns


    StatResult left = statversus;
    StatResult base_right = statprob.core.statmean; // (NormalBot uses statmean, others use statversus.)

    StatResult right = statWorse;


    if( bGamble >= 2 ) //Actionbot only
    {
        myDeterredCall_left.SetImpliedFactor(impliedOddsGain);

        //Need scaling (This could adjust RiskPrice or the geom/algb equilibrium as needed)
        // myDeterredCall_right.callingPlayers(newVersus);  // LEGACY ASSUMEFOLDS support here.
		//NOTE: The above line is commented out. We no longer set eFolds and I'm not sure it did anything anyway. See revision b71953a3ab52 for more.
	}

    //Unrelated note: TrapBot and ActionBot are based on statversus only
    if( bGamble >= 1 )
    {
        base_right = statversus;

        const float64 enemyChances = 1.0;//(ViewTable().GetNumberInHand() - 1.0) / ViewTable().GetNumberInHand() / 2;

        left = statversus;
#ifndef DEBUG_TRAP_AS_NORMAL
        //targetWorsenBy is here
        left.deterBy(detailPCT.avgDev/2.0*enemyChances);//2.0 is to estimate median
        //Need scaling
#endif

        //targetWorsenBy is also here


        right.boostBy(detailPCT.avgDev/2.0);//Need scaling


        // We have a drawing hand, so assume opponents would fold to a bet right now. This should encourage us to bet low and see more cards.
        myDeterredCall_right.insuranceDeterrent = oppInsuranceBigBet;

    }


    CombinedStatResultsGeom leftCS(left,left,true, myDeterredCall_left);
    CombinedStatResultsGeom cornerCS(statversus,statversus,true, myDeterredCall_right);


    CombinedStatResultsGeom cornerCS_fear(right,right,false, myDeterredCall_left);

    CombinedStatResultsGeom rightCS_fear(right,right,false, myDeterredCall_right);
    // const StatResult s_acted, const StatResult s_nonacted, bool bConvertToNet s_acted, s_nonacted, bConvertToNet, c)


    GainModelGeom geomModel(leftCS, myDeterredCall_left);
    GainModelNoRisk algbModel(cornerCS, myDeterredCall_right);

	GainModelGeom geomModel_fear(cornerCS_fear, myDeterredCall_left);
	GainModelNoRisk algbModel_fear(rightCS_fear, myDeterredCall_right);




#ifdef LOGPOSITION
    if (tablestate.handsToShowdownAgainst() != 1) {
        statprob.logfileAppendStatResultProbability_statworse(logFile, rightCS_fear.ViewShape(), 1);
    }

	if( bGamble == 0 )
	{ logFile << " -  NORMAL  - " << endl;}
	else if( bGamble == 1 )
	{ logFile << " -  TRAP  - " << endl;}
	else if( bGamble == 2 )
	{ logFile << " -  ACTION  - " << endl;}

    logFile << (int)tablestate.handsDealt() << " dealt, "
    << (int)tablestate.handsToOutplay() << " opp. (round), "
    << (int)tablestate.handStrengthOfRound() << " opp. assumed str., "
    << (int)tablestate.handsToShowdownAgainst() << " opp. still in"
    << std::endl;

    if( bGamble >= 1 )
    {
        logFile << "Personality tweak parameters..." << endl;
#ifdef LOGPOSITION
        //            logFile << minWin <<" ... "<< minWin+2 <<" = impliedfactor " << distrScale << endl;
        logFile << " " << improvePure <<" improvePure (pure Pr of improving) " << endl;
        logFile << "Likely Worsen By "<< targetWorsenBy << endl; //TRAPBOT, ACTIONBOT
        if( bGamble >= 2 ) logFile << "impliedOddsGain would be " << impliedOddsGain  << " (1 + detailPCT.avgDev / statmean.pct / 2) bonus" << endl; //ACTIONBOT
        logFile << "opp Likely to fold " << oppInsuranceBigBet << " (to a big bet) is the same as improveMod/2 if positive" << endl; //TRAPBOT, ACTIONBOT
        if( bGamble >= 2 ) logFile << "Can push expectedVersus from " << fullVersus << " ... " << newVersus << " ... " << (fullVersus - peopleDrawing) << " (scaled down by (1-improvePure) & detailPCT.stdDev)" << endl; //ACTIONBOT
#endif
        logFile << endl;
    }
    logFile << leftCS.ViewShape().pct << ":React/Main ... " << " ... " << rightCS_fear.ViewShape().pct << ":Act/Fear" << endl;
#endif

    ///From geom to algb
	AutoScalingFunction hybridgainDeterred_aggressive(geomModel,algbModel,0.0,geom_algb_scaler,left.pct*base_right.pct,&tablestate);
	AutoScalingFunction hybridgain_fear(geomModel_fear,algbModel_fear,0.0,geom_algb_scaler,left.pct*base_right.pct,&tablestate);


    ///From regular to fear A(x2)
	AutoScalingFunction ap(hybridgainDeterred_aggressive,hybridgain_fear,min_worst_scaler,riskprice,&tablestate, SLIDERX);

    GeomStateCombiner cg;
	StateModel choicemodel( myDeterredCall_left, &ap, cg );
#ifdef DEBUG_TRAP_AS_NORMAL
#ifdef LOGPOSITION
    logFile << "  DEBUGTRAPASNORMAL DEBUGTRAPASNORMAL DEBUGTRAPASNORMAL  " << endl;
#endif
    //StateModel choicemodel( &myDeterredCall_left, &hybridgainDeterred_aggressive );
    const float64 bestBet = solveGainModel(&choicemodel) ;
    StateModel & rolemodel = choicemodel;
#else
	///From regular to fear B(x2)
	AutoScalingFunction ap_right(hybridgainDeterred_aggressive,hybridgain_fear,min_worst_scaler,riskprice,&tablestate, SLIDERX);
    StateModel choicemodel_right( myDeterredCall_right, &ap_right, cg );



    AutoScalingFunction rolemodel(choicemodel,choicemodel_right,betToCall,riskprice,&tablestate, SLIDERX);




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



#ifdef DUMP_CSV_PLOTS
    if (bGamble == 0)
    {
        string csvfilename = csvpre;
        csvfilename.append("PLOT.csv");
        //std::cout << endl << csvfilename.c_str() << endl;
        csvfilename = StatsManager::dbFileName(ViewDealtHand(), Hand::EMPTY_HAND, csvfilename.c_str());
        //std::cout << endl << csvfilename.c_str() << endl ;

        std::ofstream excel(csvfilename.c_str()
                            , std::ios::out
                            );

        if( !excel.is_open() ) std::cerr << "\n!functionlog.cvs file access denied" << std::endl;

        excel << "b, oppFoldChance , playChance, geom, geom w/ arith, all four, statemodel" << std::endl;
        for(float64 csv_betsize = betToCall;csv_betsize < maxShowdown;csv_betsize+=ViewTable().GetChipDenom()/4)
        {
            excel << csv_betsize << ",";
            choicemodel.dump_csv_plots(excel,csv_betsize);
            excel << geomModel.f(csv_betsize) << ",";
            excel << hybridgainDeterred_aggressive.f(csv_betsize) << ",";
            excel << ap.f(csv_betsize) << ",";
            excel << choicemodel.f(csv_betsize) << ",";

            excel << std::endl;
        }


        excel.close();

    }
#endif





    const float64 bestBet = (bGamble == 0) ? solveGainModel(&choicemodel, &(statprob.core.callcumu)) : solveGainModel(&rolemodel, &(statprob.core.callcumu));

#endif








#ifdef LOGPOSITION

    const float64 nextBet = betToCall + ViewTable().GetMinRaise();
    const float64 viewBet = ( bestBet < betToCall + ViewTable().GetChipDenom() ) ? nextBet : bestBet; // If you fold, then display the callbet instead since we might as well log something.

    printFoldGain((bGamble == 0) ? choicemodel.f(viewBet) : rolemodel.f(viewBet), &(statprob.core.callcumu), tablestate);

    logFile << "\"riskprice\"... " << riskprice << "(based on scaler of " << geom_algb_scaler << ")" << endl;
    logFile << "oppFoldChance is first " << myFearControl.oppFoldStartingPct(myDeterredCall) << ", when betting b_min=" << min_worst_scaler << endl; // but why do I care?

    logFile << "Call Regular("<< viewBet <<")=" << 1.0+hybridgainDeterred_aggressive.f(bestBet) << endl;
    logFile << "   Call Fear("<< viewBet <<")=" << 1.0+hybridgain_fear.f(bestBet) << endl;


    printBetGradient< StateModel >
    (myDeterredCall_left, myDeterredCall_right, choicemodel, tablestate, viewBet, 0);


    logFile << "Guaranteed > $" << tablestate.stagnantPot() << " is in the pot for sure" << endl;
    logFile << "OppFoldChance% ... left " << myDeterredCall_left.pWin(viewBet) << " --" << myDeterredCall_right.pWin(viewBet) << " right" << endl;
    if( myDeterredCall.pWin(bestBet) > 0 )
    {
        logFile << "if playstyle NormalBot, overall utility is " << 1.0+choicemodel.f(viewBet) << endl;
        logFile << "if playstyle Trap/Ace, overall utility is " << 1.0+rolemodel.f(viewBet) << endl;
    }

#endif







    return bestBet;


}






///=================================
///   DangerBot, SpaceBot, ComBot
///=================================

// 0 : ComBot    --  statmean -> statworse
#if 0
// 1 : DangerBot --  statversus --> statworse
#endif //0
// 2 : SpaceBot  --  statversus --> statworse
//Here, statversus goes from: (2 players left in hand) statrelation  --> (all players left in hand) statranking

///DeterredGainStrategy is still ActOrReact driven. (By comparison, ImproveGainStrategy is not)
// Both ComBot and DangerBot adjust ImpliedFactor heuristically based on whether or not there are still interesting bets to be made this hand (in current & future rounds)
// SpaceBot is plain vanilla.

float64 DeterredGainStrategy::MakeBet()
{
	setupPosition();

	if( maxShowdown <= 0 ) return 0;


    const float64 awayFromDrawingHands = 1.0 / (ViewTable().NumberInHandInclAllIn() - 1);
    StatResult statversus = (statprob.statrelation * (awayFromDrawingHands)) + (statprob.statranking * (1.0-awayFromDrawingHands));
    statversus.genPCT();



#ifdef ANTI_PRESSURE_FOLDGAIN
    ExpectedCallD   tablestate(myPositionIndex,  &(ViewTable()), statprob.statranking.pct, statprob.core.statmean.pct);
    ExactCallBluffD myDeterredCall(&tablestate, statprob.core);
#else
    //ExactCallD myExpectedCall(myPositionIndex, &(ViewTable()), &choicecumu);
    ExactCallBluffD myDeterredCall(myPositionIndex, &(ViewTable()), &choicecumu, &raisecumu);
#endif

    StatResult statWorse = statprob.statworse(tablestate.handsDealt());
#ifdef LOGPOSITION
    statprob.logfileAppendStatResultProbability_statworse(logFile, statWorse, tablestate.handsDealt());
#endif // LOGPOSITION


    const float64 riskprice = myDeterredCall.RiskPrice();
    const float64 geom_algb_scaler = (riskprice < maxShowdown) ? riskprice : maxShowdown;

    OpponentFoldWait myFearControl(&tablestate);
    const float64 min_worst_scaler = myFearControl.FearStartingBet(myDeterredCall, geom_algb_scaler);


    //
    const float64 certainty = myFearControl.ActOrReact(betToCall,myBet,maxShowdown);

    const float64 uncertainty = fabs( statprob.statranking.pct - statprob.core.statmean.pct );
    const float64 timeLeft = (  detailPCT.stdDev*detailPCT.stdDev + uncertainty*uncertainty  );
    const float64 nonvolatilityFactor = 1 - timeLeft;

	const float64 nearEndOfBets = nonvolatilityFactor*(1-certainty) + certainty;



    if( bGamble <= 1 ) //DangerBot, ComBot, not SpaceBot
    {
        //small insuranceDeterrent means more likely for opponent to fold vs. call
        myDeterredCall.SetImpliedFactor( 1 / nearEndOfBets );
    }

    StatResult left = statversus;
	if( bGamble == 0 ) left = statprob.core.statmean;

    CombinedStatResultsGeom leftCS(left, left, true, myDeterredCall);
    GainModelGeom geomModel(leftCS, myDeterredCall);

    StatResult right = statWorse;
    right.repeated = 1-certainty;

    left.repeated = certainty;

    CombinedStatResultsGeom rightCS(right, right, false, myDeterredCall);
	GainModelNoRisk algbModel(rightCS, myDeterredCall);



#ifdef LOGPOSITION
    if (tablestate.handsToShowdownAgainst() != 1) {
        statprob.logfileAppendStatResultProbability_statworse(logFile, rightCS.ViewShape(), 1);
    }

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
#endif

    printCommon(tablestate);

#ifdef LOGPOSITION


	if( bGamble <= 1 )
	{
	    logFile << "Personality tweak parameters..." << endl;
        logFile << "timeLeft:                  sigma + uncertainty in Pr{win} is  " << timeLeft << endl;
        logFile << "  uncertainty (remaining convergence of statrank to statmean: " << uncertainty << endl;
        logFile << "  detailPCT.stdDev " << detailPCT.stdDev << endl;
        logFile << "nearEndOfBets         " << nearEndOfBets << endl;
        logFile << "impliedFactor... " << 1 / nearEndOfBets << endl;
        logFile << endl;
    }
    logFile << "Act(0%) or React(100%)? " << certainty << ", pct " << left.pct << " ... " << rightCS.ViewShape().pct << " ... " << right.pct << endl;

#endif

    ///Choose from geom to algb
	AutoScalingFunction hybridgainDeterred(geomModel,algbModel,min_worst_scaler,geom_algb_scaler,&tablestate, SLIDERX);

    GeomStateCombiner cg;
    StateModel ap_aggressive( myDeterredCall, &hybridgainDeterred, cg );



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

    const float64 bestBet = solveGainModel(&choicemodel, &(statprob.core.callcumu));

#ifdef LOGPOSITION


#ifdef VERBOSE_STATEMODEL_INTERFACE
    const float64 displaybet = (bestBet < betToCall) ? betToCall : bestBet;

    printFoldGain(choicemodel.f(displaybet), &(statprob.core.callcumu), tablestate);


    choicemodel.f(displaybet); //since choicemodel is ap_aggressive
    logFile << " AgainstCall("<< displaybet <<")=" << ap_aggressive.outcomeCalled.contribution << " from $" << ap_aggressive.outcomeCalled.value << " @ " << ap_aggressive.outcomeCalled.pr << endl;
    logFile << "AgainstRaise("<< displaybet <<")=" << ap_aggressive.blendedRaises.contribution << " from $" << ap_aggressive.blendedRaises.value << " @ " << ap_aggressive.blendedRaises.pr  << endl;
    logFile << "        Push("<< displaybet <<")=" << ap_aggressive.outcomePush.contribution << " from $" << ap_aggressive.outcomePush.value << " @ " << ap_aggressive.outcomePush.pr << endl;

#endif

    //if( bestBet < betToCall + ViewTable().GetChipDenom() )
    {
        logFile << "\"riskprice\"... " << riskprice << "(based on scaler " << geom_algb_scaler << ")" << endl;
        logFile << "oppFoldChance is first " << myFearControl.oppFoldStartingPct(myDeterredCall) << ", when betting b_min=" << min_worst_scaler << endl;

        logFile << "Against("<< displaybet <<")Geom=" << 1.0+geomModel.f(displaybet) << endl;
        logFile << "Against("<< displaybet <<")Algb=" << 1.0+algbModel.f(displaybet) << endl;
    }


    printBetGradient< StateModel >
    (myDeterredCall, myDeterredCall, ap_aggressive, tablestate, displaybet, 0);


    logFile << "Guaranteed > $" << tablestate.stagnantPot() << " is in the pot for sure" << endl;

    logFile << "OppFoldChance% ...    " << myDeterredCall.pWin(displaybet) << "   d\\" << myDeterredCall.pWinD(displaybet) << endl;
    if( myDeterredCall.pWin(displaybet) > 0 )
    {
        logFile << "if playstyle is Danger/Conservative, overall utility is " << choicemodel.f(displaybet) << endl;
    }

#endif

    return bestBet;


}






float64 PureGainStrategy::MakeBet()
{
	setupPosition();

	if( maxShowdown <= 0 ) return 0;



    ExpectedCallD   tablestate(myPositionIndex,  &(ViewTable()), statprob.statranking.pct, statprob.core.statmean.pct);


    StatResult left;
    StatResult left_lower;
    StatResult left_higher;

    if (statprob.statrelation.pct < statprob.statranking.pct) {
        left_lower = statprob.statrelation;
        left_higher = statprob.statranking;
    } else {
        left_lower = statprob.statrelation;
        left_higher = statprob.statranking;
    }

    if (bGamble == 2) {
        left = statprob.statrelation; // Action
    } else if (bGamble == 0) {
        left = statprob.statranking; // Normal
    } else if (bGamble == 1) {
        const float64 awayFromDrawingHands = 1.0 / (ViewTable().NumberInHandInclAllIn() - 1);
        StatResult statversus = (statprob.statrelation * (awayFromDrawingHands)) + (statprob.statranking * (1.0-awayFromDrawingHands));
        statversus.genPCT();
        // Trap
    } else if (bGamble == 3) {
        left = left_lower;
    } else if (bGamble == 4) {
        left = left_higher;
    } else {
        std::cerr << "Invalid bGamble in PureGainStrategy " << static_cast<int>(bGamble) << std::endl;
        exit(1);
    }

    PureStatResultGeom leftCS(statprob.core.statmean, left, statprob.core.foldcumu, tablestate);
    ExactCallBluffD myDeterredCall(&tablestate, statprob.core);

    // TODO(from joseph_huang): Switch this back to GainModelGeom for better bankroll management?
    // For now, simplify so we can debug other bugs easier.
    // Also, consider that true bankroll management would never call all-in without a guaranteed win.
    // Unfortunately, the Kelly criterion approach has corner cases that make it hard to get working off the bat. We'll revisit this later.
    GainModelNoRisk callModel(leftCS, myDeterredCall);

    // TODO(from yuzisee): When callgain is based on rank vs. mean, should the comparative opponentHandOpportunity's foldgain be based on rank vs. mean?
    // Consider: Opponent knows what I have vs. Opponent doesn't know what I have
    // NOTE: We had a "will call too often with Qc 3c" bug. That might be because we didn't reverse perspective up there.
    // TODO(from yuzisee): Are the other invocations of foldgain (e.g. Pr{push}) also dependent on reversed perspective?
    OpponentHandOpportunity opponentHandOpportunity(myPositionIndex, ViewTable(), statprob.core);
    CombinedStatResultsPessimistic csrp(opponentHandOpportunity, statprob.core);
    GainModelNoRisk raiseModelAlgb(csrp, myDeterredCall);
    GainModelGeom raiseModelGeom(csrp, myDeterredCall);

    GainModel &raiseModel = (bGamble < 3) ? dynamic_cast<GainModel &>(raiseModelAlgb) : dynamic_cast<GainModel &>(raiseModelGeom);


#ifdef LOGPOSITION
    logFile << "CallStrength W(" << static_cast<int>(tablestate.handStrengthOfRound()) << "c)=" << leftCS.getWinProb(betToCall) << " L=" << leftCS.getLoseProb(betToCall) << " o.w_s=(" << leftCS.ViewShape(betToCall).wins << "," << leftCS.ViewShape(betToCall).splits << ")" << endl;
    const float64 minRaiseTo = betToCall + ViewTable().GetMinRaise();
    logFile << "(MinRaise to $" << minRaiseTo << ") ";
    printPessimisticWinPct(logFile, minRaiseTo, &csrp);
    logFile << endl;


	if( bGamble == 0 )
	{ logFile << " -  statranking ca (algb) - " << endl;}
	else if( bGamble == 1 )
	{ logFile << " -  detailPCT  - " << endl;}
	else if( bGamble == 2 )
	{ logFile << " -  statrelation cg (algb) - " << endl;}
	else if( bGamble == 3 )
	{ logFile << " -  stat_low cg (geom)  - " << endl;}
	else if( bGamble == 4 )
	{ logFile << " -  stat_high ca (geom) - " << endl;}
#endif

    // Choose from ca or cg
    AlgbStateCombiner ca;
    GeomStateCombiner cg;
    IStateCombiner &stateCombiner = (bGamble / 2 == 1) ? dynamic_cast<IStateCombiner &>(cg) : dynamic_cast<IStateCombiner &>(ca);


    printCommon(tablestate);

    ///Choose from geom to algb
    const float64 aboveCallBelowRaise1 = betToCall + ViewTable().GetChipDenom() / 2.0;
    const float64 aboveCallBelowRaise2 = betToCall + ViewTable().GetChipDenom();
    AutoScalingFunction callOrRaise(callModel,raiseModel,aboveCallBelowRaise1,aboveCallBelowRaise2,&tablestate, RAW);

    StateModel ap_aggressive( myDeterredCall, &callOrRaise, stateCombiner );



    HoldemFunctionModel& choicemodel = ap_aggressive;


    const float64 bestBet = solveGainModel(&choicemodel, &(statprob.core.callcumu));
    
#ifdef LOGPOSITION
    
    
    
    
#ifdef VERBOSE_STATEMODEL_INTERFACE
    const float64 displaybet = (bestBet < betToCall) ? betToCall : bestBet;
    
    printFoldGain(choicemodel.f(displaybet), &(statprob.core.callcumu), tablestate);
    
    printStateModel(logFile, displaybet, ap_aggressive, ViewPlayer());
    
    if (betToCall < displaybet) {
        // If you raised, also show CALL
        printStateModel(logFile, betToCall, ap_aggressive, ViewPlayer());
    }
    
    if (betToCall == displaybet) {
        // If you called, also show MINRAISE
        printStateModel(logFile, minRaiseTo, ap_aggressive, ViewPlayer());
    }
    
#endif
    
    //if( bestBet < betToCall + ViewTable().GetChipDenom() )
    {
        /*
         logFile << "PlayAt($"<< displaybet <<")Call=" << callModel.f(displaybet) << endl;
         logFile << "PlayAt($"<< displaybet <<")Raise=" << raiseModel.f(displaybet) << endl;
         */
        
    }
    
    
    printBetGradient< StateModel >
    (myDeterredCall, myDeterredCall, ap_aggressive, tablestate, displaybet, &csrp);
    
    
    logFile << "Guaranteed > $" << tablestate.stagnantPot() << " is in the pot for sure" << endl;
    
    logFile << "OppFoldChance% ...    " << myDeterredCall.pWin(displaybet) << "   d\\" << myDeterredCall.pWinD(displaybet) << endl;
    if( myDeterredCall.pWin(displaybet) > 0 )
    {
        logFile << "if playstyle is Danger/Conservative, overall utility is " << choicemodel.f(displaybet) << endl;
    }
    
#endif
    
    return bestBet;
    
}



