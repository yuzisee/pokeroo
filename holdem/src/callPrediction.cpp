/***************************************************************************
 *   Copyright (C) 2006 by Joseph Huang                                    *
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

#include "callPrediction.h"
#include <math.h>
#include <float.h>
#include <algorithm>


//The idea behind eaFold, meanFold, rankFold, eaRkFold is that opponents have different reasons for folding


///In the end, we had to stick with Algb for foldgain. (At low blinds, oppfold is guaranteed in Geom)


#define NO_AUTO_RAISE



//#define ANTI_CHECK_PLAY


//#define DEBUGWATCHPARMS
#ifdef DEBUGWATCHPARAMS
#include <iostream>
#endif

const float64 ExactCallD::UNITIALIZED_QUERY = -1;

ExactCallD::~ExactCallD()
{
    if( noRaiseChance_A != 0 )
    {
        delete [] noRaiseChance_A;
    }
    if( noRaiseChanceD_A != 0 )
    {
        delete [] noRaiseChanceD_A;
    }
}

void ExactCallD::SetImpliedFactor(const float64 bonus)
{
    impliedFactor = bonus;
}


///This function is used to maximize chance to fold on a player-by-player basis
float64 ExactCallBluffD::topTwoOfThree(float64 a, float64 b, float64 c, float64 a_d, float64 b_d, float64 c_d, float64 & r) const
{
    if( b < a )
    {//a cannot be smallest
        if( b < c )
        {//b is smallest
            r = (a + c)/2;
            return (a_d + c_d)/2;
        }else //c < b < a
        {//c is smallest
            r = (a + b)/2;
            return (a_d + b_d)/2;
        }
    }
    else
    {//Since b > a, b cannot be smallest
        if( a < c )
        {//a is smallest
            r = (b + c)/2;
            return (b_d + c_d)/2;
        }else
        {//c is smallest
            r = (a + b)/2;
            return (a_d + b_d)/2;
        }
    }
}

float64 ExactCallBluffD::bottomThreeOfFour(float64 a, float64 b, float64 c, float64 d, float64 a_d, float64 b_d, float64 c_d, float64 d_d, float64 & r) const
{
    float64 x[4] = {a,b,c,d};

    std::sort(x,x+4); //Ascending, so drop bottom
    r = ( x[0] + x[1] + x[2] ) / 3;

    if( x[3] == a )
    {
        return (b_d + c_d + d_d)/3;
    }else if( x[3] == b )
    {
        return (a_d + c_d + d_d)/3;
    }else if( x[3] == c )
    {
        return (a_d + b_d + d_d)/3;
    }else
    {
        return (a_d + b_d + c_d)/3;
    }
}


float64 ExactCallBluffD::topThreeOfFour(float64 a, float64 b, float64 c, float64 d, float64 a_d, float64 b_d, float64 c_d, float64 d_d, float64 & r) const
{
    float64 x[4] = {a,b,c,d};

    std::sort(x,x+4); //Ascending, so drop first
    r = ( x[3] + x[2] + x[1] ) / 3;

    if( x[0] == a )
    {
        return (b_d + c_d + d_d)/3;
    }else if( x[0] == b )
    {
        return (a_d + c_d + d_d)/3;
    }else if( x[0] == c )
    {
        return (a_d + b_d + d_d)/3;
    }else
    {
        return (a_d + b_d + c_d)/3;
    }
}


///This function is used to maximize chance to CALL on a player-by-player basis
float64 ExactCallBluffD::bottomTwoOfThree(float64 a, float64 b, float64 c, float64 a_d, float64 b_d, float64 c_d, float64 & r) const
{
    if( b < a )
    {//b cannot be greatest
        if( a < c )
        {//c is greatest
            r = (a + b)/2;
            return (a_d + b_d)/2;
        }else //a > c, a > b
        {//a is greatest
            r = (c + b)/2;
            return (c_d + b_d)/2;
        }
    }
    else
    {//Since b >= a, a cannot be greatest
        if( b < c )
        {//c is greatest
            r = (b + a)/2;
            return (b_d + a_d)/2;
        }else
        {//b is greatest
            r = (a + c)/2;
            return (a_d + c_d)/2;
        }
    }
}

float64 ExactCallD::facedOdds_raise_Geom(const ChipPositionState & cps, float64 incrRaise, float64 fold_bet, float64 opponents, bool bCheckPossible, CallCumulationD * useMean)
{

    float64 raiseto = cps.alreadyBet + incrRaise; //Assume this is controlled to be less than or equal to bankroll

    if( raiseto >= cps.bankroll )
    {
        return 1-1.0/RAREST_HAND_CHANCE;
    }

    if( fold_bet > cps.bankroll )
    {
        fold_bet = cps.bankroll;
    }

    FacedOddsRaiseGeom a(table->GetChipDenom());

    a.pot = cps.pot;
    a.raiseTo = raiseto;
    a.fold_bet = fold_bet;
    a.bCheckPossible = bCheckPossible;
    a.riskLoss = (bCheckPossible) ? 0 : RiskLoss(cps.alreadyBet, cps.bankroll, opponents, raiseto, useMean, 0);



    const int8 N = handsDealt();
    const float64 avgBlind = (table->GetBigBlind() + table->GetSmallBlind()) * ( N - 2 )/ N / N;
    //We don't need to set w, because a.FindZero searches over w
    #ifdef SACRIFICE_COMMITTED
    a.FG.waitLength.amountSacrifice = cps.alreadyContributed + cps.alreadyBet + avgBlind;
    #else
    a.FG.waitLength.amountSacrifice = cps.alreadyBet + avgBlind;
    #endif
    a.FG.waitLength.bankroll = cps.bankroll;
    a.FG.waitLength.opponents = opponents;
    a.FG.waitLength.meanConv = useMean;
    a.FG.dw_dbet = 0; //We don't need this, unless we want the derivative of FG.f; Since we don't need any extrema or zeros of FG, we can set this to anything


/*
    const float64 fold_utility = bCheckPossible ? 0 : FG.f(fold_bet);

    if( fold_utility + raiseto < 0 )
    {//Folding is so pointless, you may as well play
        return 0;
    }
*/
    return a.FindZero(0,1);


}

