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
#include "callSituation.h"
#include <math.h>

#ifdef DEBUGSPECIFIC
#define DEBUG_GAIN
#endif
#define NO_AWKWARD_MODELS



#define DEFAULT_EPS_STEP 0.001

class HoldemFunctionModel : public virtual ScalarFunctionModel
{
    protected:
    ExpectedCallD *e;
    public:

    HoldemFunctionModel(float64 step,ExpectedCallD *c) : ScalarFunctionModel(step),e(c){};

    virtual float64 FindBestBet();
    virtual float64 FindFoldBet(const float64);

    const float64 GetFoldGain() const;

    #ifdef DEBUG_GAIN
        void breakdown(float64 points, std::ostream& target, float64 start=0, float64 end=1)
        {
            target.precision(17);

            float64 dist;
            if( points > 0 ) dist = (end-start)/points;


            target << "x,gain,dgain,wch,dwch" << std::endl;
            if( points > 0 && dist > 0 )
            {
                for( float64 i=start;i<=end;i+=dist)
                {

                    float64 y = f(i);


                    target << i << "," << y << "," << fd(i,y) << "," << e->betFraction(e->exf(i))  << "," <<  e->dexf(i) << std::endl;

                }
            }else
            {
                target << end << "," << f(end) << "," << fd(end,f(end)) << "," << e->betFraction(e->exf(end))  << "," <<  e->dexf(end) << std::endl;
            }

            target.precision(6);


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
	float64 f_battle;
	uint8 e_battle;
	float64 p_cl;
	float64 p_cw;
	public:
	static StatResult ComposeBreakdown(const float64 pct, const float64 wl);
	GainModel(const StatResult s,ExpectedCallD *c)
		: ScalarFunctionModel(c->chipDenom()),HoldemFunctionModel(c->chipDenom(),c),shape(s)
		{
		    f_battle = (c->callingPlayers());
		    //e_battle = static_cast<int8>(f_battle); //Truncate
		    e_battle = c->handsDealt()-1;
//		    const float64 t = c->chipDenom();
		    if( quantum == 0 ) quantum = 1;

		    float64 oldTotal = 1 - pow(1 - shape.loss,e_battle) + pow(shape.wins,e_battle);

            //std::cout << "Old <p_cl,p_cw>: " <<  1 - pow(1 - shape.loss,e_battle) << "," << pow(shape.wins,e_battle) << endl;
            //std::cout << "e_battle is " << (int)e_battle << "\tf_battle is " << f_battle << endl;
        ///Use f_battle instead of e_battle
            p_cl =  1 - pow(1 - shape.loss,f_battle);
            p_cw = pow(shape.wins,f_battle);

            float64 newTotal = p_cl + p_cw;
        ///Since the ratios are different, make the adjustment to normalize
            p_cl *= oldTotal/newTotal;
            p_cw *= oldTotal/newTotal;

            //std::cout << "New <p_cl,p_cw>: " << p_cl << "," << p_cw << endl;
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
                target << end << "," << f(end);
                target << "," << fd(end,f(end));
                target << "," << e->exf(end);
                target << "," << e->dexf(end);
                target << "," << p_cw << "," << p_cl <<std::endl;
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

class SlidingPairFunction : public virtual HoldemFunctionModel
{
    protected:
        virtual void query(float64 x);
        const float64 slider;
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

class AutoScalingFunction : public virtual HoldemFunctionModel
{
    private:
        float64 inline finequantum(float64 a, float64 b)
        {
            if( a < b ) return a;
            return b;
        }
    protected:
        virtual void query(float64 x);
        const float64 saturate_min, saturate_max, saturate_upto;
        float64 last_x;
        float64 y;
        float64 dy;
        ScalarFunctionModel *left;
        ScalarFunctionModel *right;

    public:
        AutoScalingFunction(ScalarFunctionModel *f_left, ScalarFunctionModel *f_right, const float64 minX, const float64 maxX ,ExpectedCallD *c)
            : ScalarFunctionModel(c->chipDenom()),HoldemFunctionModel( finequantum(f_left->quantum,f_right->quantum), c)
            , saturate_min(minX), saturate_max(maxX), saturate_upto(1), left(f_left), right(f_right){
                query(0);
            }
        AutoScalingFunction(ScalarFunctionModel *f_left, ScalarFunctionModel *f_right, const float64 minX, const float64 maxX, const float64 upto ,ExpectedCallD *c)
            : ScalarFunctionModel(c->chipDenom()),HoldemFunctionModel( finequantum(f_left->quantum,f_right->quantum), c)
            , saturate_min(minX), saturate_max(maxX), saturate_upto(upto), left(f_left), right(f_right){
                query(0);
            }
        virtual ~AutoScalingFunction(){}

        virtual float64 f(const float64);
        virtual float64 fd(const float64, const float64);

}
;

#endif



