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


#include "callPredictionFunctions.h"
#include "math_support.h"

#include <iostream>


#undef INLINE_INTEGER_POWERS

FoldGainModel::~FoldGainModel(){}

FoldWaitLengthModel::~FoldWaitLengthModel(){}


/*
const FoldWaitLengthModel & FoldWaitLengthModel::operator= ( const FoldWaitLengthModel & o )
{
    this->amountSacrificeVoluntary = o.amountSacrificeVoluntary;
	this->amountSacrificeForced = o.amountSacrificeForced;
    this->bankroll = o.bankroll;
    this->betSize = o.betSize;
    this->opponents = o.opponents;
    this->w = o.w;
    this->meanConv = o.meanConv;
    return *this;
}
 */

bool FoldWaitLengthModel::operator== ( const FoldWaitLengthModel & o ) const
{
    return (
        (o.amountSacrificeVoluntary == amountSacrificeVoluntary)
		&& (o.amountSacrificeForced == amountSacrificeForced)
        && (o.bankroll == bankroll)
        && (o.opponents == opponents)
        && (o.betSize == betSize)
        && (o.prevPot == prevPot)
        && (o.w == w)
        && (o.meanConv == meanConv)

        && (o.cacheRarity == cacheRarity)
        && (o.lastdBetSizeN == lastdBetSizeN)
        && (o.lastRawPCT == lastRawPCT)
        && (o.cached_d_dbetSize == cached_d_dbetSize)
    );
}

float64 FoldWaitLengthModel::getRawPCT(const float64 n) {
    const float64 opponentInstances = n * rarity(); // After waiting n hands, the opponent will put you in this situation opponentInstances times.
    // The winPCT you would have if you had the best hand you would wait for out of opponentInstances hands.
    const float64 rawPCT = ( opponentInstances < 1 ) ? 0 : lookup(1.0-1.0/(opponentInstances));


#ifdef DEBUGASSERT
    if(rawPCT < 0.0) {
        std::cout << "negative rawPCT?" << std::endl;
        exit(1);
    }
#endif // DEBUGASSERT

    return rawPCT;
}

float64 FoldWaitLengthModel::d_rawPCT_d_n(const float64 n, const float64 rawPCT) {
    if (n < 1) {
        return 0.0;
    }

    const float64 opponentInstances = n * rarity();
    // rawPCT = lookup(1.0-1.0/(opponentInstances))
    // d_rawPCT_d_n = d_dn lookup(1.0 - 1.0/(opponentInstances))
    //              =    (dlookup(1.0 - 1.0/opponentInstances )) * d_dn { - 1.0/opponentInstances }
    //              =    (dlookup(1.0 - 1.0/opponentInstances )) * { - d_dn opponentInstances^(-1) }
    //              =    (dlookup(1.0 - 1.0/opponentInstances )) * {        opponentInstances^(-2) } * d_dn opponentInstances
    //              =    (dlookup(1.0 - 1.0/opponentInstances )) * {        opponentInstances^(-2) } * rarity()
    //              =    (dlookup(1.0 - 1.0/opponentInstances )) * {        1.0 / n^2 / rarity^2 } * rarity()
    //              =    (dlookup(1.0 - 1.0/opponentInstances )) *          1.0 / n^2 / rarity
    //              =    (dlookup(1.0 - 1.0/opponentInstances )) *          1.0 / n / opponentInstances
    if (opponentInstances >= 1.0) {
        return dlookup(1.0 - 1.0 / opponentInstances, rawPCT) / opponentInstances / n;
    } else {
        return EPS_WIN_PCT; // some small positive number. If you want to increase rawPCT you have to increase n.
    }
}

float64 FoldWaitLengthModel::d_rawPCT_d_w(const float64 n, float64 rawPCT) {

    const float64 opponentInstances = n * rarity();
    // rawPCT = lookup(1.0-1.0/(opponentInstances))
    // d_rawPCT_d_w = d_dw lookup(1.0 - 1.0/(opponentInstances))
    //              =    (dlookup(1.0 - 1.0/opponentInstances )) * d_dw { - 1.0/opponentInstances }
    //              =    (dlookup(1.0 - 1.0/opponentInstances )) * { - d_dw opponentInstances^(-1) }
    //              =    (dlookup(1.0 - 1.0/opponentInstances )) * {        opponentInstances^(-2) } * d_dw opponentInstances
    //              =    (dlookup(1.0 - 1.0/opponentInstances )) * {        opponentInstances^(-2) } * n * d_dw rarity

    // d_dw rarity = {
    //                 - meanConv->Pr_haveWorsePCT_continuous( w ).second
    //               ,
    //                 - 1
    //               }
    // Depending on meanConv

    const float64 d_rarity_d_w = ( meanConv == 0 ) ? (-1) : (-meanConv->Pr_haveWorsePCT_continuous( w ).second);

    if (opponentInstances < 1.0) {
        // n is too low, so we're stuck right now
        return 0.0;
    } else {
        return dlookup(1.0 - 1.0 / opponentInstances, rawPCT) / opponentInstances / opponentInstances * n * d_rarity_d_w;
    }
}

