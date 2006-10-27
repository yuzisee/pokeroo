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

#include "functionbase.h"
#include <iostream>
#include <math.h>


using std::endl;

#ifndef round
#include <cmath>
inline float64 round(float64 a)
{
    return floor(a+0.5);
}

#endif


ScalarFunctionModel::~ScalarFunctionModel()
{
}

float64 ScalarFunctionModel::quadraticStep(float64 x1, float64 y1, float64 x2, float64 y2, float64 x3, float64 y3) const
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

float64 ScalarFunctionModel::trisectionStep(float64 x1, float64 y1, float64 x2, float64 y2, float64 x3, float64 y3) const
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
	    if( x2 - x1 > quantum/2 )
	    {
	        #ifdef SINGLETURNINGPOINT
	        cout << "MISUAGE OF FindTurningPoint!!!!!!!!!!!!!!!!!!" << endl;
	        exit(1);
	        #else
	        float64 leftmax = round(FindMax(x1,xb)/quantum)*quantum;
	        float64 rightmax = round(FindMax(xb,x2)/quantum)*quantum;
	        if( f(leftmax) > f(rightmax) )
	        {
	            return leftmax;
	        }
	        return rightmax;
	        #endif
	    }

        return round(xb/quantum)*quantum;
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
        if( x2 - x1 > quantum/2 )
        {
            #ifdef SINGLETURNINGPOINT
            cout << "MISUAGE OF FindTurningPoint!!!!!!!!!!!!!!!!!!" << endl;
            exit(1);
            #else
	        float64 leftmin = round(FindMin(x1,xb)/quantum)*quantum;
	        float64 rightmin = round(FindMin(xb,x2)/quantum)*quantum;
	        if( f(leftmin) < f(rightmin) )
	        {
	            return leftmin;
	        }
	        return rightmin;
	        #endif
        }

        return round(xb/quantum)*quantum;
    }


	return FindTurningPoint(x1, y1, xb, yb, x2, y2, -1);
}

float64 ScalarFunctionModel::SplitTurningPoint(float64 x1, float64 y1, float64 xa, float64 ya, float64 xb, float64 yb, float64 x2, float64 y2, float64 signDir)
{
//std::cout << "Split <" << x1 << "," << x2 << ">" << std::endl;
    if( x2 - x1 < quantum/2 )
    {
        return round(bisectionStep(x1,x2)/quantum)*quantum;
    }

    float64 leftCP, midCP, rightCP;
    float64 leftM, midM, rightM;

    if( signDir > 0 )
    {
        if( xa < xb )
        {
            leftCP = FindMax(x1,xa);
            midCP = FindMax(xa,xb);
            rightCP = FindMax(xb,x2);
        }else
        {
            leftCP = FindMax(x1,xb);
            midCP = FindMax(xb,xa);
            rightCP = FindMax(xa,x2);
        }
    }else // findMin instead of max
    {
        if( xa < xb )
        {
            leftCP = FindMin(x1,xa);
            midCP = FindMin(xa,xb);
            rightCP = FindMin(xb,x2);
        }else
        {
            leftCP = FindMin(x1,xb);
            midCP = FindMin(xb,xa);
            rightCP = FindMin(xa,x2);
        }
    }

    leftM = f(leftCP);
    midM = f(midCP);
    rightM = f(rightCP);
    if( signDir*(leftM - midM) > 0)
    {
        if( signDir*(leftM - rightM) >= 0 )
        {
            return leftCP;
        }//else //rightM is dominant to all
        {
            return rightCP;
        }
    }//else //leftM is not dominant
    {
        if( signDir*(midM - rightM) >= 0 )
        {
            return midCP;
        }//else //midM is not dominant either
        {
            return rightCP;
        }
    }
}


float64 ScalarFunctionModel::FindTurningPoint(float64 x1, float64 y1, float64 xb, float64 yb, float64 x2, float64 y2, float64 signDir)
{
    int8 stepMode = 0;
//std::cout << "Going {" << x1 << "," << x2 << "}" << std::endl;
    float64 yn;
    float64 xn;

	if( y1 == yb && yb == y2 )
	{
	    #ifdef SINGLETURNINGPOINT
		return xb;
		#else
        xn = trisectionStep(x1,y1,xb,yb,x2,y2);
        yn = f(xn);
        return SplitTurningPoint(x1,y1,xn,yn,xb,yb,x2,y2,signDir);
		#endif
	}

    while( (y1-yb)*(y2-yb) < 0 && x2 - x1 > quantum/2)
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


    while(x2 - x1 > quantum/2)
    {
        ++stepMode;
        stepMode %= 5;
        switch(stepMode)
        {
            case 0:
            case 3:
            case 4:
            default:
                xn = quadraticStep(x1,y1,xb,yb,x2,y2);
                if(!( xb != xn && xn < x2 && xn > x1 ))
                {
                    xn = searchStep(x1,y1,xb,yb,x2,y2);
                }
                break;
            case 1:
            case 2:
                xn = searchStep(x1,y1,xb,yb,x2,y2);
                break;
        }

        yn = f(xn);

        if( xb == xn )
        {
            x1 = xb;
            x2 = xb;
        }else
        if( signDir*(yn - y1) < 0 && signDir*(yn - y2) < 0 )
        {
            #ifdef SINGLETURNINGPOINT
                #ifdef BYPASS_ANOMALIES
                {
                    const float64 dyb = f(xb);
                    const float64 dyn = f(xn);

                    if( dyb == 0 )
                    {
                        return xb;
                    }
                    if( dyn == 0 )
                    {
                        return yn;
                    }

                    if( dyb*signDir > 0 && dyn*signDir < 0 )
                    {
                        if( xb < xn )
                        {
                            x1 = xb;
                            y1 = yb;
                            x2 = xn;
                            y2 = yn;

                            xb = bisectionStep(x1,x2);
                            yb = f(xb);
                        }else
                        {
                            cout << "UNABLE TO RECOVER: Function has multiple turning points!" << endl;
                            exit(1);
                            return xb;
                        }
                    }else if ( dyn*signDir > 0 && dyb*signDir < 0 )
                    {
                        if( xn < xb )
                        {
                            x1 = xn;
                            y1 = yn;
                            x2 = xb;
                            y2 = yb;

                            xb = bisectionStep(x1,x2);
                            yb = f(xb);
                        }else
                        {
                            cout << "UNABLE TO RECOVER: Function has multiple turning points!" << endl;
                            exit(1);
                            return xb;
                        }
                    }else //Slopes are either both positive or both negative
                    {
                        if( dyb*signDir > 0 )  //before turning point
                        {
                            if( xb < xn )
                            {
                                x1 = xb;
                                y1 = yb;
                                xb = xn;
                                yb = yn;
                            }else //xn < xb
                            {
                                x1 = xn;
                                y1 = yn;
                            }
                        }else //after turning point
                        {
                            if( xb < xn )
                            {
                                x2 = xn;
                                y2 = yn;
                            }else //xn < xb
                            {
                                x2 = xb;
                                y2 = yb;
                                xb = xn;
                                yb = yn;
                            }
                        }

                    }


                }
                #else
                    cout << "WARNING: Function has multiple turning points!" << endl;
                    exit(1);
                    return xb;
                #endif
            #else
                return SplitTurningPoint(x1,y1,xn,yn,xb,yb,x2,y2,signDir);
            #endif

        }
        else
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

float64 ScalarFunctionModel::bisectionStep(float64 x1, float64 x2) const
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



