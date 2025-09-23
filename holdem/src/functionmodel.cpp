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

#include <iostream>
#include <float.h>
#include <limits>

#include "functionmodel.h"
#include "inferentials.h"
#include "portability.h"

static inline constexpr float64 cleanpow(float64 b, float64 x)
{
    if( b <= DBL_EPSILON ) return 0;
    //if( b > 1 ) return 1;
    return std::pow(b,x);
}




GainModelNoRisk::~GainModelNoRisk()
{
}


GainModelGeom::~GainModelGeom()
{
}

static NetStatResult allNaN() {
    struct NetStatResult result;
    result.fLoseProb = std::numeric_limits<float64>::signaling_NaN();
    result.fOutrightWinProb = std::numeric_limits<float64>::signaling_NaN();
    result.fShape.wins = std::numeric_limits<float64>::signaling_NaN();
    result.fShape.splits = std::numeric_limits<float64>::signaling_NaN();
    result.fShape.loss = std::numeric_limits<float64>::signaling_NaN();
    result.fShape.pct = std::numeric_limits<float64>::signaling_NaN();
    result.fShape.repeated = std::numeric_limits<float64>::signaling_NaN();
    return result;
}
CombinedStatResultsPessimistic::CombinedStatResultsPessimistic(OpponentHandOpportunity & opponentHandOpportunity, CoreProbabilities & core)
:
#ifdef DEBUG_AGAINSTXOPPONENTS
traceDebug(nullptr)
,
#endif
fLastBetSize(std::numeric_limits<float64>::signaling_NaN())
,
fNet(allNaN())
,
f_d_LoseProb_dbetSize(std::numeric_limits<float64>::signaling_NaN()),f_d_WinProb_dbetSize(std::numeric_limits<float64>::signaling_NaN()),fHandsToBeat(std::numeric_limits<float64>::signaling_NaN())
,
fOpposingHands(opponentHandOpportunity)
,
fFoldCumu(&(core.foldcumu))
,
fSplitOpponents(opponentHandOpportunity.fTable.NumberInHand().inclAllIn() - 1)
{}


static std::pair<struct NetStatResult, float64> againstBestXOpponents(FoldStatsCdf & fOppCumu, float64 fractionOfHandsToBeat, float64 fractionOfHandsToBeat_dbetSize, playernumber_t fSplitOpponents
#ifdef DEBUG_AGAINSTXOPPONENTS
  , std::ofstream * logF
#endif
) {
    std::pair<struct NetStatResult, float64> result;

    const std::pair<StatResult, float64> bestXOpponents = fOppCumu.bestXHands(fractionOfHandsToBeat);
    const StatResult showdownResults = bestXOpponents.first.ReversedPerspective(); // oddsAgainstbestXHands.first;
    const float64 d_showdownPct_dX = -bestXOpponents.second; // oddsAgainstbestXHands.second;

    // da_dbetSize = da_dX * dX_dbetSize
    // Here, X === fractionOfHandsToBeat === 1.0 / fOpposingHands.handsToBeat()
    //
    // d_dbetSize{fWinProb} = d_dbetSize(fFoldCumu->oddsAgainstBestXHands(fractionOfHandsToBeat))
    //                      = fFoldCumu->oddsAgainstBestXHands ' (fractionOfHandsToBeat) * d_dbetSize(fractionOfHandsToBeat)
    //                      = fFoldCumu->oddsAgainstBestXHands ' (fractionOfHandsToBeat) * d_dbetSize(1.0 / fOpposingHands.handsToBeat())
    //                      = fFoldCumu->oddsAgainstBestXHands ' (fractionOfHandsToBeat) * (-1.0 / fOpposingHands.handsToBeat()^2) * d_dbetSize(fOpposingHands.handsToBeat())
    //                      = d_showdownPct_dX * (-fractionOfHandsToBeat * fractionOfHandsToBeat) * fractionOfHandsToBeat_dbetSize

    float64 & fWinProb = result.first.fOutrightWinProb;
    float64 & fLoseProb = result.first.fLoseProb;
    StatResult & fSplitShape = result.first.fShape;
    float64 & f_d_WinProb_dbetSize = result.second;

    // Simplification: Probability of winning the showdown is the probability of beating the single opponent with the best chance to beat you.
    fWinProb = showdownResults.wins;
    fLoseProb = showdownResults.loss;
    f_d_WinProb_dbetSize = d_showdownPct_dX * (-fractionOfHandsToBeat * fractionOfHandsToBeat) * fractionOfHandsToBeat_dbetSize;

    // To evaluate splits we need a per-player (win, split, loss).
    fSplitShape.loss = 1.0 - cleanpow(1.0 - fLoseProb, 1.0 / fSplitOpponents); //The w+s outcome for all players should be a power of w+s for shape
    fSplitShape.wins = cleanpow(fWinProb, 1.0 / fSplitOpponents);
    fSplitShape.splits = 1.0 - fSplitShape.loss - fSplitShape.wins;

    #ifdef DEBUG_AGAINSTXOPPONENTS
      if(logF != nullptr) { *logF << "\t\t showdownResults = (W: " << fWinProb << " , L: " << fLoseProb << " )  ↚  (w: " << fSplitShape.wins << " , s: " << fSplitShape.splits << " , l: " << fSplitShape.loss << ") = fSplitShape before normalizating" <<  std::endl; }
    #endif

#ifdef DEBUGASSERT
    if ((showdownResults.loss != showdownResults.loss) || (showdownResults.wins != showdownResults.wins) || (showdownResults.splits != showdownResults.splits)) {
        std::cerr << "NaN encountered in showdownResults shape" << endl;
        exit(1);
    }
#endif // DEBUGASSERT

    if (fSplitShape.splits <= std::numeric_limits<float64>::epsilon()) {
        fSplitShape.splits = 0.0;
    } else if (fSplitOpponents > 1) {
        float64 splitTotal = 0.0;
        for( int8 i=1;i<=fSplitOpponents;++i )
        {//Split with i
          const float64 multisplit_adjustment = HoldemUtil::nchoosep<float64>(fSplitOpponents,i)*std::pow(fSplitShape.wins,fSplitOpponents-i)*std::pow(fSplitShape.splits,i);
            splitTotal += multisplit_adjustment;
            #ifdef DEBUG_AGAINSTXOPPONENTS
              if(logF != nullptr) { *logF << "\t\t\t Multi-split adjustment +" << multisplit_adjustment << " from " << static_cast<int>(fSplitOpponents) << "_C_" << static_cast<int>(i) << "=" << HoldemUtil::nchoosep<int32>(fSplitOpponents,i) << " way(s) to win against nₐ=" << static_cast<int>(fSplitOpponents-i)  << " → (w^(nₐ))⋅(s^nₛ) = " << std::pow(fSplitShape.wins,fSplitOpponents-i) << " ⋅ " << std::pow(fSplitShape.splits,i) << " while splitting against nₛ=" << static_cast<int>(i) << std::endl; }
            #endif
        }

        ///Normalize, total split possibilities must add up to showdownResults.split
        if (splitTotal > 0) {
            const float64 rescaleSplitWin = cleanpow(fSplitShape.splits / splitTotal, 1.0 / fSplitOpponents); // We will rescale .wins and .splits by this amount. In total, that scales splitTotal by (fSplitShape.splits / splitTotal)
#ifdef DEBUGASSERT
            if (rescaleSplitWin != rescaleSplitWin) {
                std::cerr << "NaN encountered in rescaleSplitWin" << endl;
                exit(1);
            }
#endif // DEBUGASSERT
            fSplitShape.loss -= (fSplitShape.wins + fSplitShape.splits) * (rescaleSplitWin - 1.0); // Subtract any excess that would be created (e.g. if rescaleSplitWin > 1.0)
            fSplitShape.wins *= rescaleSplitWin;
            fSplitShape.splits *= rescaleSplitWin;
            #ifdef DEBUG_AGAINSTXOPPONENTS
              if(logF != nullptr) { *logF << "\t\t\t Multi-split rescale factor " << rescaleSplitWin << " due to " << splitTotal << "% split combinations ∴ w: " << fSplitShape.wins << " , s: " << fSplitShape.splits << " , l: " << fSplitShape.loss << std::endl; }
            #endif
        }
    }
    fSplitShape.forceRenormalize();


#ifdef DEBUGASSERT
    if ((fSplitShape.loss != fSplitShape.loss) || (fSplitShape.wins != fSplitShape.wins) || (fSplitShape.splits != fSplitShape.splits)) {
        std::cerr << "NaN encountered in fSplitShape shape" << endl;
        exit(1);
    }
#endif // DEBUGASSERT

    if(   (1 - fSplitShape.splits <= DBL_EPSILON)  || (fSplitShape.loss + fSplitShape.wins <= DBL_EPSILON)
       || (1 - showdownResults.splits <= DBL_EPSILON)  || (showdownResults.loss + showdownResults.wins <= DBL_EPSILON)
       )
    {
      #ifdef DEBUG_AGAINSTXOPPONENTS
        if(logF != nullptr) { *logF << "\t\t fSplitShape=(w: " << fSplitShape.wins << " , s: " << fSplitShape.splits << " , l: " << fSplitShape.loss << ") BOUNDARY" << std::endl; }
      #endif
        fLoseProb = 0.0;
        fWinProb = 0.0;
        f_d_WinProb_dbetSize = 0.0;
        fSplitShape.wins = 1.0; //You need wins to split, and shape is only used to split so this okay
    }
    #ifdef DEBUG_AGAINSTXOPPONENTS
    else {
      if(logF != nullptr) { *logF << "\t\t fSplitShape=(w: " << fSplitShape.wins << " , s: " << fSplitShape.splits << " , l: " << fSplitShape.loss << ") Renormalized" << std::endl; }
    }
    #endif


#ifdef DEBUGASSERT
    if (!((0 <= fLoseProb) && (0 <= fWinProb))) {
        std::cerr << "Invalid encountered in fWinProb and/or fLoseProb" << endl;
        exit(1);
    }
#endif // DEBUGASSERT

    return result;
}