// Your EV (win - loss) as a fraction, based on expected winPCT of the 1.0 - 1.0/n rank hand.
// Return value is between -1.0 and +1.0
// Will memoize while searching
float64 FoldWaitLengthModel::d_dbetSize( const float64 n )
{


    if( lastdBetSizeN != n || !bSearching )
    {
        const float64 rawPCT = getRawPCT(n);


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

                // Your showdown chance of winning, given the opponent count.
                cached_d_dbetSize = pow(rawPCT,opponents);
                
#ifdef INLINE_INTEGER_POWERS
            }//end if intOpponents == opponents , else
#endif


            // Your profit per betSize. If it's positive, you make money the more betSize gets. If negative you lose money the more betSize gets.
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

    // d_dw PW =             d_dw { 2 * pow(getRawPCT, opponents) - 1 }
    //         = 2             * d_dw { pow(getRawPCT, opponents) - 1 }
    //         = 2             * d_dw { pow(getRawPCT, opponents) }
    //         = 2 * (opponents) * pow(getRawPCT, opponents - 1) * d_dw {getRawPCT}

    const float64 rawPCT = getRawPCT(n);
    const float64 d_PW_d_w = 2.0 * opponents * pow(rawPCT, opponents - 1) * d_rawPCT_d_w(n, rawPCT);

    const float64 d_rarity_d_w = ( meanConv == 0 ) ? (-1) : (-meanConv->Pr_haveWorsePCT_continuous( w ).second);

    // d_dw rarity = {
    //                 - meanConv->Pr_haveWorsePCT_continuous( w ).second
    //               ,
    //                 - 1
    //               }

    const float64 chanceOfFoldEachHand = rarity();

    const float64 d_grossSacrifice_dw = n * amountSacrificeVoluntary * 2.0 * chanceOfFoldEachHand * d_rarity_d_w;
    //  grossSacrifice = n * (amountSacrificeVoluntary * chanceOfFoldEachHand * chanceOfFoldEachHand + amountSacrificeForced);
    //  d_grossSacrifice_dw = n * d_dw (amountSacrificeVoluntary * chanceOfFoldEachHand * chanceOfFoldEachHand + amountSacrificeForced)
    //                      = n * amountSacrificeVoluntary (d_dw (chanceOfFoldEachHand^2))
    //                      = n * amountSacrificeVoluntary 2 chanceOfFoldEachHand (d_dw (chanceOfFoldEachHand))
    //                      = n * amountSacrificeVoluntary 2 chanceOfFoldEachHand (d_dw (rarity))
    //

    const float64 winShowdown = prevPot + betSize;

    // lastF = winShowdown*PW - grossSacrifice(n)
    // d_lastF_dw = winShowdown { d_dw PW } - d_dw grossSacrifice

    return winShowdown * d_PW_d_w - d_grossSacrifice_dw;



}

// This is the derivative of f() with respect to amountSacrifice
// For now it's only used as heuristic, so assume grossSacrifice is the only user of amountSacrifice.
float64 FoldWaitLengthModel::d_dC( const float64 n )
{
    const float64 chanceOfFoldEachHand = rarity();
    const float64 chanceOfSameSituationFoldEachHand = chanceOfFoldEachHand * chanceOfFoldEachHand;
    return -(n*chanceOfSameSituationFoldEachHand);
}

// See {const float64 remainingbet = ( bankroll - grossSacrifice(n)  );} in f()
// This is the derivative of that expression with respect to n
float64 FoldWaitLengthModel::dRemainingBet_dn( )
{
    const float64 chanceOfFoldEachHand = rarity();
    const float64 chanceOfSameSituationFoldEachHand = chanceOfFoldEachHand * chanceOfFoldEachHand;
    return - (amountSacrificeVoluntary)*chanceOfSameSituationFoldEachHand - amountSacrificeForced;
}


