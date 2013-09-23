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

#include "functionmodel.h"
#include "ai.h"


static inline float64 cleanpow(float64 b, float64 x)
{
    if( b < DBL_EPSILON ) return 0;
    //if( b > 1 ) return 1;
    return pow(b,x);
}




GainModelNoRisk::~GainModelNoRisk()
{
}


GainModelGeom::~GainModelGeom()
{
}

void CombinedStatResultsPessimistic::query(float64 betSize) {
    if (fLastBetSize == betSize) {
        return;
    }
    fLastBetSize = betSize;

    const playernumber_t splitOpponents = fOpposingHands.fTable.NumberInHand().inclAllIn();

    fOpposingHands.query(betSize);
    const float64 fractionOfHandsToBeat = 1.0 / fOpposingHands.handsToBeat(); // If you have to beat N hands, expect the worst to be one of the worst 1/Nth
    const float64 fractionOfHandsToBeat_dbetSize = fOpposingHands.d_HandsToBeat_dbetSize();

    const std::pair<StatResult, float64> oddsAgainstbestXHands = fFoldCumu->oddsAgainstBestXHands(fractionOfHandsToBeat);
    const StatResult & showdownResults = oddsAgainstbestXHands.first;
    const float64 & d_showdownPct_dX = oddsAgainstbestXHands.second;

    // da_dbetSize = da_dX * dX_dbetSize
    // Here, X === fractionOfHandsToBeat === 1.0 / fOpposingHands.handsToBeat()
    //
    // d_dbetSize{fWinProb} = d_dbetSize(fFoldCumu->oddsAgainstBestXHands(fractionOfHandsToBeat))
    //                      = fFoldCumu->oddsAgainstBestXHands ' (fractionOfHandsToBeat) * d_dbetSize(fractionOfHandsToBeat)
    //                      = fFoldCumu->oddsAgainstBestXHands ' (fractionOfHandsToBeat) * d_dbetSize(1.0 / fOpposingHands.handsToBeat())
    //                      = fFoldCumu->oddsAgainstBestXHands ' (fractionOfHandsToBeat) * (-1.0 / fOpposingHands.handsToBeat()^2) * d_dbetSize(fOpposingHands.handsToBeat())
    //                      = d_showdownPct_dX * (-fractionOfHandsToBeat * fractionOfHandsToBeat) * fractionOfHandsToBeat_dbetSize

    // Simplification: Probability of winning the showdown is the probability of beating the single opponent with the best chance to beat you.
    fWinProb = showdownResults.wins;
    fLoseProb = showdownResults.loss;
    f_d_WinProb_dbetSize = d_showdownPct_dX * (-fractionOfHandsToBeat * fractionOfHandsToBeat) * fractionOfHandsToBeat_dbetSize;
    f_d_LoseProb_dbetSize = -f_d_WinProb_dbetSize;
    
    // To evaluate splits we need a per-player (win, split, loss).
    fSplitShape.loss = 1.0 - cleanpow(1.0 - fLoseProb, 1.0 / splitOpponents); //The w+s outcome for all players should be a power of w+s for shape
    fSplitShape.wins = cleanpow(fWinProb, 1.0 / splitOpponents);
    fSplitShape.splits = 1.0 - fSplitShape.loss - fSplitShape.wins;



#ifdef DEBUGASSERT
    if ((showdownResults.loss != showdownResults.loss) || (showdownResults.wins != showdownResults.wins) || (showdownResults.splits != showdownResults.splits)) {
        std::cerr << "NaN encountered in showdownResults shape" << endl;
        exit(1);
    }
#endif // DEBUGASSERT
    

    ///Normalize, total split possibilities must add up to showdownResults.split
    float64 splitTotal = 0.0;
    for( int8 i=1;i<=splitOpponents;++i )
    {//Split with i
        splitTotal += HoldemUtil::nchoosep<float64>(splitOpponents,i)*pow(fSplitShape.wins,splitOpponents-i)*pow(fSplitShape.splits,i);
    }
    const float64 rescaleSplitWin = cleanpow(fSplitShape.splits / splitTotal, 1.0 / splitOpponents); // We will rescale .wins and .splits by this amount. In total, that scales splitTotal by (fSplitShape.splits / splitTotal)
    fSplitShape.loss -= (fSplitShape.wins + fSplitShape.splits) * (rescaleSplitWin - 1.0); // Subtract any excess that would be created (e.g. if rescaleSplitWin > 1.0)
    fSplitShape.wins *= rescaleSplitWin;
    fSplitShape.splits *= rescaleSplitWin;
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
        fLoseProb = 0.0;
        fWinProb = 0.0;
        f_d_WinProb_dbetSize = 0.0;
        f_d_LoseProb_dbetSize = 0.0;
        fSplitShape.wins = 1.0; //You need wins to split, and shape is only used to split so this okay
    }


