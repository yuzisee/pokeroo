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


#ifndef log1p
#define log1p( _X_ ) log( (_X_) + 1 )
#endif


///In the end, we had to stick with Algb for foldgain. (At low blinds, oppfold is guaranteed in Geom)
//#define GEOM_FOLD_PCT
//#define CALL_SCALE_FOLD_PCT
#define CALL_ALGB_PCT


#define NO_AUTO_RAISE



//#define ANTI_CHECK_PLAY

#if defined(GEOM_FOLD_PCT) && defined(CALL_SCALE_FOLD_PCT)
Don't define BOTH FOLD_PCT types
#endif

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
ExactCallFunctionModel::~ExactCallFunctionModel(){};

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


/*
    //oppBetMake / (oppBetMake + totalexf)
    const int8 N = handsDealt();
    const float64 fNRank = (wGuess >= 1) ? 1.0/RAREST_HAND_CHANCE : (1 - e->pctWillCall(1 - wGuess));
    const float64 avgBlind = ( alreadyBet + (table->GetBigBlind() + table->GetBigBlind()) / N )
                                        * ( N - 2 )/ N ;

    return (
            log((1 - avgBlind / fNRank ) / (1 - bet))
            /
            log((1+pot) / (1 - bet))
           );
*/

///Geom returns stricly call percentage (no check)
float64 ExactCallD::facedOdds_Geom(float64 bankroll, float64 pot, float64 alreadyBet, float64 incrbet, float64 n, bool bCheckPossible)
{

    const int8 N = handsDealt();
    const float64 avgBlind = (
    #ifdef PURE_BLUFF
    alreadyBet +
    #endif
            (table->GetBigBlind() + table->GetBigBlind()) / N )
            * ( N - 2 )/ N ;

    //geomFunction.quantum = 1 / RAREST_HAND_CHANCE / 2.0;

    geomFunction.Bankroll = bankroll;
    geomFunction.pot = pot-alreadyBet;


    geomFunction.bet = incrbet+alreadyBet;
    if( geomFunction.bet > bankroll )
    {
        geomFunction.bet = bankroll;
    }
    geomFunction.alreadyBet = alreadyBet;
    geomFunction.avgBlind = avgBlind;
    geomFunction.bOppCheck = bCheckPossible;
    geomFunction.n = n;

	 #ifdef DEBUG_CALLPRED_FUNCTION
	if(bet < 0)
	{
            std::ofstream excel( "callmodel.csv" );
            if( !excel.is_open() ) std::cerr << "\n!callmodel.cvs file access denied" << std::endl;
            geomFunction.breakdown(1000,excel,0,1);
            //myExpectedCall.breakdown(0.005,excel);

            excel.close();
	}
        #endif

    const float64 g = geomFunction.FindZero( 0,1 );

    return g;

}
//1. If Geom all-in, return based on alreadyBet?
//2. If k is stretched larger than B because of wGuess, so that w would be negative? Return w=0;
//3. Is bankroll supposed to be passed in as (oppBankRoll - alreadyBet) or not? (Is betSize passed in as oppBetMake or not?)
//Is Algb different than Geom for point #3?
float64 ExactCallFunctionModel::f( const float64 w )
{
    const float64 fw = pow(w,n);

	const float64 frank = e->pctWillCall(1 - w);
    const float64 fNRank = (frank >= 1) ? 1.0/RAREST_HAND_CHANCE : (1 - frank);
    //fNRank == 0 and frank == 1? That means if you had this w pct, you would have the nuts or better.
    //const float64 potwin = bOppCheck ? ( 0 ) : (pot);

    const float64 gainFactor = pow( Bankroll+pot , fw ) * pow( Bankroll-bet , 1-fw ) - Bankroll;

    const float64 stackFactor = alreadyBet + avgBlind / fNRank;

    if( bOppCheck )
    {
        return gainFactor;
    }else
    {
        return gainFactor + stackFactor;
    }
}

