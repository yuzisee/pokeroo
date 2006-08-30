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
#include <math.h>
#include "functionmodel.h"
#include "ai.h"

using std::cout;
using std::endl;

ScalarFunctionModel::~ScalarFunctionModel()
{
}

float64 ScalarFunctionModel::quadraticStep(float64 x1, float64 y1, float64 x2, float64 y2, float64 x3, float64 y3)
{
    float64 a1,a2,a3;


    a2 = (y3-y1)*x2;
    a1 = (y2-y3)*x1;
    a3 = (y1-y2)*x3;

    float64 xn = ( a2*x2 + a1*x1 + a3*x3 )/(a2+a1+a3)/2;

    /*if( xn < x1 ) return x1;
    if( xn > x3 ) return x3;*/
    return xn;

}

float64 ScalarFunctionModel::trisectionStep(float64 x1, float64 y1, float64 x2, float64 y2, float64 x3, float64 y3)
{
    float64 ldiff, rdiff;
    ldiff = (x2-x1);
    rdiff = (x3-x2);
    if( ldiff > rdiff )
    {
        return (x2 - ldiff/2);
    }
    else if( ldiff < rdiff )
    {
        return (x2 + rdiff/2);
    }
    else
    {
        if( y1 > y3 ) return (x2 - ldiff/2);
        return (x2 + rdiff/2);
    }


}

float64 ScalarFunctionModel::searchStep(float64 x1, float64 y1, float64 x2, float64 y2, float64 x3, float64 y3)
{
    float64 dx = fd(x2,y2);

    if( y1 > y2 && y3 > y2 )//  U  find a min
    {
        if( dx > 0 ) return (x2+x1)/2;
        return (x2+x3)/2;
    }
    else if( y1 < y2 && y3 < y2 )//  ^  find a max
    {
        if( dx > 0 ) return (x2+x3)/2;
        return (x2+x1)/2;
    }
    else
    {
        return trisectionStep(x1,y1,x2,y2,x3,y3);
    }


}

float64 ScalarFunctionModel::FindMax(float64 x1, float64 x2)
{
    float64 y1 = f(x1);
    float64 y2 = f(x2);

    float64 xb = bisectionStep(x1,x2);
    float64 yb = f(xb);

	if( yb <= y1 && yb <= y2)
	{
        cout << "MISUAGE OF FindTurningPoint!!!!!!!!!!!!!!!!!!" << endl;
        return xb;
    }

	return FindTurningPoint(x1, y1, xb, yb, x2, y2, 1);
}

float64 ScalarFunctionModel::FindMin(float64 x1, float64 x2)
{
    float64 y1 = f(x1);
    float64 y2 = f(x2);

    float64 xb = bisectionStep(x1,x2);
    float64 yb = f(xb);

	if( yb >= y1 && yb >= y2)
	{
        cout << "MISUAGE OF FindTurningPoint!!!!!!!!!!!!!!!!!!" << endl;
        return xb;
    }


	return FindTurningPoint(x1, y1, xb, yb, x2, y2, -1);
}


float64 ScalarFunctionModel::FindTurningPoint(float64 x1, float64 y1, float64 xb, float64 yb, float64 x2, float64 y2, float64 signDir)
{
    int8 stepMode = 0;

    float64 yn;
    float64 xn;

	if( y1 == yb && yb == y2 )
	{
		return xb;
	}

    while( (y1-yb)*(y2-yb) < 0 && x2 - x1 > quantum)
    {   ///(y1-yb) and (y2-yb) have different signs
        ///therefore y1 and y2 are OPPOSITE vertical directions from yb.
		if( y1*signDir > y2*signDir ) ///y1 is closer
		{
			x2 = xb;
			y2 = yb;
		}else//y2 is closer
		{
			x1 = xb;
			y1 = yb;
		}

		xb = bisectionStep(x1,x2);
		yb = f(xb);
	}


    while(x2 - x1 > quantum)
    {
        ++stepMode;
        stepMode %= 4;
        switch(stepMode)
        {
            case 0:
            case 2:
            case 3:
                xn = quadraticStep(x1,y1,xb,yb,x2,y2);
                if( xb == xn || xn > x2 || xn < x1 )
                {
                    xn = searchStep(x1,y1,xb,yb,x2,y2);
                }
                break;
            case 1:
                xn = searchStep(x1,y1,xb,yb,x2,y2);
                break;
        }

        yn = f(xn);

        if( signDir*yn < signDir*y1 && signDir*yn < signDir*y2 )
        {
            cout << "WARNING: Function has multiple turning points!" << endl;
            return xb;
        }


        if( yb == yn )
        {
            if( xb < xn )
            {
                x1 = xb;
                y1 = yb;
                x2 = xn;
                y2 = yn;
            }
            else// xn < xb
            {
                x1 = xn;
                y1 = yn;
                x2 = xb;
                y2 = yb;
            }
            xb = bisectionStep(x1,x2);
            yb = f(xb);
        }
        else if( yb*signDir > yn*signDir ) ///b is the dominant point
        {
            if( xb < xn )
            {
                x2 = xn;
                y2 = yn;
            }
            else// xn < xb
            {
                x1 = xn;
                y1 = yn;
            }
        }else //n is the dominant point
        {
            if( xb < xn )
            {
                x1 = xb;
                y1 = yb;
            }else// xn < xb
            {
                x2 = xb;
                y2 = yb;
            }
            xb = xn;
            yb = yn;
        }

        //xn = searchStep(x1,y1,xb,yb,x2,y2);
    }
    return round(bisectionStep(x1,x2)/quantum)*quantum;
}


