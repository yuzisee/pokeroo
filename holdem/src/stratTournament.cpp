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
#include "stratTournament.h"

TournamentStrategy::~TournamentStrategy()
{
    #ifdef LOGTOURN
        if( logFile.is_open() )
        {
            logFile.close();
        }
    #endif
}


void TournamentStrategy::SeeCommunity(const Hand& h, const int8 cardsInCommunity)
{

    DistrShape w_wl(0);


    CommunityPlus onlyCommunity;
    onlyCommunity.SetUnique(h);

    CommunityPlus withCommunity;
    withCommunity.SetUnique(ViewHand());
    withCommunity.AppendUnique(onlyCommunity);


    //StatsManager::QueryDefense(foldcumu,withCommunity,onlyCommunity,cardsInCommunity);
    ViewTable().CachedQueryOffense(callcumu,withCommunity);
    //StatsManager::QueryOffense(callcumu,withCommunity,onlyCommunity,cardsInCommunity, &(ViewTable().communityBuffer) );
    StatsManager::Query(0,&detailPCT,&w_wl,withCommunity,onlyCommunity,cardsInCommunity);
    statmean = GainModel::ComposeBreakdown(detailPCT.mean,w_wl.mean);
    statworse = GainModel::ComposeBreakdown(detailPCT.worst,w_wl.worst);

        #ifdef LOGTOURN
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

        #endif

}