void CombinedStatResultsPessimistic::query(float64 betSize) {
    if (fLastBetSize == betSize) {
        return;
    }
    fLastBetSize = betSize;

    fOpposingHands.query(betSize);
    fHandsToBeat = fOpposingHands.handsToBeat();
    const float64 fractionOfHandsToBeat_dbetSize = fOpposingHands.d_HandsToBeat_dbetSize();

#ifdef DEBUGASSERT
    if (std::isnan(fHandsToBeat)) {
        std::cerr << "NaN encountered in fHandsToBeat" << endl;
        exit(1);
    }

    if (fractionOfHandsToBeat_dbetSize != fractionOfHandsToBeat_dbetSize) {
        std::cerr << "NaN encountered in fractionOfHandsToBeat_dbetSize" << endl;
        exit(1);
    }
#endif // DEBUGASSERT

    const float64 fractionOfHandsToBeat = 1.0 / fHandsToBeat; // If you have to beat N hands, expect the worst to be one of the worst 1/Nth

    #ifdef DEBUG_AGAINSTXOPPONENTS
      if(traceDebug != nullptr) { *traceDebug << "\t\t on betSize $" << betSize << "⛃ againstBestXOpponents(fFoldCumu, " << fHandsToBeat << "⁻¹ = " << fractionOfHandsToBeat << " , " << fractionOfHandsToBeat_dbetSize << " , " << static_cast<int>(fSplitOpponents) << ")" << std::endl; }
    #endif

    std::pair<struct NetStatResult, float64> result = againstBestXOpponents(*fFoldCumu, fractionOfHandsToBeat, fractionOfHandsToBeat_dbetSize, fSplitOpponents
      #ifdef DEBUG_AGAINSTXOPPONENTS
      , traceDebug
      #endif
    );
    fNet = result.first;
    f_d_WinProb_dbetSize = result.second;
    f_d_LoseProb_dbetSize = -f_d_WinProb_dbetSize;

}

CoarseCommunityHistogram::CoarseCommunityHistogram(const DistrShape &detailPCT, const StatResult & rankShape)
:
fMyBest(detailPCT.best)
,
fMyWorst(detailPCT.worst)
,
fNumBins(COARSE_COMMUNITY_NUM_BINS)
,
fBinWidth((detailPCT.best.pct - detailPCT.worst.pct) / fNumBins)
{
    if (fBinWidth > 0.0) {
        float64 count = 0.0;
        //const float64 &dx = fBinWidth;
        for (size_t k=0; k<COARSE_COMMUNITY_NUM_BINS; ++k) {

            // TODO(from yuzisee): Use a CallCumulation for this?
            fBins[k].freq = detailPCT.coarseHistogram[k].repeated;

            fBins[k].myChances = detailPCT.coarseHistogram[k];
            fBins[k].myChances.repeated /= detailPCT.n;

            if (fBins[k].myChances.repeated < 0.0) {
                std::cerr << "detailPCT.coarseHistogram[" << static_cast<int>(k) << "] can't be negative!\n";
                exit(1);
            }

            if (fBins[k].myChances.repeated == 0.0) {
                // Put in dummy values because this bin is empty.
                // Here, we set PCT according to the midpoint of the bin, and set splits based on rankShape.splits
                const float64 midPct = fMyWorst.pct + fBinWidth * (0.5 + k);
                fBins[k].myChances.pct = midPct;

                if (midPct * 2 < rankShape.splits) {
                    // Cap at splits.
                    fBins[k].myChances.splits = midPct * 2;
                    fBins[k].myChances.wins = 0.0;
                    fBins[k].myChances.loss = 0.0;
                } else {
                    fBins[k].myChances.splits = rankShape.splits;
                    fBins[k].myChances.wins = midPct - rankShape.splits / 2.0;
                    fBins[k].myChances.loss = 1.0 - fBins[k].myChances.wins - fBins[k].myChances.splits;
                }
            }

            count += fBins[k].freq;
        }

        if (count != detailPCT.n) {
            std::cerr << "Expecting " << static_cast<int>(detailPCT.n) << " histogram entries but only found " << static_cast<int>(count) << "\n";
            exit(1);
        }
    }

}
CoarseCommunityHistogram::~CoarseCommunityHistogram()
{

}


