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




#undef INLINE_INTEGER_POWERS







FoldGainModel::~FoldGainModel(){};

FoldWaitLengthModel::~FoldWaitLengthModel(){};



const FoldWaitLengthModel & FoldWaitLengthModel::operator= ( const FoldWaitLengthModel & o )
{
    this->amountSacrifice = o.amountSacrifice;
    this->bankroll = o.bankroll;
    this->betSize = o.betSize;
    this->opponents = o.opponents;
    this->w = o.w;
    this->meanConv = o.meanConv;
    return *this;
}

const bool FoldWaitLengthModel::operator== ( const FoldWaitLengthModel & o ) const
{
    return (
        (o.amountSacrifice == amountSacrifice)
        && (o.bankroll == bankroll)
        && (o.opponents == opponents)
        && (o.betSize == betSize)
        && (o.w == w)
        && (o.meanConv == meanConv)
    );
}

float64 FoldWaitLengthModel::d_dbetSize( const float64 n )
{


    if( lastdBetSizeN != n || !bSearching )
    {
		const float64 rawPCT = ( n < 1 ) ? 0 : lookup(1.0-1.0/(n));
        if( rawPCT != lastRawPCT || !bSearching )
        {
#ifdef INLINE_INTEGER_POWERS
            float64 intOpponents = round(opponents);
            if( intOpponents == opponents )
            {
                cached_d_dbetSize = pow(rawPCT,static_cast<uint8>(intOpponents));
            }else
            {//opponents isn't an even integer
#endif
                cached_d_dbetSize = pow(rawPCT,opponents);
#ifdef INLINE_INTEGER_POWERS
            }//end if intOpponents == opponents , else
#endif
            cached_d_dbetSize = (2*cached_d_dbetSize) - 1;
            lastRawPCT = rawPCT;
        }
        lastdBetSizeN = n;
    }else
    {
        return cached_d_dbetSize;
    }

    return cached_d_dbetSize;
}


float64 FoldWaitLengthModel::d_dw( const float64 n )
{
    if( meanConv == 0 ) return (n)*amountSacrifice/2;
    return -(n)*amountSacrifice/2 * meanConv->d_dw_only( w );
}


float64 FoldWaitLengthModel::d_dC( const float64 n )
{
    return -(n*rarity())/2;
}


const float64 FoldWaitLengthModel::dRemainingBet_dn( )
{
    return -amountSacrifice*rarity();
}


const float64 FoldWaitLengthModel::grossSacrifice( const float64 n )
{
    const float64 sacrificeCount = n*rarity();
    const float64 gross = sacrificeCount*amountSacrifice;
    return gross;
}


const float64 FoldWaitLengthModel::rarity( )
{
    if( cacheRarity >= 0 ) return cacheRarity;


    if( meanConv == 0 ){cacheRarity = 1-w;}
    else{cacheRarity= meanConv->Pr_haveWinPCT_orbetter_continuous(w);}

    if( cacheRarity < 1.0/RAREST_HAND_CHANCE ){ cacheRarity = 1.0/RAREST_HAND_CHANCE; }
    return cacheRarity;
}

const float64 FoldWaitLengthModel::lookup( const float64 rank ) const
{
    if( meanConv == 0 ) return rank;
    return meanConv->nearest_winPCT_given_rank(rank);
}

const float64 FoldWaitLengthModel::dlookup( const float64 rank, const float64 lookupped ) const
{
    if( meanConv == 0 ) return 1;
    return meanConv->inverseD(rank, lookupped);
}

//Maximizing this function gives you the best length that you want to wait for a fold for
float64 FoldWaitLengthModel::f( const float64 n )
{
    const float64 PW = d_dbetSize(n);
    const float64 remainingbet = ( bankroll - grossSacrifice(n)  );
    float64 playbet = (remainingbet < betSize ) ? remainingbet : betSize;

    if( playbet < 0 )
    {
        playbet = 0;
    }

    const float64 lastF = playbet*PW - grossSacrifice(n)/2;
    return lastF;
}

