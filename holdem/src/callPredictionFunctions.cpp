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


ExactCallFunctionModel::~ExactCallFunctionModel(){};


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

