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

#include <iostream>
#include "functionmodel.h"
#include "ai.h"

//#define DEBUGVIEWINTERMEDIARIES


/*
float64 DummyFunctionModel::f(const float64 x) const
{
    return 1+2*(1-x)*x;
}

float64 DummyFunctionModel::fd(const float64 x, const float64 y) const
{

    return -4*x+2;
}
*/
#ifndef NO_AWKWARD_MODELS
GainModelReverseNoRisk::~GainModelReverseNoRisk()
{
}

GainModelReverse::~GainModelReverse()
{
}
#endif

GainModelNoRisk::~GainModelNoRisk()
{
}

GainModel::~GainModel()
{
}

float64 HoldemFunctionModel::FindBestBet()
{
    const float64& myMoney = e->maxBet();
    const float64& betToCall = e->callBet();

    if( myMoney < betToCall ) return myMoney;
    float64 desiredBet = FindMax(betToCall,myMoney);
    
    
    
    ///PURIFY
    float64 nextOptimal = desiredBet + quantum;
    float64 prevOptimal = desiredBet - quantum;
    if( nextOptimal > myMoney ) nextOptimal = myMoney;
    if( prevOptimal < betToCall ) prevOptimal = betToCall;
    
    if( f(nextOptimal) > f(desiredBet) )
    {
        desiredBet = nextOptimal;
    }
    if( f(prevOptimal) > f(desiredBet) )
    {
        desiredBet = prevOptimal;
    }
    
    return desiredBet;
}

float64 HoldemFunctionModel::FindFoldBet(const float64 bestBet)
{
    const float64& myMoney = e->maxBet();
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


float64 GainModel::f(const float64 betSize)
{

	const float64 x = e->betFraction(betSize);
	float64 exf = e->betFraction(e->exf(betSize));
	if( exf > 1 ) exf = 1; //This is necessary because of impliedFactor
    const float64 f_pot = e->betFraction( e->prevpotChips() );
    const float64 exf_live = exf - f_pot;

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

    if( betSize < e->callBet() && betSize < e->maxBet() ) return 0; ///"Negative raise" means betting less than the minimum call = FOLD

    const int8& e_call = e_battle;//const int8 e_call = static_cast<int8>(round(exf/x));

	float64 sav=1;
	for(int8 i=1;i<=e_call;++i)
	{
        //In our model, we can assume that if it is obvious many (everyone) will split, only those who don't see that opportunity will definately fold
        //  however if it is not clear there will be a split (few split) everybody will call as expected
        //The dragCalls multiplier achieves this:
        float64 dragCalls = i;
        dragCalls /= e_call;
        dragCalls = 1 - dragCalls;
        dragCalls = dragCalls * dragCalls - dragCalls + 1;

		sav *=  pow(
                    1+( f_pot+exf_live*dragCalls )/(i+1)
                        ,
                        HoldemUtil::nchoosep<float64>(e_battle,i)*pow(shape.wins,e_battle-i)*pow(shape.splits,i)
                );
	}

//    const float64 t_result = t_1wp * t_1lp * sav - 1;

	return

        (
        pow(1+exf , p_cw)
        *
        pow(1-x , p_cl)
        *sav)
	-
		e->foldGain()
	;
	//return pow(1+f_pot+e_fix*e->pctWillCall()*x , pow(shape.wins,e_fix));  plays more cautiously to account for most people playing better cards only
	//return pow(1+f_pot+e_fix*e->pctWillCall()*x , pow(shape.wins,e_fix*e->pctWillCall()));

	//let's round e_fix downward on input
	//floor()
}


float64 GainModel::fd(const float64 betSize, const float64 y)
{

	//const float64 exf = e->pctWillCall(x/qdenom);
	//const float64 dexf = e->pctWillCallD(x/qdenom) * f_pot/qdenom/qdenom;
    float64 x = e->betFraction(betSize);
    if( x == 1 ) x -= quantum/4; //Approximate extremes to avoide division by zero

    //const float64 qdenom = (2*x+f_pot);
	float64 exf = e->betFraction(e->exf(betSize));
	if( exf > 1 ) exf = 1; //This is necessary because of impliedFactor
		//const float64 dexf = e->dexf(betSize)*betSize/x; //Chain rule where d{ exf(x*B) } = dexf(x*B)*B
	const float64 dexf = e->dexf(betSize);
    const float64 f_pot = e->betFraction(e->prevpotChips());
    const float64 exf_live = exf - f_pot;
	//const float64 qdfe_minus_called = e_tocall*x*dexf + e_tocall*exf;n
    //const int8 e_call = static_cast<int8>(round(e_called + e_tocall - 0.5));

        #ifdef DEBUGVIEWINTERMEDIARIES
            const StatResult & t_s = shape;
            const float64 & t_cl = p_cl;
            const float64 & t_cw = p_cw;
        #endif


    if( betSize < e->callBet() ) return 1; ///"Negative raise" means betting less than the minimum call = FOLD

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
                    ( (i+1+f_pot)/dragCalls + exf_live )
                    ;
        }///Else you'd just {savd+=0;} anyways
	}

 	return
	(y+e->foldGain())*
	(
	p_cw*dexf/(1+exf)

	-(p_cl)/(1-x)
	+
	savd
	);

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