const float64 ExactCallD::RiskLoss(float64 alreadyBet, float64 bankroll, float64 opponents, float64 raiseTo,  CallCumulationD * useMean, float64 * out_dPot)
{

    const int8 N = handsDealt();
    const float64 avgBlind = (table->GetBigBlind() + table->GetSmallBlind()) * ( N - 2 )/ N / N;
    FG.waitLength.meanConv = useMean;

    if(useMean == 0)
    {
        FG.waitLength.w = 1.0 - 1.0/N;
    }else
    {
        FG.waitLength.w = useMean->nearest_winPCT_given_rank(1.0 - 1.0/N);
    }
    FG.waitLength.amountSacrifice = (table->GetPotSize() - stagnantPot() - alreadyBet)/(handsIn()-1) + avgBlind;
    FG.waitLength.bankroll = (allChips() - bankroll)/(N-1);
    FG.waitLength.opponents = 1;
    FG.dw_dbet = 0; //Again, we don't need this
    const float64 riskLoss = FG.f( raiseTo ) + FG.waitLength.amountSacrifice;
    if(out_dPot != 0)
    {
        *out_dPot = FG.dF_dAmountSacrifice( raiseTo ) / (handsIn()-1) + 1 / (handsIn()-1);
    }

    return riskLoss;
}


//Here, dbetsize/dpot = 0
float64 ExactCallD::dfacedOdds_dpot_GeomDEXF(const ChipPositionState & cps, float64 incrRaise, float64 fold_bet, float64 w, float64 opponents, float64 dexf,  bool bCheckPossible, CallCumulationD * useMean)
{
    if( w <= 0 ) return 0;
    const float64 raiseto = cps.alreadyBet + incrRaise;
    if( raiseto > cps.bankroll ) return 0;


    if( fold_bet > cps.bankroll )
    {
        fold_bet = cps.bankroll;
    }


    const int8 N = handsDealt();
    const float64 avgBlind = (table->GetBigBlind() + table->GetSmallBlind()) * ( N - 2 )/ N / N;

    //The pot can't be zero, so base_minus_1 can't be 0, so base can't be 1, so log(base) can't be zero
    const float64 base_minus_1 = (cps.pot+raiseto)/(cps.bankroll-raiseto);//base = (B+pot)/(B-betSize); = 1 + (pot+betSize)/(B-betSize);

    const float64 wN_1 = pow(w,opponents-1);
    float64 fw = wN_1 * w;
    float64 dfw = opponents * wN_1;

    const float64 A = dfw * log1p( base_minus_1 );
    const float64 C = fw/(cps.bankroll+cps.pot) ;
    const float64 h_times_remaining = pow( (cps.bankroll+cps.pot)/(cps.bankroll-raiseto), fw ) * (cps.bankroll - raiseto);



    if(bCheckPossible)
    {
        return (h_times_remaining*C*dexf) / (h_times_remaining*A);
    }else
    {
    //USE FG for riskLoss
        float64 dRiskLoss_pot;
        RiskLoss(cps.alreadyBet, cps.bankroll, opponents, raiseto, useMean, &dRiskLoss_pot);

        FoldGainModel myFG(table->GetChipDenom());

    //USE myFG for F_a and F_b
        myFG.waitLength.meanConv = useMean;
        myFG.waitLength.w = w;
    #ifdef SACRIFICE_COMMITTED
        myFG.waitLength.amountSacrifice = cps.alreadyContributed + cps.alreadyBet + avgBlind;
    #else
        myFG.waitLength.amountSacrifice = cps.alreadyBet + avgBlind;
    #endif
        myFG.waitLength.bankroll = cps.bankroll;
        myFG.waitLength.opponents = opponents;
        myFG.dw_dbet = 0; //Again, we don't need this


        return (h_times_remaining*C*dexf - myFG.F_b(fold_bet) + dRiskLoss_pot*dexf) / (myFG.F_a(fold_bet) - h_times_remaining*A);
    }


}



float64 ExactCallD::facedOdds_call_Geom(const ChipPositionState & cps, float64 humanbet, float64 opponents, CallCumulationD * useMean)
{

    if( humanbet >= cps.bankroll )
    {
        return 1 - 1.0/RAREST_HAND_CHANCE;
    }


    FacedOddsCallGeom a(table->GetChipDenom());
    a.B = cps.bankroll;
    a.pot = cps.pot;
//    a.alreadyBet = cps.alreadyBet;
    a.outsidebet = humanbet;
    a.opponents = opponents;

    const int8 N = handsDealt();
    const float64 avgBlind = (table->GetBigBlind() + table->GetSmallBlind()) * ( N - 2 )/ N / N;
    #ifdef SACRIFICE_COMMITTED
    a.FG.waitLength.amountSacrifice = cps.alreadyContributed + cps.alreadyBet + avgBlind;
    #else
    a.FG.waitLength.amountSacrifice = cps.alreadyBet + avgBlind;
    #endif
    a.FG.waitLength.bankroll = cps.bankroll;
    a.FG.waitLength.opponents = opponents;
    a.FG.waitLength.meanConv = useMean;
    a.FG.dw_dbet = 0; //We don't need this, unless we want the derivative of FG.f; Since we don't need any extrema or zeros of FG, we can set this to anything

    return a.FindZero(0,1);
}

