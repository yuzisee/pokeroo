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



/*
    //oppBetMake / (oppBetMake + totalexf)
    const int8 N = handsDealt();
    const float64 fNRank = (wGuess >= 1) ? 1.0/1326.0 : (1 - e->pctWillCall(1 - wGuess));
    const float64 avgBlind = ( alreadyBet + (table->GetBigBlind() + table->GetBigBlind()) / N )
                                        * ( N - 2 )/ N ;

    return (
            log((1 - avgBlind / fNRank ) / (1 - bet))
            /
            log((1+pot) / (1 - bet))
           );
*/

float64 ExactCallD::facedOdds_Geom(float64 bankroll, float64 pot, float64 alreadyBet, float64 bet, float64 n, float64 wGuess)
{

    const int8 N = handsDealt();
    const float64 avgBlind = ( alreadyBet + (table->GetBigBlind() + table->GetBigBlind()) / N )
            * ( N - 2 )/ N ;

    //geomFunction.quantum = 1 / 1326.0 / 2.0;
    geomFunction.Bankroll = bankroll - alreadyBet; //TODO: Confirm {- alreadyBet}
    geomFunction.pot = pot;
    geomFunction.bet = bet;
    geomFunction.alreadyBet = alreadyBet;
    geomFunction.avgBlind = avgBlind;
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
    const float64 fNRank = (frank >= 1) ? 1.0/1326.0 : (1 - frank);
    //fNRank == 0 and frank == 1? That means if you had this w pct, you would have the nuts or better.


    const float64 gainFactor = pow( Bankroll+pot , fw ) * pow( Bankroll-bet , 1-fw ) - Bankroll;

    const float64 stackFactor = avgBlind / fNRank;

    return gainFactor + stackFactor;
}

float64 ExactCallFunctionModel::fd( const float64 w, const float64 y )
{
    const float64 fw = pow(w,n);
    const float64 dfw = (n<0.5) ? (0) : (n * pow(w,n-1));

    const float64 frank = e->pctWillCall(1 - w);
    const float64 fNRank = (frank >= 1) ? 1.0/1326.0 : (1 - frank);

    const float64 stackFactor = avgBlind * e->pctWillCallD(1-w) / fNRank / fNRank;

    const float64 gainFactor = (Bankroll - bet)
                                * pow(  (Bankroll+pot)/(Bankroll-bet)  ,  fw  )
                                * log1p( (Bankroll+pot)/(Bankroll-bet) - 1)
                                * dfw;



    return gainFactor - stackFactor;
}

