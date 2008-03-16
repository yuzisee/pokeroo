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

#ifndef HOLDEM_ScalarFunctions
#define HOLDEM_ScalarFunctions

#include "portability.h"



#undef DEBUG_TRACE_SEARCH
#undef SINGLETURNINGPOINT
#define BYPASS_ANOMALIES


#ifdef DEBUG_TRACE_SEARCH
#include <iostream>
#endif

class ScalarFunctionModel
{
    private:
        #ifndef SINGLETURNINGPOINT
        float64 SplitTurningPoint(float64 x1,float64 y1,float64 xb,float64 yb,float64 xn,float64 yn,float64 x2,float64 y2,float64 signDir);
        #endif
    protected:
        float64 trisectionStep(float64,float64,float64,float64,float64,float64) const;
        float64 searchStep(float64,float64,float64,float64,float64,float64);
        float64 quadraticStep(float64,float64,float64,float64,float64,float64) const;
        float64 newtonStep(float64,float64);
        float64 bisectionStep(float64,float64) const;
		virtual float64 FindTurningPoint(float64 x1,float64 y1,float64 xb,float64 yb,float64 x2,float64 y2,float64 signDir);
    public:
    float64 quantum;

    #ifdef DEBUG_TRACE_SEARCH
    bool bTraceEnable;
    #endif

    ScalarFunctionModel(float64 step) : quantum(step)
    #ifdef DEBUG_TRACE_SEARCH
    ,bTraceEnable(false)
    #endif
    {};
    virtual float64 f(const float64) = 0;
    virtual float64 fd(const float64, const float64) = 0;
	virtual float64 FindMax(float64,float64) ;
	virtual float64 FindMin(float64,float64) ;
	virtual float64 FindZero(float64,float64) ;
    virtual ~ScalarFunctionModel();


}
;



#endif