float64 ExactCallD::dfacedOdds_dbetSize_Geom(const ChipPositionState & cps, float64 humanbet, float64 dpot_dhumanbet, float64 w, float64 opponents, CallCumulationD * useMean)
{
    if( w <= 0 ) return 0;
	if( humanbet >= cps.bankroll ) return 0;


    const int8 N = handsDealt();
    const float64 avgBlind = (table->GetBigBlind() + table->GetSmallBlind()) * ( N - 2 )/ N / N;
    const float64 base_minus_1 = (cps.pot+humanbet)/(cps.bankroll-humanbet);//base = (B+pot)/(B-betSize); = 1 + (pot+betSize)/(B-betSize);

    FG.waitLength.w = w;
    #ifdef SACRIFICE_COMMITTED
    FG.waitLength.amountSacrifice = cps.alreadyContributed + cps.alreadyBet + avgBlind;
    #else
    FG.waitLength.amountSacrifice = cps.alreadyBet + avgBlind;
    #endif
    FG.waitLength.bankroll = cps.bankroll;
    FG.waitLength.opponents = opponents;
    FG.waitLength.meanConv = useMean;
    FG.dw_dbet = 0; //Again, we don't need this

    const float64 wN_1 = pow(w,opponents-1);
    float64 fw = wN_1 * w;
    float64 dfw = opponents * wN_1;

    const float64 h = pow( (cps.bankroll+cps.pot)/(cps.bankroll-humanbet), fw );
    const float64 A = dfw * log1p( base_minus_1 );
    const float64 C = (  dpot_dhumanbet/(cps.bankroll+cps.pot) + 1/(cps.bankroll-humanbet)  ) * fw;

    return
    (  (cps.bankroll*C-C*humanbet-1)*h - FG.F_b(humanbet)  )
     /
    (  FG.F_a(humanbet) - (cps.bankroll-humanbet)*h*A  )
    ;
}

float64 ExactCallD::facedOdds_Algb(const ChipPositionState & cps, float64 betSize, float64 opponents, CallCumulationD * useMean)
{
    FacedOddsAlgb a(table->GetChipDenom());
    a.pot = cps.pot;
    //a.alreadyBet = cps.alreadyBet; //just for the books?
    a.betSize = betSize;

    const int8 N = handsDealt();
    const float64 avgBlind = (table->GetBigBlind() + table->GetSmallBlind()) * ( N - 2 )/ N / N;
    #ifdef SACRIFICE_COMMITTED
    a.FG.waitLength.amountSacrifice = cps.alreadyContributed + cps.alreadyBet + avgBlind;
    #else
    a.FG.waitLength.amountSacrifice = cps.alreadyBet + avgBlind;
    #endif
    a.FG.waitLength.bankroll = cps.bankroll;
    a.FG.waitLength.opponents = opponents;
    a.FG.waitLength.meanConv = useMean;
    a.FG.dw_dbet = 0; //We don't need this...

    #if defined(DEBUG_TRACE_PWIN) && defined(DEBUG_TRACE_SEARCH)
		a.bTraceEnable = traceOut != 0 && useMean == 0;
    #endif

    return a.FindZero(0,1);
}
float64 ExactCallD::facedOddsND_Algb(const ChipPositionState & cps, float64 incrbet, float64 dpot, float64 w, float64 opponents)
{
    if( w <= 0 ) return 0;

    const float64 wN_1 = pow(w,opponents-1);

    float64 fw = wN_1 * w;
    float64 dfw = opponents * wN_1;

    float64 bet = cps.alreadyBet + incrbet;
    if( bet > cps.bankroll )
    {
        bet = cps.bankroll;
    }


    const int8 N = handsDealt();
    const float64 avgBlind = (table->GetBigBlind() + table->GetSmallBlind()) * ( N - 2 )/ N / N;
    #ifdef SACRIFICE_COMMITTED
    FG.waitLength.amountSacrifice = cps.alreadyContributed + cps.alreadyBet + avgBlind;
    #else
    FG.waitLength.amountSacrifice = cps.alreadyBet + avgBlind;
    #endif
    FG.waitLength.bankroll = cps.bankroll;
    FG.waitLength.opponents = opponents;
//derivative of algebraic uses FG_a and FG_b which are independent of useMean


    return (
                ( (1+FG.F_b(bet)) - ((dpot+1) * fw ) )
                        /
                ( (cps.pot+bet) * dfw - FG.F_a(bet) )
            );
}


