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

template <class LL, class RR>
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
        const float64 saturate_min, saturate_max, saturate_upto;
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
        LL & left;
        RR & right;

        AutoScalingFunction(LL & f_left, RR & f_right, const float64 minX, const float64 maxX ,ExpectedCallD *c
				#ifdef TRANSFORMED_AUTOSCALES
							, AutoScaleType type = ALGEBRAIC_AUTOSCALE
				#endif
			)

            : ScalarFunctionModel(c->chipDenom()),HoldemFunctionModel( finequantum(f_left.quantum,f_right.quantum), c)
            , saturate_min(minX), saturate_max(maxX), saturate_upto(1)
			#ifdef TRANSFORMED_AUTOSCALES
            , AUTOSCALE_TYPE(type)
			#endif
			, bNoRange( maxX <= minX ), left(f_left), right(f_right){
                last_x = -1;
                last_sliderx = -1;
                //query(0,0);
            }
        AutoScalingFunction(LL & f_left, RR & f_right, const float64 minX, const float64 maxX, const float64 upto ,ExpectedCallD *c
				#ifdef TRANSFORMED_AUTOSCALES
							, AutoScaleType type = ALGEBRAIC_AUTOSCALE
				#endif
			)

            : ScalarFunctionModel(c->chipDenom()),HoldemFunctionModel( finequantum(f_left.quantum,f_right.quantum), c)
            , saturate_min(minX), saturate_max(maxX), saturate_upto(upto)
            #ifdef TRANSFORMED_AUTOSCALES
            , AUTOSCALE_TYPE(type)
			#endif
			, bNoRange( maxX <= minX ), left(f_left), right(f_right){
                last_x = -1;
                last_sliderx = -1;
                //query(0,0);
            }
        virtual ~AutoScalingFunction(){}

        virtual float64 f(const float64);
        virtual float64 fd(const float64, const float64);


        float64 f_raised(float64 raisefrom, const float64);
        float64 fd_raised(float64 raisefrom, const float64, const float64);
}
;




template <class LL, class RR>
class StateModel : public virtual HoldemFunctionModel
{
    private:
    float64 last_x;
    float64 y;
    float64 dy;



    void query( const float64 );

    protected:
        ExactCallBluffD & ea;
        AutoScalingFunction<LL,RR> *fp;
        bool bSingle;



    #ifdef DEBUG_TRACE_SEARCH
    public:
    #endif
    float64 gd_raised(float64 raisefrom, float64, const float64);
    public:

		int32 firstFoldToRaise;
#ifdef VERBOSE_STATEMODEL_INTERFACE
		float64 gainWithFold;
		float64 gainNormal;
		float64 gainRaised;
#endif

    float64 g_raised(float64 raisefrom, float64);

    StateModel(ExactCallBluffD & c, AutoScalingFunction<LL,RR> *function) : ScalarFunctionModel(c.tableinfo->chipDenom()),HoldemFunctionModel(c.tableinfo->chipDenom(),c.tableinfo)
    ,last_x(-1),ea(c),fp(function),bSingle(false),firstFoldToRaise(-1)
    {
        query(0);
    }


    StateModel(ExactCallBluffD &c, LL & functionL, RR & functionR) : ScalarFunctionModel(c.tableinfo->chipDenom()),HoldemFunctionModel(c.tableinfo->chipDenom(),c.tableinfo)
    ,last_x(-1),ea(c),bSingle(true),firstFoldToRaise(-1)
    {
        if( (&functionL) != (&functionR) ) //ASSERT: LL == RR !!
        {
            std::cerr << "Static Type Error. Use this constructor only when <class LL>==<class RR>." << endl;
            exit(1);
        }
        fp = new AutoScalingFunction<LL,RR>(functionL,functionR,0,0,c.tableinfo);
        query(0);
    }


    virtual ~StateModel();

	virtual float64 f(const float64);
    virtual float64 fd(const float64, const float64);

}
;


#endif