float64 FoldWaitLengthModel::fd( const float64 n, const float64 y )
{
    const float64 remainingbet = ( bankroll - grossSacrifice(n)  );
    if(remainingbet < 0)
    {
        return 0;
    }

    const float64 wmean  = lookup(1.0-1.0/n);
    const float64 dPW_dn = 2*pow(wmean,opponents-1)*opponents/n/n*dlookup(1.0-1.0/n,wmean);

    if(remainingbet < betSize )
    {
        //Since float64 playbet = (remainingbet < betSize ) ? remainingbet : betSize;
        //And lastF = playbet*PW - grossSacrifice(n)/2;
        const float64 PW = (y + grossSacrifice(n)/2)/remainingbet; //d_dbetSize(n);
        const float64 dRemainingbet = dRemainingBet_dn();

        //d{  remainingbet*PW - n*amountSacrifice/2  }/dn
        return (dRemainingbet*PW + remainingbet*dPW_dn - grossSacrifice(n)/2);

    }else{
        return (betSize*dPW_dn - grossSacrifice(n)/2);
    }

}


float64 FoldWaitLengthModel::FindBestLength()
{
    cacheRarity = -1;
    lastdBetSizeN = -1;
    lastRawPCT = -1;

    quantum = 1.0/3.0/rarity();

    float64 maxTurns[2];
    maxTurns[0] = bankroll / amountSacrifice / rarity();

    float64 maxProfit = d_dbetSize( maxTurns[0] ) * betSize ;
    if ( maxProfit < amountSacrifice )
    {
        return 0;
    }


    maxTurns[1] = maxTurns[0];
    maxTurns[0] = maxProfit/amountSacrifice/rarity();


    while( maxTurns[0] < maxTurns[1]/2 )
    {

        maxProfit = d_dbetSize(  maxTurns[0] ) * betSize ;

        if ( maxProfit < amountSacrifice )
        {
            return 0;
        }


        maxTurns[1] = maxTurns[0];
        maxTurns[0] = maxProfit/amountSacrifice/rarity();

        if( maxTurns[1] - maxTurns[0] < quantum )
        {
            break;
        }
    }

    bSearching = true;
    const float64 bestN = FindMax(1/rarity(), ceil(maxTurns[0] + 1) );
    bSearching = false;
    return bestN;
}



void FoldGainModel::query( const float64 betSize )
{


    if(     lastBetSize == betSize
         && last_dw_dbet == dw_dbet
         && lastWaitLength == waitLength
      )
    {
        return;
    }

    waitLength.betSize = betSize;

    lastBetSize = betSize;
    last_dw_dbet = dw_dbet;
    lastWaitLength = waitLength;


    const float64 concedeGain = -waitLength.amountSacrifice;

    if( betSize <= 0 || waitLength.w <= 0 )
    {//singularity
        n = 0;
        lastf = concedeGain;
        lastFA = 0;
        lastFB = 0;
        lastFC = 0;
        lastfd = 0;
        return;
    }else
    {
        n = waitLength.FindBestLength();

		const float64 gain_ref = waitLength.f(n);
		const float64 FB_ref = waitLength.cached_d_dbetSize;
/*
		const float64 m_restored = round(n*waitLength.rarity());
		const float64 n_restored = m_restored/waitLength.rarity();

        const float64 n_below = floor(n_restored);
        const float64 n_above = ceil(n_restored);
        const float64 gain_below = waitLength.f(n_below);
        const float64 FB_below = waitLength.cached_d_dbetSize;
        const float64 gain_above = waitLength.f(n_above);
        const float64 FB_above = waitLength.cached_d_dbetSize;

    #ifdef DEBUG_TRACE_SEARCH
        if(bTraceEnable)
        {
             std::cout << std::endl << "\t\t\t\tBasicSolution n(betSize=" << betSize << ")=" << n << " based on rarity " << waitLength.rarity() << std::endl;
             std::cout << "\t\t\t\tCompare (n_below,gain_below)=" << "(" << n_below << ", " << gain_below << ")" << std::endl;
             std::cout << "\t\t\t\tCompare (n_above,gain_above)=" << "(" << n_above << ", " << gain_above << ")" << std::endl;
        }
    #endif

        if(gain_below > gain_above && n_below > 0)
        {
            n = n_below;
            lastf = gain_below;
            lastFB = FB_below;
        }else
        {
            n = n_above;
            lastf = gain_above;
            lastFB = FB_above;
        }

		if( gain_ref > lastf )
*/
		{
			lastf = gain_ref;
			lastFB = FB_ref;
		}
    }

    if( concedeGain > lastf || n < 1.0 )
    {
        n = 0;
        lastf = concedeGain;
        lastFA = 0;
        lastFB = 0;
        lastFC = 0;
    }else
    {
        lastFA = waitLength.d_dw( n );
        lastFC = waitLength.d_dC( n );
    }
    lastfd = lastFA * dw_dbet + lastFB ;

}

