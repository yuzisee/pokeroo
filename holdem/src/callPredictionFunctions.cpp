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
#include "inferentials.h"
#include "math_support.h"
#include "portability.h"

#include <iostream>


#undef INLINE_INTEGER_POWERS


// This is used by `FoldGainModel::query` to determine whether we can return cached values vs. whether we need to recompute.
// TODO(from joseph): Find a way to determine whether the cached values are stale or not...
template<typename T1, typename T2> bool FoldWaitLengthModel<T1, T2>::has_same_inputs ( const FoldWaitLengthModel<T1, T2> & o ) const
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
    );
}
// [!CAUTION]
// `.meanConv` appears in BOTH `has_same_inputs` and `has_same_cached_output_values` (!!!)
template<typename T1, typename T2> bool FoldWaitLengthModel<T1, T2>::has_same_cached_output_values ( const FoldWaitLengthModel<T1, T2> & o ) const
{
    return (
         (o.cacheRarity == cacheRarity)
        && (o.cached_d_dbetSize.input_n == cached_d_dbetSize.input_n)
        && (o.cached_d_dbetSize.output_d_dbetSize == cached_d_dbetSize.output_d_dbetSize)
        && (o.meanConv == meanConv)
    );
}

template<typename T1, typename T2> float64 FoldWaitLengthModel<T1, T2>::getRawPCT(const float64 n) {
    const float64 opponentInstances = n * rarity(); // After waiting n hands, the opponent will put you in this situation opponentInstances times.
    // The winPCT you would have if you had the best hand you would wait for out of opponentInstances hands.
    const float64 rawPCT = lookup(
      ( opponentInstances < 1 ) ? 0 : (1.0-1.0/(opponentInstances))
    );
    // Typically, FoldWaitLengthModel runs FindMax starting from `n=1/rarity()` so `n * rarity()` can get awfully close to 1.0 and due to IEEE floating point inaccuracies, you might clip below 0 right away

#ifdef DEBUGASSERT
    if(rawPCT < 0.0) {
        std::cout << "negative rawPCT?" << std::endl;
        exit(1);
    }
#endif // DEBUGASSERT

    return rawPCT;
}