// @param difficultyOpponents - e.g. The number of opponents that were at the table when the bet was made (the "implied" difficulty of the group at the time the bet was made)
// @param showdownOpponents - The number of remaining opponents that are left for you to beat by taking them to a showdown
static NetStatResult initByRank(playernumber_t difficultyOpponents, playernumber_t showdownOpponents, StatResult baseShape) {


#ifdef DEBUGASSERT
    if ((baseShape.loss != baseShape.loss) || (baseShape.wins != baseShape.wins) || (baseShape.splits != baseShape.splits)) {
        std::cerr << "NaN encountered in shape" << endl;
        exit(1);
    }
#endif // DEBUGASSERT

    NetStatResult result;

    result.fOutrightWinProb = cleanpow(baseShape.wins, difficultyOpponents);

    // Now, adjust fShape so that it applies per showdownOpponents rather than per difficultOpponents
    result.fShape = baseShape;
    result.fShape.wins =       cleanpow(      result.fOutrightWinProb, 1.0 / static_cast<double>(showdownOpponents));
    // Normalize, preserving splits
    result.fShape.loss = 1.0 - result.fShape.splits - result.fShape.wins;
    result.fShape.forceRenormalize();
    result.fShape.repeated = 1;

    result.fLoseProb =  1 - cleanpow(1 - baseShape.loss, showdownOpponents);


    // TODO(yuzisee): Unit test
    //  If showdownOpponents == difficultyOpponents: assert baseShape === fShape
    //  If split is large, then imagine difficultOpponents = 2 and showdownOpponents = 1
    //    Should outright win / outright lose ratio be preserved or "overall pct" preserved??

    if( 1.0 - result.fShape.splits <= DBL_EPSILON  || (result.fShape.loss + result.fShape.wins <= DBL_EPSILON) )
    {
        result.fLoseProb = 0;
        result.fOutrightWinProb = 0;
        result.fShape.wins = 1; //You need wins to split, and shape is only used to split so this okay
    }


#ifdef DEBUGASSERT
    if ((result.fLoseProb != result.fLoseProb) || (result.fOutrightWinProb != result.fOutrightWinProb)) {
        std::cerr << "NaN encountered in NetStatResult probabilities" << endl;
        exit(1);
    }
#endif // DEBUGASSERT

#ifdef DEBUGASSERT
    if ((result.fShape.loss != result.fShape.loss) || (result.fShape.wins != result.fShape.wins) || (result.fShape.splits != result.fShape.splits)) {
        std::cerr << "NaN encountered in NetStatResult.fShape" << endl;
        exit(1);
    }
#endif // DEBUGASSERT

    return result;
}

/**
 * The goal of this function is to break out which aspects of your win percentage scale with the number of players and which don't.
 * The affect of community cards doesn't change with the number of players.
 * Your chance of beating N players assuming a particular set of community cards must get harder as the number of players decreases.
 * TODO(from joseph_huang): is MEAN too harsh here? We don't have access to RANK anymore.
 *                          Well, after the river we call initByRank directly since we don't have multiply community outcomes (plus MEAN === RANK anyway, it's just a count of outcomes)
 *                          Well, after the turn, all the community outcomes yield post-river situations which mean binMEAN === binRANK so it's okay too
 *                          Now, after the flop the community outcomes will yield post-turn situations. MEAN and RANK differ slightly.
 *                              At the post-flop moment we have a guess of the post-flop MEAN<-->RANK inflation. How does it relate to the post-turn MEAN<-->RANK inflation?
 *                              It varies by 46 outcomes. The mean is an average of 46 discrete outcomes.
 *                          Pre-flop, the community outcomes will yield post-flop situations. MEAN and RANK differ somewhat.
 *                              At the pre-flop moment we have a guess of the pre-flop MEAN<-->RANK inflation.
 */
static NetStatResult initMultiOpponent(playernumber_t difficultyOpponents, playernumber_t showdownOpponents, const CoarseCommunityHistogram &outcomes, const NetStatResult rankComparison) {
    if (outcomes.fBinWidth <= 0.0) {
        // MEAN == RANK at this point anyway.
        // The community doesn't affect your outcome so only the opposing hands do which would be ranked and counted to compute mean.
        return rankComparison;
    }

    NetStatResult expectation; // expectation sum
    expectation.fShape.wins = 0.0;
    expectation.fShape.splits = 0.0;
    expectation.fShape.loss = 0.0;
    expectation.fShape.pct = 0.0;
    expectation.fShape.repeated = 0.0;
    expectation.fLoseProb = 0.0;
    expectation.fOutrightWinProb = 0.0;

    // For each community outcome bin, generate a NetStatResult and add it to the expectation sum above.
    for(size_t communityOutcomeBin=0; communityOutcomeBin<outcomes.fNumBins; ++communityOutcomeBin) {
        const StatResult thisCommunityBaseShape = outcomes.getBin(communityOutcomeBin).myChances;
        const float64 w = thisCommunityBaseShape.repeated; // weight

        if (w > 0.0) {
            // Compute the multi-player result assuming this community bin...
            NetStatResult thisCommunityResult = initByRank(difficultyOpponents, showdownOpponents, thisCommunityBaseShape);

            // Contribute weighted average into the overall result.
            thisCommunityResult.fShape.repeated = w;

            expectation.fShape.addByWeight(thisCommunityResult.fShape);
            expectation.fLoseProb += thisCommunityResult.fLoseProb * w;
            expectation.fOutrightWinProb += thisCommunityResult.fOutrightWinProb * w;
        }
    }

    if (rankComparison.fShape.pct < expectation.fShape.pct) {
        // Drawing hand
        return expectation;
    } else {
        // Playing hand
        return rankComparison;
    }
}