float64 ExactCallFunctionModel::fd( const float64 w, const float64 y )
{
    const float64 fw = pow(w,n);
    const float64 dfw = (n<0.5) ? (0) : (n * pow(w,n-1));

    const float64 frank = e->pctWillCall(1 - w);
    const float64 fNRank = (frank >= 1) ? 1.0/RAREST_HAND_CHANCE : (1 - frank);

    const float64 stackFactor = avgBlind * e->pctWillCallD(1-w) / fNRank / fNRank;

    const float64 gainFactor = (Bankroll - bet)
                                * pow(  (Bankroll+pot)/(Bankroll-bet)  ,  fw  )
                                * log1p( (Bankroll+pot)/(Bankroll-bet) - 1)
                                * dfw;


    if( bOppCheck )
    {
        return gainFactor;
    }else
    {
        return gainFactor - stackFactor;
    }
}

float64 ExactCallD::facedOddsND_Geom(float64 bankroll, float64 pot, float64 alreadyBet, float64 bet, float64 dpot, float64 w, float64 n, bool bCheckPossible)
{
	if( w <= 0 ) return 0;
	if( bet >= bankroll ) return 0;

    const int8 N = handsDealt();
    float64 avgBlind = ( alreadyBet + (table->GetBigBlind() + table->GetBigBlind()) / N )
            * ( N - 2 )/ N ;

    const float64 fw = pow(w,n);
    const float64 dfw = (n<0.5) ? (0) : (n * pow(w,n-1));

    if( bCheckPossible )
    {
        if( dfw == 0 )
        {
            return 0;
        }
        avgBlind = 0;
    }

    const float64 frank = e->pctWillCall(1 - w);
    const float64 fNRank = (frank >= 1) ? 1.0/RAREST_HAND_CHANCE : (1 - frank);



    const float64 A = dfw * log1p( (bankroll+pot) / (bankroll - bet) - 1 );
    const float64 C = (dpot/(bankroll+pot) + 1/(bankroll-bet))
                        *
                        fw
                       ;


    const float64 h = pow( (bankroll+pot) / (bankroll - bet) , fw );

    const float64 dwdbet = (C * (1-bet) - 1)
                            /
                           (
                                (avgBlind/h)*( avgBlind * e->pctWillCallD(1-w) ) / fNRank / fNRank
                                -
                                A * (1-bet)
                           )
                         ;

    return dwdbet;

}


float64 ExactCallD::facedOdds_Algb_step(float64 bankroll, float64 pot, float64 alreadyBet, float64 incrbet, bool bRank, float64 wGuess)
{
    //pot -= alreadyBet/2; //Half their bet is implied odds
    pot -= alreadyBet;

    float64 bet = alreadyBet + incrbet;
    if( bet > bankroll )
    {
        bet = bankroll;
    }
    //oppBetMake / (oppBetMake + totalexf)
    const int8 N = handsDealt();

    const float64 frank = bRank ? wGuess : e->pctWillCall(1 - wGuess);
    const float64 fNRank = (frank >= 1) ? 1.0/RAREST_HAND_CHANCE : (1 - frank);

    const float64 avgBlind = (table->GetBigBlind() + table->GetBigBlind()) * ( N - 2 )/ N / N;

    const float64 foldCorrectly = frank * (bet-alreadyBet);

    const float64 ret =
           (
            (bet - alreadyBet + foldCorrectly - avgBlind / fNRank ) / (bet + pot)
           );

    if (ret < 0) return 0;
    return ret;
}