float64 TournamentStrategy::MakeBet()
{


    const float64 myMoney = ViewPlayer().GetMoney();
    float64 betToCall = ViewTable().GetBetToCall();
//    const float64 highBet = betToCall;
    const float64 myBet = ViewPlayer().GetBetSize();

    if( myMoney < betToCall ) betToCall = myMoney;





    ///TODO: Enhance this. Maybe scale each separately depending on different factors? Ooooh.
    ///I want you to remember that after the river is dealt and no more community cards will come, that
    ///skew and kurtosis are undefined and other DistrShape related values are meaningless.

    float64 maxShowdown = ViewTable().GetMaxShowdown();
    if( maxShowdown > myMoney ) maxShowdown = myMoney;
//    float64 choiceScale = (betToCall - myBet)/(maxShowdown - myBet);
 //   if( maxShowdown == myBet || maxShowdown < betToCall ) choiceScale = 1;
//    if( choiceScale > 1 ) choiceScale = 1;
//    if( choiceScale < 0 ) choiceScale = 0;


    float64 distrScale = myMoney / ViewTable().GetAllChips() ;


//    if( ( bGamble % 4 ) / 2 == 0 )/* 0,1 */
//    {
//        distrScale = 1 - distrScale;
//    }/* else 2,3 */
//    if( distrScale > 1 ) distrScale = 1;
//    if( distrScale < 0 ) distrScale = 0;


/*
    float64 impliedFactor;
    const float64 improvePure = (detailPCT.improve+1)/2;
    const float64 improveDev = detailPCT.stdDev * (1-improvePure) + detailPCT.avgDev * improvePure;

    if( detailPCT.n == 1 )
    {
        impliedFactor = 1;
    }else
    {
        if( bGamble % 2 == 0 ) // 0,2
        {
            impliedFactor = 1 + improveDev*2*improvePure;
        }else // 1,3
        {
            impliedFactor = 1 + improveDev*2*(1-improvePure);
        }
    }
*/

//    const StatResult statchoice = statworse * (1-choiceScale) + statmean * (choiceScale);

    const float64 ranking3 = callcumu.pctWillCall(statmean.loss); //wins+splits
    const float64 ranking = callcumu.pctWillCall(1-statmean.wins); //wins

    StatResult statchoice;


    if( bGamble == 1 )
    {
        statchoice.loss = ranking;
        statchoice.splits = ranking3 - ranking;
        statchoice.wins = 1 - ranking3;
        //Bluff your way out
    }else
    {
        statchoice.wins = ranking;
        statchoice.splits = ranking3 - ranking;
        statchoice.loss = 1 - ranking3;
    }
    statchoice.genPCT();


    ///VARIABLE: the slider can move due to avgDev too, maybe....
    CallCumulationD &choicecumu = callcumu;
    //SlidingPairCallCumulationD choicecumu( &callcumu, &foldcumu, distrScale );
    //SlidingPairCallCumulationD choicecumu( &callcumu, &foldcumu, 0.5 );



    ExactCallD myExpectedCall(myPositionIndex, &(ViewTable()), &choicecumu);
    //myExpectedCall.SetImpliedFactor(impliedFactor);







/*

    if( bGamble / 4 == 1 )
    {
        ///Out of: choiceScale*timing[], 1-(1-choiceScale)*(1-timing[]), (choiceScale+timing[])/2
        ///We pick 1-(1-choiceScale)*(1-timing[]) because we need choiceScale=1 (or timing=1?) to force

        const float64 scaleToMean = choiceScale;
        const float64 offset = 1;

        ///When scaleToMean=0 we know we are using pct.worst case so we can assume people will fold, and we worst case covers our position
        ///When scapeToMean=1 we know we are using pct.mean and we should assume everybody will call
        const float64 enemies = myExpectedCall.handsDealt()-1 + offset;
        const float64 oppBetSoFar = myExpectedCall.oppBet() - myExpectedCall.alreadyBet();
        float64 called;
        if( highBet == 0 )
        {
            called = 0;
        }else
        {
            called = oppBetSoFar/highBet;
        }
        const float64 notcalled = enemies - called ;
        const float64 likelyToCallPCT = callcumu.pctWillCall(1-statmean.loss);

        const float64 actualcallers = called + likelyToCallPCT * notcalled;

        const float64 scaledcallers = enemies*scaleToMean + actualcallers*(1-scaleToMean);
        myExpectedCall.callingPlayers(scaledcallers);


    }
*/
//    GainModelReverse choicegain_rev(statchoice,&myExpectedCall);
//    GainModelReverseNoRisk choicegain_rnr(statworse,&myExpectedCall);

    GainModel choicegain_base(statchoice,&myExpectedCall);
//    GainModelNoRisk choicegain_nr(statworse,&myExpectedCall);

//	SlidingPairFunction gp(&choicegain_base,&choicegain_rev,distrScale,&myExpectedCall);

//	SlidingPairFunction ap(&choicegain_nr,&choicegain_rnr,distrScale,&myExpectedCall);

    GainModel &choicegain = choicegain_base;
//    SlidingPairFunction choicegain(&gp,&ap,choiceScale/2,&myExpectedCall);

        #ifdef DEBUGSPECIFIC
        if( ViewTable().handnum == DEBUGSPECIFIC )
        {
            std::ofstream excel( (ViewPlayer().GetIdent() + "functionlog.csv").c_str() );
            if( !excel.is_open() ) std::cerr << "\n!functionlog.cvs file access denied" << std::endl;
            choicegain_base.breakdown(1000,excel,betToCall,maxShowdown);
            //myExpectedCall.breakdown(0.005,excel);

            excel.close();
        }
        #endif




    float64 choicePoint = choicegain.FindBestBet();
    const float64 choiceFold = choicegain.FindZero(choicePoint,myMoney);

    const float64 callGain = choicegain.f(betToCall);
    #ifdef DEBUGASSERT
    const float64 raiseGain = choicegain.f(choicePoint);
    #endif

        #ifdef LOGTOURN


            HandPlus convertOutput;
            convertOutput.SetUnique(ViewHand());
            convertOutput.DisplayHand(logFile);

            logFile << "Bet to call " << betToCall << " (from " << myBet << ")" << endl;
            logFile << "(Mean) " << statmean.pct * 100 << "%"  << std::endl;
            logFile << "(Mean.wins) " << statmean.wins * 100 << "%"  << std::endl;
            logFile << "(Mean.splits) " << statmean.splits * 100 << "%"  << std::endl;
            logFile << "(Mean.loss) " << statmean.loss * 100 << "%"  << std::endl;
            logFile << "(Worst) " << statworse.pct * 100 << "%"  << std::endl;
            logFile << "(Outright) " << statchoice.pct * 100 << "%"  << std::endl;
            logFile << "(Outright.wins) " << statchoice.wins * 100 << "%"  << std::endl;
            logFile << "(Outright.splits) " << statchoice.splits * 100 << "%"  << std::endl;
            logFile << "(Outright.loss) " << statchoice.loss * 100 << "%"  << std::endl;
            //logFile << "impliedFactor " << impliedFactor << endl;
            /*
            if( bGamble / 4 == 1 ){
                float64 alreadyCalled = 0;
                if( betToCall != 0 ) alreadyCalled = (ViewTable().GetRoundBetsTotal() - ViewPlayer().GetBetSize())/highBet;
                logFile << "expected versus (" << myExpectedCall.callingPlayers() << ") from " << alreadyCalled << endl;
            }
            */
            logFile << "Choice Optimal " << choicePoint << endl;
            logFile << "Choice Fold " << choiceFold << endl;
            logFile << "f("<< betToCall <<")=" << callGain << endl;

        #endif

    if( choicePoint < betToCall )
    {///It's probably really close though
        logFile << "Choice Optimal < Bet to call" << endl;
        #ifdef DEBUGASSERT
            if ( choicePoint + ViewTable().GetChipDenom()/2 < betToCall )
            {
                std::cerr << "Failed assertion" << endl;
                exit(1);
            }
        #endif

        choicePoint = betToCall;

    }

    //Remember, due to rounding and chipDenom, (betToCall < choiceFold) is possible
    if( choicePoint >= choiceFold && betToCall >= choiceFold && callGain <= 0 )
    {///The highest point was the point closest to zero, and the least you can bet if you call--still worse than folding
        logFile << "CHECK/FOLD" << endl;
        return myBet;
    }
    ///Else you play wherever choicePoint is.


    #ifdef DEBUGASSERT
        if( raiseGain < 0 )
        {
            logFile << "raiseGain: f("<< choicePoint <<")=" << raiseGain << endl;
            logFile << "Redundant CHECK/FOLD detect required" << endl;
            return myBet;
        }
    #endif

    if( betToCall < choicePoint )
	{
	    logFile << "RAISETO " << choicePoint << endl << endl;
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
    logFile << "CALL " << choicePoint << endl;
    return betToCall;



}