float64 ScalarFunctionModel::newtonStep(float64 x1, float64 y1)
{
    return x1 - y1/fd(x1,y1);
}

float64 ScalarFunctionModel::bisectionStep(float64 x1, float64 x2)
{
    return (x1+x2)/2;
}

float64 ScalarFunctionModel::FindZero(float64 x1, float64 x2)
{

    float64 y1 = f(x1);
    float64 y2 = f(x2);

    if( y1 > 0 && y2 > 0 ) //x1*x2 > 0
    {
        if( y1 > y2 ) return x2;
        return x1;
    }

    if( y1 < 0 && y2 < 0 ) //x1*x2 > 0
    {
        if( y1 > y2 ) return x1;
        return x2;
    }

    float64 yb;
    float64 xb,xn;
    xb = bisectionStep(x1,x2);
    yb = f(xb);

    while( x2 - x1 > quantum )
    {

        xn = newtonStep(xb,yb);

        if(xn < x2 && xn > x1)
        {
            xb = xn;
        }///Otherwise we stay with xb which is bisection

        yb = f(xb);

        if( yb == 0 ) return xb;

        if( yb*y1 > 0 ) //same sign as y1
        {
            y1 = yb;
            x1 = xb;
        }
        else
        {
            y2 = yb;
            x2 = xb;
        }
        xb = bisectionStep(x1,x2);
        yb = f(xb);
    }
    return round(xb/quantum)*quantum;
}


float64 DummyFunctionModel::f(const float64 x) const
{
    return 1+2*(1-x)*x;
}

float64 DummyFunctionModel::fd(const float64 x, const float64 y) const
{

    return -4*x+2;
}

GainModel::~GainModel()
{
}

float64 GainModel::f(const float64 betSize) const
{

    const float64 f_pot = e->deadpotFraction();
	const float64 x = e->betFraction(betSize);
	const float64 exf = e->exf(betSize);



    const float64& t_w = shape.wins;
    const float64& t_s = shape.splits;
    const float64& t_l = shape.loss;
    const float64 t_1w = 1+f_pot+exf;
    const float64 t_cw = pow(shape.wins,e_battle);
    const float64 t_1wp = pow(t_1w , t_cw);
    const float64 t_1l = 1-x ;
    const float64 t_cl = 1 - pow(1 - shape.loss,e_battle);
    const float64 t_1lp = pow(t_1l, t_cl);


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
                    1+( f_pot+exf*dragCalls )/(i+1)
                        ,
                        HoldemUtil::nchoosep<float64>(e_battle,i)*pow(shape.wins,e_battle-i)*pow(shape.splits,i)
                );
	}

    const float64 t_result = t_1wp * t_1lp * sav - 1;

	return

        (
        pow(1+f_pot+exf , pow(shape.wins,e_battle))
        *
        pow(1-x , 1 - pow(1 - shape.loss,e_battle))
        *sav)
	-
		1
	;
	//return pow(1+f_pot+e_fix*e->pctWillCall()*x , pow(shape.wins,e_fix));  plays more cautiously to account for most people playing better cards only
	//return pow(1+f_pot+e_fix*e->pctWillCall()*x , pow(shape.wins,e_fix*e->pctWillCall()));

	//let's round e_fix downward on input
	//floor()
}


float64 GainModel::fd(const float64 betSize, const float64 y) const
{
	//const float64 exf = e->pctWillCall(x/qdenom);
	//const float64 dexf = e->pctWillCallD(x/qdenom) * f_pot/qdenom/qdenom;
    const float64 x = e->betFraction(betSize);
    const float64 f_pot = e->deadpotFraction();
    //const float64 qdenom = (2*x+f_pot);
	const float64 exf = e->exf(betSize);
	const float64 dexf = e->dexf(betSize);
	//const float64 qdfe_minus_called = e_tocall*x*dexf + e_tocall*exf;n
    //const int8 e_call = static_cast<int8>(round(e_called + e_tocall - 0.5));
    const int8 e_call = static_cast<int8>(round(exf/x)); //This choice of e_call might break down in extreme stack size difference situations

	float64 savd=0;
	for(int8 i=1;i<e_call;++i)
	{
        float64 dragCalls = e_call - i;
        dragCalls *= dragCalls;
        dragCalls /= static_cast<float64>(e_call);
        dragCalls += i;

		savd += HoldemUtil::nchoosep<float64>(e_battle,i)*pow(shape.wins,e_battle-i)*pow(shape.splits,i)
				*
				dexf
				/
				( (i+1+f_pot)/dragCalls + exf )
				;
	}

 	return
	(y+1)*
	(
	pow(shape.wins,e_battle)*dexf/(1+f_pot+exf)
	+
	(pow(1-shape.loss,e_battle)-1)/(1-x)
	+
	savd
	);

}



StatResult GainModel::ComposeBreakdown(const float64 pct, const float64 wl)
{
	StatResult a;

	if( wl == 1/2 )
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