///Algb returns check OR call percentage
float64 ExactCallD::facedOdds_Algb(float64 bankroll, float64 pot, float64 alreadyBet, float64 bet, float64 sig, bool bRank)
{
    float64 max = 1;
    float64 min = 0;
    float64 useOdds;
    float64 newOdds = pow(facedOdds_Algb_step(bankroll,pot,alreadyBet,bet,bRank,0),sig);
    float64 curOdds = newOdds*(1-sig);
    float64 prevOdds = 0;
    float64 cycleDetection = -1;
    float64 cycleQuantum = 0.5 * chipDenom() / allChips() * sig;
    do
    {
        if( newOdds > max ) newOdds = max;
        if( newOdds < min ) newOdds = min;

        if( (newOdds-curOdds)*(curOdds-prevOdds) <= 0 )
        {   //new terms are converging in alternate directions
            prevOdds = curOdds;
            curOdds = newOdds;
            useOdds = (max+min)/2;
            newOdds = pow(facedOdds_Algb_step(bankroll,pot,alreadyBet,bet,bRank,useOdds),sig)*sig + useOdds*(1-sig);
            if( fabs(cycleDetection - newOdds) < cycleQuantum )
            {
                return (newOdds+prevOdds+curOdds)/3;
            }
            if( max - min < cycleQuantum )
            {
                return (max+min)/2;
            }
        }else
        {
            prevOdds = curOdds;
            curOdds = newOdds;
            useOdds = curOdds;
            newOdds = (pow(facedOdds_Algb_step(bankroll,pot,alreadyBet,bet,bRank,useOdds),sig)*sig + useOdds*(1-sig));
        }

        if( newOdds < useOdds && max > useOdds)
        {//Decreased from useOdds
            max = useOdds;
        }else if( newOdds > useOdds && min < useOdds)
        {
            min = useOdds;
        }

        cycleDetection = prevOdds;
    }while( fabs(useOdds - newOdds) > cycleQuantum );
    return newOdds;
}