#ifdef DEBUGASSERT
    if ((fLoseProb != fLoseProb) || (fWinProb != fWinProb)) {
        std::cerr << "NaN encountered in fWinProb and/or fLoseProb" << endl;
        exit(1);
    }
#endif // DEBUGASSERT


}

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
            splitTotal += HoldemUtil::nchoosep<float64>(e_battle,i)*pow(shape.wins,e_battle-i)*pow(shape.splits,i);
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

float64 HoldemFunctionModel::GetFoldGain(CallCumulationD* const e, float64 * const foldWaitLength_out)
{
    return estat->foldGain(e,foldWaitLength_out);
}






float64 GainModelGeom::g(float64 betSize) const
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
    const float64 exf_live = exf - f_pot;

    const float64 base = estat->handBetBase();

    #ifdef DEBUGVIEWINTERMEDIARIES
        const float64& t_w = shape.wins;
        const float64& t_s = shape.splits;
        const float64& t_l = shape.loss;
        const float64 t_1w = 1+exf;
        const float64 t_cw = pow(shape.wins,e_battle);
        const float64 t_1wp = pow(t_1w , t_cw);
        const float64 t_1l = 1-x ;
        const float64 t_cl = 1 - pow(1 - shape.loss,e_battle);
        const float64 t_1lp = pow(t_1l, t_cl);
    #endif

    if( betSize < estat->callBet() && betSize < estat->maxBet() ) return -1; ///"Negative raise" means betting less than the minimum call = FOLD

    const int8 e_call = fOutcome.e_battle;//const int8 e_call = static_cast<int8>(round(exf/x));

	float64 sav=1;
	for(int8 i=1;i<=e_call;++i)
	{
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
                        HoldemUtil::nchoosep<float64>(fOutcome.e_battle,i) * pow(ViewShape().wins,fOutcome.e_battle-i) * pow(ViewShape().splits,i)
                );
	}

//    const float64 t_result = t_1wp * t_1lp * sav - 1;

    const float64 winGain = pow(base+exf , fOutcome.getWinProb(betSize));
    const float64 loseGain = pow(base-x , fOutcome.getLoseProb(betSize));

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
	const float64 fx = g(betSize) - estat->foldGain(espec.ed);
    return fx;
}

