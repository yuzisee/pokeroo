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

#ifndef HOLDEM_StateModel
#define HOLDEM_StateModel

#include "functionmodel.h"

#define VERBOSE_STATEMODEL_INTERFACE


#undef TRANSFORMED_AUTOSCALES

#ifdef TRANSFORMED_AUTOSCALES
enum AutoScaleType { ALGEBRAIC_AUTOSCALE, LOGARITHMIC_AUTOSCALE };
#endif

enum SliderBehaviour {
    SLIDERX // In this mode, Pr{raisedAgainst} and U{raisedAgainst} are evaluated at sliderx===betSize
    // Use this for legacy PlayerStrategies that use smoothly varying AutoScalingFunctions to create deterrents, etc.
    ,
    RAW // In this mode, slider is always equal to showdownBetSize.
    // Use this for pure PlayerStrategies that use discrete AutoScalingFunctions (e.g. as a switch between calling and raising).
};


class AutoScalingFunction : public virtual HoldemFunctionModel
{//NO ASSIGNMENT OPERATOR
private:
    float64 inline finequantum(float64 a, float64 b)
    {
        if( a < b ) return a;
        return b;
    }


protected:
    virtual void query(float64 sliderx, float64 x);
    const float64 saturate_min, saturate_max, saturate_upto; // saturate_upto is usually a number between 0.0 and 1.0 that causes a reduced max
    float64 last_x;
    float64 last_sliderx;
    float64 y;
    float64 dy;

    float64 yl;
    float64 yr;
    float64 fd_yl;
    float64 fd_yr;

public:

#ifdef TRANSFORMED_AUTOSCALES
    const AutoScaleType AUTOSCALE_TYPE;
#endif

    const bool bNoRange;
    IFunctionDifferentiable & left;
    IFunctionDifferentiable & right;

    const SliderBehaviour fSliderBehaviour;

    AutoScalingFunction(IFunctionDifferentiable & f_left, IFunctionDifferentiable & f_right, const float64 minX, const float64 maxX ,ExpectedCallD *c
#ifdef TRANSFORMED_AUTOSCALES
                        , AutoScaleType type = ALGEBRAIC_AUTOSCALE
#endif

                        ,
                        SliderBehaviour sliderBehaviour
    )

    : ScalarFunctionModel(c->chipDenom()),HoldemFunctionModel( finequantum(f_left.getQuantum(),f_right.getQuantum()), c)
    , saturate_min(minX), saturate_max(maxX), saturate_upto(1)
#ifdef TRANSFORMED_AUTOSCALES
    , AUTOSCALE_TYPE(type)
#endif
    , bNoRange( maxX <= minX ), left(f_left), right(f_right)
    ,
    fSliderBehaviour(sliderBehaviour)
    {
        last_x = -1;
        last_sliderx = -1;
        //query(0,0);
    }
    AutoScalingFunction(IFunctionDifferentiable & f_left, IFunctionDifferentiable & f_right, const float64 minX, const float64 maxX, const float64 upto ,ExpectedCallD *c
#ifdef TRANSFORMED_AUTOSCALES
                        , AutoScaleType type = ALGEBRAIC_AUTOSCALE
#endif
    )

    : ScalarFunctionModel(c->chipDenom()),HoldemFunctionModel( finequantum(f_left.getQuantum(),f_right.getQuantum()), c)
    , saturate_min(minX), saturate_max(maxX), saturate_upto(upto)
#ifdef TRANSFORMED_AUTOSCALES
    , AUTOSCALE_TYPE(type)
#endif
    , bNoRange( maxX <= minX ), left(f_left), right(f_right)
    ,
    fSliderBehaviour(SLIDERX)
    {
        last_x = -1;
        last_sliderx = -1;
        //query(0,0);
    }
    virtual ~AutoScalingFunction(){}

    virtual float64 f(const float64) override final;
    virtual float64 fd(const float64, const float64) override final;


    float64 f_raised(float64 raisefrom, const float64);
    float64 fd_raised(float64 raisefrom, const float64, const float64);
}
;


struct AggregatedState {
    // Overall contribution: outcome value combined with probability of that outcome
    float64 contribution;
    float64 dContribution;

    // Value of this particular outcome, if it were certain
    // If this outcome is a combination of sub-outcomes, this value will represent a "blended" value
    float64 value;

    // Probability of this outcome.
    float64 pr;

    AggregatedState()
    :
    contribution(std::numeric_limits<float64>::signaling_NaN())
    ,
    dContribution(std::numeric_limits<float64>::signaling_NaN())
    ,
    value(std::numeric_limits<float64>::signaling_NaN())
    ,
    pr(std::numeric_limits<float64>::signaling_NaN())
    {}
}
;

// All values should be expressed as a fraction of your bankroll (so that Geom and Algb can be compared directly)
// 1.0 is "no change"
class IStateCombiner {
public:
    virtual ~IStateCombiner() {}