static NetStatResult initSingleOpponent(playernumber_t difficultyOpponents, playernumber_t showdownOpponents, FoldStatsCdf &foldcumu) {
    const float64 fractionOfHandsToBeat = 1.0 / difficultyOpponents;
    return againstBestXOpponents(foldcumu, fractionOfHandsToBeat, 0.0, showdownOpponents
      #ifdef DEBUG_AGAINSTXOPPONENTS
      , nullptr
      #endif
    ).first;
}

PureStatResultGeom::PureStatResultGeom(const StatResult mean, const StatResult rank, const CoarseCommunityHistogram &outcomes, FoldStatsCdf &foldcumu, const ExpectedCallD &tableinfo)
:
fDifficultyOpponents(tableinfo.handStrengthOfRound())
,
fShowdownOpponents(tableinfo.handsToShowdownAgainst())
,
fNet(
     (tableinfo.handsToShowdownAgainst() > 1) ?

     (
     // The odds of beating multiple people isn't based on the odds of beating one person.

      // For drawing hands we're able to break out conditional probability by community outcomes vs. opponent outcomes.
      initMultiOpponent(tableinfo.handStrengthOfRound(), tableinfo.handsToShowdownAgainst(), outcomes,
                        // Otherwise, since it's more complicated than that, just go with rank for now.
                        initByRank(tableinfo.handStrengthOfRound(), tableinfo.handsToShowdownAgainst(), rank)
      )
     )
     :

     // We're only facing one actual person (even though handStrengthOfRound can be higher)
     // This allows us to calculate our odds more precisely than with just rank
     initSingleOpponent(tableinfo.handStrengthOfRound(), tableinfo.handsToShowdownAgainst(), foldcumu)   )
{}


float64 CombinedStatResultsGeom::cleangeomeanpow(float64 b1, float64 x1, float64 b2, float64 x2, float64 f_battle)
{
    if (x1 == x2) {
        // A common use case is to pass in the same StatResult for s_acted and s_nonacted.
        // Unless you intend to provide two separate StatResult objects, sometimes even repeated == 0.0 (which causes NaNs in here).
        // If we notice that you passed in the same StatResult twice, just take the geomean explicitly without considering .repeated
        return sqrt( cleanpow(b1, f_battle) * cleanpow(b2, f_battle) );
    } else {
#ifdef DEBUGASSERT
        std::cerr << "TODO(from yuzisee): Not yet tested";
        exit(1);
#endif // DEBUGASSERT
        const float64 w1 = x1 * f_battle / (x1+x2);
        const float64 w2 = x2 * f_battle / (x1+x2);
        return cleanpow(b1, w1)*cleanpow(b2,w2);
    }

    //return cleanpow( cleanpow(b1,x1)*cleanpow(b2,x2) , f_battle/(x1+x2) );
}


void CombinedStatResultsGeom::combineStatResults(const StatResult s_acted, const StatResult s_nonacted, bool bConvertToNet)
{

    /*
     #ifdef DEBUGASSERT
     // Note: During forceRenormalize e_battle is espec.tableinfo->handsIn()-1
     if(    espec.tableinfo->handsIn()-1 != (playernumber_t)f_battle  // For now, just verify that handsIn() == handsToBeat() + 1
     // eventually, (presumably) handsToBeat will vary between numberStartedRound - 1 and numberAtFirstBet - 1 depending on who is calling this.
     )
     {
     //std::cerr << "handsToBeat should be all the opposing players" << std::endl;
     //std::cerr << (int)(espec.tableinfo->handsIn()) << " in hand. ";
     //exit(1);
     }
     #endif
     */

    if (bConvertToNet) {
        ///Use f_battle instead of e_battle, convert to equivelant totalEnemy
        p_cl =  1 - cleangeomeanpow(1 - s_acted.loss,s_acted.repeated , 1 - s_nonacted.loss,s_nonacted.repeated, f_battle);
        p_cw = cleangeomeanpow(s_acted.wins,s_acted.repeated , s_nonacted.wins,s_nonacted.repeated, f_battle);
    } else {
        // When bConvertToNet is false, that means we've already received a net win percentage.
        if (s_acted.repeated == s_nonacted.repeated && s_acted.pct == s_nonacted.pct && s_acted.splits == s_nonacted.splits) {
            p_cl = s_acted.loss;
            p_cw = s_acted.wins;
        } else {
            std::cerr << "TODO(from yuzisee): !bConvertToNet for unequal s_acted s_nonacted is not yet implemented" << std::endl;
            exit(1);
        }
    }

    shape.loss = 1.0 - cleanpow(1.0 - p_cl,1.0/f_battle); //The w+s outcome for all players should be a power of w+s for shape
    shape.wins = cleanpow(p_cw,1.0/f_battle);
    shape.splits = 1.0 - shape.loss - shape.wins;

#ifdef DEBUG_TRACE_SEARCH
    std::cout << "\t\t\t(1 -  " <<    s_acted.loss << ")^" << s_acted.repeated    << "  *  ";
    std::cout <<       "(1 -  " << s_nonacted.loss << ")^" << s_nonacted.repeated << "   =   p_cl "  << p_cl << std::endl;
    std::cout << "\t\t\t" << s_acted.wins  << "^s_acted.repeated  *  " <<
    s_nonacted.wins << "^s_nonacted.repeated   =   p_cw "  << p_cw << std::endl;
#endif

#ifdef DEBUGASSERT
    if ((shape.loss != shape.loss) || (shape.wins != shape.wins) || (shape.splits != shape.splits)) {
        std::cerr << "NaN encountered in shape" << endl;
        exit(1);
    }
#endif // DEBUGASSERT

    forceRenormalize();

#ifdef DEBUG_TRACE_SEARCH
    std::cout << "\t\t\t\t p_cl after totalEnemy is  " << p_cl << std::endl;
    std::cout << "\t\t\t\t p_cw after totalEnemy is  " << p_cw << std::endl;
#endif

    shape.repeated = 1;
}

void CombinedStatResultsGeom::forceRenormalize()
{
    shape.forceRenormalize(); ///Normalize just in case; total possibility must add up to 1

    const float64 newTotal = p_cl + p_cw;
    //const float64 p_c_split = 1.0 - newTotal;

    ///Normalize, total possibilities must add up to 1 (certain splits are impossible)
    float64 splitTotal = 0;
    for( int8 i=1;i<=e_battle;++i )
    {//Split with i
        splitTotal += HoldemUtil::nchoosep<float64>(e_battle,i)*std::pow(shape.wins,e_battle-i)*std::pow(shape.splits,i);
    }

    p_cl *= (1-splitTotal)/newTotal;
    p_cw *= (1-splitTotal)/newTotal;


    if( 1 - shape.splits <= DBL_EPSILON  || (shape.loss + shape.wins <= DBL_EPSILON) )
    {
        p_cl = 0;
        p_cw = 0;
        shape.wins = 1; //You need wins to split, and shape is only used to split so this okay
    }


#ifdef DEBUGASSERT
    if ((p_cl != p_cl) || (p_cw != p_cw)) {
        std::cerr << "NaN encountered in GainModel" << endl;
        exit(1);
    }
#endif // DEBUGASSERT

}