float64 GainModelNoRisk::f(const float64 betSize)
{

	const float64 x = e->betFraction(betSize);
	float64 exf = e->betFraction(e->exf(betSize));
	if( exf > 1 ) exf = 1; //This is necessary because of impliedFactor
    const float64 f_pot = e->betFraction(e->prevpotChips());
    const float64 exf_live = exf - f_pot;

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

    if( betSize < e->callBet() && betSize < e->maxBet() ) return 0; ///"Negative raise" means betting less than the minimum call = FOLD

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
                (    1+( f_pot+exf_live*dragCalls )/(i+1)    )
                        *
                (        HoldemUtil::nchoosep<float64>(e_battle,i)*pow(shape.wins,e_battle-i)*pow(shape.splits,i)    )
                ;
	}

//    const float64 t_result = t_1wp * t_1lp * sav - 1;

	return

		   (1+exf) * p_cw
        +
           (1-x) * p_cl
        +
		   sav
	-
		e->foldGain()
	;

	//let's round e_fix downward on input
	//floor()
}


float64 GainModelNoRisk::fd(const float64 betSize, const float64 y)
{

//    const float64 x = e->betFraction(betSize);

//	const float64 exf = e->betFraction(e->exf(betSize));

	//const float64 dexf = e->dexf(betSize)*betSize/x; //Chain rule where d{ exf(x*B) } = dexf(x*B)*B
	const float64 dexf = e->dexf(betSize);



    if( betSize < e->callBet() ) return 1; ///"Negative raise" means betting less than the minimum call = FOLD

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

 	return

	(
	p_cw*dexf
	-
	(p_cl)
	+
	savd
	);

}

#ifndef NO_AWKWARD_MODELS

/*
float64 GainModelReverse::FindBestBet()
{
    const float64& myMoney = e->maxBet();
    const float64& betToCall = e->callBet();
    if( myMoney < betToCall ) return myMoney;
    return FindMin(betToCall,myMoney);
}*/

float64 GainModelReverse::f(const float64 betSize)
{
    const float64 oppBankroll = e->allChips() - e->maxBet();
    const float64 oppFoldGain = 1 + e->forfeitChips()/oppBankroll;
	const float64 x = betSize / oppBankroll;
	float64 exf = e->exf(betSize)/oppBankroll;
	if( exf > 1 ) exf = 1; //This is necessary because of impliedFactor
    const float64 f_pot = e->prevpotChips()/oppBankroll;
    const float64 exf_live = exf - f_pot;


    if( betSize < e->callBet() && betSize < e->maxBet() ) return 0; ///"Negative raise" means betting less than the minimum call = FOLD

    const int8& e_call = e_battle;//const int8 e_call = static_cast<int8>(round(exf/x));

	float64 sav=1;
	for(int8 i=1;i<=e_call;++i)
	{
        //In our model, we can assume that if it is obvious many (everyone) will split, only those who don't see that opportunity will definately fold
        //  however if it is not clear there will be a split (few split) everybody will call as expected
        //The dragCalls multiplier achieves this:
        float64 dragCalls = i;
        dragCalls /= e_call;
        dragCalls = 1 - dragCalls;
        dragCalls = dragCalls * dragCalls - dragCalls + 1;

		sav *=  pow(
                    1-( f_pot+exf_live*dragCalls )/(i+1)
                        ,
                        HoldemUtil::nchoosep<float64>(e_battle,i)*pow(shape.wins,e_battle-i)*pow(shape.splits,i)
                );
	}

//Notice that if (exf > 1), then (e->exf(betSize) > oppBankroll) which is impossible. This caused a -1.#IND. bug in the past
	return
    oppFoldGain -
        (
        pow(1-exf , p_cw)
        *
        pow(1+x , p_cl)
        *sav)
	;
}


