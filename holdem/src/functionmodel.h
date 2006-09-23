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

#ifndef HOLDEM_GainModels
#define HOLDEM_GainModels

#include "functionbase.h"
#include "inferentials.h"
#include "arenaSituation.h"
#include <math.h>

#define DEBUG_GAIN




#define DEFAULT_EPS_STEP 0.001

class HoldemFunctionModel : public virtual ScalarFunctionModel
{
    protected:
    ExpectedCallD *e;
    public:

    HoldemFunctionModel(float64 step,ExpectedCallD *c) : ScalarFunctionModel(step),e(c){};

    virtual float64 FindBestBet();


    #ifdef DEBUG_GAIN
        void breakdown(float64 points, std::ostream& target, float64 start=0, float64 end=1)
        {

            float64 dist;
            if( points > 0 ) dist = (end-start)/points;


            target << "x,gain,dgain" << std::endl;
            if( points > 0 && dist > 0 )
            {
                for( float64 i=start;i<=end;i+=dist)
                {

                    float64 y = f(i);


                    target << i << "," << y << "," << fd(i,y) << "," << std::endl;

                }
            }else
            {
                target << end << "," << f(end) << "," << fd(end,f(end)) << std::endl;
            }


        }
    #endif

}
;

/*
class DummyFunctionModel : public virtual ScalarFunctionModel
{
	public:
	DummyFunctionModel(float64 step) : ScalarFunctionModel(step){};
    virtual float64 f(const float64) const;
    virtual float64 fd(const float64, const float64) const;
}
;
*/

class GainModel : public virtual HoldemFunctionModel
{
	protected:
	StatResult shape;
	uint8 e_battle;
	float64 p_cl;
	float64 p_cw;
	public:
	static StatResult ComposeBreakdown(const float64 pct, const float64 wl);
	GainModel(const StatResult s,ExpectedCallD *c)
		: ScalarFunctionModel(c->chipDenom()),HoldemFunctionModel(c->chipDenom(),c),shape(s),e_battle(c->handsDealt()-1)
		{
//		    const float64 t = c->chipDenom();
		    if( quantum == 0 ) quantum = 1;
            p_cl =  1 - pow(1 - shape.loss,e_battle);
            p_cw = pow(shape.wins,e_battle);
		}

    virtual ~GainModel();

	virtual float64 f(const float64);
    virtual float64 fd(const float64, const float64);

    #ifdef DEBUG_GAIN
        void breakdown(float64 points, std::ostream& target, float64 start=0, float64 end=1)
        {

            float64 dist;
            if( points > 0 ) dist = (end-start)/points;


            target << "x,gain,dgain,exf.chips,dexf,win,lose" << std::endl;
            if( points > 0 && dist > 0 )
            {
                for( float64 i=start;i<=end;i+=dist)
                {
                    float64 y = f(i);
                    //float64 exf = e->pctWillCall(vodd);
                    //float64 dexf = e->pctWillCallD(vodd) * f_pot / (2*i+f_pot) /(2*i+f_pot);
                    float64 exf = e->exf(i);
                    float64 dexf = e->dexf(i);

                    target << i << "," << y << "," << fd(i,y) << "," << exf << "," << dexf << "," << p_cw << "," << p_cl << std::endl;

                }
            }else
            {
                target << end << "," << f(end) << "," << fd(end,f(end)) << "," << e->exf(end) << "," << e->dexf(end) << "," << p_cw << "," << p_cl <<std::endl;
            }


        }

        void breakdownC(float64 points, std::ostream& target, float64 start=0, float64 end=1)
        {
            float64 dist = (end-start)/points;

            target << "i,exf.chips,dexf" << std::endl;
            for( float64 i=start;i<end;i+=dist)
            {
                //float64 exf = e->pctWillCall(vodd);
                //float64 dexf = e->pctWillCallD(vodd);
                float64 exf = e->exf(i);
                float64 dexf = e->dexf(i);
                target << i << "," << exf << "," << dexf << std::endl;

            }


        }
       /* void breakdownE(float64 points, std::ostream& target)
        {
            e->breakdown(points,target);

        }*/
    #endif
}
;

class GainModelNoRisk : public virtual GainModel
{
    public:
	GainModelNoRisk(const StatResult s,ExpectedCallD *c) : ScalarFunctionModel(c->chipDenom()),HoldemFunctionModel(c->chipDenom(),c),GainModel(s,c){}
	virtual ~GainModelNoRisk();

	virtual float64 f(const float64);
    virtual float64 fd(const float64, const float64);
}
;

class GainModelReverseNoRisk : public virtual GainModel
{
    public:

    //virtual float64 FindBestBet();

	GainModelReverseNoRisk(const StatResult s,ExpectedCallD *c) : ScalarFunctionModel(c->chipDenom()),HoldemFunctionModel(c->chipDenom(),c),GainModel(s,c){}
	virtual ~GainModelReverseNoRisk();

	virtual float64 f(const float64);
    virtual float64 fd(const float64, const float64);
}
;

class GainModelReverse : public virtual GainModel
{
    public:

    //virtual float64 FindBestBet();

	GainModelReverse(const StatResult s,ExpectedCallD *c) : ScalarFunctionModel(c->chipDenom()),HoldemFunctionModel(c->chipDenom(),c),GainModel(s,c){}
	virtual ~GainModelReverse();

	virtual float64 f(const float64);
    virtual float64 fd(const float64, const float64);
}
;



class SlidingPairFunction : public virtual HoldemFunctionModel
{
    protected:
        virtual void query(float64 x);
        float64 slider;
        float64 last_x;
        float64 y;
        float64 dy;
        ScalarFunctionModel *left;
        ScalarFunctionModel *right;
    public:
	SlidingPairFunction(ScalarFunctionModel *f_left, ScalarFunctionModel *f_right, const float64 scale,ExpectedCallD *c)
	: ScalarFunctionModel(c->chipDenom()),HoldemFunctionModel(f_left->quantum*(1-scale)+f_right->quantum*scale, c), slider(scale), left(f_left), right(f_right){
	    query(0);
	    }
	virtual ~SlidingPairFunction(){}

	virtual float64 f(const float64);
    virtual float64 fd(const float64, const float64);

}
;

#endif