template<typename T1, typename T2> float64 FoldWaitLengthModel<T1, T2>::d_rawPCT_d_n(const float64 n, const float64 rawPCT) {
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

template<typename T1, typename T2> float64 FoldWaitLengthModel<T1, T2>::d_rawPCT_d_w(const float64 n, float64 rawPCT) {

    const float64 opponentInstances = n * rarity();
    // rawPCT = lookup(1.0-1.0/(opponentInstances))
    // d_rawPCT_d_w = d_dw lookup(1.0 - 1.0/(opponentInstances))
    //              =    (dlookup(1.0 - 1.0/opponentInstances )) * d_dw { - 1.0/opponentInstances }
    //              =    (dlookup(1.0 - 1.0/opponentInstances )) * { - d_dw opponentInstances^(-1) }
    //              =    (dlookup(1.0 - 1.0/opponentInstances )) * {        opponentInstances^(-2) } * d_dw opponentInstances
    //              =    (dlookup(1.0 - 1.0/opponentInstances )) * {        opponentInstances^(-2) } * n * d_dw rarity

    // d_dw rarity = {
    //                 - meanConv->Pr_haveWorsePCT_continuous( w ).first.D_v
    //               ,
    //                 - 1
    //               }
    // Depending on meanConv

    const float64 d_rarity_d_w = ( meanConv == 0 ) ? (-1) : (-meanConv->Pr_haveWorsePCT_continuous( w ).first.D_v);

    if (opponentInstances < 1.0) {
        // n is too low, so we're stuck right now
        return 0.0;
    } else {
        return dlookup(1.0 - 1.0 / opponentInstances, rawPCT) / opponentInstances / opponentInstances * n * d_rarity_d_w;
    }
}

// Your EV (win - loss) as a fraction, based on expected winPCT of the 1.0 - 1.0/n rank hand.
// @returns a value between −1.0 and +1.0
// Your profit per betSize. If it's positive, you make money the more betSize gets. If negative you lose money the more betSize gets.
static float64 compute_dE_dbetSize( const float64 rawPCT, const float64 opponents ) {
  #ifdef INLINE_INTEGER_POWERS
            float64 intOpponents = std::round(opponents);
            if( intOpponents == opponents )
            {
                cached_d_dbetSize = std::pow(rawPCT,static_cast<uint8>(intOpponents));
            }else
            {//opponents isn't an even integer
  #endif

                // Your showdown chance of winning, given the opponent count.
               const float64 cached_d_dbetSize = std::pow(rawPCT,opponents);

  #ifdef INLINE_INTEGER_POWERS
            }//end if intOpponents == opponents , else
  #endif

            return (2*cached_d_dbetSize) - 1;
}

// Return value is between -1.0 and +1.0
// Can be memoized using `.b_assume_w_is_constant`
template<typename T1, typename T2> float64 FoldWaitLengthModel<T1, T2>::d_dbetSize( const float64 n )
{
  if (cached_d_dbetSize.bHasCachedValueFor(n) ) {
      // We have a cached value, and we asked to cache it.
      return cached_d_dbetSize.output_d_dbetSize;
  }

  const float64 dE_dbetSize = compute_dE_dbetSize(getRawPCT(n), opponents);

  if (cached_d_dbetSize.b_assume_w_is_constant) {
    // We're in `b_assume_w_is_constant` mode, so cache this value to avoid recomputing it right away
    cached_d_dbetSize.input_n = n;
    cached_d_dbetSize.output_d_dbetSize = dE_dbetSize;
  }

  return dE_dbetSize;
}


template<typename T1, typename T2> float64 FoldWaitLengthModel<T1, T2>::d_dw( const float64 n )
{

    // d_dw PW =             d_dw { 2 * pow(getRawPCT, opponents) - 1 }
    //         = 2             * d_dw { pow(getRawPCT, opponents) - 1 }
    //         = 2             * d_dw { pow(getRawPCT, opponents) }
    //         = 2 * (opponents) * pow(getRawPCT, opponents - 1) * d_dw {getRawPCT}

    const float64 rawPCT = getRawPCT(n);
    const float64 d_dw_getRawPCT = d_rawPCT_d_w(n, rawPCT);
    const float64 d_PW_d_w = 2.0 * opponents * std::pow(rawPCT, opponents - 1) * d_dw_getRawPCT;

    const float64 d_rarity_d_w = ( meanConv == 0 ) ? (-1) : (-meanConv->Pr_haveWorsePCT_continuous( w ).first.D_v);

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

    #ifdef DEBUGASSERT
      if (std::isnan(winShowdown) || std::isnan(d_PW_d_w) || std::isnan(d_grossSacrifice_dw)) {
        std::cerr << "prevPot=" << prevPot << "  betSize=" << betSize << std::endl;
        std::cerr << "opponents=" << opponents << " ← d_rawPCT_d_w(" << n << ", " << rawPCT << ")" << d_dw_getRawPCT << std::endl;
        std::cerr << "  amountSacrificeVoluntary=" << amountSacrificeVoluntary << "  chanceOfFoldEachHand=" << chanceOfFoldEachHand << "  d_rarity_d_w=" << d_rarity_d_w;
        std::cerr << std::endl;
        exit(1);
      }
    #endif

    return winShowdown * d_PW_d_w - d_grossSacrifice_dw;



}

// This is the derivative of f() with respect to amountSacrifice
// For now it's only used as heuristic, so assume grossSacrifice is the only user of amountSacrifice.
template<typename T1, typename T2> float64 FoldWaitLengthModel<T1, T2>::d_dC( const float64 n )
{
    const float64 chanceOfFoldEachHand = rarity();
    const float64 chanceOfSameSituationFoldEachHand = chanceOfFoldEachHand * chanceOfFoldEachHand;
    return -(n*chanceOfSameSituationFoldEachHand);
}

// See {const float64 remainingbet = ( bankroll - grossSacrifice(n)  );} in f()
// This is the derivative of that expression with respect to n
template<typename T1, typename T2> float64 FoldWaitLengthModel<T1, T2>::dRemainingBet_dn( )
{
    const float64 chanceOfFoldEachHand = rarity();
    const float64 chanceOfSameSituationFoldEachHand = chanceOfFoldEachHand * chanceOfFoldEachHand;
    return - (amountSacrificeVoluntary)*chanceOfSameSituationFoldEachHand - amountSacrificeForced;
}


template<typename T1, typename T2> float64 FoldWaitLengthModel<T1, T2>::grossSacrifice( const float64 n )
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
template<typename T1, typename T2> float64 FoldWaitLengthModel<T1, T2>::rarity( )
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
    else{cacheRarity= 1.0 - meanConv->Pr_haveWorsePCT_continuous(w).first.v;}

    if( cacheRarity < 1.0/RAREST_HAND_CHANCE ){ cacheRarity = 1.0/RAREST_HAND_CHANCE; }
    return cacheRarity;
}