float64 ExactCallD::RaiseAmount(const float64 betSize, int32 step)
{

	float64 raiseAmount;
    float64 minRaiseDirect = minRaiseTo();
    float64 minRaiseBy = minRaiseDirect - callBet();
    float64 minRaiseBet = betSize - callBet();

    if( minRaiseBet < minRaiseDirect )
    {
        raiseAmount = betSize + minRaiseBy;

    }else{
        raiseAmount = betSize + minRaiseBet;
    }

	if( step > 0 )
	{
		raiseAmount = raiseAmount + (raiseAmount - callBet());

		if( raiseAmount < betSize + minRaiseDirect )
		{
			//Two minraises above bet to call
			raiseAmount = betSize + minRaiseDirect;
		}
		--step;
		while(step > 0)
		{
			//You would be raising by {raiseAmount - callBet()};
			raiseAmount = raiseAmount + (raiseAmount - callBet());
			--step;
		}
	}

    if( raiseAmount > maxBet() )
    {
        raiseAmount = maxBet();
    }

	return raiseAmount;
}

void ExactCallD::query(const float64 betSize)
{
    nearest = (betSize <= callBet() + table->GetChipDenom()/2) ? betSize : 0; //nearest can probably be ALWAYS callBet() to start!
    float64 peopleInHandUpper = table->GetNumberInHand() - 1;
    const float64 opponents = handsDealt()-1;
    const float64 myexf = betSize;
    const float64 mydexf = 1;

    float64 overexf = 0;
    float64 overdexf = 0;

    totalexf = table->GetPotSize() - table->ViewPlayer(playerID)->GetBetSize()  +  myexf;

    //float64 lastexf = totalexf;


    totaldexf = mydexf;
    //float64 lastdexf = totaldexf;


    #ifdef DEBUG_TRACE_DEXF
            if( traceOut != 0 ) *traceOut << "Begin Query with mydexf " << mydexf << endl;
    #endif


    noRaiseArraySize = 0;
    while( RaiseAmount(betSize,noRaiseArraySize) < maxBet() )
    {
        ++noRaiseArraySize;
    }
    //This array loops until noRaiseArraySize is the index of the element with RaiseAmount(noRaiseArraySize) == maxBet()
    ++noRaiseArraySize; //Now it's the size of the array

    if( noRaiseChance_A != 0 ) delete [] noRaiseChance_A;
    if( noRaiseChanceD_A != 0 ) delete [] noRaiseChanceD_A;

    noRaiseChance_A = new float64[noRaiseArraySize];
    noRaiseChanceD_A = new float64[noRaiseArraySize];

    for( int32 i=0; i< noRaiseArraySize; ++i)
    {
        noRaiseChance_A[i] = 1;
        noRaiseChanceD_A[i] = 0;
    }


    float64 * nextNoRaise_A = new float64[noRaiseArraySize];
    float64 * nextNoRaiseD_A = new float64[noRaiseArraySize];


    int8 pIndex = playerID;
    table->incrIndex(pIndex);
    while( pIndex != playerID )
    {

        #ifdef DEBUG_TRACE_DEXF
		if( traceOut != 0 )
		{
			*traceOut << endl << "totaldexf is " << totaldexf << " overdexf is " << overdexf << endl;
			*traceOut << "\tPlayer " << (int)pIndex;
		}
		#endif

        float64 nextexf = 0;
        float64 nextdexf = 0;

        for(int32 i=0;i<noRaiseArraySize;++i)
        {
            nextNoRaise_A[i] = 1; //Won't raise (by default)
            nextNoRaiseD_A[i] = 0;
        }

        const Player * withP = table->ViewPlayer(pIndex);
        const float64 oppBetAlready = withP->GetBetSize();
        const float64 oppPastCommit = withP->GetContribution();

        if( table->CanStillBet(pIndex) )
        {///Predict how much the bet will be
            const float64 oppBankRoll = withP->GetMoney();



            if( betSize < oppBankRoll )
            {	//Can still call, at least

                ChipPositionState oppCPS(oppBankRoll,totalexf,oppBetAlready,oppPastCommit);

                if( betSize > callBet() || table->CanRaise(pIndex, playerID) )
                { //The player can raise you if he hasn't called yet, OR you're raising

                    //if( callBet() > 0 && oppBetAlready == callBet() ) bInBlinds = false;

                    float64 prevRaise = 0;
                    ///Check for each raise percentage
                    for( int32 i=0;i<noRaiseArraySize;++i)
                    {
                        const float64 thisRaise = RaiseAmount(betSize,i);
                        const float64 oppRaiseMake = thisRaise - oppBetAlready;
                        if( oppRaiseMake > 0 )
                        {
                            if(thisRaise <= oppBankRoll)
                            {

                                const bool bOppCouldCheck = (betSize == 0) || /*(betSize == callBet())*/(oppBetAlready == betSize); //If oppBetAlready == betSize AND table->CanRaise(pIndex, playerID), the player must be in the blind. Otherwise,  table->CanRaise(pIndex, playerID) wouldn't hold
                                float64 w_r_mean = facedOdds_raise_Geom(oppCPS,oppRaiseMake, betSize, opponents,bOppCouldCheck,e);
                                float64 w_r_rank = facedOdds_raise_Geom(oppCPS,oppRaiseMake, betSize, opponents,bOppCouldCheck,0);
                                #ifdef ANTI_CHECK_PLAY
                                if( bOppCouldCheck )
                                {
                                    w_r_mean = 1;
                                    w_r_rank = 1;
                                }
                                #endif



                                const float64 noraiseRankD = dfacedOdds_dpot_GeomDEXF( oppCPS,oppRaiseMake,callBet(), w_r_rank, opponents, totaldexf, bOppCouldCheck,0);

                                const float64 noRaiseMean = 1-e->Pr_haveWinPCT_orbetter(w_r_mean);
                                const float64 noraiseMeanD = -e->d_dw_only(w_r_mean) * dfacedOdds_dpot_GeomDEXF( oppCPS,oppRaiseMake,callBet(),w_r_mean, opponents,totaldexf,bOppCouldCheck,e);

                                //nextNoRaise_A[i] = w_r_rank;
                                //nextNoRaiseD_A[i] = noraiseRankD;

                                nextNoRaise_A[i] = (noRaiseMean+w_r_rank)/2;
                                nextNoRaiseD_A[i] = (noraiseMeanD+noraiseRankD)/2;

                                prevRaise = thisRaise;
                            }else
                            {
                                const float64 oppAllInMake = oppBankRoll - oppBetAlready;

                                if (prevRaise > 0 && oppBankRoll > prevRaise && oppAllInMake > 0)
                                { //This is the all-in one.

                                    float64 w_r = 1-1.0/RAREST_HAND_CHANCE;

                                    //float64 noraiseMean = e->weakestOpponent(); //What are his odds against the weakest opponent

                                    //if(noraiseMean < w_r){ noraiseMean = 0; }

                                    nextNoRaise_A[i] = w_r;//(noraiseMean+w_r)/2;
                                    nextNoRaiseD_A[i] = 0;
                                }

                                prevRaise = 0;

                            }
                        }else{ prevRaise = 0; }
                    }//end of for loop
                }



				///Check for most likely call amount
                const float64 oppBetMake = betSize - oppBetAlready;
				//To understand the above, consider that totalexf includes already made bets

                #ifdef DEBUG_TRACE_DEXF
                if( traceOut != 0 )  *traceOut << " to bet " << oppBetMake << "more";
                #endif

                if( oppBetMake <= 0 )
                { //Definitely call
                    nextexf = 0;
                    nextdexf = 1;

                    #ifdef DEBUG_TRACE_DEXF
                    if( traceOut != 0 )  *traceOut << " ALREADY CALLED" << endl ;
                    #endif
                }else
                {


                    const float64 w = facedOdds_call_Geom(oppCPS,betSize, opponents, e);
                    nextexf = e->Pr_haveWinPCT_orbetter(w);
                    peopleInHandUpper -= 1-nextexf;

                    nextdexf = nextexf + oppBetMake * e->d_dw_only(w)
                                        * dfacedOdds_dbetSize_Geom(oppCPS,betSize,totaldexf,w, opponents, e);

                    nextexf *= oppBetMake;
                    if( oppBetAlready + nextexf > nearest )
                    {
                        nearest = oppBetAlready + nextexf;
                    }

                    #ifdef DEBUG_TRACE_DEXF
                    //if( traceOut != 0 )  *traceOut << " nextdexf=" << nextdexf << endl;
                    #endif

                }
				//End of else, blocked executed UNLESS oppBetMake <= 0

            }else
            {///Opponent would be all-in to call this bet
                const float64 oldpot = table->GetPrevPotSize();
                const float64 effroundpot = (totalexf - oldpot) * oppBankRoll / betSize;
                const float64 oppBetMake = oppBankRoll - oppBetAlready;

                ChipPositionState oppmaxCPS(oppBankRoll,oldpot + effroundpot,oppBetAlready,oppPastCommit);

                nextexf = e->Pr_haveWinPCT_orbetter( facedOdds_call_Geom(oppmaxCPS,oppBankRoll, opponents,e) );
                peopleInHandUpper -= 1-nextexf;
                nextexf *= oppBetMake ;

                if( oppBetAlready + nextexf + (betSize - oppBankRoll) > nearest )
                {
                    nearest = oppBetAlready + nextexf + (betSize - oppBankRoll);
                }

                nextdexf = 0;

                    #ifdef DEBUG_TRACE_DEXF
                    if( traceOut != 0 )  *traceOut << " Is ALL IN" << endl;
                    #endif

				//Obviously the opponent won't raise...  ie. NoRaise = 100%
				// (nextNoRaise , nextNoRaiseD ) is already (1,0)
            }


            //lastexf = nextexf;
            totalexf += nextexf;

                    #ifdef DEBUG_TRACE_DEXF
                    //if( traceOut != 0 )  *traceOut << "totaldexf was " << totaldexf;
                    #endif

            //lastdexf = nextdexf;
            totaldexf += nextdexf;


                    #ifdef DEBUG_TRACE_DEXF
                    //if( traceOut != 0 )  *traceOut << " is " << totaldexf << ",  last added " << nextdexf << endl;
                    //if( traceOut != 0 )  *traceOut << " is " << totaldexf << endl;
                    //if( traceOut != 0 )  *traceOut << " last added " << nextdexf << endl;
                    //if( traceOut != 0 )  *traceOut << endl;
                    #endif


            const float64 oppInPot = oppBetAlready + nextexf;
            if( oppInPot - betSize > DBL_EPSILON )
            {
                overexf += oppInPot - betSize;
                overdexf += nextdexf;
            }

        }

        for( int32 i=0;i<noRaiseArraySize;++i)
        {
            //At this point, each nextNoRaise is 100% unless otherwise adjusted.
            noRaiseChance_A[i] *= (nextNoRaise_A[i] < 0) ? 0 : nextNoRaise_A[i];
            if( noRaiseChance_A[i] == 0 ) //and nextNoRaiseD == 0
            {
                noRaiseChanceD_A[i] = 0;
            }else
            {
                noRaiseChanceD_A[i] += nextNoRaiseD_A[i]/nextNoRaise_A[i]; //Logairthmic differentiation
            }
        }


        table->incrIndex(pIndex);
    }

    delete [] nextNoRaise_A;
    delete [] nextNoRaiseD_A;

    #ifdef DEBUG_TRACE_DEXF
    if( traceOut != 0 )  *traceOut << endl << "Final is " << totaldexf;
    #endif

    totalexf = totalexf - myexf - overexf;
    totaldexf = totaldexf - mydexf - overdexf;

    #ifdef DEBUG_TRACE_DEXF
    if( traceOut != 0 )  *traceOut << " adjusted to " << totaldexf << " by mydexf=" << mydexf << " and overdexf=" << overdexf << endl;
    #endif

    if( totalexf < 0 ) totalexf = 0; //Due to rounding error in overexf?
    if( totaldexf < 0 ) totaldexf = 0; //Due to rounding error in overexf?

    for( int32 i=0;i<noRaiseArraySize;++i)
    {
        noRaiseChanceD_A[i] *= noRaiseChance_A[i];
    }

}

