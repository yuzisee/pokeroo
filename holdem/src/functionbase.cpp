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

#include "functionbase.h"
#include <iostream>
#include <math.h>
#include <float.h>

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


bool ScalarFunctionModel::IsSameSignOrZero(const float64 &ya, const float64 &yb) const
{
	const bool yaZero = fabs(ya) < DBL_EPSILON;
	const bool ybZero = fabs(yb) < DBL_EPSILON;
	const bool yaPositive = ya > 0;
	const bool ybPositive = yb > 0;
	return (   yaZero || ybZero || !(yaPositive ^ ybPositive)   );
}

bool ScalarFunctionModel::IsDifferentSign(const float64 &ya, const float64 &yb) const
{
	const bool yaZero = fabs(ya) < DBL_EPSILON;
	const bool ybZero = fabs(yb) < DBL_EPSILON;
	const bool yaPositive = ya > 0;
	const bool ybPositive = yb > 0;
	return (   (yaPositive ^ ybPositive) && (!yaZero) && (!ybZero)   ); //If yaPositive AND ybPositive, XOR returns false
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
        #ifdef DEBUG_TRACE_SEARCH
            if(bTraceEnable) std::cout << "\t\t\t(x1,y1)=" << x1 << std::flush;
        #endif
    const float64 y1 = f(x1);
        #ifdef DEBUG_TRACE_SEARCH
            if(bTraceEnable)
            {  std::cout <<","<< y1 << endl;
               std::cout << "\t\t\t(x2,y2)=" << x2 << std::flush; }
        #endif
    const float64 y2 = f(x2);
        #ifdef DEBUG_TRACE_SEARCH
            if(bTraceEnable) std::cout <<","<< y2 << endl;
        #endif

    const float64 xb = bisectionStep(x1,x2);
    const float64 yb = f(xb);


        #ifdef DEBUG_TRACE_SEARCH
			if(bTraceEnable)
			{
				std::streamsize olprec = std::cout.precision();
				std::cout.precision(16);
				std::cout << "\t\t\t(xb,yb)=" << xb <<","<< yb << endl;
				std::cout.precision(olprec);
			}
        #endif

	
	if( yb <= y1 && yb <= y2)
	{
		//Likely too flat. Only search the more promising edge regions
	    if( x2 - x1 > quantum/2 )
	    {
	        #ifdef SINGLETURNINGPOINT
	        cout << "MISUAGE OF FindTurningPoint!!!!!!!!!!!!!!!!!!" << endl;
	        exit(1);
	        #else

			float64 x1b = xb;
			float64 x2b = xb;
			float64 x1b_outer, x2b_outer;
			float64 y1b, y2b;
			
			do{
				x1b_outer = x1b;
				x2b_outer = x2b;
				x1b = bisectionStep(x1,x1b);
				x2b = bisectionStep(x2,x2b);

				y1b = f(x1b);
				y2b = f(x2b);

			}while( y1b <= y1 && y2b <= y2  &&  x1b - x1 > quantum/2 && x2 - x2b > quantum/2 );

	        float64 leftmax = round(FindMax(x1,x1b_outer)/quantum)*quantum;
	        float64 rightmax = round(FindMax(x2b_outer,x2)/quantum)*quantum;
	        if( f(leftmax) > f(rightmax) )
	        {
	            return leftmax;
	        }
	        return rightmax;

	        #endif
	    }

        #ifdef DEBUG_TRACE_SEARCH
            if(bTraceEnable) std::cout << "\t\t  Reduced to (xb,yb)=" << xb <<","<< yb << endl;
        #endif

        return round(xb/quantum)*quantum;
    }
        #ifdef DEBUG_TRACE_SEARCH
            if(bTraceEnable) std::cout << "\t\t  FindTurningPoint" << endl;
        #endif
	return FindTurningPoint(x1, y1, xb, yb, x2, y2, 1);
}