float64 FoldWaitLengthModel::grossSacrifice( const float64 n )
{

    const float64 numHandsPerFold = 1.0 / rarity();
    const float64 numHandsPerSameSituationFold = numHandsPerFold * numHandsPerFold; // To really get this situation again, both of you have to have comparably good hands. This is how often that happens.
	const float64 amountSacrificePerHand = (amountSacrificeVoluntary / numHandsPerSameSituationFold + amountSacrificeForced);

    const float64 gross = n * amountSacrificePerHand;
    return gross;
}

// A divisor == how many hands rare is {this->w}
// If w is close to 1.0, then Pr_haveWorsePCT_continuous(w) is close to 1.0, and cacheRarity is a small value.
// 1.0 / rarity() should be how often you can expect to get a hand this good again
// NOTE: this->meanConv if specified must be the view of the player who is choosing whether to fold.
// When predicting opponent folds, it must be core.foldcumu
// When evaluating your own folds, it must be core.handcumu
float64 FoldWaitLengthModel::rarity( )
{

#ifdef DEBUGASSERT
    if(w != w)
    {
        std::cout << " w uninitialized (NaN) in FoldWaitLengthModel!" << std::endl;
        exit(1);
    }
#endif // DEBUGASSERT

    if( cacheRarity >= 0 ) return cacheRarity;

    // In RANK mode, if you have a strong hand (e.g. w = 0.9) then you'll get something this strong every 10 hands.
    // Thus:
    if( meanConv == 0 ){cacheRarity = 1-w;}
    // In MEAN mode, if you have a strong hand (e.g. w = 0.65) you may get something this strong every 10 hands or so,
    // e.g. ...
    else{cacheRarity= 1.0 - meanConv->Pr_haveWorsePCT_continuous(w).first;}

    if( cacheRarity < 1.0/RAREST_HAND_CHANCE ){ cacheRarity = 1.0/RAREST_HAND_CHANCE; }
    return cacheRarity;
}

float64 FoldWaitLengthModel::lookup( const float64 rank ) const
{
    if( meanConv == 0 ) return rank;
    return meanConv->nearest_winPCT_given_rank(rank);
}

float64 FoldWaitLengthModel::dlookup( const float64 rank, const float64 lookupped ) const
{
    if( meanConv == 0 ) return 1;
    return meanConv->inverseD(rank, lookupped);
}

//Maximizing this function gives you the best length that you want to wait for a fold for
float64 FoldWaitLengthModel::f( const float64 n )
{
#ifdef DEBUGASSERT
    if(amountSacrificeVoluntary < 0.0)
    {
        std::cout << "amountSacrificeVoluntary cannot be negative" << std::endl;
        exit(1);
    }

    if(is_nan(betSize)) {
        std::cout << "NaN betSize uninitialized?" << std::endl;
        exit(1);
    }
#endif // DEBUGASSERT


    const float64 PW = d_dbetSize(n); // Your expected win rate EV fraction of the pot (after waiting for the best hand in n hands)
    const float64 remainingbet = ( bankroll - grossSacrifice(n)  );

    float64 winShowdown; // If I finally call (after n hands) and don't fold, we'll have ``winShowdown`` winnable in a showdown
    if (remainingbet < 0) {
        // Wouldn't be allowed in the showdown.
        winShowdown = 0.0;
    } else if (remainingbet < betSize) {
        // This call would be reduced slightly
        winShowdown = remainingbet * opponents + prevPot;
    } else {
    // This should include the pot money from previous rounds if SACRIFICE_COMMITTED is defined?
    // See also: unit tests
        winShowdown = betSize * opponents + prevPot;
        //TODO: This-round dead-pot size is not considered!!!!!
    }


    const float64 lastF = winShowdown*PW - grossSacrifice(n);


#ifdef DEBUGASSERT
    if(is_nan(lastF)) {
        std::cout << "NaN lastF result" << std::endl;
        exit(1);
    }
#endif // DEBUGASSERT

    return lastF;
}


// The derivative with f() to w is here.
// f = winShowdown * PW - grossSacrifice
// f = C0 * (2 * pow(getRawPCT, opponents) - 1) - n * (amountSacrificeVoluntary * chanceOfFoldEachHand * chanceOfFoldEachHand + amountSacrificeForced)
// f = C1 * pow(getRawPCT, opponents) - C2 - n * (C3 * rarity^2 + C4)
// f = C1 * (1.0 - 1.0 / n / rarity)^opponents - C2 - n * C3 * rarity^2 - n * C4
// If opponents == 1.0
// f = C0 * 2 - C0 * 2 / n / rarity - C2 - n * C3 * rarity^2 - n * C4