float64 HoldemFunctionModel::FindBestBet()
{
    const float64 myMoney = estat->maxBet();
    const float64 betToCall = estat->callBet();
    const float64 minRaiseBetTo = estat->minRaiseTo();

    if( myMoney < betToCall ) return myMoney;

    float64 desiredBet;

    if( myMoney < minRaiseBetTo )
    {//Won't have enough to raise, your raise will be all-in
        desiredBet = myMoney;
    }else
    {
        float64 maxRaiseBetTo = estat->maxRaiseAmount();

        desiredBet = FindMax(minRaiseBetTo,maxRaiseBetTo);


        if( desiredBet < minRaiseBetTo )
        {
            /*
             #ifdef DEBUGASSERT
             if( desiredBet < minRaiseBetTo - e->chipDenom()/4 )
             {
             #ifndef DEBUG_FUNCTIONCORE
             std::cout << "ASSERT desiredBet rounding " << desiredBet << " against minRaise,myMoney of " << minRaiseBetTo << "," << myMoney << endl;
             #endif
             std::cout << "Way too far from assertion minraise." << endl;
             exit(1);
             }
             #endif
             */
            desiredBet = minRaiseBetTo;
        }
    }

    ///desiredBet has been established, one way or another
    const float64 raiseGain = f(desiredBet);
    const float64 callGain = f(betToCall);

    if( callGain > raiseGain )
    {
        desiredBet = betToCall;
    }else
    {
        ///PURIFY
        float64 nextOptimal = desiredBet + quantum;
        float64 prevOptimal = desiredBet - quantum;
        if( desiredBet > myMoney ) desiredBet = myMoney; //due to rounding of desiredBet
        if( nextOptimal > myMoney ) nextOptimal = myMoney;
        if( prevOptimal < minRaiseBetTo ) prevOptimal = minRaiseBetTo;
        if( prevOptimal > myMoney ) prevOptimal = myMoney;

        if( nextOptimal != desiredBet && f(nextOptimal) > f(desiredBet) )
        {
            desiredBet = nextOptimal;
        }
        if( prevOptimal != desiredBet && f(prevOptimal) > f(desiredBet) )
        {
            desiredBet = prevOptimal;
        }
    }

    return desiredBet;
}

float64 HoldemFunctionModel::FindFoldBet(const float64 bestBet)
{

    const float64& myMoney = estat->maxBet();
    float64 desiredFold = FindZero(bestBet,myMoney, true);
    ///PURIFY
    float64 nextFold = desiredFold + quantum;
    float64 prevFold = desiredFold - quantum;
    if( nextFold > myMoney ) nextFold = myMoney;
    if( prevFold < bestBet ) prevFold = bestBet;

    if(  fabs(f(nextFold))  <  fabs(f(desiredFold))  )
    {
        desiredFold = nextFold;
    }
    if(  fabs(f(prevFold))  <  fabs(f(desiredFold))  )
    {
        desiredFold = prevFold;
    }

    return desiredFold;
}




// Returns a value as a fraction of your bankroll. Returning 0.0 means "lose everything"
float64 GainModelGeom::g(float64 betSize)
{

    if( betSize > estat->callBet() && betSize < estat->minRaiseTo() )
    {
        betSize = estat->callBet();
    }

    if( betSize > estat->maxBet() ) //Because of say, raiseGain
    {
        betSize = estat->maxBet();
    }

    float64 x = estat->betFraction(betSize);
    float64 exf = estat->betFraction(espec.exf(betSize));

    const float64 minexf = estat->minCallFraction(betSize); //Because of say, impliedFactor
    if( exf < minexf )
    {
        exf = minexf;
    }

    const float64 f_pot = estat->betFraction( estat->stagnantPot() );

#ifdef DEBUGVIEWINTERMEDIARIES
    const float64& t_w = shape.wins;
    const float64& t_s = shape.splits;
    const float64& t_l = shape.loss;
    const float64 t_1w = 1+exf;
    const float64 t_cw = std::pow(shape.wins,e_battle);
    const float64 t_1wp = std::pow(t_1w , t_cw);
    const float64 t_1l = 1-x ;
    const float64 t_cl = 1 - std::pow(1 - shape.loss,e_battle);
    const float64 t_1lp = std::pow(t_1l, t_cl);
#endif

    if( betSize < estat->callBet() && betSize < estat->maxBet() ) return 0.0; ///"Negative raise" means betting less than the minimum call = FOLD

    return h(x, betSize, exf, f_pot, fOutcome);
}




float64 GainModelGeom::h(float64 betFraction, float64 betSize, float64 exf, float64 f_pot, ICombinedStatResults & fOutcome) {
    const float64 base = ExpectedCallD::handBetBase();
    const float64 x = betFraction;

    const int8 e_call = fOutcome.splitOpponents();//const int8 e_call = static_cast<int8>(std::round(exf/x));
    const StatResult & splitShape = fOutcome.ViewShape(betSize);

    //const float64 exf_live = exf - f_pot;

    float64 sav=1;
    for(int8 i=1;i<=e_call;++i)
    {
        const float64 C = HoldemUtil::nchoosep<float64>(fOutcome.splitOpponents(),i) * std::pow(splitShape.wins,fOutcome.splitOpponents()-i) * std::pow(splitShape.splits,i);
        /*
         //In our model, we can assume that if it is obvious many (everyone) will split, only those who don't see that opportunity will definitely fold
         //  however if it is not clear there will be a split (few split) everybody will call as expected
         //The dragCalls multiplier achieves this:
         float64 dragCalls = i;
         dragCalls /= e_call;
         dragCalls = 1 - dragCalls;
         dragCalls = dragCalls * dragCalls - dragCalls + 1;

         sav *=  pow(
         base - x +( f_pot+x+exf_live*dragCalls )/(i+1)
         ,
         C
         );
         */
        sav *= std::pow(base + f_pot / (i+1), C);
    }

    //    const float64 t_result = t_1wp * t_1lp * sav - 1;

    const float64 winGain = std::pow(base+exf , fOutcome.getWinProb(betSize));
    const float64 loseGain = std::pow(base-x , fOutcome.getLoseProb(betSize));

    return

    (
     winGain
     *
     loseGain
     *sav)
    ;
    //return pow(1+f_pot+e_fix*e->pctWillCall()*x , pow(shape.wins,e_fix));  plays more cautiously to account for most people playing better cards only
    //return pow(1+f_pot+e_fix*e->pctWillCall()*x , pow(shape.wins,e_fix*e->pctWillCall()));

    //let's round e_fix downward on input
    //floor()
}