float64 ScalarFunctionModel::FindMin(float64 x1, float64 x2)
{
    const float64 y1 = f(x1);
    const float64 y2 = f(x2);

    const float64 xb = bisectionStep(x1,x2);
    const float64 yb = f(xb);

	if( yb >= y1 && yb >= y2)
	{
		if( x2 - x1 > quantum/2 )
		{
			#ifdef SINGLETURNINGPOINT
			cout << "MISUAGE OF FindTurningPoint!!!!!!!!!!!!!!!!!!" << endl;
			exit(1);
			#else

			float64 x1b = xb;
			float64 x2b = xb;
			float64 x1b_outer, x2b_outer;
			float64 y1b, y2b;
			
			do{
				x1b_outer = x1b;
				x2b_outer = x2b;
				x1b = bisectionStep(x1,x1b);
				x2b = bisectionStep(x2,x2b);

				y1b = f(x1b);
				y2b = f(x2b);

			}while( y1b >= y1 && y2b >= y2  &&  x1b - x1 > quantum/2 && x2 - x2b > quantum/2 );

			float64 leftmin = round(FindMin(x1,x1b_outer)/quantum)*quantum;
			float64 rightmin = round(FindMin(x2b_outer,x2)/quantum)*quantum;
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

float64 ScalarFunctionModel::SplitTurningPoint(float64 x1, float64 xa, float64 xb, float64 x2, float64 signDir)
{
        #ifdef DEBUG_TRACE_SEARCH
            if(bTraceEnable) std::cout << "\t\tSplit <" << x1 << "," << xa << "," << xb << "," << x2 << ">" << std::endl;
        #endif

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

    float64 yn;
    float64 xn;

	if( y1 == yb && yb == y2 )
	{
	    #ifdef SINGLETURNINGPOINT
		return xb;
		#else
        xn = trisectionStep(x1,y1,xb,yb,x2,y2);
        //yn = f(xn);
        return SplitTurningPoint(x1, xn, xb ,x2 ,signDir);
		#endif
	}

    {
	bool bSlopes = false;
    float64 dy1 = 0.0;
    float64 dy2 = 0.0;
    float64 dyb = 0.0;

    while( IsDifferentSign((y1-yb),(y2-yb)) && x2 - x1 > quantum/2)
    {   ///(y1-yb) and (y2-yb) have different signs
        ///therefore y1 and y2 are OPPOSITE vertical directions from yb.

        dyb = fd(xb,yb);

		if( y1*signDir > y2*signDir ) ///y1 is closer
		{
			x2 = xb;
			y2 = yb;
			dy2 = dyb;
		}else//y2 is closer
		{
			x1 = xb;
			y1 = yb;
			dy1 = dyb;
		}


        ++stepMode;
        stepMode %= 3;


        if( !bSlopes )
		{
			dy1 = fd(x1,y1);
			dy2 = fd(x2,y2);
			bSlopes = true;
		}

        //Newton step:
        //  f1 = y1 + (x-x1)*dy1
        //  f2 = y2 + (x-x2)*dy2
        //  Locus f1=f2: y1 + x*dy1 - x1*dy1 = y2 + x*dy2 - x2*dy2
        //               x(dy1 - dy2) = (y2 - y1) - (x2*dy2 - x1*dy1)
        const float64 newtonxb = ((y2 - y1) - (x2*dy2 - x1*dy1))/(dy1 - dy2);
        //Old: const float64 xb = fabs((dy2*x1 - dy1*x2)/(dy2-dy1));
        //Old: if dy2*dy1 <= 0 && fabs((dy2 - dy1)/dy2) >= fabs(quantum/2/(x2 - x1))


        #ifdef DEBUG_TRACE_SEARCH
            if(bTraceEnable) std::cout << "\t\t\tNewton <newtonxb, dy1,dy2> <" << newtonxb << ", " << dy1 << "," << dy2 << ">" << std::endl;
        #endif

        if( stepMode > 0 && newtonxb < x2 - quantum/2 && newtonxb > x1 + quantum/2 )
        {
            xb = newtonxb;
        }else
        {
            xb = bisectionStep(x1,x2);
        }

		yb = f(xb);


        #ifdef DEBUG_TRACE_SEARCH
            if(bTraceEnable) std::cout << "\t\t\tSlide x<" << x1 << "," << xb << "," << x2 << ">  y<" << y1 << "," << yb << "," << y2 << ">" << std::endl;
        #endif
	}
    }
    /*
    if( !bSlopes )
    {
        dy1 = fd(x1,y1);
        dy2 = fd(x2,y2);
        //bSlopes = true;
    }
    dyb = fd(xb,yb);
*/
    
    while(x2 - x1 > quantum/2)
    {
        #ifdef DEBUG_TRACE_SEARCH
            if(bTraceEnable) std::cout << "\t\t\tGo x<" << x1 << "," << x2 << ">  y<" << y1 << "," << y2 << ">" << std::endl;
        #endif

        ++stepMode;
        stepMode %= 2;
        switch(stepMode)
        {
			case 0:
                xn = searchStep(x1,y1,xb,yb,x2,y2);
                break;
            default:
                xn = quadraticStep(x1,y1,xb,yb,x2,y2);
                if(!( xb != xn && xn < x2 && xn > x1 ))
                {
                    xn = searchStep(x1,y1,xb,yb,x2,y2);
                }
				else if( fabs(xn - xb) < fabs(xn - x1)/2 && fabs(xn - xb) < fabs(xn - x2)/2 )
				{//Very close to xb, try to tighten bounds
					xn += xn - xb;
				}
                break;
        }

        yn = f(xn);

        // If x1 and x2 differ by quantum/2, then a golden section search would be
        //    quantum/2/phi/phi
        //  = quantum / 2 / (1 + sqrt(5))^2 * 2^2 = quantum * 2 / (1 + sqrt(5))^2 = quantum * 2 / (6 + 2 * sqrt(5)) = quantum / (3 + sqrt(5))
        if( fabs(xb - xn) < quantum / (3.0 + sqrt(5.0)) )
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

                    if( fabs(dyb) < DBL_EPSILON )
                    {
                        return xb;
                    }
                    if( fabs(dyn) < DBL_EPSILON )
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
                //Then don't BYPASS_ANOMALIES
                    cout << "WARNING: Function has multiple turning points!" << endl;
                    exit(1);
                    return xb;
                #endif
            #else
            //Then don't SINGLE_TURNING_POINT
                return SplitTurningPoint(x1, xn, xb, x2, signDir);
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


float64 ScalarFunctionModel::newtonStep(float64 x1, float64 y1, float64 dy1)
{
    return x1 - y1/dy1;
}

float64 ScalarFunctionModel::bisectionStep(float64 x1, float64 x2) const
{
    return (x1+x2)/2;
}

float64 ScalarFunctionModel::regularfalsiStep(float64 x1, float64 y1, float64 x2, float64 y2) const
{//A single step from the method of False Positions
    if (y2 == y1) {
        return bisectionStep(x1, x2);
    }


	return (x1*y2 - x2*y1)/(y2 - y1);
}

float64 ScalarFunctionModel::FindZero(float64 x1, float64 x2, bool bRoundToQuantum)
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
    float64 xb;

    xb = regularfalsiStep(x1,y1,x2,y2);
    yb = f(xb);

        #ifdef DEBUG_TRACE_ZERO
            if(bTraceEnable)
            {
                std::cout << "FindZero (x1,xb,x2) = (" << x1 <<","<< xb <<","<< x2 << ")" << endl;
                std::cout << "FindZero (y1,yb,y2) = (" << y1 <<","<< yb <<","<< y2 << ")" << endl;
            }
        #endif


	int8 stepMode = 0;
    while( x2 - x1 > quantum )
    {

		const float64 projfd = fd(xb,yb);
		if( fabs(projfd) > DBL_EPSILON )
		{
			const float64 xn = newtonStep(xb,yb,projfd);


			#ifdef DEBUG_TRACE_ZERO
				if(bTraceEnable) std::cout << "\t\tPossible xn " << xn << std::endl;
			#endif

			bool bNewtonValid = xn < x2 - quantum/2 && xn > x1 + quantum/2;

			if(bNewtonValid)
			{
				const float64 yn = f(xn);
				///Possible shortcut
				if( IsDifferentSign(yb,yn) ) //different signs
				{
					if( xb < xn )
					{
						x1 = xb;
						y1 = yb;
						x2 = xn;
						y2 = yn;
					}else{
						x1 = xn;
						y1 = yn;
						x2 = xb;
						y2 = yb;
					}

					#ifdef DEBUG_TRACE_ZERO
						if(bTraceEnable) std::cout << "\t\t\t Shortcut! Switch to xn and xb: new (x1,x2) = (" << x1 << "," << x2 << ")" << std::endl;
					#endif

					xb = regularfalsiStep(x1,y1,x2,y2);
					yb = f(xb);
				}else if(fabs(yn) < fabs(yb)) //Newton is closer than False Position
				{//No shortcut
					xb = xn;
					yb = yn;

					#ifdef DEBUG_TRACE_ZERO
						if(bTraceEnable) std::cout << "\t\t\t No shortcut" << std::endl;
					#endif
				}
			}///Otherwise we stay with xb and yb which is false position or bisection
		}///Newton couldn't have been valid.

        #ifdef DEBUG_TRACE_ZERO
            if(bTraceEnable) std::cout << "\t\tSelected <xb,yb> = <" << xb << "," << yb << ">" << std::endl;
        #endif

        if( fabs(yb) < DBL_EPSILON ) {
            break;
            // this returns xb.
        }

        if( !IsDifferentSign(yb,y1) ) //same sign as y1
        {
            y1 = yb;
            x1 = xb;
        }
        else //same sign as y2
        {
            y2 = yb;
            x2 = xb;
        }

		++stepMode;
		stepMode%=2;
		if( stepMode == 0 )
		{
			xb = bisectionStep(x1,x2);
		}else
		{
	        xb = regularfalsiStep(x1,y1,x2,y2);
		}
        yb = f(xb);

        #ifdef DEBUG_TRACE_ZERO
            if(bTraceEnable)
            {
                std::cout << "\tNext (x1,xb,x2) = (" << x1 <<","<< xb <<","<< x2 << ")" << endl;
                std::cout << "\tNext (y1,yb,y2) = (" << y1 <<","<< yb <<","<< y2 << ")" << endl;
            }
        #endif
    }

    #ifdef DEBUG_TRACE_ZERO
        if(bTraceEnable) std::cout << "Done. f(" << round(xb/quantum)*quantum << ") is zero" << endl;
    #endif

    if (bRoundToQuantum) {
        // e.g. when it's about chip counts, (or bet denominations) we want to round to a valid chip quantity.
        return round(xb/quantum)*quantum;
    } else {
        // e.g. sometimes we're searching for raw probabilities, etc. In this case, just take the value.
        return xb;
    }
}