void ExactCallBluffD::query(const float64 betSize)
{
    ExactCallD::query(betSize);

    float64 countMayFold = table->GetNumberInHand() - 1 ;


    const float64 myexf = betSize;
    const float64 mydexf = 1;

#ifdef DEBUG_TRACE_PWIN
		if( traceOut != 0 ) *traceOut << "Begin Query" << endl;
#endif

    float64 nextFold = -1;
    float64 nextFoldPartial = 0;
    allFoldChance = 1;
    allFoldChanceD = 0;
    const float64 origPot = table->GetPotSize() - table->ViewPlayer(playerID)->GetBetSize()  +  myexf;
    const float64 origPotD = mydexf;


    if( betSize < minRaiseTo() - chipDenom()/4 || callBet() >= table->GetMaxShowdown(table->ViewPlayer(playerID)->GetMoney()) )
    { //Bet is a call, no chance of oppFold
        allFoldChance = 0;
        allFoldChanceD = 0;
        nextFold = 0;
        nextFoldPartial = 0;

		#ifdef DEBUG_TRACE_PWIN
		    if( traceOut != 0 ) *traceOut << "N/A. Call doesn't pWin" << endl;
		#endif
    }

    int8 pIndex = playerID;
    table->incrIndex(pIndex);
    while( pIndex != playerID && allFoldChance > 0)
    {
		#ifdef DEBUG_TRACE_PWIN
		if( traceOut != 0 )
		{
			*traceOut << "allFoldChance is " << allFoldChance << ",  last multiplied by " << nextFold << endl;
			*traceOut << "\tPlayer " << (int)pIndex;
		}
		#endif

        const float64 oppBetAlready = table->ViewPlayer(pIndex)->GetBetSize();
        const float64 oppPastCommit = table->ViewPlayer(pIndex)->GetContribution();

        const float64 nLinear =/* 1/countToBeat ;*/ (countMayFold + insuranceDeterrent);

        if( table->CanStillBet(pIndex) )
        {///Predict how much the bet will be
            const float64 oppBankRoll = table->ViewPlayer(pIndex)->GetMoney();

		#ifdef DEBUG_TRACE_PWIN
		    if( traceOut != 0 ) *traceOut << " can still bet" << endl;
		#endif

            if( betSize < oppBankRoll )
            {

		#ifdef DEBUG_TRACE_PWIN
		    if( traceOut != 0 ) *traceOut << "\t\tCan raise" << endl;
		#endif

                ChipPositionState opporigCPS(oppBankRoll,origPot,oppBetAlready,oppPastCommit);
                const float64 oppBetMake = betSize - oppBetAlready;
                //To understand the above, consider that totalexf includes already made bets


                float64 w_rank = facedOdds_Algb(opporigCPS,oppBetMake,nLinear,0);
                float64 w_mean = facedOdds_Algb(opporigCPS,oppBetMake,nLinear,e);
                if( nLinear <= 0 )
                {
                    w_mean = 1;
                    w_rank = 0;
                }
                const float64 dw_dbetSize_mean = facedOddsND_Algb( opporigCPS,oppBetMake,origPotD,w_mean, nLinear);
                const float64 dw_dbetSize_rank = facedOddsND_Algb( opporigCPS,oppBetMake,origPotD,w_rank, nLinear);


                    //float64 oppCommitted = stagnantPot() - table->ViewPlayer(pIndex)->GetContribution();
                    //oppCommitted = oppCommitted / (oppCommitted + oppBankRoll);
                    //ea-> is if they know your hand
                    const float64 eaFold = (1 - ea->Pr_haveWinPCT_orbetter_continuous( w_mean ));// *(1 - oppCommitted);
                    //e-> is if they don't know your hand
                    const float64 meanFold = 1 - e->Pr_haveWinPCT_orbetter( w_mean );
                    //w is if they don't know your hand
                    const float64 rankFold = w_rank;
                    //handRarity is based on if they know your hand
                    const float64 eaRkFold = 1-handRarity;

					#ifdef DEBUG_TRACE_PWIN
						if( traceOut != 0 )
						{
						    *traceOut << "\t\tWillFold (eaFold,meanFold,rankFold,eaRkFold) = (" << eaFold << "," << meanFold << "," << rankFold << "," << eaRkFold << ")" << endl;
						    *traceOut << "\t\t\tusing w_rank = " << w_rank << endl;
						}

					#endif

//(nLinear <= 0 && wn > 1) ? 0 :


                    const float64 rankFoldPartial = dw_dbetSize_rank;
                    const float64 meanFoldPartial = -e->d_dw_only( w_mean ) * dw_dbetSize_mean;
                    const float64 eaFoldPartial = -ea->d_dw_only( w_mean ) * dw_dbetSize_mean;
                    const float64 eaRkFoldPartial = 0;

                    ///topTwoOfThree is on a player-by-player basis
                    nextFoldPartial = bottomThreeOfFour(eaFold,meanFold,rankFold,eaRkFold,eaFoldPartial,meanFoldPartial,rankFoldPartial,eaRkFoldPartial,nextFold);
                    //nextFold = (eaFold+meanFold+rankFold+eaRkFold)/4;
                    //nextFoldPartial=(eaFoldPartial+meanFoldPartial+rankFoldPartial+eaRkFoldPartial)/4;

                    //nextFold = sqrt((eaFold*eaFold+rankFold*rankFold)/2);
                    //nextFoldPartial = (eaFold*eaFoldPartial+rankFold*rankFoldPartial)*sqrt(2)/nextFold ;
                    //nextFold = (meanFold+rankFold+eaFold)/3;
                    //nextFoldPartial = (meanFoldPartial+rankFoldPartial+eaFoldPartial)/3;

					#ifdef DEBUG_TRACE_PWIN
						if( traceOut != 0 ) *traceOut << "\t\tPicked = (" << nextFold << ")" << endl;
					#endif


            }else
            {///Opponent would be all-in to call this bet

				#ifdef DEBUG_TRACE_PWIN
					if( traceOut != 0 ) *traceOut << "\t\tShort stacked" << endl;
				#endif

                const float64 oldpot = table->GetPrevPotSize();
                const float64 effroundpot = (origPot - oldpot) * oppBankRoll / betSize;
                const float64 oppBetMake = oppBankRoll - oppBetAlready;

                if( oppBankRoll < minRaiseTo() - chipDenom()/4 )
                { //Guaranteed call
                    allFoldChance = 0;
                    allFoldChanceD = 0;
                    nextFold = 0;
                }else
                {
                    ChipPositionState opporigmaxCPS(oppBankRoll,oldpot + effroundpot,oppBetAlready,oppPastCommit);

                    float64 w_mean = facedOdds_Algb(opporigmaxCPS,oppBetMake, nLinear,e);
                    float64 w_rank = facedOdds_Algb(opporigmaxCPS,oppBetMake, nLinear,0);

                    if( nLinear <= 0 )
                    {
                        w_mean = 1;
                        w_rank = 1;
                    }

                    //float64 oppCommitted = table->ViewPlayer(pIndex)->GetContribution();
                    //oppCommitted = oppCommitted / (oppCommitted + oppBankRoll);
                    const float64 eaFold = (1 - ea->Pr_haveWinPCT_orbetter_continuous( w_mean ));//*(1 - oppCommitted);
                    const float64 meanFold = 1 - e->Pr_haveWinPCT_orbetter( w_mean );
                    const float64 rankFold = w_rank;
                    const float64 eaRkFold = 1-handRarity;

                    ///topTwoOfThree is on a player-by-player basis
                    bottomThreeOfFour(eaFold,meanFold,rankFold,eaRkFold,0,0,0,0,nextFold);
//                    nextFold = (eaFold+meanFold+rankFold+eaRkFold)/4;


                    //nextFold = (meanFold+rankFold+eaFold)/3;
                    //nextFold = sqrt((eaFold*eaFold+rankFold*rankFold)/2);

                    if( nextFold > 1 ) nextFold = 1;

                }
                nextFoldPartial = 0;
            }


            countMayFold -= 1;
//            countToBeat  += 1;

            if(
               (allFoldChance == 0 && allFoldChanceD == 0) || (nextFold == 0 && nextFoldPartial == 0)
              )
            {
                allFoldChance = 0;
                allFoldChanceD = 0;
            }else
            {
                allFoldChance *= nextFold;
                allFoldChanceD += nextFoldPartial / nextFold;
            }


        }else
        {//Player can't bet anymore

            #ifdef DEBUG_TRACE_PWIN
		        if( traceOut != 0 ) *traceOut << " ignored" << endl;
		    #endif

            if( oppBetAlready > 0 )
            {//Must have gone all-in just now, or at least this round
                #ifdef DEBUG_TRACE_PWIN
		            if( traceOut != 0 ) *traceOut << "N/A! All-in detected." << endl;
		        #endif

                allFoldChance = 0;
                allFoldChanceD = 0;
            }
        }


        table->incrIndex(pIndex);
    }

    allFoldChanceD *= allFoldChance;

}

