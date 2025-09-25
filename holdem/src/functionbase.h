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

#ifndef HOLDEM_ScalarFunctions
#define HOLDEM_ScalarFunctions

#include "portability.h"
#include "debug_flags.h"

//------------------------
//Behaviour
#undef SINGLETURNINGPOINT
#define BYPASS_ANOMALIES

//------------------------
//Bookkeeping
#if defined(DEBUG_TRACE_SEARCH) || defined(DEBUG_TRACE_ZERO)
#include <iostream>
#endif


class IFunctionDifferentiable {
public:


    virtual ~IFunctionDifferentiable() {}

    virtual float64 f(const float64) = 0;
    virtual float64 fd(const float64 x, const float64 y) = 0;

    virtual float64 getQuantum() const = 0;
}
;

// TODO(from yuzisee): This doesn't require inheritance. Use composition instead.
class ScalarFunctionModel : public IFunctionDifferentiable
{
    private:
        #ifndef SINGLETURNINGPOINT
        float64 SplitTurningPoint(float64 x1, float64 xb, float64 xn, float64 x2, float64 signDir);
        #endif

		constexpr bool IsDifferentSign(const float64&,const float64&) const;
		constexpr bool IsSameSignOrZero(const float64&,const float64&) const;
    protected:

        // When no other information is available, just pick an x based on the other x values.
        float64 trisectionStep(float64,float64,float64,float64,float64,float64) const;

        float64 searchStep(float64,float64,float64,float64,float64,float64);
        float64 quadraticStep(float64,float64,float64,float64,float64,float64) const;
        float64 newtonStep(float64,float64,float64);
        float64 bisectionStep(float64,float64) const;
		float64 regularfalsiStep(float64,float64,float64,float64) const;
		virtual float64 FindTurningPoint(float64 x1,float64 y1,float64 xb,float64 yb,float64 x2,float64 y2,float64 signDir);
    public:
    float64 quantum;
    float64 getQuantum() const override final { return quantum; }

    #if defined(DEBUG_TRACE_SEARCH) || defined(DEBUG_TRACE_ZERO)
    std::ostream * traceEnable;
    #endif

    ScalarFunctionModel(float64 step) : quantum(step)
    #if defined(DEBUG_TRACE_SEARCH) || defined(DEBUG_TRACE_ZERO)
    ,traceEnable(nullptr)
    #endif
    {};
	virtual float64 FindMax(float64,float64) ;
	virtual float64 FindMin(float64,float64) ;
	virtual float64 FindZero(float64,float64, bool bRoundToQuantum) ;
    virtual ~ScalarFunctionModel();


}
;



#endif
