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
#include "functionmodel.h"
#include "ai.h"




GainModelNoRisk::~GainModelNoRisk()
{
}


GainModel::~GainModel()
{
}


float64 GainModel::cleangeomeanpow(float64 b1, float64 x1, float64 b2, float64 x2, float64 f_battle)
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


void GainModel::combineStatResults(const StatResult s_acted, const StatResult s_nonacted)
{

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

        if( quantum == 0 ) quantum = 1;


    ///Use f_battle instead of e_battle, convert to equivelant totalEnemy
        p_cl =  1 - cleangeomeanpow(1 - s_acted.loss,s_acted.repeated , 1 - s_nonacted.loss,s_nonacted.repeated, f_battle);
        p_cw = cleangeomeanpow(s_acted.wins,s_acted.repeated , s_nonacted.wins,s_nonacted.repeated, f_battle);


        #ifdef DEBUG_TRACE_SEARCH
        std::cout << "\t\t\t(1 -  " <<    s_acted.loss << ")^" << s_acted.repeated    << "  *  ";
        std::cout <<       "(1 -  " << s_nonacted.loss << ")^" << s_nonacted.repeated << "   =   p_cl "  << p_cl << std::endl;
        std::cout << "\t\t\t" << s_acted.wins  << "^s_acted.repeated  *  " <<
                    s_nonacted.wins << "^s_nonacted.repeated   =   p_cw "  << p_cw << std::endl;
        #endif


        shape.loss = 1.0 - cleanpow(1.0 - p_cl,1.0/f_battle); //The w+s outcome for all players should be a power of w+s for shape
        shape.wins = cleanpow(p_cw,1.0/f_battle);
        shape.splits = 1.0 - shape.loss - shape.wins;


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

void GainModel::forceRenormalize()
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
    float64 desiredFold = FindZero(bestBet,myMoney);
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






float64 GainModel::g(float64 betSize)
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

    const int8 e_call = e_battle;//const int8 e_call = static_cast<int8>(round(exf/x));

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
                        HoldemUtil::nchoosep<float64>(e_battle,i) * pow(shape.wins,e_battle-i) * pow(shape.splits,i)
                );
	}

//    const float64 t_result = t_1wp * t_1lp * sav - 1;

	return

        (
        pow(base+exf , p_cw)
        *
        pow(base-x , p_cl)
        *sav)
	;
	//return pow(1+f_pot+e_fix*e->pctWillCall()*x , pow(shape.wins,e_fix));  plays more cautiously to account for most people playing better cards only
	//return pow(1+f_pot+e_fix*e->pctWillCall()*x , pow(shape.wins,e_fix*e->pctWillCall()));

	//let's round e_fix downward on input
	//floor()
}

float64 GainModel::f(const float64 betSize)
{
	const float64 fx = g(betSize) - estat->foldGain(espec.ed);
    return fx;
}


float64 GainModel::gd(const float64 betSize, const float64 y)
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
 	if( x >= 1 ) x = 1.0 - fracQuantum; //Approximate extremes to avoide division by zero




    //const float64 qdenom = (2*x+f_pot);
	float64 exf = estat->betFraction(espec.exf(betSize));


    const float64 minexf = estat->minCallFraction(betSize); //Because of say, impliedFactor
    if( exf < minexf - fracQuantum )
    {
        #ifdef DEBUG_TRACE_SEARCH
            if(bTraceEnable) std::cout << "\t\t\tvery low exf for now: " << exf << " < " << minexf << std::endl;
        #endif

        return 0; //Incremental decrease is zero, so incremental increase must be zero at the limit
    }


		//const float64 dexf = e->dexf(betSize)*betSize/x; //Chain rule where d{ exf(x*B) } = dexf(x*B)*B  //Note: B is determined by betSize/x
	const float64 dexf = espec.dexf(betSize); ///This is actually e->betFraction( e->dexf(betSize)*betSize/x ) = e->dexf(betSize)
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
    const int8 e_call = e_battle; //Probably manditory if dragCalls is used

	float64 savd=0;
	for(int8 i=1;i<=e_call;++i)
	{
        float64 dragCalls = e_call - i;
        dragCalls *= dragCalls;
        dragCalls /= static_cast<float64>(e_call);
        dragCalls += i;

        if( dragCalls != 0 )
        {
            savd += HoldemUtil::nchoosep<float64>(e_battle,i)*pow(shape.wins,e_battle-i)*pow(shape.splits,i)
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

    //y is passed in as (y+e->foldGain())
 	return
 	(y)*
	(
	p_cw*dexf/(1+exf)

	-(p_cl)/(1-x)
	+
	savd
	);

}

float64 GainModel::fd(const float64 betSize, const float64 y)
{

    const float64 efg = estat->foldGain(espec.ed);
    const float64 betVal = gd(betSize, y+efg);

    #ifdef DEBUG_TRACE_SEARCH
    if(bTraceEnable) std::cout << "\t\tfd figures " << betVal << std::endl;
    #endif

    return betVal;
}


StatResult GainModel::ComposeBreakdown(const float64 pct, const float64 wl)
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

    const int8& e_call = e_battle;//const int8 e_call = static_cast<int8>(round(exf/x));

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
                (        HoldemUtil::nchoosep<float64>(e_battle,i)*pow(shape.wins,e_battle-i)*pow(shape.splits,i)    )
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

    const float64 onWin = (base+exf) * p_cw;
    const float64 onLose = (base-x) * p_cl;
    const float64 onSplit = sav;

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


float64 GainModelNoRisk::gd(float64 betSize, const float64 y)
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

//    const float64 x = e->betFraction(betSize);

//	const float64 exf = e->betFraction(e->exf(betSize));

	//const float64 dexf = e->dexf(betSize)*betSize/x; //Chain rule where d{ exf(x*B) } = dexf(x*B)*B
	const float64 dexf = espec.dexf(betSize);


#ifdef DEBUG_TRACE_SEARCH
    if(bTraceEnable && betSize < estat->callBet()) std::cout << "\t\t\tbetSize would be a fold!" << betSize << std::endl;
#endif
    if( betSize < estat->callBet() ) return 1; ///"Negative raise" means betting less than the minimum call = FOLD


    //const int8 e_call = static_cast<int8>(round(exf/x)); //This choice of e_call might break down in extreme stack size difference situations
    const int8 e_call = e_battle; //Probably manditory if dragCalls is used

	float64 savd=0;
	for(int8 i=1;i<=e_call;++i)
	{
        float64 dragCalls = e_call - i;
        dragCalls *= dragCalls;
        dragCalls /= static_cast<float64>(e_call);
        dragCalls += i;

        if( dragCalls != 0 )
        {
            savd += HoldemUtil::nchoosep<float64>(e_battle,i)*pow(shape.wins,e_battle-i)*pow(shape.splits,i)
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
	p_cw*dexf
	-
	(p_cl)
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