float64 ExactCallBluffD::PushGain()
{
    const float64 baseFraction = betFraction(table->GetPotSize() - alreadyBet());

#ifdef BLIND_ADJUSTED_FOLD
    const float64 rawWinFreq = (1.0 / table->GetNumberAtTable()) ;
    const float64 blindsPow = rawWinFreq*(1.0 - 1.0 / table->GetNumberAtTable());

    const float64 bigBlindFraction = betFraction( table->GetBigBlind() );
    const float64 smallBlindFraction = betFraction( table->GetSmallBlind() );
#ifdef CONSISTENT_AGG
    const float64 handFreq = 1+handRarity;//1/(1-handRarity);
    if( handRarity >= 1 ) //all hands are better than this one
    {
           //1326 is the number of hands possible
//		#ifdef PURE_BLUFF
            return 1;

//		#else
//			return (1 + baseFraction + (bigBlindFraction+smallBlindFraction)*blindsPow*RAREST_HAND_CHANCE);
//		#endif
    }
	#ifdef PURE_BLUFF
        const float64 //totalFG;
        //if( blindsPow*handFreq > 1 )
        //{
            totalFG = (1 + (baseFraction+(bigBlindFraction+smallBlindFraction)*blindsPow)*handFreq);
        //}else
        //{
        //    totalFG = (1 + baseFraction+(bigBlindFraction+smallBlindFraction*blindsPow)*handFreq);
        //}
	#else
		const float64 totalFG = (1 + baseFraction + ((bigBlindFraction+smallBlindFraction)*blindsPow)*handFreq);
	#endif

#else
    const float64 blindsGain = (1 + baseFraction + bigBlindFraction)*(1 + baseFraction + smallBlindFraction);
        //const float64 blindPerHandGain = ( ViewTable().GetBigBlind()+ViewTable().GetSmallBlind() ) / myMoney / ViewTable().GetNumberAtTable();
    const float64 totalFG = pow(1+baseFraction,1-2*blindsPow)*pow(blindsGain,blindsPow);

#endif



    return totalFG;
#else
    return 1 + baseFraction;
#endif
}