// NOTE: This function is not completely accurate, since ViewShape is affected by betSize but it's derivative is not considered.
float64 GainModelGeom::gd(const float64 betSize, const float64 y) const
{
	//const float64 exf = e->pctWillCall(x/qdenom);
	//const float64 dexf = e->pctWillCallD(x/qdenom) * f_pot/qdenom/qdenom;


    const float64 adjQuantum = quantum/4;

    const float64 fracQuantum = estat->betFraction(adjQuantum);

	if( betSize > estat->callBet()+adjQuantum && betSize < estat->minRaiseTo()-adjQuantum )
	{
   	    	#ifdef DEBUG_TRACE_SEARCH
				if(bTraceEnable) std::cout << "\t\t\tWithin minraise, reevaluate... @ " << estat->callBet() << " and " << estat->minRaiseTo() << " instead of " << betSize << std::endl;
			#endif


		const float64 splitDist = gd(estat->callBet(),y)*(estat->minRaiseTo()-betSize)+gd(estat->minRaiseTo(),y)*(estat->callBet()-betSize);
		return splitDist/(estat->minRaiseTo() - estat->callBet());
	}
	float64 x = estat->betFraction(betSize);
    float64 dx = estat->betFraction(1.0);
 	if( x >= 1 ){
        dx = 0.0;
        x = 1.0 - fracQuantum; //Approximate extremes to avoide division by zero
    }



    //const float64 qdenom = (2*x+f_pot);
	float64 exf = estat->betFraction(espec.exf(betSize));


    float64 dexf = espec.dexf(betSize); ///This is actually e->betFraction( e->dexf(betSize)*betSize/x ) = e->dexf(betSize)


    const float64 minexf = estat->minCallFraction(betSize); //Because of say, impliedFactor
    if( exf < minexf - fracQuantum )
    {
        #ifdef DEBUG_TRACE_SEARCH
            if(bTraceEnable) std::cout << "\t\t\tvery low exf for now: " << exf << " < " << minexf << std::endl;
        #endif

        dexf = 0.0;
    }

     const float64 base = estat->handBetBase();

		//const float64 dexf = e->dexf(betSize)*betSize/x; //Chain rule where d{ exf(x*B) } = dexf(x*B)*B  //Note: B is determined by betSize/x
    const float64 f_pot = estat->betFraction(estat->stagnantPot());
    const float64 exf_live = exf - f_pot;
	//const float64 qdfe_minus_called = e_tocall*x*dexf + e_tocall*exf;n
    //const int8 e_call = static_cast<int8>(round(e_called + e_tocall - 0.5));

        #ifdef DEBUGVIEWINTERMEDIARIES
            const StatResult & t_s = shape;
            const float64 & t_cl = p_cl;
            const float64 & t_cw = p_cw;
        #endif


    #ifdef DEBUG_TRACE_SEARCH
    if(bTraceEnable && betSize < estat->callBet()) std::cout << "\t\t\tbetSize would be a fold!" << betSize << std::endl;
    #endif

    if( betSize < estat->callBet() ) return 1; ///"Negative raise" means betting less than the minimum call = FOLD

    //const int8 e_call = static_cast<int8>(round(exf/x)); //This choice of e_call might break down in extreme stack size difference situations
    const int8 e_call = fOutcome.e_battle; //Probably manditory if dragCalls is used

	float64 savd=0;
	for(int8 i=1;i<=e_call;++i)
	{
        float64 dragCalls = e_call - i;
        dragCalls *= dragCalls;
        dragCalls /= static_cast<float64>(e_call);
        dragCalls += i;

        if( dragCalls != 0 )
        {
            savd += HoldemUtil::nchoosep<float64>(fOutcome.e_battle,i)*pow(ViewShape().wins,fOutcome.e_battle-i)*pow(ViewShape().splits,i)
                    *
                    dexf
                    /
                    ( (i+1+f_pot + x)/dragCalls + exf_live )
                    ;
        }///Else you'd just {savd+=0;} anyways
	}

    #ifdef DEBUG_TRACE_SEARCH
        if(bTraceEnable) std::cout << "\t\t\t\tdexf = " << dexf << std::endl;
    #endif

    //y is passed in as (y+e->foldGain()) which essentially gives you g()

     return
 	(y)*
	(
	/*fOutcome.getWinProb(betSize)*dexf/(1+exf)

	-(fOutcome.getLoseProb(betSize))/(1-x)
	*/

     //
//     d_dbetSize log{const float64 winGain = pow(base+exf , fOutcome.getWinProb(betSize));}
//     d_dbetSize fOutcome.getWinProb(betSize) log{base+exf}
//     {d_dbetSize fOutcome.getWinProb(betSize)} log{base+exf} + fOutcome.getWinProb(betSize) d_dbetSize log{base+exf}
//     fOutcome.get_d_WinProb_dbetSize(betSize) log{base+exf} + fOutcome.getWinProb(betSize) dexf/{base+exf}
    fOutcome.get_d_WinProb_dbetSize(betSize) * log(base + exf) + fOutcome.getWinProb(betSize) * dexf / (base + exf)
//     d_dbetSize log{const float64 loseGain = pow(base-x , fOutcome.getLoseProb(betSize));}
//     d_dbetSize fOutcome.getLoseProb(betSize) log{base-x}
//     {d_dbetSize fOutcome.getLoseProb(betSize)} log{base-x} + fOutcome.getLoseProb(betSize) d_dbetSize log{base-x}
//     fOutcome.get_d_LoseProb_dbetSize(betSize) log{base-x} + fOutcome.getLoseProb(betSize) (-dx)/(base-x)
     + fOutcome.get_d_LoseProb_dbetSize(betSize) * log(base-x) - fOutcome.getLoseProb(betSize) * dx/(base-x)
     
	+ savd
	);

}

float64 GainModelGeom::fd(const float64 betSize, const float64 y)
{

    const float64 efg = estat->foldGain(espec.ed);
    const float64 betVal = gd(betSize, y+efg);

    #ifdef DEBUG_TRACE_SEARCH
    if(bTraceEnable) std::cout << "\t\tfd figures " << betVal << std::endl;
    #endif

    return betVal;
}


StatResult CombinedStatResultsGeom::ComposeBreakdown(const float64 pct, const float64 wl)
{
	StatResult a;

	if( wl == 1.0/2.0)
	{///PCT is 0.5 here also
		a.wins = 0.5;
		a.loss = 0.5;
		a.splits = 0;
	}
	else
	{
		a.wins = (2*pct - 1)*wl/(2*wl-1);
		a.splits = (pct - a.wins)*2;
		a.loss = 1 - a.wins - a.splits;
		a.genPCT();
	}
	return a;
}



