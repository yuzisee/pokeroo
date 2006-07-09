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

    if( y1 > y2 && y3 > y2 )//  U
    {
        if( dx > 0 ) return (x2+x1)/2;
        return (x2+x3)/2;
    }
    else if( y1 < y2 && y3 < y2 )// ^
    {
        if( dx > 0 ) return (x2+x3)/2;
        return (x2+x1)/2;
    }
    else
    {
        return trisectionStep(x1,y1,x2,y2,x3,y3);
    }


}

float64 ScalarFunctionModel::FindTurningPoint(float64 x1, float64 x2)
{
    int8 stepMode = 0;

    float64 signDir;
    float64 y1,yb,y2,yn;
    float64 xb;
    float64 xn;

    y1 = f(x1);
    y2 = f(x2);

    xb = bisectionStep(x1,x2);
    yb = f(xb);


    if( (y1-yb)*(y2-yb) < 0 )
    {   ///(y1-yb) and (y2-yb) have different signs
        ///therefore y1 and y2 are OPPOSITE vertical directions from yb. fd is not monotomic.
        cout << "MISUAGE OF FindTurningPoint!!!!!!!!!!!!!!!!!!" << endl;
        return xb;
    }

    if( yb >= y1 )
    {
        signDir = +1;
    }
    else if( yb <= y2 )
    {
        signDir = -1;
    }
    else
    {///They must be equal
        cout << "MISUAGE OF FindTurningPoint!!!!!!!!!!!!!!!!!!" << endl;
        return xb;
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

        if( signDir*yn < signDir*y1 || signDir*yn < signDir*y2 )
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


float64 DummyFunctionModel::f(float64 x) const
{
    return 1+2*(1-x)*x;
}

float64 DummyFunctionModel::fd(float64 x, float64 y) const
{

    return -4*x+2;
}