float64 ExactCallBluffD::pWin(const float64 betSize)
{

    if( queryinputbluff != betSize )
    {
        query(betSize);
        queryinput = betSize;
        queryinputbluff = betSize;
    }

    ///Try pow(,impliedFactor) maybe
    return allFoldChance;//*impliedFactor;
}

float64 ExactCallBluffD::pWinD(const float64 betSize)
{
    if( queryinput != betSize )
    {
        query(betSize);
        queryinput = betSize;
    }
    return allFoldChanceD;//*impliedFactor;
}


float64 ExactCallD::pRaise(const float64 betSize, const int32 step)
{
    if( queryinput != betSize )
    {
        query(betSize);
        queryinput = betSize;
    }

    if( step < noRaiseArraySize ) return 1-noRaiseChance_A[step];
    else return -1;
}

float64 ExactCallD::pRaiseD(const float64 betSize, const int32 step)
{
    if( queryinput != betSize )
    {
        query(betSize);
        queryinput = betSize;
    }

    if( step < noRaiseArraySize ) return -noRaiseChanceD_A[step];
    else return 0;
}




float64 ExactCallD::exf(const float64 betSize)
{
    //if( queryinput == UNINITIALIZED_QUERY )
    if( queryinput != betSize )
    {
        query(betSize);
        queryinput = betSize;
    }
    return totalexf*impliedFactor + (betSize - nearest);
}