float64 GainModelGeom::f(const float64 betSize)
{
    const float64 wls = g(betSize);
    const float64 fx = wls;
    return fx;
}

// NOTE: This function is not completely accurate, since ViewShape is affected by betSize but it's derivative is not considered.
float64 GainModelGeom::gd(const float64 betSize, const float64 y)
{
    //const float64 exf = e->pctWillCall(x/qdenom);
    //const float64 dexf = e->pctWillCallD(x/qdenom) * f_pot/qdenom/qdenom;


    const float64 adjQuantum = quantum/4;

    const float64 fracQuantum = estat->betFraction(adjQuantum);

    if( betSize > estat->callBet()+adjQuantum && betSize < estat->minRaiseTo()-adjQuantum )
    {
#ifdef DEBUG_TRACE_SEARCH
        if(traceEnable != nullptr) std::cout << "\t\t\tWithin minraise, reevaluate... @ " << estat->callBet() << " and " << estat->minRaiseTo() << " instead of " << betSize << std::endl;
#endif


        const float64 splitDist = gd(estat->callBet(),y)*(estat->minRaiseTo()-betSize)+gd(estat->minRaiseTo(),y)*(estat->callBet()-betSize);
        return splitDist/(estat->minRaiseTo() - estat->callBet());
    }
    float64 x = estat->betFraction(betSize);
    float64 dx = estat->betFraction(1.0);
    if( x >= 1 ){
        //dx = 0.0;
        x = 1.0 - fracQuantum; //Approximate extremes to avoid division by zero
    }



    //const float64 qdenom = (2*x+f_pot);
    float64 exf = estat->betFraction(espec.exf(betSize));


    float64 dexf = espec.dexf(betSize); ///This is actually e->betFraction( e->dexf(betSize)*betSize/x ) = e->dexf(betSize)


    const float64 minexf = estat->minCallFraction(betSize); //Because of say, impliedFactor
    if( exf < minexf - fracQuantum )
    {
#ifdef DEBUG_TRACE_SEARCH
        if(traceEnable != nullptr) std::cout << "\t\t\tvery low exf for now: " << exf << " < " << minexf << std::endl;
#endif

        dexf = 0.0;
    }


    //const float64 dexf = e->dexf(betSize)*betSize/x; //Chain rule where d{ exf(x*B) } = dexf(x*B)*B  //Note: B is determined by betSize/x
    const float64 f_pot = estat->betFraction(estat->stagnantPot());

    //const float64 qdfe_minus_called = e_tocall*x*dexf + e_tocall*exf;n
    //const int8 e_call = static_cast<int8>(std::round(e_called + e_tocall - 0.5));

#ifdef DEBUGVIEWINTERMEDIARIES
    const StatResult & t_s = shape;
    const float64 & t_cl = p_cl;
    const float64 & t_cw = p_cw;
#endif


#ifdef DEBUG_TRACE_SEARCH
    if((traceEnable != nullptr) && betSize < estat->callBet()) std::cout << "\t\t\tbetSize would be a fold!" << betSize << std::endl;
#endif

    if( betSize < estat->callBet() ) return 1; ///"Negative raise" means betting less than the minimum call = FOLD

    #ifdef DEBUG_TRACE_SEARCH
        if(traceEnable != nullptr) std::cout << "\t\t\t\tGainModelGeom::gd → hdx(…,dexf = " << dexf << std::endl;
    #endif
    return hdx(x, betSize, exf, dexf, f_pot, dx, fOutcome, y) * estat->betFraction(1.0);

}