float64 GainModelNoRisk::g(float64 betSize) const
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
    const float64 exf_live = exf - f_pot;

    const float64 base = estat->handBetBase();

        #ifdef DEBUGVIEWINTERMEDIARIES

            const float64& t_w = shape.wins;
            const float64& t_s = shape.splits;
            const float64& t_l = shape.loss;
            const float64 t_1w = 1+exf;
            const float64 t_cw = pow(shape.wins,e_battle);
            const float64 t_1wp = (t_1w * t_cw);
            const float64 t_1l = 1-x ;
            const float64 t_cl = 1 - pow(1 - shape.loss,e_battle);
            const float64 t_1lp = (t_1l* t_cl);

        #endif

    if( betSize < estat->callBet() && betSize < estat->maxBet() ) return -1; ///"Negative raise" means betting less than the minimum call = FOLD

    const int8& e_call = fOutcome.e_battle;//const int8 e_call = static_cast<int8>(round(exf/x));

	float64 sav=0;
	for(int8 i=1;i<=e_call;++i)
	{
        //In our model, we can assume that if it is obvious many (everyone) will split, only those who don't see that opportunity will definately fold
        //  however if it is not clear there will be a split (few split) everybody will call as expected
        //The dragCalls multiplier achieves this:
        float64 dragCalls = i;
        dragCalls /= e_call;
        dragCalls = 1 - dragCalls;
        dragCalls = dragCalls * dragCalls - dragCalls + 1;

		sav +=
                (    base - x+( f_pot+x+exf_live*dragCalls )/(i+1)    )
                        *
                (        HoldemUtil::nchoosep<float64>(fOutcome.e_battle,i)*pow(ViewShape().wins,fOutcome.e_battle-i)*pow(ViewShape().splits,i)    )
                ;
	}

//    const float64 t_result = t_1wp * t_1lp * sav - 1;

#ifdef DEBUG_TRACE_SEARCH
    if(bTraceEnable)
    {
         std::cout << "\t\t\tbase+exf " << (base+exf)  << "   *   p_cw "  << p_cw << std::endl;
         std::cout << "\t\t\tbase-x " << (base-x)  << "   *   p_cl "  << p_cl << std::endl;
         std::cout << "\t\t\tsav " << (sav)  << std::endl;
    }
#endif

    const float64 onWin = (base+exf) * fOutcome.getWinProb(betSize);
    const float64 onLose = (base-x) * fOutcome.getLoseProb(betSize);
    
	return

		   onWin
        +
           onLose
        +
		   sav

	;

	//let's round e_fix downward on input
	//floor()
}

float64 GainModelNoRisk::f(const float64 betSize)
{
    const float64 wls = g(betSize);
    const float64 batna = estat->foldGain(espec.ed);
	const float64 fx = wls - batna;
    return fx;
}


// NOTE: This function is not completely accurate, since ViewShape is affected by betSize but it's derivative is not considered.
float64 GainModelNoRisk::gd(float64 betSize, const float64 y) const
{

    const float64 adjQuantum = quantum/4;


	if( betSize > estat->callBet()+adjQuantum && betSize < estat->minRaiseTo() - adjQuantum )
	{
	    	#ifdef DEBUG_TRACE_SEARCH
				if(bTraceEnable) std::cout << "\t\t\tWithin minraise, reevaluate... @ " << estat->callBet() << " and " << estat->minRaiseTo() << " instead of " << betSize << std::endl;
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

    const float64 base = estat->handBetBase();


#ifdef DEBUG_TRACE_SEARCH
    if(bTraceEnable && betSize < estat->callBet()) std::cout << "\t\t\tbetSize would be a fold!" << betSize << std::endl;
#endif
    if( betSize < estat->callBet() ) return 1; ///"Negative raise" means betting less than the minimum call = FOLD


    //const int8 e_call = static_cast<int8>(round(exf/x)); //This choice of e_call might break down in extreme stack size difference situations
    const int8 e_call = fOutcome.e_battle; //Probably manditory if dragCalls is used

	float64 savd=0;
	for(int8 i=1;i<=e_call;++i)
	{
        float64 dragCalls = e_call - i;
        dragCalls *= dragCalls;
        dragCalls /= static_cast<float64>(e_call);
        dragCalls += i;

        if( dragCalls != 0 )
        {
            savd += HoldemUtil::nchoosep<float64>(fOutcome.e_battle,i)
            *pow(ViewShape().wins,fOutcome.e_battle-i)
            *pow(ViewShape().splits,i)
                    *
                    dexf * dragCalls
                    /
					(i+1)
                    ;
        }///Else you'd just {savd+=0;} anyways
	}



			#ifdef DEBUG_TRACE_SEARCH
				if(bTraceEnable) std::cout << "\t\t\t\tdexf = " << dexf << std::endl;
			#endif


 	return
	(
	fOutcome.getWinProb(betSize)*dexf + fOutcome.get_d_WinProb_dbetSize(betSize)*(base + exf)
	- (fOutcome.getLoseProb(betSize)) + (base-x) * fOutcome.get_d_LoseProb_dbetSize(betSize)
	+
	savd
	);

}

float64 GainModelNoRisk::fd(const float64 betSize, const float64 y)
{
    return gd(betSize, y+estat->foldGain(espec.ed));
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