float64 ExactCallD::facedOddsND_Geom(float64 bankroll, float64 pot, float64 alreadyBet, float64 bet, float64 dpot, float64 w, float64 n)
{
	if( w <= 0 ) return 0;
	if( bet >= bankroll ) return 0;

    const int8 N = handsDealt();
    const float64 avgBlind = ( alreadyBet + (table->GetBigBlind() + table->GetBigBlind()) / N )
            * ( N - 2 )/ N ;

    const float64 frank = e->pctWillCall(1 - w);
    const float64 fNRank = (frank >= 1) ? 1.0/1326.0 : (1 - frank);


    const float64 fw = pow(w,n);
    const float64 dfw = (n<0.5) ? (0) : (n * pow(w,n-1));


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


float64 ExactCallD::facedOdds_Algb(float64 bankroll, float64 pot, float64 alreadyBet, float64 bet, float64 wGuess)
{
    //oppBetMake / (oppBetMake + totalexf)
    const int8 N = handsDealt();

    const float64 frank = e->pctWillCall(1 - wGuess);
    const float64 fNRank = (frank >= 1) ? 1.0/1326.0 : (1 - frank);

    const float64 avgBlind = (table->GetBigBlind() + table->GetBigBlind()) * ( N - 2 )/ N / N;

    const float64 ret =
           (
            (bet - alreadyBet - avgBlind / fNRank ) / (bet + pot)
           );

    if (ret < 0) return 0;
    return ret;
}

float64 ExactCallD::facedOddsND_Algb(float64 bankroll, float64 pot, float64 alreadyBet, float64 bet, float64 dpot, float64 w, float64 n)
{
	//TODO: Really?
	//Approximate at the limit. It should be about linear in this region anyways, and we probably only
	//hit this case when there is no zero in the range [0..1].
	//if( w <= 1.0/1326.0 / 4 ) w = 1.0/1326.0 / 4;
	if( w <= 0 ) return 0;

    const int8 N = handsDealt();
    const float64 frank = e->pctWillCall(1 - w);
	const float64 dfrank = e->pctWillCallD(1-w);
    const float64 fNRank = (frank >= 1) ? 1.0/1326.0 : (1 - frank);
    const float64 avgBlind = (table->GetBigBlind() + table->GetBigBlind()) * ( N - 2 )/ N / N;

    const float64 fw = pow(w,n);
    const float64 dfw = (n<0.5) ? (0) : (n * pow(w,n-1));

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
                (1 - fw*(dpot + 1))
                /
                (   (pot+bet)*dfw
                    +
                    ( avgBlind * dfrank ) / fNRank / fNRank
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

void ExactCallD::query(const float64 betSize)
{

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


        const float64 oppBetAlready = table->ViewPlayer(pIndex)->GetBetSize();

        if( table->CanStillBet(pIndex) )
        {///Predict how much the bet will be
            const float64 oppBankRoll = table->ViewPlayer(pIndex)->GetMoney();

            if( betSize < oppBankRoll )
            {

                const float64 oppBetMake = betSize - oppBetAlready;
				//To understand the above, consider that totalexf includes already made bets


                if( oppBetMake <= 0 )
                {
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
                    for( int32 i=0;i<noRaiseArraySize;++i)
                    {
						const float64 thisRaise = RaiseAmount(betSize,i);
                        const float64 oppRaiseMake = thisRaise - oppBetAlready;
                        if( oppRaiseMake > 0 && thisRaise >= oppBankRoll )
                        {
                            const float64 w_r = facedOdds_Geom(oppBankRoll,totalexf,oppBetAlready,oppRaiseMake, 1/significance);
                            nextNoRaise_A[i] = 1 - e->pctWillCall( w_r );
                            nextNoRaiseD_A[i] = - e->pctWillCallD(  w_r  )  * facedOddsND_Geom( oppBankRoll,totalexf,oppBetAlready,oppRaiseMake,totaldexf,w_r, 1/significance );
                        }
                    }

                    //const float64 wn = facedOdds_Geom(oppBankRoll,totalexf,oppBetAlready,oppBetMake, 1/significance); //f(w) = w^n
                    //const float64 w = pow( wn, significance );         //f-1(w) = w
                    const float64 w = facedOdds_Geom(oppBankRoll,totalexf,oppBetAlready,oppBetMake, 1/significance);
                    nextexf = e->pctWillCall( w );


                    nextdexf = nextexf + oppBetMake * e->pctWillCallD(  w  )
                            * facedOddsND_Geom( oppBankRoll,totalexf,oppBetAlready,oppBetMake,totaldexf,w, 1/significance );//wn, wn/w / significance );
                    //              *  pow(  oppBetMake / (oppBetMake + totalexf)  , significance - 1 ) * significance
//                                * (totalexf - oppBetMake * totaldexf)
//                                 /(oppBetMake + totalexf) /(oppBetMake + totalexf);
                    nextexf *= oppBetMake;

                }
            }else
            {///Opponent would be all-in to call this bet
                const float64 oldpot = table->GetPrevPotSize();
                const float64 effroundpot = (totalexf - oldpot) * oppBankRoll / betSize;
                const float64 oppBetMake = oppBankRoll - oppBetAlready;


                nextexf = oppBetMake * e->pctWillCall( pow( facedOdds_Geom(oppBankRoll, oldpot + effroundpot,oppBetAlready,oppBetMake, 1/significance),significance) );

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
            noRaiseChance_A[i] *= nextNoRaise_A[i];
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
//    const float64 countToBeat = table->GetNumberAtTable() - 1 ;
//    float64 countToBeat = table->GetNumberAtTable() - table->GetNumberInHand() + 1 ;
//	float64 countMayFold = 1 ;
    const float64 myexf = betSize;
    const float64 mydexf = 1;


    float64 nextFold;
    float64 nextFoldPartial;
    allFoldChance = 1;
    allFoldChanceD = 0;
    const float64 origPot = table->GetPotSize() - table->ViewPlayer(playerID)->GetBetSize()  +  myexf;
    const float64 origPotD = mydexf;


    if( betSize < minRaiseTo() - chipDenom()/4 || callBet() >= table->GetMaxShowdown() )
    {
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

        const float64 significanceLinear =/* 1/countToBeat ;
                const float64 significance =*/ 1/countMayFold;

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

                const float64 wn = facedOdds_Algb(oppBankRoll,origPot,oppBetAlready,oppBetMake);
                const float64 w = pow( wn, significanceLinear );
				//const float64 dfwdbetSize = (w <= 0) ? 0 : (wn/w / significanceLinear);

//                const float64 nextFoldF = 1 -
                nextFold = w;
//                        pow(  oppBetMake / (oppBetMake + origPot)  , significanceLinear  );
//				const float64 nextFoldPartialF = -
                nextFoldPartial =
                        facedOddsND_Algb( oppBankRoll,origPot,oppBetAlready,oppBetMake,origPotD,w, 1/significanceLinear);
//                        pow(  oppBetMake / (oppBetMake + origPot)  , significanceLinear - 1 ) * significanceLinear
//                        * (origPot - oppBetMake * origPotD)
//                    /(oppBetMake + origPot) /(oppBetMake + origPot);


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
                {
                    allFoldChance = 0;
                    allFoldChanceD = 0;
                    nextFold = 0;
                }else
                {
/*
                    const float64 nextFoldB = e->pctWillCall( pow(oppBetMake / (oppBetMake + oldpot + effroundpot),significance) );
*/
//					const float64 nextFoldF = 1 -

                    nextFold = pow( facedOdds_Algb(oppBankRoll,oldpot + effroundpot,oppBetAlready,oppBetMake),significanceLinear) ;
//                            pow(oppBetMake / (oppBetMake + oldpot + effroundpot),significanceLinear );
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
    const float64 handFreq = 1/(1-handRarity);
    if( handRarity >= 1 ) //all hands are better than this one
    {
           //1326 is the number of hands possible
        return (1 + baseFraction + (bigBlindFraction+smallBlindFraction)*blindsPow*1326);
    }
    const float64 totalFG = (1 + baseFraction + (bigBlindFraction+smallBlindFraction)*blindsPow*handFreq);

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
    if( queryinput != betSize )
    {
        query(betSize);
        queryinput = betSize;
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
    return totalexf*impliedFactor;
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