// NOTE: This function is not completely accurate, since ViewShape is affected by betSize but it's derivative is not considered.
float64 GainModelGeom::hdx(float64 betFraction, float64 betSize, float64 exf, float64 dexf, float64 f_pot, float64 dx, ICombinedStatResults & fOutcome, float64 y) {
    const float64 base = ExpectedCallD::handBetBase();
    const float64 x = betFraction;
    //const float64 exf_live = exf - f_pot;

    //const int8 e_call = static_cast<int8>(std::round(exf/x)); //This choice of e_call might break down in extreme stack size difference situations
    //const int8 e_call = fOutcome.splitOpponents(); //Probably manditory if dragCalls is used
    //const StatResult & splitShape = fOutcome.ViewShape(betSize);


    float64 savd=0;
    /*
     for(int8 i=1;i<=e_call;++i)
     {
     float64 dragCalls = e_call - i;
     dragCalls *= dragCalls;
     dragCalls /= static_cast<float64>(e_call);
     dragCalls += i;

     if( dragCalls != 0 )
     {
     //     log sav = C *   log ( base -x + ( x + f_pot + exf_live * dragCalls) / (i+1))
     // d sav / sav = C * d log ( base -x + ( x + f_pot + exf_live * dragCalls) / (i+1))
     // d sav = sav * C *    {d ( base -x + ( x + f_pot + exf_live * dragCalls) / (i+1)) } / sav_root
     // d sav = sav * C *       ( base - 1.0 dx + ( 1.0 dx + dexf * dragCalls ) / (i+1))  / sav_root
     // d sav / sav = C *       ( base - 1.0 dx + ( 1.0 dx + dexf * dragCalls ) / (i+1))  / sav_root
     //
     // OR
     //
     //         sav =     ( -x + ( x + f_pot + exf_live * dragCalls) / (i+1)) ^ C
     //       d sav = C * ( -x + ( x + f_pot + exf_live * dragCalls) / (i+1)) ^ (C-1) * d ( -x + ( x + f_pot + exf_live * dragCalls) / (i+1))
     //       d sav / sav = C * sav_root^(C-2) * d ( -x + ( x + f_pot + exf_live * dragCalls) / (i+1))
     //       d sav / sav = C * sav_root^(C-2) *  ( - 1.0 dx + ( 1.0 dx + dexf * dragCalls) / (i+1))
     //
     const float64 sav_root = ( base -x + ( x + f_pot + exf_live * dragCalls) / (i+1));
     const float64 C = HoldemUtil::nchoosep<float64>(fOutcome.splitOpponents(),i)*pow(splitShape.wins,fOutcome.splitOpponents()-i)*pow(splitShape.splits,i);


     savd += C * (-1.0 + (1.0 + dexf * dragCalls) / (i+1)) / sav_root;
     }///Else you'd just {savd+=0;} anyways
     }*/

    //y is passed in as (y+e->foldGain()) which essentially gives you g()

    return
    (y)*
    (
     /*fOutcome.getWinProb(betSize)*dexf/(1+exf)

      -(fOutcome.getLoseProb(betSize))/(1-x)
      */


     //             d log pow(base+exf , fOutcome.getWinProb(betSize))
     //             d fOutcome.getWinProb(betSize) log (base+exf)
     //     {           d getWinProb(betSize)            } log (base+exf) + getWinProb(betSize) * d log (base+exf)
     //     { get_d_WinProb_dbetSize(betSize) dBetSize   } log (base+exf) + getWinProb(betSize) * dexf / (base+exf) dx
     //     { get_d_WinProb_dbetSize(betSize) dx * MONEY } log (base+exf) + getWinProb(betSize) * dexf / (base+exf) dx
     // betSize / MONEY = x
     // dBetSize = dx * MONEY


     //             d fOutcome.getLoseProb(betSize) log (base-x)
     //     { get_d_LoseProb_dbetSize(betSize) dx * MONEY } log (base-x) + getLoseProb(betSize) * d log (base-x)
     //     { get_d_LoseProb_dbetSize(betSize) dx * MONEY } log (base-x) + getLoseProb(betSize) * (-1.0 dx) / (base-x)

     //     const float64 loseGain = pow(base-x , fOutcome.getLoseProb(betSize));

     //  df_dx = df_dBetSize * dBetsize_dx

     //
     //     d_dbetSize log{const float64 winGain = pow(base+exf , fOutcome.getWinProb(betSize));}
     //     d_dbetSize fOutcome.getWinProb(betSize) log{base+exf}
     //     {d_dbetSize fOutcome.getWinProb(betSize)} log{base+exf} + fOutcome.getWinProb(betSize) d_dbetSize log{base+exf}
     //     fOutcome.get_d_WinProb_dbetSize(betSize) log{base+exf} + fOutcome.getWinProb(betSize) dexf/{base+exf}
     fOutcome.get_d_WinProb_dbetSize(betSize) * log(base + exf) / dx + fOutcome.getWinProb(betSize) * dexf / (base + exf)
     //     d_dbetSize log{const float64 loseGain = pow(base-x , fOutcome.getLoseProb(betSize));}
     //     d_dbetSize fOutcome.getLoseProb(betSize) log{base-x}
     //     {d_dbetSize fOutcome.getLoseProb(betSize)} log{base-x} + fOutcome.getLoseProb(betSize) d_dbetSize log{base-x}
     //     fOutcome.get_d_LoseProb_dbetSize(betSize) log{base-x} + fOutcome.getLoseProb(betSize) (-dx)/(base-x)
     + fOutcome.get_d_LoseProb_dbetSize(betSize) * log(base-x) / dx + fOutcome.getLoseProb(betSize) * (-1.0) / (base-x)

     + savd
     );
}

float64 GainModelGeom::fd(const float64 betSize, const float64 y)
{
    const float64 betVal = gd(betSize, y);

#ifdef DEBUG_TRACE_SEARCH
    if(traceEnable != nullptr) std::cout << "\t\tfd figures " << betVal << std::endl;
#endif

    return betVal;
}



// Returns a value as a fraction of your bankroll. Returning 0.0 means "lose everything"
float64 GainModelNoRisk::g(float64 betSize)
{

    if( betSize > estat->callBet() && betSize < estat->minRaiseTo() )
    {
        betSize = estat->callBet();
    }

    float64 x = estat->betFraction(betSize);
    float64 exf = estat->betFraction(espec.exf(betSize));

    const float64 minexf = estat->minCallFraction(betSize); //Because of say, impliedFactor
    if( exf < minexf )
    {
        exf = minexf;
    }


    const float64 f_pot = estat->betFraction(estat->stagnantPot());

#ifdef DEBUGVIEWINTERMEDIARIES

    const float64& t_w = shape.wins;
    const float64& t_s = shape.splits;
    const float64& t_l = shape.loss;
    const float64 t_1w = 1+exf;
    const float64 t_cw = std::pow(shape.wins,e_battle);
    const float64 t_1wp = (t_1w * t_cw);
    const float64 t_1l = 1-x ;
    const float64 t_cl = 1 - std::pow(1 - shape.loss,e_battle);
    const float64 t_1lp = (t_1l* t_cl);

#endif

    if( betSize < estat->callBet() && betSize < estat->maxBet() ) return 0.0; ///"Negative raise" means betting less than the minimum call = FOLD

    #ifdef DEBUG_TRACE_SEARCH
        if(traceEnable != nullptr)
        {
          const StatResult & inShape = fOutcome.ViewShape(betSize);
            std::cout << "\t\t\t(t_w,t_s,t_l) " << inShape.wins << "," << inShape.splits << "," << inShape.loss << std::endl;
            std::cout << "\t\t\t(betSize,f_pot) " << x << " , " << f_pot << " , " << std::endl;
            std::cout << "\t\t\tt_1w " << (1+exf) << std::endl;
            std::cout << "\t\t\tt_1l " << (1-x) << std::endl;
        }
    #endif
    return h(x, betSize, exf, f_pot, fOutcome);
}