float64 ExactCallD::dexf(const float64 betSize)
{
//    if( query == UNINITIALIZED_QUERY )
    if( queryinput != betSize )
    {
        query(betSize);
        queryinput = betSize;
    }
    return totaldexf*impliedFactor;
}


float64 ExactCallD::ActOrReact(float64 callb, float64 lastbet, float64 limit) const
{
//One must consider the possibilities:
//1. [ACT] The player betting has been folding all this time, and has hit his/her hand ALREADY
//2. [ACT] The opponents have not bet yet, and would be the reactors of this hand.
//3. [REACT] The pot is large from previous rounds, opponents can't fold easily

    //const float64 avgControl = (stagnantPot() + table->GetUnbetBlindsTotal()) / table->GetNumberInHand();
    //const float64 raiseOverBet = (callb + avgControl);// < lastbet) ? 0 : (callb + avgControl - lastbet) ;

    const float64 raiseOverOthers = (table->GetPotSize() - callb) / table->GetNumberInHand();
    const float64 raiseOver = (raiseOverOthers);// + raiseOverBet)/2;
    const float64 actOrReact = (raiseOver > limit) ? 1 : (raiseOver / limit);
    return actOrReact;
}

float64 ExactCallBluffD::RiskPrice()
{
    //const float64 N = table->GetNumberAtTable();
	const int8 Ne_int = table->GetNumberInHand() - 1;
    const float64 Ne = static_cast<float64>(Ne_int);

    const float64 estSacrifice = (table->GetPotSize() - table->ViewPlayer(playerID)->GetBetSize());

    const float64 maxStack = table->GetAllChips();
    const float64 maxShowdown = table->GetMaxShowdown(table->ViewPlayer(playerID)->GetMoney());

	//FG.bTraceEnable = true;

    FG.waitLength.w = ea->nearest_winPCT_given_rank(1.0 - 1.0/Ne); //If you're past the flop, we need definitely consider only the true number of opponents
    FG.waitLength.amountSacrifice = estSacrifice; //rarity() already implies the Ne
    FG.waitLength.bankroll = maxStack;
    FG.waitLength.opponents = 1;
    FG.waitLength.meanConv = ea; //TODO: Is this a good idea?
    const float64 riskprice = FG.FindZero(table->GetMinRaise() + callBet(),maxShowdown);

    FG.f(riskprice);

    if( FG.n > 0 )
    {
        return riskprice;
    }else
    {
        return maxShowdown;
    }


}