// For each value of rarity, there is some value where fd = 0;
// 0 = fd = +C0 * 2 / n^2 / rarity - C3 * rarity^2 - C4
// C0 * 2 / n^2 / rarity = C3 * rarity^2 - C4
// C0 * 2 / n^2 = C3 * rarity^3 - C4 * rarity
// 1.0 / n^2 = (C3 * rarity^3 - C4 * rarity) / C0 / 2
// ... = (1 / n)
// n = ...
//  Integrate with respect to rarity
// I_n =...
// I_n(1.0) - I_n(rarity_c)
// Rarity of n is (1.0 / n)
// When is it better to fold? When:
// rarity > 1.0 / n
// rarity > ...
float64 FoldWaitLengthModel::fd( const float64 n, const float64 y )
{
    const float64 remainingbet = ( bankroll - grossSacrifice(n)  );
    if(remainingbet < 0)
    {
        return 0;
    }

    const float64 wmean  = getRawPCT(n);
    const float64 dPW_dn = 2*pow(wmean,opponents-1)*opponents * d_rawPCT_d_n(n, wmean);

    const float64 dRemainingbet = dRemainingBet_dn();

    if(remainingbet < betSize )
    {
        const float64 winShowdown = remainingbet * opponents + prevPot;
        const float64 PW = (y + grossSacrifice(n))/winShowdown; //Faster than computing: d_dbetSize(n)

        //Since lastF = winShowdown*PW - grossSacrifice(n)/AVG_FOLDWAITPCT;
        //          === (remainingbet + prevPot) * PW - grossSacrifice(n) / AVG_FOLDWAITPCT
        ///         === (remainingbet          ) * PW - grossSacrifice(n) / AVG_FOLDWAITPCT  + prevPot * PW
        //          === (bankroll - grossSacrifice(n)) * PW - grossSacrifice(n) / AVG_FOLDWAITPCT + prevPot * PW
        //          === (         - grossSacrifice(n)) * PW - grossSacrifice(n) / AVG_FOLDWAITPCT + prevPot * PW + bankroll * PW
        //          ===           -grossSacrifice(n)   *(PW + 1.0 / AVG_FOLDWAITPCT)              +(prevPot + bankroll) * PW
        // Taking the derivative with respect to n...
        //      lastF_dn =        dRemainingbet * (PW + 1.0 / AVG_FOLDWAITPCT) - grossSacrifice(n) * dPW_dn + (prevPot + bankroll) * dPW_dn
        return dRemainingbet * (PW + 1.0) + (prevPot + remainingbet) * dPW_dn;



        //d{  remainingbet*PW - n*amountSacrifice/AVG_FOLDWAITPCT  }/dn
        //return (dRemainingbet*PW + remainingbet*dPW_dn - grossSacrifice(n)/AVG_FOLDWAITPCT);

    }else{
        //Since lastF = winShowdown*PW - grossSacrifice(n)/AVG_FOLDWAITPCT;
        //          === (betSize + prevPot) * PW - grossSacrifice(n) / AVG_FOLDWAITPCT

        const float64 winShowdown = betSize * opponents + prevPot;
        return (winShowdown * dPW_dn  +  dRemainingbet);
    }

}


float64 FoldWaitLengthModel::FindBestLength()
{
    cacheRarity = -1;
    lastdBetSizeN = -1;
    lastRawPCT = -1;

    quantum = (1.0/3.0); // Get to the number of hands played.


#ifdef DEBUGASSERT
    if(amountSacrificeForced <= 0.0 && amountSacrificeVoluntary <= 0.0)
    {
        std::cout << "amountSacrificeForced should always be positive or we'll have maxTurns == \\infty which breaks things. It can't really be zero anyway." << std::endl;
        exit(1);
    }
#endif // DEBUGASSERT


    const float64 numHandsPerSameSituationFold = 1.0 / rarity() / rarity();
	const float64 amountSacrificePerHand = (amountSacrificeVoluntary / numHandsPerSameSituationFold + amountSacrificeForced);
    const float64 amountSacrificePerFOLD = (amountSacrificeVoluntary + amountSacrificeForced * numHandsPerSameSituationFold);

    float64 maxTurns[2];
    maxTurns[0] = bankroll / amountSacrificePerHand;

    // Assuming no cost to folding, what's the best hand we could end up with and how much would it win?
    float64 maxProfit = d_dbetSize( maxTurns[0] ) * betSize ;
    if ( maxProfit < amountSacrificePerFOLD )
    {
        // Can't even win back the cost of folding once, even if we automatically started with the best hand in maxTurns[0] hands.
        // So return 0.0
        return 0;
    }


    // Dummy value to get the loop started.
    //maxTurns[1] = std::numeric_limits<float64>::infinity();

    do
    {
        // Okay, so let's consider the most you could win.
        // How many folds does that afford you at most?
        maxTurns[1] = maxTurns[0]; // (store the old value of maxTurns[0])
        maxTurns[0] = maxProfit/amountSacrificePerHand;


        if( maxTurns[1] - maxTurns[0] < quantum )
        {
            // There exists some chance of profitability.
            // Break out of this loop and call FindMax() below.
            break;
        }

        maxProfit = d_dbetSize(  maxTurns[0] ) * betSize ;

        if ( maxProfit < amountSacrificePerFOLD )
        {
            // So to wait so long and we wouldn't even earn back the cost of the first fold? Then there is no fold length that is worthwhile.
            return 0;
        }

    }while( maxTurns[0] < maxTurns[1]/2 );
    // UNTIL: These two guys are reasonably close enough that a search makes sense.

    bSearching = true;
    const float64 bestN = FindMax(1/rarity(), ceil(maxTurns[0] + 1) );
    bSearching = false;
    return bestN;
}