float64 GainModelNoRisk::h(float64 betFraction, float64 betSize, float64 exf, float64 f_pot,  ICombinedStatResults & fOutcome) {
    const float64 base = ExpectedCallD::handBetBase();
    const float64 x = betFraction;
    //const float64 exf_live = exf - f_pot;


    const int8& e_call = fOutcome.splitOpponents();//const int8 e_call = static_cast<int8>(std::round(exf/x));
    const StatResult & splitShape = fOutcome.ViewShape(betSize);

    // Sometimes exf is lower than the number of people we'd consider for split.
    // Only people who have put their money in can be part of the split, of course.
    //float64 maxOpposingSplittersBasedOnExf = std::floor(exf / x);

    float64 sav=0.0;
    for(int8 i=1;i<=e_call;++i)
    {

        //In our model, we can assume that if it is obvious many (everyone) will split, only those who don't see that opportunity will definately fold
        //  however if it is not clear there will be a split (few split) everybody will call as expected
        //The dragCalls multiplier achieves this:
        /*float64 dragCalls = 1; *//*i;
                                    dragCalls /= e_call;
                                    dragCalls = 1 - dragCalls;
                                    dragCalls = dragCalls * dragCalls - dragCalls + 1;
                                    */

        // Probability of this split
        const float64 C =  HoldemUtil::nchoosep<float64>(fOutcome.splitOpponents(),i)*std::pow(splitShape.wins,fOutcome.splitOpponents()-i)*std::pow(splitShape.splits,i)  ;

        //if (i - maxOpposingSplittersBasedOnExf <= DBL_EPSILON) {
        // This number of splitters is okay. Let's go ahead with it.

        const float64 sav_base =

        // Bet to split across (i+1) players

        // SIMPLIFYING ASSUMPTION:
        // Everyone who would know they're in the split for this outcome will call, anyone else will fold.
        // Then, it's just f_pot which is split amongst the splittors.
        // Everything else is

        // Otherwise, we have to use dragCalls to avoid extra exf from third players from falling into obvious two-player split situations.
        // And, if you don't do it right, sav can end up negative, which makes no sense.

        f_pot / (i+1)

        /*
         - x + // your current bet is deduced from your current money so it can be added to the prize pool
         ( x // Your current bet is shared among the splittors
         + f_pot+exf_live*dragCalls // The pot is split too (with the exf_live fraction discounted -- note: "f_pot + exf_live === exf"
         )/(i+1)
         */
        ;

        sav += sav_base * C;

        //    savPrbUsed += C;
        //} else {
        //    savPrbUnused += C;
        //}
    }



#ifdef DEBUGASSERT
    if (sav < 0.0) {
        std::cerr << "Splits are never negative because the people who split all had to call." << endl;
        exit(1);
    }
#endif // DEBUGASSERT

    //    const float64 t_result = t_1wp * t_1lp * sav - 1;

    const float64 onWin = exf * fOutcome.getWinProb(betSize);
    const float64 onLose = -x * fOutcome.getLoseProb(betSize);

    const float64 result = base +

    onWin
    +
    onLose
    +
    sav

    ;


#ifdef DEBUGASSERT
    if (result < 0.0) {
        std::cerr << "You can't lose more than all your money: GainModelNoRisk" << endl;
        exit(1);
    }
#endif // DEBUGASSERT

    return result;
}


float64 GainModelNoRisk::f(const float64 betSize)
{
    const float64 wls = g(betSize);
    const float64 fx = wls;
    return fx;
}


float64 GainModelNoRisk::gd(float64 betSize, const float64 y)
{

    const float64 adjQuantum = quantum/4;


    if( betSize > estat->callBet()+adjQuantum && betSize < estat->minRaiseTo() - adjQuantum )
    {
#ifdef DEBUG_TRACE_SEARCH
        if(traceEnable != nullptr) std::cout << "\t\t\tWithin minraise, reevaluate... @ " << estat->callBet() << " and " << estat->minRaiseTo() << " instead of " << betSize << std::endl;
#endif

        const float64 splitDist = gd(estat->callBet(),y)*(estat->minRaiseTo()-betSize)+gd(estat->minRaiseTo(),y)*(estat->callBet()-betSize);
        return splitDist/(estat->minRaiseTo() - estat->callBet());
    }

    float64 x = estat->betFraction(betSize);


    //const float64 dexf = e->dexf(betSize)*betSize/x; //Chain rule where d{ exf(x*B) } = dexf(x*B)*B
    float64 dexf = espec.dexf(betSize);
    float64 exf = estat->betFraction(espec.exf(betSize));

    const float64 minexf = estat->minCallFraction(betSize); //Because of say, impliedFactor
    if( exf < minexf )
    {
        exf = minexf;
        dexf = 0.0;
    }



#ifdef DEBUG_TRACE_SEARCH
    if((traceEnable != nullptr) && betSize < estat->callBet()) std::cout << "\t\t\tbetSize would be a fold!" << betSize << std::endl;
#endif
    if( betSize < estat->callBet() ) return 1; ///"Negative raise" means betting less than the minimum call = FOLD

    //     dh/dx * dx/dbetSize
    #ifdef DEBUG_TRACE_SEARCH
        if(traceEnable != nullptr) std::cout << "\t\t\t\tGainModelNoRisk::gd → hdx(…,dexf = " << dexf << std::endl;
    #endif
    return hdx(x, betSize, exf, dexf, fOutcome, y) * estat->betFraction(1.0);

}

float64 GainModelNoRisk::hdx(float64 betFraction, float64 betSize, float64 exf, float64 dexf, ICombinedStatResults & fOutcome, float64 y) {
    const float64 x = betFraction;

    //const int8 e_call = static_cast<int8>(std::round(exf/x)); //This choice of e_call might break down in extreme stack size difference situations
    //const int8 e_call = fOutcome.splitOpponents(); //Probably manditory if dragCalls is used
    //const StatResult & splitShape = fOutcome.ViewShape(betSize);


    float64 savd=0;
    /*
     for(int8 i=1;i<=e_call;++i)
     {
     const float64 C = HoldemUtil::nchoosep<float64>(fOutcome.splitOpponents(),i)
     *pow(splitShape.wins,fOutcome.splitOpponents()-i)
     *pow(splitShape.splits,i);

     // sav = C * ( f_pot / (i+1))

     savd +=
     */
    /*
     float64 dragCalls = e_call - i;
     dragCalls *= dragCalls;
     dragCalls /= static_cast<float64>(e_call);
     dragCalls += i;

     if( dragCalls != 0 )
     {
     // sav = C * ( -x + ( x + f_pot + exf_live * dragCalls) / (i+1))
     // d sav = C * ( - 1.0 dx + ( 1.0 dx + dexf * dragCalls ) / (i+1))

     savd += C
     *
     ((1.0 + dexf * dragCalls) / (i+1) - 1.0)
     ;
     }///Else you'd just {savd+=0;} anyways
     */
    //}

    return
    (
     fOutcome.getWinProb(betSize)*dexf + fOutcome.get_d_WinProb_dbetSize(betSize)*(exf)
     - (fOutcome.getLoseProb(betSize)) + (-x) * fOutcome.get_d_LoseProb_dbetSize(betSize)
     +
     savd
     );
}

float64 GainModelNoRisk::fd(const float64 betSize, const float64 y)
{
    return gd(betSize, y);
}


void SlidingPairFunction::query(float64 x)
{
    const float64 yl = left->f(x);
    const float64 yr = right->f(x);
    last_x = x;
    y = yl*(1-slider)+yr*slider;
    dy = left->fd(x,yl)*(1-slider) + right->fd(x,yr)*slider;
}

float64 SlidingPairFunction::f(const float64 x)
{
    if( last_x != x )
    {
        query(x);
    }
    return y;
}

float64 SlidingPairFunction::fd(const float64 x, const float64 y_dummy)
{
    if( last_x != x )
    {
        query(x);
    }
    return dy;
}