    virtual struct AggregatedState createOutcome(float64 value, float64 probability, float64 dValue, float64 dProbability) const = 0;
    virtual struct AggregatedState createBlendedOutcome(size_t arraySize, float64 * values, float64 * probabilities, float64 * dValues, float64 * dProbabilities) const = 0;

    virtual struct AggregatedState combinedContributionOf(const struct AggregatedState &a, const struct AggregatedState &b, const struct AggregatedState &c) const = 0;
}
;

class GeomStateCombiner : public IStateCombiner {
public:
    virtual ~GeomStateCombiner() {}

    struct AggregatedState createOutcome(float64 value, float64 probability, float64 dValue, float64 dProbability) const override final;
    struct AggregatedState createBlendedOutcome(size_t arraySize, float64 * values, float64 * probabilities, float64 * dValues, float64 * dProbabilities) const override final;

    struct AggregatedState combinedContributionOf(const struct AggregatedState &a, const struct AggregatedState &b, const struct AggregatedState &c) const override final;
};

class AlgbStateCombiner : public IStateCombiner {
public:
    virtual ~AlgbStateCombiner() {}

    struct AggregatedState createOutcome(float64 value, float64 probability, float64 dValue, float64 dProbability) const override final;
    struct AggregatedState createBlendedOutcome(size_t arraySize, float64 * values, float64 * probabilities, float64 * dValues, float64 * dProbabilities) const override final;

    struct AggregatedState combinedContributionOf(const struct AggregatedState &a, const struct AggregatedState &b, const struct AggregatedState &c) const override final;
};


// Evaluate gainWithFold*gainNormal*gainRaised
// on an AutoScalingFunction
// and relative to foldGain and everything.
class StateModel : public virtual HoldemFunctionModel
{
private:
    float64 last_x;
    float64 y;
    float64 dy;


    // I will use <tt>ea</tt> to determine the push and raised chances, and
    // I call g_raised in two places to determine gainNormal as well as (in a loop,) gainRaised
    void query( const float64 );

protected:
    ExactCallBluffD & ea;
    FoldOrCall fMyFoldGain; // My current foldgain with the same units as my CombinedStatResult (for proper comparison with call vs. fold)
    const IStateCombiner & fStateCombiner;
    AutoScalingFunction *fp;
    const bool bSingle;


#ifdef DEBUG_TRACE_SEARCH
public:
#endif
    float64 gd_raised(float64 raisefrom, float64, const float64);
public:

    int32 firstFoldToRaise;

    // Outcome if push succeeds (e.g. all fold to you)
    struct AggregatedState outcomePush;

    // Outcome if bet is called with no raises (e.g. calls only)
    struct AggregatedState outcomeCalled;

    // "Effective outcome" of all possibilities where bet is eventually raised (either now or in a future round)
    // It's blended because there are actually several raise possibilities and this blends them into one.
    struct AggregatedState blendedRaises;


    float64 g_raised(float64 raisefrom, float64);

    StateModel(ExactCallBluffD & c, AutoScalingFunction *function, const IStateCombiner & stateCombiner

               )
    : ScalarFunctionModel(c.tableinfo->chipDenom()),HoldemFunctionModel(c.tableinfo->chipDenom(),c.tableinfo)
    ,last_x(std::numeric_limits<float64>::signaling_NaN())
    ,
    ea(c)
    ,
    fMyFoldGain(*(c.tableinfo->table),c.fCore)
    ,
    fStateCombiner(stateCombiner)
    ,
    fp(function),bSingle(false)
    ,
    firstFoldToRaise(-1)
    {
    }
    
    /*
     StateModel(ExactCallBluffD &c, IFunctionDifferentiable &functionL, IFunctionDifferentiable &functionR) : ScalarFunctionModel(c.tableinfo->chipDenom()),HoldemFunctionModel(c.tableinfo->chipDenom(),c.tableinfo)
     ,last_x(-1)
     ,
     ea(c)
     ,
     fMyFoldGain(*(c.tableinfo->table), c.fCore)
     ,
     bSingle(true),firstFoldToRaise(-1)
     {
     if( ((HoldemFunctionModel *)(&functionL)) != (HoldemFunctionModel *)(&functionR) ) //ASSERT: LL == RR !!
     {
     std::cerr << "Static Type Error. Use this constructor only when <class LL>==<class RR>." << endl;
     exit(1);
     }
     fp = new AutoScalingFunction(functionL,functionR,0,0,c.tableinfo);
     query(0);
     }*/


    virtual ~StateModel();

	virtual float64 f(const float64);
    virtual float64 fd(const float64, const float64);

    static bool willFoldToReraise
    (
     const float64 raiseAmount
     ,
     const float64 playGain
     ,
     FoldOrCall & fMyFoldGain
     ,
     ExpectedCallD & myInfo
     ,
     const float64 hypotheticalMyRaiseTo
     );



#ifdef DUMP_CSV_PLOTS
	float64 oppFoldChance;
	float64 playChance;
    void dump_csv_plots(std::ostream &targetoutput, float64 betSize)
    {


        targetoutput.precision(4);
        targetoutput << oppFoldChance << "," << playChance << "," << std::flush;


    }
#endif

}
;



#endif

