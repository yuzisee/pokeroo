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

#include "callPredictionFunctions.h"
#include <math.h>



#ifndef log1p
#define log1p( _X_ ) log( (_X_) + 1 )
#endif



FoldGainModel::~FoldGainModel(){};

FoldWaitLengthModel::~FoldWaitLengthModel(){};

#ifdef OLD_PREDICTION_ALGORITHM
ExactCallFunctionModel::~ExactCallFunctionModel(){};
#endif

float64 FoldWaitLengthModel::d_dbetSize( const float64 n )
{
    return (2*pow(1.0-1.0/n,opponents)) - 1;
}

//Maximizing this function gives you the best length that you want to wait for a fold for
float64 FoldWaitLengthModel::f( const float64 n )
{
    const float64 PW = d_dbetSize(n);
    const float64 remainingbet = ( bankroll - n*amountSacrifice  );
    float64 playbet = (remainingbet < betSize ) ? remainingbet : betSize;

    if( playbet < 0 )
    {
        playbet = 0;
    }

    return playbet*PW - n*amountSacrifice;
}

float64 FoldWaitLengthModel::fd( const float64 n, const float64 y )
{
    const float64 PW = d_dbetSize(n);
    const float64 dPW_dn = 2*pow(1.0-1.0/n,opponents-1)*opponents/n/n;
    const float64 remainingbet = ( bankroll - n*amountSacrifice  );
    if(remainingbet < 0)
    {
        return 0;
    }else if(remainingbet < betSize )
    {
        const float64 dRemainingbet = -amountSacrifice;

        //d{  remainingbet*PW - n*amountSacrifice  }/dn
        return (dRemainingbet*PW + remainingbet*dPW_dn - amountSacrifice);

    }else{
        return (betSize*dPW_dn - amountSacrifice);
    }
}




void FoldGainModel::query( const float64 betSize )
{
    if( lastBankroll == bankroll && lastAmountSacrifice == amountSacrifice && lastOpponents == opponents && lastBetSize == betSize )
    {
        return;
    }
    lastBankroll = bankroll;
    lastBetSize = betSize;
    lastOpponents = opponents;
    lastAmountSacrifice = amountSacrifice;

    waitLength.amountSacrifice = amountSacrifice;
    waitLength.opponents = opponents;
    waitLength.betSize = betSize;
    waitLength.bankroll = bankroll;

    n = waitLength.FindMax(1, ceil(bankroll / amountSacrifice) + 1);

    const float64 n_below = floor(n);
    const float64 n_above = ceil(n);
    const float64 gain_below = waitLength.f(n_below);
    const float64 gain_above = waitLength.f(n_above);
    if(gain_below > gain_above && n_below > 0)
    {
        n = n_below;
        lastf = gain_below;
    }else
    {
        n = n_above;
        lastf = gain_above;
    }

    const float64 concedeGain = -amountSacrifice;

    if( concedeGain > lastf )
    {
        n = 0;
        lastf = concedeGain;
        lastfd = 0;
    }else
    {
        lastfd = waitLength.d_dbetSize( n );
    }

}

float64 FoldGainModel::f( const float64 betSize )
{
    query(betSize);
    return lastf;

}

float64 FoldGainModel::fd( const float64 betSize, const float64 y )
{
    query(betSize);
    return lastfd;
}


#ifdef OLD_PREDICTION_ALGORITHM
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
#endif