float64 ExactCallD::facedOddsND_Algb(float64 bankroll, float64 pot, float64 alreadyBet, float64 incrbet, float64 dpot, float64 w, float64 n, bool bRank)
{
    float64 bet = alreadyBet + incrbet;
    if( bet > bankroll )
    {
        bet = bankroll;
    }
	//TODO: Really?
	//Approximate at the limit. It should be about linear in this region anyways, and we probably only
	//hit this case when there is no zero in the range [0..1].
	//if( w <= 1.0/RAREST_HAND_CHANCE / 4 ) w = 1.0/RAREST_HAND_CHANCE / 4;
	if( w <= 0 ) return 0;
	if( n < 0.5 ) return 0;

    const int8 N = handsDealt();
    const float64 frank = bRank ? w : e->pctWillCall(1 - w);
	const float64 dfrank = bRank ? -1 : e->pctWillCallD(1-w);
    const float64 fNRank = (frank >= 1) ? 1.0/RAREST_HAND_CHANCE : (1 - frank);
    const float64 avgBlind = (table->GetBigBlind() + table->GetBigBlind()) * ( N - 2 )/ N / N;

    const float64 fw = pow(w,n);
    const float64 dfw = (n<0.5) ? (0) : (n * pow(w,n-1));

    const float64 DfoldCorrectly = frank;
    const float64 DfoldCorrectlyDw = (bet - alreadyBet) * dfrank;
    //    (pot - bet * dpot)
    //    /(bet + pot) /(bet + pot);

    //to
    /*
    =
    {1 - ( dpot + 1 ) f(w) }
    over

            (
                ( pot + betSize ) f'(w)
                +
                    {avgBlind * pctWillCall' ( 1 - w ) }
                    over
                    ( 1 - pctWillCall ( 1 - w ) )^2
            )
    */



    return (
                (1 - fw*(dpot + 1) + DfoldCorrectly)
                /
                (   (pot+bet)*dfw
                    +
                    ( avgBlind * dfrank ) / fNRank / fNRank
                    - DfoldCorrectlyDw
                )
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

void ExactCallD::GenerateRaiseChances(float64 raisebet, float64 noraiseRank_prescaled, float64 noraiseRankD_prescaled, const Player * withP, float64 sig, float64 liveOpp, float64 & out, float64 & outD)
{
        const float64 upperlimit = table->GetMaxShowdown();// RiskPrice();
        const float64 uReact = ActOrReact(callBet(),withP->GetBetSize(),upperlimit);
        const float64 dist = raisebet / table->GetMaxShowdown();
        const float64 percentReact = (dist > 1) ? uReact : (1-(1-uReact)*dist);
        const float64 needed_pcw = pow(noraiseRank_prescaled,1/sig);

        const float64 extra_pcw = pow(noraiseRank_prescaled,1/sig-liveOpp);
        const float64 p_cw_draw = extra_pcw*pow(e->strongestOpponent().pct*noraiseRank_prescaled,liveOpp);
        //needed_pcw = f(noraiseRank) * percentReact + p_cw_draw * (1 - percentReact)
        //f(noraiseRank) = (needed_pcw - p_cw_draw * (1 - percentReact))  /  percentReact;
        //Note: percentReact can be zero if you are in the blind, since your lastbet == betToCall
        //       Also, dist has to be 1
        const float64 a = (needed_pcw - p_cw_draw * (1 - percentReact));
        const float64 pcw_to_win = a  /  percentReact;


        if( pcw_to_win > 1 || percentReact <= 0 )
        {
            out = 1;
            outD = 0;
            return;
        }else if( p_cw_draw > needed_pcw || pcw_to_win <= 0 )
        {
            out = 0;
            outD = 0;
            return;
        }


        const float64 noraiseRank = pow(pcw_to_win,sig);
        const float64 noraiseRankD = pow(pow(1/a,sig)*noraiseRank_prescaled,1/sig-1)/pow(percentReact,sig)*noraiseRankD_prescaled;

        const float64 noraiseMean = 1 - e->pctWillCall( noraiseRank );
        const float64 noraiseMeanD = - e->pctWillCallD(  noraiseRank  )  * noraiseRankD;
//Crappy raiseGain effect
        float64 pushChips = - withP->GetContribution() - withP->GetBetSize() + table->GetPotSize();//stagnantPot();
        pushChips = pushChips / (pushChips + withP->GetMoney());

        #ifdef NO_AUTO_RAISE
        if( noraiseMean > noraiseRank )
        { //Rank is more likely to raise
        #endif
            out = noraiseMean*(1-pushChips) + noraiseRank*(pushChips);
            outD = noraiseMeanD*(1-pushChips) + noraiseRankD*(pushChips);
        #ifdef NO_AUTO_RAISE
        }else
        { //Rank is more likely NOT to raise
            out = noraiseRank*(1-pushChips) + noraiseMean*(pushChips);
            outD = noraiseRankD*(1-pushChips) + noraiseMeanD*(pushChips);
        }
        #endif

        if( out > 1 )
        {
            out = 1;
            outD = 0;
        }

}

void ExactCallD::query(const float64 betSize)
{
    nearest = (betSize <= callBet() + table->GetChipDenom()/2) ? betSize : 0;
    float64 peopleInHandUpper = table->GetNumberInHand() - 1;
    const float64 significance = 1/static_cast<float64>( handsDealt()-1 );
    const float64 myexf = betSize;
    const float64 mydexf = 1;

    float64 overexf = 0;
    float64 overdexf = 0;

    totalexf = table->GetPotSize() - table->ViewPlayer(playerID)->GetBetSize()  +  myexf;

    //float64 lastexf = totalexf;


    totaldexf = mydexf;
    //float64 lastdexf = totaldexf;



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
        float64 nextexf = 0;
        float64 nextdexf = 0;

        for(int32 i=0;i<noRaiseArraySize;++i)
        {
            nextNoRaise_A[i] = 1; //Won't raise (by default)
            nextNoRaiseD_A[i] = 0;
        }

        const Player * withP = table->ViewPlayer(pIndex);
        const float64 oppBetAlready = withP->GetBetSize();

        if( table->CanStillBet(pIndex) )
        {///Predict how much the bet will be
            const float64 oppBankRoll = withP->GetMoney();

            if( betSize < oppBankRoll )
            {	//Can still call, at least

                if( betSize > callBet() || table->CanRaise(pIndex, playerID) )
                { //The player can raise you if he hasn't called yet, OR you're raising

                    //if( callBet() > 0 && oppBetAlready == callBet() ) bInBlinds = false;

                    ///Check for each raise percentage
                    for( int32 i=0;i<noRaiseArraySize;++i)
                    {
                        const float64 thisRaise = RaiseAmount(betSize,i);
                        const float64 oppRaiseMake = thisRaise - oppBetAlready;
                        if( oppRaiseMake > 0 && thisRaise <= oppBankRoll )
                        {
                            const bool bOppCouldCheck = (betSize == 0) || /*(betSize == callBet())*/(oppBetAlready == betSize); //If oppBetAlready == betSize AND table->CanRaise(pIndex, playerID), the player must be in the blind. Otherwise,  table->CanRaise(pIndex, playerID) wouldn't hold
                            float64 w_r = facedOdds_Geom(oppBankRoll,totalexf,oppBetAlready,oppRaiseMake, 1/significance,bOppCouldCheck);
                            #ifdef ANTI_CHECK_PLAY
                            if( bOppCouldCheck )
                            {
                                w_r = 1;
                            }
                            #endif




                            const float64 noraiseRank = w_r;
                            const float64 noraiseRankD = facedOddsND_Geom( oppBankRoll,totalexf,oppBetAlready,oppRaiseMake,totaldexf,w_r, 1/significance, false );

                            GenerateRaiseChances(thisRaise,noraiseRank,noraiseRankD,withP,significance,peopleInHandUpper,nextNoRaise_A[i],nextNoRaiseD_A[i]);

                        }
                    }
                }



				///Check for most likely call amount
                const float64 oppBetMake = betSize - oppBetAlready;
				//To understand the above, consider that totalexf includes already made bets

                if( oppBetMake <= 0 )
                { //Definitely call
                    nextexf = 0;
                    nextdexf = 1;
                }else
                {
#ifdef DEBUGWATCHPARMS
                    const float64 vodd = pow(  oppBetMake / (oppBetMake + totalexf), significance);
                    const float64 willCall = e->pctWillCall( pow(  oppBetMake / (oppBetMake + totalexf)  , significance  ) );
                    const float64 willCallD = e->pctWillCallD(   pow(  oppBetMake / (oppBetMake + totalexf)  , significance  )  );

/*
                    std::cout << willCall << " ... " << willCallD << std::endl;
                    std::cout << "significance " << significance << std::endl;
                    std::cout << "(oppBetMake + totalexf) " << (oppBetMake + totalexf) << std::endl;
                    std::cout << "(oppBetMake ) " << (oppBetMake ) << std::endl;
                    std::cout << "(betSize) " << (oppBetMake) << std::endl;
                    std::cout << "(oppBetAlready) " << (oppBetAlready) << std::endl;
*/
#endif
                    //const float64 wn = facedOdds_Geom(oppBankRoll,totalexf,oppBetAlready,oppBetMake, 1/significance); //f(w) = w^n
                    //const float64 w = pow( wn, significance );         //f-1(w) = w
                    const float64 w = facedOdds_Geom(oppBankRoll,totalexf,oppBetAlready,oppBetMake, 1/significance, false);
                    nextexf = e->pctWillCall( w );
                    peopleInHandUpper -= 1-nextexf;

                    nextdexf = nextexf + oppBetMake * e->pctWillCallD(  w  )
                            * facedOddsND_Geom( oppBankRoll,totalexf,oppBetAlready,oppBetMake,totaldexf,w, 1/significance, false );//wn, wn/w / significance );
                    //              *  pow(  oppBetMake / (oppBetMake + totalexf)  , significance - 1 ) * significance
//                                * (totalexf - oppBetMake * totaldexf)
//                                 /(oppBetMake + totalexf) /(oppBetMake + totalexf);
                    nextexf *= oppBetMake;
                    if( oppBetAlready + nextexf > nearest )
                    {
                        nearest = oppBetAlready + nextexf;
                    }

                }
				//End of else, blocked executed UNLESS oppBetMake <= 0

            }else
            {///Opponent would be all-in to call this bet
                const float64 oldpot = table->GetPrevPotSize();
                const float64 effroundpot = (totalexf - oldpot) * oppBankRoll / betSize;
                const float64 oppBetMake = oppBankRoll - oppBetAlready;


                nextexf = e->pctWillCall( pow( facedOdds_Geom(oppBankRoll, oldpot + effroundpot,oppBetAlready,oppBetMake, 1/significance, false),significance) );
                peopleInHandUpper -= 1-nextexf;
                nextexf *= oppBetMake ;

                if( oppBetAlready + nextexf + (betSize - oppBankRoll) > nearest )
                {
                    nearest = oppBetAlready + nextexf + (betSize - oppBankRoll);
                }

                nextdexf = 0;

				//Obviously the opponent won't raise...  ie. NoRaise = 100%
				// (nextNoRaise , nextNoRaiseD ) is already (1,0)
            }


            //lastexf = nextexf;
            totalexf += nextexf;

            //lastdexf = nextdexf;
            totaldexf += nextdexf;

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

        const float64 oppInPot = oppBetAlready + nextexf;
        if( oppInPot > betSize )
        {
            overexf += oppInPot - betSize;
            overdexf += nextdexf;
        }

        table->incrIndex(pIndex);
    }

    delete [] nextNoRaise_A;
    delete [] nextNoRaiseD_A;

    totalexf = totalexf - myexf - overexf;
    totaldexf = totaldexf - mydexf - overdexf;

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
#if defined( GEOM_FOLD_PCT )
    //const float64 countToBeat = table->GetNumberAtTable() - 1 ;
    //const float64 significance = 1.0 / countToBeat;
#elif defined( CALL_SCALE_FOLD_PCT )

    const float64 significanceCall = 1;//1.0/countMayFold;
#else
    //const float64 significanceLinear = 1/countToBeat;
//    float64 countToBeat = table->GetNumberAtTable() - table->GetNumberInHand() + 1 ;
//	float64 countMayFold = 1 ;
#endif


    const float64 myexf = betSize;
    const float64 mydexf = 1;




    float64 nextFold;
    float64 nextFoldPartial;
    allFoldChance = 1;
    allFoldChanceD = 0;
    const float64 origPot = table->GetPotSize() - table->ViewPlayer(playerID)->GetBetSize()  +  myexf;
    const float64 origPotD = mydexf;


    if( betSize < minRaiseTo() - chipDenom()/4 || callBet() >= table->GetMaxShowdown() )
    { //Bet is a call, no chance of oppFold
        allFoldChance = 0;
        allFoldChanceD = 0;
        nextFold = 0;
        nextFoldPartial = 0;
    }

    int8 pIndex = playerID;
    table->incrIndex(pIndex);
    while( pIndex != playerID && allFoldChance > 0)
    {

        const float64 oppBetAlready = table->ViewPlayer(pIndex)->GetBetSize();

        const float64 nLinear =/* 1/countToBeat ;*/ (countMayFold + insuranceDeterrent);

        if( table->CanStillBet(pIndex) )
        {///Predict how much the bet will be
            const float64 oppBankRoll = table->ViewPlayer(pIndex)->GetMoney();

            if( betSize < oppBankRoll )
            {

                const float64 oppBetMake = betSize - oppBetAlready;
                //To understand the above, consider that totalexf includes already made bets



/*

                const float64 nextFoldB = ea->pctWillCall( pow(  oppBetMake / (oppBetMake + origPot)  , significance  ) );
                const float64 nextFoldPartialB = ea->pctWillCallD(   pow(  oppBetMake / (oppBetMake + origPot)  , significance  )  )
                *  pow(  oppBetMake / (oppBetMake + origPot)  , significance - 1 ) * significance
                * (origPot - oppBetMake * origPotD)
                                                 /(oppBetMake + origPot) /(oppBetMake + origPot);
*/

#if defined( GEOM_FOLD_PCT )
                const float64 w = facedOdds_Geom(oppBankRoll,origPot,oppBetAlready,oppBetMake, 1/significance);
                nextFold = w;
                nextFoldPartial =
                        facedOddsND_Geom( oppBankRoll,origPot,oppBetAlready,oppBetMake,origPotD,w, 1/significance );

#elif defined( CALL_SCALE_FOLD_PCT )
                const float64 w = facedOdds_Geom(oppBankRoll,origPot,oppBetAlready,oppBetMake, 1/significanceCall);
                nextFold = 1 - ea->pctWillCall( w );
                nextFoldPartial = -ea->pctWillCallD( w ) *
                        facedOddsND_Geom( oppBankRoll,origPot,oppBetAlready,oppBetMake,origPotD,w, 1/significanceCall );



#else
                float64 w_mean = facedOdds_Algb(oppBankRoll,origPot,oppBetAlready,oppBetMake,1.0/nLinear, false);
                float64 w_rank = facedOdds_Algb(oppBankRoll,origPot,oppBetAlready,oppBetMake,1.0/nLinear, true);

                //float64 w = pow( wn, 1.0/nLinear );
				//const float64 dfwdbetSize = (w <= 0) ? 0 : (wn/w / significanceLinear);
                if( nLinear <= 0 )
                {
                    w_rank = 1;
                    w_mean = 1;
                }

//                        pow(  oppBetMake / (oppBetMake + origPot)  , significanceLinear  );

    #ifdef CALL_ALGB_PCT
                    float64 oppCommitted = stagnantPot() - table->ViewPlayer(pIndex)->GetContribution();
                    oppCommitted = oppCommitted / (oppCommitted + oppBankRoll);

                    const float64 eaFold = (1 - ea->pctWillCall( w_mean ))*(1 - oppCommitted);
                    const float64 meanFold = 1 - e->pctWillCall( w_mean );
                    const float64 rankFold = w_rank;
                    const float64 eaRkFold = 1-handRarity;

//(nLinear <= 0 && wn > 1) ? 0 :

                    const float64 baseFoldPartial = facedOddsND_Algb( oppBankRoll,origPot,oppBetAlready,oppBetMake,origPotD,w_mean, nLinear, false);
                    const float64 rankFoldPartial = facedOddsND_Algb( oppBankRoll,origPot,oppBetAlready,oppBetMake,origPotD,w_rank, nLinear, true);
                    const float64 meanFoldPartial = -e->pctWillCallD( w_mean ) * baseFoldPartial;
                    const float64 eaFoldPartial = -ea->pctWillCallD( w_mean ) * baseFoldPartial;
                    const float64 eaRkFoldPartial = 0;

                    ///topTwoOfThree is on a player-by-player basis
                    //nextFoldPartial = bottomThreeOfFour(eaFold,meanFold,rankFold,eaRkFold,eaFoldPartial,meanFoldPartial,rankFoldPartial,eaRkFoldPartial,nextFold);
                    nextFold = (eaFold+meanFold+rankFold+eaRkFold)/4;
                    nextFoldPartial=(eaFoldPartial+meanFoldPartial+rankFoldPartial+eaRkFoldPartial)/4;

                    //nextFold = sqrt((eaFold*eaFold+rankFold*rankFold)/2);
                    //nextFoldPartial = (eaFold*eaFoldPartial+rankFold*rankFoldPartial)*sqrt(2)/nextFold ;
                    //nextFold = (meanFold+rankFold+eaFold)/3;
                    //nextFoldPartial = (meanFoldPartial+rankFoldPartial+eaFoldPartial)/3;

    #else
                    nextFold = w;

//				const float64 nextFoldPartialF = -
                nextFoldPartial =
                        facedOddsND_Algb( oppBankRoll,origPot,oppBetAlready,oppBetMake,origPotD,w, 1/significanceLinear)/(1-minimaxAdjustment);
//                        pow(  oppBetMake / (oppBetMake + origPot)  , significanceLinear - 1 ) * significanceLinear
//                        * (origPot - oppBetMake * origPotD)
//                    /(oppBetMake + origPot) /(oppBetMake + origPot);

    #endif

#endif
/*
#ifndef GEOM_COMBO_FOLDPCT

                nextFold = 1 - (nextFoldB + nextFoldF)/2;
                nextFoldPartial = -(nextFoldPartialB + nextFoldPartialF)/2;

#else

                if( nextFoldB <= 0 || nextFoldF <= 0 )
                {//If they are not going to call no matter what
                nextFold = 1; //So they will fold no matter what
                nextFoldPartial = 0;
						//Betting more won't improve this player's chance of folding, which is already 100%
            }else
                {
                nextFold = sqrt(nextFoldB*nextFoldF);
                nextFoldPartial = -nextFold*(nextFoldPartialB/nextFoldB + nextFoldPartialF/nextFoldF)/2;
                nextFold = 1 - nextFold;
            }

#endif
*/

            }else
            {///Opponent would be all-in to call this bet
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
/*
                    const float64 nextFoldB = e->pctWillCall( pow(oppBetMake / (oppBetMake + oldpot + effroundpot),significance) );
*/
//					const float64 nextFoldF = 1 -

#if defined( GEOM_FOLD_PCT )

                    nextFold = facedOdds_Geom(oppBankRoll, oldpot + effroundpot,oppBetAlready,oppBetMake, 1/significance);
#elif defined( CALL_SCALE_FOLD_PCT )
                    nextFold = 1 - ea->pctWillCall(
                            facedOdds_Geom(oppBankRoll, oldpot + effroundpot,oppBetAlready,oppBetMake, 1/significanceCall)
                        );
#else
                    float64 w_mean = facedOdds_Algb(oppBankRoll,oldpot + effroundpot,oppBetAlready,oppBetMake, 1.0/nLinear,false);
                    float64 w_rank = facedOdds_Algb(oppBankRoll,oldpot + effroundpot,oppBetAlready,oppBetMake, 1.0/nLinear,true);
                    //float64 w =
                      //                  pow( wn,1.0/nLinear) ;
//                            pow(oppBetMake / (oppBetMake + oldpot + effroundpot),significanceLinear );


                    if( nLinear <= 0 )
                    {
                        w_rank = 1;
                        w_mean = 1;
                    }

    #ifdef CALL_ALGB_PCT
                    float64 oppCommitted = table->ViewPlayer(pIndex)->GetContribution();
                    oppCommitted = oppCommitted / (oppCommitted + oppBankRoll);
                    const float64 eaFold = (1 - ea->pctWillCall( w_mean ))*(1 - oppCommitted);
                    const float64 meanFold = 1 - e->pctWillCall( w_mean );
                    const float64 rankFold = w_rank;
                    const float64 eaRkFold = 1-handRarity;

                    ///topTwoOfThree is on a player-by-player basis
//                    bottomThreeOfFour(eaFold,meanFold,rankFold,eaRkFold,0,0,0,0,nextFold);
                    nextFold = (eaFold+meanFold+rankFold+eaRkFold)/4;


                    //nextFold = (meanFold+rankFold+eaFold)/3;
                    //nextFold = sqrt((eaFold*eaFold+rankFold*rankFold)/2);
    #else

                    nextFold = w;
    #endif
                    if( nextFold > 1 ) nextFold = 1;
#endif
/*
#ifndef GEOM_COMBO_FOLDPCT
                    nextFold = 1 - (nextFoldB + nextFoldF)/2;
#else
                    nextFold = 1-sqrt(nextFoldB*nextFoldF);
#endif
*/
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
            if( oppBetAlready > 0 )
            {//Must have gone all-in just now, or at least this round
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


float64 ExactCallD::ActOrReact(float64 callb, float64 lastbet, float64 limit)
{
/*
    const float64 avgControl = (stagnantPot() + table->GetUnbetBlindsTotal()) / table->GetNumberInHand();
    const float64 raiseOver = (callb + avgControl);// < lastbet) ? 0 : (callb + avgControl - lastbet) ;
*/
    const float64 raiseOver = (table->GetPotSize() - callb) / table->GetNumberInHand();
    const float64 actOrReact = (raiseOver > limit) ? 1 : (raiseOver / limit);
    return actOrReact;
}

float64 ExactCallD::RiskPrice()
{
    const float64 myBet = 0;
    const float64 averageStack = table->GetAllChips() / table->GetNumberAtTable();
    float64 riskprice = sqrt(averageStack*( table->GetPotSize() - myBet ));
    return riskprice;
}