template<typename T1, typename T2> float64 FoldWaitLengthModel<T1, T2>::lookup( const float64 rank ) const
{
    if( meanConv == 0 ) return rank;
    return meanConv->nearest_winPCT_given_rank(rank);
}

template<typename T1, typename T2> float64 FoldWaitLengthModel<T1, T2>::dlookup( const float64 rank, const float64 lookupped ) const
{
    if( meanConv == 0 ) return 1;
    return meanConv->inverseD(rank, lookupped);
}

//Maximizing this function gives you the best length that you want to wait for a fold for
template<typename T1, typename T2> float64 FoldWaitLengthModel<T1, T2>::f( const float64 n )
{
#ifdef DEBUGASSERT
    if(amountSacrificeVoluntary < 0.0)
    {
        std::cout << "amountSacrificeVoluntary cannot be negative" << std::endl;
        exit(1);
    }

    if(std::isnan(betSize)) {
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
    if(std::isnan(lastF)) {
        std::cout << "NaN lastF result" << std::endl;
        exit(1);
    }
#endif // DEBUGASSERT

  #ifdef DEBUG_TRACE_PWIN
			if(traceEnable != nullptr) *traceEnable << "\t\t\t\t FoldWaitLengthModel(n=" << n << ", bSearching=" << cached_d_dbetSize.b_assume_w_is_constant << ") compares remainingbet=" << remainingbet << " vs. betSize=" << betSize << " ↦ " << lastF << " based on (winShowdown)" << winShowdown << " ⋅ " << PW << "(PW)" << std::endl;
	#endif

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
template<typename T1, typename T2> float64 FoldWaitLengthModel<T1, T2>::fd( const float64 n, const float64 y )
{
    const float64 remainingbet = ( bankroll - grossSacrifice(n)  );
    if(remainingbet < 0)
    {
        return 0;
    }

    const float64 wmean  = getRawPCT(n);
    const float64 dPW_dn = 2*std::pow(wmean,opponents-1)*opponents * d_rawPCT_d_n(n, wmean);

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


template<typename T1, typename T2> float64 FoldWaitLengthModel<T1, T2>::FindBestLength()
{
    this->resetCaches();

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
    // i.e. std::pow(rawPCT,opponents) * betSize;
    if ( maxProfit < amountSacrificePerFOLD )
    {
        // Can't even win back the cost of folding once, even if we automatically started with the best hand in maxTurns[0] hands.
        // So return 0.0
        return 0;
    }

    // We will search from `1/rarity()` → `bankroll / amountSacrificePerHand`
    //                                     ^^^ we'll start with maxTurns[0] is the entire bankroll
    // We want to find the value of `n` that maximizes `this->f(n)`
    // But if even you can fold the entire bankroll, you'd only win `maxProfit` below, so
    //   we should really be searching only up to `maxProfit/amountSacrificePerHand`
    // ...but then if that wins only $x (repeat)

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
        // `d_dbetSize( maxTurns[0] )` relates to the size of the final showdown after waiting `maxTurns[0]` hands
        maxProfit = d_dbetSize(  maxTurns[0] ) * betSize ;

        if ( maxProfit < amountSacrificePerFOLD )
        {
            // So to wait so long and we wouldn't even earn back the cost of the first fold? Then there is no fold length that is worthwhile.
            return 0;
        }

    }while( maxTurns[0] < maxTurns[1]/2 );
    // UNTIL: These two guys are reasonably close enough that a search makes sense.
    // i.e if we aren't cutting the search space in half anymore with each iteration, move on to FindMax below

#if defined(DEBUG_TRACE_SEARCH) || defined(DEBUG_TRACE_ZERO)
    if(this->traceEnable != nullptr) {
      *traceEnable << "ScalarFunctionModel::FindMax(" << (1/rarity()) <<  "," << ceil(maxTurns[0] + 1) << ") is next" << std::endl;
    }
#endif
    this->resetCaches();
    this->cached_d_dbetSize.b_assume_w_is_constant = true;
    const float64 bestN = FindMax(1/rarity(), ceil(maxTurns[0] + 1) );
    this->resetCaches(); // will also clear b_assume_w_is_constant so nothing to worry about, carry on
    #if defined(DEBUG_TRACE_SEARCH) || defined(DEBUG_TRACE_ZERO)
        if(this->traceEnable != nullptr) {
          *traceEnable << "\t\t\t\tFindMax DONE! FoldWaitLengthModel::FindBestLength bestN=" << bestN << std::endl;
        }
    #endif
    return bestN;
}

template<typename T1, typename T2> void FoldWaitLengthModel<T1, T2>::load(const ChipPositionState &cps, float64 avgBlind) {
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

// Rarity depends on `meanConv` so we have to clear the cache if we are updating meanConv
template<typename T1, typename T2> void FoldWaitLengthModel<T1, T2>::setMeanConv(CallCumulationD<T1, T2> * new_meanConv) {
    cacheRarity = std::numeric_limits<float64>::signaling_NaN();
    this->meanConv = new_meanConv;
}

// Rarity depends on `w`, so we have to clear the cache if we are updating w
template<typename T1, typename T2> void FoldWaitLengthModel<T1, T2>::setW(float64 neww) {
    cacheRarity = std::numeric_limits<float64>::signaling_NaN();
    this->w = neww;
}

template<typename T1, typename T2> float64 FoldWaitLengthModel<T1, T2>::getW() const {
    return w;
}

template<typename T1, typename T2> void FoldGainModel<T1, T2>::query( const float64 betSize )
{

    if(     lastBetSize == betSize
         //&& last_dw_dbet == dw_dbet
         && lastWaitLength.has_same_inputs(waitLength)
      )
    {
        return;
    }

    waitLength.betSize = betSize;

    lastBetSize = betSize;
    //last_dw_dbet = dw_dbet;
    lastWaitLength.copyFrom_noCaches(waitLength);


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
        #if defined(DEBUG_TRACE_SEARCH) || (defined(DEBUG_TRACE_PWIN) && defined(DEBUG_TRACE_ZERO))
          if (this->traceEnable != nullptr) {
            *traceEnable << "\t\t\tFoldGainModel's FoldWaitLengthModel::FindBestLength $" << betSize << "⛁ will call FindMax(nₘᵢₙ,nₘₐₓ)" << std::endl;
            waitLength.traceEnable = this->traceEnable;
          n = waitLength.FindBestLength();
            waitLength.traceEnable = nullptr;
            *traceEnable << "\t\t\t└─> FoldGainModel: gain_ref = FoldWaitLengthModel::f(n=" << n << ")" << std::endl;
          } else
        #endif
        {
          n = waitLength.FindBestLength();
        }

		const float64 gain_ref = waitLength.f(n);
		const float64 FB_ref = waitLength.d_dbetSize(n);
/*
		const float64 m_restored = std::round(n*waitLength.rarity());
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
      // NOW, differentiate:
      //   ∇ lastf = ∇ waitLength.f(n)
        lastFA = waitLength.d_dw( n ); // differentiate `waitLength.f(n)` by ∂w
        // lastFB was set earlier above ↑ differentiate `waitLength.f(n)` by ∂betSize
        lastFC = waitLength.d_dC( n ); // differentiate `waitLength.f(n)` by ∂amountSacrifice
        // dF/dAmountSacrifice = d/dAmountSacrifice {waitLength.f(n)}
        //                    ~= d/dAmountSacrifice {-grossSacrifice(n)}
        //                    ~= d/dAmountSacrifice {-n * amountSacrificePerHand};
    }

    // NOTE: dw_dbet == 0.0 so for now this part is simple.
    // TODO(from yuzisee): If later dw_dbet is to be provided, you can uncomment below.
    lastfd = // lastFA * dw_dbet +
    lastFB ;

#ifdef DEBUGASSERT
    if(std::isnan(lastf))
    {
        std::cout << " lastf (NaN) in FoldWaitLengthModel! Probably you forgot to initalize something..." << std::endl;
        exit(1);
    }
#endif // DEBUGASSERT


#ifdef DEBUGASSERT
    if(std::isnan(lastfd))
    {
        std::cout << " lastfd (NaN) in FoldWaitLengthModel! Probably you forgot to initalize something..." << std::endl;
        exit(1);
    }
#endif // DEBUGASSERT
}

template<typename T1, typename T2> float64 FoldGainModel<T1, T2>::F_a( const float64 betSize ) {   query(betSize);return lastFA;   }

// Given a fixed number of hands to wait, this is FoldWaitLengthModel's d_dbetsize, a.k.a. winnings fraction.
template<typename T1, typename T2> float64 FoldGainModel<T1, T2>::F_b( const float64 betSize ) {   query(betSize);return lastFB;   }

template<typename T1, typename T2> float64 FoldGainModel<T1, T2>::dF_dAmountSacrifice( const float64 betSize) {    query(betSize);return lastFC;   }

template<typename T1, typename T2> float64 FoldGainModel<T1, T2>::f( const float64 betSize )
{
    query(betSize);
    return lastf;

}

template<typename T1, typename T2> float64 FoldGainModel<T1, T2>::fd( const float64 betSize, const float64 y )
{
    query(betSize);
    return lastfd;
}

template<typename T> void FacedOddsCallGeom<T>::query( const float64 w )
{
    if( lastW == w ) return;
    lastW = w;

    FG.waitLength.setW( w );
//Chip scale
    const float64 fw = std::pow(w,FG.waitLength.opponents);
    const float64 U = std::pow(B+pot,fw)*std::pow(B-outsidebet,1-fw);

    lastF = U - B - FG.f(outsidebet);

    const float64 dfw = FG.waitLength.opponents*std::pow(w,FG.waitLength.opponents-1);
    const float64 dU_dw = dfw*std::log1p((pot+outsidebet)/(B-outsidebet)) * U;


    lastFD = dU_dw;
    if( FG.n > 0 )
    {
        lastFD -= FG.waitLength.d_dw(FG.n);
    }

}

template<typename T> float64 FacedOddsCallGeom<T>::f( const float64 w )
{
    query(w);
    return lastF;
}

template<typename T> float64 FacedOddsCallGeom<T>::fd( const float64 w, const float64 excessU )
{
    query(w);
    return lastFD;
}

template<typename T> void FacedOddsAlgb<T>::query( const float64 w )
{
    if( lastW == w ) return;
    lastW = w;

    FG.waitLength.setW( w );
//Chip scale
    const float64 fw = std::pow(w,FG.waitLength.opponents);
    const float64 U = (pot + betSize)*fw;

    #ifdef DEBUG_TRACE_PWIN
        if(traceEnable != nullptr)
        {
             *traceEnable << "\t\t\t faceOddsAlgb.FG.waitLength.setW(" << w << ") & SEARCH!" << std::endl;
             //FG.waitLength.bTraceEnable = true;
             FG.traceEnable = this->traceEnable;
        }
    #endif

    lastF = U - betSize - FG.f(betSize);

    #ifdef DEBUG_TRACE_PWIN
        if(traceEnable != nullptr)
        {
             *traceEnable << "\t\t\t⎯⎯▸ faceOddsAlgb:" << w << "(U − betSize − FG.f, FG.n) = (" << U << " − " << betSize << " − " << FG.f(betSize) << " , " << FG.n << ")" << std::endl;
        }
    #endif

    const float64 dU_dw = U*FG.waitLength.opponents/w;




#ifdef DEBUGASSERT
    if(std::isnan(lastF))
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

template<typename T> float64 FacedOddsAlgb<T>::f( const float64 w ) { query(w);  return lastF; }
template<typename T> float64 FacedOddsAlgb<T>::fd( const float64 w, const float64 excessU ) { query(w);  return lastFD; }

template<typename T> void FacedOddsRaiseGeom<T>::configure_with(FacedOddsRaiseGeom &a, const HypotheticalBet &hypotheticalRaise, float64 currentRiskLoss) {
  const struct ChipPositionState &cps = hypotheticalRaise.bettorSituation;

  a.callPot = cps.pot;
  a.raisedPot = cps.pot + (hypotheticalRaise.bWillGetCalled ? (hypotheticalRaise.betIncrease()) : 0);
    a.raiseTo = hypotheticalRaise.hypotheticalRaiseTo;
    a.fold_bet = hypotheticalRaise.fold_bet();
    a.faced_bet = hypotheticalRaise.faced_bet();
    a.bCheckPossible = hypotheticalRaise.bCouldHaveChecked();
    a.riskLoss = currentRiskLoss;
  a.bRaiseWouldBeCalled = hypotheticalRaise.bWillGetCalled;
}

struct ShowdownOpponents {
  const float64 showdownOpponents;
  const float64 raisedPot;

  // @return matches `float64 fw` of src/callPrediction.cpp#ExactCallD::dfacedOdds_dpot_GeomDEXF
  inline float64 fw(const float64 w) const {
    return std::pow(w,showdownOpponents);
  }

  inline float64 dfw(const float64 w) const {
    return showdownOpponents * std::pow(w,showdownOpponents-1);
  }
}
;

// lastF = U - nonRaiseGain
//       = std::pow(1 + pot/FG.waitLength.bankroll  , fw)*std::pow(1 - raiseTo/FG.waitLength.bankroll  , 1 - fw)   −   nonRaiseGain
//         ^^^ "win the pot if you win the showdown"     * ^^^ "lose your entire raiseTo if you lose the showdown" −   nonRaiseGain
template<typename T> void FacedOddsRaiseGeom<T>::query( const float64 w )
{
    if( lastW == w ) return;
    lastW = w;

    FG.waitLength.setW( w );
//Fraction scale
    const struct ShowdownOpponents showdown_opponents = {
      #ifdef REFINED_FACED_ODDS_RAISE_GEOM
        FG.waitLength.opponents - (bRaiseWouldBeCalled ? 0 : 0.5), // TODO(from joseph): 0.5 is a Schrodinger's player, maybe fold vs. maybe not fold. If we know this hypothetical raise will push one player out FOR SURE !00% guaranteed, we could deduct a full 1.0 instead of 0.5 (but is that too obnoxious?) Let's go with 0.5 for now.
      #else
        FG.waitLength.opponents,
      #endif
      raisedPot
    };
    const float64 fw = showdown_opponents.fw(w);
    const float64 dfw = showdown_opponents.dfw(w);
    // ln(U) = ln{  std::pow(1 + raisedPot/FG.waitLength.bankroll  , fw)*std::pow(1 - raiseTo/FG.waitLength.bankroll  , 1 - fw)  }
    // ln(U) =            fw * ln{ 1 + raisedPot/FG.waitLength.bankroll } + (1−fw)* ln{ 1 - raiseTo/FG.waitLength.bankroll }
    // dU_dw = U * d_dw { fw * ln( 1 + raisedPot/FG.waitLength.bankroll )  −  fw * ln ( 1 - raiseTo/FG.waitLength.bankroll ) }
    // dU_dw = U * dfw  *  (  ln( 1 + raisedPot/FG.waitLength.bankroll )    −    ln ( 1 - raiseTo/FG.waitLength.bankroll )  )
    // dU_dw = U * dfw   *   ln{  (1 + raisedPot/FG.waitLength.bankroll)  / (1 - raiseTo/FG.waitLength.bankroll)  }
    // dU_dw = U * dfw   *   ln{ (1 - raiseTo/FG.waitLength.bankroll + raiseTo/FG.waitLength.bankroll + raisedPot/FG.waitLength.bankroll) / (1 - raiseTo/FG.waitLength.bankroll) }
    // dU_dw = U * dfw   *   ln{ 1.0 + (raiseTo/FG.waitLength.bankroll + raisedPot/FG.waitLength.bankroll) / (1 - raiseTo/FG.waitLength.bankroll) }
    // dU_dw = U * dfw   *   ln{ 1.0 + (raiseTo/FG.waitLength.bankroll + raisedPot/FG.waitLength.bankroll) * FG.waitLength.bankroll / (FG.waitLength.bankroll - raiseTo)  }
    // dU_dw = U * dfw   *   ln{ 1.0 + (raiseTo + raisedPot) / (FG.waitLength.bankroll - raiseTo) }
    const float64 U = std::pow(1 + showdown_opponents.raisedPot/FG.waitLength.bankroll  , fw)*std::pow(1 - raiseTo/FG.waitLength.bankroll  , 1 - fw);
    const float64 dU_dw = U * dfw*std::log1p((showdown_opponents.raisedPot+raiseTo)/(FG.waitLength.bankroll-raiseTo));
    const ValueAndSlope U_by_w = {U, dU_dw};

    ValueAndSlope excess_by_w = {1.0, 0.0};

    if( !bCheckPossible )
    {
      // Nominally, FoldGain is
      //     betSize * "pr{W} after n_hands_to_wait" - betSize "Pr{L} after n_hands_to_wait"         - n_hands_to_wait * betSacrifice
      //     betSize * "pr{W} after n_hands_to_wait" - betSize (1.0 - "Pr{W} after n_hands_to_wait") - n_hands_to_wait * betSacrifice
      // 2 * betSize * "pr{W} after n_hands_to_wait"             - n_hands_to_wait * betSacrifice - betSize
      // 2 * betSize * (1.0 - 1.0 / n_hands_to_wait)^N_opponents - n_hands_to_wait * betSacrifice - betSize
      // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
      //       ↑ seems like `F_a` i.e. `lastFA` is the
      //              derivative of this section?
      //                                                           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
      //                                                           ↑ seems like `F_c` i.e. `lastFC`
      //                                                          is the derivative of this section?
      //
      // So, is `lastFB` a.k.a. `F_b` the derivative of the last `-betSize` on the end, there?
      //
      // Furthermore, `ExactCallD::dfacedOdds_raise_dfacedBet_GeomDEXF` has a `float64 A;` and a `float64 C;`  so are they related to these in some way?
        excess_by_w.v += FG.f(fold_bet) / FG.waitLength.bankroll;
        excess_by_w.D_v += FG.waitLength.d_dw(FG.n)/FG.waitLength.bankroll;
    }

  //We need to compare raising to the opportunity cost of calling/folding
	//Depending on whether call or fold is more profitable, we choose the most significant opportunity cost
#ifdef REFINED_FACED_ODDS_RAISE_GEOM
  // If we were to _call_ instead of raising to `raiseTo`, what would our Geom gain be?
  // Your current bankroll (i.e. the maximum that `raiseTo - fold_bet` could be) is:
  //   `FG.waitLength.bankroll - this->fold_bet`
  // If you win the hand by calling, you'll get everything in the pot:
  //  + Bankroll after calling: `FG.waitLength.bankroll - this->faced_bet`
  //  + You get back all the chips you put in, before this: `this->fold_bet`
  //  + You also get back all the new chips you're putting in to call the bet: `faced_bet - fold_bet`
  //  + You also get back everyone else's (where previous round chips are considered "someone else's") chips that are in the pot: `this->pot - this->fold_bet`
  //  = FG.waitLength.bankroll - this->fold_bet + this->pot
  // If you lose the hand by calling, you'll end up with
  //   `FG.waitLength.bankroll - this->faced_bet`
  const float64 callIncrLoss = (FG.waitLength.bankroll - this->faced_bet) / (FG.waitLength.bankroll - this->fold_bet);
  const float64 callIncrBase = (FG.waitLength.bankroll - this->fold_bet + callPot)/(FG.waitLength.bankroll - this->fold_bet);
  const float64 callGain = std::pow(callIncrLoss, 1 - fw) * std::pow(callIncrBase,fw);


	// TODO(from joseph): Idea → if it's the _final_ betting round, we can still use FoldWaitGainModel, right?
	const float64 applyRiskLoss = (bCheckPossible || bRaiseWouldBeCalled) ? riskLoss : 0.0;
	// And then you should still set bUseCall accordingly, in that case

#else
  const float64 callIncrLoss = 1 - this->fold_bet / FG.waitLength.bankroll;
  const float64 callIncrBase = (FG.waitLength.bankroll + callPot)/(FG.waitLength.bankroll - this->fold_bet); // = 1 + (pot + fold_bet) / (bankroll - fold_bet);
  const float64 applyRiskLoss =  (bCheckPossible) ? 0 : riskLoss;

	const float64 callGain =
	  bRaiseWouldBeCalled ? (
			callIncrLoss * std::pow(callIncrBase,fw)
		) : 0;

#endif

  bool bUseCall = ( callGain > excess_by_w.v );
  const ValueAndSlope nonRaiseGain = (bUseCall ?
	  (   //calling is more profitable than folding
	  	ValueAndSlope {
				callGain,
				//          y =   callGain
				//          y =   std::pow(callIncrLoss, 1 - fw)  *  std::pow(callIncrBase,fw)
				//       ln y = ln std::pow(callIncrLoss,1-fw)  + ln std::pow(callIncrBase,fw)
				//       ln y =          (1-fw) * ln callIncrLoss  +  fw * ln(callIncrBase)
				// d/dw { ln y } = d/dw { (1-fw) * ln callIncrLoss } + d/dw { fw * ln(callIncrBase) }
				// (1/y) * dy/dw = (ln callIncrLoss) * d/dw { 1-fw } + ln(callIncrBase) * d/dw { fw }
				// (1/y) * dy/dw = (ln callIncrLoss) * dfw * (-1)    + ln(callIncrBase) * dfw
				// (1/y) * dy/dw =  ln(callIncrBase) * dfw    - (ln callIncrLoss) * dfw
				// (1/y) * dy/dw =    dfw * (ln(callIncrBase) - ln(callIncrLoss))
				//         dy/dw = y * dfw * (ln(callIncrBase) - ln(callIncrLoss))
				//         dy/dw = y * dfw * (ln(callIncrBase/callIncrLoss))
				//         dy/dw = y * dfw * (ln(1 + callIncrBase/callIncrLoss - 1))
				//         dy/dw = y * dfw * (ln1p(callIncrBase/callIncrLoss - 1))
				//         dy/dw = y * dfw * (ln1p((callIncrBase-callIncrLoss)/callIncrLoss))
				#ifdef REFINED_FACED_ODDS_RAISE_GEOM
					callGain * dfw * stable_ln_div(callIncrBase, callIncrLoss)
					// We use the `std::log1p(…) formulation to ensure numerical stability in the extreme case when `callIncrBase` could be quite large whereas `callIncrLoss` could be quite small
				#else
					/*const float64 dL_dw =*/ dfw*std::log1p(callIncrBase) * callGain
				#endif
			}
	  ) : (
	    //else, folding (opportunity cost) is more profitable than calling (expected value)
	  	excess_by_w
	  )
	)
	;

// Raise only if (U + riskLoss) is better than `nonRaiseGain`
  lastF_by_w = U_by_w;
  lastF_by_w.v += applyRiskLoss / FG.waitLength.bankroll - nonRaiseGain.v;

    if( (!bCheckPossible) && FG.n > 0 && !bUseCall)
    {
      lastF_by_w.D_v -= nonRaiseGain.D_v;
    }
    if (bUseCall) {
      lastF_by_w.D_v -= nonRaiseGain.D_v;
    }

}

template<typename T> float64 FacedOddsRaiseGeom<T>::f( const float64 w ) { query(w);  return lastF_by_w.v; }
template<typename T> float64 FacedOddsRaiseGeom<T>::fd( const float64 w, const float64 excessU ) { query(w);  return lastF_by_w.D_v; }

template<typename T1, typename T2> FoldGainModel<T1, T2>::~FoldGainModel(){}
template<typename T1, typename T2> FoldWaitLengthModel<T1, T2>::~FoldWaitLengthModel(){}

template class FoldWaitLengthModel<void, void>;
template class FoldWaitLengthModel<void, OppositionPerspective>;
template class FoldWaitLengthModel<PlayerStrategyPerspective, OppositionPerspective>;

template class FacedOddsRaiseGeom<void>;
template class FacedOddsRaiseGeom<PlayerStrategyPerspective>;