float64 FoldGainModel::F_a( const float64 betSize ) {   query(betSize);return lastFA;   }
float64 FoldGainModel::F_b( const float64 betSize ) {   query(betSize);return lastFB;   }
float64 FoldGainModel::dF_dAmountSacrifice( const float64 betSize) {    query(betSize);return lastFC;   }

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

void FacedOddsCallGeom::query( const float64 w )
{
    if( lastW == w ) return;
    lastW = w;

    FG.waitLength.w = w;
//Chip scale
    const float64 fw = pow(w,FG.waitLength.opponents);
    const float64 U = pow(B+pot,fw)*pow(B-outsidebet,1-fw);

    lastF = U - B - FG.f(outsidebet);

    const float64 dfw = FG.waitLength.opponents*pow(w,FG.waitLength.opponents-1);
    const float64 dU_dw = dfw*log1p((pot+outsidebet)/(B-outsidebet)) * U;


    lastFD = dU_dw;
    if( FG.n > 0 )
    {
        lastFD -= FG.waitLength.d_dw(FG.n);
    }

}

float64 FacedOddsCallGeom::f( const float64 w )
{
    query(w);
    return lastF;
}


float64 FacedOddsCallGeom::fd( const float64 w, const float64 excessU )
{
    query(w);
    return lastFD;
}

void FacedOddsAlgb::query( const float64 w )
{
    if( lastW == w ) return;
    lastW = w;

    FG.waitLength.w = w;
//Chip scale
    const float64 fw = pow(w,FG.waitLength.opponents);
    const float64 U = (pot + betSize)*fw;

    #ifdef DEBUG_TRACE_SEARCH
        if(bTraceEnable)
        {
             std::cout << "\t\t\t faceOddsAlgb.FG.waitLength.SEARCH!" << std::endl;
             //FG.waitLength.bTraceEnable = true;
             FG.bTraceEnable = true;
        }
    #endif

    lastF = U - betSize - FG.f(betSize);

    #ifdef DEBUG_TRACE_SEARCH
        if(bTraceEnable)
        {
             std::cout << "\t\t\t faceOddsAlgb(U,betSize,FG.f,FG.n) = (" << U << " , " << betSize << " , " << FG.f(betSize) << " , " << FG.n << ")" << std::endl;
        }
    #endif

    const float64 dU_dw = U*FG.waitLength.opponents/w;

    lastFD = dU_dw;
    if( FG.n > 0 )
    {
        lastFD -= FG.waitLength.d_dw(FG.n);
    }
}

float64 FacedOddsAlgb::f( const float64 w ) { query(w);  return lastF; }
float64 FacedOddsAlgb::fd( const float64 w, const float64 excessU ) { query(w);  return lastFD; }


void FacedOddsRaiseGeom::query( const float64 w )
{
    if( lastW == w ) return;
    lastW = w;

    FG.waitLength.w = w;
//Fraction scale
    const float64 fw = pow(w,FG.waitLength.opponents);

    const float64 U = pow(1 + pot/FG.waitLength.bankroll  , fw)*pow(1 - raiseTo/FG.waitLength.bankroll  , 1 - fw);
    float64 excess = 1;

    if( !bCheckPossible )
    {
        excess += FG.f(fold_bet) / FG.waitLength.bankroll;
    }

	float64 nonRaiseGain = excess - riskLoss / FG.waitLength.bankroll;

	bool bUseCall = false;
	const float64 callGain = callIncrLoss * pow(callIncrBase,fw);
	//Test against call AND fold possibilities
	if( callGain > nonRaiseGain )
	{
		nonRaiseGain = callGain;
		bUseCall = true;
	}


    lastF = U - nonRaiseGain;


    const float64 dfw = FG.waitLength.opponents*pow(w,FG.waitLength.opponents-1);
    const float64 dU_dw = dfw*log1p((pot+raiseTo)/(FG.waitLength.bankroll-raiseTo)) * U;

    lastFD = dU_dw;
    if( (!bCheckPossible) && FG.n > 0 && !bUseCall)
    {
        lastFD -= FG.waitLength.d_dw(FG.n)/FG.waitLength.bankroll;
    }

	if( bUseCall )
	{
		const float64 dL_dw = dfw*log1p(callIncrBase) * callGain;
		lastFD -= dL_dw;
	}
}

float64 FacedOddsRaiseGeom::f( const float64 w ) { query(w);  return lastF; }
float64 FacedOddsRaiseGeom::fd( const float64 w, const float64 excessU ) { query(w);  return lastFD; }