void FoldWaitLengthModel::load(const ChipPositionState &cps, float64 avgBlind) {
#ifdef SACRIFICE_COMMITTED
    amountSacrificeForced = avgBlind;
    setAmountSacrificeVoluntary(cps.alreadyContributed + cps.alreadyBet
    // Since the blind is already included in forced.
                                - avgBlind);
#else
    amountSacrifice = cps.alreadyBet + avgBlind;
#endif
    bankroll = cps.bankroll;
    prevPot = cps.prevPot;
}

// Rarity depends on cached_d_dBetSize, so we have to clear the cache if we are updating w
void FoldWaitLengthModel::setW(float64 neww) {
    cacheRarity = std::numeric_limits<float64>::signaling_NaN();
    w = neww;
}

float64 FoldWaitLengthModel::getW() const {
    return w;
}


void FoldGainModel::query( const float64 betSize )
{


    if(     lastBetSize == betSize
         //&& last_dw_dbet == dw_dbet
         && lastWaitLength == waitLength
      )
    {
        return;
    }

    waitLength.betSize = betSize;

    lastBetSize = betSize;
    //last_dw_dbet = dw_dbet;
    lastWaitLength = waitLength;


	const float64 concedeGain = -waitLength.amountSacrificeVoluntary -waitLength.amountSacrificeForced;

    if( betSize <= 0 || waitLength.getW() <= 0 )
    {//singularity
        n = 0;
        lastf = concedeGain;
        //lastFA = 0;
        lastFB = 0;
        lastFC = 0;
        lastfd = 0;
        return;
    }else
    {
        n = waitLength.FindBestLength();

		const float64 gain_ref = waitLength.f(n);
		const float64 FB_ref = waitLength.get_cached_d_dbetSize();
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

    // NOTE: dw_dbet == 0.0 so for now this part is simple.
    // TODO(from yuzisee): If later dw_dbet is to be provided, you can uncomment below.
    lastfd = // lastFA * dw_dbet +
    lastFB ;

#ifdef DEBUGASSERT
    if(is_nan(lastf))
    {
        std::cout << " lastf (NaN) in FoldWaitLengthModel! Probably you forgot to initalize something..." << std::endl;
        exit(1);
    }
#endif // DEBUGASSERT


#ifdef DEBUGASSERT
    if(is_nan(lastfd))
    {
        std::cout << " lastfd (NaN) in FoldWaitLengthModel! Probably you forgot to initalize something..." << std::endl;
        exit(1);
    }
#endif // DEBUGASSERT
}

float64 FoldGainModel::F_a( const float64 betSize ) {   query(betSize);return lastFA;   }

// Given a fixed number of hands to wait, this is FoldWaitLengthModel's d_dbetsize, a.k.a. winnings fraction.
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

    FG.waitLength.setW( w );
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

    FG.waitLength.setW( w );
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




#ifdef DEBUGASSERT
    if(is_nan(lastF))
    {
        std::cout << " (NaN) returned by FacedOddsAlgb" << std::endl;
        exit(1);
    }
#endif // DEBUGASSERT

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

    FG.waitLength.setW( w );
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

	//We need to compare raising to the opportunity cost of calling/folding
	//Depending on whether call or fold is more profitable, we choose the most significant opportunity cost
	if( callGain > nonRaiseGain )
	{   //calling is more profitable than folding
		nonRaiseGain = callGain;
		bUseCall = true;
	}//else, folding (opportunity cost) is more profitable than calling (expected value)


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