float64 GainModelReverse::fd(const float64 betSize, const float64 y)
{
    const float64 oppBankroll = e->allChips() - e->maxBet();
    const float64 oppFoldGain = 1 + e->forfeitChips()/oppBankroll;
	float64 x = betSize / oppBankroll;

    float64 exf = e->exf(betSize)/oppBankroll;
    if( exf > 1 ) exf = 1; //This is necessary because of impliedFactor

    if( exf == 1 )
    {
        x -= quantum/4; //Approximate extremes to avoide division by zero
        exf = e->exf(x * oppBankroll);
    }

	const float64 dexf = e->dexf(betSize);

    //if( x == 1 ) x -= quantum/4; //Approximate extremes to avoide division by zero

    const float64 f_pot = e->prevpotChips() / oppBankroll;
    const float64 exf_live = exf - f_pot;



    if( betSize < e->callBet() ) return 1; ///"Negative raise" means betting less than the minimum call = FOLD

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
                    (-dexf)
                    /
                    ( (i+1+f_pot)/dragCalls - exf_live )
                    ;
        }///Else you'd just {savd+=0;} anyways
	}

 	return
 	(
            (y-oppFoldGain)*
            (
            p_cw*(-dexf)/(1-exf)

            +(p_cl)/(1+x)
            +
            savd
            )
	);
	;

}


float64 GainModelReverseNoRisk::f(const float64 betSize)
{

    const float64 oppBankroll = e->allChips() - e->maxBet();
    const float64 f_pot = e->prevpotChips()/oppBankroll;
    const float64 oppFoldGain = 1 + e->forfeitChips()/oppBankroll;
	const float64 x = betSize / oppBankroll;
	float64 exf = e->exf(betSize)/oppBankroll;
	if( exf > 1 ) exf = 1; //This is necessary because of impliedFactor
    const float64 exf_live = exf - f_pot;



    if( betSize < e->callBet() && betSize < e->maxBet() ) return 0; ///"Negative raise" means betting less than the minimum call = FOLD

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
                (    1-( f_pot+exf_live*dragCalls )/(i+1)    )
                        *
                (        HoldemUtil::nchoosep<float64>(e_battle,i)*pow(shape.wins,e_battle-i)*pow(shape.splits,i)    )
                ;
	}

//    const float64 t_result = t_1wp * t_1lp * sav - 1;

	return
    oppFoldGain
    -
    (
		   (1-exf) * p_cw
        +
           (1+x) * p_cl
        +
		   sav
	)
	;

	//let's round e_fix downward on input
	//floor()
}


float64 GainModelReverseNoRisk::fd(const float64 betSize, const float64 y)
{

//    const float64 oppBankroll = e->allChips() - e->maxBet();
//    const float64 oppFoldGain = 1 + e->forfeitChips();
//	float64 x = betSize / oppBankroll;
//    float64 exf = e->exf(betSize)/oppBankroll;
/*
    if( exf == 1 )
    {
        x -= quantum/4; //Approximate extremes to avoid division by zero
        exf = e->exf(x * oppBankroll);
    }
*/
	const float64 dexf = e->dexf(betSize);

//    const float64 f_pot = e->deadpotChips() / oppBankroll;
//    const float64 exf_live = exf - f_pot;



    if( betSize < e->callBet() ) return 1; ///"Negative raise" means betting less than the minimum call = FOLD

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
                    (-dexf) * dragCalls
                    /
					(i+1)
                    ;
        }///Else you'd just {savd+=0;} anyways
	}

 	return

	(
	-p_cw*(-dexf)
	-
	(p_cl)
	-
	savd
	);

}

#endif

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


void AutoScalingFunction::query(float64 x)
{
    const float64 yl = left->f(x);
    const float64 yr = right->f(x);
    last_x = x;

    const float64 autoSlope = saturate_upto / (saturate_max - saturate_min) ;
    const float64 slider = (x - saturate_min) * autoSlope ;


    if( slider > 1 )
    {
        y = yr;
        dy = fd(x, yr);
    }
    else if( slider < 0 )
    {
        y = yl;
        dy = fd(x, yl);
    }
    else
    {
        y = yl*(1-slider)+yr*slider;
        dy = left->fd(x,yl)*(1-slider) - yl*autoSlope   +   right->fd(x,yr)*slider + yr*autoSlope;
    }

}

float64 AutoScalingFunction::f(const float64 x)
{
    if( last_x != x )
    {
        query(x);
    }
    return y;
}

float64 AutoScalingFunction::fd(const float64 x, const float64 y_dummy)
{
    if( last_x != x )
    {
        query(x);
    }
    return dy;
}


